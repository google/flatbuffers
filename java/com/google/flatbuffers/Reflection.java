package com.google.flatbuffers;

import com.google.flatbuffers.reflection.BaseType;
import com.google.flatbuffers.reflection.Field;
import com.google.flatbuffers.reflection.Schema;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

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
    checkFieldType(field.type().baseType(), Bool);
    int o = table.__offset(field.offset());
    return o != 0 ? 0 != table.bb.get(o + table.bb_pos) : defaultValue;
  }

  public static boolean setBoolField(Table table, Field field, boolean value) {
    checkFieldType(field.type().baseType(), Bool);
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
    checkFieldType(field.type().baseType(), Byte);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.get(o + table.bb_pos) : defaultValue;
  }

  public static boolean setByteField(Table table, Field field, byte value) {
    checkFieldType(field.type().baseType(), Byte);
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
    checkFieldType(field.type().baseType(), UByte);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.get(o + table.bb_pos) & UBYTE_MASK : defaultValue;
  }

  public static boolean setUByteField(Table table, Field field, int value) {
    checkFieldType(field.type().baseType(), UByte);
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
    checkFieldType(field.type().baseType(), Short);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getShort(o + table.bb_pos) : defaultValue;
  }

  public static boolean setShortField(Table table, Field field, short value) {
    checkFieldType(field.type().baseType(), Short);
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
    checkFieldType(field.type().baseType(), UShort);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getShort(o + table.bb_pos) & USHORT_MASK : defaultValue;
  }

  public static boolean setUShortField(Table table, Field field, int value) {
    checkFieldType(field.type().baseType(), UShort);
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
    checkFieldType(field.type().baseType(), Int);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getInt(o + table.bb_pos)  : defaultValue;
  }

  public static boolean setIntField(Table table, Field field, int value) {
    checkFieldType(field.type().baseType(), Int);
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
    checkFieldType(field.type().baseType(), UInt);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getInt(o + table.bb_pos) & UINT_MASK : defaultValue;
  }

  public static boolean setUIntField(Table table, Field field, long value) {
    checkFieldType(field.type().baseType(), UInt);
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
    checkFieldType(field.type().baseType(), Long);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getLong(o + table.bb_pos) : defaultValue;
  }

  public static boolean setLongField(Table table, Field field, long value) {
    checkFieldType(field.type().baseType(), Long);
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
    checkFieldType(field.type().baseType(), ULong);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getLong(o + table.bb_pos) : defaultValue;
  }

  public static boolean setULongField(Table table, Field field, long value) {
    checkFieldType(field.type().baseType(), ULong);
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
    checkFieldType(field.type().baseType(), Float);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getFloat(o + table.bb_pos) : defaultValue;
  }

  public static boolean setFloatField(Table table, Field field, float value) {
    checkFieldType(field.type().baseType(), Float);
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
    checkFieldType(field.type().baseType(), Double);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getDouble(o + table.bb_pos) : defaultValue;
  }

  public static boolean setDoubleField(Table table, Field field, double value) {
    checkFieldType(field.type().baseType(), Double);
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
    checkFieldType(field.type().baseType(), String);
    int o = table.__offset(field.offset());
    return o != 0 ? table.__string(o + table.bb_pos) : defaultValue;
  }

  public static Table getObjField(Table table, Field field) {
    return getObjField(table, field, new Table());
  }

  public static Table getObjField(Table table, Field field, Table obj) {
    checkFieldType(field.type().baseType(), Obj);
    int o = table.__offset(field.offset());
    if (o != 0) {
      obj.__init(table.__indirect(o + table.bb_pos), table.bb);
      return obj;
    } else {
      return null;
    }
  }

  public static int getVectorLength(Table table, Field field) {
    checkFieldType(field.type().baseType(), Vector);
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
  private static void checkFieldType(byte actual, byte expected) {
    if (actual != expected) {
      throw new IllegalArgumentException("Invalid type expected: " + BaseType.name(expected) + ", but was: " + BaseType.name(actual));
    }
  }

}
