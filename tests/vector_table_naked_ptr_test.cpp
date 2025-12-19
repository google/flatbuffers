#include "tests/vector_table_naked_ptr_test.h"

#include "tests/test_assert.h"
#include "tests/vector_table_naked_ptr/vector_table_naked_ptr_generated.h"

namespace flatbuffers {
namespace tests {

void VectorTableNakedPtrTest() {
  // ---------------------------------------
  // 1) Build original native objects
  // ---------------------------------------
  auto* b1 = new BT();
  auto* b2 = new BT();
  b1->id = 777;
  b2->id = 888;

  AT a_src;
  a_src.b.push_back(b1);
  a_src.b.push_back(b2);

  // ---------------------------------------
  // 2) Pack into FlatBuffer
  // ---------------------------------------
  flatbuffers::FlatBufferBuilder fbb;
  auto a_offset = A::Pack(fbb, &a_src);
  FinishABuffer(fbb, a_offset);

  const void* buf = fbb.GetBufferPointer();
  const A* a_fb = GetA(buf);

  // ---------------------------------------
  // 3) Pre-allocate destination with garbage
  // ---------------------------------------
  AT a_dst;

  // Pre-fill with wrong pointers to ensure UnPackTo overwrites correctly
  a_dst.b.resize(2);
  a_dst.b[0] = new BT();
  a_dst.b[1] = new BT();
  a_dst.b[0]->id = -1;
  a_dst.b[1]->id = -1;

  // ---------------------------------------
  // 4) Call the function under test: UnPackTo
  // ---------------------------------------
  a_fb->UnPackTo(&a_dst, nullptr);

  // ---------------------------------------
  // 5) Validate overwrite + deep copy
  // ---------------------------------------
  TEST_ASSERT(a_dst.b.size() == 2);

  TEST_ASSERT(a_dst.b[0] != nullptr);
  TEST_ASSERT(a_dst.b[1] != nullptr);

  TEST_ASSERT(a_dst.b[0]->id == 777);
  TEST_ASSERT(a_dst.b[1]->id == 888);

  // Ensure original pointers were not reused
  TEST_ASSERT(a_dst.b[0] != b1);
  TEST_ASSERT(a_dst.b[1] != b2);

  // ---------------------------------------
  // 6) Clean up ownership
  // ---------------------------------------
  delete a_dst.b[0];
  delete a_dst.b[1];

  delete b1;
  delete b2;
}

}  // namespace tests
}  // namespace flatbuffers
