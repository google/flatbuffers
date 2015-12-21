// automatically generated, do not modify

namespace NamespaceA
{

using System;
using FlatBuffers;

public struct TableInFirstNS : ITable<TableInFirstNS> {
  private readonly TablePos pos;

  public TableInFirstNS(int _i, ByteBuffer _bb) { this.pos = new TablePos(_i, _bb); }
  public TableInFirstNS(TablePos pos) { this.pos = pos; }

  ByteBuffer IFieldGroup.ByteBuffer { get { return this.pos.bb; } }
  TablePos ITable.TablePos { get { return this.pos; } }
  TableInFirstNS ITable<TableInFirstNS>.Construct(TablePos pos) { return new TableInFirstNS(pos); }

  public static TableInFirstNS GetRootAsTableInFirstNS(ByteBuffer _bb) { return (new TableInFirstNS(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }

  public NamespaceA.NamespaceB.TableInNestedNS? FooTable { get { int o = this.pos.__offset(4); return o != 0 ? new NamespaceA.NamespaceB.TableInNestedNS(this.pos.__indirect(o + this.pos.bb_pos), this.pos.bb) : (NamespaceA.NamespaceB.TableInNestedNS?)null; } }
  public NamespaceA.NamespaceB.EnumInNestedNS FooEnum { get { int o = this.pos.__offset(6); return o != 0 ? (NamespaceA.NamespaceB.EnumInNestedNS)this.pos.bb.GetSbyte(o + this.pos.bb_pos) : NamespaceA.NamespaceB.EnumInNestedNS.A; } }
  public bool MutateFooEnum(NamespaceA.NamespaceB.EnumInNestedNS foo_enum) { int o = this.pos.__offset(6); if (o != 0) { this.pos.bb.PutSbyte(o + this.pos.bb_pos, (sbyte)foo_enum); return true; } else { return false; } }
  public NamespaceA.NamespaceB.StructInNestedNS? FooStruct { get { int o = this.pos.__offset(8); return o != 0 ? new NamespaceA.NamespaceB.StructInNestedNS(o + this.pos.bb_pos, this.pos.bb) : (NamespaceA.NamespaceB.StructInNestedNS?)null; } }

  public static void StartTableInFirstNS(FlatBufferBuilder builder) { builder.StartObject(3); }
  public static void AddFooTable(FlatBufferBuilder builder, Offset<NamespaceA.NamespaceB.TableInNestedNS> fooTableOffset) { builder.AddOffset(0, fooTableOffset.Value, 0); }
  public static void AddFooEnum(FlatBufferBuilder builder, NamespaceA.NamespaceB.EnumInNestedNS fooEnum) { builder.AddSbyte(1, (sbyte)fooEnum, 0); }
  public static void AddFooStruct(FlatBufferBuilder builder, Offset<NamespaceA.NamespaceB.StructInNestedNS> fooStructOffset) { builder.AddStruct(2, fooStructOffset.Value, 0); }
  public static Offset<TableInFirstNS> EndTableInFirstNS(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<TableInFirstNS>(o);
  }
};


}
