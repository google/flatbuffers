// automatically generated, do not modify

namespace MyGame.Example
{

using FlatBuffers;

public sealed class Stat : Table {
  public static Stat GetRootAsStat(ByteBuffer _bb) { return GetRootAsStat(_bb, new Stat()); }
  public static Stat GetRootAsStat(ByteBuffer _bb, Stat obj) { return (obj.__init(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public Stat __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public string Id { get { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; } }
  public long Val { get { int o = __offset(6); return o != 0 ? bb.GetLong(o + bb_pos) : (long)0; } }
  public bool MutateVal(long val) { int o = __offset(6); if (o != 0) { bb.PutLong(o + bb_pos, val); return true; } else { return false; } }
  public ushort Count { get { int o = __offset(8); return o != 0 ? bb.GetUshort(o + bb_pos) : (ushort)0; } }
  public bool MutateCount(ushort count) { int o = __offset(8); if (o != 0) { bb.PutUshort(o + bb_pos, count); return true; } else { return false; } }

  public static int CreateStat(FlatBufferBuilder builder,
      int id = 0,
      long val = 0,
      ushort count = 0) {
    builder.StartObject(3);
    Stat.AddVal(builder, val);
    Stat.AddId(builder, id);
    Stat.AddCount(builder, count);
    return Stat.EndStat(builder);
  }

  public static void StartStat(FlatBufferBuilder builder) { builder.StartObject(3); }
  public static void AddId(FlatBufferBuilder builder, int idOffset) { builder.AddOffset(0, idOffset, 0); }
  public static void AddVal(FlatBufferBuilder builder, long val) { builder.AddLong(1, val, 0); }
  public static void AddCount(FlatBufferBuilder builder, ushort count) { builder.AddUshort(2, count, 0); }
  public static int EndStat(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return o;
  }
};


}
