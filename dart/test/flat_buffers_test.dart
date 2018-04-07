// Copyright (c) 2016, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'dart:typed_data';

import 'package:flat_buffers/flat_buffers.dart';
import 'package:test/test.dart';
import 'package:test_reflective_loader/test_reflective_loader.dart';

main() {
  defineReflectiveSuite(() {
    defineReflectiveTests(BuilderTest);
  });
}

int indexToField(int index) {
  return (1 + 1 + index) * 2;
}

@reflectiveTest
class BuilderTest {
  void test_error_addInt32_withoutStartTable() {
    Builder builder = new Builder();
    expect(() {
      builder.addInt32(0, 0);
    }, throwsStateError);
  }

  void test_error_addOffset_withoutStartTable() {
    Builder builder = new Builder();
    expect(() {
      builder.addOffset(0, 0);
    }, throwsStateError);
  }

  void test_error_endTable_withoutStartTable() {
    Builder builder = new Builder();
    expect(() {
      builder.endTable();
    }, throwsStateError);
  }

  void test_error_startTable_duringTable() {
    Builder builder = new Builder();
    builder.startTable();
    expect(() {
      builder.startTable();
    }, throwsStateError);
  }

  void test_error_writeString_duringTable() {
    Builder builder = new Builder();
    builder.startTable();
    expect(() {
      builder.writeString('12345');
    }, throwsStateError);
  }

  void test_file_identifier() {
    Uint8List byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      builder.startTable();
      int offset = builder.endTable();
      byteList = builder.finish(offset, 'Az~ÿ');
    }
    // Convert byteList to a ByteData so that we can read data from it.
    ByteData byteData = byteList.buffer.asByteData(byteList.offsetInBytes);
    // First 4 bytes are an offset to the table data.
    int tableDataLoc = byteData.getUint32(0, Endianness.LITTLE_ENDIAN);
    // Next 4 bytes are the file identifier.
    expect(byteData.getUint8(4), 65); // 'a'
    expect(byteData.getUint8(5), 122); // 'z'
    expect(byteData.getUint8(6), 126); // '~'
    expect(byteData.getUint8(7), 255); // 'ÿ'
    // First 4 bytes of the table data are a backwards offset to the vtable.
    int vTableLoc = tableDataLoc -
        byteData.getInt32(tableDataLoc, Endianness.LITTLE_ENDIAN);
    // First 2 bytes of the vtable are the size of the vtable in bytes, which
    // should be 4.
    expect(byteData.getUint16(vTableLoc, Endianness.LITTLE_ENDIAN), 4);
    // Next 2 bytes are the size of the object in bytes (including the vtable
    // pointer), which should be 4.
    expect(byteData.getUint16(vTableLoc + 2, Endianness.LITTLE_ENDIAN), 4);
  }

  void test_low() {
    Builder builder = new Builder(initialSize: 0);
    expect((builder..putUint8(1)).lowFinish(), [1]);
    expect((builder..putUint32(2)).lowFinish(), [2, 0, 0, 0, 0, 0, 0, 1]);
    expect((builder..putUint8(3)).lowFinish(),
        [0, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 1]);
    expect((builder..putUint8(4)).lowFinish(),
        [0, 0, 4, 3, 2, 0, 0, 0, 0, 0, 0, 1]);
    expect((builder..putUint8(5)).lowFinish(),
        [0, 5, 4, 3, 2, 0, 0, 0, 0, 0, 0, 1]);
    expect((builder..putUint32(6)).lowFinish(),
        [6, 0, 0, 0, 0, 5, 4, 3, 2, 0, 0, 0, 0, 0, 0, 1]);
  }

  void test_table_default() {
    List<int> byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      builder.startTable();
      builder.addInt32(0, 10, 10);
      builder.addInt32(1, 20, 10);
      int offset = builder.endTable();
      byteList = builder.finish(offset);
    }
    // read and verify
    BufferContext buffer = new BufferContext.fromBytes(byteList);
    int objectOffset = buffer.derefObject(0);
    // was not written, so uses the new default value
    expect(
        const Int32Reader()
            .vTableGet(buffer, objectOffset, indexToField(0), 15),
        15);
    // has the written value
    expect(
        const Int32Reader()
            .vTableGet(buffer, objectOffset, indexToField(1), 15),
        20);
  }

  void test_table_format() {
    Uint8List byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      builder.startTable();
      builder.addInt32(0, 10);
      builder.addInt32(1, 20);
      builder.addInt32(2, 30);
      byteList = builder.finish(builder.endTable());
    }
    // Convert byteList to a ByteData so that we can read data from it.
    ByteData byteData = byteList.buffer.asByteData(byteList.offsetInBytes);
    // First 4 bytes are an offset to the table data.
    int tableDataLoc = byteData.getUint32(0, Endianness.LITTLE_ENDIAN);
    // First 4 bytes of the table data are a backwards offset to the vtable.
    int vTableLoc = tableDataLoc -
        byteData.getInt32(tableDataLoc, Endianness.LITTLE_ENDIAN);
    // First 2 bytes of the vtable are the size of the vtable in bytes, which
    // should be 10.
    expect(byteData.getUint16(vTableLoc, Endianness.LITTLE_ENDIAN), 10);
    // Next 2 bytes are the size of the object in bytes (including the vtable
    // pointer), which should be 16.
    expect(byteData.getUint16(vTableLoc + 2, Endianness.LITTLE_ENDIAN), 16);
    // Remaining 6 bytes are the offsets within the object where the ints are
    // located.
    for (int i = 0; i < 3; i++) {
      int offset =
          byteData.getUint16(vTableLoc + 4 + 2 * i, Endianness.LITTLE_ENDIAN);
      expect(byteData.getInt32(tableDataLoc + offset, Endianness.LITTLE_ENDIAN),
          10 + 10 * i);
    }
  }

  void test_table_string() {
    String latinString = 'test';
    String unicodeString = 'Проба пера';
    List<int> byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      int latinStringOffset = builder.writeString(latinString);
      int unicodeStringOffset = builder.writeString(unicodeString);
      builder.startTable();
      builder.addOffset(0, latinStringOffset);
      builder.addOffset(1, unicodeStringOffset);
      int offset = builder.endTable();
      byteList = builder.finish(offset);
    }
    // read and verify
    BufferContext buf = new BufferContext.fromBytes(byteList);
    int objectOffset = buf.derefObject(0);
    expect(const StringReader().vTableGet(buf, objectOffset, indexToField(0)),
        latinString);
    expect(const StringReader().vTableGet(buf, objectOffset, indexToField(1)),
        unicodeString);
  }

  void test_table_types() {
    List<int> byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      int stringOffset = builder.writeString('12345');
      builder.startTable();
      builder.addBool(0, true);
      builder.addInt8(1, 10);
      builder.addInt32(2, 20);
      builder.addOffset(3, stringOffset);
      builder.addInt32(4, 40);
      builder.addUint32(5, 0x9ABCDEF0);
      builder.addUint8(6, 0x9A);
      int offset = builder.endTable();
      byteList = builder.finish(offset);
    }
    // read and verify
    BufferContext buf = new BufferContext.fromBytes(byteList);
    int objectOffset = buf.derefObject(0);
    expect(
        const BoolReader().vTableGet(buf, objectOffset, indexToField(0)), true);
    expect(
        const Int8Reader().vTableGet(buf, objectOffset, indexToField(1)), 10);
    expect(
        const Int32Reader().vTableGet(buf, objectOffset, indexToField(2)), 20);
    expect(const StringReader().vTableGet(buf, objectOffset, indexToField(3)),
        '12345');
    expect(
        const Int32Reader().vTableGet(buf, objectOffset, indexToField(4)), 40);
    expect(const Uint32Reader().vTableGet(buf, objectOffset, indexToField(5)),
        0x9ABCDEF0);
    expect(const Uint8Reader().vTableGet(buf, objectOffset, indexToField(6)),
        0x9A);
  }

  void test_writeList_of_Uint32() {
    List<int> values = <int>[10, 100, 12345, 0x9abcdef0];
    // write
    List<int> byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      int offset = builder.writeListUint32(values);
      byteList = builder.finish(offset);
    }
    // read and verify
    BufferContext buf = new BufferContext.fromBytes(byteList);
    List<int> items = const Uint32ListReader().read(buf, 0);
    expect(items, hasLength(4));
    expect(items, orderedEquals(values));
  }

  void test_writeList_ofBool() {
    void verifyListBooleans(int len, List<int> trueBits) {
      // write
      List<int> byteList;
      {
        Builder builder = new Builder(initialSize: 0);
        List<bool> values = new List<bool>.filled(len, false);
        for (int bit in trueBits) {
          values[bit] = true;
        }
        int offset = builder.writeListBool(values);
        byteList = builder.finish(offset);
      }
      // read and verify
      BufferContext buf = new BufferContext.fromBytes(byteList);
      List<bool> items = const BoolListReader().read(buf, 0);
      expect(items, hasLength(len));
      for (int i = 0; i < items.length; i++) {
        expect(items[i], trueBits.contains(i), reason: 'bit $i of $len');
      }
    }

    verifyListBooleans(0, <int>[]);
    verifyListBooleans(1, <int>[]);
    verifyListBooleans(1, <int>[0]);
    verifyListBooleans(31, <int>[0, 1]);
    verifyListBooleans(31, <int>[1, 2, 24, 25, 30]);
    verifyListBooleans(31, <int>[0, 30]);
    verifyListBooleans(32, <int>[1, 2, 24, 25, 31]);
    verifyListBooleans(33, <int>[1, 2, 24, 25, 32]);
    verifyListBooleans(33, <int>[1, 2, 24, 25, 31, 32]);
    verifyListBooleans(63, <int>[]);
    verifyListBooleans(63, <int>[0, 1, 2, 61, 62]);
    verifyListBooleans(63, new List<int>.generate(63, (i) => i));
    verifyListBooleans(64, <int>[]);
    verifyListBooleans(64, <int>[0, 1, 2, 61, 62, 63]);
    verifyListBooleans(64, <int>[1, 2, 62]);
    verifyListBooleans(64, <int>[0, 1, 2, 63]);
    verifyListBooleans(64, new List<int>.generate(64, (i) => i));
    verifyListBooleans(100, <int>[0, 3, 30, 60, 90, 99]);
  }

  void test_writeList_ofInt32() {
    List<int> byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      int offset = builder.writeListInt32(<int>[1, 2, 3, 4, 5]);
      byteList = builder.finish(offset);
    }
    // read and verify
    BufferContext buf = new BufferContext.fromBytes(byteList);
    List<int> items = const ListReader<int>(const Int32Reader()).read(buf, 0);
    expect(items, hasLength(5));
    expect(items, orderedEquals(<int>[1, 2, 3, 4, 5]));
  }

  void test_writeList_ofFloat64() {
    List<double> values = <double>[-1.234567, 3.4E+9, -5.6E-13, 7.8, 12.13];
    // write
    List<int> byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      int offset = builder.writeListFloat64(values);
      byteList = builder.finish(offset);
    }

    // read and verify
    BufferContext buf = new BufferContext.fromBytes(byteList);
    List<double> items = const Float64ListReader().read(buf, 0);

    expect(items, hasLength(values.length));
    for (int i = 0; i < values.length; i++) {
      expect(values[i], closeTo(items[i], .001));
    }
  }

  void test_writeList_ofFloat32() {
    List<double> values = [1.0, 2.23, -3.213, 7.8, 12.13];
    // write
    List<int> byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      int offset = builder.writeListFloat32(values);
      byteList = builder.finish(offset);
    }
    // read and verify
    BufferContext buf = new BufferContext.fromBytes(byteList);
    List<double> items = const Float32ListReader().read(buf, 0);
    expect(items, hasLength(5));
    for (int i = 0; i < values.length; i++) {
      expect(values[i], closeTo(items[i], .001));
    }
  }

  void test_writeList_ofObjects() {
    List<int> byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      // write the object #1
      int object1;
      {
        builder.startTable();
        builder.addInt32(0, 10);
        builder.addInt32(1, 20);
        object1 = builder.endTable();
      }
      // write the object #1
      int object2;
      {
        builder.startTable();
        builder.addInt32(0, 100);
        builder.addInt32(1, 200);
        object2 = builder.endTable();
      }
      // write the list
      int offset = builder.writeList([object1, object2]);
      byteList = builder.finish(offset);
    }
    // read and verify
    BufferContext buf = new BufferContext.fromBytes(byteList);
    List<TestPointImpl> items =
        const ListReader<TestPointImpl>(const TestPointReader()).read(buf, 0);
    expect(items, hasLength(2));
    expect(items[0].x, 10);
    expect(items[0].y, 20);
    expect(items[1].x, 100);
    expect(items[1].y, 200);
  }

  void test_writeList_ofStrings_asRoot() {
    List<int> byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      int str1 = builder.writeString('12345');
      int str2 = builder.writeString('ABC');
      int offset = builder.writeList([str1, str2]);
      byteList = builder.finish(offset);
    }
    // read and verify
    BufferContext buf = new BufferContext.fromBytes(byteList);
    List<String> items =
        const ListReader<String>(const StringReader()).read(buf, 0);
    expect(items, hasLength(2));
    expect(items, contains('12345'));
    expect(items, contains('ABC'));
  }

  void test_writeList_ofStrings_inObject() {
    List<int> byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      int listOffset = builder.writeList(
          [builder.writeString('12345'), builder.writeString('ABC')]);
      builder.startTable();
      builder.addOffset(0, listOffset);
      int offset = builder.endTable();
      byteList = builder.finish(offset);
    }
    // read and verify
    BufferContext buf = new BufferContext.fromBytes(byteList);
    StringListWrapperImpl reader = new StringListWrapperReader().read(buf, 0);
    List<String> items = reader.items;
    expect(items, hasLength(2));
    expect(items, contains('12345'));
    expect(items, contains('ABC'));
  }

  void test_writeList_ofUint32() {
    List<int> byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      int offset = builder.writeListUint32(<int>[1, 2, 0x9ABCDEF0]);
      byteList = builder.finish(offset);
    }
    // read and verify
    BufferContext buf = new BufferContext.fromBytes(byteList);
    List<int> items = const Uint32ListReader().read(buf, 0);
    expect(items, hasLength(3));
    expect(items, orderedEquals(<int>[1, 2, 0x9ABCDEF0]));
  }

 void test_writeList_ofUint16() {
    List<int> byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      int offset = builder.writeListUint16(<int>[1, 2, 60000]);
      byteList = builder.finish(offset);
    }
    // read and verify
    BufferContext buf = new BufferContext.fromBytes(byteList);
    List<int> items = const Uint16ListReader().read(buf, 0);
    expect(items, hasLength(3));
    expect(items, orderedEquals(<int>[1, 2, 60000]));
  }
  void test_writeList_ofUint8() {
    List<int> byteList;
    {
      Builder builder = new Builder(initialSize: 0);
      int offset = builder.writeListUint8(<int>[1, 2, 3, 4, 0x9A]);
      byteList = builder.finish(offset);
    }
    // read and verify
    BufferContext buf = new BufferContext.fromBytes(byteList);
    List<int> items = const Uint8ListReader().read(buf, 0);
    expect(items, hasLength(5));
    expect(items, orderedEquals(<int>[1, 2, 3, 4, 0x9A]));
  }
}

class StringListWrapperImpl {
  final BufferContext bp;
  final int offset;

  StringListWrapperImpl(this.bp, this.offset);

  List<String> get items => const ListReader<String>(const StringReader())
      .vTableGet(bp, offset, indexToField(0));
}

class StringListWrapperReader extends TableReader<StringListWrapperImpl> {
  const StringListWrapperReader();

  @override
  StringListWrapperImpl createObject(BufferContext object, int offset) {
    return new StringListWrapperImpl(object, offset);
  }
}

class TestPointImpl {
  final BufferContext bp;
  final int offset;

  TestPointImpl(this.bp, this.offset);

  int get x => const Int32Reader().vTableGet(bp, offset, indexToField(0), 0);

  int get y => const Int32Reader().vTableGet(bp, offset, indexToField(1), 0);
}

class TestPointReader extends TableReader<TestPointImpl> {
  const TestPointReader();

  @override
  TestPointImpl createObject(BufferContext object, int offset) {
    return new TestPointImpl(object, offset);
  }
}
