// automatically generated by the FlatBuffers compiler, do not modify

package Testing.KeySearch;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

@SuppressWarnings("unused")
public final class ULongEntry extends Table {
  public static ULongEntry getRootAsULongEntry(ByteBuffer _bb) { return getRootAsULongEntry(_bb, new ULongEntry()); }
  public static ULongEntry getRootAsULongEntry(ByteBuffer _bb, ULongEntry obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__assign(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public void __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; }
  public ULongEntry __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public long key() { int o = __offset(4); return o != 0 ? bb.getLong(o + bb_pos) : 0L; }
  public boolean mutateKey(long key) { int o = __offset(4); if (o != 0) { bb.putLong(o + bb_pos, key); return true; } else { return false; } }
  public long value() { int o = __offset(6); return o != 0 ? bb.getLong(o + bb_pos) : 9223372036854775807L; }
  public boolean mutateValue(long value) { int o = __offset(6); if (o != 0) { bb.putLong(o + bb_pos, value); return true; } else { return false; } }

  public static int createULongEntry(FlatBufferBuilder builder,
      long key,
      long value) {
    builder.startObject(2);
    ULongEntry.addValue(builder, value);
    ULongEntry.addKey(builder, key);
    return ULongEntry.endULongEntry(builder);
  }

  public static void startULongEntry(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addKey(FlatBufferBuilder builder, long key) { builder.addLong(0, key, 0L); }
  public static void addValue(FlatBufferBuilder builder, long value) { builder.addLong(1, value, 9223372036854775807L); }
  public static int endULongEntry(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }

  @Override
  protected int keysCompare(Integer o1, Integer o2, ByteBuffer _bb) {
    long val_1 = _bb.getLong(__offset(4, o1, _bb));
    long val_2 = _bb.getLong(__offset(4, o2, _bb));
    return val_1 > val_2 ? 1 : val_1 < val_2 ? -1 : 0;
  }
}

