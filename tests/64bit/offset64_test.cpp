#include "offset64_test.h"

#include <stdint.h>

#include <cstdint>
#include <fstream>
// TODO(derekbailey): remove when done debugging
#include <iostream>
#include <limits>
#include <ostream>

#include "evolution/v1_generated.h"
#include "evolution/v2_generated.h"
#include "flatbuffers/base.h"
#include "flatbuffers/buffer.h"
#include "flatbuffers/flatbuffer_builder.h"
#include "flatbuffers/flatbuffers.h"
#include "test_64bit_generated.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {

void Offset64Test() {
  flatbuffers::FlatBufferBuilder64 builder;

  const size_t far_vector_size = 1LL << 2;
  const size_t big_vector_size = 1LL << 31;

  {
    // First create the vectors that will be copied to the buffer.
    std::vector<uint8_t> far_data;
    far_data.resize(far_vector_size);
    far_data[0] = 4;
    far_data[far_vector_size - 1] = 2;

    std::vector<uint8_t> big_data;
    big_data.resize(big_vector_size);
    big_data[0] = 8;
    big_data[big_vector_size - 1] = 3;

    // Then serialize all the fields that have 64-bit offsets, as these must be
    // serialized before any 32-bit fields are added to the buffer.
    const Offset64<Vector<uint8_t>> far_vector_offset =
        builder.CreateVector64<Vector>(far_data);

    const Offset64<String> far_string_offset =
        builder.CreateString<Offset64>("some far string");

    const Offset64<Vector64<uint8_t>> big_vector_offset =
        builder.CreateVector64(big_data);

    // Now that we are done with the 64-bit fields, we can create and add the
    // normal fields.
    const Offset<String> near_string_offset =
        builder.CreateString("some near string");

    // Finish by building the root table by passing in all the offsets.
    const Offset<RootTable> root_table_offset =
        CreateRootTable(builder, far_vector_offset, 0, far_string_offset,
                        big_vector_offset, near_string_offset);

    // Finish the buffer.
    builder.Finish(root_table_offset);

    // Ensure the buffer is big.
    TEST_ASSERT(builder.GetSize() > FLATBUFFERS_MAX_BUFFER_SIZE);

    Verifier::Options options;
    // Allow the verifier to verify 64-bit buffers.
    options.max_size = FLATBUFFERS_MAX_64_BUFFER_SIZE;
    options.assert = true;

    Verifier verifier(builder.GetBufferPointer(), builder.GetSize(), options);

    TEST_EQ(VerifyRootTableBuffer(verifier), true);
  }

  {
    const RootTable *root_table = GetRootTable(builder.GetBufferPointer());

    // Expect the far vector to be properly sized.
    TEST_EQ(root_table->far_vector()->size(), far_vector_size);
    TEST_EQ(root_table->far_vector()->Get(0), 4);
    TEST_EQ(root_table->far_vector()->Get(far_vector_size - 1), 2);

    TEST_EQ_STR(root_table->far_string()->c_str(), "some far string");

    // Expect the big vector to be properly sized.
    TEST_EQ(root_table->big_vector()->size(), big_vector_size);
    TEST_EQ(root_table->big_vector()->Get(0), 8);
    TEST_EQ(root_table->big_vector()->Get(big_vector_size - 1), 3);

    TEST_EQ_STR(root_table->near_string()->c_str(), "some near string");
  }
}

void Offset64SerializedFirst() {
  flatbuffers::FlatBufferBuilder64 fbb;

  // First create the vectors that will be copied to the buffer.
  std::vector<uint8_t> data;
  data.resize(64);

  // Then serialize all the fields that have 64-bit offsets, as these must be
  // serialized before any 32-bit fields are added to the buffer.
  fbb.CreateVector64(data);

  // TODO(derekbailey): figure out how to test assertions.
  // Uncommenting this line should fail the test with an assertion.
  // fbb.CreateString("some near string");

  fbb.CreateVector64(data);
}

void Offset64NestedFlatBuffer() {
  flatbuffers::FlatBufferBuilder64 fbb;

  // First serialize a nested buffer.
  const Offset<String> near_string_offset =
      fbb.CreateString("nested: some near string");

  // Finish by building the root table by passing in all the offsets.
  const Offset<RootTable> root_table_offset =
      CreateRootTable(fbb, 0, 0, 0, 0, near_string_offset, 0);

  // Finish the buffer.
  fbb.Finish(root_table_offset);

  // Ensure the buffer is valid.
  const RootTable *root_table = GetRootTable(fbb.GetBufferPointer());
  TEST_EQ_STR(root_table->near_string()->c_str(), "nested: some near string");

  // Copy the data out of the builder.
  std::vector<uint8_t> nested_data{ fbb.GetBufferPointer(),
                                    fbb.GetBufferPointer() + fbb.GetSize() };

  {
    // Clear so we can reuse the builder.
    fbb.Clear();

    const Offset64<Vector64<uint8_t>> nested_flatbuffer_offset =
        fbb.CreateVector64<Vector64>(nested_data);

    // Now that we are done with the 64-bit fields, we can create and add the
    // normal fields.
    const Offset<String> near_string_offset =
        fbb.CreateString("some near string");

    // Finish by building the root table by passing in all the offsets.
    const Offset<RootTable> root_table_offset = CreateRootTable(
        fbb, 0, 0, 0, 0, near_string_offset, nested_flatbuffer_offset);

    // Finish the buffer.
    fbb.Finish(root_table_offset);

    Verifier::Options options;
    // Allow the verifier to verify 64-bit buffers.
    options.max_size = FLATBUFFERS_MAX_64_BUFFER_SIZE;
    options.assert = true;

    Verifier verifier(fbb.GetBufferPointer(), fbb.GetSize(), options);

    TEST_EQ(VerifyRootTableBuffer(verifier), true);
  }

  {
    const RootTable *root_table = GetRootTable(fbb.GetBufferPointer());

    // Test that the parent buffer field is ok.
    TEST_EQ_STR(root_table->near_string()->c_str(), "some near string");

    // Expect nested buffer to be properly sized.
    TEST_EQ(root_table->nested_root()->size(), nested_data.size());

    // Expect the direct accessors to the nested buffer work.
    TEST_EQ_STR(root_table->nested_root_nested_root()->near_string()->c_str(),
                "nested: some near string");
  }
}

void Offset64CreateDirect() {
  flatbuffers::FlatBufferBuilder64 fbb;

  // Create a vector of some data
  std::vector<uint8_t> data{ 0, 1, 2 };

  // Call the "Direct" creation method to ensure that things are added to the
  // buffer in the correct order, Offset64 first followed by any Offsets.
  const Offset<RootTable> root_table_offset = CreateRootTableDirect(
      fbb, &data, 0, "some far string", &data, "some near string");

  // Finish the buffer.
  fbb.Finish(root_table_offset);

  Verifier::Options options;
  // Allow the verifier to verify 64-bit buffers.
  options.max_size = FLATBUFFERS_MAX_64_BUFFER_SIZE;
  options.assert = true;

  Verifier verifier(fbb.GetBufferPointer(), fbb.GetSize(), options);

  TEST_EQ(VerifyRootTableBuffer(verifier), true);

  // Verify the data.
  const RootTable *root_table = GetRootTable(fbb.GetBufferPointer());
  TEST_EQ(root_table->far_vector()->size(), data.size());
  TEST_EQ(root_table->big_vector()->size(), data.size());
  TEST_EQ_STR(root_table->far_string()->c_str(), "some far string");
  TEST_EQ_STR(root_table->near_string()->c_str(), "some near string");
}

void Offset64Evolution() {
  // Some common data for the tests.
  const std::vector<uint8_t> data = { 1, 2, 3, 4 };
  const std::vector<uint8_t> big_data = { 6, 7, 8, 9, 10 };

  // Built V1 read V2
  {
    // Use the 32-bit builder since V1 doesn't have any 64-bit offsets.
    FlatBufferBuilder builder;

    builder.Finish(v1::CreateRootTableDirect(builder, 1234, &data));

    // Use each version to get a view at the root table.
    auto v1_root = v1::GetRootTable(builder.GetBufferPointer());
    auto v2_root = v2::GetRootTable(builder.GetBufferPointer());

    // Test field equivalents for fields common to V1 and V2.
    TEST_EQ(v1_root->a(), v2_root->a());

    TEST_EQ(v1_root->b(), v2_root->b());
    TEST_EQ(v1_root->b()->Get(2), 3);
    TEST_EQ(v2_root->b()->Get(2), 3);

    // This field is added in V2, so it should be null since V1 couldn't have
    // written it.
    TEST_ASSERT(v2_root->big_vector() == nullptr);
  }

  // Built V2 read V1
  {
    // Use the 64-bit builder since V2 has 64-bit offsets.
    FlatBufferBuilder64 builder;

    builder.Finish(v2::CreateRootTableDirect(builder, 1234, &data, &big_data));

    // Use each version to get a view at the root table.
    auto v1_root = v1::GetRootTable(builder.GetBufferPointer());
    auto v2_root = v2::GetRootTable(builder.GetBufferPointer());

    // Test field equivalents for fields common to V1 and V2.
    TEST_EQ(v1_root->a(), v2_root->a());

    TEST_EQ(v1_root->b(), v2_root->b());
    TEST_EQ(v1_root->b()->Get(2), 3);
    TEST_EQ(v2_root->b()->Get(2), 3);

    // Test that V2 can read the big vector, which V1 doesn't even have
    // accessors for (i.e. v1_root->big_vector() doesn't exist).
    TEST_ASSERT(v2_root->big_vector() != nullptr);
    TEST_EQ(v2_root->big_vector()->size(), big_data.size());
    TEST_EQ(v2_root->big_vector()->Get(2), 8);
  }

  // Built V2 read V1, bigger than max 32-bit buffer sized.
  // This checks that even a large buffer can still be read by V1.
  {
    // Use the 64-bit builder since V2 has 64-bit offsets.
    FlatBufferBuilder64 builder;

    std::vector<uint8_t> giant_data;
    giant_data.resize(1LL << 31);
    giant_data[2] = 42;

    builder.Finish(
        v2::CreateRootTableDirect(builder, 1234, &data, &giant_data));

    // Ensure the buffer is bigger than the 32-bit size limit for V1.
    TEST_ASSERT(builder.GetSize() > FLATBUFFERS_MAX_BUFFER_SIZE);

    // Use each version to get a view at the root table.
    auto v1_root = v1::GetRootTable(builder.GetBufferPointer());
    auto v2_root = v2::GetRootTable(builder.GetBufferPointer());

    // Test field equivalents for fields common to V1 and V2.
    TEST_EQ(v1_root->a(), v2_root->a());

    TEST_EQ(v1_root->b(), v2_root->b());
    TEST_EQ(v1_root->b()->Get(2), 3);
    TEST_EQ(v2_root->b()->Get(2), 3);

    // Test that V2 can read the big vector, which V1 doesn't even have
    // accessors for (i.e. v1_root->big_vector() doesn't exist).
    TEST_ASSERT(v2_root->big_vector() != nullptr);
    TEST_EQ(v2_root->big_vector()->size(), giant_data.size());
    TEST_EQ(v2_root->big_vector()->Get(2), 42);
  }
}

}  // namespace tests
}  // namespace flatbuffers
