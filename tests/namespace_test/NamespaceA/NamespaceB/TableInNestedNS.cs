// automatically generated, do not modify

namespace NamespaceA.NamespaceB
{

using System;
using FlatBuffers;

public struct TableInNestedNS : ITable {
  private readonly TablePos pos;

  public TableInNestedNS(int _i, ByteBuffer _bb) { this.pos = new TablePos(_i, _bb); }
  public TableInNestedNS(TablePos pos) { this.pos = pos; }

  ByteBuffer IFieldGroup.ByteBuffer { get { return this.pos.bb; } }
  TablePos ITable.TablePos { get { return this.pos; } }

  public static TableInNestedNS GetRootAsTableInNestedNS(ByteBuffer _bb) { return (new TableInNestedNS(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }

  public int Foo { get { int o = this.pos.__offset(4); return o != 0 ? this.pos.bb.GetInt(o + this.pos.bb_pos) : (int)0; } }
  public bool MutateFoo(int foo) { int o = this.pos.__offset(4); if (o != 0) { this.pos.bb.PutInt(o + this.pos.bb_pos, foo); return true; } else { return false; } }

  public static Offset<TableInNestedNS> CreateTableInNestedNS(FlatBufferBuilder builder,
      int foo = 0) {
    builder.StartObject(1);
    TableInNestedNS.AddFoo(builder, foo);
    return TableInNestedNS.EndTableInNestedNS(builder);
  }

  public static void StartTableInNestedNS(FlatBufferBuilder builder) { builder.StartObject(1); }
  public static void AddFoo(FlatBufferBuilder builder, int foo) { builder.AddInt(0, foo, 0); }
  public static Offset<TableInNestedNS> EndTableInNestedNS(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<TableInNestedNS>(o);
  }
};


}
