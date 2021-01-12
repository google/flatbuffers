// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace optional_scalars
{

using global::System;
using global::System.Collections.Generic;
using global::FlatBuffers;

public struct ScalarStuff : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static void ValidateVersion() { FlatBufferConstants.FLATBUFFERS_1_12_0(); }
  public static ScalarStuff GetRootAsScalarStuff(ByteBuffer _bb) { return GetRootAsScalarStuff(_bb, new ScalarStuff()); }
  public static ScalarStuff GetRootAsScalarStuff(ByteBuffer _bb, ScalarStuff obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public static bool ScalarStuffBufferHasIdentifier(ByteBuffer _bb) { return Table.__has_identifier(_bb, "NULL"); }
  public void __init(int _i, ByteBuffer _bb) { __p = new Table(_i, _bb); }
  public ScalarStuff __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public sbyte JustI8 { get { int o = __p.__offset(4); return o != 0 ? __p.bb.GetSbyte(o + __p.bb_pos) : (sbyte)0; } }
  public sbyte? MaybeI8 { get { int o = __p.__offset(6); return o != 0 ? __p.bb.GetSbyte(o + __p.bb_pos) : (sbyte?)null; } }
  public sbyte DefaultI8 { get { int o = __p.__offset(8); return o != 0 ? __p.bb.GetSbyte(o + __p.bb_pos) : (sbyte)42; } }
  public byte JustU8 { get { int o = __p.__offset(10); return o != 0 ? __p.bb.Get(o + __p.bb_pos) : (byte)0; } }
  public byte? MaybeU8 { get { int o = __p.__offset(12); return o != 0 ? __p.bb.Get(o + __p.bb_pos) : (byte?)null; } }
  public byte DefaultU8 { get { int o = __p.__offset(14); return o != 0 ? __p.bb.Get(o + __p.bb_pos) : (byte)42; } }
  public short JustI16 { get { int o = __p.__offset(16); return o != 0 ? __p.bb.GetShort(o + __p.bb_pos) : (short)0; } }
  public short? MaybeI16 { get { int o = __p.__offset(18); return o != 0 ? __p.bb.GetShort(o + __p.bb_pos) : (short?)null; } }
  public short DefaultI16 { get { int o = __p.__offset(20); return o != 0 ? __p.bb.GetShort(o + __p.bb_pos) : (short)42; } }
  public ushort JustU16 { get { int o = __p.__offset(22); return o != 0 ? __p.bb.GetUshort(o + __p.bb_pos) : (ushort)0; } }
  public ushort? MaybeU16 { get { int o = __p.__offset(24); return o != 0 ? __p.bb.GetUshort(o + __p.bb_pos) : (ushort?)null; } }
  public ushort DefaultU16 { get { int o = __p.__offset(26); return o != 0 ? __p.bb.GetUshort(o + __p.bb_pos) : (ushort)42; } }
  public int JustI32 { get { int o = __p.__offset(28); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)0; } }
  public int? MaybeI32 { get { int o = __p.__offset(30); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int?)null; } }
  public int DefaultI32 { get { int o = __p.__offset(32); return o != 0 ? __p.bb.GetInt(o + __p.bb_pos) : (int)42; } }
  public uint JustU32 { get { int o = __p.__offset(34); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)0; } }
  public uint? MaybeU32 { get { int o = __p.__offset(36); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint?)null; } }
  public uint DefaultU32 { get { int o = __p.__offset(38); return o != 0 ? __p.bb.GetUint(o + __p.bb_pos) : (uint)42; } }
  public long JustI64 { get { int o = __p.__offset(40); return o != 0 ? __p.bb.GetLong(o + __p.bb_pos) : (long)0; } }
  public long? MaybeI64 { get { int o = __p.__offset(42); return o != 0 ? __p.bb.GetLong(o + __p.bb_pos) : (long?)null; } }
  public long DefaultI64 { get { int o = __p.__offset(44); return o != 0 ? __p.bb.GetLong(o + __p.bb_pos) : (long)42; } }
  public ulong JustU64 { get { int o = __p.__offset(46); return o != 0 ? __p.bb.GetUlong(o + __p.bb_pos) : (ulong)0; } }
  public ulong? MaybeU64 { get { int o = __p.__offset(48); return o != 0 ? __p.bb.GetUlong(o + __p.bb_pos) : (ulong?)null; } }
  public ulong DefaultU64 { get { int o = __p.__offset(50); return o != 0 ? __p.bb.GetUlong(o + __p.bb_pos) : (ulong)42; } }
  public float JustF32 { get { int o = __p.__offset(52); return o != 0 ? __p.bb.GetFloat(o + __p.bb_pos) : (float)0.0f; } }
  public float? MaybeF32 { get { int o = __p.__offset(54); return o != 0 ? __p.bb.GetFloat(o + __p.bb_pos) : (float?)null; } }
  public float DefaultF32 { get { int o = __p.__offset(56); return o != 0 ? __p.bb.GetFloat(o + __p.bb_pos) : (float)42.0f; } }
  public double JustF64 { get { int o = __p.__offset(58); return o != 0 ? __p.bb.GetDouble(o + __p.bb_pos) : (double)0.0; } }
  public double? MaybeF64 { get { int o = __p.__offset(60); return o != 0 ? __p.bb.GetDouble(o + __p.bb_pos) : (double?)null; } }
  public double DefaultF64 { get { int o = __p.__offset(62); return o != 0 ? __p.bb.GetDouble(o + __p.bb_pos) : (double)42.0; } }
  public bool JustBool { get { int o = __p.__offset(64); return o != 0 ? 0!=__p.bb.Get(o + __p.bb_pos) : (bool)false; } }
  public bool? MaybeBool { get { int o = __p.__offset(66); return o != 0 ? 0!=__p.bb.Get(o + __p.bb_pos) : (bool?)null; } }
  public bool DefaultBool { get { int o = __p.__offset(68); return o != 0 ? 0!=__p.bb.Get(o + __p.bb_pos) : (bool)true; } }
  public optional_scalars.OptionalByte JustEnum { get { int o = __p.__offset(70); return o != 0 ? (optional_scalars.OptionalByte)__p.bb.GetSbyte(o + __p.bb_pos) : optional_scalars.OptionalByte.None; } }
  public optional_scalars.OptionalByte? MaybeEnum { get { int o = __p.__offset(72); return o != 0 ? (optional_scalars.OptionalByte)__p.bb.GetSbyte(o + __p.bb_pos) : (optional_scalars.OptionalByte?)null; } }
  public optional_scalars.OptionalByte DefaultEnum { get { int o = __p.__offset(74); return o != 0 ? (optional_scalars.OptionalByte)__p.bb.GetSbyte(o + __p.bb_pos) : optional_scalars.OptionalByte.One; } }

  public static Offset<optional_scalars.ScalarStuff> CreateScalarStuff(FlatBufferBuilder builder,
      sbyte just_i8 = 0,
      sbyte? maybe_i8 = null,
      sbyte default_i8 = 42,
      byte just_u8 = 0,
      byte? maybe_u8 = null,
      byte default_u8 = 42,
      short just_i16 = 0,
      short? maybe_i16 = null,
      short default_i16 = 42,
      ushort just_u16 = 0,
      ushort? maybe_u16 = null,
      ushort default_u16 = 42,
      int just_i32 = 0,
      int? maybe_i32 = null,
      int default_i32 = 42,
      uint just_u32 = 0,
      uint? maybe_u32 = null,
      uint default_u32 = 42,
      long just_i64 = 0,
      long? maybe_i64 = null,
      long default_i64 = 42,
      ulong just_u64 = 0,
      ulong? maybe_u64 = null,
      ulong default_u64 = 42,
      float just_f32 = 0.0f,
      float? maybe_f32 = null,
      float default_f32 = 42.0f,
      double just_f64 = 0.0,
      double? maybe_f64 = null,
      double default_f64 = 42.0,
      bool just_bool = false,
      bool? maybe_bool = null,
      bool default_bool = true,
      optional_scalars.OptionalByte just_enum = optional_scalars.OptionalByte.None,
      optional_scalars.OptionalByte? maybe_enum = null,
      optional_scalars.OptionalByte default_enum = optional_scalars.OptionalByte.One) {
    builder.StartTable(36);
    ScalarStuff.AddDefaultF64(builder, default_f64);
    ScalarStuff.AddMaybeF64(builder, maybe_f64);
    ScalarStuff.AddJustF64(builder, just_f64);
    ScalarStuff.AddDefaultU64(builder, default_u64);
    ScalarStuff.AddMaybeU64(builder, maybe_u64);
    ScalarStuff.AddJustU64(builder, just_u64);
    ScalarStuff.AddDefaultI64(builder, default_i64);
    ScalarStuff.AddMaybeI64(builder, maybe_i64);
    ScalarStuff.AddJustI64(builder, just_i64);
    ScalarStuff.AddDefaultF32(builder, default_f32);
    ScalarStuff.AddMaybeF32(builder, maybe_f32);
    ScalarStuff.AddJustF32(builder, just_f32);
    ScalarStuff.AddDefaultU32(builder, default_u32);
    ScalarStuff.AddMaybeU32(builder, maybe_u32);
    ScalarStuff.AddJustU32(builder, just_u32);
    ScalarStuff.AddDefaultI32(builder, default_i32);
    ScalarStuff.AddMaybeI32(builder, maybe_i32);
    ScalarStuff.AddJustI32(builder, just_i32);
    ScalarStuff.AddDefaultU16(builder, default_u16);
    ScalarStuff.AddMaybeU16(builder, maybe_u16);
    ScalarStuff.AddJustU16(builder, just_u16);
    ScalarStuff.AddDefaultI16(builder, default_i16);
    ScalarStuff.AddMaybeI16(builder, maybe_i16);
    ScalarStuff.AddJustI16(builder, just_i16);
    ScalarStuff.AddDefaultEnum(builder, default_enum);
    ScalarStuff.AddMaybeEnum(builder, maybe_enum);
    ScalarStuff.AddJustEnum(builder, just_enum);
    ScalarStuff.AddDefaultBool(builder, default_bool);
    ScalarStuff.AddMaybeBool(builder, maybe_bool);
    ScalarStuff.AddJustBool(builder, just_bool);
    ScalarStuff.AddDefaultU8(builder, default_u8);
    ScalarStuff.AddMaybeU8(builder, maybe_u8);
    ScalarStuff.AddJustU8(builder, just_u8);
    ScalarStuff.AddDefaultI8(builder, default_i8);
    ScalarStuff.AddMaybeI8(builder, maybe_i8);
    ScalarStuff.AddJustI8(builder, just_i8);
    return ScalarStuff.EndScalarStuff(builder);
  }

  public static void StartScalarStuff(FlatBufferBuilder builder) { builder.StartTable(36); }
  public static void AddJustI8(FlatBufferBuilder builder, sbyte justI8) { builder.AddSbyte(0, justI8, 0); }
  public static void AddMaybeI8(FlatBufferBuilder builder, sbyte? maybeI8) { builder.AddSbyte(1, maybeI8); }
  public static void AddDefaultI8(FlatBufferBuilder builder, sbyte defaultI8) { builder.AddSbyte(2, defaultI8, 42); }
  public static void AddJustU8(FlatBufferBuilder builder, byte justU8) { builder.AddByte(3, justU8, 0); }
  public static void AddMaybeU8(FlatBufferBuilder builder, byte? maybeU8) { builder.AddByte(4, maybeU8); }
  public static void AddDefaultU8(FlatBufferBuilder builder, byte defaultU8) { builder.AddByte(5, defaultU8, 42); }
  public static void AddJustI16(FlatBufferBuilder builder, short justI16) { builder.AddShort(6, justI16, 0); }
  public static void AddMaybeI16(FlatBufferBuilder builder, short? maybeI16) { builder.AddShort(7, maybeI16); }
  public static void AddDefaultI16(FlatBufferBuilder builder, short defaultI16) { builder.AddShort(8, defaultI16, 42); }
  public static void AddJustU16(FlatBufferBuilder builder, ushort justU16) { builder.AddUshort(9, justU16, 0); }
  public static void AddMaybeU16(FlatBufferBuilder builder, ushort? maybeU16) { builder.AddUshort(10, maybeU16); }
  public static void AddDefaultU16(FlatBufferBuilder builder, ushort defaultU16) { builder.AddUshort(11, defaultU16, 42); }
  public static void AddJustI32(FlatBufferBuilder builder, int justI32) { builder.AddInt(12, justI32, 0); }
  public static void AddMaybeI32(FlatBufferBuilder builder, int? maybeI32) { builder.AddInt(13, maybeI32); }
  public static void AddDefaultI32(FlatBufferBuilder builder, int defaultI32) { builder.AddInt(14, defaultI32, 42); }
  public static void AddJustU32(FlatBufferBuilder builder, uint justU32) { builder.AddUint(15, justU32, 0); }
  public static void AddMaybeU32(FlatBufferBuilder builder, uint? maybeU32) { builder.AddUint(16, maybeU32); }
  public static void AddDefaultU32(FlatBufferBuilder builder, uint defaultU32) { builder.AddUint(17, defaultU32, 42); }
  public static void AddJustI64(FlatBufferBuilder builder, long justI64) { builder.AddLong(18, justI64, 0); }
  public static void AddMaybeI64(FlatBufferBuilder builder, long? maybeI64) { builder.AddLong(19, maybeI64); }
  public static void AddDefaultI64(FlatBufferBuilder builder, long defaultI64) { builder.AddLong(20, defaultI64, 42); }
  public static void AddJustU64(FlatBufferBuilder builder, ulong justU64) { builder.AddUlong(21, justU64, 0); }
  public static void AddMaybeU64(FlatBufferBuilder builder, ulong? maybeU64) { builder.AddUlong(22, maybeU64); }
  public static void AddDefaultU64(FlatBufferBuilder builder, ulong defaultU64) { builder.AddUlong(23, defaultU64, 42); }
  public static void AddJustF32(FlatBufferBuilder builder, float justF32) { builder.AddFloat(24, justF32, 0.0f); }
  public static void AddMaybeF32(FlatBufferBuilder builder, float? maybeF32) { builder.AddFloat(25, maybeF32); }
  public static void AddDefaultF32(FlatBufferBuilder builder, float defaultF32) { builder.AddFloat(26, defaultF32, 42.0f); }
  public static void AddJustF64(FlatBufferBuilder builder, double justF64) { builder.AddDouble(27, justF64, 0.0); }
  public static void AddMaybeF64(FlatBufferBuilder builder, double? maybeF64) { builder.AddDouble(28, maybeF64); }
  public static void AddDefaultF64(FlatBufferBuilder builder, double defaultF64) { builder.AddDouble(29, defaultF64, 42.0); }
  public static void AddJustBool(FlatBufferBuilder builder, bool justBool) { builder.AddBool(30, justBool, false); }
  public static void AddMaybeBool(FlatBufferBuilder builder, bool? maybeBool) { builder.AddBool(31, maybeBool); }
  public static void AddDefaultBool(FlatBufferBuilder builder, bool defaultBool) { builder.AddBool(32, defaultBool, true); }
  public static void AddJustEnum(FlatBufferBuilder builder, optional_scalars.OptionalByte justEnum) { builder.AddSbyte(33, (sbyte)justEnum, 0); }
  public static void AddMaybeEnum(FlatBufferBuilder builder, optional_scalars.OptionalByte? maybeEnum) { builder.AddSbyte(34, (sbyte)maybeEnum); }
  public static void AddDefaultEnum(FlatBufferBuilder builder, optional_scalars.OptionalByte defaultEnum) { builder.AddSbyte(35, (sbyte)defaultEnum, 1); }
  public static Offset<optional_scalars.ScalarStuff> EndScalarStuff(FlatBufferBuilder builder) {
    int o = builder.EndTable();
    return new Offset<optional_scalars.ScalarStuff>(o);
  }
  public static void FinishScalarStuffBuffer(FlatBufferBuilder builder, Offset<optional_scalars.ScalarStuff> offset) { builder.Finish(offset.Value, "NULL"); }
  public static void FinishSizePrefixedScalarStuffBuffer(FlatBufferBuilder builder, Offset<optional_scalars.ScalarStuff> offset) { builder.FinishSizePrefixed(offset.Value, "NULL"); }
  public ScalarStuffT UnPack() {
    var _o = new ScalarStuffT();
    this.UnPackTo(_o);
    return _o;
  }
  public void UnPackTo(ScalarStuffT _o) {
    _o.JustI8 = this.JustI8;
    _o.MaybeI8 = this.MaybeI8;
    _o.DefaultI8 = this.DefaultI8;
    _o.JustU8 = this.JustU8;
    _o.MaybeU8 = this.MaybeU8;
    _o.DefaultU8 = this.DefaultU8;
    _o.JustI16 = this.JustI16;
    _o.MaybeI16 = this.MaybeI16;
    _o.DefaultI16 = this.DefaultI16;
    _o.JustU16 = this.JustU16;
    _o.MaybeU16 = this.MaybeU16;
    _o.DefaultU16 = this.DefaultU16;
    _o.JustI32 = this.JustI32;
    _o.MaybeI32 = this.MaybeI32;
    _o.DefaultI32 = this.DefaultI32;
    _o.JustU32 = this.JustU32;
    _o.MaybeU32 = this.MaybeU32;
    _o.DefaultU32 = this.DefaultU32;
    _o.JustI64 = this.JustI64;
    _o.MaybeI64 = this.MaybeI64;
    _o.DefaultI64 = this.DefaultI64;
    _o.JustU64 = this.JustU64;
    _o.MaybeU64 = this.MaybeU64;
    _o.DefaultU64 = this.DefaultU64;
    _o.JustF32 = this.JustF32;
    _o.MaybeF32 = this.MaybeF32;
    _o.DefaultF32 = this.DefaultF32;
    _o.JustF64 = this.JustF64;
    _o.MaybeF64 = this.MaybeF64;
    _o.DefaultF64 = this.DefaultF64;
    _o.JustBool = this.JustBool;
    _o.MaybeBool = this.MaybeBool;
    _o.DefaultBool = this.DefaultBool;
    _o.JustEnum = this.JustEnum;
    _o.MaybeEnum = this.MaybeEnum;
    _o.DefaultEnum = this.DefaultEnum;
  }
  public static Offset<optional_scalars.ScalarStuff> Pack(FlatBufferBuilder builder, ScalarStuffT _o) {
    if (_o == null) return default(Offset<optional_scalars.ScalarStuff>);
    return CreateScalarStuff(
      builder,
      _o.JustI8,
      _o.MaybeI8,
      _o.DefaultI8,
      _o.JustU8,
      _o.MaybeU8,
      _o.DefaultU8,
      _o.JustI16,
      _o.MaybeI16,
      _o.DefaultI16,
      _o.JustU16,
      _o.MaybeU16,
      _o.DefaultU16,
      _o.JustI32,
      _o.MaybeI32,
      _o.DefaultI32,
      _o.JustU32,
      _o.MaybeU32,
      _o.DefaultU32,
      _o.JustI64,
      _o.MaybeI64,
      _o.DefaultI64,
      _o.JustU64,
      _o.MaybeU64,
      _o.DefaultU64,
      _o.JustF32,
      _o.MaybeF32,
      _o.DefaultF32,
      _o.JustF64,
      _o.MaybeF64,
      _o.DefaultF64,
      _o.JustBool,
      _o.MaybeBool,
      _o.DefaultBool,
      _o.JustEnum,
      _o.MaybeEnum,
      _o.DefaultEnum);
  }
};

public class ScalarStuffT
{
  public sbyte JustI8 { get; set; }
  public sbyte? MaybeI8 { get; set; }
  public sbyte DefaultI8 { get; set; }
  public byte JustU8 { get; set; }
  public byte? MaybeU8 { get; set; }
  public byte DefaultU8 { get; set; }
  public short JustI16 { get; set; }
  public short? MaybeI16 { get; set; }
  public short DefaultI16 { get; set; }
  public ushort JustU16 { get; set; }
  public ushort? MaybeU16 { get; set; }
  public ushort DefaultU16 { get; set; }
  public int JustI32 { get; set; }
  public int? MaybeI32 { get; set; }
  public int DefaultI32 { get; set; }
  public uint JustU32 { get; set; }
  public uint? MaybeU32 { get; set; }
  public uint DefaultU32 { get; set; }
  public long JustI64 { get; set; }
  public long? MaybeI64 { get; set; }
  public long DefaultI64 { get; set; }
  public ulong JustU64 { get; set; }
  public ulong? MaybeU64 { get; set; }
  public ulong DefaultU64 { get; set; }
  public float JustF32 { get; set; }
  public float? MaybeF32 { get; set; }
  public float DefaultF32 { get; set; }
  public double JustF64 { get; set; }
  public double? MaybeF64 { get; set; }
  public double DefaultF64 { get; set; }
  public bool JustBool { get; set; }
  public bool? MaybeBool { get; set; }
  public bool DefaultBool { get; set; }
  public optional_scalars.OptionalByte JustEnum { get; set; }
  public optional_scalars.OptionalByte? MaybeEnum { get; set; }
  public optional_scalars.OptionalByte DefaultEnum { get; set; }

  public ScalarStuffT() {
    this.JustI8 = 0;
    this.MaybeI8 = null;
    this.DefaultI8 = 42;
    this.JustU8 = 0;
    this.MaybeU8 = null;
    this.DefaultU8 = 42;
    this.JustI16 = 0;
    this.MaybeI16 = null;
    this.DefaultI16 = 42;
    this.JustU16 = 0;
    this.MaybeU16 = null;
    this.DefaultU16 = 42;
    this.JustI32 = 0;
    this.MaybeI32 = null;
    this.DefaultI32 = 42;
    this.JustU32 = 0;
    this.MaybeU32 = null;
    this.DefaultU32 = 42;
    this.JustI64 = 0;
    this.MaybeI64 = null;
    this.DefaultI64 = 42;
    this.JustU64 = 0;
    this.MaybeU64 = null;
    this.DefaultU64 = 42;
    this.JustF32 = 0.0f;
    this.MaybeF32 = null;
    this.DefaultF32 = 42.0f;
    this.JustF64 = 0.0;
    this.MaybeF64 = null;
    this.DefaultF64 = 42.0;
    this.JustBool = false;
    this.MaybeBool = null;
    this.DefaultBool = true;
    this.JustEnum = optional_scalars.OptionalByte.None;
    this.MaybeEnum = null;
    this.DefaultEnum = optional_scalars.OptionalByte.One;
  }
  public static ScalarStuffT DeserializeFromBinary(byte[] fbBuffer) {
    return ScalarStuff.GetRootAsScalarStuff(new ByteBuffer(fbBuffer)).UnPack();
  }
  public byte[] SerializeToBinary() {
    var fbb = new FlatBufferBuilder(0x10000);
    ScalarStuff.FinishScalarStuffBuffer(fbb, ScalarStuff.Pack(fbb, this));
    return fbb.DataBuffer.ToSizedArray();
  }
}


}
