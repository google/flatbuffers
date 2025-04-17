// automatically generated by the FlatBuffers compiler, do not modify

package MyGame;

import com.google.flatbuffers.BaseVector;
import com.google.flatbuffers.BooleanVector;
import com.google.flatbuffers.ByteVector;
import com.google.flatbuffers.Constants;
import com.google.flatbuffers.DoubleVector;
import com.google.flatbuffers.FlatBufferBuilder;
import com.google.flatbuffers.FloatVector;
import com.google.flatbuffers.IntVector;
import com.google.flatbuffers.LongVector;
import com.google.flatbuffers.ShortVector;
import com.google.flatbuffers.StringVector;
import com.google.flatbuffers.Struct;
import com.google.flatbuffers.UnionVector;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

@SuppressWarnings("unused")
public final class MonsterExtra extends com.google.flatbuffers.Table {
  public static void ValidateVersion() { Constants.FLATBUFFERS_25_2_10(); }
  public static MonsterExtra getRootAsMonsterExtra(ByteBuffer _bb) { return getRootAsMonsterExtra(_bb, new MonsterExtra()); }
  public static MonsterExtra getRootAsMonsterExtra(ByteBuffer _bb, MonsterExtra obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__assign(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public static boolean MonsterExtraBufferHasIdentifier(ByteBuffer _bb) { return __has_identifier(_bb, "MONE"); }
  public void __init(int _i, ByteBuffer _bb) { __reset(_i, _bb); }
  public MonsterExtra __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public double d0() { int o = __offset(4); return o != 0 ? bb.getDouble(o + bb_pos) : Double.NaN; }
  public boolean mutateD0(double d0) { int o = __offset(4); if (o != 0) { bb.putDouble(o + bb_pos, d0); return true; } else { return false; } }
  public double d1() { int o = __offset(6); return o != 0 ? bb.getDouble(o + bb_pos) : Double.NaN; }
  public boolean mutateD1(double d1) { int o = __offset(6); if (o != 0) { bb.putDouble(o + bb_pos, d1); return true; } else { return false; } }
  public double d2() { int o = __offset(8); return o != 0 ? bb.getDouble(o + bb_pos) : Double.POSITIVE_INFINITY; }
  public boolean mutateD2(double d2) { int o = __offset(8); if (o != 0) { bb.putDouble(o + bb_pos, d2); return true; } else { return false; } }
  public double d3() { int o = __offset(10); return o != 0 ? bb.getDouble(o + bb_pos) : Double.NEGATIVE_INFINITY; }
  public boolean mutateD3(double d3) { int o = __offset(10); if (o != 0) { bb.putDouble(o + bb_pos, d3); return true; } else { return false; } }
  public float f0() { int o = __offset(12); return o != 0 ? bb.getFloat(o + bb_pos) : Float.NaN; }
  public boolean mutateF0(float f0) { int o = __offset(12); if (o != 0) { bb.putFloat(o + bb_pos, f0); return true; } else { return false; } }
  public float f1() { int o = __offset(14); return o != 0 ? bb.getFloat(o + bb_pos) : Float.NaN; }
  public boolean mutateF1(float f1) { int o = __offset(14); if (o != 0) { bb.putFloat(o + bb_pos, f1); return true; } else { return false; } }
  public float f2() { int o = __offset(16); return o != 0 ? bb.getFloat(o + bb_pos) : Float.POSITIVE_INFINITY; }
  public boolean mutateF2(float f2) { int o = __offset(16); if (o != 0) { bb.putFloat(o + bb_pos, f2); return true; } else { return false; } }
  public float f3() { int o = __offset(18); return o != 0 ? bb.getFloat(o + bb_pos) : Float.NEGATIVE_INFINITY; }
  public boolean mutateF3(float f3) { int o = __offset(18); if (o != 0) { bb.putFloat(o + bb_pos, f3); return true; } else { return false; } }
  public double dvec(int j) { int o = __offset(20); return o != 0 ? bb.getDouble(__vector(o) + j * 8) : 0; }
  public int dvecLength() { int o = __offset(20); return o != 0 ? __vector_len(o) : 0; }
  public DoubleVector dvecVector() { return dvecVector(new DoubleVector()); }
  public DoubleVector dvecVector(DoubleVector obj) { int o = __offset(20); return o != 0 ? obj.__assign(__vector(o), bb) : null; }
  public ByteBuffer dvecAsByteBuffer() { return __vector_as_bytebuffer(20, 8); }
  public ByteBuffer dvecInByteBuffer(ByteBuffer _bb) { return __vector_in_bytebuffer(_bb, 20, 8); }
  public boolean mutateDvec(int j, double dvec) { int o = __offset(20); if (o != 0) { bb.putDouble(__vector(o) + j * 8, dvec); return true; } else { return false; } }
  public float fvec(int j) { int o = __offset(22); return o != 0 ? bb.getFloat(__vector(o) + j * 4) : 0; }
  public int fvecLength() { int o = __offset(22); return o != 0 ? __vector_len(o) : 0; }
  public FloatVector fvecVector() { return fvecVector(new FloatVector()); }
  public FloatVector fvecVector(FloatVector obj) { int o = __offset(22); return o != 0 ? obj.__assign(__vector(o), bb) : null; }
  public ByteBuffer fvecAsByteBuffer() { return __vector_as_bytebuffer(22, 4); }
  public ByteBuffer fvecInByteBuffer(ByteBuffer _bb) { return __vector_in_bytebuffer(_bb, 22, 4); }
  public boolean mutateFvec(int j, float fvec) { int o = __offset(22); if (o != 0) { bb.putFloat(__vector(o) + j * 4, fvec); return true; } else { return false; } }

  public static int createMonsterExtra(FlatBufferBuilder builder,
      double d0,
      double d1,
      double d2,
      double d3,
      float f0,
      float f1,
      float f2,
      float f3,
      int dvecOffset,
      int fvecOffset) {
    builder.startTable(11);
    MonsterExtra.addD3(builder, d3);
    MonsterExtra.addD2(builder, d2);
    MonsterExtra.addD1(builder, d1);
    MonsterExtra.addD0(builder, d0);
    MonsterExtra.addFvec(builder, fvecOffset);
    MonsterExtra.addDvec(builder, dvecOffset);
    MonsterExtra.addF3(builder, f3);
    MonsterExtra.addF2(builder, f2);
    MonsterExtra.addF1(builder, f1);
    MonsterExtra.addF0(builder, f0);
    return MonsterExtra.endMonsterExtra(builder);
  }

  public static void startMonsterExtra(FlatBufferBuilder builder) { builder.startTable(11); }
  public static void addD0(FlatBufferBuilder builder, double d0) { builder.addDouble(0, d0, Double.NaN); }
  public static void addD1(FlatBufferBuilder builder, double d1) { builder.addDouble(1, d1, Double.NaN); }
  public static void addD2(FlatBufferBuilder builder, double d2) { builder.addDouble(2, d2, Double.POSITIVE_INFINITY); }
  public static void addD3(FlatBufferBuilder builder, double d3) { builder.addDouble(3, d3, Double.NEGATIVE_INFINITY); }
  public static void addF0(FlatBufferBuilder builder, float f0) { builder.addFloat(4, f0, Float.NaN); }
  public static void addF1(FlatBufferBuilder builder, float f1) { builder.addFloat(5, f1, Float.NaN); }
  public static void addF2(FlatBufferBuilder builder, float f2) { builder.addFloat(6, f2, Float.POSITIVE_INFINITY); }
  public static void addF3(FlatBufferBuilder builder, float f3) { builder.addFloat(7, f3, Float.NEGATIVE_INFINITY); }
  public static void addDvec(FlatBufferBuilder builder, int dvecOffset) { builder.addOffset(8, dvecOffset, 0); }
  public static int createDvecVector(FlatBufferBuilder builder, double[] data) { builder.startVector(8, data.length, 8); for (int i = data.length - 1; i >= 0; i--) builder.addDouble(data[i]); return builder.endVector(); }
  public static void startDvecVector(FlatBufferBuilder builder, int numElems) { builder.startVector(8, numElems, 8); }
  public static void addFvec(FlatBufferBuilder builder, int fvecOffset) { builder.addOffset(9, fvecOffset, 0); }
  public static int createFvecVector(FlatBufferBuilder builder, float[] data) { builder.startVector(4, data.length, 4); for (int i = data.length - 1; i >= 0; i--) builder.addFloat(data[i]); return builder.endVector(); }
  public static void startFvecVector(FlatBufferBuilder builder, int numElems) { builder.startVector(4, numElems, 4); }
  public static int endMonsterExtra(FlatBufferBuilder builder) {
    int o = builder.endTable();
    return o;
  }
  public static void finishMonsterExtraBuffer(FlatBufferBuilder builder, int offset) { builder.finish(offset, "MONE"); }
  public static void finishSizePrefixedMonsterExtraBuffer(FlatBufferBuilder builder, int offset) { builder.finishSizePrefixed(offset, "MONE"); }

  public static final class _Vector extends BaseVector {
    public _Vector __assign(int _vector, int _element_size, ByteBuffer _bb) { __reset(_vector, _element_size, _bb); return this; }

    public MonsterExtra get(int j) { return get(new MonsterExtra(), j); }
    public MonsterExtra get(MonsterExtra obj, int j) {  return obj.__assign(__indirect(__element(j), bb), bb); }
  }
  public MonsterExtraT unpack() {
    MonsterExtraT _o = new MonsterExtraT();
    unpackTo(_o);
    return _o;
  }
  public void unpackTo(MonsterExtraT _o) {
    double _oD0 = d0();
    _o.setD0(_oD0);
    double _oD1 = d1();
    _o.setD1(_oD1);
    double _oD2 = d2();
    _o.setD2(_oD2);
    double _oD3 = d3();
    _o.setD3(_oD3);
    float _oF0 = f0();
    _o.setF0(_oF0);
    float _oF1 = f1();
    _o.setF1(_oF1);
    float _oF2 = f2();
    _o.setF2(_oF2);
    float _oF3 = f3();
    _o.setF3(_oF3);
    double[] _oDvec = new double[dvecLength()];
    for (int _j = 0; _j < dvecLength(); ++_j) {_oDvec[_j] = dvec(_j);}
    _o.setDvec(_oDvec);
    float[] _oFvec = new float[fvecLength()];
    for (int _j = 0; _j < fvecLength(); ++_j) {_oFvec[_j] = fvec(_j);}
    _o.setFvec(_oFvec);
  }
  public static int pack(FlatBufferBuilder builder, MonsterExtraT _o) {
    if (_o == null) return 0;
    int _dvec = 0;
    if (_o.getDvec() != null) {
      _dvec = createDvecVector(builder, _o.getDvec());
    }
    int _fvec = 0;
    if (_o.getFvec() != null) {
      _fvec = createFvecVector(builder, _o.getFvec());
    }
    return createMonsterExtra(
      builder,
      _o.getD0(),
      _o.getD1(),
      _o.getD2(),
      _o.getD3(),
      _o.getF0(),
      _o.getF1(),
      _o.getF2(),
      _o.getF3(),
      _dvec,
      _fvec);
  }
}

