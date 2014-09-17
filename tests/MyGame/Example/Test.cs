// automatically generated, do not modify

namespace MyGame.Example
{

using FlatBuffers;

public class Test : Struct {
  public Test __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public short A() { return bb.GetShort(bb_pos + 0); }
  public sbyte B() { return bb.GetSbyte(bb_pos + 2); }

  public static int CreateTest(FlatBufferBuilder builder, short A, sbyte B) {
    builder.Prep(2, 4);
    builder.Pad(1);
    builder.PutSbyte(B);
    builder.PutShort(A);
    return builder.Offset();
  }
};


}
