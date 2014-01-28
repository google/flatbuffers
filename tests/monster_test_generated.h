// automatically generated, do not modify

#include "flatbuffers/flatbuffers.h"

namespace MyGame {
namespace Example {

enum {
  Color_Red = 0,
  Color_Green = 1,
  Color_Blue = 2,
};

inline const char **EnumNamesColor() {
  static const char *names[] = { "Red", "Green", "Blue", nullptr };
  return names;
}

inline const char *EnumNameColor(int e) { return EnumNamesColor()[e]; }

enum {
  Any_NONE = 0,
  Any_Monster = 1,
};

inline const char **EnumNamesAny() {
  static const char *names[] = { "NONE", "Monster", nullptr };
  return names;
}

inline const char *EnumNameAny(int e) { return EnumNamesAny()[e]; }

struct Test;
struct Vec3;
struct Monster;

MANUALLY_ALIGNED_STRUCT(2) Test {
 private:
  int16_t a_;
  int8_t b_;
  int8_t __padding0;

 public:
  Test(int16_t a, int8_t b)
    : a_(flatbuffers::EndianScalar(a)), b_(flatbuffers::EndianScalar(b)), __padding0(0) {}

  int16_t a() const { return flatbuffers::EndianScalar(a_); }
  int8_t b() const { return flatbuffers::EndianScalar(b_); }
};
STRUCT_END(Test, 4);

MANUALLY_ALIGNED_STRUCT(16) Vec3 {
 private:
  float x_;
  float y_;
  float z_;
  int32_t __padding0;
  double test1_;
  int8_t test2_;
  int8_t __padding1;
  Test test3_;
  int16_t __padding2;

 public:
  Vec3(float x, float y, float z, double test1, int8_t test2, const Test &test3)
    : x_(flatbuffers::EndianScalar(x)), y_(flatbuffers::EndianScalar(y)), z_(flatbuffers::EndianScalar(z)), __padding0(0), test1_(flatbuffers::EndianScalar(test1)), test2_(flatbuffers::EndianScalar(test2)), __padding1(0), test3_(test3), __padding2(0) {}

  float x() const { return flatbuffers::EndianScalar(x_); }
  float y() const { return flatbuffers::EndianScalar(y_); }
  float z() const { return flatbuffers::EndianScalar(z_); }
  double test1() const { return flatbuffers::EndianScalar(test1_); }
  int8_t test2() const { return flatbuffers::EndianScalar(test2_); }
  const Test &test3() const { return test3_; }
};
STRUCT_END(Vec3, 32);

struct Monster : private flatbuffers::Table {
  const Vec3 *pos() const { return GetStruct<const Vec3 *>(4); }
  int16_t mana() const { return GetField<int16_t>(6, 150); }
  int16_t hp() const { return GetField<int16_t>(8, 100); }
  const flatbuffers::String *name() const { return GetPointer<const flatbuffers::String *>(10); }
  const flatbuffers::Vector<uint8_t> *inventory() const { return GetPointer<const flatbuffers::Vector<uint8_t> *>(14); }
  /// an example documentation comment: this will end up in the generated code multiline too
  int8_t color() const { return GetField<int8_t>(16, 2); }
  uint8_t test_type() const { return GetField<uint8_t>(18, 0); }
  const void *test() const { return GetPointer<const void *>(20); }
  const flatbuffers::Vector<const Test *> *test4() const { return GetPointer<const flatbuffers::Vector<const Test *> *>(22); }
};

struct MonsterBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_pos(const Vec3 *pos) { fbb_.AddStruct(4, pos); }
  void add_mana(int16_t mana) { fbb_.AddElement<int16_t>(6, mana, 150); }
  void add_hp(int16_t hp) { fbb_.AddElement<int16_t>(8, hp, 100); }
  void add_name(flatbuffers::Offset<flatbuffers::String> name) { fbb_.AddOffset(10, name); }
  void add_inventory(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> inventory) { fbb_.AddOffset(14, inventory); }
  void add_color(int8_t color) { fbb_.AddElement<int8_t>(16, color, 2); }
  void add_test_type(uint8_t test_type) { fbb_.AddElement<uint8_t>(18, test_type, 0); }
  void add_test(flatbuffers::Offset<void> test) { fbb_.AddOffset(20, test); }
  void add_test4(flatbuffers::Offset<flatbuffers::Vector<const Test *>> test4) { fbb_.AddOffset(22, test4); }
  MonsterBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  flatbuffers::Offset<Monster> Finish() { return flatbuffers::Offset<Monster>(fbb_.EndTable(start_, 10)); }
};

inline flatbuffers::Offset<Monster> CreateMonster(flatbuffers::FlatBufferBuilder &_fbb, const Vec3 *pos, int16_t mana, int16_t hp, flatbuffers::Offset<flatbuffers::String> name, flatbuffers::Offset<flatbuffers::Vector<uint8_t>> inventory, int8_t color, uint8_t test_type, flatbuffers::Offset<void> test, flatbuffers::Offset<flatbuffers::Vector<const Test *>> test4) {
  MonsterBuilder builder_(_fbb);
  builder_.add_test4(test4);
  builder_.add_test(test);
  builder_.add_inventory(inventory);
  builder_.add_name(name);
  builder_.add_pos(pos);
  builder_.add_hp(hp);
  builder_.add_mana(mana);
  builder_.add_test_type(test_type);
  builder_.add_color(color);
  return builder_.Finish();
}

inline const Monster *GetMonster(const void *buf) { return flatbuffers::GetRoot<Monster>(buf); }

}; // namespace MyGame
}; // namespace Example
