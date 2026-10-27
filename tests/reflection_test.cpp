#include "reflection_test.h"

#include "flatbuffers/idl.h"
#include "flatbuffers/minireflect.h"
#include "flatbuffers/reflection.h"
#include "flatbuffers/reflection_generated.h"
#include "flatbuffers/verifier.h"
#include "monster_test.h"
#include "monster_test_generated.h"
#include "test_assert.h"
#include "tests/arrays_test_generated.h"

namespace flatbuffers {
namespace tests {

using namespace MyGame::Example;

void ReflectionTest(const std::string& tests_data_path, uint8_t* flatbuf,
                    size_t length) {
  // Load a binary schema.
  std::string bfbsfile;
  TEST_EQ(flatbuffers::LoadFile((tests_data_path + "monster_test.bfbs").c_str(),
                                true, &bfbsfile),
          true);

  // Verify it, just in case:
  flatbuffers::Verifier verifier(
      reinterpret_cast<const uint8_t*>(bfbsfile.c_str()), bfbsfile.length());
  TEST_EQ(reflection::VerifySchemaBuffer(verifier), true);

  // Make sure the schema is what we expect it to be.
  auto& schema = *reflection::GetSchema(bfbsfile.c_str());
  auto root_table = schema.root_table();

  // Check the declaration files.
  TEST_EQ_STR(root_table->name()->c_str(), "MyGame.Example.Monster");
  TEST_EQ_STR(root_table->declaration_file()->c_str(), "//monster_test.fbs");
  TEST_EQ_STR(
      schema.objects()->LookupByKey("TableA")->declaration_file()->c_str(),
      "//include_test/include_test1.fbs");
  TEST_EQ_STR(schema.objects()
                  ->LookupByKey("MyGame.OtherNameSpace.Unused")
                  ->declaration_file()
                  ->c_str(),
              "//include_test/sub/include_test2.fbs");
  TEST_EQ_STR(schema.enums()
                  ->LookupByKey("MyGame.OtherNameSpace.FromInclude")
                  ->declaration_file()
                  ->c_str(),
              "//include_test/sub/include_test2.fbs");

  // Check scheam filenames and their includes.
  TEST_EQ(schema.fbs_files()->size(), 3);

  const auto fbs0 = schema.fbs_files()->Get(0);
  TEST_EQ_STR(fbs0->filename()->c_str(), "//include_test/include_test1.fbs");
  const auto fbs0_includes = fbs0->included_filenames();
  TEST_EQ(fbs0_includes->size(), 2);

  // TODO(caspern): Should we force or disallow inclusion of self?
  TEST_EQ_STR(fbs0_includes->Get(0)->c_str(),
              "//include_test/include_test1.fbs");
  TEST_EQ_STR(fbs0_includes->Get(1)->c_str(),
              "//include_test/sub/include_test2.fbs");

  const auto fbs1 = schema.fbs_files()->Get(1);
  TEST_EQ_STR(fbs1->filename()->c_str(),
              "//include_test/sub/include_test2.fbs");
  const auto fbs1_includes = fbs1->included_filenames();
  TEST_EQ(fbs1_includes->size(), 2);
  TEST_EQ_STR(fbs1_includes->Get(0)->c_str(),
              "//include_test/include_test1.fbs");
  TEST_EQ_STR(fbs1_includes->Get(1)->c_str(),
              "//include_test/sub/include_test2.fbs");

  const auto fbs2 = schema.fbs_files()->Get(2);
  TEST_EQ_STR(fbs2->filename()->c_str(), "//monster_test.fbs");
  const auto fbs2_includes = fbs2->included_filenames();
  TEST_EQ(fbs2_includes->size(), 1);
  TEST_EQ_STR(fbs2_includes->Get(0)->c_str(),
              "//include_test/include_test1.fbs");

  // Check Root table fields
  auto fields = root_table->fields();
  auto hp_field_ptr = fields->LookupByKey("hp");
  TEST_NOTNULL(hp_field_ptr);
  auto& hp_field = *hp_field_ptr;
  TEST_EQ_STR(hp_field.name()->c_str(), "hp");
  TEST_EQ(hp_field.id(), 2);
  TEST_EQ(hp_field.type()->base_type(), reflection::Short);

  auto friendly_field_ptr = fields->LookupByKey("friendly");
  TEST_NOTNULL(friendly_field_ptr);
  TEST_NOTNULL(friendly_field_ptr->attributes());
  TEST_NOTNULL(friendly_field_ptr->attributes()->LookupByKey("priority"));

  // Make sure the table index is what we expect it to be.
  auto pos_field_ptr = fields->LookupByKey("pos");
  TEST_NOTNULL(pos_field_ptr);
  TEST_EQ(pos_field_ptr->type()->base_type(), reflection::Obj);
  auto pos_table_ptr = schema.objects()->Get(pos_field_ptr->type()->index());
  TEST_NOTNULL(pos_table_ptr);
  TEST_EQ_STR(pos_table_ptr->name()->c_str(), "MyGame.Example.Vec3");

  // Test nullability of fields: hp is a 0-default scalar, pos is a struct =>
  // optional, and name is a required string => not optional.
  TEST_EQ(hp_field.optional(), false);
  TEST_EQ(pos_field_ptr->optional(), true);
  TEST_EQ(fields->LookupByKey("name")->optional(), false);

  // Now use it to dynamically access a buffer.
  auto& root = *flatbuffers::GetAnyRoot(flatbuf);

  // Verify the buffer first using reflection based verification
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), flatbuf, length),
          true);

  auto hp = flatbuffers::GetFieldI<uint16_t>(root, hp_field);
  TEST_EQ(hp, 80);

  // Rather than needing to know the type, we can also get the value of
  // any field as an int64_t/double/string, regardless of what it actually is.
  auto hp_int64 = flatbuffers::GetAnyFieldI(root, hp_field);
  TEST_EQ(hp_int64, 80);
  auto hp_double = flatbuffers::GetAnyFieldF(root, hp_field);
  TEST_EQ(hp_double, 80.0);
  auto hp_string = flatbuffers::GetAnyFieldS(root, hp_field, &schema);
  TEST_EQ_STR(hp_string.c_str(), "80");

  // Get struct field through reflection
  auto pos_struct = flatbuffers::GetFieldStruct(root, *pos_field_ptr);
  TEST_NOTNULL(pos_struct);
  TEST_EQ(flatbuffers::GetAnyFieldF(*pos_struct,
                                    *pos_table_ptr->fields()->LookupByKey("z")),
          3.0f);

  auto test3_field = pos_table_ptr->fields()->LookupByKey("test3");
  auto test3_struct = flatbuffers::GetFieldStruct(*pos_struct, *test3_field);
  TEST_NOTNULL(test3_struct);
  auto test3_object = schema.objects()->Get(test3_field->type()->index());

  TEST_EQ(flatbuffers::GetAnyFieldF(*test3_struct,
                                    *test3_object->fields()->LookupByKey("a")),
          10);

  // We can also modify it.
  flatbuffers::SetField<uint16_t>(&root, hp_field, 200);
  hp = flatbuffers::GetFieldI<uint16_t>(root, hp_field);
  TEST_EQ(hp, 200);

  // We can also set fields generically:
  flatbuffers::SetAnyFieldI(&root, hp_field, 300);
  hp_int64 = flatbuffers::GetAnyFieldI(root, hp_field);
  TEST_EQ(hp_int64, 300);
  flatbuffers::SetAnyFieldF(&root, hp_field, 300.5);
  hp_int64 = flatbuffers::GetAnyFieldI(root, hp_field);
  TEST_EQ(hp_int64, 300);
  flatbuffers::SetAnyFieldS(&root, hp_field, "300");
  hp_int64 = flatbuffers::GetAnyFieldI(root, hp_field);
  TEST_EQ(hp_int64, 300);

  // Test buffer is valid after the modifications
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), flatbuf, length),
          true);

  // Reset it, for further tests.
  flatbuffers::SetField<uint16_t>(&root, hp_field, 80);

  // More advanced functionality: changing the size of items in-line!
  // First we put the FlatBuffer inside an std::vector.
  std::vector<uint8_t> resizingbuf(flatbuf, flatbuf + length);
  // Find the field we want to modify.
  auto& name_field = *fields->LookupByKey("name");
  // Get the root.
  // This time we wrap the result from GetAnyRoot in a smartpointer that
  // will keep rroot valid as resizingbuf resizes.
  auto rroot = flatbuffers::piv(flatbuffers::GetAnyRoot(resizingbuf.data()),
                                resizingbuf);
  SetString(schema, "totally new string", GetFieldS(**rroot, name_field),
            &resizingbuf);
  // Here resizingbuf has changed, but rroot is still valid.
  TEST_EQ_STR(GetFieldS(**rroot, name_field)->c_str(), "totally new string");
  // Now lets extend a vector by 100 elements (10 -> 110).
  auto& inventory_field = *fields->LookupByKey("inventory");
  auto rinventory = flatbuffers::piv(
      flatbuffers::GetFieldV<uint8_t>(**rroot, inventory_field), resizingbuf);
  flatbuffers::ResizeVector<uint8_t>(schema, 110, 50, *rinventory,
                                     &resizingbuf);
  // rinventory still valid, so lets read from it.
  TEST_EQ(rinventory->Get(10), 50);

  // For reflection uses not covered already, there is a more powerful way:
  // we can simply generate whatever object we want to add/modify in a
  // FlatBuffer of its own, then add that to an existing FlatBuffer:
  // As an example, let's add a string to an array of strings.
  // First, find our field:
  auto& testarrayofstring_field = *fields->LookupByKey("testarrayofstring");
  // Find the vector value:
  auto rtestarrayofstring = flatbuffers::piv(
      flatbuffers::GetFieldV<flatbuffers::Offset<flatbuffers::String>>(
          **rroot, testarrayofstring_field),
      resizingbuf);
  // It's a vector of 2 strings, to which we add one more, initialized to
  // offset 0.
  flatbuffers::ResizeVector<flatbuffers::Offset<flatbuffers::String>>(
      schema, 3, 0, *rtestarrayofstring, &resizingbuf);
  // Here we just create a buffer that contans a single string, but this
  // could also be any complex set of tables and other values.
  flatbuffers::FlatBufferBuilder stringfbb;
  stringfbb.Finish(stringfbb.CreateString("hank"));
  // Add the contents of it to our existing FlatBuffer.
  // We do this last, so the pointer doesn't get invalidated (since it is
  // at the end of the buffer):
  auto string_ptr = flatbuffers::AddFlatBuffer(
      resizingbuf, stringfbb.GetBufferPointer(), stringfbb.GetSize());
  // Finally, set the new value in the vector.
  rtestarrayofstring->MutateOffset(2, string_ptr);
  TEST_EQ_STR(rtestarrayofstring->Get(0)->c_str(), "bob");
  TEST_EQ_STR(rtestarrayofstring->Get(2)->c_str(), "hank");
  // Test integrity of all resize operations above.
  flatbuffers::Verifier resize_verifier(
      reinterpret_cast<const uint8_t*>(resizingbuf.data()), resizingbuf.size());
  TEST_EQ(VerifyMonsterBuffer(resize_verifier), true);

  // Test buffer is valid using reflection as well
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), resizingbuf.data(),
                              resizingbuf.size()),
          true);

  // As an additional test, also set it on the name field.
  // Note: unlike the name change above, this just overwrites the offset,
  // rather than changing the string in-place.
  SetFieldT(*rroot, name_field, string_ptr);
  TEST_EQ_STR(GetFieldS(**rroot, name_field)->c_str(), "hank");

  // Using reflection, rather than mutating binary FlatBuffers, we can also copy
  // tables and other things out of other FlatBuffers into a FlatBufferBuilder,
  // either part or whole.
  flatbuffers::FlatBufferBuilder fbb;
  auto root_offset = flatbuffers::CopyTable(
      fbb, schema, *root_table, *flatbuffers::GetAnyRoot(flatbuf), true);
  fbb.Finish(root_offset, MonsterIdentifier());
  // Test that it was copied correctly:
  AccessFlatBufferTest(fbb.GetBufferPointer(), fbb.GetSize());

  // Test buffer is valid using reflection as well
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(),
                              fbb.GetBufferPointer(), fbb.GetSize()),
          true);
}

// CopyTable must align vectors of structs to the struct's alignment, as it
// already does for struct fields, not to the size of an offset.
void CopyTableStructVectorAlignmentTest() {
  const char* schema_src =
      "struct S8 { a: double; }"
      "table T { v: [S8]; w: [ubyte]; }"
      "root_type T;";
  flatbuffers::Parser schema_parser;
  TEST_EQ(schema_parser.Parse(schema_src), true);
  schema_parser.Serialize();
  const auto& schema =
      *reflection::GetSchema(schema_parser.builder_.GetBufferPointer());
  const auto& v_field = *schema.root_table()->fields()->LookupByKey("v");

  // CopyTable serializes fields in name order, back to front, so w ends up in
  // front of v: varying its length moves v through every 4-byte slot.
  for (int pad = 0; pad < 8; pad++) {
    std::string json = "{ v: [{ a: 1.5 }], w: [";
    for (int i = 0; i < pad; i++) json += i ? ",1" : "1";
    json += "] }";
    flatbuffers::Parser data_parser;
    TEST_EQ(data_parser.Parse(schema_src), true);
    TEST_EQ(data_parser.Parse(json.c_str()), true);

    flatbuffers::FlatBufferBuilder fbb;
    fbb.Finish(flatbuffers::CopyTable(
        fbb, schema, *schema.root_table(),
        *flatbuffers::GetAnyRoot(data_parser.builder_.GetBufferPointer())));
    const auto buf = fbb.GetBufferPointer();
    const auto v =
        flatbuffers::GetFieldAnyV(*flatbuffers::GetAnyRoot(buf), v_field);
    TEST_NOTNULL(v);
    const auto elems = reinterpret_cast<uintptr_t>(v->Data()) -
                       reinterpret_cast<uintptr_t>(buf);
    TEST_EQ(elems % 8, 0u);
  }
}

// Resizing a buffer in place must keep data after the insertion point aligned
// for structs with force_align larger than any scalar.
void ResizeForceAlignTest() {
  const char* schema_src =
      "struct F (force_align: 16) { a: int; }"
      "table RZ { s: string; v: [F]; }"
      "root_type RZ;";
  flatbuffers::Parser schema_parser;
  TEST_EQ(schema_parser.Parse(schema_src), true);
  schema_parser.Serialize();
  const auto& schema =
      *reflection::GetSchema(schema_parser.builder_.GetBufferPointer());
  const auto& s_field = *schema.root_table()->fields()->LookupByKey("s");
  const auto& v_field = *schema.root_table()->fields()->LookupByKey("v");

  // v is serialized first, so it ends up after s and moves when s grows.
  flatbuffers::Parser data_parser;
  TEST_EQ(data_parser.Parse(schema_src), true);
  TEST_EQ(data_parser.Parse("{ v: [{ a: 1 }], s: \"ab\" }"), true);
  const auto data = data_parser.builder_.GetBufferPointer();
  const auto v0 =
      flatbuffers::GetFieldAnyV(*flatbuffers::GetAnyRoot(data), v_field);
  TEST_EQ((reinterpret_cast<uintptr_t>(v0->Data()) -
           reinterpret_cast<uintptr_t>(data)) %
              16,
          0u);

  for (size_t grow = 1; grow <= 16; grow++) {
    std::vector<uint8_t> buf(data, data + data_parser.builder_.GetSize());
    auto root = flatbuffers::GetAnyRoot(buf.data());
    flatbuffers::SetString(schema, std::string(2 + grow, 'x'),
                           flatbuffers::GetFieldS(*root, s_field), &buf);
    root = flatbuffers::GetAnyRoot(buf.data());
    const auto v = flatbuffers::GetFieldAnyV(*root, v_field);
    TEST_NOTNULL(v);
    TEST_EQ(ReadScalar<int32_t>(v->Data()), 1);
    const auto elems = reinterpret_cast<uintptr_t>(v->Data()) -
                       reinterpret_cast<uintptr_t>(buf.data());
    TEST_EQ(elems % 16, 0u);
  }
}

namespace {

// A binary schema plus a buffer parsed from JSON with it.
struct ParsedBuffer {
  std::vector<uint8_t> bfbs;
  std::vector<uint8_t> data;

  const reflection::Schema& schema() const {
    return *reflection::GetSchema(bfbs.data());
  }
  const reflection::Object& object(const char* name) const {
    return *schema().objects()->LookupByKey(name);
  }
  const reflection::Field& field(const char* object_name,
                                 const char* name) const {
    return *object(object_name).fields()->LookupByKey(name);
  }
  bool Verify(const std::vector<uint8_t>& buf) const {
    return flatbuffers::Verify(schema(), *schema().root_table(), buf.data(),
                               buf.size());
  }
};

flatbuffers::IDLOptions ReflectionTestOptions(bool builtins) {
  flatbuffers::IDLOptions opts;
  // Unions with struct and string members need C++ as the target language.
  opts.lang_to_generate = flatbuffers::IDLOptions::kCpp;
  // Keeps attributes such as force_align in the binary schema.
  opts.binary_schema_builtins = builtins;
  return opts;
}

ParsedBuffer Parse(const char* schema_src, const std::string& json,
                   bool builtins = false) {
  ParsedBuffer parsed;
  flatbuffers::Parser schema_parser(ReflectionTestOptions(builtins));
  TEST_EQ(schema_parser.Parse(schema_src), true);
  schema_parser.Serialize();
  parsed.bfbs.assign(schema_parser.builder_.GetBufferPointer(),
                     schema_parser.builder_.GetBufferPointer() +
                         schema_parser.builder_.GetSize());
  flatbuffers::Parser data_parser(ReflectionTestOptions(builtins));
  TEST_EQ(data_parser.Parse(schema_src), true);
  TEST_EQ(data_parser.Parse(json.c_str()), true);
  parsed.data.assign(
      data_parser.builder_.GetBufferPointer(),
      data_parser.builder_.GetBufferPointer() + data_parser.builder_.GetSize());
  return parsed;
}

std::string ToJson(const char* schema_src, const uint8_t* buf) {
  flatbuffers::Parser parser(ReflectionTestOptions(false));
  TEST_EQ(parser.Parse(schema_src), true);
  std::string json;
  TEST_NULL(flatbuffers::GenText(parser, buf, &json));
  return json;
}

// Copies "data" with CopyTable, using "schema" to interpret it.
std::vector<uint8_t> Copy(const reflection::Schema& schema,
                          const std::vector<uint8_t>& data) {
  flatbuffers::FlatBufferBuilder fbb;
  fbb.Finish(flatbuffers::CopyTable(fbb, schema, *schema.root_table(),
                                    *flatbuffers::GetAnyRoot(data.data())));
  return std::vector<uint8_t>(fbb.GetBufferPointer(),
                              fbb.GetBufferPointer() + fbb.GetSize());
}

std::vector<uint8_t> Copy(const ParsedBuffer& parsed) {
  return Copy(parsed.schema(), parsed.data);
}

size_t OffsetIn(const std::vector<uint8_t>& buf, const void* p) {
  return static_cast<size_t>(reinterpret_cast<uintptr_t>(p) -
                             reinterpret_cast<uintptr_t>(buf.data()));
}

}  // namespace

// Shrinking a string must only remove bytes of the old string, not the data
// in front of it (here the end of the inv vector).
void SetStringShrinkTest() {
  const char* schema_src =
      "table T { inv: [ubyte]; name: string; } root_type T;";
  // name is serialized first, so inv ends up directly in front of it.
  const auto parsed =
      Parse(schema_src, "{ name: \"MyMonster\", inv: [0, 1, 2, 3, 4] }");
  const auto& name_field = parsed.field("T", "name");
  const auto& inv_field = parsed.field("T", "inv");

  const char* sequences[][3] = {
      {"", nullptr, nullptr},
      {"My", nullptr, nullptr},
      {"MyMonsterMyMonsterMyMonsterMyMonster", "", nullptr},
      {"MyMonsterMyMonsterMyMonsterMyMonster", "abc", "q"},
  };
  for (auto& sequence : sequences) {
    auto buf = parsed.data;
    for (auto name : sequence) {
      if (!name) break;
      flatbuffers::SetString(
          parsed.schema(), name,
          flatbuffers::GetFieldS(*flatbuffers::GetAnyRoot(buf.data()),
                                 name_field),
          &buf);
      TEST_EQ(parsed.Verify(buf), true);
      const auto& root = *flatbuffers::GetAnyRoot(buf.data());
      TEST_EQ_STR(flatbuffers::GetFieldS(root, name_field)->c_str(), name);
      const auto inv = flatbuffers::GetFieldV<uint8_t>(root, inv_field);
      TEST_EQ(inv->size(), 5u);
      for (uoffset_t i = 0; i < inv->size(); i++) TEST_EQ(inv->Get(i), i);
    }
  }
}

// Resizing a vector that ends exactly where another table's vtable begins
// must not adjust that table's vtable offset: both move together.
void ResizeVectorBeforeVTableTest() {
  const char* schema_src =
      "struct P { a: short; b: byte; }"
      "table Inner { name: string; x: int; }"
      "table T { inner: Inner; v: [P]; }"
      "root_type T;";
  bool boundary_seen = false;
  for (int n = 1; n <= 4; n++) {
    // inner is serialized first, so v ends up in front of its vtable.
    std::string json = "{ inner: { name: \"Fred\", x: 7 }, v: [";
    for (int i = 0; i < n; i++)
      json += i ? ", { a: 1, b: 2 }" : "{ a: 1, b: 2 }";
    json += "] }";
    const auto parsed = Parse(schema_src, json);
    const auto& inner_field = parsed.field("T", "inner");
    const auto& v_field = parsed.field("T", "v");
    const auto& name_field = parsed.field("Inner", "name");
    const auto& x_field = parsed.field("Inner", "x");
    const auto& p_def = parsed.object("P");

    const auto& root = *flatbuffers::GetAnyRoot(parsed.data.data());
    const auto vec = flatbuffers::GetFieldAnyV(root, v_field);
    const auto vec_end =
        vec->Data() + vec->size() * static_cast<size_t>(p_def.bytesize());
    boundary_seen |=
        vec_end == flatbuffers::GetFieldT(root, inner_field)->GetVTable();

    for (uoffset_t newsize = 0; newsize <= 6; newsize++) {
      auto buf = parsed.data;
      flatbuffers::ResizeAnyVector(
          parsed.schema(), newsize,
          flatbuffers::GetFieldAnyV(*flatbuffers::GetAnyRoot(buf.data()),
                                    v_field),
          static_cast<uoffset_t>(n), static_cast<uoffset_t>(p_def.bytesize()),
          &buf);
      TEST_EQ(parsed.Verify(buf), true);
      const auto& inner = *flatbuffers::GetFieldT(
          *flatbuffers::GetAnyRoot(buf.data()), inner_field);
      TEST_EQ_STR(flatbuffers::GetFieldS(inner, name_field)->c_str(), "Fred");
      TEST_EQ(flatbuffers::GetFieldI<int32_t>(inner, x_field), 7);
    }
  }
  // Make sure the layout above actually exercises the boundary case.
  TEST_EQ(boundary_seen, true);
}

// Resizing must adjust the offsets stored in vectors of unions.
void ResizeUnionVectorTest() {
  const char* schema_src =
      "table S { s: string; } union U { S } table T { uv: [U]; }"
      "root_type T;";
  const auto parsed =
      Parse(schema_src,
            "{ uv_type: [S, S], uv: [{ s: \"first\" }, { s: \"second\" }] }");
  const auto& uv_field = parsed.field("T", "uv");
  const auto& s_field = parsed.field("S", "s");
  auto elements = [&](const std::vector<uint8_t>& buf) {
    return flatbuffers::GetAnyRoot(buf.data())
        ->GetPointer<const Vector<Offset<Table>>*>(uv_field.offset());
  };

  auto buf = parsed.data;
  flatbuffers::SetString(
      parsed.schema(), "second-longer",
      flatbuffers::GetFieldS(*elements(buf)->Get(1), s_field), &buf);
  TEST_EQ(parsed.Verify(buf), true);
  TEST_EQ_STR(flatbuffers::GetFieldS(*elements(buf)->Get(0), s_field)->c_str(),
              "first");
  TEST_EQ_STR(flatbuffers::GetFieldS(*elements(buf)->Get(1), s_field)->c_str(),
              "second-longer");
}

// Unions can hold tables, structs and strings; resizing and copying must
// handle each kind instead of treating every member as a table.
void UnionMemberKindsTest() {
  const char* schema_src =
      "struct P { x: int; y: int; }"
      "table X { a: string; }"
      "union U { P, X, str: string }"
      "table T { s: string; u: U; }"
      "root_type T;";
  // u is serialized first, so it ends up after s and moves when s grows.
  const char* jsons[] = {
      "{ u_type: P, u: { x: 1000000000, y: 3 }, s: \"ab\" }",
      "{ u_type: X, u: { a: \"table member\" }, s: \"ab\" }",
      "{ u_type: str, u: \"string member\", s: \"ab\" }",
  };
  for (auto json : jsons) {
    const auto parsed = Parse(schema_src, json);
    const auto& s_field = parsed.field("T", "s");
    const auto expected = ToJson(schema_src, parsed.data.data());

    const auto copy = Copy(parsed);
    TEST_EQ(parsed.Verify(copy), true);
    TEST_EQ_STR(ToJson(schema_src, copy.data()).c_str(), expected.c_str());

    const std::string longer = "abcdefghijklmnopq";
    auto buf = parsed.data;
    flatbuffers::SetString(
        parsed.schema(), longer,
        flatbuffers::GetFieldS(*flatbuffers::GetAnyRoot(buf.data()), s_field),
        &buf);
    TEST_EQ(parsed.Verify(buf), true);
    auto resized = ToJson(schema_src, buf.data());
    const auto at = resized.find(longer);
    TEST_EQ(at != std::string::npos, true);
    resized.replace(at, longer.size(), "ab");
    TEST_EQ_STR(resized.c_str(), expected.c_str());
  }
}

// CopyTable must copy vectors of unions element by element, not as raw
// offsets into the source buffer.
void CopyTableUnionVectorTest() {
  const char* schema_src =
      "table X { a: int; } table Y { b: string; } union U { X, Y }"
      "table R { u: [U]; } root_type R;";
  const auto parsed =
      Parse(schema_src, "{ u_type: [X, Y], u: [{ a: 7 }, { b: \"hi\" }] }");
  const auto copy = Copy(parsed);
  TEST_EQ(parsed.Verify(copy), true);
  TEST_EQ_STR(ToJson(schema_src, copy.data()).c_str(),
              ToJson(schema_src, parsed.data.data()).c_str());
}

// Copying newer data with an older schema can meet union members the older
// schema doesn't know. CopyTable can't copy their values, so it must not keep
// their tags either: they become NONE.
void CopyTableUnknownUnionMembersTest() {
  const char* v1 =
      "table X { a: int; } table Y { b: string; } union U { X, Y }"
      "table R { u: U; uv: [U]; } root_type R;";
  const char* v2 =
      "table X { a: int; } table Y { b: string; }"
      "table Z { c: string; d: int; } union U { X, Y, Z, s: string }"
      "table R { u: U; uv: [U]; } root_type R;";
  const auto newer = Parse(v2,
                           "{ u_type: Z, u: { c: \"q\", d: 4 },"
                           "  uv_type: [X, Z, s],"
                           "  uv: [{ a: 1 }, { c: \"q\", d: 4 }, \"str\"] }");
  const auto older = Parse(v1, "{}");

  const auto copy = Copy(older.schema(), newer.data);
  TEST_EQ(older.Verify(copy), true);
  TEST_EQ(newer.Verify(copy), true);
  const auto& root = *flatbuffers::GetAnyRoot(copy.data());
  TEST_EQ(flatbuffers::GetFieldI<uint8_t>(root, newer.field("R", "u_type")), 0);
  const auto types =
      flatbuffers::GetFieldV<uint8_t>(root, newer.field("R", "uv_type"));
  TEST_NOTNULL(types);
  TEST_EQ(types->size(), 3u);
  TEST_EQ(types->Get(0), 1);  // X
  TEST_EQ(types->Get(1), 0);  // Z: unknown to v1, so NONE.
  TEST_EQ(types->Get(2), 0);  // s: unknown to v1, so NONE.
  const auto uv = root.GetPointer<const Vector<Offset<Table>>*>(
      newer.field("R", "uv").offset());
  TEST_EQ(flatbuffers::GetFieldI<int32_t>(*uv->Get(0), newer.field("X", "a")),
          1);
}

// Vectors with force_align, nested FlatBuffers and FlexBuffers are aligned
// beyond their element size when built; resizing must keep that, and so must
// CopyTable, except for FlexBuffers, which the generated code doesn't align
// either.
void AlignedVectorAttributesTest() {
  const char* schema_src =
      "table N { d: double; }"
      "table X { a: int; } union U { X }"
      "table T { v: [ubyte] (force_align: 16);"
      "          ss: [string] (force_align: 16);"
      "          xs: [X] (force_align: 16);"
      "          uv: [U] (force_align: 16);"
      "          n: [ubyte] (nested_flatbuffer: \"N\");"
      "          f: [ubyte] (flexbuffer);"
      "          s: string; zz: [ubyte]; }"
      "root_type T;";
  for (int pad = 0; pad < 16; pad++) {
    // Everything but zz is serialized before s, so it moves when s grows.
    // CopyTable serializes in name order, so zz ends up in front of the rest.
    std::string json =
        "{ v: [1, 2, 3], ss: [\"a\"], xs: [{ a: 1 }], uv_type: [X],"
        "  uv: [{ a: 2 }], n: { d: 1.5 }, f: { a: 1.0 }, s: \"ab\", zz: [";
    for (int i = 0; i < pad; i++) json += i ? ",1" : "1";
    json += "] }";
    const auto parsed = Parse(schema_src, json, true);
    const auto check = [&](const std::vector<uint8_t>& buf, bool flex) {
      TEST_EQ(parsed.Verify(buf), true);
      const auto& root = *flatbuffers::GetAnyRoot(buf.data());
      const auto data_offset = [&](const char* name) {
        return OffsetIn(
            buf,
            flatbuffers::GetFieldAnyV(root, parsed.field("T", name))->Data());
      };
      TEST_EQ(data_offset("v") % 16, 0u);
      TEST_EQ(data_offset("ss") % 16, 0u);
      TEST_EQ(data_offset("xs") % 16, 0u);
      TEST_EQ(data_offset("uv") % 16, 0u);
      TEST_EQ(data_offset("n") % 8, 0u);
      if (flex) TEST_EQ(data_offset("f") % 8, 0u);
    };
    check(parsed.data, true);
    check(Copy(parsed), false);
    for (size_t grow = 1; grow <= 16; grow++) {
      auto buf = parsed.data;
      flatbuffers::SetString(
          parsed.schema(), std::string(2 + grow, 'x'),
          flatbuffers::GetFieldS(*flatbuffers::GetAnyRoot(buf.data()),
                                 parsed.field("T", "s")),
          &buf);
      check(buf, true);
    }
  }
}

// The first thing CopyTable copies ends up at the end of the new buffer, so
// aligning it beyond what its data needs leaves padding after everything the
// copy refers to, and SizeVerifier then computes too small a size.
void CopyTableTrailingPaddingTest() {
  struct {
    const char* schema;
    const char* json;
    size_t align;  // What the data in "a" needs.
  } cases[] = {
      // A nested buffer that only needs 4-byte alignment.
      {"table N { i: int; } table T { a: [ubyte] (nested_flatbuffer: \"N\"); }"
       "root_type T;",
       "{ a: { i: 7 } }", 4},
      {"table T { a: [ubyte] (flexbuffer); } root_type T;", "{ a: 1234 }", 1},
  };
  for (auto& c : cases) {
    const auto parsed = Parse(c.schema, c.json, true);
    const auto copy = Copy(parsed);
    TEST_EQ(parsed.Verify(copy), true);
    const auto a = flatbuffers::GetFieldAnyV(
        *flatbuffers::GetAnyRoot(copy.data()), parsed.field("T", "a"));
    TEST_NOTNULL(a);
    const auto start = OffsetIn(copy, a->Data());
    TEST_EQ(start % c.align, 0u);
    // What SizeVerifier computes: the end of the data, rounded up to the size
    // of an offset.
    const auto end = start + a->size();
    TEST_EQ((end + sizeof(uoffset_t) - 1) & ~(sizeof(uoffset_t) - 1),
            copy.size());
  }
}

// AddFlatBuffer must place the added buffer so that data aligned within it
// (here a force_align: 16 struct) stays aligned.
void AddFlatBufferAlignmentTest() {
  const auto parsed = Parse(
      "struct F (force_align: 16) { a: int; } table T { f: F; }"
      "root_type T;",
      "{ f: { a: 7 } }");
  const auto& f_field = parsed.field("T", "f");
  for (size_t host_size = 0; host_size < 32; host_size++) {
    std::vector<uint8_t> host(host_size, 0);
    const auto table =
        reinterpret_cast<const Table*>(flatbuffers::AddFlatBuffer(
            host, parsed.data.data(), parsed.data.size()));
    TEST_NOTNULL(table);
    const auto f = table->GetStruct<const uint8_t*>(f_field.offset());
    TEST_EQ(ReadScalar<int32_t>(f), 7);
    TEST_EQ(OffsetIn(host, f) % 16, 0u);
  }
}

// Test that ForAllFields with reverse=true iterates fields in descending
// ID order. This exercises a fix for an operator precedence bug where the
// expression `size() - i + 1` was evaluated as `(size() - i) + 1` instead
// of the correct `size() - (i + 1)`, causing an out-of-bounds read.
void ForAllFieldsReverseTest(const std::string& tests_data_path) {
  // Load the binary schema.
  std::string bfbsfile;
  TEST_EQ(flatbuffers::LoadFile((tests_data_path + "monster_test.bfbs").c_str(),
                                true, &bfbsfile),
          true);

  // Verify the schema.
  flatbuffers::Verifier verifier(
      reinterpret_cast<const uint8_t*>(bfbsfile.c_str()), bfbsfile.length());
  TEST_EQ(reflection::VerifySchemaBuffer(verifier), true);

  auto& schema = *reflection::GetSchema(bfbsfile.c_str());

  // Use the Stat table which has 3 fields with sequential IDs:
  //   id:string   (id: 0)
  //   val:long     (id: 1)
  //   count:ushort (id: 2)
  auto stat_object = schema.objects()->LookupByKey("MyGame.Example.Stat");
  TEST_NOTNULL(stat_object);
  TEST_EQ(stat_object->fields()->size(), 3u);

  // Test forward iteration: fields should come in ascending ID order (0,1,2).
  {
    std::vector<uint16_t> forward_ids;
    flatbuffers::ForAllFields(
        stat_object, /*reverse=*/false,
        [&forward_ids](const reflection::Field* field) {
          forward_ids.push_back(field->id());
        });
    TEST_EQ(forward_ids.size(), 3u);
    TEST_EQ(forward_ids[0], 0);  // id
    TEST_EQ(forward_ids[1], 1);  // val
    TEST_EQ(forward_ids[2], 2);  // count
  }

  // Test reverse iteration: fields should come in descending ID order (2,1,0).
  // With the old buggy code `size() - i + 1`, at i=0 this would compute
  // index 3 - 0 + 1 = 4, which is out of bounds for a size-3 vector.
  {
    std::vector<uint16_t> reverse_ids;
    flatbuffers::ForAllFields(
        stat_object, /*reverse=*/true,
        [&reverse_ids](const reflection::Field* field) {
          reverse_ids.push_back(field->id());
        });
    TEST_EQ(reverse_ids.size(), 3u);
    TEST_EQ(reverse_ids[0], 2);  // count (highest ID first)
    TEST_EQ(reverse_ids[1], 1);  // val
    TEST_EQ(reverse_ids[2], 0);  // id (lowest ID last)
  }

  // Also test with the root Monster table which has many fields and
  // non-sequential definition order vs. ID order (e.g., pos=id:0, hp=id:2,
  // mana=id:1). This ensures the ID-to-index mapping works correctly.
  auto root_table = schema.root_table();
  TEST_NOTNULL(root_table);
  {
    std::vector<uint16_t> forward_ids;
    flatbuffers::ForAllFields(
        root_table, /*reverse=*/false,
        [&forward_ids](const reflection::Field* field) {
          forward_ids.push_back(field->id());
        });
    // Verify ascending order.
    for (size_t i = 1; i < forward_ids.size(); ++i) {
      TEST_ASSERT(forward_ids[i - 1] < forward_ids[i]);
    }
  }
  {
    std::vector<uint16_t> reverse_ids;
    flatbuffers::ForAllFields(
        root_table, /*reverse=*/true,
        [&reverse_ids](const reflection::Field* field) {
          reverse_ids.push_back(field->id());
        });
    // Verify descending order.
    for (size_t i = 1; i < reverse_ids.size(); ++i) {
      TEST_ASSERT(reverse_ids[i - 1] > reverse_ids[i]);
    }
  }
}

void MiniReflectFlatBuffersTest(uint8_t* flatbuf) {
  auto s =
      flatbuffers::FlatBufferToString(flatbuf, Monster::MiniReflectTypeTable());
  TEST_EQ_STR(
      s.c_str(),
      "{ "
      "pos: { x: 1.0, y: 2.0, z: 3.0, test1: 0.0, test2: Red, test3: "
      "{ a: 10, b: 20 } }, "
      "hp: 80, "
      "name: \"MyMonster\", "
      "inventory: [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ], "
      "test_type: Monster, "
      "test: { name: \"Fred\" }, "
      "test4: [ { a: 10, b: 20 }, { a: 30, b: 40 } ], "
      "testarrayofstring: [ \"bob\", \"fred\", \"bob\", \"fred\" ], "
      "testarrayoftables: [ { hp: 1000, name: \"Barney\" }, { name: \"Fred\" "
      "}, "
      "{ name: \"Wilma\" } ], "
      // TODO(wvo): should really print this nested buffer correctly.
      "testnestedflatbuffer: [ 124, 0, 0, 0, 77, 79, 78, 83, 0, 0, 114, 0, 16, "
      "0, 0, 0, 4, 0, 6, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "
      "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "
      "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "
      "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "
      "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 114, 0, 0, 0, 0, 0, 0, 0, "
      "8, 0, 0, 0, 0, 0, 192, 127, 13, 0, 0, 0, 78, 101, 115, 116, 101, 100, "
      "77, 111, 110, 115, 116, 101, 114, 0, 0, 0 ], "
      "testarrayofstring2: [ \"jane\", \"mary\" ], "
      "testarrayofsortedstruct: [ { id: 0, distance: 0 }, "
      "{ id: 2, distance: 20 }, { id: 3, distance: 30 }, "
      "{ id: 4, distance: 40 } ], "
      "flex: [ 210, 4, 5, 2 ], "
      "test5: [ { a: 10, b: 20 }, { a: 30, b: 40 } ], "
      "vector_of_enums: [ Blue, Green ], "
      "scalar_key_sorted_tables: [ { id: \"miss\" } ], "
      "nan_default: nan "
      "}");

  Test test(16, 32);
  Vec3 vec(1, 2, 3, 1.5, Color_Red, test);
  flatbuffers::FlatBufferBuilder vec_builder;
  vec_builder.Finish(vec_builder.CreateStruct(vec));
  auto vec_buffer = vec_builder.Release();
  auto vec_str = flatbuffers::FlatBufferToString(vec_buffer.data(),
                                                 Vec3::MiniReflectTypeTable());
  TEST_EQ_STR(vec_str.c_str(),
              "{ x: 1.0, y: 2.0, z: 3.0, test1: 1.5, test2: Red, test3: { a: "
              "16, b: 32 } }");
}

void MiniReflectFixedLengthArrayTest() {
  // VS10 does not support typed enums, exclude from tests
#if !defined(_MSC_VER) || _MSC_VER >= 1700
  flatbuffers::FlatBufferBuilder fbb;
  MyGame::Example::ArrayStruct aStruct(2, 12, 1);
  auto aTable = MyGame::Example::CreateArrayTable(fbb, &aStruct);
  fbb.Finish(aTable);

  auto flatbuf = fbb.Release();
  auto s = flatbuffers::FlatBufferToString(
      flatbuf.data(), MyGame::Example::ArrayTableTypeTable());
  TEST_EQ_STR(
      "{ "
      "a: { a: 2.0, "
      "b: [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], "
      "c: 12, "
      "d: [ { a: [ 0, 0 ], b: A, c: [ A, A ], d: [ 0, 0 ] }, "
      "{ a: [ 0, 0 ], b: A, c: [ A, A ], d: [ 0, 0 ] } ], "
      "e: 1, f: [ 0, 0 ] } "
      "}",
      s.c_str());
#endif
}

}  // namespace tests
}  // namespace flatbuffers
