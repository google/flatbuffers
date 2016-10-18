// automatically generated by the FlatBuffers compiler, do not modify

namespace Testing.KeySearch
{

using System;
using FlatBuffers;

public struct UByteEntry : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static UByteEntry GetRootAsUByteEntry(ByteBuffer _bb) { return GetRootAsUByteEntry(_bb, new UByteEntry()); }
  public static UByteEntry GetRootAsUByteEntry(ByteBuffer _bb, UByteEntry obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public UByteEntry __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public byte Key { get { int o = __p.__offset(4); return o != 0 ? __p.bb.Get(o + __p.bb_pos) : (byte)0; } }
  public bool MutateKey(byte key) { int o = __p.__offset(4); if (o != 0) { __p.bb.Put(o + __p.bb_pos, key); return true; } else { return false; } }
  public byte Value { get { int o = __p.__offset(6); return o != 0 ? __p.bb.Get(o + __p.bb_pos) : (byte)255; } }
  public bool MutateValue(byte value) { int o = __p.__offset(6); if (o != 0) { __p.bb.Put(o + __p.bb_pos, value); return true; } else { return false; } }

  public static Offset<UByteEntry> CreateUByteEntry(FlatBufferBuilder builder,
      byte key = 0,
      byte value = 255) {
    builder.StartObject(2);
    UByteEntry.AddValue(builder, value);
    UByteEntry.AddKey(builder, key);
    return UByteEntry.EndUByteEntry(builder);
  }

  public static void StartUByteEntry(FlatBufferBuilder builder) { builder.StartObject(2); }
  public static void AddKey(FlatBufferBuilder builder, byte key) { builder.AddByte(0, key, 0); }
  public static void AddValue(FlatBufferBuilder builder, byte value) { builder.AddByte(1, value, 255); }
  public static Offset<UByteEntry> EndUByteEntry(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<UByteEntry>(o);
  }

  public static VectorOffset CreateMySortedVectorOfTables(FlatBufferBuilder builder, Offset<UByteEntry>[] offsets) {
    Array.Sort(offsets, (Offset<UByteEntry> o1, Offset<UByteEntry> o2) => builder.DataBufferp.bb.Get(Table.__offset(4, o1.Value, builder.DataBuffer)).CompareTo(builder.DataBufferp.bb.Get(Table.__offset(4, o2.Value, builder.DataBuffer))));
    return builder.CreateVectorOfTables(offsets);
  }

  public static UByteEntry? LookupByKey(VectorOffset vectorOffset, byte key, ByteBuffer bb) {
    int vectorLocation = bb.Length - vectorOffset.Value;
    int span = bb.GetInt(vectorLocation);
    int start = 0;
    vectorLocation += 4;
    while (span != 0) {
      int middle = span / 2;
      int tableOffset = Table.__indirect(vectorLocation + 4 * (start + middle), bb);
      int comp = __p.bb.Get(Table.__offset(4, bb.Length - tableOffset, bb)).CompareTo(key);
      if (comp > 0) {
        span = middle;
      } else if (comp < 0) {
        middle++;
        start += middle;
        span -= middle;
      } else {
        return new UByteEntry().__assign(tableOffset, bb);
      }
    }
    return null;
  }
};


}
