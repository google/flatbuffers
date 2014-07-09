// automatically generated, do not modify

package MyGame.Example;

import java.nio.*;
import java.lang.*;
import java.util.*;
import flatbuffers.*;

public class Monster extends Table {
  public static Monster getRootAsMonster(ByteBuffer _bb, int offset) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (new Monster()).__init(_bb.getInt(offset) + offset, _bb); }
  public Monster __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }
  public Vec3 pos() { return pos(new Vec3()); }
  public Vec3 pos(Vec3 obj) { int o = __offset(4); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }
  public short mana() { int o = __offset(6); return o != 0 ? bb.getShort(o + bb_pos) : 150; }
  public short hp() { int o = __offset(8); return o != 0 ? bb.getShort(o + bb_pos) : 100; }
  public String name() { int o = __offset(10); return o != 0 ? __string(o) : null; }
  public byte inventory(int j) { int o = __offset(14); return o != 0 ? bb.get(__vector(o) + j * 1) : 0; }
  public int inventoryLength() { int o = __offset(14); return o != 0 ? __vector_len(o) : 0; }
  public byte color() { int o = __offset(16); return o != 0 ? bb.get(o + bb_pos) : 2; }
  public byte testType() { int o = __offset(18); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public Table test(Table obj) { int o = __offset(20); return o != 0 ? __union(obj, o) : null; }
  public Test test4(int j) { return test4(new Test(), j); }
  public Test test4(Test obj, int j) { int o = __offset(22); return o != 0 ? obj.__init(__vector(o) + j * 4, bb) : null; }
  public int test4Length() { int o = __offset(22); return o != 0 ? __vector_len(o) : 0; }
  public String testarrayofstring(int j) { int o = __offset(24); return o != 0 ? __string(__vector(o) + j * 4) : null; }
  public int testarrayofstringLength() { int o = __offset(24); return o != 0 ? __vector_len(o) : 0; }
  /// an example documentation comment: this will end up in the generated code multiline too
  public Monster testarrayoftables(int j) { return testarrayoftables(new Monster(), j); }
  public Monster testarrayoftables(Monster obj, int j) { int o = __offset(26); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int testarrayoftablesLength() { int o = __offset(26); return o != 0 ? __vector_len(o) : 0; }
  public Monster enemy() { return enemy(new Monster()); }
  public Monster enemy(Monster obj) { int o = __offset(28); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }

  public static void startMonster(FlatBufferBuilder builder) { builder.startObject(13); }
  public static void addPos(FlatBufferBuilder builder, int pos) { builder.addStruct(0, pos, 0); }
  public static void addMana(FlatBufferBuilder builder, short mana) { builder.addShort(1, mana, 150); }
  public static void addHp(FlatBufferBuilder builder, short hp) { builder.addShort(2, hp, 100); }
  public static void addName(FlatBufferBuilder builder, int name) { builder.addOffset(3, name, 0); }
  public static void addInventory(FlatBufferBuilder builder, int inventory) { builder.addOffset(5, inventory, 0); }
  public static void startInventoryVector(FlatBufferBuilder builder, int numElems) { builder.startVector(1, numElems); }
  public static void addColor(FlatBufferBuilder builder, byte color) { builder.addByte(6, color, 2); }
  public static void addTestType(FlatBufferBuilder builder, byte testType) { builder.addByte(7, testType, 0); }
  public static void addTest(FlatBufferBuilder builder, int test) { builder.addOffset(8, test, 0); }
  public static void addTest4(FlatBufferBuilder builder, int test4) { builder.addOffset(9, test4, 0); }
  public static void startTest4Vector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems); }
  public static void addTestarrayofstring(FlatBufferBuilder builder, int testarrayofstring) { builder.addOffset(10, testarrayofstring, 0); }
  public static void startTestarrayofstringVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems); }
  public static void addTestarrayoftables(FlatBufferBuilder builder, int testarrayoftables) { builder.addOffset(11, testarrayoftables, 0); }
  public static void startTestarrayoftablesVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems); }
  public static void addEnemy(FlatBufferBuilder builder, int enemy) { builder.addOffset(12, enemy, 0); }
  public static int endMonster(FlatBufferBuilder builder) { return builder.endObject(); }
};

