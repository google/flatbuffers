#include "offset64_test.h"

#include <bits/stdint-uintn.h>

#include "flatbuffers/flatbuffers.h"
#include "test_64bit_generated.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {

void Offset64Test() {
  flatbuffers::FlatBufferBuilder fbb;

  std::vector<uint8_t> data;
  data.resize(10);

  const Offset64<Vector<uint8_t>> big_vector_offset = fbb.CreateVector64(data);

  RootTableBuilder root_table_builder(fbb);
  root_table_builder.add_big_vector(big_vector_offset);
  const Offset<RootTable> root_table_offset = root_table_builder.Finish();

  fbb.Finish(root_table_offset);

  flatbuffers::Verifier verifier(fbb.GetBufferPointer(), fbb.GetSize());
  TEST_EQ(VerifyRootTableBuffer(verifier), true);
}

}  // namespace tests
}  // namespace flatbuffers
