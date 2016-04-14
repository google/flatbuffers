// automatically generated, do not modify

module myGame.sample.monster;

import google.flatbuffers;

import myGame.sample.vec3;
import myGame.sample.color;
import myGame.sample.weapon;
import myGame.sample.equipment;

struct Monster {
  mixin Table!Monster;

  static Monster getRootAsMonster(ByteBuffer _bb) {  return Monster.init_(_bb.get!int(_bb.position()) + _bb.position(), _bb); }
  @property   Nullable!Vec3 pos() { int o = __offset(4); return o != 0 ? Nullable!Vec3(Vec3.init_(o + _pos, _buffer)) : Nullable!Vec3.init; }
  @property   short mana() { int o = __offset(6); return o != 0 ? _buffer.get!short(o + _pos) : 150; }
  @property   short hp() { int o = __offset(8); return o != 0 ? _buffer.get!short(o + _pos) : 100; }
  @property   Nullable!string name() { int o = __offset(10); return o != 0 ? Nullable!string(__string(o + _pos)) : Nullable!string.init; }
  auto inventory() { return Iterator!(Monster, ubyte, "inventory")(this); }
    ubyte inventory(int j) { int o = __offset(14); return o != 0 ? _buffer.get!ubyte(__dvector(o) + j * 1) : 0; }
  @property int inventoryLength() { int o = __offset(14); return o != 0 ? __vector_len(o) : 0; }
  @property   ubyte color() { int o = __offset(16); return o != 0 ? _buffer.get!ubyte(o + _pos) : 2; }
  auto weapons() { return Iterator!(Monster, Weapon, "weapons")(this); }
    Nullable!Weapon weapons(int j) { int o = __offset(18); return o != 0 ? Nullable!Weapon(Weapon.init_(__indirect(__dvector(o) + j * 4), _buffer)) : Nullable!Weapon.init; }
  @property int weaponsLength() { int o = __offset(18); return o != 0 ? __vector_len(o) : 0; }
  @property   ubyte equippedType() { int o = __offset(20); return o != 0 ? _buffer.get!ubyte(o + _pos) : 0; }
    Nullable!T equipped(T)() { int o = __offset(22); return o != 0 ? Nullable!T(__union!(T)(o)) : Nullable!T.init; }

  static void startMonster(FlatBufferBuilder builder) { builder.startObject(10); }
  static void addPos(FlatBufferBuilder builder, int posOffset) { builder.addStruct(0, posOffset, 0); }
  static void addMana(FlatBufferBuilder builder, short mana) { builder.addShort(1, mana, 150); }
  static void addHp(FlatBufferBuilder builder, short hp) { builder.addShort(2, hp, 100); }
  static void addName(FlatBufferBuilder builder, int nameOffset) { builder.addOffset(3, nameOffset, 0); }
  static void addInventory(FlatBufferBuilder builder, int inventoryOffset) { builder.addOffset(5, inventoryOffset, 0); }
  static int createInventoryVector(FlatBufferBuilder builder, ubyte[] data) { builder.startVector(1, cast(int)data.length, 1); for (int i = cast(int)data.length - 1; i >= 0; i--) builder.addUbyte(data[i]); return builder.endVector(); }
  static void startInventoryVector(FlatBufferBuilder builder, int numElems) { builder.startVector(1, numElems, 1); }
  static void addColor(FlatBufferBuilder builder, ubyte color) { builder.addUbyte(6, color, 2); }
  static void addWeapons(FlatBufferBuilder builder, int weaponsOffset) { builder.addOffset(7, weaponsOffset, 0); }
  static int createWeaponsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, cast(int)data.length, 4); for (int i = cast(int)data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  static void startWeaponsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  static void addEquippedType(FlatBufferBuilder builder, ubyte equippedType) { builder.addUbyte(8, equippedType, 0); }
  static void addEquipped(FlatBufferBuilder builder, int equippedOffset) { builder.addOffset(9, equippedOffset, 0); }
  static int endMonster(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
  static void finishMonsterBuffer(FlatBufferBuilder builder, int offset) { builder.finish(offset); }
}

