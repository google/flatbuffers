// automatically generated, do not modify

module MyGame.Example.Vec3;

import flatbuffers;

import MyGame.Example.Color;
import MyGame.Example.Test;

struct Vec3 {
  mixin Struct!Vec3;

  @property   float x() { return _buffer.get!float(_pos + 0); }
  @property   float y() { return _buffer.get!float(_pos + 4); }
  @property   float z() { return _buffer.get!float(_pos + 8); }
  @property   double test1() { return _buffer.get!double(_pos + 16); }
  @property   byte test2() { return _buffer.get!byte(_pos + 24); }
  @property   Nullable!Test test3() { return Nullable!Test(Test.init_(_pos + 26, _buffer)); }

  static int createVec3(FlatBufferBuilder builder, float x, float y, float z, double test1, byte test2, short Test_a, byte Test_b) {
    builder.prep(16, 32);
    builder.pad(2);
    builder.prep(2, 4);
    builder.pad(1);
    builder.put!byte(Test_b);
    builder.put!short(Test_a);
    builder.pad(1);
    builder.put!byte(test2);
    builder.put!double(test1);
    builder.pad(4);
    builder.put!float(z);
    builder.put!float(y);
    builder.put!float(x);
    return builder.offset();
  }
}

