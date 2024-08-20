#include "alignment_test.h"

#include "tests/alignment_test_generated.h"
#include "flatbuffers/flatbuffer_builder.h"
#include "flatbuffers/util.h"
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
  TEST_ASSERT(verifier.VerifyBuffer<BadAlignmentRoot>(nullptr));


  // ============= Test Small Structs Vector misalignment ========

  builder.Clear();

  // creating 5 structs with 2 bytes each
  // 10 bytes in total for Vector data is needed
  std::vector<EvenSmallStruct> even_vector = { { 2, 1 }, { 3, 1 }, { 4, 1 } };
  std::vector<OddSmallStruct> odd_vector = { { 6, 5, 4 },
                                             { 9, 8, 7 },
                                             { 1, 2, 3 } };
  // CreateVectorOfStructs is used in the generated CreateSmallStructsDirect()
  // method, but we test it directly
  Offset<Vector<const EvenSmallStruct *>> even_structs_offset =
      builder.CreateVectorOfStructs<EvenSmallStruct>(even_vector);
  Offset<Vector<const OddSmallStruct *>> odd_structs_offset =
      builder.CreateVectorOfStructs<OddSmallStruct>(odd_vector);
  Offset<SmallStructs> small_structs_root =
      CreateSmallStructs(builder, even_structs_offset, odd_structs_offset);

  builder.Finish(small_structs_root);

  // Save the binary that we later can annotate with `flatc --annotate` command
  // NOTE: the conversion of the JSON data to --binary via `flatc --binary`
  //       command is not changed with that fix and was always producing the
  //       correct binary data.
  // SaveFile("alignment_test_{before,after}_fix.bin",
  //          reinterpret_cast<char *>(builder.GetBufferPointer()),
  //          builder.GetSize(), true);

  Verifier verifier_small_structs(builder.GetBufferPointer(),
                                  builder.GetSize());
  TEST_ASSERT(verifier_small_structs.VerifyBuffer<SmallStructs>(nullptr));

  // Reading SmallStructs vector values back and compare with original
  auto root_msg =
      flatbuffers::GetRoot<SmallStructs>(builder.GetBufferPointer());

  TEST_EQ(root_msg->even_structs()->size(), even_vector.size());
  for (flatbuffers::uoffset_t i = 0; i < root_msg->even_structs()->size();
       ++i) {
    TEST_EQ(even_vector[i].var_0(), root_msg->even_structs()->Get(i)->var_0());
    TEST_EQ(even_vector[i].var_1(), root_msg->even_structs()->Get(i)->var_1());
  }

  TEST_EQ(root_msg->odd_structs()->size(), even_vector.size());
  for (flatbuffers::uoffset_t i = 0; i < root_msg->odd_structs()->size(); ++i) {
    TEST_EQ(odd_vector[i].var_0(), root_msg->odd_structs()->Get(i)->var_0());
    TEST_EQ(odd_vector[i].var_1(), root_msg->odd_structs()->Get(i)->var_1());
  }
}

}  // namespace tests
}  // namespace flatbuffers
