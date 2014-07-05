// automatically generated, do not modify

namespace MyGame.Example
{

using FlatBuffersNet;

public class Vec3 : Struct {
  public Vec3 __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }
  public float X() { return bb.GetFloat(bb_pos + 0); }
  public float Y() { return bb.GetFloat(bb_pos + 4); }
  public float Z() { return bb.GetFloat(bb_pos + 8); }
  public double Test1() { return bb.GetDouble(bb_pos + 16); }
  public byte Test2() { return bb.Get(bb_pos + 24); }
  public Test Test3() { return Test3(new Test()); }
  public Test Test3(Test obj) { return obj.__init(bb_pos + 26, bb); }

  public static int CreateVec3(FlatBufferBuilder builder, float X, float Y, float Z, double Test1, byte Test2, short Test_A, byte Test_B) {
    builder.Prep(16, 32);
    builder.Pad(2);
    builder.Prep(2, 4);
    builder.Pad(1);
    builder.PutByte(Test_B);
    builder.PutShort(Test_A);
    builder.Pad(1);
    builder.PutByte(Test2);
    builder.PutDouble(Test1);
    builder.Pad(4);
    builder.PutFloat(Z);
    builder.PutFloat(Y);
    builder.PutFloat(X);
    return builder.Offset;
  }
};


}
