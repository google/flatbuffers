// automatically generated by the FlatBuffers compiler, do not modify

package MyGame.Example;


@SuppressWarnings("unused")
public final class StructOfStructsOfStructs extends Struct {
  public void __init(int _i, ByteBuffer _bb) { __reset(_i, _bb); }
  public StructOfStructsOfStructs __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public MyGame.Example.StructOfStructs a() { return a(new MyGame.Example.StructOfStructs()); }
  public MyGame.Example.StructOfStructs a(MyGame.Example.StructOfStructs obj) { return obj.__assign(bb_pos + 0, bb); }

  public static int createStructOfStructsOfStructs(FlatBufferBuilder builder, long a_a_id, long a_a_distance, short a_b_a, byte a_b_b, long a_c_id, long a_c_distance) {
    builder.prep(4, 20);
    builder.prep(4, 20);
    builder.prep(4, 8);
    builder.putInt((int) a_c_distance);
    builder.putInt((int) a_c_id);
    builder.prep(2, 4);
    builder.pad(1);
    builder.putByte(a_b_b);
    builder.putShort(a_b_a);
    builder.prep(4, 8);
    builder.putInt((int) a_a_distance);
    builder.putInt((int) a_a_id);
    return builder.offset();
  }

  public static final class Vector extends BaseVector {
    public Vector __assign(int _vector, int _element_size, ByteBuffer _bb) { __reset(_vector, _element_size, _bb); return this; }

    public StructOfStructsOfStructs get(int j) { return get(new StructOfStructsOfStructs(), j); }
    public StructOfStructsOfStructs get(StructOfStructsOfStructs obj, int j) {  return obj.__assign(__element(j), bb); }
  }
  public StructOfStructsOfStructsT unpack() {
    StructOfStructsOfStructsT _o = new StructOfStructsOfStructsT();
    unpackTo(_o);
    return _o;
  }
  public void unpackTo(StructOfStructsOfStructsT _o) {
    a().unpackTo(_o.getA());
  }
  public static int pack(FlatBufferBuilder builder, StructOfStructsOfStructsT _o) {
    if (_o == null) return 0;
    int _a_a_id = (int) _o.getA().getA().getId();
    int _a_a_distance = (int) _o.getA().getA().getDistance();
    short _a_b_a = _o.getA().getB().getA();
    byte _a_b_b = _o.getA().getB().getB();
    int _a_c_id = (int) _o.getA().getC().getId();
    int _a_c_distance = (int) _o.getA().getC().getDistance();
    return createStructOfStructsOfStructs(
      builder,
      _a_a_id,
      _a_a_distance,
      _a_b_a,
      _a_b_b,
      _a_c_id,
      _a_c_distance);
  }
}

