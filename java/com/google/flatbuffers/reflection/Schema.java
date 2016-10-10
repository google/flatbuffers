// automatically generated by the FlatBuffers compiler, do not modify

package com.google.flatbuffers.reflection;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

@SuppressWarnings("unused")
public final class Schema extends Table {
  public static Schema getRootAsSchema(ByteBuffer _bb) { return getRootAsSchema(_bb, new Schema()); }
  public static Schema getRootAsSchema(ByteBuffer _bb, Schema obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__assign(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public static boolean SchemaBufferHasIdentifier(ByteBuffer _bb) { return __has_identifier(_bb, "BFBS"); }
  public Schema __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public Object objects(int j) { return objects(new Object(), j); }
  public Object objects(Object obj, int j) { int o = __offset(4); return o != 0 ? obj.__assign(__indirect(__vector(o) + j * 4), bb) : null; }
  public int objectsLength() { int o = __offset(4); return o != 0 ? __vector_len(o) : 0; }
  public Object objectsByKey( String key ) { int vectorOffset = __vector(__offset(4)) - 4; return vectorOffset != 0 ? Object.lookupByKey(bb.array().length - vectorOffset, key, bb) : null;  }
  public Enum enums(int j) { return enums(new Enum(), j); }
  public Enum enums(Enum obj, int j) { int o = __offset(6); return o != 0 ? obj.__assign(__indirect(__vector(o) + j * 4), bb) : null; }
  public int enumsLength() { int o = __offset(6); return o != 0 ? __vector_len(o) : 0; }
  public Enum enumsByKey( String key ) { int vectorOffset = __vector(__offset(6)) - 4; return vectorOffset != 0 ? Enum.lookupByKey(bb.array().length - vectorOffset, key, bb) : null;  }
  public String fileIdent() { int o = __offset(8); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer fileIdentAsByteBuffer() { return __vector_as_bytebuffer(8, 1); }
  public String fileExt() { int o = __offset(10); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer fileExtAsByteBuffer() { return __vector_as_bytebuffer(10, 1); }
  public Object rootTable() { return rootTable(new Object()); }
  public Object rootTable(Object obj) { int o = __offset(12); return o != 0 ? obj.__assign(__indirect(o + bb_pos), bb) : null; }

  public static int createSchema(FlatBufferBuilder builder,
      int objectsOffset,
      int enumsOffset,
      int file_identOffset,
      int file_extOffset,
      int root_tableOffset) {
    builder.startObject(5);
    Schema.addRootTable(builder, root_tableOffset);
    Schema.addFileExt(builder, file_extOffset);
    Schema.addFileIdent(builder, file_identOffset);
    Schema.addEnums(builder, enumsOffset);
    Schema.addObjects(builder, objectsOffset);
    return Schema.endSchema(builder);
  }

  public static void startSchema(FlatBufferBuilder builder) { builder.startObject(5); }
  public static void addObjects(FlatBufferBuilder builder, int objectsOffset) { builder.addOffset(0, objectsOffset, 0); }
  public static int createObjectsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startObjectsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addEnums(FlatBufferBuilder builder, int enumsOffset) { builder.addOffset(1, enumsOffset, 0); }
  public static int createEnumsVector(FlatBufferBuilder builder, int[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addOffset(data[i]); return builder.endVector(); }
  public static void startEnumsVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static void addFileIdent(FlatBufferBuilder builder, int fileIdentOffset) { builder.addOffset(2, fileIdentOffset, 0); }
  public static void addFileExt(FlatBufferBuilder builder, int fileExtOffset) { builder.addOffset(3, fileExtOffset, 0); }
  public static void addRootTable(FlatBufferBuilder builder, int rootTableOffset) { builder.addOffset(4, rootTableOffset, 0); }
  public static int endSchema(FlatBufferBuilder builder) {
    int o = builder.endObject();
    builder.required(o, 4);  // objects
    builder.required(o, 6);  // enums
    return o;
  }
  public static void finishSchemaBuffer(FlatBufferBuilder builder, int offset) { builder.finish(offset, "BFBS"); }
}

