#include "offset64_test.h"

#include <stdint.h>
#include <limits>
#include <iostream>

#include "flatbuffers/base.h"
#include "flatbuffers/flatbuffers.h"
#include "test_64bit_generated.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {

void Offset64Test() {
  flatbuffers::FlatBufferBuilder fbb;

  size_t vector_size = 3LL << 3; // change to 30 to get 3 GiB.

  {
    std::vector<uint8_t> data;
    data.resize(vector_size); 

    const Offset64<Vector<uint8_t>> big_vector_offset =
        fbb.CreateVector64(data);

    RootTableBuilder root_table_builder(fbb);
    root_table_builder.add_big_vector(big_vector_offset);
    const Offset<RootTable> root_table_offset = root_table_builder.Finish();

    fbb.Finish(root_table_offset);


    Verifier::Options options;
    // Allow the verifier to verify 64-bit buffers.
    options.max_size = FLATBUFFERS_MAX_64_BUFFER_SIZE;

    Verifier verifier(fbb.GetBufferPointer(), fbb.GetSize(), options);

    TEST_EQ(VerifyRootTableBuffer(verifier), true);
  }

  {
    const RootTable *root_table = GetRootTable(fbb.GetBufferPointer());

    // Expect the big vector to be properly sized.
    TEST_EQ(root_table->big_vector()->size(), vector_size);
  }
}

}  // namespace tests
}  // namespace flatbuffers
