// automatically generated, do not modify

namespace MyGame.Example
{

using System;
using FlatBuffers;

public struct Test : IStruct {
  private readonly StructPos pos;

  public Test(int _i, ByteBuffer _bb) { this.pos = new StructPos(_i, _bb); }
  public Test(StructPos pos) { this.pos = pos; }

  ByteBuffer IFieldGroup.ByteBuffer { get { return this.pos.bb; } }
  StructPos IStruct.StructPos { get { return this.pos; } }


  public short A { get { return this.pos.bb.GetShort(this.pos.bb_pos + 0); } }
  public void MutateA(short a) { this.pos.bb.PutShort(this.pos.bb_pos + 0, a); }
  public sbyte B { get { return this.pos.bb.GetSbyte(this.pos.bb_pos + 2); } }
  public void MutateB(sbyte b) { this.pos.bb.PutSbyte(this.pos.bb_pos + 2, b); }

  public static Offset<Test> CreateTest(FlatBufferBuilder builder, short A, sbyte B) {
    builder.Prep(2, 4);
    builder.Pad(1);
    builder.PutSbyte(B);
    builder.PutShort(A);
    return new Offset<Test>(builder.Offset);
  }
};


}
