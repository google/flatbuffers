#include "flatbuffers/grpc.h"
#include "monster_test_generated.h"
#include "test_assert.h"
#include "test_builder.h"

bool verify(flatbuffers::grpc::Message<Monster> &msg, const std::string &expected_name, Color color) {
  const Monster *monster = msg.GetRoot();
  return (monster->name()->str() == expected_name) && (monster->color() == color);
}

bool release_n_verify(flatbuffers::grpc::MessageBuilder &mbb, const std::string &expected_name, Color color) {
  flatbuffers::grpc::Message<Monster> msg = mbb.ReleaseMessage<Monster>();
  const Monster *monster = msg.GetRoot();
  return (monster->name()->str() == expected_name) && (monster->color() == color);
}

template <>
struct BuilderReuseTests<flatbuffers::grpc::MessageBuilder> {
  static void builder_reusable_after_release_message_test(TestSelector selector) {
    if (!selector.count(REUSABLE_AFTER_RELEASE_MESSAGE)) {
      return;
    }

    flatbuffers::grpc::MessageBuilder b1;
    std::vector<flatbuffers::grpc::Message<Monster>> buffers;
    for (int i = 0; i < 5; ++i) {
      auto root_offset1 = populate1(b1);
      b1.Finish(root_offset1);
      buffers.push_back(b1.ReleaseMessage<Monster>());
      TEST_ASSERT_FUNC(verify(buffers[i], m1_name, m1_color));
    }
  }

  static void builder_reusable_after_release_test(TestSelector selector) {
    if (!selector.count(REUSABLE_AFTER_RELEASE)) {
      return;
    }

    // FIXME: Populate-Release loop fails assert(GRPC_SLICE_IS_EMPTY(slice_)).

    flatbuffers::grpc::MessageBuilder b1;
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

    flatbuffers::grpc::MessageBuilder b1;
    for (int i = 0; i < 5; ++i) {
      auto root_offset1 = populate1(b1);
      b1.Finish(root_offset1);
      size_t size, offset;
      grpc_slice slice;
      const uint8_t *buf = b1.ReleaseRaw(size, offset, slice);
      TEST_ASSERT_FUNC(verify(buf, offset, m1_name, m1_color));
      grpc_slice_unref(slice);
    }
  }

  static void builder_reusable_after_release_and_move_assign_test(TestSelector selector) {
    if (!selector.count(REUSABLE_AFTER_RELEASE_AND_MOVE_ASSIGN)) {
      return;
    }

    // FIXME: Release-move_assign loop fails assert(p == GRPC_SLICE_START_PTR(slice_)).

    flatbuffers::grpc::MessageBuilder b1;
    std::vector<flatbuffers::DetachedBuffer> buffers;

    for (int i = 0; i < 1; ++i) {
      auto root_offset1 = populate1(b1);
      b1.Finish(root_offset1);
      buffers.push_back(b1.Release());
      TEST_ASSERT_FUNC(verify(buffers[i], m1_name, m1_color));

      // bring b1 back to life.
      flatbuffers::grpc::MessageBuilder b2;
      b1 = std::move(b2);
      TEST_EQ_FUNC(b1.GetSize(), 0);
      TEST_EQ_FUNC(b2.GetSize(), 0);
    }
  }

  static void builder_reusable_after_release_message_and_move_assign_test(TestSelector selector) {
    if (!selector.count(REUSABLE_AFTER_RELEASE_MESSAGE_AND_MOVE_ASSIGN)) {
      return;
    }

    flatbuffers::grpc::MessageBuilder b1;
    std::vector<flatbuffers::grpc::Message<Monster>> buffers;

    for (int i = 0; i < 5; ++i) {
      auto root_offset1 = populate1(b1);
      b1.Finish(root_offset1);
      buffers.push_back(b1.ReleaseMessage<Monster>());
      TEST_ASSERT_FUNC(verify(buffers[i], m1_name, m1_color));

      // bring b1 back to life.
      flatbuffers::grpc::MessageBuilder b2;
      b1 = std::move(b2);
      TEST_EQ_FUNC(b1.GetSize(), 0);
      TEST_EQ_FUNC(b2.GetSize(), 0);
    }
  }

  static void builder_reusable_after_releaseraw_and_move_assign_test(TestSelector selector) {
    if (!selector.count(REUSABLE_AFTER_RELEASE_RAW_AND_MOVE_ASSIGN)) {
      return;
    }

    flatbuffers::grpc::MessageBuilder b1;
    for (int i = 0; i < 5; ++i) {
      auto root_offset1 = populate1(b1);
      b1.Finish(root_offset1);
      size_t size, offset;
      grpc_slice slice = grpc_empty_slice();
      const uint8_t *buf = b1.ReleaseRaw(size, offset, slice);
      TEST_ASSERT_FUNC(verify(buf, offset, m1_name, m1_color));
      grpc_slice_unref(slice);

      flatbuffers::grpc::MessageBuilder b2;
      b1 = std::move(b2); 
      TEST_EQ_FUNC(b1.GetSize(), 0);
      TEST_EQ_FUNC(b2.GetSize(), 0);
    }
  }

  static void run_tests(TestSelector selector) {
    builder_reusable_after_release_test(selector);
    builder_reusable_after_release_message_test(selector);
    builder_reusable_after_releaseraw_test(selector);
    builder_reusable_after_release_and_move_assign_test(selector);
    builder_reusable_after_releaseraw_and_move_assign_test(selector);
    builder_reusable_after_release_message_and_move_assign_test(selector);
  }
};

void slice_allocator_tests() {
  // move-construct no-delete test
  {
    size_t size = 2048;
    flatbuffers::grpc::SliceAllocator sa1;
    uint8_t *buf = sa1.allocate(size);
    TEST_ASSERT_FUNC(buf != 0);
    buf[0] = 100;
    buf[size-1] = 200;
    flatbuffers::grpc::SliceAllocator sa2(std::move(sa1));
    // buf should be deleted after move-construct
    TEST_EQ_FUNC(buf[0], 100);
    TEST_EQ_FUNC(buf[size-1], 200);
    // buf is freed here
  }

  // move-assign test
  {
    flatbuffers::grpc::SliceAllocator sa1, sa2;
    uint8_t *buf = sa1.allocate(2048);
    sa1 = std::move(sa2);
    // sa1 deletes previously allocated memory in move-assign.
    // So buf is no longer usable here.
    TEST_ASSERT_FUNC(buf != 0);
  }
}

void message_builder_tests() {
  slice_allocator_tests();
  BuilderTests<flatbuffers::grpc::MessageBuilder>::all_tests();

  BuilderReuseTestSelector tests[6] = {
    // REUSABLE_AFTER_RELEASE,                 // Assertion failed: (GRPC_SLICE_IS_EMPTY(slice_))
    // REUSABLE_AFTER_RELEASE_AND_MOVE_ASSIGN, // Assertion failed: (p == GRPC_SLICE_START_PTR(slice_)

    REUSABLE_AFTER_RELEASE_RAW,
    REUSABLE_AFTER_RELEASE_MESSAGE,
    REUSABLE_AFTER_RELEASE_MESSAGE_AND_MOVE_ASSIGN,
    REUSABLE_AFTER_RELEASE_RAW_AND_MOVE_ASSIGN
  };

  BuilderReuseTests<flatbuffers::grpc::MessageBuilder>::run_tests(TestSelector(tests, tests+6));
}
