// <auto-generated>
// automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>
package MyGame.Example;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

@SuppressWarnings("unused")
public final class Ability extends Struct {
  public void __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; }
  public Ability __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public long id() { return (long)bb.getInt(bb_pos + 0) & 0xFFFFFFFFL; }
  public void mutateId(long id) { bb.putInt(bb_pos + 0, (int)id); }
  public long distance() { return (long)bb.getInt(bb_pos + 4) & 0xFFFFFFFFL; }
  public void mutateDistance(long distance) { bb.putInt(bb_pos + 4, (int)distance); }

  public static int createAbility(FlatBufferBuilder builder, long id, long distance) {
    builder.prep(4, 8);
    builder.putInt((int)distance);
    builder.putInt((int)id);
    return builder.offset();
  }
}

