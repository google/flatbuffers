// automatically generated by the FlatBuffers compiler, do not modify

import * as flatbuffers from 'flatbuffers';



/**
 * @constructor
 */
export class TypeAliases {
  /**
   * @type flatbuffers.ByteBuffer
   */
  bb: flatbuffers.ByteBuffer|null = null;

  /**
   * @type number
   */
  bb_pos = 0;
/**
 * @param number i
 * @param flatbuffers.ByteBuffer bb
 * @returns TypeAliases
 */
__init(i:number, bb:flatbuffers.ByteBuffer):TypeAliases {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

/**
 * @param flatbuffers.ByteBuffer bb
 * @param TypeAliases= obj
 * @returns TypeAliases
 */
static getRootAsTypeAliases(bb:flatbuffers.ByteBuffer, obj?:TypeAliases):TypeAliases {
  return (obj || new TypeAliases()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

/**
 * @param flatbuffers.ByteBuffer bb
 * @param TypeAliases= obj
 * @returns TypeAliases
 */
static getSizePrefixedRootAsTypeAliases(bb:flatbuffers.ByteBuffer, obj?:TypeAliases):TypeAliases {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new TypeAliases()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

/**
 * @returns number
 */
i8():number {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.readInt8(this.bb_pos + offset) : 0;
}

/**
 * @param number value
 * @returns boolean
 */
mutate_i8(value:number):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 4);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeInt8(this.bb_pos + offset, value);
  return true;
}

/**
 * @returns number
 */
u8():number {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? this.bb!.readUint8(this.bb_pos + offset) : 0;
}

/**
 * @param number value
 * @returns boolean
 */
mutate_u8(value:number):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 6);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeUint8(this.bb_pos + offset, value);
  return true;
}

/**
 * @returns number
 */
i16():number {
  const offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? this.bb!.readInt16(this.bb_pos + offset) : 0;
}

/**
 * @param number value
 * @returns boolean
 */
mutate_i16(value:number):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 8);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeInt16(this.bb_pos + offset, value);
  return true;
}

/**
 * @returns number
 */
u16():number {
  const offset = this.bb!.__offset(this.bb_pos, 10);
  return offset ? this.bb!.readUint16(this.bb_pos + offset) : 0;
}

/**
 * @param number value
 * @returns boolean
 */
mutate_u16(value:number):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 10);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeUint16(this.bb_pos + offset, value);
  return true;
}

/**
 * @returns number
 */
i32():number {
  const offset = this.bb!.__offset(this.bb_pos, 12);
  return offset ? this.bb!.readInt32(this.bb_pos + offset) : 0;
}

/**
 * @param number value
 * @returns boolean
 */
mutate_i32(value:number):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 12);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeInt32(this.bb_pos + offset, value);
  return true;
}

/**
 * @returns number
 */
u32():number {
  const offset = this.bb!.__offset(this.bb_pos, 14);
  return offset ? this.bb!.readUint32(this.bb_pos + offset) : 0;
}

/**
 * @param number value
 * @returns boolean
 */
mutate_u32(value:number):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 14);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeUint32(this.bb_pos + offset, value);
  return true;
}

/**
 * @returns flatbuffers.Long
 */
i64():flatbuffers.Long {
  const offset = this.bb!.__offset(this.bb_pos, 16);
  return offset ? this.bb!.readInt64(this.bb_pos + offset) : this.bb!.createLong(0, 0);
}

/**
 * @param flatbuffers.Long value
 * @returns boolean
 */
mutate_i64(value:flatbuffers.Long):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 16);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeInt64(this.bb_pos + offset, value);
  return true;
}

/**
 * @returns flatbuffers.Long
 */
u64():flatbuffers.Long {
  const offset = this.bb!.__offset(this.bb_pos, 18);
  return offset ? this.bb!.readUint64(this.bb_pos + offset) : this.bb!.createLong(0, 0);
}

/**
 * @param flatbuffers.Long value
 * @returns boolean
 */
mutate_u64(value:flatbuffers.Long):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 18);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeUint64(this.bb_pos + offset, value);
  return true;
}

/**
 * @returns number
 */
f32():number {
  const offset = this.bb!.__offset(this.bb_pos, 20);
  return offset ? this.bb!.readFloat32(this.bb_pos + offset) : 0.0;
}

/**
 * @param number value
 * @returns boolean
 */
mutate_f32(value:number):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 20);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeFloat32(this.bb_pos + offset, value);
  return true;
}

/**
 * @returns number
 */
f64():number {
  const offset = this.bb!.__offset(this.bb_pos, 22);
  return offset ? this.bb!.readFloat64(this.bb_pos + offset) : 0.0;
}

/**
 * @param number value
 * @returns boolean
 */
mutate_f64(value:number):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 22);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeFloat64(this.bb_pos + offset, value);
  return true;
}

/**
 * @param number index
 * @returns number
 */
v8(index: number):number|null {
  const offset = this.bb!.__offset(this.bb_pos, 24);
  return offset ? this.bb!.readInt8(this.bb!.__vector(this.bb_pos + offset) + index) : 0;
}

/**
 * @returns number
 */
v8Length():number {
  const offset = this.bb!.__offset(this.bb_pos, 24);
  return offset ? this.bb!.__vector_len(this.bb_pos + offset) : 0;
}

/**
 * @returns Int8Array
 */
v8Array():Int8Array|null {
  const offset = this.bb!.__offset(this.bb_pos, 24);
  return offset ? new Int8Array(this.bb!.bytes().buffer, this.bb!.bytes().byteOffset + this.bb!.__vector(this.bb_pos + offset), this.bb!.__vector_len(this.bb_pos + offset)) : null;
}

/**
 * @param number index
 * @returns number
 */
vf64(index: number):number|null {
  const offset = this.bb!.__offset(this.bb_pos, 26);
  return offset ? this.bb!.readFloat64(this.bb!.__vector(this.bb_pos + offset) + index * 8) : 0;
}

/**
 * @returns number
 */
vf64Length():number {
  const offset = this.bb!.__offset(this.bb_pos, 26);
  return offset ? this.bb!.__vector_len(this.bb_pos + offset) : 0;
}

/**
 * @returns Float64Array
 */
vf64Array():Float64Array|null {
  const offset = this.bb!.__offset(this.bb_pos, 26);
  return offset ? new Float64Array(this.bb!.bytes().buffer, this.bb!.bytes().byteOffset + this.bb!.__vector(this.bb_pos + offset), this.bb!.__vector_len(this.bb_pos + offset)) : null;
}

/**
 * @returns string
 */
static getFullyQualifiedName():string {
  return 'MyGame.Example.TypeAliases';
}

/**
 * @param flatbuffers.Builder builder
 */
static startTypeAliases(builder:flatbuffers.Builder) {
  builder.startObject(12);
}

/**
 * @param flatbuffers.Builder builder
 * @param number i8
 */
static addI8(builder:flatbuffers.Builder, i8:number) {
  builder.addFieldInt8(0, i8, 0);
}

/**
 * @param flatbuffers.Builder builder
 * @param number u8
 */
static addU8(builder:flatbuffers.Builder, u8:number) {
  builder.addFieldInt8(1, u8, 0);
}

/**
 * @param flatbuffers.Builder builder
 * @param number i16
 */
static addI16(builder:flatbuffers.Builder, i16:number) {
  builder.addFieldInt16(2, i16, 0);
}

/**
 * @param flatbuffers.Builder builder
 * @param number u16
 */
static addU16(builder:flatbuffers.Builder, u16:number) {
  builder.addFieldInt16(3, u16, 0);
}

/**
 * @param flatbuffers.Builder builder
 * @param number i32
 */
static addI32(builder:flatbuffers.Builder, i32:number) {
  builder.addFieldInt32(4, i32, 0);
}

/**
 * @param flatbuffers.Builder builder
 * @param number u32
 */
static addU32(builder:flatbuffers.Builder, u32:number) {
  builder.addFieldInt32(5, u32, 0);
}

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Long i64
 */
static addI64(builder:flatbuffers.Builder, i64:flatbuffers.Long) {
  builder.addFieldInt64(6, i64, builder.createLong(0, 0));
}

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Long u64
 */
static addU64(builder:flatbuffers.Builder, u64:flatbuffers.Long) {
  builder.addFieldInt64(7, u64, builder.createLong(0, 0));
}

/**
 * @param flatbuffers.Builder builder
 * @param number f32
 */
static addF32(builder:flatbuffers.Builder, f32:number) {
  builder.addFieldFloat32(8, f32, 0.0);
}

/**
 * @param flatbuffers.Builder builder
 * @param number f64
 */
static addF64(builder:flatbuffers.Builder, f64:number) {
  builder.addFieldFloat64(9, f64, 0.0);
}

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Offset v8Offset
 */
static addV8(builder:flatbuffers.Builder, v8Offset:flatbuffers.Offset) {
  builder.addFieldOffset(10, v8Offset, 0);
}

/**
 * @param flatbuffers.Builder builder
 * @param Array.<number> data
 * @returns flatbuffers.Offset
 */
static createV8Vector(builder:flatbuffers.Builder, data:number[]|Int8Array):flatbuffers.Offset;
/**
 * @deprecated This Uint8Array overload will be removed in the future.
 */
static createV8Vector(builder:flatbuffers.Builder, data:number[]|Uint8Array):flatbuffers.Offset;
static createV8Vector(builder:flatbuffers.Builder, data:number[]|Int8Array|Uint8Array):flatbuffers.Offset {
  builder.startVector(1, data.length, 1);
  for (let i = data.length - 1; i >= 0; i--) {
    builder.addInt8(data[i]);
  }
  return builder.endVector();
}

/**
 * @param flatbuffers.Builder builder
 * @param number numElems
 */
static startV8Vector(builder:flatbuffers.Builder, numElems:number) {
  builder.startVector(1, numElems, 1);
}

/**
 * @param flatbuffers.Builder builder
 * @param flatbuffers.Offset vf64Offset
 */
static addVf64(builder:flatbuffers.Builder, vf64Offset:flatbuffers.Offset) {
  builder.addFieldOffset(11, vf64Offset, 0);
}

/**
 * @param flatbuffers.Builder builder
 * @param Array.<number> data
 * @returns flatbuffers.Offset
 */
static createVf64Vector(builder:flatbuffers.Builder, data:number[]|Float64Array):flatbuffers.Offset;
/**
 * @deprecated This Uint8Array overload will be removed in the future.
 */
static createVf64Vector(builder:flatbuffers.Builder, data:number[]|Uint8Array):flatbuffers.Offset;
static createVf64Vector(builder:flatbuffers.Builder, data:number[]|Float64Array|Uint8Array):flatbuffers.Offset {
  builder.startVector(8, data.length, 8);
  for (let i = data.length - 1; i >= 0; i--) {
    builder.addFloat64(data[i]);
  }
  return builder.endVector();
}

/**
 * @param flatbuffers.Builder builder
 * @param number numElems
 */
static startVf64Vector(builder:flatbuffers.Builder, numElems:number) {
  builder.startVector(8, numElems, 8);
}

/**
 * @param flatbuffers.Builder builder
 * @returns flatbuffers.Offset
 */
static endTypeAliases(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createTypeAliases(builder:flatbuffers.Builder, i8:number, u8:number, i16:number, u16:number, i32:number, u32:number, i64:flatbuffers.Long, u64:flatbuffers.Long, f32:number, f64:number, v8Offset:flatbuffers.Offset, vf64Offset:flatbuffers.Offset):flatbuffers.Offset {
  TypeAliases.startTypeAliases(builder);
  TypeAliases.addI8(builder, i8);
  TypeAliases.addU8(builder, u8);
  TypeAliases.addI16(builder, i16);
  TypeAliases.addU16(builder, u16);
  TypeAliases.addI32(builder, i32);
  TypeAliases.addU32(builder, u32);
  TypeAliases.addI64(builder, i64);
  TypeAliases.addU64(builder, u64);
  TypeAliases.addF32(builder, f32);
  TypeAliases.addF64(builder, f64);
  TypeAliases.addV8(builder, v8Offset);
  TypeAliases.addVf64(builder, vf64Offset);
  return TypeAliases.endTypeAliases(builder);
}

serialize():Uint8Array {
  return this.bb!.bytes();
}

static deserialize(buffer: Uint8Array):TypeAliases {
  return TypeAliases.getRootAsTypeAliases(new flatbuffers.ByteBuffer(buffer))
}

/**
 * @returns TypeAliasesT
 */
unpack(): TypeAliasesT {
  return new TypeAliasesT(
    this.i8(),
    this.u8(),
    this.i16(),
    this.u16(),
    this.i32(),
    this.u32(),
    this.i64(),
    this.u64(),
    this.f32(),
    this.f64(),
    this.bb!.createScalarList(this.v8.bind(this), this.v8Length()),
    this.bb!.createScalarList(this.vf64.bind(this), this.vf64Length())
  );
}

/**
 * @param TypeAliasesT _o
 */
unpackTo(_o: TypeAliasesT): void {
  _o.i8 = this.i8();
  _o.u8 = this.u8();
  _o.i16 = this.i16();
  _o.u16 = this.u16();
  _o.i32 = this.i32();
  _o.u32 = this.u32();
  _o.i64 = this.i64();
  _o.u64 = this.u64();
  _o.f32 = this.f32();
  _o.f64 = this.f64();
  _o.v8 = this.bb!.createScalarList(this.v8.bind(this), this.v8Length());
  _o.vf64 = this.bb!.createScalarList(this.vf64.bind(this), this.vf64Length());
}
}

export class TypeAliasesT {
/**
 * @constructor
 * @param number i8
 * @param number u8
 * @param number i16
 * @param number u16
 * @param number i32
 * @param number u32
 * @param flatbuffers.Long i64
 * @param flatbuffers.Long u64
 * @param number f32
 * @param number f64
 * @param (number)[] v8
 * @param (number)[] vf64
 */
constructor(
  public i8: number = 0,
  public u8: number = 0,
  public i16: number = 0,
  public u16: number = 0,
  public i32: number = 0,
  public u32: number = 0,
  public i64: flatbuffers.Long = flatbuffers.createLong(0, 0),
  public u64: flatbuffers.Long = flatbuffers.createLong(0, 0),
  public f32: number = 0.0,
  public f64: number = 0.0,
  public v8: (number)[] = [],
  public vf64: (number)[] = []
){}

/**
 * @param flatbuffers.Builder builder
 * @returns flatbuffers.Offset
 */
pack(builder:flatbuffers.Builder): flatbuffers.Offset {
  const v8 = TypeAliases.createV8Vector(builder, this.v8);
  const vf64 = TypeAliases.createVf64Vector(builder, this.vf64);

  return TypeAliases.createTypeAliases(builder,
    this.i8,
    this.u8,
    this.i16,
    this.u16,
    this.i32,
    this.u32,
    this.i64,
    this.u64,
    this.f32,
    this.f64,
    v8,
    vf64
  );
}
}
