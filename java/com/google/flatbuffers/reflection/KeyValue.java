// automatically generated by the FlatBuffers compiler, do not modify

package com.google.flatbuffers.reflection;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

@SuppressWarnings("unused")
public final class KeyValue extends Table {
  public static KeyValue getRootAsKeyValue(ByteBuffer _bb) { return getRootAsKeyValue(_bb, new KeyValue()); }
  public static KeyValue getRootAsKeyValue(ByteBuffer _bb, KeyValue obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__assign(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public KeyValue __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public String key() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer keyAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public String value() { int o = __offset(6); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer valueAsByteBuffer() { return __vector_as_bytebuffer(6, 1); }

  public static int createKeyValue(FlatBufferBuilder builder,
      int keyOffset,
      int valueOffset) {
    builder.startObject(2);
    KeyValue.addValue(builder, valueOffset);
    KeyValue.addKey(builder, keyOffset);
    return KeyValue.endKeyValue(builder);
  }

  public static void startKeyValue(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addKey(FlatBufferBuilder builder, int keyOffset) { builder.addOffset(0, keyOffset, 0); }
  public static void addValue(FlatBufferBuilder builder, int valueOffset) { builder.addOffset(1, valueOffset, 0); }
  public static int endKeyValue(FlatBufferBuilder builder) {
    int o = builder.endObject();
    builder.required(o, 4);  // key
    return o;
  }

  @Override
  protected int keysCompare(Integer o1, Integer o2, ByteBuffer _bb) { return compareStrings(__offset(4, o1, _bb), __offset(4, o2, _bb), _bb); }

  public static KeyValue lookupByKey(int vectorOffset, String key, ByteBuffer bb) {
    byte[] byteKey = key.getBytes(Table.UTF8_CHARSET.get());
    int vectorLocation = bb.array().length - vectorOffset;
    int span = bb.getInt(vectorLocation);
    int start = 0;
    vectorLocation += 4;
    while (span != 0) {
      int middle = span / 2;
      int tableOffset = __indirect(vectorLocation + 4 * (start + middle), bb);
      int comp = compareStrings(__offset(4, bb.array().length - tableOffset, bb), byteKey, bb);
      if (comp > 0) {
        span = middle;
      } else if (comp < 0) {
        middle++;
        start += middle;
        span -= middle;
      } else {
        return new KeyValue().__assign(tableOffset, bb);
      }
    }
    return null;
  }
}

