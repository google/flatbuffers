/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.flatbuffers;

import com.google.flatbuffers.reflection.BaseType;
import com.google.flatbuffers.reflection.Field;
import com.google.flatbuffers.reflection.Schema;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import static com.google.flatbuffers.Constants.SIZEOF_DOUBLE;
import static com.google.flatbuffers.Constants.SIZEOF_FLOAT;
import static com.google.flatbuffers.Constants.SIZEOF_INT;
import static com.google.flatbuffers.Constants.SIZEOF_LONG;
import static com.google.flatbuffers.Constants.SIZEOF_SHORT;
import static com.google.flatbuffers.reflection.BaseType.Bool;
import static com.google.flatbuffers.reflection.BaseType.Byte;
import static com.google.flatbuffers.reflection.BaseType.Double;
import static com.google.flatbuffers.reflection.BaseType.Float;
import static com.google.flatbuffers.reflection.BaseType.Int;
import static com.google.flatbuffers.reflection.BaseType.Long;
import static com.google.flatbuffers.reflection.BaseType.Obj;
import static com.google.flatbuffers.reflection.BaseType.Short;
import static com.google.flatbuffers.reflection.BaseType.String;
import static com.google.flatbuffers.reflection.BaseType.UByte;
import static com.google.flatbuffers.reflection.BaseType.UInt;
import static com.google.flatbuffers.reflection.BaseType.ULong;
import static com.google.flatbuffers.reflection.BaseType.UShort;
import static com.google.flatbuffers.reflection.BaseType.Vector;

/**
 * Help class to manipulate flatbuffers document by reflection.
 */
public final class Reflection {

  private static final int UBYTE_MASK = 0xFF;
  private static final int USHORT_MASK = 0xFFFF;
  private static final long UINT_MASK = 0xFFFFFFFFL;

  private Reflection() {
  }

  public static com.google.flatbuffers.reflection.Object getSchemaChildTable(Schema schema, String tableName) {
    return getSchemaChildTable(schema, schema.rootTable(), tableName);
  }

  public static com.google.flatbuffers.reflection.Object getSchemaChildTable(Schema schema, com.google.flatbuffers.reflection.Object parent, String tableName) {
    return schema.objects(parent.fieldsByKey(tableName).type().index());
  }

  public static com.google.flatbuffers.reflection.Object getSchemaChildTable(Schema schema, com.google.flatbuffers.reflection.Object parent, com.google.flatbuffers.reflection.Object childToReuse,String tableName) {
    return schema.objects(childToReuse, parent.fieldsByKey(tableName).type().index());
  }

  public static Table getRootTable(ByteBuffer _bb) {
    return getRootTable(_bb, new Table());
  }

  public static Table getRootTable(ByteBuffer _bb, Table obj) {
    _bb.order(ByteOrder.LITTLE_ENDIAN);
    obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb);
    return obj;
  }

  public static boolean hasValue(Table table, Field field) {
    int o = table.__offset(field.offset());
    return o != 0;
  }

  public static boolean getBoolField(Table table, Field field) {
    return getBoolField(table, field, 0 != field.defaultInteger());
  }

  public static boolean getBoolField(Table table, Field field, boolean defaultValue) {
    assert checkFieldType(field.type().baseType(), Bool);
    int o = table.__offset(field.offset());
    return o != 0 ? 0 != table.bb.get(o + table.bb_pos) : defaultValue;
  }

  public static boolean getBoolsField(Table table, Field field, int index) {
    return getBoolsField(table, field, false, index);
  }

  public static boolean getBoolsField(Table table, Field field, boolean defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Bool);
    int o = table.__offset(field.offset());
    return o != 0 ? 0 != table.bb.get(table.__vector(o) + index) : defaultValue;
  }

  public static boolean setBoolField(Table table, Field field, boolean value) {
    assert checkFieldType(field.type().baseType(), Bool);
    int o = table.__offset(field.offset());
    if (o != 0) {
      table.bb.put(o + table.bb_pos, (byte) (value ? 1 : 0));
      return true;
    } else {
      return false;
    }
  }

  public static byte getByteField(Table table, Field field) {
    return getByteField(table, field, (byte) field.defaultInteger());
  }

  public static byte getByteField(Table table, Field field, byte defaultValue) {
    assert checkFieldType(field.type().baseType(), Byte);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.get(o + table.bb_pos) : defaultValue;
  }

  public static byte getBytesField(Table table, Field field, int index) {
    return getBytesField(table, field, (byte) 0, index);
  }

  public static byte getBytesField(Table table, Field field, byte defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Byte);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.get(table.__vector(o) + index) : defaultValue;
  }

  public static boolean setByteField(Table table, Field field, byte value) {
    assert checkFieldType(field.type().baseType(), Byte);
    int o = table.__offset(field.offset());
    if (o != 0) {
      table.bb.put(o + table.bb_pos, value);
      return true;
    } else {
      return false;
    }
  }

  public static int getUByteField(Table table, Field field) {
    return getUByteField(table, field, (int) field.defaultInteger());
  }

  public static int getUByteField(Table table, Field field, int defaultValue) {
    assert checkFieldType(field.type().baseType(), UByte);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.get(o + table.bb_pos) & UBYTE_MASK : defaultValue;
  }

  public static int getUBytesField(Table table, Field field, int index) {
    return getUBytesField(table, field, 0, index);
  }

  public static int getUBytesField(Table table, Field field, int defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), UByte);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.get(table.__vector(o) + index) & UBYTE_MASK : defaultValue;
  }

  public static boolean setUByteField(Table table, Field field, int value) {
    assert checkFieldType(field.type().baseType(), UByte);
    int o = table.__offset(field.offset());
    if (o != 0) {
      table.bb.put(o + table.bb_pos, (byte) value);
      return true;
    } else {
      return false;
    }
  }

  public static short getShortField(Table table, Field field) {
    return getShortField(table, field, (short) field.defaultInteger());
  }

  public static short getShortField(Table table, Field field, short defaultValue) {
    assert checkFieldType(field.type().baseType(), Short);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getShort(o + table.bb_pos) : defaultValue;
  }

  public static short getShortsField(Table table, Field field, int index) {
    return getShortsField(table, field, (short) 0, index);
  }

  public static short getShortsField(Table table, Field field, short defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Short);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getShort(table.__vector(o) + index * SIZEOF_SHORT) : defaultValue;
  }

  public static boolean setShortField(Table table, Field field, short value) {
    assert checkFieldType(field.type().baseType(), Short);
    int o = table.__offset(field.offset());
    if (o != 0) {
      table.bb.putShort(o + table.bb_pos, value);
      return true;
    } else {
      return false;
    }
  }

  public static int getUShortField(Table table, Field field) {
    return getUShortField(table, field, (int) field.defaultInteger());
  }

  public static int getUShortField(Table table, Field field, int defaultValue) {
    assert checkFieldType(field.type().baseType(), UShort);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getShort(o + table.bb_pos) & USHORT_MASK : defaultValue;
  }

  public static int getUShortsField(Table table, Field field, int index) {
    return getUShortsField(table, field, 0, index);
  }

  public static int getUShortsField(Table table, Field field, int defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), UShort);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getShort(table.__vector(o) + index * SIZEOF_SHORT) & USHORT_MASK : defaultValue;
  }

  public static boolean setUShortField(Table table, Field field, int value) {
    assert checkFieldType(field.type().baseType(), UShort);
    int o = table.__offset(field.offset());
    if (o != 0) {
      table.bb.putShort(o + table.bb_pos, (short) value);
      return true;
    } else {
      return false;
    }
  }

  public static int getIntField(Table table, Field field) {
    return getIntField(table, field, (int) field.defaultInteger());
  }

  public static int getIntField(Table table, Field field, int defaultValue) {
    assert checkFieldType(field.type().baseType(), Int);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getInt(o + table.bb_pos) : defaultValue;
  }

  public static int getIntsField(Table table, Field field, int index) {
    return getIntsField(table, field, 0, index);
  }

  public static int getIntsField(Table table, Field field, int defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Int);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getInt(table.__vector(o) + index * SIZEOF_INT) : defaultValue;
  }

  public static boolean setIntField(Table table, Field field, int value) {
    assert checkFieldType(field.type().baseType(), Int);
    int o = table.__offset(field.offset());
    if (o != 0) {
      table.bb.putInt(o + table.bb_pos, value);
      return true;
    } else {
      return false;
    }
  }

  public static long getUIntField(Table table, Field field) {
    return getUIntField(table, field, field.defaultInteger());
  }

  public static long getUIntField(Table table, Field field, long defaultValue) {
    assert checkFieldType(field.type().baseType(), UInt);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getInt(o + table.bb_pos) & UINT_MASK : defaultValue;
  }

  public static long getUIntsField(Table table, Field field, int index) {
    return getUIntsField(table, field, 0L, index);
  }

  public static long getUIntsField(Table table, Field field, long defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), UInt);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getInt(table.__vector(o) + index * SIZEOF_INT) & UINT_MASK : defaultValue;
  }

  public static boolean setUIntField(Table table, Field field, long value) {
    assert checkFieldType(field.type().baseType(), UInt);
    int o = table.__offset(field.offset());
    if (o != 0) {
      table.bb.putInt(o + table.bb_pos, (int) value);
      return true;
    } else {
      return false;
    }
  }

  public static long getLongField(Table table, Field field) {
    return getLongField(table, field, field.defaultInteger());
  }

  public static long getLongField(Table table, Field field, long defaultValue) {
    assert checkFieldType(field.type().baseType(), Long);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getLong(o + table.bb_pos) : defaultValue;
  }

  public static long getLongsField(Table table, Field field, int index) {
    return getLongsField(table, field, 0L, index);
  }

  public static long getLongsField(Table table, Field field, long defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Long);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getLong(table.__vector(o) + index * SIZEOF_LONG) : defaultValue;
  }

  public static boolean setLongField(Table table, Field field, long value) {
    assert checkFieldType(field.type().baseType(), Long);
    int o = table.__offset(field.offset());
    if (o != 0) {
      table.bb.putLong(o + table.bb_pos, value);
      return true;
    } else {
      return false;
    }
  }

  public static long getULongField(Table table, Field field) {
    return getULongField(table, field, field.defaultInteger());
  }

  public static long getULongField(Table table, Field field, long defaultValue) {
    assert checkFieldType(field.type().baseType(), ULong);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getLong(o + table.bb_pos) : defaultValue;
  }

  public static long getULongsField(Table table, Field field, int index) {
    return getULongsField(table, field, 0L, index);
  }

  public static long getULongsField(Table table, Field field, long defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), ULong);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getLong(table.__vector(o) + index * SIZEOF_LONG) : defaultValue;
  }

  public static boolean setULongField(Table table, Field field, long value) {
    assert checkFieldType(field.type().baseType(), ULong);
    int o = table.__offset(field.offset());
    if (o != 0) {
      table.bb.putLong(o + table.bb_pos, value);
      return true;
    } else {
      return false;
    }
  }

  public static float getFloatField(Table table, Field field) {
    return getFloatField(table, field, field.defaultInteger());
  }

  public static float getFloatField(Table table, Field field, float defaultValue) {
    assert checkFieldType(field.type().baseType(), Float);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getFloat(o + table.bb_pos) : defaultValue;
  }

  public static float getFloatsField(Table table, Field field, int index) {
    return getFloatsField(table, field, 0F, index);
  }

  public static float getFloatsField(Table table, Field field, float defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Float);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getFloat(table.__vector(o) + index * SIZEOF_FLOAT) : defaultValue;
  }

  public static boolean setFloatField(Table table, Field field, float value) {
    assert checkFieldType(field.type().baseType(), Float);
    int o = table.__offset(field.offset());
    if (o != 0) {
      table.bb.putFloat(o + table.bb_pos, value);
      return true;
    } else {
      return false;
    }
  }

  public static double getDoubleField(Table table, Field field) {
    return getDoubleField(table, field, field.defaultInteger());
  }

  public static double getDoubleField(Table table, Field field, double defaultValue) {
    assert checkFieldType(field.type().baseType(), Double);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getDouble(o + table.bb_pos) : defaultValue;
  }

  public static double getDoublesField(Table table, Field field, int index) {
    return getDoublesField(table, field, 0D, index);
  }

  public static double getDoublesField(Table table, Field field, double defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Double);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getDouble(table.__vector(o) + index * SIZEOF_DOUBLE) : defaultValue;
  }

  public static boolean setDoubleField(Table table, Field field, double value) {
    assert checkFieldType(field.type().baseType(), Double);
    int o = table.__offset(field.offset());
    if (o != 0) {
      table.bb.putDouble(o + table.bb_pos, value);
      return true;
    } else {
      return false;
    }
  }

  public static String getStringField(Table table, Field field) {
    return getStringField(table, field, null);
  }

  public static String getStringField(Table table, Field field, String defaultValue) {
    assert checkFieldType(field.type().baseType(), String);
    int o = table.__offset(field.offset());
    return o != 0 ? table.__string(o + table.bb_pos) : defaultValue;
  }

  public static String getStringsField(Table table, Field field, int index) {
    return getStringsField(table, field, null, index);
  }

  public static String getStringsField(Table table, Field field, String defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), String);
    int o = table.__offset(field.offset());
    return o != 0 ? table.__string(table.__vector(o) + index * SIZEOF_INT) : defaultValue;
  }

  public static Table getObjField(Table table, Field field) {
    return getObjField(table, field, new Table());
  }

  public static Table getObjField(Table table, Field field, Table obj) {
    assert checkFieldType(field.type().baseType(), Obj);
    int o = table.__offset(field.offset());
    if (o != 0) {
      obj.__init(table.__indirect(o + table.bb_pos), table.bb);
      return obj;
    } else {
      return null;
    }
  }

  public static Table getObjsField(Table table, Field field, int index) {
    return getObjsField(table, field, new Table(), index);
  }

  public static Table getObjsField(Table table, Field field, Table obj, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Obj);
    int o = table.__offset(field.offset());
    if (o != 0) {
      obj.__init(table.__indirect(table.__vector(o) + index * SIZEOF_INT), table.bb);
      return obj;
    } else {
      return null;
    }
  }

  public static int getVectorLength(Table table, Field field) {
    assert checkFieldType(field.type().baseType(), Vector);
    int o = table.__offset(field.offset());
    return o != 0 ? table.__vector_len(o) : 0;
  }

  /**
   * Check if the 2 given types are the same. If not throw an {@link IllegalArgumentException}
   *
   * @param actual   the actual type to check against the expected.
   * @param expected the expected type whose the actual must be equal to.
   * @throws IllegalArgumentException if actual and expected types are not equal.
   * @see BaseType
   */
  private static boolean checkFieldType(byte actual, byte expected) {
    if (actual != expected) {
      throw new IllegalArgumentException("Invalid type expected: " + BaseType.name(expected)
          + ", but was: " + BaseType.name(actual));
    } else {
      return true;
    }
  }

}
