// automatically generated by the FlatBuffers compiler, do not modify

package union_underlying_type;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

@SuppressWarnings("unused")
public final class B extends Table {
  public static void ValidateVersion() { Constants.FLATBUFFERS_2_0_0(); }
  public static B getRootAsB(ByteBuffer _bb) { return getRootAsB(_bb, new B()); }
  public static B getRootAsB(ByteBuffer _bb, B obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__assign(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __reset(_i, _bb); }
  public B __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }


  public static void startB(FlatBufferBuilder builder) { builder.startTable(0); }
  public static int endB(FlatBufferBuilder builder) {
    int o = builder.endTable();
    return o;
  }

  public static final class Vector extends BaseVector {
    public Vector __assign(int _vector, int _element_size, ByteBuffer _bb) { __reset(_vector, _element_size, _bb); return this; }

    public B get(int j) { return get(new B(), j); }
    public B get(B obj, int j) {  return obj.__assign(__indirect(__element(j), bb), bb); }
  }
  public BT unpack() {
    BT _o = new BT();
    unpackTo(_o);
    return _o;
  }
  public void unpackTo(BT _o) {
  }
  public static int pack(FlatBufferBuilder builder, BT _o) {
    if (_o == null) return 0;
    startB(builder);
    return endB(builder);
  }
}

