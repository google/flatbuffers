#include "reflection_test.h"

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

// Regression test for a heap out-of-bounds read in the reflection-based
// Verify() path. A malicious/corrupt reflection::Schema can encode a
// Type.index() that does not correspond to any entry in Schema.objects()
// (or Schema.enums()). Previously, VerifyObject/VerifyVector/VerifyUnion
// passed such an index directly to Vector::Get(), whose only bounds check
// is an assert that is compiled out in release (NDEBUG) builds, resulting
// in an out-of-bounds read. This test builds such a schema (with an empty
// `objects` vector and a field whose type.index() is far out of range) and
// confirms that Verify() now fails closed (returns false) instead of
// reading out of bounds.
void ReflectionInvalidObjectIndexTest() {
  // ---- Build a malicious schema: one Object ("Root") with one Field
  // ("f") of base_type Obj whose type.index() is out of range, while
  // `objects` itself is left empty. ----
  flatbuffers::FlatBufferBuilder schema_fbb;

  auto type_off = reflection::CreateType(
      schema_fbb, reflection::Obj, reflection::None, /*index=*/999999);
  auto fname_off = schema_fbb.CreateString("f");
  auto field_off =
      reflection::CreateField(schema_fbb, fname_off, type_off,
                              /*id=*/0, /*offset=*/4);

  std::vector<flatbuffers::Offset<reflection::Field>> fields_vec{field_off};
  auto oname_off = schema_fbb.CreateString("Root");
  auto fields_off =
      schema_fbb.CreateVectorOfSortedTables<reflection::Field>(&fields_vec);
  auto root_obj_off = reflection::CreateObject(
      schema_fbb, oname_off, fields_off, /*is_struct=*/false);

  // `objects` is intentionally empty: any index() access into it is
  // therefore out of range.
  std::vector<flatbuffers::Offset<reflection::Object>> objects_vec;
  auto objects_off =
      schema_fbb.CreateVectorOfSortedTables<reflection::Object>(
          &objects_vec);

  std::vector<flatbuffers::Offset<reflection::Enum>> enums_vec;
  auto enums_off =
      schema_fbb.CreateVectorOfSortedTables<reflection::Enum>(&enums_vec);

  auto schema_off = reflection::CreateSchema(
      schema_fbb, objects_off, enums_off, /*file_ident=*/0, /*file_ext=*/0,
      root_obj_off);
  schema_fbb.Finish(schema_off, reflection::SchemaIdentifier());

  const uint8_t* schema_buf = schema_fbb.GetBufferPointer();
  const size_t schema_size = schema_fbb.GetSize();

  flatbuffers::Verifier schema_verifier(schema_buf, schema_size);
  TEST_EQ(reflection::VerifySchemaBuffer(schema_verifier), true);

  const reflection::Schema* schema =
      flatbuffers::GetRoot<reflection::Schema>(schema_buf);
  TEST_EQ(schema->objects()->size(), 0u);

  const reflection::Object* root = schema->root_table();

  // ---- Build a minimal, well-formed data buffer matching the schema: a
  // table with one field at vtable slot 4 pointing at a valid (empty)
  // child table. ----
  flatbuffers::FlatBufferBuilder data_fbb;
  auto child_start = data_fbb.StartTable();
  flatbuffers::Offset<flatbuffers::Table> child_off(
      data_fbb.EndTable(child_start));

  auto root_start = data_fbb.StartTable();
  data_fbb.AddOffset(4, child_off);
  auto root_end = data_fbb.EndTable(root_start);
  data_fbb.Finish(flatbuffers::Offset<flatbuffers::Table>(root_end));

  // Before the fix, this call reads out of bounds (crashes under ASan /
  // UBSan in release builds; hits a compiled-out assert in debug builds).
  // After the fix, it must fail closed.
  bool result = flatbuffers::Verify(*schema, *root, data_fbb.GetBufferPointer(),
                                    data_fbb.GetSize());
  TEST_EQ(result, false);
}

// Regression test for a heap out-of-bounds read in CopyTable, sharing the
// same root cause as ReflectionInvalidObjectIndexTest above: a malicious/
// corrupt reflection::Schema can encode a Type.index() that does not
// correspond to any entry in Schema.objects(). CopyTable's contract does
// not require the schema to have been verified against the data it's
// paired with here (and even a caller that did verify may have done so
// against a different, benign schema than the one passed to CopyTable), so
// this is independently reachable from the already-known Verify() gap.
// Confirmed via a real ASan SEGV (heap-buffer-overflow read) before the
// fix, at the schema.objects()->Get(fielddef.type()->index()) call in the
// Obj case of CopyTable's first field loop.
void CopyTableInvalidObjectIndexTest() {
  // Same malicious-schema shape as ReflectionInvalidObjectIndexTest: one
  // Object ("Root") with one Field ("f") of base_type Obj whose
  // type.index() is out of range, while `objects` itself is empty.
  flatbuffers::FlatBufferBuilder schema_fbb;

  auto type_off = reflection::CreateType(
      schema_fbb, reflection::Obj, reflection::None, /*index=*/999999);
  auto fname_off = schema_fbb.CreateString("f");
  auto field_off =
      reflection::CreateField(schema_fbb, fname_off, type_off,
                              /*id=*/0, /*offset=*/4);

  std::vector<flatbuffers::Offset<reflection::Field>> fields_vec{field_off};
  auto oname_off = schema_fbb.CreateString("Root");
  auto fields_off =
      schema_fbb.CreateVectorOfSortedTables<reflection::Field>(&fields_vec);
  auto root_obj_off = reflection::CreateObject(
      schema_fbb, oname_off, fields_off, /*is_struct=*/false);

  std::vector<flatbuffers::Offset<reflection::Object>> objects_vec;
  auto objects_off =
      schema_fbb.CreateVectorOfSortedTables<reflection::Object>(
          &objects_vec);
  std::vector<flatbuffers::Offset<reflection::Enum>> enums_vec;
  auto enums_off =
      schema_fbb.CreateVectorOfSortedTables<reflection::Enum>(&enums_vec);

  auto schema_off = reflection::CreateSchema(
      schema_fbb, objects_off, enums_off, /*file_ident=*/0, /*file_ext=*/0,
      root_obj_off);
  schema_fbb.Finish(schema_off, reflection::SchemaIdentifier());

  const reflection::Schema* schema = flatbuffers::GetRoot<reflection::Schema>(
      schema_fbb.GetBufferPointer());
  TEST_EQ(schema->objects()->size(), 0u);
  const reflection::Object* root = schema->root_table();

  // A minimal, well-formed data buffer matching the schema: a table with
  // one field at vtable slot 4 pointing at a valid (empty) child table.
  flatbuffers::FlatBufferBuilder data_fbb;
  auto child_start = data_fbb.StartTable();
  flatbuffers::Offset<flatbuffers::Table> child_off(
      data_fbb.EndTable(child_start));
  auto root_start = data_fbb.StartTable();
  data_fbb.AddOffset(4, child_off);
  auto root_end = data_fbb.EndTable(root_start);
  data_fbb.Finish(flatbuffers::Offset<flatbuffers::Table>(root_end));

  const flatbuffers::Table* table = flatbuffers::GetRoot<flatbuffers::Table>(
      data_fbb.GetBufferPointer());

  // Before the fix, this reads out of bounds (confirmed via a real ASan
  // SEGV in a release/NDEBUG build). After the fix, the malformed field is
  // skipped and CopyTable returns a valid (if incomplete) table rather
  // than crashing.
  flatbuffers::FlatBufferBuilder out_fbb;
  auto result =
      flatbuffers::CopyTable(out_fbb, *schema, *root, *table, false);
  TEST_EQ(result.o != 0, true);
}

// Regression test for a heap out-of-bounds read in
// ResizeContext::ResizeTable, reachable via the public SetString/
// ResizeAnyVector mutation API. Same root cause as
// CopyTableInvalidObjectIndexTest above, in a different function:
// ResizeTable walks the ENTIRE object graph from the root table (not just
// the field being resized) to adjust offsets for the insertion point, and
// reached the same unchecked schema.objects()->Get(fielddef.type()->
// index()) pattern while doing so. Confirmed via a real ASan SEGV inside
// ResizeContext::ResizeTable before the fix.
void ResizeTableInvalidObjectIndexTest() {
  // Malicious schema with TWO fields: "f" (Obj, out-of-range index, to be
  // reached by the field walk) and "s" (String, to actually trigger a
  // resize via SetString).
  flatbuffers::FlatBufferBuilder schema_fbb;

  auto obj_type_off = reflection::CreateType(
      schema_fbb, reflection::Obj, reflection::None, /*index=*/999999);
  auto fname_off = schema_fbb.CreateString("f");
  auto field_f_off = reflection::CreateField(schema_fbb, fname_off,
                                             obj_type_off,
                                             /*id=*/0, /*offset=*/4);

  auto str_type_off =
      reflection::CreateType(schema_fbb, reflection::String);
  auto sname_off = schema_fbb.CreateString("s");
  auto field_s_off = reflection::CreateField(schema_fbb, sname_off,
                                             str_type_off,
                                             /*id=*/1, /*offset=*/6);

  std::vector<flatbuffers::Offset<reflection::Field>> fields_vec{field_f_off,
                                                                  field_s_off};
  auto oname_off = schema_fbb.CreateString("Root");
  auto fields_off =
      schema_fbb.CreateVectorOfSortedTables<reflection::Field>(&fields_vec);
  auto root_obj_off = reflection::CreateObject(
      schema_fbb, oname_off, fields_off, /*is_struct=*/false);

  std::vector<flatbuffers::Offset<reflection::Object>> objects_vec;
  auto objects_off =
      schema_fbb.CreateVectorOfSortedTables<reflection::Object>(
          &objects_vec);
  std::vector<flatbuffers::Offset<reflection::Enum>> enums_vec;
  auto enums_off =
      schema_fbb.CreateVectorOfSortedTables<reflection::Enum>(&enums_vec);

  auto schema_off = reflection::CreateSchema(
      schema_fbb, objects_off, enums_off, /*file_ident=*/0, /*file_ext=*/0,
      root_obj_off);
  schema_fbb.Finish(schema_off, reflection::SchemaIdentifier());

  const reflection::Schema* schema = flatbuffers::GetRoot<reflection::Schema>(
      schema_fbb.GetBufferPointer());
  TEST_EQ(schema->objects()->size(), 0u);
  const reflection::Object* root = schema->root_table();

  // Data buffer: root table with field "f" (offset 4) pointing at a valid
  // (empty) child table, and field "s" (offset 6) holding a short string.
  flatbuffers::FlatBufferBuilder data_fbb;
  auto child_start = data_fbb.StartTable();
  auto child_off = data_fbb.EndTable(child_start);
  auto str_off = data_fbb.CreateString("hi");
  auto root_start = data_fbb.StartTable();
  data_fbb.AddOffset(4, flatbuffers::Offset<void>(child_off));
  data_fbb.AddOffset(6, flatbuffers::Offset<void>(str_off.o));
  auto root_end = data_fbb.EndTable(root_start);
  data_fbb.Finish(flatbuffers::Offset<flatbuffers::Table>(root_end));

  std::vector<uint8_t> flatbuf(
      data_fbb.GetBufferPointer(),
      data_fbb.GetBufferPointer() + data_fbb.GetSize());

  const flatbuffers::Table* table =
      flatbuffers::GetRoot<flatbuffers::Table>(flatbuf.data());
  const flatbuffers::String* str_in_buf =
      table->GetPointer<const flatbuffers::String*>(6);

  // Before the fix, SetString's resize walk over the whole object graph
  // reaches the malicious "f" field and reads out of bounds (confirmed via
  // a real ASan SEGV in ResizeContext::ResizeTable). After the fix, the
  // malformed field is skipped during the walk and the string is resized
  // normally.
  flatbuffers::SetString(*schema, "a longer replacement string", str_in_buf,
                         &flatbuf, root);

  const flatbuffers::Table* table_after =
      flatbuffers::GetRoot<flatbuffers::Table>(flatbuf.data());
  auto str_after = table_after->GetPointer<const flatbuffers::String*>(6);
  TEST_EQ_STR(str_after->c_str(), "a longer replacement string");
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

