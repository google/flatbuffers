#ifndef TEST_BUILDER_H
#define TEST_BUILDER_H

#include <set>
#include "monster_test_generated.h"
#include "flatbuffers/flatbuffers.h"
#include "test_assert.h"

using namespace MyGame::Example;
namespace flatbuffers {
namespace grpc {
class MessageBuilder;
}
}

extern const std::string m1_name;
extern const Color m1_color;
extern const std::string m2_name;
extern const Color m2_color;

flatbuffers::Offset<Monster> populate1(flatbuffers::FlatBufferBuilder &builder);
flatbuffers::Offset<Monster> populate2(flatbuffers::FlatBufferBuilder &builder);

uint8_t *release_raw_base(flatbuffers::FlatBufferBuilder &fbb, size_t &size, size_t &offset);

void free_raw(flatbuffers::grpc::MessageBuilder &mbb, uint8_t *buf);
void free_raw(flatbuffers::FlatBufferBuilder &fbb, uint8_t *buf);

bool verify(const flatbuffers::DetachedBuffer &buf, const std::string &expected_name, Color color);
bool verify(const uint8_t *buf, size_t offset, const std::string &expected_name, Color color);

bool release_n_verify(flatbuffers::FlatBufferBuilder &fbb, const std::string &expected_name, Color color);
bool release_n_verify(flatbuffers::grpc::MessageBuilder &mbb, const std::string &expected_name, Color color);

template <class Builder>
struct BuilderTests {
  static void empty_builder_movector_test() {
    Builder b1;
    size_t b1_size = b1.GetSize();
    Builder b2(std::move(b1));
    size_t b2_size = b2.GetSize();
    TEST_EQ_FUNC(b1_size, 0);
    TEST_EQ_FUNC(b1_size, b2_size);
  }

  static void nonempty_builder_movector_test() {
    Builder b1;
    populate1(b1);
    size_t b1_size = b1.GetSize();
    Builder b2(std::move(b1));
    TEST_EQ_FUNC(b1_size, b2.GetSize());
    TEST_EQ_FUNC(b1.GetSize(), 0);
  }

  static void builder_movector_before_finish_test() {
    Builder b1;
    auto root_offset1 = populate1(b1);
    Builder b2(std::move(b1));
    b2.Finish(root_offset1);
    TEST_ASSERT_FUNC(release_n_verify(b2, m1_name, m1_color));
    TEST_EQ_FUNC(b1.GetSize(), 0);
  }

  static void builder_movector_after_finish_test() {
    Builder b1;
    auto root_offset1 = populate1(b1);
    b1.Finish(root_offset1);
    auto b1_size = b1.GetSize();
    Builder b2(std::move(b1));
    TEST_EQ_FUNC(b2.GetSize(), b1_size);
    TEST_ASSERT_FUNC(release_n_verify(b2, m1_name, m1_color));
    TEST_EQ_FUNC(b1.GetSize(), 0);
  }

  static void builder_move_assign_before_finish_test() {
    Builder b1;
    auto root_offset1 = populate1(b1);
    Builder b2;
    populate2(b2);
    b2 = std::move(b1);
    b2.Finish(root_offset1);
    TEST_ASSERT_FUNC(release_n_verify(b2, m1_name, m1_color));
    TEST_EQ_FUNC(b1.GetSize(), 0);
  }

  static void builder_move_assign_after_finish_test() {
    Builder b1;
    auto root_offset1 = populate1(b1);
    b1.Finish(root_offset1);
    auto b1_size = b1.GetSize();
    Builder b2;
    auto root_offset2 = populate2(b2);
    b2.Finish(root_offset2);
    b2 = std::move(b1);
    TEST_EQ_FUNC(b2.GetSize(), b1_size);
    TEST_ASSERT_FUNC(release_n_verify(b2, m1_name, m1_color));
    TEST_EQ_FUNC(b1.GetSize(), 0);
  }

  static void builder_swap_before_finish_test() {
    Builder b1;
    auto root_offset1 = populate1(b1);
    auto size1 = b1.GetSize();
    Builder b2;
    auto root_offset2 = populate2(b2);
    auto size2 = b2.GetSize();
    b1.Swap(b2);
    b1.Finish(root_offset2);
    b2.Finish(root_offset1);
    TEST_EQ_FUNC(b1.GetSize() > size2, true);
    TEST_EQ_FUNC(b2.GetSize() > size1, true);
    TEST_ASSERT_FUNC(release_n_verify(b1, m2_name, m2_color));
    TEST_ASSERT_FUNC(release_n_verify(b2, m1_name, m1_color));
  }

  static void builder_swap_after_finish_test() {
    Builder b1;
    auto root_offset1 = populate1(b1);
    b1.Finish(root_offset1);
    auto size1 = b1.GetSize();
    Builder b2;
    auto root_offset2 = populate2(b2);
    b2.Finish(root_offset2);
    auto size2 = b2.GetSize();
    b1.Swap(b2);
    TEST_EQ_FUNC(b1.GetSize(), size2);
    TEST_EQ_FUNC(b2.GetSize(), size1);
    TEST_ASSERT_FUNC(release_n_verify(b1, m2_name, m2_color));
    TEST_ASSERT_FUNC(release_n_verify(b2, m1_name, m1_color));
  }

  static void builder_move_assign_after_release_test() {
    Builder b1;
    auto root_offset1 = populate1(b1);
    b1.Finish(root_offset1);
    {
      flatbuffers::DetachedBuffer b1_detached = b1.Release();
      // detached buffer is deleted
    }
    Builder b2;
    auto root_offset2 = populate2(b2);
    b2.Finish(root_offset2);
    auto b2_size = b2.GetSize();
    // Move into a released builder.
    b1 = std::move(b2);
    TEST_EQ_FUNC(b1.GetSize(), b2_size);
    TEST_ASSERT_FUNC(release_n_verify(b1, m2_name, m2_color));
    TEST_EQ_FUNC(b2.GetSize(), 0);
  }

  static void builder_move_assign_after_releaseraw_test() {
    Builder b1;
    auto root_offset1 = populate1(b1);
    b1.Finish(root_offset1);
    size_t size, offset;
    uint8_t *buf = release_raw_base(b1, size, offset);
    TEST_ASSERT_FUNC(verify(buf, offset, m1_name, m1_color));
    free_raw(b1, buf);
    Builder b2;
    auto root_offset2 = populate2(b2);
    b2.Finish(root_offset2);
    auto b2_size = b2.GetSize();
    // Move into a released builder.
    b1 = std::move(b2);
    TEST_EQ_FUNC(b1.GetSize(), b2_size);
    TEST_ASSERT_FUNC(release_n_verify(b1, m2_name, m2_color));
    TEST_EQ_FUNC(b2.GetSize(), 0);
  }

  static void all_tests() {
    empty_builder_movector_test();
    nonempty_builder_movector_test();
    builder_movector_before_finish_test();
    builder_movector_after_finish_test();
    builder_move_assign_before_finish_test();
    builder_move_assign_after_finish_test();
    builder_swap_before_finish_test();
    builder_swap_after_finish_test();
    builder_move_assign_after_release_test();
    builder_move_assign_after_releaseraw_test();
  }
};

enum BuilderReuseTestSelector {
  REUSABLE_AFTER_RELEASE = 1,
  REUSABLE_AFTER_RELEASE_RAW = 2,
  REUSABLE_AFTER_RELEASE_MESSAGE = 3,
  REUSABLE_AFTER_RELEASE_AND_MOVE_ASSIGN = 4,
  REUSABLE_AFTER_RELEASE_RAW_AND_MOVE_ASSIGN = 5,
  REUSABLE_AFTER_RELEASE_MESSAGE_AND_MOVE_ASSIGN = 6
};

typedef std::set<BuilderReuseTestSelector> TestSelector;

template <class Builder>
struct BuilderReuseTests {
  static void builder_reusable_after_release_test(TestSelector selector) {
    if (!selector.count(REUSABLE_AFTER_RELEASE)) {
      return;
    }

    Builder b1;
    std::vector<flatbuffers::DetachedBuffer> buffers;
    for (int i = 0; i < 5; ++i) {
      auto root_offset1 = populate1(b1);
      b1.Finish(root_offset1);
      buffers.push_back(b1.Release());
      TEST_ASSERT_FUNC(verify(buffers[i], m1_name, m1_color));
    }
  }

  static void builder_reusable_after_releaseraw_test(TestSelector selector) {
    if (!selector.count(REUSABLE_AFTER_RELEASE_RAW)) {
      return;
    }

    Builder b1;
    for (int i = 0; i < 5; ++i) {
      auto root_offset1 = populate1(b1);
      b1.Finish(root_offset1);
      size_t size, offset;
      uint8_t *buf = release_raw_base(b1, size, offset);
      TEST_ASSERT_FUNC(verify(buf, offset, m1_name, m1_color));
      free_raw(b1, buf);
    }
  }

  static void builder_reusable_after_release_and_move_assign_test(TestSelector selector) {
    if (!selector.count(REUSABLE_AFTER_RELEASE_AND_MOVE_ASSIGN)) {
      return;
    }

    Builder b1;
    std::vector<flatbuffers::DetachedBuffer> buffers;
    for (int i = 0; i < 5; ++i) {
      auto root_offset1 = populate1(b1);
      b1.Finish(root_offset1);
      buffers.push_back(b1.Release());
      TEST_ASSERT_FUNC(verify(buffers[i], m1_name, m1_color));
      Builder b2;
      b1 = std::move(b2);
      TEST_EQ_FUNC(b2.GetSize(), 0);
    }
  }

  static void builder_reusable_after_releaseraw_and_move_assign_test(TestSelector selector) {
    if (!selector.count(REUSABLE_AFTER_RELEASE_RAW_AND_MOVE_ASSIGN)) {
      return;
    }

    Builder b1;
    for (int i = 0; i < 5; ++i) {
      auto root_offset1 = populate1(b1);
      b1.Finish(root_offset1);
      size_t size, offset;
      uint8_t *buf = release_raw_base(b1, size, offset);
      TEST_ASSERT_FUNC(verify(buf, offset, m1_name, m1_color));
      free_raw(b1, buf);
      Builder b2;
      b1 = std::move(b2);
      TEST_EQ_FUNC(b2.GetSize(), 0);
    }
  }

  static void run_tests(TestSelector selector) {
    builder_reusable_after_release_test(selector);
    builder_reusable_after_releaseraw_test(selector);
    builder_reusable_after_release_and_move_assign_test(selector);
    builder_reusable_after_releaseraw_and_move_assign_test(selector);
  }
};

#endif // TEST_BUILDER_H
