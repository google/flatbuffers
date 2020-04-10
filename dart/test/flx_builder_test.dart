import 'dart:typed_data';

import 'package:flat_buffers/flex_buffers.dart';
import 'package:test/test.dart';

void main() {
  test('build with single value', () {
    {
      var flx = FlxBuilder();
      flx.addNull();
      expect(flx.finish(), [0, 0, 1]);
    }
    {
      var flx = FlxBuilder();
      flx.addBool(true);
      expect(flx.finish(), [1, 104, 1]);
    }
    {
      var flx = FlxBuilder();
      flx.addBool(false);
      expect(flx.finish(), [0, 104, 1]);
    }
    {
      var flx = FlxBuilder();
      flx.addInt(1);
      expect(flx.finish(), [1, 4, 1]);
    }
    {
      var flx = FlxBuilder();
      flx.addInt(230);
      expect(flx.finish(), [230, 0, 5, 2]);
    }
    {
      var flx = FlxBuilder();
      flx.addInt(1025);
      expect(flx.finish(), [1, 4, 5, 2]);
    }
    {
      var flx = FlxBuilder();
      flx.addInt(-1025);
      expect(flx.finish(), [255, 251, 5, 2]);
    }
    {
      var flx = FlxBuilder();
      flx.addDouble(0.1);
      expect(flx.finish(), [154, 153, 153, 153, 153, 153, 185, 63, 15, 8]);
    }
    {
      var flx = FlxBuilder();
      flx.addDouble(0.5);
      expect(flx.finish(), [0, 0, 0, 63, 14, 4]);
    }
    {
      var flx = FlxBuilder();
      flx.addString('Maxim');
      expect(flx.finish(), [5, 77, 97, 120, 105, 109, 0, 6, 20, 1]);
    }
    {
      var flx = FlxBuilder();
      flx.addString('hello 😱');
      expect(flx.finish(), [10, 104, 101, 108, 108, 111, 32, 240, 159, 152, 177, 0, 11, 20, 1]);
    }
  });

  test('build vector', (){
    {
      var flx = FlxBuilder()
        ..startVector()
        ..addInt(1)
        ..addInt(2)
        ..end()
      ;
      expect(flx.finish(), [1, 2, 2, 64, 1]);
    }
    {
      var flx = FlxBuilder()
        ..startVector()
        ..addInt(-1)
        ..addInt(256)
        ..end()
      ;
      expect(flx.finish(), [255, 255, 0, 1, 4, 65, 1]);
    }
    {
      var flx = FlxBuilder()
        ..startVector()
        ..addInt(-45)
        ..addInt(256000)
        ..end()
      ;
      expect(flx.finish(), [211, 255, 255, 255, 0, 232, 3, 0, 8, 66, 1]);
    }
    {
      var flx = FlxBuilder()
        ..startVector()
        ..addDouble(1.1)
        ..addDouble(-256)
        ..end()
      ;
      expect(flx.finish(), [154, 153, 153, 153, 153, 153, 241, 63, 0, 0, 0, 0, 0, 0, 112, 192, 16, 75, 1]);
    }
    {
      var flx = FlxBuilder()
        ..startVector()
        ..addInt(1)
        ..addInt(2)
        ..addInt(4)
        ..end()
      ;
      expect(flx.finish(), [1, 2, 4, 3, 76, 1]);
    }
    {
      var flx = FlxBuilder()
        ..startVector()
        ..addInt(-1)
        ..addInt(256)
        ..addInt(4)
        ..end()
      ;
      expect(flx.finish(), [255, 255, 0, 1, 4, 0, 6, 77, 1]);
    }
    {
      var flx = FlxBuilder()
        ..startVector()
          ..startVector()
          ..addInt(61)
          ..end()
        ..addInt(64)
        ..end()
      ;
      expect(flx.finish(), [1, 61, 2, 2, 64, 44, 4, 4, 40, 1]);
    }
    {
      var flx = FlxBuilder()
        ..startVector()
        ..addString('foo')
        ..addString('bar')
        ..addString('baz')
        ..end()
      ;
      expect(flx.finish(), [3, 102, 111, 111, 0, 3, 98, 97, 114, 0, 3, 98, 97, 122, 0, 3, 15, 11, 7, 3, 60, 1]);
    }
    {
      var flx = FlxBuilder()
        ..startVector()
        ..addString('foo')
        ..addString('bar')
        ..addString('baz')
        ..addString('foo')
        ..addString('bar')
        ..addString('baz')
        ..end()
      ;
      expect(flx.finish(), [3, 102, 111, 111, 0, 3, 98, 97, 114, 0, 3, 98, 97, 122, 0, 6, 15, 11, 7, 18, 14, 10, 6, 60, 1]);
    }
    {
      var flx = FlxBuilder()
        ..startVector()
        ..addBool(true)
        ..addBool(false)
        ..addBool(true)
        ..end()
      ;
      expect(flx.finish(), [3, 1, 0, 1, 3, 144, 1]);
    }
    {
      var flx = FlxBuilder()
        ..startVector()
        ..addString('foo')
        ..addInt(1)
        ..addInt(-5)
        ..addDouble(1.3)
        ..addBool(true)
        ..end()
      ;
      expect(flx.finish(), [
        3, 102, 111, 111, 0, 0, 0, 0,
        5, 0, 0, 0, 0, 0, 0, 0,
        15, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0,
        251, 255, 255, 255, 255, 255, 255, 255,
        205, 204, 204, 204, 204, 204, 244, 63,
        1, 0, 0, 0, 0, 0, 0, 0,
        20, 4, 4, 15, 104, 45, 43, 1]);
    }
  });

  test('build map', ()
  {
    {
      var flx = FlxBuilder()
        ..startMap()
        ..addKey('a')
        ..addInt(12)
        ..end()
      ;
      expect(flx.finish(), [97, 0, 1, 3, 1, 1, 1, 12, 4, 2, 36, 1]);
    }
    {
      var flx = FlxBuilder()
        ..startMap()
        ..addKey('a')
        ..addInt(12)
        ..addKey('')
        ..addInt(45)
        ..end()
      ;
      expect(flx.finish(), [97, 0, 0, 2, 2, 5, 2, 1, 2, 45, 12, 4, 4, 4, 36, 1]);
    }
    {
      var flx = FlxBuilder()
        ..startVector()
          ..startMap()
            ..addKey('something')
            ..addInt(12)
          ..end()
          ..startMap()
            ..addKey('something')
            ..addInt(45)
          ..end()
        ..end()
      ;
      expect(flx.finish(), [115, 111, 109, 101, 116, 104, 105, 110, 103, 0,
        1, 11, 1, 1, 1, 12, 4, 6, 1, 1, 45, 4, 2, 8, 4, 36, 36, 4, 40, 1]);
    }
  });

  test('build blob', ()
  {
    {
      var flx = FlxBuilder()
        ..addBlob(Uint8List.fromList([1, 2, 3]).buffer)
      ;
      expect(flx.finish(), [3, 1, 2, 3, 3, 100, 1]);
    }
  });

  test('build from object', (){
    expect(FlxBuilder.buildFromObject(Uint8List.fromList([1, 2, 3]).buffer).asUint8List(), [3, 1, 2, 3, 3, 100, 1]);
    expect(FlxBuilder.buildFromObject(null).asUint8List(), [0, 0, 1]);
    expect(FlxBuilder.buildFromObject(true).asUint8List(), [1, 104, 1]);
    expect(FlxBuilder.buildFromObject(false).asUint8List(), [0, 104, 1]);
    expect(FlxBuilder.buildFromObject(25).asUint8List(), [25, 4, 1]);
    expect(FlxBuilder.buildFromObject(-250).asUint8List(), [6, 255, 5, 2]);
    expect(FlxBuilder.buildFromObject(-2.50).asUint8List(), [0, 0, 32, 192, 14, 4]);
    expect(FlxBuilder.buildFromObject('Maxim').asUint8List(), [5, 77, 97, 120, 105, 109, 0, 6, 20, 1]);
    expect(FlxBuilder.buildFromObject([1, 3.3, 'max', true, null, false]).asUint8List(), [
      3, 109, 97, 120, 0, 0, 0, 0,
      6, 0, 0, 0, 0, 0, 0, 0,
      1, 0, 0, 0, 0, 0, 0, 0,
      102, 102, 102, 102, 102, 102, 10, 64,
      31, 0, 0, 0, 0, 0, 0, 0,
      1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      4, 15, 20, 104, 0, 104, 54, 43, 1
    ]);
    expect(FlxBuilder.buildFromObject([{'something':12}, {'something': 45}]).asUint8List(), [
      115, 111, 109, 101, 116, 104, 105, 110, 103, 0,
      1, 11, 1, 1, 1, 12, 4, 6, 1, 1, 45, 4, 2, 8, 4, 36, 36, 4, 40, 1
    ]);
  });

  test('add double indirectly', (){
    var flx = FlxBuilder()
      ..addDoubleIndirectly(0.1)
    ;
    expect(flx.finish(), [154, 153, 153, 153, 153, 153, 185, 63, 8, 35, 1]);
  });

  test('add double indirectly to vector with cache', (){
    var flx = FlxBuilder()
      ..startVector()
      ..addDoubleIndirectly(0.1, true)
      ..addDoubleIndirectly(0.1, true)
      ..addDoubleIndirectly(0.1, true)
      ..addDoubleIndirectly(0.1, true)
      ..end()
    ;
    expect(flx.finish(), [154, 153, 153, 153, 153, 153, 185, 63,
      4, 9, 10, 11, 12, 35, 35, 35, 35, 8, 40, 1]);
  });

  test('add int indirectly', (){
    var flx = FlxBuilder()
      ..addIntIndirectly(2345234523452345)
    ;
    expect(flx.finish(), [185, 115, 175, 118, 250, 84, 8, 0, 8, 27, 1]);
  });

  test('add int indirectly to vector with cache', (){
    var flx = FlxBuilder()
      ..startVector()
      ..addIntIndirectly(2345234523452345, true)
      ..addIntIndirectly(2345234523452345, true)
      ..addIntIndirectly(2345234523452345, true)
      ..addIntIndirectly(2345234523452345, true)
      ..end()
    ;
    expect(flx.finish(), [185, 115, 175, 118, 250, 84, 8, 0,
      4, 9, 10, 11, 12, 27, 27, 27, 27, 8, 40, 1]);
  });

  test('snapshot', (){
    var flx = FlxBuilder();
    flx.startVector();
    flx.addInt(12);
    expect(flx.snapshot().asUint8List(), [1, 12, 1, 44, 1]);
    flx.addInt(24);
    expect(flx.snapshot().asUint8List(), [12, 24, 2, 64, 1]);
    flx.addInt(45);
    expect(flx.snapshot().asUint8List(), [12, 24, 45, 3, 76, 1]);
  });
}

