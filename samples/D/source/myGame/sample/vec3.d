// automatically generated, do not modify

module myGame.sample.vec3;

import google.flatbuffers;

struct Vec3 {
  mixin Struct!Vec3;

  @property   float x() { return _buffer.get!float(_pos + 0); }
  @property   float y() { return _buffer.get!float(_pos + 4); }
  @property   float z() { return _buffer.get!float(_pos + 8); }

  static int createVec3(FlatBufferBuilder builder, float x, float y, float z) {
    builder.prep(4, 12);
    builder.put!float(z);
    builder.put!float(y);
    builder.put!float(x);
    return builder.offset();
  }
}

