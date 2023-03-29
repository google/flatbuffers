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

  // BadAlignmentSmall *small;
  // Offset<Vector<const BadAlignmentSmall *>> small_offset =
  //     builder.CreateUninitializedVectorOfStructs(9, &small);
  // (void)small; // We do not have to write data to trigger the test failure

  // Offset<Vector<const BadAlignmentSmall *>> small_offset =
  //     builder.CreateUninitializedVectorOfStructs(9, &small);

  // Offset<BadAlignmentRoot> root =
  //     CreateBadAlignmentRoot(builder, outer_large, small_offset);

  std::vector<BadAlignmentSmall> small_vector{{2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}};

  Offset<BadAlignmentRoot> root =
    CreateBadAlignmentRootDirect(builder, outer_large, &small_vector);

  // builder.ForceVectorAlignment(small_vector.size(), sizeof(BadAlignmentSmall), 8);
  // auto small__ = builder.CreateVectorOfStructs<BadAlignmentSmall>(small_vector);
  // auto small__ = CreateVectorOfStructs<BadAlignmentSmall>(builder, small_vector);
  // Offset<BadAlignmentRoot> root =
  //     CreateBadAlignmentRoot(builder, outer_large, small__);

  builder.Finish(root);

  auto root_msg =
      flatbuffers::GetRoot<BadAlignmentRoot>(builder.GetBufferPointer());

  std::cout << "msg.size = " << root_msg->small()->size() << std::endl;
  for (size_t i = 0; i < root_msg->small()->size(); ++i) {
    std::cout << "msg.small[" << i
              << "] = " << (int)root_msg->small()->Get(i)->var_0() << ":"
              << (int)root_msg->small()->Get(i)->var_1() << std::endl;
  }

  Verifier verifier(builder.GetBufferPointer(), builder.GetSize());
  TEST_ASSERT(VerifyBadAlignmentRootBuffer(verifier));
}

}  // namespace tests
}  // namespace flatbuffers
