// automatically generated, do not modify

module MyGame.Example.Test;

import flatbuffers;

struct Test {
  mixin Struct!Test;

  @property   short a() { return _buffer.get!short(_pos + 0); }
  @property   ubyte b() { return _buffer.get!ubyte(_pos + 2); }

  static int createTest(FlatBufferBuilder builder, short a, ubyte b) {
    builder.prep(2, 4);
    builder.pad(1);
    builder.put!ubyte(b);
    builder.put!short(a);
    return builder.offset();
  }
}

