// automatically generated by the FlatBuffers compiler, do not modify

package MyGame.Example;


@SuppressWarnings("unused")
public final class Vec3 extends Struct {
  public void __init(int _i, ByteBuffer _bb) { __reset(_i, _bb); }
  public Vec3 __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public float x() { return bb.getFloat(bb_pos + 0); }
  public void mutateX(float x) { bb.putFloat(bb_pos + 0, x); }
  public float y() { return bb.getFloat(bb_pos + 4); }
  public void mutateY(float y) { bb.putFloat(bb_pos + 4, y); }
  public float z() { return bb.getFloat(bb_pos + 8); }
  public void mutateZ(float z) { bb.putFloat(bb_pos + 8, z); }
  public double test1() { return bb.getDouble(bb_pos + 16); }
  public void mutateTest1(double test1) { bb.putDouble(bb_pos + 16, test1); }
  public int test2() { return bb.get(bb_pos + 24) & 0xFF; }
  public void mutateTest2(int test2) { bb.put(bb_pos + 24, (byte) test2); }
  public MyGame.Example.Test test3() { return test3(new MyGame.Example.Test()); }
  public MyGame.Example.Test test3(MyGame.Example.Test obj) { return obj.__assign(bb_pos + 26, bb); }

  public static int createVec3(FlatBufferBuilder builder, float x, float y, float z, double test1, int test2, short test3_a, byte test3_b) {
    builder.prep(8, 32);
    builder.pad(2);
    builder.prep(2, 4);
    builder.pad(1);
    builder.putByte(test3_b);
    builder.putShort(test3_a);
    builder.pad(1);
    builder.putByte((byte) test2);
    builder.putDouble(test1);
    builder.pad(4);
    builder.putFloat(z);
    builder.putFloat(y);
    builder.putFloat(x);
    return builder.offset();
  }

  public static final class Vector extends BaseVector {
    public Vector __assign(int _vector, int _element_size, ByteBuffer _bb) { __reset(_vector, _element_size, _bb); return this; }

    public Vec3 get(int j) { return get(new Vec3(), j); }
    public Vec3 get(Vec3 obj, int j) {  return obj.__assign(__element(j), bb); }
  }
  public Vec3T unpack() {
    Vec3T _o = new Vec3T();
    unpackTo(_o);
    return _o;
  }
  public void unpackTo(Vec3T _o) {
    float _oX = x();
    _o.setX(_oX);
    float _oY = y();
    _o.setY(_oY);
    float _oZ = z();
    _o.setZ(_oZ);
    double _oTest1 = test1();
    _o.setTest1(_oTest1);
    int _oTest2 = test2();
    _o.setTest2(_oTest2);
    test3().unpackTo(_o.getTest3());
  }
  public static int pack(FlatBufferBuilder builder, Vec3T _o) {
    if (_o == null) return 0;
    short _test3_a = _o.getTest3().getA();
    byte _test3_b = _o.getTest3().getB();
    return createVec3(
      builder,
      _o.getX(),
      _o.getY(),
      _o.getZ(),
      _o.getTest1(),
      _o.getTest2(),
      _test3_a,
      _test3_b);
  }
}

