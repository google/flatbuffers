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

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import static com.google.flatbuffers.Constants.SIZEOF_DOUBLE;
import static com.google.flatbuffers.Constants.SIZEOF_FLOAT;
import static com.google.flatbuffers.Constants.SIZEOF_INT;
import static com.google.flatbuffers.Constants.SIZEOF_LONG;
import static com.google.flatbuffers.Constants.SIZEOF_SHORT;
import static com.google.flatbuffers.Constants.UBYTE_MASK;
import static com.google.flatbuffers.Constants.UINT_MASK;
import static com.google.flatbuffers.Constants.USHORT_MASK;
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
 * Helper class to manipulate flatbuffers document by reflection.
 *
 * <p>Reflection can be used to manipulate byte buffers of flatbuffers objects from their binary
 * flatbuffers schema without generated code.</p>
 *
 * <h4>Usage</h4> We assume you have a valid binary schema called {@code monster.bfbs} and you have
 * loaded it into a {@link ByteBuffer}.
 * <pre>
 *   // load the schema from a byte buffer
 *   Schema schema = Schema.getRootAsSchema(bb);
 *   // then you get the root table of the schema as follow.
 *   com.google.flatbuffers.reflection.Object rootTable = schema.rootTable();
 *   // you can get a field definition from a table definition
 *   Field hpField = rootTable.fieldsByKey("hp");
 *   // you can check if a field is present or not
 *   boolean hasHp = Reflection.isFieldPresent(root, hpField);
 *   // you can get the value associated to field
 *   short shortHp = Reflection.getShortField(root, hpField);
 *   // you can set the value of a field
 *   Reflection.setShortField(root, hpField, (short) 200);
 * </pre>
 * <p><strong>Parameter checking:</strong> the check of the parameters is only activated if you
 * enable the assertions at the VM arguments. You activate the assertion by adding {@code -ea}
 * option to the VM arguments.</p>
 */
public final class Reflection {

  private Reflection() {
  }

  /**
   * Returns the root table of a given byte buffer.
   * <p>Create a new instance of {@link Table} initialize it to the root table
   * stored in the byte buffer.</p>
   *
   * @param _bb the byte buffer whose root table is to be instantiated.
   * @return the root table from the given byte buffer.
   * @see #getRootTable(ByteBuffer, Table)
   */
  public static Table getRootTable(ByteBuffer _bb) {
    return getRootTable(_bb, new Table());
  }

  /**
   * Returns the given table initialized to the root table from the byte buffer. <p>Initialize the
   * given table to the root table stored into the byte buffer. This method can be used instead of
   * the {@link #getRootTable(ByteBuffer)} inorder to reuse the table instance.</p>
   *
   * @param _bb the byte buffer whose root table is to be initialized.
   * @param obj the table to initialized.
   * @return the table {@code obj} initialized to the root table stored in the given byte buffer
   * {@code _bb}.
   * @see #getRootTable(ByteBuffer)
   */
  public static Table getRootTable(ByteBuffer _bb, Table obj) {
    _bb.order(ByteOrder.LITTLE_ENDIAN);
    obj.__init(_bb.getInt(_bb.position()) + _bb.position(), _bb);
    return obj;
  }

  /**
   * Returns <tt>true</tt> if the <tt>table</tt> contains the specified <tt>field</tt>. More
   * formally, returns <tt>true</tt> if and only if this <tt>table</tt> has an offset different from
   * <tt>0</tt> at the <tt>field</tt> offset position.
   *
   * @param table the table whose presence of the field is to be tested.
   * @param field the field whose presence in the table is to be tested.
   * @return <tt>true</tt> if the <tt>table</tt> contains the specified <tt>field</tt>.
   */
  public static boolean isFieldPresent(Table table, Field field) {
    int o = table.__offset(field.offset());
    return o != 0;
  }

  /**
   * Gets the boolean value for the given field. Returns the value if the field is present otherwise
   * the default value associated to the field is returned.
   *
   * <p>This method is a precise type method, the field should have the same type than this method
   * name implies. If you want you can enable the validation of the field type by activating the
   * <tt>-ea</tt> VM argument. No special check are made to validate that the field belongs to the
   * given table it is the responsability of the caller to ensure this consistency.</p>
   *
   * @param table the table whose contains the value associated to the field
   * @param field the field whose associated value is to be returned from the table
   * @return the value associated to the field if present or the field default value if not present.
   * @throws IllegalArgumentException if the field type is not the equal to {@link BaseType#Bool}
   *                                  and the <tt>-ea</tt> VM argument is activated
   * @see #getBoolField(Table, Field, boolean)
   * @see #isFieldPresent(Table, Field)
   */
  public static boolean getBoolField(Table table, Field field) {
    return getBoolField(table, field, 0 != field.defaultInteger());
  }

  /**
   * Gets the boolean value for the given field. Returns the value if the field is present otherwise
   * the default value associated to the field is returned.
   *
   * <p>This method is a precise type method, the field should have the same type than this method
   * name implies. If you want you can enable the validation of the field type by activating the
   * <tt>-ea</tt> VM argument. No special check are made to validate that the field belongs to the
   * given table it is the responsability of the caller to ensure this consistency.</p>
   *
   * @param table        the table whose contains the value associated to the field
   * @param field        the field whose associated value is to be returned from the table
   * @param defaultValue the default value to be returned if the field is not present
   * @return the value associated to the field if present or the field default value if not present.
   * @throws IllegalArgumentException if the field type is not the equal to {@link BaseType#Bool}
   *                                  and the <tt>-ea</tt> VM argument is activated
   * @see #getBoolField(Table, Field)
   * @see #isFieldPresent(Table, Field)
   * @see #setBoolField(Table, Field, boolean)
   */
  public static boolean getBoolField(Table table, Field field, boolean defaultValue) {
    assert checkFieldType(field.type().baseType(), Bool);
    int o = table.__offset(field.offset());
    return o != 0 ? 0 != table.bb.get(o + table.bb_pos) : defaultValue;
  }

  /**
   * Replaces the current field value with the given new value. The value is replaced only if the
   * field is present. Returns <tt>true</tt> if the field is present and therefore the value can be
   * replaced otherwise <tt>false</tt>.
   *
   * <p>This method is a precise type method, the field should have the same type than this method
   * name implies. If you want you can enable the validation of the field type by activating the
   * <tt>-ea</tt> VM argument. No special check are made to validate that the field belongs to the
   * given table it is the responsability of the caller to ensure this consistency.</p>
   *
   * @param table the table whose the value associated is to be associated to the field
   * @param field the field whose associated value is to be associated
   * @param value the value to be stored at the field position
   * @return <tt>true</tt> if the field is present and therefore the value can be replaced otherwise
   * <tt>false</tt>
   * @throws IllegalArgumentException if the field type is not the equal to {@link BaseType#Bool}
   *                                  and the <tt>-ea</tt> VM argument is activated
   * @see #getBoolField(Table, Field)
   * @see #getBoolField(Table, Field, boolean)
   * @see #isFieldPresent(Table, Field)
   */
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

  /**
   * Gets the boolean value for the given array field at the given position. Returns the value if
   * the field array is present otherwise the given default value.
   *
   * <p>This method is a precise type method, the field should have the same array element type than
   * this method name implies. If you want you can enable the validation of the field type by
   * activating the <tt>-ea</tt> VM argument. No special check are made to validate that the field
   * belongs to the given table it is the responsability of the caller to ensure this
   * consistency.</p>
   *
   * @param table the table whose contains the value associated to the field
   * @param field the array field whose associated value is to be returned from the table
   * @param index the index of the value in the array to return
   * @return the value associated to the array field at the given index if the field is present or
   * the field default value if not present.
   * @throws IllegalArgumentException if the array field element type is not the equal to a {@link
   *                                  BaseType#Vector} of {@link BaseType#Bool} and the <tt>-ea</tt>
   *                                  VM argument is activated
   * @see #getBoolsField(Table, Field, boolean, int)
   * @see #isFieldPresent(Table, Field)
   * @see #setBoolsField(Table, Field, boolean, int)
   */
  public static boolean getBoolsField(Table table, Field field, int index) {
    return getBoolsField(table, field, false, index);
  }

  /**
   * Gets the boolean value of the element of the array pointed by the given field at the given
   * index. Returns the value if the field array is present otherwise the given default value.
   *
   * <p>This method is a precise type method, the field should have the same array element type than
   * this method return type. If you want you can enable the validation of the field type by
   * activating the <tt>-ea</tt> VM argument. No special check are made to validate that the field
   * belongs to the given table it is the responsability of the caller to ensure this
   * consistency.</p>
   *
   * @param table        the table whose contains the value associated to the field
   * @param field        the array field whose associated value is to be returned from the table
   * @param defaultValue the default value to be returned if the array field is not present
   * @param index        the index of the element in the array to return
   * @return the value associated to the array field at the given index if the field is present or
   * the field default value if not present.
   * @throws IllegalArgumentException if the array field element type is not the equal to {@link
   *                                  BaseType#Vector} of {@link BaseType#Bool} and the <tt>-ea</tt>
   *                                  VM argument is activated
   * @see #getBoolsField(Table, Field, int)
   * @see #isFieldPresent(Table, Field)
   */
  public static boolean getBoolsField(Table table, Field field, boolean defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Bool);
    int o = table.__offset(field.offset());
    return o != 0 ? 0 != table.bb.get(table.__vector(o) + index) : defaultValue;
  }

  public static boolean[] getBoolsField(Table table, Field field) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Bool);
    int vectorLength = getVectorLength(table, field);
    boolean[] vector = new boolean[vectorLength];
    int o = table.__offset(field.offset());
    for (int index = 0; index < vectorLength; index++) {
      vector[index] = 0 != table.bb.get(table.__vector(o) + index);
    }
    return vector;
  }

  /**
   * Replaces the current field array element with the given new value at the given index. The
   * element is replaced only if the field is present. Returns <tt>true</tt> if the field is present
   * and therefore the value can be replaced otherwise <tt>false</tt>.
   *
   * <p>This method is a precise type method, the field should have the same array element type than
   * this method name implies. If you want you can enable the validation of the field type by
   * activating the <tt>-ea</tt> VM argument. No special check are made to validate that the field
   * belongs to the given table it is the responsability of the caller to ensure this
   * consistency.</p>
   *
   * @param table the table whose the value associated is to be associated to the field
   * @param field the field whose associated value is to be associated
   * @param value the value to be stored at the field position at the given index
   * @param index the index of the element to replace
   * @return <tt>true</tt> if the field is present and therefore the element value can be replaced
   * otherwise <tt>false</tt>
   * @throws IllegalArgumentException if the field type is not the equal to {@link BaseType#Vector}
   *                                  of {@link BaseType#Bool} and the <tt>-ea</tt> VM argument is
   *                                  activated
   * @see #getBoolsField(Table, Field, int)
   * @see #getBoolsField(Table, Field, boolean, int)
   * @see #isFieldPresent(Table, Field)
   */
  public static boolean setBoolsField(Table table, Field field, boolean value, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Bool);
    int o = table.__offset(field.offset());
    if (o != 0) {
      table.bb.put(table.__vector(o) + index, (byte) (value ? 1 : 0));
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

  public static byte getBytesField(Table table, Field field, int index) {
    return getBytesField(table, field, (byte) 0, index);
  }

  public static byte getBytesField(Table table, Field field, byte defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Byte);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.get(table.__vector(o) + index) : defaultValue;
  }

  public static byte[] getBytesField(Table table, Field field) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Byte);
    int vectorLength = getVectorLength(table, field);
    byte[] vector = new byte[vectorLength];
    if (vectorLength != 0) {
      int o = table.__offset(field.offset());
      o = table.__vector(o);
      for (int index = 0; index < vectorLength; index++) {
        vector[index] = table.bb.get(o + index);
      }
    }
    return vector;
  }

  public static int getUByteField(Table table, Field field) {
    return getUByteField(table, field, (int) field.defaultInteger());
  }

  public static int getUByteField(Table table, Field field, int defaultValue) {
    assert checkFieldType(field.type().baseType(), UByte);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.get(o + table.bb_pos) & UBYTE_MASK : defaultValue;
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

  public static int getUBytesField(Table table, Field field, int index) {
    return getUBytesField(table, field, 0, index);
  }

  public static int getUBytesField(Table table, Field field, int defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), UByte);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.get(table.__vector(o) + index) & UBYTE_MASK : defaultValue;
  }

  public static int[] getUBytesField(Table table, Field field) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), UByte);
    int vectorLength = getVectorLength(table, field);
    int[] vector = new int[vectorLength];
    if (vectorLength != 0) {
      int o = table.__offset(field.offset());
      o = table.__vector(o);
      for (int index = 0; index < vectorLength; index++) {
        vector[index] = table.bb.get(o + index) & UBYTE_MASK;
      }
    }
    return vector;
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

  public static short[] getShortsField(Table table, Field field) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Short);
    int vectorLength = getVectorLength(table, field);
    short[] vector = new short[vectorLength];
    if (vectorLength != 0) {
      int o = table.__offset(field.offset());
      o = table.__vector(o);
      for (int index = 0; index < vectorLength; index++) {
        vector[index] = table.bb.getShort(o + index * SIZEOF_SHORT);
      }
    }
    return vector;
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

  public static int getUShortsField(Table table, Field field, int index) {
    return getUShortsField(table, field, 0, index);
  }

  public static int getUShortsField(Table table, Field field, int defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), UShort);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getShort(table.__vector(o) + index * SIZEOF_SHORT) & USHORT_MASK : defaultValue;
  }

  public static int[] getUShortsField(Table table, Field field) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), UShort);
    int vectorLength = getVectorLength(table, field);
    int[] vector = new int[vectorLength];
    if (vectorLength != 0) {
      int o = table.__offset(field.offset());
      o = table.__vector(o);
      for (int index = 0; index < vectorLength; index++) {
        vector[index] = table.bb.getShort(o + index * SIZEOF_SHORT) & USHORT_MASK;
      }
    }
    return vector;
  }

  public static int getIntField(Table table, Field field) {
    return getIntField(table, field, (int) field.defaultInteger());
  }

  public static int getIntField(Table table, Field field, int defaultValue) {
    assert checkFieldType(field.type().baseType(), Int);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getInt(o + table.bb_pos) : defaultValue;
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

  public static int getIntsField(Table table, Field field, int index) {
    return getIntsField(table, field, 0, index);
  }

  public static int getIntsField(Table table, Field field, int defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Int);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getInt(table.__vector(o) + index * SIZEOF_INT) : defaultValue;
  }

  public static int[] getIntsField(Table table, Field field) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Int);
    int vectorLength = getVectorLength(table, field);
    int[] vector = new int[vectorLength];
    if (vectorLength != 0) {
      int o = table.__offset(field.offset());
      o = table.__vector(o);
      for (int index = 0; index < vectorLength; index++) {
        vector[index] = table.bb.getInt(o + index * SIZEOF_INT);
      }
    }
    return vector;
  }

  public static long getUIntField(Table table, Field field) {
    return getUIntField(table, field, field.defaultInteger());
  }

  public static long getUIntField(Table table, Field field, long defaultValue) {
    assert checkFieldType(field.type().baseType(), UInt);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getInt(o + table.bb_pos) & UINT_MASK : defaultValue;
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

  public static long getUIntsField(Table table, Field field, int index) {
    return getUIntsField(table, field, 0L, index);
  }

  public static long getUIntsField(Table table, Field field, long defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), UInt);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getInt(table.__vector(o) + index * SIZEOF_INT) & UINT_MASK : defaultValue;
  }

  public static long[] getUIntsField(Table table, Field field) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), UInt);
    int vectorLength = getVectorLength(table, field);
    long[] vector = new long[vectorLength];
    if (vectorLength != 0) {
      int o = table.__offset(field.offset());
      o = table.__vector(o);
      for (int index = 0; index < vectorLength; index++) {
        vector[index] = table.bb.getInt(o + index * SIZEOF_INT) & UINT_MASK;
      }
    }
    return vector;
  }

  public static long getLongField(Table table, Field field) {
    return getLongField(table, field, field.defaultInteger());
  }

  public static long getLongField(Table table, Field field, long defaultValue) {
    assert checkFieldType(field.type().baseType(), Long);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getLong(o + table.bb_pos) : defaultValue;
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

  public static long getLongsField(Table table, Field field, int index) {
    return getLongsField(table, field, 0L, index);
  }

  public static long getLongsField(Table table, Field field, long defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Long);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getLong(table.__vector(o) + index * SIZEOF_LONG) : defaultValue;
  }

  public static long[] getLongsField(Table table, Field field) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Long);
    int vectorLength = getVectorLength(table, field);
    long[] vector = new long[vectorLength];
    if (vectorLength != 0) {
      int o = table.__offset(field.offset());
      o = table.__vector(o);
      for (int index = 0; index < vectorLength; index++) {
        vector[index] = table.bb.getLong(o + index * SIZEOF_LONG);
      }
    }
    return vector;
  }


  public static long getULongField(Table table, Field field) {
    return getULongField(table, field, field.defaultInteger());
  }

  public static long getULongField(Table table, Field field, long defaultValue) {
    assert checkFieldType(field.type().baseType(), ULong);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getLong(o + table.bb_pos) : defaultValue;
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

  public static long getULongsField(Table table, Field field, int index) {
    return getULongsField(table, field, 0L, index);
  }

  public static long getULongsField(Table table, Field field, long defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), ULong);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getLong(table.__vector(o) + index * SIZEOF_LONG) : defaultValue;
  }

  public static long[] getULongsField(Table table, Field field) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), ULong);
    int vectorLength = getVectorLength(table, field);
    long[] vector = new long[vectorLength];
    if (vectorLength != 0) {
      int o = table.__offset(field.offset());
      o = table.__vector(o);
      for (int index = 0; index < vectorLength; index++) {
        vector[index] = table.bb.getLong(o + index * SIZEOF_LONG);
      }
    }
    return vector;
  }

  public static float getFloatField(Table table, Field field) {
    return getFloatField(table, field, field.defaultInteger());
  }

  public static float getFloatField(Table table, Field field, float defaultValue) {
    assert checkFieldType(field.type().baseType(), Float);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getFloat(o + table.bb_pos) : defaultValue;
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

  public static float getFloatsField(Table table, Field field, int index) {
    return getFloatsField(table, field, 0F, index);
  }

  public static float getFloatsField(Table table, Field field, float defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Float);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getFloat(table.__vector(o) + index * SIZEOF_FLOAT) : defaultValue;
  }

  public static float[] getFloatsField(Table table, Field field) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Float);
    int vectorLength = getVectorLength(table, field);
    float[] vector = new float[vectorLength];
    if (vectorLength != 0) {
      int o = table.__offset(field.offset());
      o = table.__vector(o);
      for (int index = 0; index < vectorLength; index++) {
        vector[index] = table.bb.getFloat(o + index * SIZEOF_FLOAT);
      }
    }
    return vector;
  }


  public static double getDoubleField(Table table, Field field) {
    return getDoubleField(table, field, field.defaultInteger());
  }

  public static double getDoubleField(Table table, Field field, double defaultValue) {
    assert checkFieldType(field.type().baseType(), Double);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getDouble(o + table.bb_pos) : defaultValue;
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

  public static double getDoublesField(Table table, Field field, int index) {
    return getDoublesField(table, field, 0D, index);
  }

  public static double getDoublesField(Table table, Field field, double defaultValue, int index) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Double);
    int o = table.__offset(field.offset());
    return o != 0 ? table.bb.getDouble(table.__vector(o) + index * SIZEOF_DOUBLE) : defaultValue;
  }

  public static double[] getDoublesField(Table table, Field field) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Double);
    int vectorLength = getVectorLength(table, field);
    double[] vector = new double[vectorLength];
    if (vectorLength != 0) {
      int o = table.__offset(field.offset());
      o = table.__vector(o);
      for (int index = 0; index < vectorLength; index++) {
        vector[index] = table.bb.getDouble(o + index * SIZEOF_DOUBLE);
      }
    }
    return vector;
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

  public static String[] getStringsField(Table table, Field field) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), String);
    int vectorLength = getVectorLength(table, field);
    String[] vector = new String[vectorLength];
    if (vectorLength != 0) {
      int o = table.__offset(field.offset());
      o = table.__vector(o);
      for (int index = 0; index < vectorLength; index++) {
        vector[index] = table.__string(o + index * SIZEOF_INT);
      }
    }
    return vector;
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

  public static Table[] getObjsField(Table table, Field field) {
    assert checkFieldType(field.type().baseType(), Vector);
    assert checkFieldType(field.type().element(), Obj);
    int vectorLength = getVectorLength(table, field);
    Table[] vector = new Table[vectorLength];
    if (vectorLength != 0) {
      int o = table.__offset(field.offset());
      o = table.__vector(o);
      for (int index = 0; index < vectorLength; index++) {
        Table obj = new Table();
        obj.__init(table.__indirect(o + index * SIZEOF_INT), table.bb);
        vector[index] = obj;
      }
    }
    return vector;
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
