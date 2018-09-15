#include "flatbuffers/grpc.h"
#include "monster_test_generated.h"

static int builder_test_error = 0;

#define test_assert(condition) do { \
  if(!(condition)) { \
    fprintf(stderr, "%s:%d: %s failed.\n", __FILE__, __LINE__, #condition);\
    builder_test_error = 1;\
  } \
} while(0)

using namespace MyGame::Example;

const std::string m1_name = "Cyberdemon";
const Color m1_color = Color_Red;
const std::string m2_name = "Imp";
const Color m2_color = Color_Green;

flatbuffers::Offset<Monster> populate1(flatbuffers::FlatBufferBuilder &builder) {
  auto name_offset = builder.CreateString(m1_name);
  return CreateMonster(builder, nullptr, 0, 0, name_offset, 0, m1_color);
}

flatbuffers::Offset<Monster> populate2(flatbuffers::FlatBufferBuilder &builder) {
  auto name_offset = builder.CreateString(m2_name);
  return CreateMonster(builder, nullptr, 0, 0, name_offset, 0, m2_color);
}

bool release_n_verify(flatbuffers::FlatBufferBuilder &fbb, const std::string &expected_name, Color color) {
  flatbuffers::DetachedBuffer buf = fbb.Release();
  const Monster *monster = flatbuffers::GetRoot<Monster>(buf.data());
  return (monster->name()->str() == expected_name) && (monster->color() == color);
}

bool release_n_verify(flatbuffers::grpc::MessageBuilder &mbb, const std::string &expected_name, Color color) {
  flatbuffers::grpc::Message<Monster> msg = mbb.ReleaseMessage<Monster>();
  const Monster *monster = msg.GetRoot();
  return (monster->name()->str() == expected_name) && (monster->color() == color);
}

struct OwnedAllocator : public flatbuffers::DefaultAllocator {};

struct TestHeapMessageBuilder : public flatbuffers::FlatBufferBuilder {
  TestHeapMessageBuilder()
    : flatbuffers::FlatBufferBuilder(2048, new OwnedAllocator(), true) {}
};

template <class Builder>
struct BuilderTests {
  static void empty_builder_movector_test() {
    Builder b1;
    size_t b1_size = b1.GetSize();
    Builder b2(std::move(b1));
    size_t b2_size = b2.GetSize();
    test_assert(b1_size == 0);
    test_assert(b1_size == b2_size);
  }

  static void nonempty_builder_movector_test() {
    Builder b1;
    populate1(b1);
    size_t b1_size = b1.GetSize();
    Builder b2(std::move(b1));
    test_assert(b1_size == b2.GetSize());
    test_assert(0 == b1.GetSize());
  }

  static void builder_movector_before_finish_test() {
    Builder b1;
    auto root_offset1 = populate1(b1);
    Builder b2(std::move(b1));
    b2.Finish(root_offset1);
    test_assert(release_n_verify(b2, m1_name, m1_color));
    test_assert(0 == b1.GetSize());
  }

  static void builder_movector_after_finish_test() {
    Builder b1;
    auto root_offset1 = populate1(b1);
    b1.Finish(root_offset1);
    Builder b2(std::move(b1));
    test_assert(release_n_verify(b2, m1_name, m1_color));
    test_assert(0 == b1.GetSize());
  }

  static void builder_move_assign_before_finish_test() {
    Builder b1;
    auto root_offset1 = populate1(b1);
    Builder b2;
    populate2(b2);
    b2 = std::move(b1);
    b2.Finish(root_offset1);
    test_assert(release_n_verify(b2, m1_name, m1_color));
    test_assert(0 == b1.GetSize());
  }

  static void builder_move_assign_after_finish_test() {
    Builder b1;
    auto root_offset1 = populate1(b1);
    b1.Finish(root_offset1);
    Builder b2;
    auto root_offset2 = populate2(b2);
    b2.Finish(root_offset2);
    b2 = std::move(b1);
    test_assert(release_n_verify(b2, m1_name, m1_color));
    test_assert(0 == b1.GetSize());
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
    test_assert(b1.GetSize() > size2);
    test_assert(b2.GetSize() > size1);
    test_assert(release_n_verify(b1, m2_name, m2_color));
    test_assert(release_n_verify(b2, m1_name, m1_color));
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
    test_assert(b1.GetSize() == size2);
    test_assert(b2.GetSize() == size1);
    test_assert(release_n_verify(b1, m2_name, m2_color));
    test_assert(release_n_verify(b2, m1_name, m1_color));
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
  }
};

int builder_tests() {
  BuilderTests<flatbuffers::grpc::MessageBuilder>::all_tests();
  BuilderTests<flatbuffers::FlatBufferBuilder>::all_tests();
  BuilderTests<TestHeapMessageBuilder>::all_tests();
  return builder_test_error;
}
