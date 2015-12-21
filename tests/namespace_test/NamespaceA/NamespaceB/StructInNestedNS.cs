// automatically generated, do not modify

namespace NamespaceA.NamespaceB
{

using System;
using FlatBuffers;

public struct StructInNestedNS : IStruct {
  private readonly StructPos pos;

  public StructInNestedNS(int _i, ByteBuffer _bb) { this.pos = new StructPos(_i, _bb); }
  public StructInNestedNS(StructPos pos) { this.pos = pos; }

  ByteBuffer IFieldGroup.ByteBuffer { get { return this.pos.bb; } }
  StructPos IStruct.StructPos { get { return this.pos; } }


  public int A { get { return this.pos.bb.GetInt(this.pos.bb_pos + 0); } }
  public void MutateA(int a) { this.pos.bb.PutInt(this.pos.bb_pos + 0, a); }
  public int B { get { return this.pos.bb.GetInt(this.pos.bb_pos + 4); } }
  public void MutateB(int b) { this.pos.bb.PutInt(this.pos.bb_pos + 4, b); }

  public static Offset<StructInNestedNS> CreateStructInNestedNS(FlatBufferBuilder builder, int A, int B) {
    builder.Prep(4, 8);
    builder.PutInt(B);
    builder.PutInt(A);
    return new Offset<StructInNestedNS>(builder.Offset);
  }
};


}
