// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_MONSTERTEST_MYGAME_EXAMPLE_H_
#define FLATBUFFERS_GENERATED_MONSTERTEST_MYGAME_EXAMPLE_H_

#include "flatbuffers/flatbuffers.h"

namespace MyGame {
namespace Example2 {

struct Monster;
struct MonsterT;

}  // namespace Example2

namespace Example {

struct Test;

struct TestSimpleTableWithEnum;
struct TestSimpleTableWithEnumT;

struct Vec3;

struct Stat;
struct StatT;

struct Monster;
struct MonsterT;

enum Color {
  Color_Red = 1,
  Color_Green = 2,
  Color_Blue = 8,
  Color_NONE = 0,
  Color_ANY = 11
};

inline const char **EnumNamesColor() {
  static const char *names[] = { "Red", "Green", "", "", "", "", "", "Blue", nullptr };
  return names;
}

inline const char *EnumNameColor(Color e) { return EnumNamesColor()[static_cast<int>(e) - static_cast<int>(Color_Red)]; }

enum Any {
  Any_NONE = 0,
  Any_Monster = 1,
  Any_TestSimpleTableWithEnum = 2,
  Any_MyGame_Example2_Monster = 3,
  Any_MIN = Any_NONE,
  Any_MAX = Any_MyGame_Example2_Monster
};

struct AnyUnion {
  Any type;

  flatbuffers::NativeTable *table;
  AnyUnion() : type(Any_NONE), table(nullptr) {}
  AnyUnion(const AnyUnion &);
  AnyUnion &operator=(const AnyUnion &);
  ~AnyUnion();

  void UnPack(const void *union_obj, Any _t);
  flatbuffers::Offset<void> Pack(flatbuffers::FlatBufferBuilder &_fbb) const;

  MonsterT *AsMonster() { return type == Any_Monster ? reinterpret_cast<MonsterT *>(table) : nullptr; }
  AnyUnion &operator=(const MonsterT &_o);
  TestSimpleTableWithEnumT *AsTestSimpleTableWithEnum() { return type == Any_TestSimpleTableWithEnum ? reinterpret_cast<TestSimpleTableWithEnumT *>(table) : nullptr; }
  AnyUnion &operator=(const TestSimpleTableWithEnumT &_o);
  MyGame::Example2::MonsterT *AsMyGame_Example2_Monster() { return type == Any_MyGame_Example2_Monster ? reinterpret_cast<MyGame::Example2::MonsterT *>(table) : nullptr; }
  AnyUnion &operator=(const MyGame::Example2::MonsterT &_o);
};

inline const char **EnumNamesAny() {
  static const char *names[] = { "NONE", "Monster", "TestSimpleTableWithEnum", "MyGame_Example2_Monster", nullptr };
  return names;
}

inline const char *EnumNameAny(Any e) { return EnumNamesAny()[static_cast<int>(e)]; }

inline bool VerifyAny(flatbuffers::Verifier &verifier, const void *union_obj, Any type);

MANUALLY_ALIGNED_STRUCT(2) Test FLATBUFFERS_FINAL_CLASS {
 private:
  int16_t a_;
  int8_t b_;
  int8_t __padding0;

 public:
  Test() { memset(this, 0, sizeof(Test)); }
  Test(const Test &_o) { memcpy(this, &_o, sizeof(Test)); }
  Test(int16_t _a, int8_t _b)
    : a_(flatbuffers::EndianScalar(_a)), b_(flatbuffers::EndianScalar(_b)), __padding0(0) { (void)__padding0; }

  int16_t a() const { return flatbuffers::EndianScalar(a_); }
  void mutate_a(int16_t _a) { flatbuffers::WriteScalar(&a_, _a); }
  int8_t b() const { return flatbuffers::EndianScalar(b_); }
  void mutate_b(int8_t _b) { flatbuffers::WriteScalar(&b_, _b); }
};
STRUCT_END(Test, 4);

MANUALLY_ALIGNED_STRUCT(16) Vec3 FLATBUFFERS_FINAL_CLASS {
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
  Vec3() { memset(this, 0, sizeof(Vec3)); }
  Vec3(const Vec3 &_o) { memcpy(this, &_o, sizeof(Vec3)); }
  Vec3(float _x, float _y, float _z, double _test1, Color _test2, const Test &_test3)
    : x_(flatbuffers::EndianScalar(_x)), y_(flatbuffers::EndianScalar(_y)), z_(flatbuffers::EndianScalar(_z)), __padding0(0), test1_(flatbuffers::EndianScalar(_test1)), test2_(flatbuffers::EndianScalar(static_cast<int8_t>(_test2))), __padding1(0), test3_(_test3), __padding2(0) { (void)__padding0; (void)__padding1; (void)__padding2; }

  float x() const { return flatbuffers::EndianScalar(x_); }
  void mutate_x(float _x) { flatbuffers::WriteScalar(&x_, _x); }
  float y() const { return flatbuffers::EndianScalar(y_); }
  void mutate_y(float _y) { flatbuffers::WriteScalar(&y_, _y); }
  float z() const { return flatbuffers::EndianScalar(z_); }
  void mutate_z(float _z) { flatbuffers::WriteScalar(&z_, _z); }
  double test1() const { return flatbuffers::EndianScalar(test1_); }
  void mutate_test1(double _test1) { flatbuffers::WriteScalar(&test1_, _test1); }
  Color test2() const { return static_cast<Color>(flatbuffers::EndianScalar(test2_)); }
  void mutate_test2(Color _test2) { flatbuffers::WriteScalar(&test2_, static_cast<int8_t>(_test2)); }
  const Test &test3() const { return test3_; }
  Test &mutable_test3() { return test3_; }
};
STRUCT_END(Vec3, 32);

}  // namespace Example

namespace Example2 {

struct Monster FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           verifier.EndTable();
  }
};

struct MonsterT : public flatbuffers::NativeTable {
  flatbuffers::Offset<Monster> Pack(flatbuffers::FlatBufferBuilder &_fbb) const;
  void UnPack(const Monster *object);
  inline MonsterT& operator=(const Monster *object) { UnPack(object); return *this;}
  explicit MonsterT(const Monster *object) { UnPack(object); }

  MonsterT() {}

};

struct MonsterBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  MonsterBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  MonsterBuilder &operator=(const MonsterBuilder &);
  flatbuffers::Offset<Monster> Finish() {
    auto o = flatbuffers::Offset<Monster>(fbb_.EndTable(start_, 0));
    return o;
  }
};

inline flatbuffers::Offset<Monster> CreateMonster(flatbuffers::FlatBufferBuilder &_fbb) {
  MonsterBuilder builder_(_fbb);
  return builder_.Finish();
}

}  // namespace Example2

namespace Example {

struct TestSimpleTableWithEnum FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_COLOR = 4
  };
  Color color() const { return static_cast<Color>(GetField<int8_t>(VT_COLOR, 2)); }
  bool mutate_color(Color _color) { return SetField(VT_COLOR, static_cast<int8_t>(_color)); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_COLOR) &&
           verifier.EndTable();
  }
};

struct TestSimpleTableWithEnumT : public flatbuffers::NativeTable {
  flatbuffers::Offset<TestSimpleTableWithEnum> Pack(flatbuffers::FlatBufferBuilder &_fbb) const;
  void UnPack(const TestSimpleTableWithEnum *object);
  inline TestSimpleTableWithEnumT& operator=(const TestSimpleTableWithEnum *object) { UnPack(object); return *this;}
  explicit TestSimpleTableWithEnumT(const TestSimpleTableWithEnum *object) { UnPack(object); }

  Color color;
  TestSimpleTableWithEnumT()
    : color(Color_Green) {}

};

struct TestSimpleTableWithEnumBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_color(Color color) { fbb_.AddElement<int8_t>(TestSimpleTableWithEnum::VT_COLOR, static_cast<int8_t>(color), 2); }
  TestSimpleTableWithEnumBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  TestSimpleTableWithEnumBuilder &operator=(const TestSimpleTableWithEnumBuilder &);
  flatbuffers::Offset<TestSimpleTableWithEnum> Finish() {
    auto o = flatbuffers::Offset<TestSimpleTableWithEnum>(fbb_.EndTable(start_, 1));
    return o;
  }
};

inline flatbuffers::Offset<TestSimpleTableWithEnum> CreateTestSimpleTableWithEnum(flatbuffers::FlatBufferBuilder &_fbb,
    Color color = Color_Green) {
  TestSimpleTableWithEnumBuilder builder_(_fbb);
  builder_.add_color(color);
  return builder_.Finish();
}

struct Stat FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_ID = 4,
    VT_VAL = 6,
    VT_COUNT = 8
  };
  const flatbuffers::String *id() const { return GetPointer<const flatbuffers::String *>(VT_ID); }
  flatbuffers::String *mutable_id() { return GetPointer<flatbuffers::String *>(VT_ID); }
  int64_t val() const { return GetField<int64_t>(VT_VAL, 0); }
  bool mutate_val(int64_t _val) { return SetField(VT_VAL, _val); }
  uint16_t count() const { return GetField<uint16_t>(VT_COUNT, 0); }
  bool mutate_count(uint16_t _count) { return SetField(VT_COUNT, _count); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_ID) &&
           verifier.Verify(id()) &&
           VerifyField<int64_t>(verifier, VT_VAL) &&
           VerifyField<uint16_t>(verifier, VT_COUNT) &&
           verifier.EndTable();
  }
};

struct StatT : public flatbuffers::NativeTable {
  flatbuffers::Offset<Stat> Pack(flatbuffers::FlatBufferBuilder &_fbb) const;
  void UnPack(const Stat *object);
  inline StatT& operator=(const Stat *object) { UnPack(object); return *this;}
  explicit StatT(const Stat *object) { UnPack(object); }

  std::string id;
  int64_t val;
  uint16_t count;
  StatT()
    : val(0)
    , count(0) {}

};

struct StatBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_id(flatbuffers::Offset<flatbuffers::String> id) { fbb_.AddOffset(Stat::VT_ID, id); }
  void add_val(int64_t val) { fbb_.AddElement<int64_t>(Stat::VT_VAL, val, 0); }
  void add_count(uint16_t count) { fbb_.AddElement<uint16_t>(Stat::VT_COUNT, count, 0); }
  StatBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  StatBuilder &operator=(const StatBuilder &);
  flatbuffers::Offset<Stat> Finish() {
    auto o = flatbuffers::Offset<Stat>(fbb_.EndTable(start_, 3));
    return o;
  }
};

inline flatbuffers::Offset<Stat> CreateStat(flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> id = 0,
    int64_t val = 0,
    uint16_t count = 0) {
  StatBuilder builder_(_fbb);
  builder_.add_val(val);
  builder_.add_id(id);
  builder_.add_count(count);
  return builder_.Finish();
}

inline flatbuffers::Offset<Stat> CreateStat(flatbuffers::FlatBufferBuilder &_fbb,
    const char *id = nullptr,
    int64_t val = 0,
    uint16_t count = 0) {
  return CreateStat(_fbb, id ? 0 : _fbb.CreateString(id), val, count);
}

/// an example documentation comment: monster object
struct Monster FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_POS = 4,
    VT_MANA = 6,
    VT_HP = 8,
    VT_NAME = 10,
    VT_INVENTORY = 14,
    VT_COLOR = 16,
    VT_TEST_TYPE = 18,
    VT_TEST = 20,
    VT_TEST4 = 22,
    VT_TESTARRAYOFSTRING = 24,
    VT_TESTARRAYOFTABLES = 26,
    VT_ENEMY = 28,
    VT_TESTNESTEDFLATBUFFER = 30,
    VT_TESTEMPTY = 32,
    VT_TESTBOOL = 34,
    VT_TESTHASHS32_FNV1 = 36,
    VT_TESTHASHU32_FNV1 = 38,
    VT_TESTHASHS64_FNV1 = 40,
    VT_TESTHASHU64_FNV1 = 42,
    VT_TESTHASHS32_FNV1A = 44,
    VT_TESTHASHU32_FNV1A = 46,
    VT_TESTHASHS64_FNV1A = 48,
    VT_TESTHASHU64_FNV1A = 50,
    VT_TESTARRAYOFBOOLS = 52,
    VT_TESTF = 54,
    VT_TESTF2 = 56,
    VT_TESTF3 = 58,
    VT_TESTARRAYOFSTRING2 = 60
  };
  const Vec3 *pos() const { return GetStruct<const Vec3 *>(VT_POS); }
  Vec3 *mutable_pos() { return GetStruct<Vec3 *>(VT_POS); }
  int16_t mana() const { return GetField<int16_t>(VT_MANA, 150); }
  bool mutate_mana(int16_t _mana) { return SetField(VT_MANA, _mana); }
  int16_t hp() const { return GetField<int16_t>(VT_HP, 100); }
  bool mutate_hp(int16_t _hp) { return SetField(VT_HP, _hp); }
  const flatbuffers::String *name() const { return GetPointer<const flatbuffers::String *>(VT_NAME); }
  flatbuffers::String *mutable_name() { return GetPointer<flatbuffers::String *>(VT_NAME); }
  bool KeyCompareLessThan(const Monster *o) const { return *name() < *o->name(); }
  int KeyCompareWithValue(const char *val) const { return strcmp(name()->c_str(), val); }
  const flatbuffers::Vector<uint8_t> *inventory() const { return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_INVENTORY); }
  flatbuffers::Vector<uint8_t> *mutable_inventory() { return GetPointer<flatbuffers::Vector<uint8_t> *>(VT_INVENTORY); }
  Color color() const { return static_cast<Color>(GetField<int8_t>(VT_COLOR, 8)); }
  bool mutate_color(Color _color) { return SetField(VT_COLOR, static_cast<int8_t>(_color)); }
  Any test_type() const { return static_cast<Any>(GetField<uint8_t>(VT_TEST_TYPE, 0)); }
  bool mutate_test_type(Any _test_type) { return SetField(VT_TEST_TYPE, static_cast<uint8_t>(_test_type)); }
  const void *test() const { return GetPointer<const void *>(VT_TEST); }
  void *mutable_test() { return GetPointer<void *>(VT_TEST); }
  const flatbuffers::Vector<const Test *> *test4() const { return GetPointer<const flatbuffers::Vector<const Test *> *>(VT_TEST4); }
  flatbuffers::Vector<const Test *> *mutable_test4() { return GetPointer<flatbuffers::Vector<const Test *> *>(VT_TEST4); }
  const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *testarrayofstring() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(VT_TESTARRAYOFSTRING); }
  flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *mutable_testarrayofstring() { return GetPointer<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(VT_TESTARRAYOFSTRING); }
  /// an example documentation comment: this will end up in the generated code
  /// multiline too
  const flatbuffers::Vector<flatbuffers::Offset<Monster>> *testarrayoftables() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<Monster>> *>(VT_TESTARRAYOFTABLES); }
  flatbuffers::Vector<flatbuffers::Offset<Monster>> *mutable_testarrayoftables() { return GetPointer<flatbuffers::Vector<flatbuffers::Offset<Monster>> *>(VT_TESTARRAYOFTABLES); }
  const Monster *enemy() const { return GetPointer<const Monster *>(VT_ENEMY); }
  Monster *mutable_enemy() { return GetPointer<Monster *>(VT_ENEMY); }
  const flatbuffers::Vector<uint8_t> *testnestedflatbuffer() const { return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_TESTNESTEDFLATBUFFER); }
  flatbuffers::Vector<uint8_t> *mutable_testnestedflatbuffer() { return GetPointer<flatbuffers::Vector<uint8_t> *>(VT_TESTNESTEDFLATBUFFER); }
  const MyGame::Example::Monster *testnestedflatbuffer_nested_root() const { return flatbuffers::GetRoot<MyGame::Example::Monster>(testnestedflatbuffer()->Data()); }
  const Stat *testempty() const { return GetPointer<const Stat *>(VT_TESTEMPTY); }
  Stat *mutable_testempty() { return GetPointer<Stat *>(VT_TESTEMPTY); }
  bool testbool() const { return GetField<uint8_t>(VT_TESTBOOL, 0) != 0; }
  bool mutate_testbool(bool _testbool) { return SetField(VT_TESTBOOL, static_cast<uint8_t>(_testbool)); }
  int32_t testhashs32_fnv1() const { return GetField<int32_t>(VT_TESTHASHS32_FNV1, 0); }
  bool mutate_testhashs32_fnv1(int32_t _testhashs32_fnv1) { return SetField(VT_TESTHASHS32_FNV1, _testhashs32_fnv1); }
  uint32_t testhashu32_fnv1() const { return GetField<uint32_t>(VT_TESTHASHU32_FNV1, 0); }
  bool mutate_testhashu32_fnv1(uint32_t _testhashu32_fnv1) { return SetField(VT_TESTHASHU32_FNV1, _testhashu32_fnv1); }
  int64_t testhashs64_fnv1() const { return GetField<int64_t>(VT_TESTHASHS64_FNV1, 0); }
  bool mutate_testhashs64_fnv1(int64_t _testhashs64_fnv1) { return SetField(VT_TESTHASHS64_FNV1, _testhashs64_fnv1); }
  uint64_t testhashu64_fnv1() const { return GetField<uint64_t>(VT_TESTHASHU64_FNV1, 0); }
  bool mutate_testhashu64_fnv1(uint64_t _testhashu64_fnv1) { return SetField(VT_TESTHASHU64_FNV1, _testhashu64_fnv1); }
  int32_t testhashs32_fnv1a() const { return GetField<int32_t>(VT_TESTHASHS32_FNV1A, 0); }
  bool mutate_testhashs32_fnv1a(int32_t _testhashs32_fnv1a) { return SetField(VT_TESTHASHS32_FNV1A, _testhashs32_fnv1a); }
  uint32_t testhashu32_fnv1a() const { return GetField<uint32_t>(VT_TESTHASHU32_FNV1A, 0); }
  bool mutate_testhashu32_fnv1a(uint32_t _testhashu32_fnv1a) { return SetField(VT_TESTHASHU32_FNV1A, _testhashu32_fnv1a); }
  int64_t testhashs64_fnv1a() const { return GetField<int64_t>(VT_TESTHASHS64_FNV1A, 0); }
  bool mutate_testhashs64_fnv1a(int64_t _testhashs64_fnv1a) { return SetField(VT_TESTHASHS64_FNV1A, _testhashs64_fnv1a); }
  uint64_t testhashu64_fnv1a() const { return GetField<uint64_t>(VT_TESTHASHU64_FNV1A, 0); }
  bool mutate_testhashu64_fnv1a(uint64_t _testhashu64_fnv1a) { return SetField(VT_TESTHASHU64_FNV1A, _testhashu64_fnv1a); }
  const flatbuffers::Vector<uint8_t> *testarrayofbools() const { return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_TESTARRAYOFBOOLS); }
  flatbuffers::Vector<uint8_t> *mutable_testarrayofbools() { return GetPointer<flatbuffers::Vector<uint8_t> *>(VT_TESTARRAYOFBOOLS); }
  float testf() const { return GetField<float>(VT_TESTF, 3.14159f); }
  bool mutate_testf(float _testf) { return SetField(VT_TESTF, _testf); }
  float testf2() const { return GetField<float>(VT_TESTF2, 3.0f); }
  bool mutate_testf2(float _testf2) { return SetField(VT_TESTF2, _testf2); }
  float testf3() const { return GetField<float>(VT_TESTF3, 0.0f); }
  bool mutate_testf3(float _testf3) { return SetField(VT_TESTF3, _testf3); }
  const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *testarrayofstring2() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(VT_TESTARRAYOFSTRING2); }
  flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *mutable_testarrayofstring2() { return GetPointer<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(VT_TESTARRAYOFSTRING2); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<Vec3>(verifier, VT_POS) &&
           VerifyField<int16_t>(verifier, VT_MANA) &&
           VerifyField<int16_t>(verifier, VT_HP) &&
           VerifyFieldRequired<flatbuffers::uoffset_t>(verifier, VT_NAME) &&
           verifier.Verify(name()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_INVENTORY) &&
           verifier.Verify(inventory()) &&
           VerifyField<int8_t>(verifier, VT_COLOR) &&
           VerifyField<uint8_t>(verifier, VT_TEST_TYPE) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_TEST) &&
           VerifyAny(verifier, test(), test_type()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_TEST4) &&
           verifier.Verify(test4()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_TESTARRAYOFSTRING) &&
           verifier.Verify(testarrayofstring()) &&
           verifier.VerifyVectorOfStrings(testarrayofstring()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_TESTARRAYOFTABLES) &&
           verifier.Verify(testarrayoftables()) &&
           verifier.VerifyVectorOfTables(testarrayoftables()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_ENEMY) &&
           verifier.VerifyTable(enemy()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_TESTNESTEDFLATBUFFER) &&
           verifier.Verify(testnestedflatbuffer()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_TESTEMPTY) &&
           verifier.VerifyTable(testempty()) &&
           VerifyField<uint8_t>(verifier, VT_TESTBOOL) &&
           VerifyField<int32_t>(verifier, VT_TESTHASHS32_FNV1) &&
           VerifyField<uint32_t>(verifier, VT_TESTHASHU32_FNV1) &&
           VerifyField<int64_t>(verifier, VT_TESTHASHS64_FNV1) &&
           VerifyField<uint64_t>(verifier, VT_TESTHASHU64_FNV1) &&
           VerifyField<int32_t>(verifier, VT_TESTHASHS32_FNV1A) &&
           VerifyField<uint32_t>(verifier, VT_TESTHASHU32_FNV1A) &&
           VerifyField<int64_t>(verifier, VT_TESTHASHS64_FNV1A) &&
           VerifyField<uint64_t>(verifier, VT_TESTHASHU64_FNV1A) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_TESTARRAYOFBOOLS) &&
           verifier.Verify(testarrayofbools()) &&
           VerifyField<float>(verifier, VT_TESTF) &&
           VerifyField<float>(verifier, VT_TESTF2) &&
           VerifyField<float>(verifier, VT_TESTF3) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_TESTARRAYOFSTRING2) &&
           verifier.Verify(testarrayofstring2()) &&
           verifier.VerifyVectorOfStrings(testarrayofstring2()) &&
           verifier.EndTable();
  }
};

struct MonsterT : public flatbuffers::NativeTable {
  flatbuffers::Offset<Monster> Pack(flatbuffers::FlatBufferBuilder &_fbb) const;
  void UnPack(const Monster *object);
  inline MonsterT& operator=(const Monster *object) { UnPack(object); return *this;}
  explicit MonsterT(const Monster *object) { UnPack(object); }

  flatbuffers::Optional<Vec3> pos;
  int16_t mana;
  int16_t hp;
  std::string name;
  std::vector<uint8_t> inventory;
  Color color;
  AnyUnion test;
  std::vector<Test> test4;
  std::vector<std::string> testarrayofstring;
  std::vector<MonsterT> testarrayoftables;
  flatbuffers::OptionalTable<MonsterT, Monster> enemy;
  std::vector<uint8_t> testnestedflatbuffer;
  flatbuffers::OptionalTable<StatT, Stat> testempty;
  bool testbool;
  int32_t testhashs32_fnv1;
  uint32_t testhashu32_fnv1;
  int64_t testhashs64_fnv1;
  uint64_t testhashu64_fnv1;
  int32_t testhashs32_fnv1a;
  uint32_t testhashu32_fnv1a;
  int64_t testhashs64_fnv1a;
  uint64_t testhashu64_fnv1a;
  std::vector<bool> testarrayofbools;
  float testf;
  float testf2;
  float testf3;
  std::vector<std::string> testarrayofstring2;
  MonsterT()
    : mana(150)
    , hp(100)
    , color(Color_Blue)
    , testbool(false)
    , testhashs32_fnv1(0)
    , testhashu32_fnv1(0)
    , testhashs64_fnv1(0)
    , testhashu64_fnv1(0)
    , testhashs32_fnv1a(0)
    , testhashu32_fnv1a(0)
    , testhashs64_fnv1a(0)
    , testhashu64_fnv1a(0)
    , testf(3.14159f)
    , testf2(3.0f)
    , testf3(0.0f) {}

};

struct MonsterBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_pos(const Vec3 *pos) { fbb_.AddStruct(Monster::VT_POS, pos); }
  void add_mana(int16_t mana) { fbb_.AddElement<int16_t>(Monster::VT_MANA, mana, 150); }
  void add_hp(int16_t hp) { fbb_.AddElement<int16_t>(Monster::VT_HP, hp, 100); }
  void add_name(flatbuffers::Offset<flatbuffers::String> name) { fbb_.AddOffset(Monster::VT_NAME, name); }
  void add_inventory(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> inventory) { fbb_.AddOffset(Monster::VT_INVENTORY, inventory); }
  void add_color(Color color) { fbb_.AddElement<int8_t>(Monster::VT_COLOR, static_cast<int8_t>(color), 8); }
  void add_test_type(Any test_type) { fbb_.AddElement<uint8_t>(Monster::VT_TEST_TYPE, static_cast<uint8_t>(test_type), 0); }
  void add_test(flatbuffers::Offset<void> test) { fbb_.AddOffset(Monster::VT_TEST, test); }
  void add_test4(flatbuffers::Offset<flatbuffers::Vector<const Test *>> test4) { fbb_.AddOffset(Monster::VT_TEST4, test4); }
  void add_testarrayofstring(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> testarrayofstring) { fbb_.AddOffset(Monster::VT_TESTARRAYOFSTRING, testarrayofstring); }
  void add_testarrayoftables(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Monster>>> testarrayoftables) { fbb_.AddOffset(Monster::VT_TESTARRAYOFTABLES, testarrayoftables); }
  void add_enemy(flatbuffers::Offset<Monster> enemy) { fbb_.AddOffset(Monster::VT_ENEMY, enemy); }
  void add_testnestedflatbuffer(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> testnestedflatbuffer) { fbb_.AddOffset(Monster::VT_TESTNESTEDFLATBUFFER, testnestedflatbuffer); }
  void add_testempty(flatbuffers::Offset<Stat> testempty) { fbb_.AddOffset(Monster::VT_TESTEMPTY, testempty); }
  void add_testbool(bool testbool) { fbb_.AddElement<uint8_t>(Monster::VT_TESTBOOL, static_cast<uint8_t>(testbool), 0); }
  void add_testhashs32_fnv1(int32_t testhashs32_fnv1) { fbb_.AddElement<int32_t>(Monster::VT_TESTHASHS32_FNV1, testhashs32_fnv1, 0); }
  void add_testhashu32_fnv1(uint32_t testhashu32_fnv1) { fbb_.AddElement<uint32_t>(Monster::VT_TESTHASHU32_FNV1, testhashu32_fnv1, 0); }
  void add_testhashs64_fnv1(int64_t testhashs64_fnv1) { fbb_.AddElement<int64_t>(Monster::VT_TESTHASHS64_FNV1, testhashs64_fnv1, 0); }
  void add_testhashu64_fnv1(uint64_t testhashu64_fnv1) { fbb_.AddElement<uint64_t>(Monster::VT_TESTHASHU64_FNV1, testhashu64_fnv1, 0); }
  void add_testhashs32_fnv1a(int32_t testhashs32_fnv1a) { fbb_.AddElement<int32_t>(Monster::VT_TESTHASHS32_FNV1A, testhashs32_fnv1a, 0); }
  void add_testhashu32_fnv1a(uint32_t testhashu32_fnv1a) { fbb_.AddElement<uint32_t>(Monster::VT_TESTHASHU32_FNV1A, testhashu32_fnv1a, 0); }
  void add_testhashs64_fnv1a(int64_t testhashs64_fnv1a) { fbb_.AddElement<int64_t>(Monster::VT_TESTHASHS64_FNV1A, testhashs64_fnv1a, 0); }
  void add_testhashu64_fnv1a(uint64_t testhashu64_fnv1a) { fbb_.AddElement<uint64_t>(Monster::VT_TESTHASHU64_FNV1A, testhashu64_fnv1a, 0); }
  void add_testarrayofbools(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> testarrayofbools) { fbb_.AddOffset(Monster::VT_TESTARRAYOFBOOLS, testarrayofbools); }
  void add_testf(float testf) { fbb_.AddElement<float>(Monster::VT_TESTF, testf, 3.14159f); }
  void add_testf2(float testf2) { fbb_.AddElement<float>(Monster::VT_TESTF2, testf2, 3.0f); }
  void add_testf3(float testf3) { fbb_.AddElement<float>(Monster::VT_TESTF3, testf3, 0.0f); }
  void add_testarrayofstring2(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> testarrayofstring2) { fbb_.AddOffset(Monster::VT_TESTARRAYOFSTRING2, testarrayofstring2); }
  MonsterBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  MonsterBuilder &operator=(const MonsterBuilder &);
  flatbuffers::Offset<Monster> Finish() {
    auto o = flatbuffers::Offset<Monster>(fbb_.EndTable(start_, 29));
    fbb_.Required(o, Monster::VT_NAME);  // name
    return o;
  }
};

inline flatbuffers::Offset<Monster> CreateMonster(flatbuffers::FlatBufferBuilder &_fbb,
    const Vec3 *pos = 0,
    int16_t mana = 150,
    int16_t hp = 100,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>> inventory = 0,
    Color color = Color_Blue,
    Any test_type = Any_NONE,
    flatbuffers::Offset<void> test = 0,
    flatbuffers::Offset<flatbuffers::Vector<const Test *>> test4 = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> testarrayofstring = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Monster>>> testarrayoftables = 0,
    flatbuffers::Offset<Monster> enemy = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>> testnestedflatbuffer = 0,
    flatbuffers::Offset<Stat> testempty = 0,
    bool testbool = false,
    int32_t testhashs32_fnv1 = 0,
    uint32_t testhashu32_fnv1 = 0,
    int64_t testhashs64_fnv1 = 0,
    uint64_t testhashu64_fnv1 = 0,
    int32_t testhashs32_fnv1a = 0,
    uint32_t testhashu32_fnv1a = 0,
    int64_t testhashs64_fnv1a = 0,
    uint64_t testhashu64_fnv1a = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>> testarrayofbools = 0,
    float testf = 3.14159f,
    float testf2 = 3.0f,
    float testf3 = 0.0f,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> testarrayofstring2 = 0) {
  MonsterBuilder builder_(_fbb);
  builder_.add_testhashu64_fnv1a(testhashu64_fnv1a);
  builder_.add_testhashs64_fnv1a(testhashs64_fnv1a);
  builder_.add_testhashu64_fnv1(testhashu64_fnv1);
  builder_.add_testhashs64_fnv1(testhashs64_fnv1);
  builder_.add_testarrayofstring2(testarrayofstring2);
  builder_.add_testf3(testf3);
  builder_.add_testf2(testf2);
  builder_.add_testf(testf);
  builder_.add_testarrayofbools(testarrayofbools);
  builder_.add_testhashu32_fnv1a(testhashu32_fnv1a);
  builder_.add_testhashs32_fnv1a(testhashs32_fnv1a);
  builder_.add_testhashu32_fnv1(testhashu32_fnv1);
  builder_.add_testhashs32_fnv1(testhashs32_fnv1);
  builder_.add_testempty(testempty);
  builder_.add_testnestedflatbuffer(testnestedflatbuffer);
  builder_.add_enemy(enemy);
  builder_.add_testarrayoftables(testarrayoftables);
  builder_.add_testarrayofstring(testarrayofstring);
  builder_.add_test4(test4);
  builder_.add_test(test);
  builder_.add_inventory(inventory);
  builder_.add_name(name);
  builder_.add_pos(pos);
  builder_.add_hp(hp);
  builder_.add_mana(mana);
  builder_.add_testbool(testbool);
  builder_.add_test_type(test_type);
  builder_.add_color(color);
  return builder_.Finish();
}

inline flatbuffers::Offset<Monster> CreateMonster(flatbuffers::FlatBufferBuilder &_fbb,
    const Vec3 *pos = 0,
    int16_t mana = 150,
    int16_t hp = 100,
    const char *name = nullptr,
    const std::vector<uint8_t> *inventory = nullptr,
    Color color = Color_Blue,
    Any test_type = Any_NONE,
    flatbuffers::Offset<void> test = 0,
    const std::vector<const Test *> *test4 = nullptr,
    const std::vector<flatbuffers::Offset<flatbuffers::String>> *testarrayofstring = nullptr,
    const std::vector<flatbuffers::Offset<Monster>> *testarrayoftables = nullptr,
    flatbuffers::Offset<Monster> enemy = 0,
    const std::vector<uint8_t> *testnestedflatbuffer = nullptr,
    flatbuffers::Offset<Stat> testempty = 0,
    bool testbool = false,
    int32_t testhashs32_fnv1 = 0,
    uint32_t testhashu32_fnv1 = 0,
    int64_t testhashs64_fnv1 = 0,
    uint64_t testhashu64_fnv1 = 0,
    int32_t testhashs32_fnv1a = 0,
    uint32_t testhashu32_fnv1a = 0,
    int64_t testhashs64_fnv1a = 0,
    uint64_t testhashu64_fnv1a = 0,
    const std::vector<uint8_t> *testarrayofbools = nullptr,
    float testf = 3.14159f,
    float testf2 = 3.0f,
    float testf3 = 0.0f,
    const std::vector<flatbuffers::Offset<flatbuffers::String>> *testarrayofstring2 = nullptr) {
  return CreateMonster(_fbb, pos, mana, hp, name ? 0 : _fbb.CreateString(name), inventory ? 0 : _fbb.CreateVector<uint8_t>(*inventory), color, test_type, test, test4 ? 0 : _fbb.CreateVector<const Test *>(*test4), testarrayofstring ? 0 : _fbb.CreateVector<flatbuffers::Offset<flatbuffers::String>>(*testarrayofstring), testarrayoftables ? 0 : _fbb.CreateVector<flatbuffers::Offset<Monster>>(*testarrayoftables), enemy, testnestedflatbuffer ? 0 : _fbb.CreateVector<uint8_t>(*testnestedflatbuffer), testempty, testbool, testhashs32_fnv1, testhashu32_fnv1, testhashs64_fnv1, testhashu64_fnv1, testhashs32_fnv1a, testhashu32_fnv1a, testhashs64_fnv1a, testhashu64_fnv1a, testarrayofbools ? 0 : _fbb.CreateVector<uint8_t>(*testarrayofbools), testf, testf2, testf3, testarrayofstring2 ? 0 : _fbb.CreateVector<flatbuffers::Offset<flatbuffers::String>>(*testarrayofstring2));
}

}  // namespace Example

namespace Example2 {

inline void MonsterT::UnPack(const Monster *_o) {
  (void)_o;
}

inline flatbuffers::Offset<Monster> MonsterT::Pack(flatbuffers::FlatBufferBuilder &_fbb) const {
  return CreateMonster(_fbb);
}

}  // namespace Example2

namespace Example {

inline void TestSimpleTableWithEnumT::UnPack(const TestSimpleTableWithEnum *_o) {
  color = _o->color();
}

inline flatbuffers::Offset<TestSimpleTableWithEnum> TestSimpleTableWithEnumT::Pack(flatbuffers::FlatBufferBuilder &_fbb) const {
  return CreateTestSimpleTableWithEnum(_fbb,
    color);
}

inline void StatT::UnPack(const Stat *_o) {
  id.assign(_o->id()->c_str(), _o->id()->size());
  val = _o->val();
  count = _o->count();
}

inline flatbuffers::Offset<Stat> StatT::Pack(flatbuffers::FlatBufferBuilder &_fbb) const {
  return CreateStat(_fbb,
    id.size() ? _fbb.CreateString(id) : 0,
    val,
    count);
}

inline void MonsterT::UnPack(const Monster *_o) {
  pos = _o->pos();
  mana = _o->mana();
  hp = _o->hp();
  name.assign(_o->name()->c_str(), _o->name()->size());
  inventory.clear();
  if (_o->inventory()) {
    inventory.reserve(_o->inventory()->size());
    for (auto it = _o->inventory()->begin(); it != _o->inventory()->end(); ++it)
      inventory.push_back((*it));
  }
  color = _o->color();
  test.UnPack(_o->test(), _o->test_type());
  test4.clear();
  if (_o->test4()) {
    test4.reserve(_o->test4()->size());
    for (auto it = _o->test4()->begin(); it != _o->test4()->end(); ++it)
      test4.push_back(*(*it));
  }
  testarrayofstring.clear();
  if (_o->testarrayofstring()) {
    testarrayofstring.reserve(_o->testarrayofstring()->size());
    for (auto it = _o->testarrayofstring()->begin(); it != _o->testarrayofstring()->end(); ++it)
      testarrayofstring.push_back(std::string((*it)->c_str(), (*it)->size()));
  }
  testarrayoftables.clear();
  if (_o->testarrayoftables()) {
    testarrayoftables.reserve(_o->testarrayoftables()->size());
    for (auto it = _o->testarrayoftables()->begin(); it != _o->testarrayoftables()->end(); ++it)
      testarrayoftables.push_back(MonsterT((*it)));
  }
  enemy = _o->enemy();
  testnestedflatbuffer.clear();
  if (_o->testnestedflatbuffer()) {
    testnestedflatbuffer.reserve(_o->testnestedflatbuffer()->size());
    for (auto it = _o->testnestedflatbuffer()->begin(); it != _o->testnestedflatbuffer()->end(); ++it)
      testnestedflatbuffer.push_back((*it));
  }
  testempty = _o->testempty();
  testbool = _o->testbool();
  testhashs32_fnv1 = _o->testhashs32_fnv1();
  testhashu32_fnv1 = _o->testhashu32_fnv1();
  testhashs64_fnv1 = _o->testhashs64_fnv1();
  testhashu64_fnv1 = _o->testhashu64_fnv1();
  testhashs32_fnv1a = _o->testhashs32_fnv1a();
  testhashu32_fnv1a = _o->testhashu32_fnv1a();
  testhashs64_fnv1a = _o->testhashs64_fnv1a();
  testhashu64_fnv1a = _o->testhashu64_fnv1a();
  testarrayofbools.clear();
  if (_o->testarrayofbools()) {
    testarrayofbools.reserve(_o->testarrayofbools()->size());
    for (auto it = _o->testarrayofbools()->begin(); it != _o->testarrayofbools()->end(); ++it)
      testarrayofbools.push_back((*it) != 0);
  }
  testf = _o->testf();
  testf2 = _o->testf2();
  testf3 = _o->testf3();
  testarrayofstring2.clear();
  if (_o->testarrayofstring2()) {
    testarrayofstring2.reserve(_o->testarrayofstring2()->size());
    for (auto it = _o->testarrayofstring2()->begin(); it != _o->testarrayofstring2()->end(); ++it)
      testarrayofstring2.push_back(std::string((*it)->c_str(), (*it)->size()));
  }
}

inline flatbuffers::Offset<Monster> MonsterT::Pack(flatbuffers::FlatBufferBuilder &_fbb) const {
  return CreateMonster(_fbb,
    pos,
    mana,
    hp,
    name.size() ? _fbb.CreateString(name) : 0,
    inventory.size() ? _fbb.CreateVector(inventory) : 0,
    color,
    test.type,
    test.Pack(_fbb),
    test4.size() ? _fbb.CreateVectorOfStructs(test4) : 0,
    testarrayofstring.size() ? _fbb.CreateVectorOfStrings(testarrayofstring) : 0,
    testarrayoftables.size() ? _fbb.CreateVector<flatbuffers::Offset<Monster>>(testarrayoftables.size(), [&](size_t i) { return testarrayoftables[i].Pack(_fbb); }) : 0,
    enemy ? enemy->Pack(_fbb) : 0,
    testnestedflatbuffer.size() ? _fbb.CreateVector(testnestedflatbuffer) : 0,
    testempty ? testempty->Pack(_fbb) : 0,
    testbool,
    testhashs32_fnv1,
    testhashu32_fnv1,
    testhashs64_fnv1,
    testhashu64_fnv1,
    testhashs32_fnv1a,
    testhashu32_fnv1a,
    testhashs64_fnv1a,
    testhashu64_fnv1a,
    testarrayofbools.size() ? _fbb.CreateVector(testarrayofbools) : 0,
    testf,
    testf2,
    testf3,
    testarrayofstring2.size() ? _fbb.CreateVectorOfStrings(testarrayofstring2) : 0);
}

inline bool VerifyAny(flatbuffers::Verifier &verifier, const void *union_obj, Any type) {
  switch (type) {
    case Any_NONE: return true;
    case Any_Monster: return verifier.VerifyTable(reinterpret_cast<const Monster *>(union_obj));
    case Any_TestSimpleTableWithEnum: return verifier.VerifyTable(reinterpret_cast<const TestSimpleTableWithEnum *>(union_obj));
    case Any_MyGame_Example2_Monster: return verifier.VerifyTable(reinterpret_cast<const MyGame::Example2::Monster *>(union_obj));
    default: return false;
  }
}

inline void AnyUnion::UnPack(const void *union_obj, Any _t) {
  type = _t;
  delete table;
  if (!union_obj) { table = nullptr; type = Any_NONE; return; }
  switch (_t) {
    case Any_NONE: table = nullptr; break;
    case Any_Monster: table = new MonsterT(reinterpret_cast<const Monster *>(union_obj)); break;
    case Any_TestSimpleTableWithEnum: table = new TestSimpleTableWithEnumT(reinterpret_cast<const TestSimpleTableWithEnum *>(union_obj)); break;
    case Any_MyGame_Example2_Monster: table = new MyGame::Example2::MonsterT(reinterpret_cast<const MyGame::Example2::Monster *>(union_obj)); break;
    default: table = nullptr; type = Any_NONE;
  }
}

inline flatbuffers::Offset<void> AnyUnion::Pack(flatbuffers::FlatBufferBuilder &_fbb) const {
  switch (type) {
    case Any_NONE: return 0;
    case Any_Monster: return static_cast<const MonsterT *>(table)->Pack(_fbb).Union();
    case Any_TestSimpleTableWithEnum: return static_cast<const TestSimpleTableWithEnumT *>(table)->Pack(_fbb).Union();
    case Any_MyGame_Example2_Monster: return static_cast<const MyGame::Example2::MonsterT *>(table)->Pack(_fbb).Union();
    default: return 0;
  }
}

inline AnyUnion::AnyUnion(const AnyUnion &other) : type(Any_NONE), table(nullptr) { *this = other; }
inline AnyUnion& AnyUnion::operator=(const AnyUnion &other) {
  type = other.type;
  delete table;
  switch (other.type) {
    case Any_Monster: table = new MonsterT(*(static_cast<MonsterT *>(other.table))); break;
    case Any_TestSimpleTableWithEnum: table = new TestSimpleTableWithEnumT(*(static_cast<TestSimpleTableWithEnumT *>(other.table))); break;
    case Any_MyGame_Example2_Monster: table = new MyGame::Example2::MonsterT(*(static_cast<MyGame::Example2::MonsterT *>(other.table))); break;
    default:
      type = Any_NONE;
      table = nullptr;
      break;
  }
  return *this;
}

inline AnyUnion::~AnyUnion() {
  switch (type) {
    case Any_Monster: delete static_cast<MonsterT *>(table); break;
    case Any_TestSimpleTableWithEnum: delete static_cast<TestSimpleTableWithEnumT *>(table); break;
    case Any_MyGame_Example2_Monster: delete static_cast<MyGame::Example2::MonsterT *>(table); break;
    default: assert(!table); break;
  }
}

inline AnyUnion& AnyUnion::operator=(const MonsterT &_o) {
  type = Any_Monster;
  delete table;
  table = new MonsterT(_o);
  return *this;
}
inline AnyUnion& AnyUnion::operator=(const TestSimpleTableWithEnumT &_o) {
  type = Any_TestSimpleTableWithEnum;
  delete table;
  table = new TestSimpleTableWithEnumT(_o);
  return *this;
}
inline AnyUnion& AnyUnion::operator=(const MyGame::Example2::MonsterT &_o) {
  type = Any_MyGame_Example2_Monster;
  delete table;
  table = new MyGame::Example2::MonsterT(_o);
  return *this;
}

inline const MyGame::Example::Monster *GetMonster(const void *buf) { return flatbuffers::GetRoot<MyGame::Example::Monster>(buf); }

inline Monster *GetMutableMonster(void *buf) { return flatbuffers::GetMutableRoot<Monster>(buf); }

inline const char *MonsterIdentifier() { return "MONS"; }

inline bool MonsterBufferHasIdentifier(const void *buf) { return flatbuffers::BufferHasIdentifier(buf, MonsterIdentifier()); }

inline bool VerifyMonsterBuffer(flatbuffers::Verifier &verifier) { return verifier.VerifyBuffer<MyGame::Example::Monster>(MonsterIdentifier()); }

inline const char *MonsterExtension() { return "mon"; }

inline void FinishMonsterBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<MyGame::Example::Monster> root) { fbb.Finish(root, MonsterIdentifier()); }

}  // namespace Example
}  // namespace MyGame

#endif  // FLATBUFFERS_GENERATED_MONSTERTEST_MYGAME_EXAMPLE_H_
