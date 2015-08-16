// automatically generated, do not modify

namespace MyGame.Example
{

using FlatBuffers;

public sealed class Monster : Table {
  public static Monster GetRootAsMonster(ByteBuffer _bb) { return GetRootAsMonster(_bb, new Monster()); }
  public static Monster GetRootAsMonster(ByteBuffer _bb, Monster obj) { return (obj.__init(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public static bool MonsterBufferHasIdentifier(ByteBuffer _bb) { return __has_identifier(_bb, "MONS"); }
  public Monster __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Vec3 Pos { get { return GetPos(new Vec3()); } }
  public Vec3 GetPos(Vec3 obj) { int o = __offset(4); return o != 0 ? obj.__init(o + bb_pos, bb) : null; }
  public short Mana { get { int o = __offset(6); return o != 0 ? bb.GetShort(o + bb_pos) : (short)150; } }
  public short Hp { get { int o = __offset(8); return o != 0 ? bb.GetShort(o + bb_pos) : (short)100; } }
  public string Name { get { int o = __offset(10); return o != 0 ? __string(o + bb_pos) : null; } }
  public byte GetInventory(int j) { int o = __offset(14); return o != 0 ? bb.Get(__vector(o) + j * 1) : (byte)0; }
  public int InventoryLength { get { int o = __offset(14); return o != 0 ? __vector_len(o) : 0; } }
  public Color Color { get { int o = __offset(16); return o != 0 ? (Color)bb.GetSbyte(o + bb_pos) : (Color)8; } }
  public Any TestType { get { int o = __offset(18); return o != 0 ? (Any)bb.Get(o + bb_pos) : (Any)0; } }
  public TTable GetTest<TTable>(TTable obj) where TTable : Table { int o = __offset(20); return o != 0 ? __union(obj, o) : null; }
  public Test GetTest4(int j) { return GetTest4(new Test(), j); }
  public Test GetTest4(Test obj, int j) { int o = __offset(22); return o != 0 ? obj.__init(__vector(o) + j * 4, bb) : null; }
  public int Test4Length { get { int o = __offset(22); return o != 0 ? __vector_len(o) : 0; } }
  public string GetTestarrayofstring(int j) { int o = __offset(24); return o != 0 ? __string(__vector(o) + j * 4) : null; }
  public int TestarrayofstringLength { get { int o = __offset(24); return o != 0 ? __vector_len(o) : 0; } }
  /// an example documentation comment: this will end up in the generated code
  /// multiline too
  public Monster GetTestarrayoftables(int j) { return GetTestarrayoftables(new Monster(), j); }
  public Monster GetTestarrayoftables(Monster obj, int j) { int o = __offset(26); return o != 0 ? obj.__init(__indirect(__vector(o) + j * 4), bb) : null; }
  public int TestarrayoftablesLength { get { int o = __offset(26); return o != 0 ? __vector_len(o) : 0; } }
  public Monster Enemy { get { return GetEnemy(new Monster()); } }
  public Monster GetEnemy(Monster obj) { int o = __offset(28); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public byte GetTestnestedflatbuffer(int j) { int o = __offset(30); return o != 0 ? bb.Get(__vector(o) + j * 1) : (byte)0; }
  public int TestnestedflatbufferLength { get { int o = __offset(30); return o != 0 ? __vector_len(o) : 0; } }
  public Stat Testempty { get { return GetTestempty(new Stat()); } }
  public Stat GetTestempty(Stat obj) { int o = __offset(32); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public bool Testbool { get { int o = __offset(34); return o != 0 ? 0!=bb.Get(o + bb_pos) : (bool)false; } }
  public int Testhashs32Fnv1 { get { int o = __offset(36); return o != 0 ? bb.GetInt(o + bb_pos) : (int)0; } }
  public uint Testhashu32Fnv1 { get { int o = __offset(38); return o != 0 ? bb.GetUint(o + bb_pos) : (uint)0; } }
  public long Testhashs64Fnv1 { get { int o = __offset(40); return o != 0 ? bb.GetLong(o + bb_pos) : (long)0; } }
  public ulong Testhashu64Fnv1 { get { int o = __offset(42); return o != 0 ? bb.GetUlong(o + bb_pos) : (ulong)0; } }
  public int Testhashs32Fnv1a { get { int o = __offset(44); return o != 0 ? bb.GetInt(o + bb_pos) : (int)0; } }
  public uint Testhashu32Fnv1a { get { int o = __offset(46); return o != 0 ? bb.GetUint(o + bb_pos) : (uint)0; } }
  public long Testhashs64Fnv1a { get { int o = __offset(48); return o != 0 ? bb.GetLong(o + bb_pos) : (long)0; } }
  public ulong Testhashu64Fnv1a { get { int o = __offset(50); return o != 0 ? bb.GetUlong(o + bb_pos) : (ulong)0; } }
  public bool GetTestarrayofbools(int j) { int o = __offset(52); return o != 0 ? 0!=bb.Get(__vector(o) + j * 1) : false; }
  public int TestarrayofboolsLength { get { int o = __offset(52); return o != 0 ? __vector_len(o) : 0; } }

  public static void StartMonster(FlatBufferBuilder builder) { builder.StartObject(25); }
  public static void AddPos(FlatBufferBuilder builder, Offset<Vec3> posOffset) { builder.AddStruct(0, posOffset.Value, 0); }
  public static void AddMana(FlatBufferBuilder builder, short mana) { builder.AddShort(1, mana, 150); }
  public static void AddHp(FlatBufferBuilder builder, short hp) { builder.AddShort(2, hp, 100); }
  public static void AddName(FlatBufferBuilder builder, StringOffset nameOffset) { builder.AddOffset(3, nameOffset.Value, 0); }
  public static void AddInventory(FlatBufferBuilder builder, VectorOffset inventoryOffset) { builder.AddOffset(5, inventoryOffset.Value, 0); }
  public static VectorOffset CreateInventoryVector(FlatBufferBuilder builder, byte[] data) { builder.StartVector(1, data.Length, 1); for (int i = data.Length - 1; i >= 0; i--) builder.AddByte(data[i]); return builder.EndVector(); }
  public static void StartInventoryVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(1, numElems, 1); }
  public static void AddColor(FlatBufferBuilder builder, Color color) { builder.AddSbyte(6, (sbyte)(color), 8); }
  public static void AddTestType(FlatBufferBuilder builder, Any testType) { builder.AddByte(7, (byte)(testType), 0); }
  public static void AddTest(FlatBufferBuilder builder, int testOffset) { builder.AddOffset(8, testOffset, 0); }
  public static void AddTest4(FlatBufferBuilder builder, VectorOffset test4Offset) { builder.AddOffset(9, test4Offset.Value, 0); }
  public static void StartTest4Vector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems, 2); }
  public static void AddTestarrayofstring(FlatBufferBuilder builder, VectorOffset testarrayofstringOffset) { builder.AddOffset(10, testarrayofstringOffset.Value, 0); }
  public static VectorOffset CreateTestarrayofstringVector(FlatBufferBuilder builder, StringOffset[] data) { builder.StartVector(4, data.Length, 4); for (int i = data.Length - 1; i >= 0; i--) builder.AddOffset(data[i].Value); return builder.EndVector(); }
  public static void StartTestarrayofstringVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems, 4); }
  public static void AddTestarrayoftables(FlatBufferBuilder builder, VectorOffset testarrayoftablesOffset) { builder.AddOffset(11, testarrayoftablesOffset.Value, 0); }
  public static VectorOffset CreateTestarrayoftablesVector(FlatBufferBuilder builder, Offset<Monster>[] data) { builder.StartVector(4, data.Length, 4); for (int i = data.Length - 1; i >= 0; i--) builder.AddOffset(data[i].Value); return builder.EndVector(); }
  public static void StartTestarrayoftablesVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(4, numElems, 4); }
  public static void AddEnemy(FlatBufferBuilder builder, Offset<Monster> enemyOffset) { builder.AddOffset(12, enemyOffset.Value, 0); }
  public static void AddTestnestedflatbuffer(FlatBufferBuilder builder, VectorOffset testnestedflatbufferOffset) { builder.AddOffset(13, testnestedflatbufferOffset.Value, 0); }
  public static VectorOffset CreateTestnestedflatbufferVector(FlatBufferBuilder builder, byte[] data) { builder.StartVector(1, data.Length, 1); for (int i = data.Length - 1; i >= 0; i--) builder.AddByte(data[i]); return builder.EndVector(); }
  public static void StartTestnestedflatbufferVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(1, numElems, 1); }
  public static void AddTestempty(FlatBufferBuilder builder, Offset<Stat> testemptyOffset) { builder.AddOffset(14, testemptyOffset.Value, 0); }
  public static void AddTestbool(FlatBufferBuilder builder, bool testbool) { builder.AddBool(15, testbool, false); }
  public static void AddTesthashs32Fnv1(FlatBufferBuilder builder, int testhashs32Fnv1) { builder.AddInt(16, testhashs32Fnv1, 0); }
  public static void AddTesthashu32Fnv1(FlatBufferBuilder builder, uint testhashu32Fnv1) { builder.AddUint(17, testhashu32Fnv1, 0); }
  public static void AddTesthashs64Fnv1(FlatBufferBuilder builder, long testhashs64Fnv1) { builder.AddLong(18, testhashs64Fnv1, 0); }
  public static void AddTesthashu64Fnv1(FlatBufferBuilder builder, ulong testhashu64Fnv1) { builder.AddUlong(19, testhashu64Fnv1, 0); }
  public static void AddTesthashs32Fnv1a(FlatBufferBuilder builder, int testhashs32Fnv1a) { builder.AddInt(20, testhashs32Fnv1a, 0); }
  public static void AddTesthashu32Fnv1a(FlatBufferBuilder builder, uint testhashu32Fnv1a) { builder.AddUint(21, testhashu32Fnv1a, 0); }
  public static void AddTesthashs64Fnv1a(FlatBufferBuilder builder, long testhashs64Fnv1a) { builder.AddLong(22, testhashs64Fnv1a, 0); }
  public static void AddTesthashu64Fnv1a(FlatBufferBuilder builder, ulong testhashu64Fnv1a) { builder.AddUlong(23, testhashu64Fnv1a, 0); }
  public static void AddTestarrayofbools(FlatBufferBuilder builder, VectorOffset testarrayofboolsOffset) { builder.AddOffset(24, testarrayofboolsOffset.Value, 0); }
  public static VectorOffset CreateTestarrayofboolsVector(FlatBufferBuilder builder, bool[] data) { builder.StartVector(1, data.Length, 1); for (int i = data.Length - 1; i >= 0; i--) builder.AddBool(data[i]); return builder.EndVector(); }
  public static void StartTestarrayofboolsVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(1, numElems, 1); }
  public static Offset<Monster> EndMonster(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    builder.Required(o, 10);  // name
    return new Offset<Monster>(o);
  }
  public static void FinishMonsterBuffer(FlatBufferBuilder builder, Offset<Monster> offset) { builder.Finish(offset.Value, "MONS"); }
};


}
