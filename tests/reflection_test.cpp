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

static void ReflectionAdversarialVerificationTest(
    const reflection::Schema& schema, const uint8_t* valid_flatbuf,
    size_t valid_length);

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

  ReflectionAdversarialVerificationTest(schema, fbb.GetBufferPointer(),
                                        fbb.GetSize());
}

void ReflectionAdversarialVerificationTest(const reflection::Schema& schema,
                                           const uint8_t* valid_flatbuf,
                                           size_t valid_length) {
  // 1. Null buffer rejection.
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), nullptr, 0), false);
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), nullptr, 1024),
          false);
  TEST_EQ(
      flatbuffers::VerifySizePrefixed(schema, *schema.root_table(), nullptr, 0),
      false);
  TEST_EQ(flatbuffers::VerifySizePrefixed(schema, *schema.root_table(), nullptr,
                                          1024),
          false);

  // 2. Truncation below minimum buffer size.
  uint8_t tiny[8] = {0};
  for (size_t l = 0; l < FLATBUFFERS_MIN_BUFFER_SIZE; ++l) {
    TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), tiny, l), false);
    TEST_EQ(
        flatbuffers::VerifySizePrefixed(schema, *schema.root_table(), tiny, l),
        false);
  }

  // 3. Malformed root offsets on min-sized buffers.
  uint8_t zero_root[16] = {0};
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), zero_root,
                              sizeof(zero_root)),
          false);

  uint8_t oob_root[16] = {100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), oob_root,
                              sizeof(oob_root)),
          false);

  uint8_t max_root[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0,
                          0,    0,    0,    0,    0, 0, 0, 0};
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), max_root,
                              sizeof(max_root)),
          false);

  // 4. Truncations of an otherwise valid flatbuffer.
  for (size_t trunc = FLATBUFFERS_MIN_BUFFER_SIZE; trunc < valid_length;
       trunc += 17) {
    TEST_EQ(
        flatbuffers::Verify(schema, *schema.root_table(), valid_flatbuf, trunc),
        false);
  }

  // 5. Size-prefixed buffer validation and corruption.
  flatbuffers::FlatBufferBuilder fbb_sp;
  auto root_offset_sp =
      flatbuffers::CopyTable(fbb_sp, schema, *schema.root_table(),
                             *flatbuffers::GetAnyRoot(valid_flatbuf), true);
  fbb_sp.FinishSizePrefixed(root_offset_sp, MonsterIdentifier());
  TEST_EQ(flatbuffers::VerifySizePrefixed(schema, *schema.root_table(),
                                          fbb_sp.GetBufferPointer(),
                                          fbb_sp.GetSize()),
          true);

  // Size prefix exceeds total buffer length.
  std::vector<uint8_t> corrupted_sp(
      fbb_sp.GetBufferPointer(), fbb_sp.GetBufferPointer() + fbb_sp.GetSize());
  uoffset_t bad_prefix = static_cast<uoffset_t>(fbb_sp.GetSize() + 64);
  memcpy(corrupted_sp.data(), &bad_prefix, sizeof(bad_prefix));
  TEST_EQ(
      flatbuffers::VerifySizePrefixed(schema, *schema.root_table(),
                                      corrupted_sp.data(), corrupted_sp.size()),
      false);

  // Size prefix buffer truncated below minimum content size.
  TEST_EQ(flatbuffers::VerifySizePrefixed(schema, *schema.root_table(),
                                          fbb_sp.GetBufferPointer(),
                                          sizeof(uoffset_t) + 1),
          false);

  // 6. Adversarial mutations of table offsets and fields.
  std::vector<uint8_t> corrupted(valid_flatbuf, valid_flatbuf + valid_length);
  uoffset_t root_offset = flatbuffers::ReadScalar<uoffset_t>(corrupted.data());
  soffset_t vtable_offset =
      flatbuffers::ReadScalar<soffset_t>(corrupted.data() + root_offset);
  uint8_t* vtable_ptr = corrupted.data() + root_offset - vtable_offset;

  // Wild vtable offset pointing out of bounds.
  corrupted.assign(valid_flatbuf, valid_flatbuf + valid_length);
  soffset_t bad_vtable_offset = 0x7FFFFFFF;
  memcpy(corrupted.data() + root_offset, &bad_vtable_offset,
         sizeof(bad_vtable_offset));
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), corrupted.data(),
                              corrupted.size()),
          false);

  // Missing required field: "name" is required in Monster schema.
  corrupted.assign(valid_flatbuf, valid_flatbuf + valid_length);
  root_offset = flatbuffers::ReadScalar<uoffset_t>(corrupted.data());
  vtable_offset =
      flatbuffers::ReadScalar<soffset_t>(corrupted.data() + root_offset);
  vtable_ptr = corrupted.data() + root_offset - vtable_offset;
  auto name_field = schema.root_table()->fields()->LookupByKey("name");
  TEST_NOTNULL(name_field);
  voffset_t name_voffset = name_field->offset();
  voffset_t zero_field = 0;
  memcpy(vtable_ptr + name_voffset, &zero_field, sizeof(zero_field));
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), corrupted.data(),
                              corrupted.size()),
          false);

  // Corrupted string offset.
  corrupted.assign(valid_flatbuf, valid_flatbuf + valid_length);
  root_offset = flatbuffers::ReadScalar<uoffset_t>(corrupted.data());
  vtable_offset =
      flatbuffers::ReadScalar<soffset_t>(corrupted.data() + root_offset);
  vtable_ptr = corrupted.data() + root_offset - vtable_offset;
  voffset_t name_field_pos =
      flatbuffers::ReadScalar<voffset_t>(vtable_ptr + name_voffset);
  if (name_field_pos) {
    uoffset_t bad_string_offset = 0x7FFFFFF0;
    memcpy(corrupted.data() + root_offset + name_field_pos, &bad_string_offset,
           sizeof(bad_string_offset));
    TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), corrupted.data(),
                                corrupted.size()),
            false);
  }

  // Corrupted vector offset.
  corrupted.assign(valid_flatbuf, valid_flatbuf + valid_length);
  root_offset = flatbuffers::ReadScalar<uoffset_t>(corrupted.data());
  vtable_offset =
      flatbuffers::ReadScalar<soffset_t>(corrupted.data() + root_offset);
  vtable_ptr = corrupted.data() + root_offset - vtable_offset;
  auto inv_field = schema.root_table()->fields()->LookupByKey("inventory");
  TEST_NOTNULL(inv_field);
  voffset_t inv_field_pos =
      flatbuffers::ReadScalar<voffset_t>(vtable_ptr + inv_field->offset());
  if (inv_field_pos) {
    uoffset_t bad_vec_offset = 0x7FFFFFF0;
    memcpy(corrupted.data() + root_offset + inv_field_pos, &bad_vec_offset,
           sizeof(bad_vec_offset));
    TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), corrupted.data(),
                                corrupted.size()),
            false);
  }

  // Corrupted table vector element offset.
  corrupted.assign(valid_flatbuf, valid_flatbuf + valid_length);
  root_offset = flatbuffers::ReadScalar<uoffset_t>(corrupted.data());
  vtable_offset =
      flatbuffers::ReadScalar<soffset_t>(corrupted.data() + root_offset);
  vtable_ptr = corrupted.data() + root_offset - vtable_offset;
  auto tables_field =
      schema.root_table()->fields()->LookupByKey("testarrayoftables");
  TEST_NOTNULL(tables_field);
  voffset_t tables_field_pos =
      flatbuffers::ReadScalar<voffset_t>(vtable_ptr + tables_field->offset());
  if (tables_field_pos) {
    uint8_t* vec_ptr = corrupted.data() + root_offset + tables_field_pos;
    uoffset_t vec_offset = flatbuffers::ReadScalar<uoffset_t>(vec_ptr);
    uint8_t* vec_data = vec_ptr + vec_offset;
    uoffset_t vec_len = flatbuffers::ReadScalar<uoffset_t>(vec_data);
    if (vec_len > 0) {
      uoffset_t bad_elem_offset = 0x7FFFFFF0;
      memcpy(vec_data + sizeof(uoffset_t), &bad_elem_offset,
             sizeof(bad_elem_offset));
      TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(),
                                  corrupted.data(), corrupted.size()),
              false);
    }
  }

  // Corrupted union fields:
  // Inconsistent union: discriminant is set (non-zero) but union value offset
  // in vtable is 0 (omitted).
  corrupted.assign(valid_flatbuf, valid_flatbuf + valid_length);
  root_offset = flatbuffers::ReadScalar<uoffset_t>(corrupted.data());
  vtable_offset =
      flatbuffers::ReadScalar<soffset_t>(corrupted.data() + root_offset);
  vtable_ptr = corrupted.data() + root_offset - vtable_offset;
  auto utype_field = schema.root_table()->fields()->LookupByKey("test_type");
  auto uval_field = schema.root_table()->fields()->LookupByKey("test");
  TEST_NOTNULL(utype_field);
  TEST_NOTNULL(uval_field);
  voffset_t utype_field_pos =
      flatbuffers::ReadScalar<voffset_t>(vtable_ptr + utype_field->offset());
  if (utype_field_pos) {
    uint8_t valid_utype = 1;
    memcpy(corrupted.data() + root_offset + utype_field_pos, &valid_utype,
           sizeof(valid_utype));
    voffset_t zero_uval = 0;
    memcpy(vtable_ptr + uval_field->offset(), &zero_uval, sizeof(zero_uval));
    TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), corrupted.data(),
                                corrupted.size()),
            false);
  }

  // 7. Dynamic Union Vector Safety Test:
  // Tests null pointer dereference prevention on missing union type vector.
  flatbuffers::IDLOptions idl_opts;
  idl_opts.lang_to_generate |= flatbuffers::IDLOptions::kBinary;
  flatbuffers::Parser idl_parser(idl_opts);
  TEST_EQ(idl_parser.Parse("table Item { id:int; }\n"
                           "union AnyItem { Item }\n"
                           "table Container { items:[AnyItem]; }\n"
                           "root_type Container;\n"),
          true);
  TEST_EQ(idl_parser.Parse("{ items_type: [Item], items: [{id: 42}] }"), true);
  auto container_buf = idl_parser.builder_.Release();
  idl_parser.Serialize();
  auto container_schema =
      reflection::GetSchema(idl_parser.builder_.GetBufferPointer());
  TEST_NOTNULL(container_schema);

  auto valid_c_buf = container_buf.data();
  auto valid_c_size = container_buf.size();
  TEST_EQ(
      flatbuffers::Verify(*container_schema, *container_schema->root_table(),
                          valid_c_buf, valid_c_size),
      true);

  // Inconsistent union vector: values vector present, but types vector zeroed
  // in vtable! (Prior to our hardening, this dereferenced type_vec as nullptr
  // in type_vec->size())
  std::vector<uint8_t> bad_union_vec(valid_c_buf, valid_c_buf + valid_c_size);
  uoffset_t c_root = flatbuffers::ReadScalar<uoffset_t>(bad_union_vec.data());
  soffset_t c_vt_offset =
      flatbuffers::ReadScalar<soffset_t>(bad_union_vec.data() + c_root);
  uint8_t* c_vt_ptr = bad_union_vec.data() + c_root - c_vt_offset;
  auto items_field =
      container_schema->root_table()->fields()->LookupByKey("items");
  TEST_NOTNULL(items_field);
  voffset_t utype_voffset = items_field->offset() - sizeof(voffset_t);
  voffset_t zero_vt = 0;
  memcpy(c_vt_ptr + utype_voffset, &zero_vt, sizeof(zero_vt));
  TEST_EQ(
      flatbuffers::Verify(*container_schema, *container_schema->root_table(),
                          bad_union_vec.data(), bad_union_vec.size()),
      false);

  // Inconsistent union vector: types vector present, but values vector zeroed
  // in vtable!
  std::vector<uint8_t> bad_union_vals(valid_c_buf, valid_c_buf + valid_c_size);
  c_root = flatbuffers::ReadScalar<uoffset_t>(bad_union_vals.data());
  c_vt_offset =
      flatbuffers::ReadScalar<soffset_t>(bad_union_vals.data() + c_root);
  c_vt_ptr = bad_union_vals.data() + c_root - c_vt_offset;
  memcpy(c_vt_ptr + items_field->offset(), &zero_vt, sizeof(zero_vt));
  TEST_EQ(
      flatbuffers::Verify(*container_schema, *container_schema->root_table(),
                          bad_union_vals.data(), bad_union_vals.size()),
      false);
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
    flatbuffers::ForAllFields(stat_object, /*reverse=*/false,
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
    flatbuffers::ForAllFields(stat_object, /*reverse=*/true,
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
    flatbuffers::ForAllFields(root_table, /*reverse=*/false,
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
    flatbuffers::ForAllFields(root_table, /*reverse=*/true,
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
