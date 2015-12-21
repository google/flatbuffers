// automatically generated, do not modify

namespace MyGame.Example
{

using System;
using FlatBuffers;

/// an example documentation comment: monster object
public struct Monster : ITable {
  private readonly TablePos pos;

  public Monster(int _i, ByteBuffer _bb) { this.pos = new TablePos(_i, _bb); }
  public Monster(TablePos pos) { this.pos = pos; }

  ByteBuffer IFieldGroup.ByteBuffer { get { return this.pos.bb; } }
  TablePos ITable.TablePos { get { return this.pos; } }

  public static Monster GetRootAsMonster(ByteBuffer _bb) { return (new Monster(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public static bool MonsterBufferHasIdentifier(ByteBuffer _bb) { return TablePos.__has_identifier(_bb, "MONS"); }

  public Vec3? Pos { get { int o = this.pos.__offset(4); return o != 0 ? new Vec3(o + this.pos.bb_pos, this.pos.bb) : (Vec3?)null; } }
  public short Mana { get { int o = this.pos.__offset(6); return o != 0 ? this.pos.bb.GetShort(o + this.pos.bb_pos) : (short)150; } }
  public bool MutateMana(short mana) { int o = this.pos.__offset(6); if (o != 0) { this.pos.bb.PutShort(o + this.pos.bb_pos, mana); return true; } else { return false; } }
  public short Hp { get { int o = this.pos.__offset(8); return o != 0 ? this.pos.bb.GetShort(o + this.pos.bb_pos) : (short)100; } }
  public bool MutateHp(short hp) { int o = this.pos.__offset(8); if (o != 0) { this.pos.bb.PutShort(o + this.pos.bb_pos, hp); return true; } else { return false; } }
  public string Name { get { int o = this.pos.__offset(10); return o != 0 ? this.pos.__string(o + this.pos.bb_pos) : null; } }
  public ArraySegment<byte>? GetNameBytes() { return this.pos.__vector_as_arraysegment(10); }
  public byte GetInventory(int j) { int o = this.pos.__offset(14); return o != 0 ? this.pos.bb.Get(this.pos.__vector(o) + j * 1) : (byte)0; }
  public int InventoryLength { get { int o = this.pos.__offset(14); return o != 0 ? this.pos.__vector_len(o) : 0; } }
  public ArraySegment<byte>? GetInventoryBytes() { return this.pos.__vector_as_arraysegment(14); }
  public bool MutateInventory(int j, byte inventory) { int o = this.pos.__offset(14); if (o != 0) { this.pos.bb.Put(this.pos.__vector(o) + j * 1, inventory); return true; } else { return false; } }
  public Color Color { get { int o = this.pos.__offset(16); return o != 0 ? (Color)this.pos.bb.GetSbyte(o + this.pos.bb_pos) : Color.Blue; } }
  public bool MutateColor(Color color) { int o = this.pos.__offset(16); if (o != 0) { this.pos.bb.PutSbyte(o + this.pos.bb_pos, (sbyte)color); return true; } else { return false; } }
  public Any TestType { get { int o = this.pos.__offset(18); return o != 0 ? (Any)this.pos.bb.Get(o + this.pos.bb_pos) : Any.NONE; } }
  public bool MutateTestType(Any test_type) { int o = this.pos.__offset(18); if (o != 0) { this.pos.bb.Put(o + this.pos.bb_pos, (byte)test_type); return true; } else { return false; } }
  public Monster? GetTestAsMonster() { int o = this.pos.__offset(20); return o != 0 ? new Monster(this.pos.__union(o)) : (Monster?)null; }
  public TestSimpleTableWithEnum? GetTestAsTestSimpleTableWithEnum() { int o = this.pos.__offset(20); return o != 0 ? new TestSimpleTableWithEnum(this.pos.__union(o)) : (TestSimpleTableWithEnum?)null; }
  public Test? GetTest4(int j) { int o = this.pos.__offset(22); return o != 0 ? new Test(this.pos.__vector(o) + j * 4, this.pos.bb) : (Test?)null; }
  public int Test4Length { get { int o = this.pos.__offset(22); return o != 0 ? this.pos.__vector_len(o) : 0; } }
  public string GetTestarrayofstring(int j) { int o = this.pos.__offset(24); return o != 0 ? this.pos.__string(this.pos.__vector(o) + j * 4) : null; }
  public int TestarrayofstringLength { get { int o = this.pos.__offset(24); return o != 0 ? this.pos.__vector_len(o) : 0; } }
  /// an example documentation comment: this will end up in the generated code
  /// multiline too
  public Monster? GetTestarrayoftables(int j) { int o = this.pos.__offset(26); return o != 0 ? new Monster(this.pos.__indirect(this.pos.__vector(o) + j * 4), this.pos.bb) : (Monster?)null; }
  public int TestarrayoftablesLength { get { int o = this.pos.__offset(26); return o != 0 ? this.pos.__vector_len(o) : 0; } }
  public Monster? Enemy { get { int o = this.pos.__offset(28); return o != 0 ? new Monster(this.pos.__indirect(o + this.pos.bb_pos), this.pos.bb) : (Monster?)null; } }
  public byte GetTestnestedflatbuffer(int j) { int o = this.pos.__offset(30); return o != 0 ? this.pos.bb.Get(this.pos.__vector(o) + j * 1) : (byte)0; }
  public int TestnestedflatbufferLength { get { int o = this.pos.__offset(30); return o != 0 ? this.pos.__vector_len(o) : 0; } }
  public ArraySegment<byte>? GetTestnestedflatbufferBytes() { return this.pos.__vector_as_arraysegment(30); }
  public bool MutateTestnestedflatbuffer(int j, byte testnestedflatbuffer) { int o = this.pos.__offset(30); if (o != 0) { this.pos.bb.Put(this.pos.__vector(o) + j * 1, testnestedflatbuffer); return true; } else { return false; } }
  public Stat? Testempty { get { int o = this.pos.__offset(32); return o != 0 ? new Stat(this.pos.__indirect(o + this.pos.bb_pos), this.pos.bb) : (Stat?)null; } }
  public bool Testbool { get { int o = this.pos.__offset(34); return o != 0 ? 0!=this.pos.bb.Get(o + this.pos.bb_pos) : (bool)false; } }
  public bool MutateTestbool(bool testbool) { int o = this.pos.__offset(34); if (o != 0) { this.pos.bb.Put(o + this.pos.bb_pos, (byte)(testbool ? 1 : 0)); return true; } else { return false; } }
  public int Testhashs32Fnv1 { get { int o = this.pos.__offset(36); return o != 0 ? this.pos.bb.GetInt(o + this.pos.bb_pos) : (int)0; } }
  public bool MutateTesthashs32Fnv1(int testhashs32_fnv1) { int o = this.pos.__offset(36); if (o != 0) { this.pos.bb.PutInt(o + this.pos.bb_pos, testhashs32_fnv1); return true; } else { return false; } }
  public uint Testhashu32Fnv1 { get { int o = this.pos.__offset(38); return o != 0 ? this.pos.bb.GetUint(o + this.pos.bb_pos) : (uint)0; } }
  public bool MutateTesthashu32Fnv1(uint testhashu32_fnv1) { int o = this.pos.__offset(38); if (o != 0) { this.pos.bb.PutUint(o + this.pos.bb_pos, testhashu32_fnv1); return true; } else { return false; } }
  public long Testhashs64Fnv1 { get { int o = this.pos.__offset(40); return o != 0 ? this.pos.bb.GetLong(o + this.pos.bb_pos) : (long)0; } }
  public bool MutateTesthashs64Fnv1(long testhashs64_fnv1) { int o = this.pos.__offset(40); if (o != 0) { this.pos.bb.PutLong(o + this.pos.bb_pos, testhashs64_fnv1); return true; } else { return false; } }
  public ulong Testhashu64Fnv1 { get { int o = this.pos.__offset(42); return o != 0 ? this.pos.bb.GetUlong(o + this.pos.bb_pos) : (ulong)0; } }
  public bool MutateTesthashu64Fnv1(ulong testhashu64_fnv1) { int o = this.pos.__offset(42); if (o != 0) { this.pos.bb.PutUlong(o + this.pos.bb_pos, testhashu64_fnv1); return true; } else { return false; } }
  public int Testhashs32Fnv1a { get { int o = this.pos.__offset(44); return o != 0 ? this.pos.bb.GetInt(o + this.pos.bb_pos) : (int)0; } }
  public bool MutateTesthashs32Fnv1a(int testhashs32_fnv1a) { int o = this.pos.__offset(44); if (o != 0) { this.pos.bb.PutInt(o + this.pos.bb_pos, testhashs32_fnv1a); return true; } else { return false; } }
  public uint Testhashu32Fnv1a { get { int o = this.pos.__offset(46); return o != 0 ? this.pos.bb.GetUint(o + this.pos.bb_pos) : (uint)0; } }
  public bool MutateTesthashu32Fnv1a(uint testhashu32_fnv1a) { int o = this.pos.__offset(46); if (o != 0) { this.pos.bb.PutUint(o + this.pos.bb_pos, testhashu32_fnv1a); return true; } else { return false; } }
  public long Testhashs64Fnv1a { get { int o = this.pos.__offset(48); return o != 0 ? this.pos.bb.GetLong(o + this.pos.bb_pos) : (long)0; } }
  public bool MutateTesthashs64Fnv1a(long testhashs64_fnv1a) { int o = this.pos.__offset(48); if (o != 0) { this.pos.bb.PutLong(o + this.pos.bb_pos, testhashs64_fnv1a); return true; } else { return false; } }
  public ulong Testhashu64Fnv1a { get { int o = this.pos.__offset(50); return o != 0 ? this.pos.bb.GetUlong(o + this.pos.bb_pos) : (ulong)0; } }
  public bool MutateTesthashu64Fnv1a(ulong testhashu64_fnv1a) { int o = this.pos.__offset(50); if (o != 0) { this.pos.bb.PutUlong(o + this.pos.bb_pos, testhashu64_fnv1a); return true; } else { return false; } }
  public bool GetTestarrayofbools(int j) { int o = this.pos.__offset(52); return o != 0 ? 0!=this.pos.bb.Get(this.pos.__vector(o) + j * 1) : false; }
  public int TestarrayofboolsLength { get { int o = this.pos.__offset(52); return o != 0 ? this.pos.__vector_len(o) : 0; } }
  public ArraySegment<byte>? GetTestarrayofboolsBytes() { return this.pos.__vector_as_arraysegment(52); }
  public bool MutateTestarrayofbools(int j, bool testarrayofbools) { int o = this.pos.__offset(52); if (o != 0) { this.pos.bb.Put(this.pos.__vector(o) + j * 1, (byte)(testarrayofbools ? 1 : 0)); return true; } else { return false; } }

  public static void StartMonster(FlatBufferBuilder builder) { builder.StartObject(25); }
  public static void AddPos(FlatBufferBuilder builder, Offset<Vec3> posOffset) { builder.AddStruct(0, posOffset.Value, 0); }
  public static void AddMana(FlatBufferBuilder builder, short mana) { builder.AddShort(1, mana, 150); }
  public static void AddHp(FlatBufferBuilder builder, short hp) { builder.AddShort(2, hp, 100); }
  public static void AddName(FlatBufferBuilder builder, StringOffset nameOffset) { builder.AddOffset(3, nameOffset.Value, 0); }
  public static void AddInventory(FlatBufferBuilder builder, VectorOffset inventoryOffset) { builder.AddOffset(5, inventoryOffset.Value, 0); }
  public static VectorOffset CreateInventoryVector(FlatBufferBuilder builder, byte[] data) { builder.StartVector(1, data.Length, 1); for (int i = data.Length - 1; i >= 0; i--) builder.AddByte(data[i]); return builder.EndVector(); }
  public static void StartInventoryVector(FlatBufferBuilder builder, int numElems) { builder.StartVector(1, numElems, 1); }
  public static void AddColor(FlatBufferBuilder builder, Color color) { builder.AddSbyte(6, (sbyte)color, 8); }
  public static void AddTestType(FlatBufferBuilder builder, Any testType) { builder.AddByte(7, (byte)testType, 0); }
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
