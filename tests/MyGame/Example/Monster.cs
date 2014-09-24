// automatically generated, do not modify

namespace MyGame.Example
{

using FlatBuffers;

public class Monster : Table {
  public static Monster GetRootAsMonster(ByteBuffer _bb) { return (new Monster()).__init(_bb.GetInt(_bb.position()) + _bb.position(), _bb); }
  public static bool MonsterBufferHasIdentifier(ByteBuffer _bb) { return __has_identifier(_bb, "MONS"); }
  public Monster __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Vec3 Pos() { return Pos(new Vec3()); }
  public Vec3 Pos(Vec3 obj) { int o = __offset(4); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }
  public short Mana() { int o = __offset(6); return o != 0 ? bb.GetShort(o + bb_pos) : (short)150; }
  public short Hp() { int o = __offset(8); return o != 0 ? bb.GetShort(o + bb_pos) : (short)100; }
  public string Name() { int o = __offset(10); return o != 0 ? __string(o + bb_pos) : null; }
  public byte Inventory(int j) { int o = __offset(14); return o != 0 ? bb.Get(__vector(o) + j * 1) : (byte)0; }
  public int InventoryLength() { int o = __offset(14); return o != 0 ? __vector_len(o) : 0; }
  public sbyte Color() { int o = __offset(16); return o != 0 ? bb.GetSbyte(o + bb_pos) : (sbyte)8; }
  public byte TestType() { int o = __offset(18); return o != 0 ? bb.Get(o + bb_pos) : (byte)0; }
  public Table Test(Table obj) { int o = __offset(20); return o != 0 ? __union(obj, o) : null; }
  public Test Test4(int j) { return Test4(new Test(), j); }
  public Test Test4(Test obj, int j) { int o = __offset(22); return o != 0 ? obj.__init(__vector(o) + j * 4, bb) : null; }
  public int Test4Length() { int o = __offset(22); return o != 0 ? __vector_len(o) : 0; }
  public string Testarrayofstring(int j) { int o = __offset(24); return o != 0 ? __string(__vector(o) + j * 4) : null; }
  public int TestarrayofstringLength() { int o = __offset(24); return o != 0 ? __vector_len(o) : 0; }
  /// an example documentation comment: this will end up in the generated code
  /// multiline too
  public Monster Testarrayoftables(int j) { return Testarrayoftables(new Monster(), j); }
  public Monster Testarrayoftables(Monster obj, int j) { int o = __offset(26); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int TestarrayoftablesLength() { int o = __offset(26); return o != 0 ? __vector_len(o) : 0; }
  public Monster Enemy() { return Enemy(new Monster()); }
  public Monster Enemy(Monster obj) { int o = __offset(28); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public byte Testnestedflatbuffer(int j) { int o = __offset(30); return o != 0 ? bb.Get(__vector(o) + j * 1) : (byte)0; }
  public int TestnestedflatbufferLength() { int o = __offset(30); return o != 0 ? __vector_len(o) : 0; }
  public Monster Testempty() { return Testempty(new Monster()); }
  public Monster Testempty(Monster obj) { int o = __offset(32); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }

  public static void StartMonster(FlatBufferBuilder builder) { builder.StartObject(15); }
  public static void AddPos(FlatBufferBuilder builder, int posOffset) { builder.AddStruct(0, posOffset, 0); }
  public static void AddMana(FlatBufferBuilder builder, short mana) { builder.AddShort(1, mana, 150); }
  public static void AddHp(FlatBufferBuilder builder, short hp) { builder.AddShort(2, hp, 100); }
  public static void AddName(FlatBufferBuilder builder, int nameOffset) { builder.AddOffset(3, nameOffset, 0); }
  public static void AddInventory(FlatBufferBuilder builder, int inventoryOffset) { builder.AddOffset(5, inventoryOffset, 0); }
  public static int CreateInventoryVector(FlatBufferBuilder builder, byte[] data) { builder.StartVector(1, data.Length, 1); for (int i = data.Length - 1; i >= 0; i--) builder.AddByte(data[i]); return builder.EndVector(); }
  public static void StartInventoryVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(1, numElems, 1); }
  public static void AddColor(FlatBufferBuilder builder, sbyte color) { builder.AddSbyte(6, color, 8); }
  public static void AddTestType(FlatBufferBuilder builder, byte testType) { builder.AddByte(7, testType, 0); }
  public static void AddTest(FlatBufferBuilder builder, int testOffset) { builder.AddOffset(8, testOffset, 0); }
  public static void AddTest4(FlatBufferBuilder builder, int test4Offset) { builder.AddOffset(9, test4Offset, 0); }
  public static void StartTest4Vector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems, 2); }
  public static void AddTestarrayofstring(FlatBufferBuilder builder, int testarrayofstringOffset) { builder.AddOffset(10, testarrayofstringOffset, 0); }
  public static int CreateTestarrayofstringVector(FlatBufferBuilder builder, int[] data) { builder.StartVector(4, data.Length, 4); for (int i = data.Length - 1; i >= 0; i--) builder.AddOffset(data[i]); return builder.EndVector(); }
  public static void StartTestarrayofstringVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems, 4); }
  public static void AddTestarrayoftables(FlatBufferBuilder builder, int testarrayoftablesOffset) { builder.AddOffset(11, testarrayoftablesOffset, 0); }
  public static int CreateTestarrayoftablesVector(FlatBufferBuilder builder, int[] data) { builder.StartVector(4, data.Length, 4); for (int i = data.Length - 1; i >= 0; i--) builder.AddOffset(data[i]); return builder.EndVector(); }
  public static void StartTestarrayoftablesVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems, 4); }
  public static void AddEnemy(FlatBufferBuilder builder, int enemyOffset) { builder.AddOffset(12, enemyOffset, 0); }
  public static void AddTestnestedflatbuffer(FlatBufferBuilder builder, int testnestedflatbufferOffset) { builder.AddOffset(13, testnestedflatbufferOffset, 0); }
  public static int CreateTestnestedflatbufferVector(FlatBufferBuilder builder, byte[] data) { builder.StartVector(1, data.Length, 1); for (int i = data.Length - 1; i >= 0; i--) builder.AddByte(data[i]); return builder.EndVector(); }
  public static void StartTestnestedflatbufferVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(1, numElems, 1); }
  public static void AddTestempty(FlatBufferBuilder builder, int testemptyOffset) { builder.AddOffset(14, testemptyOffset, 0); }
  public static int EndMonster(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    builder.Required(o, 10);  // name
    return o;
  }
  public static void FinishMonsterBuffer(FlatBufferBuilder builder, int offset) { builder.Finish(offset, "MONS"); }
};


}
