// automatically generated by the FlatBuffers compiler, do not modify

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
import com.google.flatbuffers.Table;
import com.google.flatbuffers.UnionVector;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

@SuppressWarnings("unused")
public final class HandFan extends Table {
  public static void ValidateVersion() { Constants.FLATBUFFERS_24_3_6(); }
  public static HandFan getRootAsHandFan(ByteBuffer _bb) { return getRootAsHandFan(_bb, new HandFan()); }
  public static HandFan getRootAsHandFan(ByteBuffer _bb, HandFan obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__assign(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __reset(_i, _bb); }
  public HandFan __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public int length() { int o = __offset(4); return o != 0 ? bb.getInt(o + bb_pos) : 0; }
  public boolean mutateLength(int length) { int o = __offset(4); if (o != 0) { bb.putInt(o + bb_pos, length); return true; } else { return false; } }

  public static int createHandFan(FlatBufferBuilder builder,
      int length) {
    builder.startTable(1);
    HandFan.addLength(builder, length);
    return HandFan.endHandFan(builder);
  }

  public static void startHandFan(FlatBufferBuilder builder) { builder.startTable(1); }
  public static void addLength(FlatBufferBuilder builder, int length) { builder.addInt(0, length, 0); }
  public static int endHandFan(FlatBufferBuilder builder) {
    int o = builder.endTable();
    return o;
  }

  public static final class Vector extends BaseVector {
    public Vector __assign(int _vector, int _element_size, ByteBuffer _bb) { __reset(_vector, _element_size, _bb); return this; }

    public HandFan get(int j) { return get(new HandFan(), j); }
    public HandFan get(HandFan obj, int j) {  return obj.__assign(__indirect(__element(j), bb), bb); }
  }
  public HandFanT unpack() {
    HandFanT _o = new HandFanT();
    unpackTo(_o);
    return _o;
  }
  public void unpackTo(HandFanT _o) {
    int _oLength = length();
    _o.setLength(_oLength);
  }
  public static int pack(FlatBufferBuilder builder, HandFanT _o) {
    if (_o == null) return 0;
    return createHandFan(
      builder,
      _o.getLength());
  }
}

