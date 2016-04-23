// automatically generated, do not modify

module MyGame.Example.TestSimpleTableWithEnum;

import flatbuffers;

import MyGame.Example.Color;

struct TestSimpleTableWithEnum {
  mixin Table!TestSimpleTableWithEnum;

  static TestSimpleTableWithEnum getRootAsTestSimpleTableWithEnum(ByteBuffer _bb) {  return TestSimpleTableWithEnum.init_(_bb.get!int(_bb.position()) + _bb.position(), _bb); }
  @property   ubyte color() { int o = __offset(4); return o != 0 ? _buffer.get!ubyte(o + _pos) : 2; }

  static int createTestSimpleTableWithEnum(FlatBufferBuilder builder,
      ubyte color) {
    builder.startObject(1);
    TestSimpleTableWithEnum.addColor(builder, color);
    return TestSimpleTableWithEnum.endTestSimpleTableWithEnum(builder);
  }

  static void startTestSimpleTableWithEnum(FlatBufferBuilder builder) { builder.startObject(1); }
  static void addColor(FlatBufferBuilder builder, ubyte color) { builder.addUbyte(0, color, 2); }
  static int endTestSimpleTableWithEnum(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
}

