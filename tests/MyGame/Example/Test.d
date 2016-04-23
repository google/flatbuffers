// automatically generated, do not modify

module MyGame.Example.Test;

import flatbuffers;

struct Test {
  mixin Struct!Test;

  @property   short a() { return _buffer.get!short(_pos + 0); }
  @property   byte b() { return _buffer.get!byte(_pos + 2); }

  static int createTest(FlatBufferBuilder builder, short a, byte b) {
    builder.prep(2, 4);
    builder.pad(1);
    builder.put!byte(b);
    builder.put!short(a);
    return builder.offset();
  }
}

