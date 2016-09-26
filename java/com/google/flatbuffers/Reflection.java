package com.google.flatbuffers;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import com.google.flatbuffers.reflection.Field;

/**
 * Help class to manipulate flatbuffers document by reflection.
 */
public final class Reflection {
  private Reflection() {
    throw new AssertionError("Not for you!");
  }

  public static Table getRootTable(ByteBuffer _bb) {
    return getRootTable(_bb, new Table());
  }

  public static Table getRootTable(ByteBuffer _bb, Table obj) {
    _bb.order(ByteOrder.LITTLE_ENDIAN);
    return obj._init(_bb.getInt(_bb.position()) + _bb.position(), _bb);
  }

  public static int getIntField(Table root, Field field) {
    return (int)field.defaultInteger();
  }
}
