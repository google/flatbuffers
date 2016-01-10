// automatically generated, do not modify

namespace MyGame.Example
{

using System;
using FlatBuffers;

public struct TestSimpleTableWithEnum : ITable<TestSimpleTableWithEnum> {
  private readonly TablePos pos;

  public TestSimpleTableWithEnum(int _i, ByteBuffer _bb) { this.pos = new TablePos(_i, _bb); }
  public TestSimpleTableWithEnum(TablePos pos) { this.pos = pos; }

  ByteBuffer IFieldGroup.ByteBuffer { get { return this.pos.bb; } }
  TablePos ITable.TablePos { get { return this.pos; } }
  TestSimpleTableWithEnum ITable<TestSimpleTableWithEnum>.Construct(TablePos pos) { return new TestSimpleTableWithEnum(pos); }

  public static TestSimpleTableWithEnum GetRootAsTestSimpleTableWithEnum(ByteBuffer _bb) { return (new TestSimpleTableWithEnum(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }

  public Color Color { get { int o = this.pos.__offset(4); return o != 0 ? (Color)this.pos.bb.GetSbyte(o + this.pos.bb_pos) : Color.Green; } }
  public bool MutateColor(Color color) { int o = this.pos.__offset(4); if (o != 0) { this.pos.bb.PutSbyte(o + this.pos.bb_pos, (sbyte)color); return true; } else { return false; } }

  public static Offset<TestSimpleTableWithEnum> CreateTestSimpleTableWithEnum(FlatBufferBuilder builder,
      Color color = Color.Green) {
    builder.StartObject(1);
    TestSimpleTableWithEnum.AddColor(builder, color);
    return TestSimpleTableWithEnum.EndTestSimpleTableWithEnum(builder);
  }

  public static void StartTestSimpleTableWithEnum(FlatBufferBuilder builder) { builder.StartObject(1); }
  public static void AddColor(FlatBufferBuilder builder, Color color) { builder.AddSbyte(0, (sbyte)color, 2); }
  public static Offset<TestSimpleTableWithEnum> EndTestSimpleTableWithEnum(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<TestSimpleTableWithEnum>(o);
  }
};


}
