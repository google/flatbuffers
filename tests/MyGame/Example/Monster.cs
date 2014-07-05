// automatically generated, do not modify

namespace MyGame.Example
{

using FlatBuffersNet;

public class Monster : Table {
  public static Monster GetRootAsMonster(ByteBuffer _bb, int offset) { return (new Monster()).__init(_bb.GetInt(offset) + offset, _bb); }
  public Monster __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }
  public Vec3 Pos() { return Pos(new Vec3()); }
  public Vec3 Pos(Vec3 obj) { int o = __offset(4); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }
  public short Mana() { int o = __offset(6); return o != 0 ? bb.GetShort(o + bb_pos) : (short)150; }
  public short Hp() { int o = __offset(8); return o != 0 ? bb.GetShort(o + bb_pos) : (short)100; }
  public string Name() { int o = __offset(10); return o != 0 ? __string(o) : null; }
  public byte Inventory(int j) { int o = __offset(14); return o != 0 ? bb.Get(__vector(o) + j * 1) : (byte)0; }
  public int InventoryLength() { int o = __offset(14); return o != 0 ? __vector_len(o) : 0; }
  /* an example documentation comment: this will end up in the generated code multiline too*/
  public byte Color() { int o = __offset(16); return o != 0 ? bb.Get(o + bb_pos) : (byte)2; }
  public byte TestType() { int o = __offset(18); return o != 0 ? bb.Get(o + bb_pos) : (byte)0; }
  public Table Test(Table obj) { int o = __offset(20); return o != 0 ? __union(obj, o) : null; }
  public Test Test4(int j) { return Test4(new Test(), j); }
  public Test Test4(Test obj, int j) { int o = __offset(22); return o != 0 ? obj.__init(__vector(o) + j * 4, bb) : null; }
  public int Test4Length() { int o = __offset(22); return o != 0 ? __vector_len(o) : 0; }
  public string Testarrayofstring(int j) { int o = __offset(24); return o != 0 ? __string(__vector(o) + j * 4) : null; }
  public int TestarrayofstringLength() { int o = __offset(24); return o != 0 ? __vector_len(o) : 0; }
  public Monster Testarrayoftables(int j) { return Testarrayoftables(new Monster(), j); }
  public Monster Testarrayoftables(Monster obj, int j) { int o = __offset(26); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int TestarrayoftablesLength() { int o = __offset(26); return o != 0 ? __vector_len(o) : 0; }

  public static void StartMonster(FlatBufferBuilder builder) { builder.StartObject(12); }
  public static void AddPos(FlatBufferBuilder builder, int pos) { builder.AddStruct(0, pos, 0); }
  public static void AddMana(FlatBufferBuilder builder, short mana) { builder.AddShort(1, mana, 150); }
  public static void AddHp(FlatBufferBuilder builder, short hp) { builder.AddShort(2, hp, 100); }
  public static void AddName(FlatBufferBuilder builder, int name) { builder.AddOffset(3, name, 0); }
  public static void AddInventory(FlatBufferBuilder builder, int inventory) { builder.AddOffset(5, inventory, 0); }
  public static void StartInventoryVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems); }
  public static void AddColor(FlatBufferBuilder builder, byte color) { builder.AddByte(6, color, 2); }
  public static void AddTestType(FlatBufferBuilder builder, byte testType) { builder.AddByte(7, testType, 0); }
  public static void AddTest(FlatBufferBuilder builder, int test) { builder.AddOffset(8, test, 0); }
  public static void AddTest4(FlatBufferBuilder builder, int test4) { builder.AddOffset(9, test4, 0); }
  public static void StartTest4Vector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems); }
  public static void AddTestarrayofstring(FlatBufferBuilder builder, int testarrayofstring) { builder.AddOffset(10, testarrayofstring, 0); }
  public static void StartTestarrayofstringVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems); }
  public static void AddTestarrayoftables(FlatBufferBuilder builder, int testarrayoftables) { builder.AddOffset(11, testarrayoftables, 0); }
  public static void StartTestarrayoftablesVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems); }
  public static int EndMonster(FlatBufferBuilder builder) { return builder.EndObject(); }
};


}
