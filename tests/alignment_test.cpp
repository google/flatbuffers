#include "alignment_test.h"

#include "tests/alignment_test_generated.h"
#include "flatbuffers/flatbuffer_builder.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {

void AlignmentTest() {
  FlatBufferBuilder builder;

  BadAlignmentLarge large;
  Offset<OuterLarge> outer_large = CreateOuterLarge(builder, &large);

  BadAlignmentSmall *small;
  Offset<Vector<const BadAlignmentSmall *>> small_offset =
      builder.CreateUninitializedVectorOfStructs(9, &small);
  (void)small;  // We do not have to write data to trigger the test failure

  Offset<BadAlignmentRoot> root =
      CreateBadAlignmentRoot(builder, outer_large, small_offset);

  builder.Finish(root);

  Verifier verifier(builder.GetBufferPointer(), builder.GetSize());
  TEST_ASSERT(VerifyBadAlignmentRootBuffer(verifier));
}

}  // namespace tests
}  // namespace flatbuffers
