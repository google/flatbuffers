// Reflection verifier union security regression tests.
//
// Covers the fixes for the reflection-union verifier defects:
//   1. Union member resolution by declared VALUE, not by values() position,
//      with the discriminator read at the enum's underlying width (so sparse
//      unions such as `union ABC : int { A = 555, B = 666 }` verify correctly).
//   2. Bounds checks on Type.index before any schema.objects()->Get() /
//      enums()->Get() in the reflection verifier (SchemaIsValid + guards).
//   3. Null-safe GetUnionType for reflected consumers (CopyTable/ResizeTable).
//
// Regression assertions (previously failing / crashing):
//   * a legitimate buffer with discriminator 555 (stored int32) now VERIFIES
//     (previously rejected because the tag was read as a uint8).
//   * a hand-crafted tag that is a position but not a declared value (1) is
//     REJECTED (previously accepted -> type confusion).
//   * a schema whose union-member Type.index is out of range is rejected by
//     reflection::Verify (SchemaIsValid) instead of performing an OOB read.
//   * CopyTable on an unresolvable-union buffer no longer crashes.
//
// Self-contained: embeds the schema, no external .fbs/.bfbs/flatc needed.
//
// Registered from tests/test.cpp as ReflectionUnionSecurityTest() (the same way
// ReflectionTest() is), and compiled into the `flattests` target via
// FlatBuffers_Tests_SRCS in CMakeLists.txt.

#include "flatbuffers/idl.h"
#include "flatbuffers/reflection.h"
#include "flatbuffers/reflection_generated.h"
#include "flatbuffers/util.h"
#include "reflection_test.h"
#include "test_assert.h"
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

using namespace flatbuffers;

namespace flatbuffers {
namespace tests {

namespace {

const char* kSparseSchema =
    "namespace P;"
    "table A { a:int; }"        // objects[0]
    "table B { b:string; }"     // objects[1]
    "union ABC:int { A = 555, B = 666 }"
    "table D { test_union:ABC; }"
    "root_type D;";

const char* kDenseSchema =
    "namespace P;"
    "table A { a:int; }"        // objects[0]
    "table B { b:string; }"     // objects[1]
    "union U:int { A = 1, B = 2 }"
    "table Root { u:U; }"
    "root_type Root;";

// Build D{ test_union_type = tag, test_union = A{ a=7 } or B{ b="..." } }.
std::vector<uint8_t> BuildD(const std::string& schema_text, int32_t tag,
                            bool payload_is_b) {
  Parser parser;
  if (!parser.Parse(schema_text.c_str())) return {};
  FlatBufferBuilder fbb(1024);
  Offset<String> s;
  if (payload_is_b) s = fbb.CreateString("HELLO");
  uoffset_t mstart = fbb.StartTable();
  if (payload_is_b) {
    fbb.AddOffset(4, s);
  } else {
    fbb.AddElement<int32_t>(4, 7, 0);
  }
  auto member = fbb.EndTable(mstart);
  uoffset_t dstart = fbb.StartTable();
  fbb.AddElement<int32_t>(4, tag, 0);  // test_union_type (int32 on the wire)
  fbb.AddOffset(6, Offset<void>(member));
  auto d = fbb.EndTable(dstart);
  fbb.Finish(Offset<Table>(d));
  return std::vector<uint8_t>(fbb.GetBufferPointer(),
                              fbb.GetBufferPointer() + fbb.GetSize());
}

// Parse a schema text and return the in-memory reflection Schema + object.
bool GetSchemaAndObject(const std::string& schema_text, const char* obj_name,
                        Parser* parser, const reflection::Schema** schema_out,
                        const reflection::Object** obj_out) {
  if (!parser->Parse(schema_text.c_str())) return false;
  parser->Serialize();
  *schema_out = reflection::GetSchema(parser->builder_.GetBufferPointer());
  *obj_out = (*schema_out)->objects()->LookupByKey(obj_name);
  return *obj_out != nullptr;
}

// Patch a reflection::Type.index (vtable slot VT_INDEX == 8) in place.  The
// Type must live inside the (mutable) schema buffer.
void PatchTypeIndex(const reflection::Type* type, int32_t new_index) {
  const auto* tbl = reinterpret_cast<const Table*>(type);
  auto voff = tbl->GetOptionalFieldOffset(8 /* VT_INDEX */);
  TEST_ASSERT(voff != 0);
  uint8_t* loc =
      const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(type) + voff);
  memcpy(loc, &new_index, sizeof(new_index));
}

void SparseUnionTests() {
  TEST_OUTPUT_LINE("SparseUnionTests");

  Parser parser;
  const reflection::Schema* schema = nullptr;
  const reflection::Object* d_obj = nullptr;
  TEST_EQ(GetSchemaAndObject(kSparseSchema, "P.D", &parser, &schema, &d_obj),
          true);

  // (1) Legitimate buffers produced by an official builder (discriminator
  // stored as int32 = 555 / 666) must now verify.  Before the fix they were
  // rejected: the tag was read as uint8 (555 & 0xFF == 43 >= 3).
  {
    auto buf = BuildD(kSparseSchema, 555, false);
    TEST_EQ(flatbuffers::Verify(*schema, *d_obj, buf.data(), buf.size()),
            true);
  }
  {
    auto buf = BuildD(kSparseSchema, 666, true);
    TEST_EQ(flatbuffers::Verify(*schema, *d_obj, buf.data(), buf.size()),
            true);
  }

  // (2) A tag that is a valid values() position but NOT a declared member
  // value (1; declared are 555/666) must be rejected, not resolved by position.
  {
    auto buf = BuildD(kSparseSchema, 1, true);  // payload is B, tag 1
    TEST_EQ(flatbuffers::Verify(*schema, *d_obj, buf.data(), buf.size()),
            false);
  }
}

void OutOfRangeIndexTests() {
  TEST_OUTPUT_LINE("OutOfRangeIndexTests");

  Parser parser;
  const reflection::Schema* schema = nullptr;
  const reflection::Object* root_obj = nullptr;
  TEST_EQ(GetSchemaAndObject(kDenseSchema, "P.Root", &parser, &schema,
                             &root_obj),
          true);

  // Copy schema buffer so we can patch it, then re-derive the schema pointers
  // from the COPY (the parser-owned buffer must not be patched).
  std::vector<uint8_t> schema_buf(
      parser.builder_.GetBufferPointer(),
      parser.builder_.GetBufferPointer() + parser.builder_.GetSize());
  schema = reflection::GetSchema(schema_buf.data());
  root_obj = schema->objects()->LookupByKey("P.Root");
  TEST_NOTNULL(root_obj);

  // Member A (value 1) is at values()->Get(1); its union_type.index -> objects[0].
  auto u_enum = schema->enums()->LookupByKey("P.U");
  const reflection::EnumVal* ev_a = u_enum->values()->Get(1);
  const reflection::Type* a_type = ev_a->union_type();
  TEST_EQ(a_type->index(), 0);

  // Patch A's Type.index far out of range.  SchemaIsValid (called by
  // reflection::Verify) must reject the schema, so Verify returns false
  // instead of performing an out-of-bounds read.
  PatchTypeIndex(a_type, 1000);
  schema = reflection::GetSchema(schema_buf.data());
  auto ev_a2 = schema->enums()->LookupByKey("P.U")->values()->Get(1);
  TEST_EQ(static_cast<int>(ev_a2->union_type()->index()), 1000);

  TEST_EQ(flatbuffers::SchemaIsValid(*schema), false);

  // Legal data buffer (dense: tag 1 == A) against the malformed schema.
  auto buf = BuildD(kDenseSchema, 1, false);
  // reflection::Verify must return false (schema rejected) and MUST NOT crash.
  TEST_EQ(flatbuffers::Verify(*schema, *root_obj, buf.data(), buf.size()),
          false);
}

void ConsumerNullSafetyTests() {
  TEST_OUTPUT_LINE("ConsumerNullSafetyTests");

  Parser parser;
  const reflection::Schema* schema = nullptr;
  const reflection::Object* d_obj = nullptr;
  TEST_EQ(GetSchemaAndObject(kSparseSchema, "P.D", &parser, &schema, &d_obj),
          true);

  // Buffer with an unresolvable union tag (1, not a declared value).
  auto buf = BuildD(kSparseSchema, 1, true);

  // reflection::Verify rejects it...
  TEST_EQ(flatbuffers::Verify(*schema, *d_obj, buf.data(), buf.size()),
          false);

  // ...and handing it to a reflected consumer (CopyTable) must not crash:
  // GetUnionType returns failure and the union field is skipped.
  FlatBufferBuilder out(1024);
  const auto* root =
      reinterpret_cast<const Table*>(GetRoot<Table>(buf.data()));
  Offset<const Table*> copied = CopyTable(out, *schema, *d_obj, *root);
  TEST_ASSERT(!copied.IsNull());
}

}  // namespace

void ReflectionUnionSecurityTest() {
  SparseUnionTests();
  OutOfRangeIndexTests();
  ConsumerNullSafetyTests();
  TEST_OUTPUT_LINE("ReflectionUnionSecurityTest: PASSED");
}

}  // namespace tests
}  // namespace flatbuffers

