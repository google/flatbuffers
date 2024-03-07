// automatically generated by the FlatBuffers compiler, do not modify

package com.google.flatbuffers.reflection;

import com.google.flatbuffers.BaseVector;
import com.google.flatbuffers.BooleanVector;
import com.google.flatbuffers.ByteVector;
import com.google.flatbuffers.Constants;
import com.google.flatbuffers.DoubleVector;
import com.google.flatbuffers.FlatBufferBuilder;
import com.google.flatbuffers.FloatVector;
import com.google.flatbuffers.IntVector;
import com.google.flatbuffers.LongVector;
import com.google.flatbuffers.ShortVector;
import com.google.flatbuffers.StringVector;
import com.google.flatbuffers.Struct;
import com.google.flatbuffers.Table;
import com.google.flatbuffers.UnionVector;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

@SuppressWarnings("unused")
public final class Type extends Table {
  public static void ValidateVersion() { Constants.FLATBUFFERS_24_3_7(); }
  public static Type getRootAsType(ByteBuffer _bb) { return getRootAsType(_bb, new Type()); }
  public static Type getRootAsType(ByteBuffer _bb, Type obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__assign(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __reset(_i, _bb); }
  public Type __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public byte baseType() { int o = __offset(4); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public byte element() { int o = __offset(6); return o != 0 ? bb.get(o + bb_pos) : 0; }
  public int index() { int o = __offset(8); return o != 0 ? bb.getInt(o + bb_pos) : -1; }
  public int fixedLength() { int o = __offset(10); return o != 0 ? bb.getShort(o + bb_pos) & 0xFFFF : 0; }
  /**
   * The size (octets) of the `base_type` field.
   */
  public long baseSize() { int o = __offset(12); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 4L; }
  /**
   * The size (octets) of the `element` field, if present.
   */
  public long elementSize() { int o = __offset(14); return o != 0 ? (long)bb.getInt(o + bb_pos) & 0xFFFFFFFFL : 0L; }

  public static int createType(FlatBufferBuilder builder,
      byte baseType,
      byte element,
      int index,
      int fixedLength,
      long baseSize,
      long elementSize) {
    builder.startTable(6);
    Type.addElementSize(builder, elementSize);
    Type.addBaseSize(builder, baseSize);
    Type.addIndex(builder, index);
    Type.addFixedLength(builder, fixedLength);
    Type.addElement(builder, element);
    Type.addBaseType(builder, baseType);
    return Type.endType(builder);
  }

  public static void startType(FlatBufferBuilder builder) { builder.startTable(6); }
  public static void addBaseType(FlatBufferBuilder builder, byte baseType) { builder.addByte(0, baseType, 0); }
  public static void addElement(FlatBufferBuilder builder, byte element) { builder.addByte(1, element, 0); }
  public static void addIndex(FlatBufferBuilder builder, int index) { builder.addInt(2, index, -1); }
  public static void addFixedLength(FlatBufferBuilder builder, int fixedLength) { builder.addShort(3, (short) fixedLength, (short) 0); }
  public static void addBaseSize(FlatBufferBuilder builder, long baseSize) { builder.addInt(4, (int) baseSize, (int) 4L); }
  public static void addElementSize(FlatBufferBuilder builder, long elementSize) { builder.addInt(5, (int) elementSize, (int) 0L); }
  public static int endType(FlatBufferBuilder builder) {
    int o = builder.endTable();
    return o;
  }

  public static final class Vector extends BaseVector {
    public Vector __assign(int _vector, int _element_size, ByteBuffer _bb) { __reset(_vector, _element_size, _bb); return this; }

    public Type get(int j) { return get(new Type(), j); }
    public Type get(Type obj, int j) {  return obj.__assign(__indirect(__element(j), bb), bb); }
  }
}

