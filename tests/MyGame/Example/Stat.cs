// automatically generated, do not modify

namespace MyGame.Example
{

using System;
using FlatBuffers;

public struct Stat : ITable {
  private readonly TablePos pos;

  public Stat(int _i, ByteBuffer _bb) { this.pos = new TablePos(_i, _bb); }
  public Stat(TablePos pos) { this.pos = pos; }

  ByteBuffer IFieldGroup.ByteBuffer { get { return this.pos.bb; } }
  TablePos ITable.TablePos { get { return this.pos; } }

  public static Stat GetRootAsStat(ByteBuffer _bb) { return (new Stat(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }

  public string Id { get { int o = this.pos.__offset(4); return o != 0 ? this.pos.__string(o + this.pos.bb_pos) : null; } }
  public ArraySegment<byte>? GetIdBytes() { return this.pos.__vector_as_arraysegment(4); }
  public long Val { get { int o = this.pos.__offset(6); return o != 0 ? this.pos.bb.GetLong(o + this.pos.bb_pos) : (long)0; } }
  public bool MutateVal(long val) { int o = this.pos.__offset(6); if (o != 0) { this.pos.bb.PutLong(o + this.pos.bb_pos, val); return true; } else { return false; } }
  public ushort Count { get { int o = this.pos.__offset(8); return o != 0 ? this.pos.bb.GetUshort(o + this.pos.bb_pos) : (ushort)0; } }
  public bool MutateCount(ushort count) { int o = this.pos.__offset(8); if (o != 0) { this.pos.bb.PutUshort(o + this.pos.bb_pos, count); return true; } else { return false; } }

  public static Offset<Stat> CreateStat(FlatBufferBuilder builder,
      StringOffset idOffset = default(StringOffset),
      long val = 0,
      ushort count = 0) {
    builder.StartObject(3);
    Stat.AddVal(builder, val);
    Stat.AddId(builder, idOffset);
    Stat.AddCount(builder, count);
    return Stat.EndStat(builder);
  }

  public static void StartStat(FlatBufferBuilder builder) { builder.StartObject(3); }
  public static void AddId(FlatBufferBuilder builder, StringOffset idOffset) { builder.AddOffset(0, idOffset.Value, 0); }
  public static void AddVal(FlatBufferBuilder builder, long val) { builder.AddLong(1, val, 0); }
  public static void AddCount(FlatBufferBuilder builder, ushort count) { builder.AddUshort(2, count, 0); }
  public static Offset<Stat> EndStat(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<Stat>(o);
  }
};


}
