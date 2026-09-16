// Parser::Deserialize and reflection verification input-contract security
// regression tests.
//
// Covers four hardening fixes:
//   1. Length validation before identifier reads in
//      Parser::Deserialize(const uint8_t*, size_t) (fixes an out-of-bounds
//      strncmp on buffers shorter than the identifier).
//   2. SymbolTable::Add ownership fix (duplicate check before insertion) so
//      the Parser::Deserialize error paths no longer double-free.
//   3. base_type/element range validation in Type::Deserialize (prevents
//      out-of-range wire values from reaching SizeOf()'s default case).
//   4. VerifyObject treats a None-typed field as a verification failure
//      instead of asserting.
//
// Self-contained: schemas are embedded or constructed at runtime; no external
// .fbs/.bfbs/flatc needed.  Registered from tests/test.cpp and compiled into
// the `flattests` target via FlatBuffers_Tests_SRCS in CMakeLists.txt.

#include "flatbuffers/idl.h"
#include "flatbuffers/reflection.h"
#include "flatbuffers/reflection_generated.h"
#include "flatbuffers/util.h"
#include "reflection_test.h"
#include "test_assert.h"
#include <cstdint>
#include <cstdio>
#include <vector>

using namespace flatbuffers;

namespace flatbuffers {
namespace tests {

namespace {

// A 10-byte "schema" that is far too short to contain a FlatBuffer file
// identifier.  Regression input for fix (1): previously this made
// Parser::Deserialize read past the end of the buffer in BufferHasIdentifier
// (ASan: heap-buffer-overflow READ of size 1 in strncmp).
const uint8_t kShortBuffer[10] = { 0x00, 0x00, 0x00, 0x01, 0x20,
                                   0x03, 0x18, 0x20, 0x03, 0x18 };

// Build an in-memory reflection Schema for `schema_text` and return a copy of
// its serialized bytes (mutable, so tests can patch fields).
bool BuildSchemaBytes(const char* schema_text, Parser* parser,
                      std::vector<uint8_t>* schema_bytes) {
  if (!parser->Parse(schema_text)) return false;
  parser->Serialize();
  schema_bytes->assign(parser->builder_.GetBufferPointer(),
                       parser->builder_.GetBufferPointer() +
                           parser->builder_.GetSize());
  return true;
}

void EntryLengthValidationTest() {
  TEST_OUTPUT_LINE("EntryLengthValidationTest");
  Parser parser;
  // (1) Short buffer must be rejected before any identifier read: no crash,
  // Deserialize returns false.
  TEST_EQ(parser.Deserialize(kShortBuffer, sizeof(kShortBuffer)), false);
}

void SymbolTableOwnershipTest() {
  TEST_OUTPUT_LINE("SymbolTableOwnershipTest");
  // Build a serialized schema with two enums in one namespace, then patch the
  // second enum's name in-place so the two enums share the same
  // fully-qualified name (P.E1).  Parser::Deserialize's enums loop then hits
  // SymbolTable::Add on a duplicate; with the ownership fix the error path
  // returns false and destroying the parser must not double-free.
  Parser parser;
  std::vector<uint8_t> bytes;
  const char* two_enums_table =
      "namespace P;"
      "enum E1 : int { A = 1 }"
      "enum E2 : int { B = 2 }"
      "table T { }"
      "root_type T;";
  TEST_EQ(BuildSchemaBytes(two_enums_table, &parser, &bytes), true);

  const reflection::Schema* schema = reflection::GetSchema(bytes.data());
  auto enums = schema->enums();
  TEST_EQ(enums->size(), 2u);
  const reflection::Enum* e1 = enums->Get(1);
  // Enum names are stored fully qualified in the serialized schema.
  TEST_EQ_STR(e1->name()->c_str(), "P.E2");

  // Overwrite e1's name string "P.E2" (4 chars + NUL) with "P.E1" in place so
  // the fully-qualified name becomes P.E1 -- a duplicate of the first enum.
  const ::flatbuffers::String* s1 = e1->name();
  uint8_t* name_bytes = const_cast<uint8_t*>(
      reinterpret_cast<const uint8_t*>(s1->Data()));
  name_bytes[0] = 'P';
  name_bytes[1] = '.';
  name_bytes[2] = 'E';
  name_bytes[3] = '1';
  name_bytes[4] = '\0';

  // Deserialize must detect the duplicate, return false on the enums loop, and
  // destroying the parser must not crash (ASan would report a double-free if
  // the ownership fix regressed).
  Parser p2;
  TEST_EQ(p2.Deserialize(bytes.data(), bytes.size()), false);
}

void OutOfRangeBaseTypeTest() {
  TEST_OUTPUT_LINE("OutOfRangeBaseTypeTest");
  // Build a schema with a table field whose Type we can rewrite, then patch
  // its base_type to an out-of-range value (reflection::MaxBaseType == 19,
  // which has no case in the internal BaseType set).  Type::Deserialize must
  // reject it (previously the unchecked static_cast let the value through to
  // SizeOf()'s FLATBUFFERS_ASSERT(0) default in debug builds, or wrong size
  // metadata in release builds).
  Parser parser;
  std::vector<uint8_t> bytes;
  const char* field_schema =
      "namespace P;"
      "table Inner { }"
      "table Outer { i: Inner; }"
      "root_type Outer;";
  TEST_EQ(BuildSchemaBytes(field_schema, &parser, &bytes), true);

  const reflection::Schema* schema = reflection::GetSchema(bytes.data());
  auto outer = schema->objects()->LookupByKey("P.Outer");
  TEST_NOTNULL(outer);
  const reflection::Field* f = outer->fields()->Get(0);
  TEST_EQ_STR(f->name()->c_str(), "i");
  const reflection::Type* t = f->type();
  TEST_EQ(static_cast<int>(t->base_type()), static_cast<int>(reflection::Obj));

  // Patch base_type (vtable slot VT_BASE_TYPE == 4) to MaxBaseType (19).
  const auto* tbl = reinterpret_cast<const Table*>(t);
  auto voff = tbl->GetOptionalFieldOffset(4 /* VT_BASE_TYPE */);
  TEST_ASSERT(voff != 0);
  uint8_t* loc = const_cast<uint8_t*>(
      reinterpret_cast<const uint8_t*>(t) + voff);
  *loc = static_cast<uint8_t>(reflection::MaxBaseType);

  // Deserialize must reject the out-of-range base_type.
  Parser p3;
  TEST_EQ(p3.Deserialize(bytes.data(), bytes.size()), false);
}

void NoneFieldVerifyTest() {
  TEST_OUTPUT_LINE("NoneFieldVerifyTest");
  // Build a schema for T { a: int } and patch the field's base_type to None
  // (0).  reflection::Verify must treat the None-typed field as a
  // verification failure and return false (previously it hit
  // FLATBUFFERS_ASSERT(false) in VerifyObject).
  Parser parser;
  std::vector<uint8_t> bytes;
  const char* schema_txt =
      "namespace P;"
      "table T { a: int; }"
      "root_type T;";
  TEST_EQ(BuildSchemaBytes(schema_txt, &parser, &bytes), true);

  const reflection::Schema* schema = reflection::GetSchema(bytes.data());
  auto t_obj = schema->objects()->LookupByKey("P.T");
  TEST_NOTNULL(t_obj);
  const reflection::Field* f = t_obj->fields()->Get(0);
  const reflection::Type* ft = f->type();
  const auto* tbl = reinterpret_cast<const Table*>(ft);
  auto voff = tbl->GetOptionalFieldOffset(4 /* VT_BASE_TYPE */);
  TEST_ASSERT(voff != 0);
  uint8_t* loc = const_cast<uint8_t*>(
      reinterpret_cast<const uint8_t*>(ft) + voff);
  *loc = 0;  // reflection::None

  // A legal data buffer for T{ a = 1 }.
  FlatBufferBuilder fbb(1024);
  uoffset_t s = fbb.StartTable();
  fbb.AddElement<int32_t>(4, 1, 0);
  auto t = fbb.EndTable(s);
  fbb.Finish(Offset<Table>(t));

  schema = reflection::GetSchema(bytes.data());
  t_obj = schema->objects()->LookupByKey("P.T");
  TEST_EQ(flatbuffers::Verify(*schema, *t_obj, fbb.GetBufferPointer(),
                              fbb.GetSize()),
          false);
}

}  // namespace

void ReflectionEntrySecurityTest() {
  EntryLengthValidationTest();
  SymbolTableOwnershipTest();
  OutOfRangeBaseTypeTest();
  NoneFieldVerifyTest();
  TEST_OUTPUT_LINE("ReflectionEntrySecurityTest: PASSED");
}

}  // namespace tests
}  // namespace flatbuffers
