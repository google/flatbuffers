#include "alignment_test.h"

#include "tests/alignment_test_generated.h"
#include "flatbuffers/flatbuffer_builder.h"
#include "test_assert.h"
#include <iostream>

namespace flatbuffers {
namespace tests {

void AlignmentTest() {  
  FlatBufferBuilder builder;

  BadAlignmentLarge large;
  Offset<OuterLarge> outer_large = CreateOuterLarge(builder, &large);

  BadAlignmentSmall *small;
  Offset<Vector<const BadAlignmentSmall *>> small_offset =
      builder.CreateUninitializedVectorOfStructs(9, &small);
  (void)small; // We do not have to write data to trigger the test failure

  Offset<BadAlignmentRoot> root =
      CreateBadAlignmentRoot(builder, outer_large, small_offset);

  builder.Finish(root);

  Verifier verifier(builder.GetBufferPointer(), builder.GetSize());
  TEST_ASSERT(VerifyBadAlignmentRootBuffer(verifier));

  builder.Clear();

  std::vector<JustSmallStruct> small_vector = {
    { 2, 1 }, { 3, 1 }, { 4, 1 }, { 5, 1 }, { 6, 1 }
  };
  Offset<SmallStructs> small_structs_root =
      CreateSmallStructsDirect(builder, &small_vector);

  builder.Finish(small_structs_root);

  Verifier verifier_small_structs(builder.GetBufferPointer(),
                                  builder.GetSize());
  TEST_ASSERT(verifier_small_structs.VerifyBuffer<SmallStructs>(nullptr));

  auto root_msg =
      flatbuffers::GetRoot<SmallStructs>(builder.GetBufferPointer());

  std::cout << "msg.size = " << root_msg->small_structs()->size() << std::endl;
  TEST_EQ(root_msg->small_structs()->size(), small_vector.size());

  for (size_t i = 0; i < root_msg->small_structs()->size(); ++i) {
    std::cout << "msg.small_structs[" << i
              << "] = " << (int)root_msg->small_structs()->Get(i)->var_0()
              << ":" << (int)root_msg->small_structs()->Get(i)->var_1()
              << std::endl;
    TEST_EQ(root_msg->small_structs()->Get(i)->var_0(),
            small_vector[i].var_0());
    TEST_EQ(root_msg->small_structs()->Get(i)->var_1(),
            small_vector[i].var_1());
  }
}

}  // namespace tests
}  // namespace flatbuffers
