"use strict";
var __create = Object.create;
var __defProp = Object.defineProperty;
var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
var __getOwnPropNames = Object.getOwnPropertyNames;
var __getProtoOf = Object.getPrototypeOf;
var __hasOwnProp = Object.prototype.hasOwnProperty;
var __export = (target, all) => {
  for (var name in all)
    __defProp(target, name, { get: all[name], enumerable: true });
};
var __copyProps = (to, from, except, desc) => {
  if (from && typeof from === "object" || typeof from === "function") {
    for (let key of __getOwnPropNames(from))
      if (!__hasOwnProp.call(to, key) && key !== except)
        __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
  }
  return to;
};
var __toESM = (mod, isNodeMode, target) => (target = mod != null ? __create(__getProtoOf(mod)) : {}, __copyProps(
  // If the importer is in node compatibility mode or this is not an ESM
  // file that has been converted to a CommonJS file using a Babel-
  // compatible transform (i.e. "__esModule" has not been set), then set
  // "default" to the CommonJS "module.exports" for node compatibility.
  isNodeMode || !mod || !mod.__esModule ? __defProp(target, "default", { value: mod, enumerable: true }) : target,
  mod
));
var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

// ts-undefined-for-optionals/optional_scalars.ts
var optional_scalars_exports2 = {};
__export(optional_scalars_exports2, {
  optional_scalars: () => optional_scalars_exports
});
module.exports = __toCommonJS(optional_scalars_exports2);

// ts-undefined-for-optionals/optional-scalars.ts
var optional_scalars_exports = {};
__export(optional_scalars_exports, {
  OptionalByte: () => OptionalByte,
  ScalarStuff: () => ScalarStuff,
  ScalarStuffT: () => ScalarStuffT
});

// ts-undefined-for-optionals/optional-scalars/optional-byte.ts
var OptionalByte = /* @__PURE__ */ ((OptionalByte2) => {
  OptionalByte2[OptionalByte2["None"] = 0] = "None";
  OptionalByte2[OptionalByte2["One"] = 1] = "One";
  OptionalByte2[OptionalByte2["Two"] = 2] = "Two";
  return OptionalByte2;
})(OptionalByte || {});

// ts-undefined-for-optionals/optional-scalars/scalar-stuff.ts
var flatbuffers = __toESM(require("flatbuffers"), 1);
var ScalarStuff = class _ScalarStuff {
  constructor() {
    this.bb = void 0;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsScalarStuff(bb, obj) {
    return (obj || new _ScalarStuff()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsScalarStuff(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
    return (obj || new _ScalarStuff()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static bufferHasIdentifier(bb) {
    return bb.__has_identifier("NULL");
  }
  justI8() {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.readInt8(this.bb_pos + offset) : 0;
  }
  maybeI8() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.readInt8(this.bb_pos + offset) : void 0;
  }
  defaultI8() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? this.bb.readInt8(this.bb_pos + offset) : 42;
  }
  justU8() {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? this.bb.readUint8(this.bb_pos + offset) : 0;
  }
  maybeU8() {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? this.bb.readUint8(this.bb_pos + offset) : void 0;
  }
  defaultU8() {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? this.bb.readUint8(this.bb_pos + offset) : 42;
  }
  justI16() {
    const offset = this.bb.__offset(this.bb_pos, 16);
    return offset ? this.bb.readInt16(this.bb_pos + offset) : 0;
  }
  maybeI16() {
    const offset = this.bb.__offset(this.bb_pos, 18);
    return offset ? this.bb.readInt16(this.bb_pos + offset) : void 0;
  }
  defaultI16() {
    const offset = this.bb.__offset(this.bb_pos, 20);
    return offset ? this.bb.readInt16(this.bb_pos + offset) : 42;
  }
  justU16() {
    const offset = this.bb.__offset(this.bb_pos, 22);
    return offset ? this.bb.readUint16(this.bb_pos + offset) : 0;
  }
  maybeU16() {
    const offset = this.bb.__offset(this.bb_pos, 24);
    return offset ? this.bb.readUint16(this.bb_pos + offset) : void 0;
  }
  defaultU16() {
    const offset = this.bb.__offset(this.bb_pos, 26);
    return offset ? this.bb.readUint16(this.bb_pos + offset) : 42;
  }
  justI32() {
    const offset = this.bb.__offset(this.bb_pos, 28);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : 0;
  }
  maybeI32() {
    const offset = this.bb.__offset(this.bb_pos, 30);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : void 0;
  }
  defaultI32() {
    const offset = this.bb.__offset(this.bb_pos, 32);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : 42;
  }
  justU32() {
    const offset = this.bb.__offset(this.bb_pos, 34);
    return offset ? this.bb.readUint32(this.bb_pos + offset) : 0;
  }
  maybeU32() {
    const offset = this.bb.__offset(this.bb_pos, 36);
    return offset ? this.bb.readUint32(this.bb_pos + offset) : void 0;
  }
  defaultU32() {
    const offset = this.bb.__offset(this.bb_pos, 38);
    return offset ? this.bb.readUint32(this.bb_pos + offset) : 42;
  }
  justI64() {
    const offset = this.bb.__offset(this.bb_pos, 40);
    return offset ? this.bb.readInt64(this.bb_pos + offset) : BigInt("0");
  }
  maybeI64() {
    const offset = this.bb.__offset(this.bb_pos, 42);
    return offset ? this.bb.readInt64(this.bb_pos + offset) : void 0;
  }
  defaultI64() {
    const offset = this.bb.__offset(this.bb_pos, 44);
    return offset ? this.bb.readInt64(this.bb_pos + offset) : BigInt("42");
  }
  justU64() {
    const offset = this.bb.__offset(this.bb_pos, 46);
    return offset ? this.bb.readUint64(this.bb_pos + offset) : BigInt("0");
  }
  maybeU64() {
    const offset = this.bb.__offset(this.bb_pos, 48);
    return offset ? this.bb.readUint64(this.bb_pos + offset) : void 0;
  }
  defaultU64() {
    const offset = this.bb.__offset(this.bb_pos, 50);
    return offset ? this.bb.readUint64(this.bb_pos + offset) : BigInt("42");
  }
  justF32() {
    const offset = this.bb.__offset(this.bb_pos, 52);
    return offset ? this.bb.readFloat32(this.bb_pos + offset) : 0;
  }
  maybeF32() {
    const offset = this.bb.__offset(this.bb_pos, 54);
    return offset ? this.bb.readFloat32(this.bb_pos + offset) : void 0;
  }
  defaultF32() {
    const offset = this.bb.__offset(this.bb_pos, 56);
    return offset ? this.bb.readFloat32(this.bb_pos + offset) : 42;
  }
  justF64() {
    const offset = this.bb.__offset(this.bb_pos, 58);
    return offset ? this.bb.readFloat64(this.bb_pos + offset) : 0;
  }
  maybeF64() {
    const offset = this.bb.__offset(this.bb_pos, 60);
    return offset ? this.bb.readFloat64(this.bb_pos + offset) : void 0;
  }
  defaultF64() {
    const offset = this.bb.__offset(this.bb_pos, 62);
    return offset ? this.bb.readFloat64(this.bb_pos + offset) : 42;
  }
  justBool() {
    const offset = this.bb.__offset(this.bb_pos, 64);
    return offset ? !!this.bb.readInt8(this.bb_pos + offset) : false;
  }
  maybeBool() {
    const offset = this.bb.__offset(this.bb_pos, 66);
    return offset ? !!this.bb.readInt8(this.bb_pos + offset) : void 0;
  }
  defaultBool() {
    const offset = this.bb.__offset(this.bb_pos, 68);
    return offset ? !!this.bb.readInt8(this.bb_pos + offset) : true;
  }
  justEnum() {
    const offset = this.bb.__offset(this.bb_pos, 70);
    return offset ? this.bb.readInt8(this.bb_pos + offset) : 0 /* None */;
  }
  maybeEnum() {
    const offset = this.bb.__offset(this.bb_pos, 72);
    return offset ? this.bb.readInt8(this.bb_pos + offset) : void 0;
  }
  defaultEnum() {
    const offset = this.bb.__offset(this.bb_pos, 74);
    return offset ? this.bb.readInt8(this.bb_pos + offset) : 1 /* One */;
  }
  static startScalarStuff(builder) {
    builder.startObject(36);
  }
  static addJustI8(builder, justI8) {
    builder.addFieldInt8(0, justI8, 0);
  }
  static addMaybeI8(builder, maybeI8) {
    builder.addFieldInt8(1, maybeI8, void 0);
  }
  static addDefaultI8(builder, defaultI8) {
    builder.addFieldInt8(2, defaultI8, 42);
  }
  static addJustU8(builder, justU8) {
    builder.addFieldInt8(3, justU8, 0);
  }
  static addMaybeU8(builder, maybeU8) {
    builder.addFieldInt8(4, maybeU8, void 0);
  }
  static addDefaultU8(builder, defaultU8) {
    builder.addFieldInt8(5, defaultU8, 42);
  }
  static addJustI16(builder, justI16) {
    builder.addFieldInt16(6, justI16, 0);
  }
  static addMaybeI16(builder, maybeI16) {
    builder.addFieldInt16(7, maybeI16, void 0);
  }
  static addDefaultI16(builder, defaultI16) {
    builder.addFieldInt16(8, defaultI16, 42);
  }
  static addJustU16(builder, justU16) {
    builder.addFieldInt16(9, justU16, 0);
  }
  static addMaybeU16(builder, maybeU16) {
    builder.addFieldInt16(10, maybeU16, void 0);
  }
  static addDefaultU16(builder, defaultU16) {
    builder.addFieldInt16(11, defaultU16, 42);
  }
  static addJustI32(builder, justI32) {
    builder.addFieldInt32(12, justI32, 0);
  }
  static addMaybeI32(builder, maybeI32) {
    builder.addFieldInt32(13, maybeI32, void 0);
  }
  static addDefaultI32(builder, defaultI32) {
    builder.addFieldInt32(14, defaultI32, 42);
  }
  static addJustU32(builder, justU32) {
    builder.addFieldInt32(15, justU32, 0);
  }
  static addMaybeU32(builder, maybeU32) {
    builder.addFieldInt32(16, maybeU32, void 0);
  }
  static addDefaultU32(builder, defaultU32) {
    builder.addFieldInt32(17, defaultU32, 42);
  }
  static addJustI64(builder, justI64) {
    builder.addFieldInt64(18, justI64, BigInt("0"));
  }
  static addMaybeI64(builder, maybeI64) {
    builder.addFieldInt64(19, maybeI64, void 0);
  }
  static addDefaultI64(builder, defaultI64) {
    builder.addFieldInt64(20, defaultI64, BigInt("42"));
  }
  static addJustU64(builder, justU64) {
    builder.addFieldInt64(21, justU64, BigInt("0"));
  }
  static addMaybeU64(builder, maybeU64) {
    builder.addFieldInt64(22, maybeU64, void 0);
  }
  static addDefaultU64(builder, defaultU64) {
    builder.addFieldInt64(23, defaultU64, BigInt("42"));
  }
  static addJustF32(builder, justF32) {
    builder.addFieldFloat32(24, justF32, 0);
  }
  static addMaybeF32(builder, maybeF32) {
    builder.addFieldFloat32(25, maybeF32, void 0);
  }
  static addDefaultF32(builder, defaultF32) {
    builder.addFieldFloat32(26, defaultF32, 42);
  }
  static addJustF64(builder, justF64) {
    builder.addFieldFloat64(27, justF64, 0);
  }
  static addMaybeF64(builder, maybeF64) {
    builder.addFieldFloat64(28, maybeF64, void 0);
  }
  static addDefaultF64(builder, defaultF64) {
    builder.addFieldFloat64(29, defaultF64, 42);
  }
  static addJustBool(builder, justBool) {
    builder.addFieldInt8(30, +justBool, 0);
  }
  static addMaybeBool(builder, maybeBool) {
    builder.addFieldInt8(31, +maybeBool, void 0);
  }
  static addDefaultBool(builder, defaultBool) {
    builder.addFieldInt8(32, +defaultBool, 1);
  }
  static addJustEnum(builder, justEnum) {
    builder.addFieldInt8(33, justEnum, 0 /* None */);
  }
  static addMaybeEnum(builder, maybeEnum) {
    builder.addFieldInt8(34, maybeEnum, void 0);
  }
  static addDefaultEnum(builder, defaultEnum) {
    builder.addFieldInt8(35, defaultEnum, 1 /* One */);
  }
  static endScalarStuff(builder) {
    const offset = builder.endObject();
    return offset;
  }
  static finishScalarStuffBuffer(builder, offset) {
    builder.finish(offset, "NULL");
  }
  static finishSizePrefixedScalarStuffBuffer(builder, offset) {
    builder.finish(offset, "NULL", true);
  }
  static createScalarStuff(builder, justI8, maybeI8, defaultI8, justU8, maybeU8, defaultU8, justI16, maybeI16, defaultI16, justU16, maybeU16, defaultU16, justI32, maybeI32, defaultI32, justU32, maybeU32, defaultU32, justI64, maybeI64, defaultI64, justU64, maybeU64, defaultU64, justF32, maybeF32, defaultF32, justF64, maybeF64, defaultF64, justBool, maybeBool, defaultBool, justEnum, maybeEnum, defaultEnum) {
    _ScalarStuff.startScalarStuff(builder);
    _ScalarStuff.addJustI8(builder, justI8);
    if (maybeI8 !== void 0)
      _ScalarStuff.addMaybeI8(builder, maybeI8);
    _ScalarStuff.addDefaultI8(builder, defaultI8);
    _ScalarStuff.addJustU8(builder, justU8);
    if (maybeU8 !== void 0)
      _ScalarStuff.addMaybeU8(builder, maybeU8);
    _ScalarStuff.addDefaultU8(builder, defaultU8);
    _ScalarStuff.addJustI16(builder, justI16);
    if (maybeI16 !== void 0)
      _ScalarStuff.addMaybeI16(builder, maybeI16);
    _ScalarStuff.addDefaultI16(builder, defaultI16);
    _ScalarStuff.addJustU16(builder, justU16);
    if (maybeU16 !== void 0)
      _ScalarStuff.addMaybeU16(builder, maybeU16);
    _ScalarStuff.addDefaultU16(builder, defaultU16);
    _ScalarStuff.addJustI32(builder, justI32);
    if (maybeI32 !== void 0)
      _ScalarStuff.addMaybeI32(builder, maybeI32);
    _ScalarStuff.addDefaultI32(builder, defaultI32);
    _ScalarStuff.addJustU32(builder, justU32);
    if (maybeU32 !== void 0)
      _ScalarStuff.addMaybeU32(builder, maybeU32);
    _ScalarStuff.addDefaultU32(builder, defaultU32);
    _ScalarStuff.addJustI64(builder, justI64);
    if (maybeI64 !== void 0)
      _ScalarStuff.addMaybeI64(builder, maybeI64);
    _ScalarStuff.addDefaultI64(builder, defaultI64);
    _ScalarStuff.addJustU64(builder, justU64);
    if (maybeU64 !== void 0)
      _ScalarStuff.addMaybeU64(builder, maybeU64);
    _ScalarStuff.addDefaultU64(builder, defaultU64);
    _ScalarStuff.addJustF32(builder, justF32);
    if (maybeF32 !== void 0)
      _ScalarStuff.addMaybeF32(builder, maybeF32);
    _ScalarStuff.addDefaultF32(builder, defaultF32);
    _ScalarStuff.addJustF64(builder, justF64);
    if (maybeF64 !== void 0)
      _ScalarStuff.addMaybeF64(builder, maybeF64);
    _ScalarStuff.addDefaultF64(builder, defaultF64);
    _ScalarStuff.addJustBool(builder, justBool);
    if (maybeBool !== void 0)
      _ScalarStuff.addMaybeBool(builder, maybeBool);
    _ScalarStuff.addDefaultBool(builder, defaultBool);
    _ScalarStuff.addJustEnum(builder, justEnum);
    if (maybeEnum !== void 0)
      _ScalarStuff.addMaybeEnum(builder, maybeEnum);
    _ScalarStuff.addDefaultEnum(builder, defaultEnum);
    return _ScalarStuff.endScalarStuff(builder);
  }
  unpack() {
    return new ScalarStuffT(
      this.justI8(),
      this.maybeI8(),
      this.defaultI8(),
      this.justU8(),
      this.maybeU8(),
      this.defaultU8(),
      this.justI16(),
      this.maybeI16(),
      this.defaultI16(),
      this.justU16(),
      this.maybeU16(),
      this.defaultU16(),
      this.justI32(),
      this.maybeI32(),
      this.defaultI32(),
      this.justU32(),
      this.maybeU32(),
      this.defaultU32(),
      this.justI64(),
      this.maybeI64(),
      this.defaultI64(),
      this.justU64(),
      this.maybeU64(),
      this.defaultU64(),
      this.justF32(),
      this.maybeF32(),
      this.defaultF32(),
      this.justF64(),
      this.maybeF64(),
      this.defaultF64(),
      this.justBool(),
      this.maybeBool(),
      this.defaultBool(),
      this.justEnum(),
      this.maybeEnum(),
      this.defaultEnum()
    );
  }
  unpackTo(_o) {
    _o.justI8 = this.justI8();
    _o.maybeI8 = this.maybeI8();
    _o.defaultI8 = this.defaultI8();
    _o.justU8 = this.justU8();
    _o.maybeU8 = this.maybeU8();
    _o.defaultU8 = this.defaultU8();
    _o.justI16 = this.justI16();
    _o.maybeI16 = this.maybeI16();
    _o.defaultI16 = this.defaultI16();
    _o.justU16 = this.justU16();
    _o.maybeU16 = this.maybeU16();
    _o.defaultU16 = this.defaultU16();
    _o.justI32 = this.justI32();
    _o.maybeI32 = this.maybeI32();
    _o.defaultI32 = this.defaultI32();
    _o.justU32 = this.justU32();
    _o.maybeU32 = this.maybeU32();
    _o.defaultU32 = this.defaultU32();
    _o.justI64 = this.justI64();
    _o.maybeI64 = this.maybeI64();
    _o.defaultI64 = this.defaultI64();
    _o.justU64 = this.justU64();
    _o.maybeU64 = this.maybeU64();
    _o.defaultU64 = this.defaultU64();
    _o.justF32 = this.justF32();
    _o.maybeF32 = this.maybeF32();
    _o.defaultF32 = this.defaultF32();
    _o.justF64 = this.justF64();
    _o.maybeF64 = this.maybeF64();
    _o.defaultF64 = this.defaultF64();
    _o.justBool = this.justBool();
    _o.maybeBool = this.maybeBool();
    _o.defaultBool = this.defaultBool();
    _o.justEnum = this.justEnum();
    _o.maybeEnum = this.maybeEnum();
    _o.defaultEnum = this.defaultEnum();
  }
};
var ScalarStuffT = class {
  constructor(justI8 = 0, maybeI8 = void 0, defaultI8 = 42, justU8 = 0, maybeU8 = void 0, defaultU8 = 42, justI16 = 0, maybeI16 = void 0, defaultI16 = 42, justU16 = 0, maybeU16 = void 0, defaultU16 = 42, justI32 = 0, maybeI32 = void 0, defaultI32 = 42, justU32 = 0, maybeU32 = void 0, defaultU32 = 42, justI64 = BigInt("0"), maybeI64 = void 0, defaultI64 = BigInt("42"), justU64 = BigInt("0"), maybeU64 = void 0, defaultU64 = BigInt("42"), justF32 = 0, maybeF32 = void 0, defaultF32 = 42, justF64 = 0, maybeF64 = void 0, defaultF64 = 42, justBool = false, maybeBool = void 0, defaultBool = true, justEnum = 0 /* None */, maybeEnum = void 0, defaultEnum = 1 /* One */) {
    this.justI8 = justI8;
    this.maybeI8 = maybeI8;
    this.defaultI8 = defaultI8;
    this.justU8 = justU8;
    this.maybeU8 = maybeU8;
    this.defaultU8 = defaultU8;
    this.justI16 = justI16;
    this.maybeI16 = maybeI16;
    this.defaultI16 = defaultI16;
    this.justU16 = justU16;
    this.maybeU16 = maybeU16;
    this.defaultU16 = defaultU16;
    this.justI32 = justI32;
    this.maybeI32 = maybeI32;
    this.defaultI32 = defaultI32;
    this.justU32 = justU32;
    this.maybeU32 = maybeU32;
    this.defaultU32 = defaultU32;
    this.justI64 = justI64;
    this.maybeI64 = maybeI64;
    this.defaultI64 = defaultI64;
    this.justU64 = justU64;
    this.maybeU64 = maybeU64;
    this.defaultU64 = defaultU64;
    this.justF32 = justF32;
    this.maybeF32 = maybeF32;
    this.defaultF32 = defaultF32;
    this.justF64 = justF64;
    this.maybeF64 = maybeF64;
    this.defaultF64 = defaultF64;
    this.justBool = justBool;
    this.maybeBool = maybeBool;
    this.defaultBool = defaultBool;
    this.justEnum = justEnum;
    this.maybeEnum = maybeEnum;
    this.defaultEnum = defaultEnum;
  }
  pack(builder) {
    return ScalarStuff.createScalarStuff(
      builder,
      this.justI8,
      this.maybeI8,
      this.defaultI8,
      this.justU8,
      this.maybeU8,
      this.defaultU8,
      this.justI16,
      this.maybeI16,
      this.defaultI16,
      this.justU16,
      this.maybeU16,
      this.defaultU16,
      this.justI32,
      this.maybeI32,
      this.defaultI32,
      this.justU32,
      this.maybeU32,
      this.defaultU32,
      this.justI64,
      this.maybeI64,
      this.defaultI64,
      this.justU64,
      this.maybeU64,
      this.defaultU64,
      this.justF32,
      this.maybeF32,
      this.defaultF32,
      this.justF64,
      this.maybeF64,
      this.defaultF64,
      this.justBool,
      this.maybeBool,
      this.defaultBool,
      this.justEnum,
      this.maybeEnum,
      this.defaultEnum
    );
  }
};
