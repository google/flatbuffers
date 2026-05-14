#include "tests/default_vectors_strings_test.h"

#include <cstdint>
#include <vector>

#include "include/flatbuffers/buffer.h"
#include "include/flatbuffers/flatbuffer_builder.h"
#include "include/flatbuffers/string.h"
#include "include/flatbuffers/vector.h"
#include "include/flatbuffers/verifier.h"
#include "tests/default_vectors_strings_test.fbs.h"
#include "tests/test_assert.h"

namespace flatbuffers {
namespace tests {

using flatbuffers::FlatBufferBuilder64;
using flatbuffers::Offset;
using flatbuffers::String;
using flatbuffers::Verifier;

void DefaultVectorsStringsTest_EmptyOnDefault_Const() {
  FlatBufferBuilder64 builder;

  // Create table without providing the fields with defaults.
  DefaultVectorsStringsTest::TableWithDefaultVectorsBuilder tbl_builder(
      builder);
  tbl_builder.add_regular_int(100);
  auto offset = tbl_builder.Finish();
  builder.Finish(offset);

  Verifier verifier(builder.GetBufferPointer(), builder.GetSize());
  TEST_ASSERT(
      DefaultVectorsStringsTest::VerifyTableWithDefaultVectorsBuffer(verifier));

  const auto* table = DefaultVectorsStringsTest::GetTableWithDefaultVectors(
      builder.GetBufferPointer());
  TEST_NOTNULL(table);

  // Verify default scalar vectors.
  TEST_NOTNULL(table->int_vec());
  TEST_EQ(table->int_vec()->size(), 0);
  TEST_NOTNULL(table->bool_vec());
  TEST_EQ(table->bool_vec()->size(), 0);
  TEST_NOTNULL(table->char_vec());
  TEST_EQ(table->char_vec()->size(), 0);
  TEST_NOTNULL(table->uchar_vec());
  TEST_EQ(table->uchar_vec()->size(), 0);
  TEST_NOTNULL(table->short_vec());
  TEST_EQ(table->short_vec()->size(), 0);
  TEST_NOTNULL(table->ushort_vec());
  TEST_EQ(table->ushort_vec()->size(), 0);
  TEST_NOTNULL(table->uint_vec());
  TEST_EQ(table->uint_vec()->size(), 0);
  TEST_NOTNULL(table->long_vec());
  TEST_EQ(table->long_vec()->size(), 0);
  TEST_NOTNULL(table->ulong_vec());
  TEST_EQ(table->ulong_vec()->size(), 0);
  TEST_NOTNULL(table->float_vec());
  TEST_EQ(table->float_vec()->size(), 0);
  TEST_NOTNULL(table->double_vec());
  TEST_EQ(table->double_vec()->size(), 0);

  // Verify default string_vec.
  TEST_NOTNULL(table->string_vec());
  TEST_EQ(table->string_vec()->size(), 0);

  // Verify default string fields.
  TEST_NOTNULL(table->empty_string());
  TEST_EQ_STR(table->empty_string()->c_str(), "");
  TEST_NOTNULL(table->some_string());
  TEST_EQ_STR(table->some_string()->c_str(), "some");

  // Verify default struct_vec.
  TEST_NOTNULL(table->struct_vec());
  TEST_EQ(table->struct_vec()->size(), 0);

  // Verify default table_vec.
  TEST_NOTNULL(table->table_vec());
  TEST_EQ(table->table_vec()->size(), 0);

  // Verify default enum_vec.
  TEST_NOTNULL(table->enum_vec());
  TEST_EQ(table->enum_vec()->size(), 0);

  // Verify non-default vector field.
  TEST_NULL(table->regular_int_vec());

  // Verify pointer and offset64 combinations.
  TEST_NOTNULL(table->int_vec64());
  TEST_EQ(table->int_vec64()->size(), 0);
  TEST_NOTNULL(table->int_vec_offset64());
  TEST_EQ(table->int_vec_offset64()->size(), 0);

  // Verify non-default field.
  TEST_EQ(table->regular_int(), 100);
}

void DefaultVectorsStringsTest_EmptyOnDefault_Mutable() {
  FlatBufferBuilder64 builder;

  // Create table without providing the fields with defaults.
  DefaultVectorsStringsTest::TableWithDefaultVectorsBuilder tbl_builder(
      builder);
  tbl_builder.add_regular_int(100);
  auto offset = tbl_builder.Finish();
  builder.Finish(offset);

  Verifier verifier(builder.GetBufferPointer(), builder.GetSize());
  TEST_ASSERT(
      DefaultVectorsStringsTest::VerifyTableWithDefaultVectorsBuffer(verifier));

  auto* mutable_table =
      DefaultVectorsStringsTest::GetMutableTableWithDefaultVectors(
          builder.GetBufferPointer());
  TEST_NOTNULL(mutable_table);

  // Verify default scalar vectors.
  TEST_NOTNULL(mutable_table->mutable_int_vec());
  TEST_EQ(mutable_table->mutable_int_vec()->size(), 0);
  TEST_NOTNULL(mutable_table->mutable_bool_vec());
  TEST_EQ(mutable_table->mutable_bool_vec()->size(), 0);
  TEST_NOTNULL(mutable_table->mutable_char_vec());
  TEST_EQ(mutable_table->mutable_char_vec()->size(), 0);
  TEST_NOTNULL(mutable_table->mutable_uchar_vec());
  TEST_EQ(mutable_table->mutable_uchar_vec()->size(), 0);
  TEST_NOTNULL(mutable_table->mutable_short_vec());
  TEST_EQ(mutable_table->mutable_short_vec()->size(), 0);
  TEST_NOTNULL(mutable_table->mutable_ushort_vec());
  TEST_EQ(mutable_table->mutable_ushort_vec()->size(), 0);
  TEST_NOTNULL(mutable_table->mutable_uint_vec());
  TEST_EQ(mutable_table->mutable_uint_vec()->size(), 0);
  TEST_NOTNULL(mutable_table->mutable_long_vec());
  TEST_EQ(mutable_table->mutable_long_vec()->size(), 0);
  TEST_NOTNULL(mutable_table->mutable_ulong_vec());
  TEST_EQ(mutable_table->mutable_ulong_vec()->size(), 0);
  TEST_NOTNULL(mutable_table->mutable_float_vec());
  TEST_EQ(mutable_table->mutable_float_vec()->size(), 0);
  TEST_NOTNULL(mutable_table->mutable_double_vec());
  TEST_EQ(mutable_table->mutable_double_vec()->size(), 0);

  // Verify default struct_vec.
  TEST_NOTNULL(mutable_table->mutable_struct_vec());
  TEST_EQ(mutable_table->mutable_struct_vec()->size(), 0);

  // Verify default table_vec.
  TEST_NOTNULL(mutable_table->mutable_table_vec());
  TEST_EQ(mutable_table->mutable_table_vec()->size(), 0);

  // Verify default enum_vec.
  TEST_NOTNULL(mutable_table->mutable_enum_vec());
  TEST_EQ(mutable_table->mutable_enum_vec()->size(), 0);

  // Verify non-default vector field.
  TEST_NULL(mutable_table->mutable_regular_int_vec());

  // Verify pointer and offset64 combinations.
  TEST_NOTNULL(mutable_table->mutable_int_vec64());
  TEST_EQ(mutable_table->mutable_int_vec64()->size(), 0);
  TEST_NOTNULL(mutable_table->mutable_int_vec_offset64());
  TEST_EQ(mutable_table->mutable_int_vec_offset64()->size(), 0);
}

void DefaultVectorsStringsTest_WithValues() {
  // Create a table with values for the defaulted vector fields.
  FlatBufferBuilder64 builder;
  auto int_vec64 = builder.CreateVector64(std::vector<int>({30, 40}));
  auto int_vec_offset64 =
      builder.CreateVector64<flatbuffers::Vector>(std::vector<int>({50, 60}));
  auto int_vec = builder.CreateVector(std::vector<int>({1, 2}));
  auto bool_vec = builder.CreateVector(std::vector<uint8_t>({true, false}));
  auto string_vec = builder.CreateVector(std::vector<Offset<String>>(
      {builder.CreateString("a"), builder.CreateString("b")}));
  auto empty_string = builder.CreateString("not empty");
  auto some_string = builder.CreateString("not some");
  DefaultVectorsStringsTest::MyStruct structs[] = {
      DefaultVectorsStringsTest::MyStruct(1, 2),
      DefaultVectorsStringsTest::MyStruct(3, 4)};
  auto struct_vec = builder.CreateVectorOfStructs(structs, 2);
  auto regular_int_vec = builder.CreateVector(std::vector<int>({10, 20}));

  DefaultVectorsStringsTest::TableWithDefaultVectorsBuilder tbl_builder(
      builder);
  tbl_builder.add_int_vec(int_vec);
  tbl_builder.add_int_vec64(int_vec64);
  tbl_builder.add_int_vec_offset64(int_vec_offset64);

  tbl_builder.add_bool_vec(bool_vec);
  tbl_builder.add_string_vec(string_vec);
  tbl_builder.add_empty_string(empty_string);
  tbl_builder.add_some_string(some_string);
  tbl_builder.add_struct_vec(struct_vec);
  tbl_builder.add_regular_int_vec(regular_int_vec);
  auto offset = tbl_builder.Finish();
  builder.Finish(offset);

  Verifier verifier(builder.GetBufferPointer(), builder.GetSize());
  TEST_ASSERT(
      DefaultVectorsStringsTest::VerifyTableWithDefaultVectorsBuffer(verifier));

  const auto* table = DefaultVectorsStringsTest::GetTableWithDefaultVectors(
      builder.GetBufferPointer());

  TEST_EQ(table->int_vec()->size(), 2);
  TEST_EQ(table->int_vec()->Get(1), 2);
  TEST_EQ(table->bool_vec()->size(), 2);
  TEST_EQ(table->bool_vec()->Get(0), true);
  TEST_EQ(table->string_vec()->size(), 2);
  TEST_EQ_STR(table->string_vec()->Get(1)->c_str(), "b");
  TEST_EQ_STR(table->empty_string()->c_str(), "not empty");
  TEST_EQ_STR(table->some_string()->c_str(), "not some");
  TEST_EQ(table->struct_vec()->size(), 2);
  TEST_EQ(table->struct_vec()->Get(1)->b(), 4);
  TEST_EQ(table->regular_int_vec()->size(), 2);
  TEST_EQ(table->regular_int_vec()->Get(1), 20);

  TEST_EQ(table->int_vec64()->size(), 2);
  TEST_EQ(table->int_vec64()->Get(1), 40);
  TEST_EQ(table->int_vec_offset64()->size(), 2);
  TEST_EQ(table->int_vec_offset64()->Get(1), 60);
}

void DefaultVectorsStringsTest_BufferSize() {
  FlatBufferBuilder64 builder;
  // Create a table where all fields with default values are omitted.
  DefaultVectorsStringsTest::TableWithDefaultVectorsBuilder tbl_builder(
      builder);
  auto offset = tbl_builder.Finish();
  builder.Finish(offset);

  Verifier verifier(builder.GetBufferPointer(), builder.GetSize());
  TEST_ASSERT(
      DefaultVectorsStringsTest::VerifyTableWithDefaultVectorsBuffer(verifier));

  // The buffer should be small when only defaults are used.
  // This value can be adjusted if the schema changes.
  constexpr unsigned int buffer_size_threshold_in_bytes = 12;
  TEST_ASSERT(builder.GetSize() <= buffer_size_threshold_in_bytes);
}

void DefaultVectorsStringsTest() {
  DefaultVectorsStringsTest_EmptyOnDefault_Const();
  DefaultVectorsStringsTest_EmptyOnDefault_Mutable();
  DefaultVectorsStringsTest_WithValues();
  DefaultVectorsStringsTest_BufferSize();
}

}  // namespace tests
}  // namespace flatbuffers
