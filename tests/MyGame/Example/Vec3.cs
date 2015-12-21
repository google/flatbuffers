// automatically generated, do not modify

namespace MyGame.Example
{

using System;
using FlatBuffers;

public struct Vec3 : IStruct {
  private readonly StructPos pos;

  public Vec3(int _i, ByteBuffer _bb) { this.pos = new StructPos(_i, _bb); }
  public Vec3(StructPos pos) { this.pos = pos; }

  ByteBuffer IFieldGroup.ByteBuffer { get { return this.pos.bb; } }
  StructPos IStruct.StructPos { get { return this.pos; } }


  public float X { get { return this.pos.bb.GetFloat(this.pos.bb_pos + 0); } }
  public void MutateX(float x) { this.pos.bb.PutFloat(this.pos.bb_pos + 0, x); }
  public float Y { get { return this.pos.bb.GetFloat(this.pos.bb_pos + 4); } }
  public void MutateY(float y) { this.pos.bb.PutFloat(this.pos.bb_pos + 4, y); }
  public float Z { get { return this.pos.bb.GetFloat(this.pos.bb_pos + 8); } }
  public void MutateZ(float z) { this.pos.bb.PutFloat(this.pos.bb_pos + 8, z); }
  public double Test1 { get { return this.pos.bb.GetDouble(this.pos.bb_pos + 16); } }
  public void MutateTest1(double test1) { this.pos.bb.PutDouble(this.pos.bb_pos + 16, test1); }
  public Color Test2 { get { return (Color)this.pos.bb.GetSbyte(this.pos.bb_pos + 24); } }
  public void MutateTest2(Color test2) { this.pos.bb.PutSbyte(this.pos.bb_pos + 24, (sbyte)test2); }
  public Test Test3 { get { return new Test(this.pos.bb_pos + 26, this.pos.bb); } }

  public static Offset<Vec3> CreateVec3(FlatBufferBuilder builder, float X, float Y, float Z, double Test1, Color Test2, short test3_A, sbyte test3_B) {
    builder.Prep(16, 32);
    builder.Pad(2);
    builder.Prep(2, 4);
    builder.Pad(1);
    builder.PutSbyte(test3_B);
    builder.PutShort(test3_A);
    builder.Pad(1);
    builder.PutSbyte((sbyte)Test2);
    builder.PutDouble(Test1);
    builder.Pad(4);
    builder.PutFloat(Z);
    builder.PutFloat(Y);
    builder.PutFloat(X);
    return new Offset<Vec3>(builder.Offset);
  }
};


}
