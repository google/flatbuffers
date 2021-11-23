#include "flatbuffers/vector_downward.h"

#include <gtest/gtest.h>

#include "flatbuffers/base.h"
#include "flatbuffers/buffer.h"

namespace {

using namespace flatbuffers;

class VectorDownwardTest : public testing::Test {};

TEST_F(VectorDownwardTest, InitialSettings) {
  vector_downward vec{ 1024, nullptr, false, AlignOf<largest_scalar_t>() };

  EXPECT_EQ(vec.capacity(), 0);
  EXPECT_EQ(vec.size(), 0);
  EXPECT_EQ(vec.scratch_size(), 0);
}

TEST_F(VectorDownwardTest, Resizes) {
  vector_downward vec{ 1024, nullptr, false, AlignOf<largest_scalar_t>() };

  EXPECT_EQ(vec.capacity(), 0);
  vec.make_space(2048);
  EXPECT_EQ(vec.capacity(), 2048);
}

}  // namespace