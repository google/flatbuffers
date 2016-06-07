package com.google.flatbuffers.test;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;

import org.junit.Test;

import com.google.flatbuffer.test.MyTable;
import com.google.flatbuffers.FlatBufferBuilder;

/**
 * Dummy Test to demo JUnit usage.
 */
public class DummyTest {
  @Test
  public void testDummy() {
    FlatBufferBuilder builder = new FlatBufferBuilder();

    int tableOffSet = MyTable.createMyTable(builder, 42);
    MyTable.finishMyTableBuffer(builder, tableOffSet);
    MyTable myTable = MyTable.getRootAsMyTable(builder.dataBuffer());

    assertThat(myTable.foo(), is(42));
  }
}
