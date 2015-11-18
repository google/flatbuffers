// automatically generated, do not modify

namespace MyGame.Example
{

using FlatBuffers;

public sealed class TestSimpleTableWithEnum : Table {
  public static TestSimpleTableWithEnum GetRootAsTestSimpleTableWithEnum(ByteBuffer _bb) { return GetRootAsTestSimpleTableWithEnum(_bb, new TestSimpleTableWithEnum()); }
  public static TestSimpleTableWithEnum GetRootAsTestSimpleTableWithEnum(ByteBuffer _bb, TestSimpleTableWithEnum obj) { return (obj.__init(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public TestSimpleTableWithEnum __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public Color Color { get { int o = __offset(4); return o != 0 ? (Color)bb.GetSbyte(o + bb_pos) : (Color)2; } }
  public bool MutateColor(Color color) { int o = __offset(4); if (o != 0) { bb.PutSbyte(o + bb_pos, (sbyte)color); return true; } else { return false; } }

  public static Offset<TestSimpleTableWithEnum> CreateTestSimpleTableWithEnum(FlatBufferBuilder builder,
      Color color = Color.Green) {
    builder.StartObject(1);
    TestSimpleTableWithEnum.AddColor(builder, color);
    return TestSimpleTableWithEnum.EndTestSimpleTableWithEnum(builder);
  }

  public static void StartTestSimpleTableWithEnum(FlatBufferBuilder builder) { builder.StartObject(1); }
  public static void AddColor(FlatBufferBuilder builder, Color color) { builder.AddSbyte(0, (sbyte)(color), 2); }
  public static Offset<TestSimpleTableWithEnum> EndTestSimpleTableWithEnum(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<TestSimpleTableWithEnum>(o);
  }
};


}
