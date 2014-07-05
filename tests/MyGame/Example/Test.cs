// automatically generated, do not modify

namespace MyGame.Example
{

using FlatBuffersNet;

public class Test : Struct {
  public Test __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }
  public short A() { return bb.GetShort(bb_pos + 0); }
  public byte B() { return bb.Get(bb_pos + 2); }

  public static int CreateTest(FlatBufferBuilder builder, short A, byte B) {
    builder.Prep(2, 4);
    builder.Pad(1);
    builder.PutByte(B);
    builder.PutShort(A);
    return builder.Offset;
  }
};


}
