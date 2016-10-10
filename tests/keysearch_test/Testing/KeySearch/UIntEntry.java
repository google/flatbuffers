// automatically generated by the FlatBuffers compiler, do not modify

package Testing.KeySearch;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

@SuppressWarnings("unused")
public final class UIntEntry extends Table {
  public static UIntEntry getRootAsUIntEntry(ByteBuffer _bb) { return getRootAsUIntEntry(_bb, new UIntEntry()); }
  public static UIntEntry getRootAsUIntEntry(ByteBuffer _bb, UIntEntry obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__assign(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public void __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; }
  public UIntEntry __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public long key() { int o = __offset(4); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0; }
  public boolean mutateKey(long key) { int o = __offset(4); if (o != 0) { bb.putInt(o + bb_pos, (int)key); return true; } else { return false; } }
  public long value() { int o = __offset(6); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 4294967295; }
  public boolean mutateValue(long value) { int o = __offset(6); if (o != 0) { bb.putInt(o + bb_pos, (int)value); return true; } else { return false; } }

  public static int createUIntEntry(FlatBufferBuilder builder,
      long key,
      long value) {
    builder.startObject(2);
    UIntEntry.addValue(builder, value);
    UIntEntry.addKey(builder, key);
    return UIntEntry.endUIntEntry(builder);
  }

  public static void startUIntEntry(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addKey(FlatBufferBuilder builder, long key) { builder.addInt(0, (int)key, 0); }
  public static void addValue(FlatBufferBuilder builder, long value) { builder.addInt(1, (int)value, 4294967295); }
  public static int endUIntEntry(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }

  @Override
  protected int keysCompare(Integer o1, Integer o2, ByteBuffer _bb) {
    int val_1 = _bb.getInt(__offset(4, o1, _bb));
    int val_2 = _bb.getInt(__offset(4, o2, _bb));
    return val_1 > val_2 ? 1 : val_1 < val_2 ? -1 : 0;
  }

  public static UIntEntry lookupByKey(int vectorOffset, int key, ByteBuffer bb) {
    int vectorLocation = bb.array().length - vectorOffset;
    int span = bb.getInt(vectorLocation);
    int start = 0;
    vectorLocation += 4;
    while (span != 0) {
      int middle = span / 2;
      int tableOffset = __indirect(vectorLocation + 4 * (start + middle), bb);
      int val = bb.getInt(__offset(4, bb.array().length - tableOffset, bb));
      int comp = val > key ? 1 : val < key ? -1 : 0;
      if (comp > 0) {
        span = middle;
      } else if (comp < 0) {
        middle++;
        start += middle;
        span -= middle;
      } else {
        return new UIntEntry().__assign(tableOffset, bb);
      }
    }
    return null;
  }
}

