// automatically generated, do not modify

module MyGame.Example.Stat;

import flatbuffers;

struct Stat {
  mixin Table!Stat;

  static Stat getRootAsStat(ByteBuffer _bb) {  return Stat.init_(_bb.get!int(_bb.position()) + _bb.position(), _bb); }
  @property   Nullable!string id() { int o = __offset(4); return o != 0 ? Nullable!string(__string(o + _pos)) : Nullable!string.init; }
  @property   long val() { int o = __offset(6); return o != 0 ? _buffer.get!long(o + _pos) : 0; }
  @property   ushort count() { int o = __offset(8); return o != 0 ? _buffer.get!ushort(o + _pos) : 0; }

  static int createStat(FlatBufferBuilder builder,
      int id,
      long val,
      ushort count) {
    builder.startObject(3);
    Stat.addVal(builder, val);
    Stat.addId(builder, id);
    Stat.addCount(builder, count);
    return Stat.endStat(builder);
  }

  static void startStat(FlatBufferBuilder builder) { builder.startObject(3); }
  static void addId(FlatBufferBuilder builder, int idOffset) { builder.addOffset(0, idOffset, 0); }
  static void addVal(FlatBufferBuilder builder, long val) { builder.addLong(1, val, 0); }
  static void addCount(FlatBufferBuilder builder, ushort count) { builder.addUshort(2, count, 0); }
  static int endStat(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
}

