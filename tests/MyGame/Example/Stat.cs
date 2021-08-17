// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace MyGame.Example
{

using global::System;
using global::System.Collections.Generic;
using global::FlatBuffers;

public struct Stat : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static void ValidateVersion() { FlatBufferConstants.FLATBUFFERS_2_0_0(); }
  public static Stat GetRootAsStat(ByteBuffer _bb) { return GetRootAsStat(_bb, new Stat()); }
  public static Stat GetRootAsStat(ByteBuffer _bb, Stat obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p = new Table(_i, _bb); }
  public Stat __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public string Id { get { int o = __p.__offset(4); return o != 0 ? __p.__string(o + __p.bb_pos) : null; } }
#if ENABLE_SPAN_T
  public Span<byte> GetIdBytes() { return __p.__vector_as_span<byte>(4, 1); }
#else
  public ArraySegment<byte>? GetIdBytes() { return __p.__vector_as_arraysegment(4); }
#endif
  public byte[] GetIdArray() { return __p.__vector_as_array<byte>(4); }
  public long Val { get { int o = __p.__offset(6); return o != 0 ? __p.bb.GetLong(o + __p.bb_pos) : (long)0; } }
  public bool MutateVal(long val) { int o = __p.__offset(6); if (o != 0) { __p.bb.PutLong(o + __p.bb_pos, val); return true; } else { return false; } }
  public ushort Count { get { int o = __p.__offset(8); return o != 0 ? __p.bb.GetUshort(o + __p.bb_pos) : (ushort)0; } }
  public bool MutateCount(ushort count) { int o = __p.__offset(8); if (o != 0) { __p.bb.PutUshort(o + __p.bb_pos, count); return true; } else { return false; } }

  public static Offset<global::MyGame.Example.Stat> CreateStat(FlatBufferBuilder builder,
      StringOffset idOffset = default(StringOffset),
      long val = 0,
      ushort count = 0) {
    builder.StartTable(3);
    Stat.AddVal(builder, val);
    Stat.AddId(builder, idOffset);
    Stat.AddCount(builder, count);
    return Stat.EndStat(builder);
  }

  public static void StartStat(FlatBufferBuilder builder) { builder.StartTable(3); }
  public static void AddId(FlatBufferBuilder builder, StringOffset idOffset) { builder.AddOffset(0, idOffset.Value, 0); }
  public static void AddVal(FlatBufferBuilder builder, long val) { builder.AddLong(1, val, 0); }
  public static void AddCount(FlatBufferBuilder builder, ushort count) { builder.AddUshort(2, count, 0); }
  public static Offset<global::MyGame.Example.Stat> EndStat(FlatBufferBuilder builder) {
    int o = builder.EndTable();
    return new Offset<global::MyGame.Example.Stat>(o);
  }

  public static VectorOffset CreateSortedVectorOfStat(FlatBufferBuilder builder, Offset<Stat>[] offsets) {
    Array.Sort(offsets, (Offset<Stat> o1, Offset<Stat> o2) => builder.DataBuffer.GetUshort(Table.__offset(8, o1.Value, builder.DataBuffer)).CompareTo(builder.DataBuffer.GetUshort(Table.__offset(8, o2.Value, builder.DataBuffer))));
    return builder.CreateVectorOfTables(offsets);
  }

  public static Stat? __lookup_by_key(int vectorLocation, ushort key, ByteBuffer bb) {
    int span = bb.GetInt(vectorLocation - 4);
    int start = 0;
    while (span != 0) {
      int middle = span / 2;
      int tableOffset = Table.__indirect(vectorLocation + 4 * (start + middle), bb);
      int comp = bb.GetUshort(Table.__offset(8, bb.Length - tableOffset, bb)).CompareTo(key);
      if (comp > 0) {
        span = middle;
      } else if (comp < 0) {
        middle++;
        start += middle;
        span -= middle;
      } else {
        return new Stat().__assign(tableOffset, bb);
      }
    }
    return null;
  }
  public StatT UnPack() {
    var _o = new StatT();
    this.UnPackTo(_o);
    return _o;
  }
  public void UnPackTo(StatT _o) {
    _o.Id = this.Id;
    _o.Val = this.Val;
    _o.Count = this.Count;
  }
  public static Offset<global::MyGame.Example.Stat> Pack(FlatBufferBuilder builder, StatT _o) {
    if (_o == null) return default(Offset<global::MyGame.Example.Stat>);
    var _id = _o.Id == null ? default(StringOffset) : builder.CreateString(_o.Id);
    return CreateStat(
      builder,
      _id,
      _o.Val,
      _o.Count);
  }
}

public class StatT
{
  [Newtonsoft.Json.JsonProperty("id")]
  public string Id { get; set; }
  [Newtonsoft.Json.JsonProperty("val")]
  public long Val { get; set; }
  [Newtonsoft.Json.JsonProperty("count")]
  public ushort Count { get; set; }

  public StatT() {
    this.Id = null;
    this.Val = 0;
    this.Count = 0;
  }
}


}
