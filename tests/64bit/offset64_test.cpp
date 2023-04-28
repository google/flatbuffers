#include "offset64_test.h"

#include <stdint.h>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <ostream>

#include "flatbuffers/base.h"
#include "flatbuffers/buffer.h"
#include "flatbuffers/flatbuffers.h"
#include "test_64bit_generated.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {

void Offset64Test() {
  flatbuffers::FlatBufferBuilder64 fbb;

  size_t far_vector_size = 1LL << 2;
  size_t big_vector_size =
      2LL << 2;  // boost to a large number to make a big buffer

  {
    // First create the vectors that will be copied to the buffer.
    std::vector<uint8_t> far_data;
    far_data.resize(far_vector_size);
    far_data[0] = 4;
    far_data[far_vector_size-1] = 2;

    std::vector<uint8_t> big_data;
    big_data.resize(big_vector_size);
    big_data[0] = 8;
    big_data[big_vector_size-1] = 3;

    // Then serialize all the fields that have 64-bit offsets, as these must be
    // serialized before any 32-bit fields are added to the buffer.
    const Offset64<Vector<uint8_t>> far_vector_offset =
        fbb.CreateVector64<Vector>(far_data);

    const Offset64<String> far_string_offset =
        fbb.CreateString<Offset64>("some far string");

    const Offset64<Vector64<uint8_t>> big_vector_offset =
        fbb.CreateVector64(big_data);

    // Now that we are done with the 64-bit fields, we can create and add the
    // normal fields.
    const Offset<String> near_string_offset =
        fbb.CreateString("some near string");

    // Finish by building the root table by passing in all the offsets.
    const Offset<RootTable> root_table_offset =
        CreateRootTable(fbb, far_vector_offset, 0, far_string_offset,
                        big_vector_offset, near_string_offset);

    // TODO(derekbailey): the CreateRootTableDirect won't work correctly since
    // it builds the offsets in definition order, not 64-bit fields first. This
    // should be fixed, or possibly not emitted if it will play oddly.

    // Finish the buffer.
    fbb.Finish(root_table_offset);

    std::cout << "Buffer Size: 0x" << std::hex << fbb.GetSize() << std::endl;

    Verifier::Options options;
    // Allow the verifier to verify 64-bit buffers.
    options.max_size = FLATBUFFERS_MAX_64_BUFFER_SIZE;
    options.assert = true;

    // TODO(derekbailey): remove once we don't need to analyze the binary.
    // SaveFile("tests/64bit/offset64_test_cpp.bin",
    //          reinterpret_cast<const char *>(fbb.GetBufferPointer()),
    //          fbb.GetSize(), true);

    Verifier verifier(fbb.GetBufferPointer(), fbb.GetSize(), options);

    TEST_EQ(VerifyRootTableBuffer(verifier), true);
  }

  {
    const RootTable *root_table = GetRootTable(fbb.GetBufferPointer());

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

    // TODO(derekbailey): the CreateRootTableDirect won't work correctly since
    // it builds the offsets in definition order, not 64-bit fields first. This
    // should be fixed, or possibly not emitted if it will play oddly.

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

}  // namespace tests
}  // namespace flatbuffers
