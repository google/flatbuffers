// automatically generated, do not modify

module MyGame.Example.Monster;

import flatbuffers;

import MyGame.Example.Vec3;
import MyGame.Example.Color;
import MyGame.Example.Any;
import MyGame.Example.Test;
import MyGame.Example.Monster;
import MyGame.Example.Stat;

/// an example documentation comment: monster object
struct Monster {
  mixin Table!Monster;

  static Monster getRootAsMonster(ByteBuffer _bb) {  return Monster.init_(_bb.get!int(_bb.position()) + _bb.position(), _bb); }
  static boolMonsterBufferHasIdentifier(ByteBuffer _bb) { return __has_identifier(_bb, "MONS"); }
  @property   Nullable!Vec3 pos() { int o = __offset(4); return o != 0 ? Nullable!Vec3(Vec3.init_(o + _pos, _buffer)) : Nullable!Vec3.init; }
  @property   short mana() { int o = __offset(6); return o != 0 ? _buffer.get!short(o + _pos) : 150; }
  @property   short hp() { int o = __offset(8); return o != 0 ? _buffer.get!short(o + _pos) : 100; }
  @property   Nullable!string name() { int o = __offset(10); return o != 0 ? Nullable!string(__string(o + _pos)) : Nullable!string.init; }
  auto inventory() { return Iterator!(Monster, ubyte, "inventory")(this); }
    ubyte inventory(int j) { int o = __offset(14); return o != 0 ? _buffer.get!ubyte(__dvector(o) + j * 1)  : 0; }
  @property int inventoryLength() { int o = __offset(14); return o != 0 ? __vector_len(o) : 0; }
  @property   byte color() { int o = __offset(16); return o != 0 ? _buffer.get!byte(o + _pos) : 8; }
  @property   ubyte testType() { int o = __offset(18); return o != 0 ? _buffer.get!ubyte(o + _pos) : 0; }
    Nullable!T test(T)() { int o = __offset(20); return o != 0 ? Nullable!T(__union!(T)(o)) : Nullable!T.init; }
  auto test4() { return Iterator!(Monster, Test, "test4")(this); }
    Nullable!Test test4(int j) { int o = __offset(22); return o != 0 ? Nullable!Test(Test.init_(__dvector(o) + j * 4, _buffer)) : Nullable!Test.init; }
  @property int test4Length() { int o = __offset(22); return o != 0 ? __vector_len(o) : 0; }
  auto testarrayofstring() { return Iterator!(Monster, string, "testarrayofstring")(this); }
    Nullable!string testarrayofstring(int j) { int o = __offset(24); return o != 0 ? Nullable!string(__string(__dvector(o) + j * 4)) : Nullable!string.init; }
  @property int testarrayofstringLength() { int o = __offset(24); return o != 0 ? __vector_len(o) : 0; }
  /// an example documentation comment: this will end up in the generated code
  /// multiline too
  auto testarrayoftables() { return Iterator!(Monster, Monster, "testarrayoftables")(this); }
    Nullable!Monster testarrayoftables(int j) { int o = __offset(26); return o != 0 ? Nullable!Monster(Monster.init_(__indirect(__dvector(o) + j * 4), _buffer)) : Nullable!Monster.init; }
  @property int testarrayoftablesLength() { int o = __offset(26); return o != 0 ? __vector_len(o) : 0; }
  @property   Nullable!Monster enemy() { int o = __offset(28); return o != 0 ? Nullable!Monster(Monster.init_(__indirect(o + _pos), _buffer)) : Nullable!Monster.init; }
  auto testnestedflatbuffer() { return Iterator!(Monster, ubyte, "testnestedflatbuffer")(this); }
    ubyte testnestedflatbuffer(int j) { int o = __offset(30); return o != 0 ? _buffer.get!ubyte(__dvector(o) + j * 1)  : 0; }
  @property int testnestedflatbufferLength() { int o = __offset(30); return o != 0 ? __vector_len(o) : 0; }
  @property   Nullable!Stat testempty() { int o = __offset(32); return o != 0 ? Nullable!Stat(Stat.init_(__indirect(o + _pos), _buffer)) : Nullable!Stat.init; }
  @property   bool testbool() { int o = __offset(34); return o != 0 ? _buffer.get!ubyte(o + _pos) != 0  : false; }
  @property   int testhashs32Fnv1() { int o = __offset(36); return o != 0 ? _buffer.get!int(o + _pos) : 0; }
  @property   uint testhashu32Fnv1() { int o = __offset(38); return o != 0 ? _buffer.get!uint(o + _pos) : 0; }
  @property   long testhashs64Fnv1() { int o = __offset(40); return o != 0 ? _buffer.get!long(o + _pos) : 0; }
  @property   ulong testhashu64Fnv1() { int o = __offset(42); return o != 0 ? _buffer.get!ulong(o + _pos) : 0; }
  @property   int testhashs32Fnv1a() { int o = __offset(44); return o != 0 ? _buffer.get!int(o + _pos) : 0; }
  @property   uint testhashu32Fnv1a() { int o = __offset(46); return o != 0 ? _buffer.get!uint(o + _pos) : 0; }
  @property   long testhashs64Fnv1a() { int o = __offset(48); return o != 0 ? _buffer.get!long(o + _pos) : 0; }
  @property   ulong testhashu64Fnv1a() { int o = __offset(50); return o != 0 ? _buffer.get!ulong(o + _pos) : 0; }
  auto testarrayofbools() { return Iterator!(Monster, bool, "testarrayofbools")(this); }
    bool testarrayofbools(int j) { int o = __offset(52); return o != 0 ? _buffer.get!ubyte(__dvector(o) + j * 1)  != 0  : false; }
  @property int testarrayofboolsLength() { int o = __offset(52); return o != 0 ? __vector_len(o) : 0; }
  @property   float testf() { int o = __offset(54); return o != 0 ? _buffer.get!float(o + _pos) : 3.14159; }
  @property   float testf2() { int o = __offset(56); return o != 0 ? _buffer.get!float(o + _pos) : 3.0; }

  static void startMonster(FlatBufferBuilder builder) { builder.startObject(27); }
  static void addPos(FlatBufferBuilder builder, int posOffset) { builder.addStruct(0, posOffset, 0); }
  static void addMana(FlatBufferBuilder builder, short mana) { builder.addShort(1, mana, 150); }
  static void addHp(FlatBufferBuilder builder, short hp) { builder.addShort(2, hp, 100); }
  static void addName(FlatBufferBuilder builder, int nameOffset) { builder.addOffset(3, nameOffset, 0); }
  static void addInventory(FlatBufferBuilder builder, int inventoryOffset) { builder.addOffset(5, inventoryOffset, 0); }
  static int createInventoryVector(FlatBufferBuilder builder, ubyte[] data) { builder.startVector(1, cast(int)data.length, 1); for (int i = cast(int)data.length - 1; i >= 0; i--) builder.addUbyte(data[i]); return builder.endVector(); }
  static void startInventoryVector(FlatBufferBuilder builder, int numElems) { builder.startVector(1, numElems, 1); }
  static void addColor(FlatBufferBuilder builder, byte color) { builder.addByte(6, color, 8); }
  static void addTestType(FlatBufferBuilder builder, ubyte testType) { builder.addUbyte(7, testType, 0); }
  static void addTest(FlatBufferBuilder builder, int testOffset) { builder.addOffset(8, testOffset, 0); }
  static void addTest4(FlatBufferBuilder builder, int test4Offset) { builder.addOffset(9, test4Offset, 0); }
  static void startTest4Vector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 2); }
  static void addTestarrayofstring(FlatBufferBuilder builder, int testarrayofstringOffset) { builder.addOffset(10, testarrayofstringOffset, 0); }
  static int createTestarrayofstringVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, cast(int)data.length, 4); for (int i = cast(int)data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  static void startTestarrayofstringVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  static void addTestarrayoftables(FlatBufferBuilder builder, int testarrayoftablesOffset) { builder.addOffset(11, testarrayoftablesOffset, 0); }
  static int createTestarrayoftablesVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, cast(int)data.length, 4); for (int i = cast(int)data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  static void startTestarrayoftablesVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  static void addEnemy(FlatBufferBuilder builder, int enemyOffset) { builder.addOffset(12, enemyOffset, 0); }
  static void addTestnestedflatbuffer(FlatBufferBuilder builder, int testnestedflatbufferOffset) { builder.addOffset(13, testnestedflatbufferOffset, 0); }
  static int createTestnestedflatbufferVector(FlatBufferBuilder builder, ubyte[] data) { builder.startVector(1, cast(int)data.length, 1); for (int i = cast(int)data.length - 1; i >= 0; i--) builder.addUbyte(data[i]); return builder.endVector(); }
  static void startTestnestedflatbufferVector(FlatBufferBuilder builder, int numElems) { builder.startVector(1, numElems, 1); }
  static void addTestempty(FlatBufferBuilder builder, int testemptyOffset) { builder.addOffset(14, testemptyOffset, 0); }
  static void addTestbool(FlatBufferBuilder builder, bool testbool) { builder.addBool(15, testbool, false); }
  static void addTesthashs32Fnv1(FlatBufferBuilder builder, int testhashs32Fnv1) { builder.addInt(16, testhashs32Fnv1, 0); }
  static void addTesthashu32Fnv1(FlatBufferBuilder builder, uint testhashu32Fnv1) { builder.addUint(17, testhashu32Fnv1, 0); }
  static void addTesthashs64Fnv1(FlatBufferBuilder builder, long testhashs64Fnv1) { builder.addLong(18, testhashs64Fnv1, 0); }
  static void addTesthashu64Fnv1(FlatBufferBuilder builder, ulong testhashu64Fnv1) { builder.addUlong(19, testhashu64Fnv1, 0); }
  static void addTesthashs32Fnv1a(FlatBufferBuilder builder, int testhashs32Fnv1a) { builder.addInt(20, testhashs32Fnv1a, 0); }
  static void addTesthashu32Fnv1a(FlatBufferBuilder builder, uint testhashu32Fnv1a) { builder.addUint(21, testhashu32Fnv1a, 0); }
  static void addTesthashs64Fnv1a(FlatBufferBuilder builder, long testhashs64Fnv1a) { builder.addLong(22, testhashs64Fnv1a, 0); }
  static void addTesthashu64Fnv1a(FlatBufferBuilder builder, ulong testhashu64Fnv1a) { builder.addUlong(23, testhashu64Fnv1a, 0); }
  static void addTestarrayofbools(FlatBufferBuilder builder, int testarrayofboolsOffset) { builder.addOffset(24, testarrayofboolsOffset, false); }
  static int createTestarrayofboolsVector(FlatBufferBuilder builder, bool[] data) { builder.startVector(1, cast(int)data.length, 1); for (int i = cast(int)data.length - 1; i >= 0; i--) builder.addBool(data[i]); return builder.endVector(); }
  static void startTestarrayofboolsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(1, numElems, 1); }
  static void addTestf(FlatBufferBuilder builder, float testf) { builder.addFloat(25, testf, 3.14159); }
  static void addTestf2(FlatBufferBuilder builder, float testf2) { builder.addFloat(26, testf2, 3.0); }
  static int endMonster(FlatBufferBuilder builder) {
    int o = builder.endObject();
    builder.required(o, 10);  // name
    return o;
  }
  static void finishMonsterBuffer(FlatBufferBuilder builder, int offset) { builder.finish(offset, "MONS"); }
}

