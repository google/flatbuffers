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

// optional_values/optional_values.ts
var optional_values_exports = {};
__export(optional_values_exports, {
  LalalaOptions: () => LalalaOptions,
  LalalaOptionsT: () => LalalaOptionsT,
  OptionalValues: () => OptionalValues,
  OptionalValuesT: () => OptionalValuesT
});
module.exports = __toCommonJS(optional_values_exports);

// optional_values/lalala-options.ts
var flatbuffers = __toESM(require("flatbuffers"), 1);
var LalalaOptions = class _LalalaOptions {
  constructor() {
    this.bb = void 0;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsLalalaOptions(bb, obj) {
    return (obj || new _LalalaOptions()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsLalalaOptions(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
    return (obj || new _LalalaOptions()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  option1() {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.readUint64(this.bb_pos + offset) : BigInt("0");
  }
  option2(index) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.readUint32(this.bb.__vector(this.bb_pos + offset) + index * 4) : 0;
  }
  option2Length() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  option2Array() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? new Uint32Array(this.bb.bytes().buffer, this.bb.bytes().byteOffset + this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : void 0;
  }
  option3(index) {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? this.bb.readUint32(this.bb.__vector(this.bb_pos + offset) + index * 4) : 0;
  }
  option3Length() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  option3Array() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? new Uint32Array(this.bb.bytes().buffer, this.bb.bytes().byteOffset + this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : void 0;
  }
  static startLalalaOptions(builder) {
    builder.startObject(3);
  }
  static addOption1(builder, option1) {
    builder.addFieldInt64(0, option1, BigInt("0"));
  }
  static addOption2(builder, option2Offset) {
    builder.addFieldOffset(1, option2Offset, 0);
  }
  static createOption2Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addInt32(data[i]);
    }
    return builder.endVector();
  }
  static startOption2Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static addOption3(builder, option3Offset) {
    builder.addFieldOffset(2, option3Offset, 0);
  }
  static createOption3Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addInt32(data[i]);
    }
    return builder.endVector();
  }
  static startOption3Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static endLalalaOptions(builder) {
    const offset = builder.endObject();
    builder.requiredField(offset, 8);
    return offset;
  }
  static createLalalaOptions(builder, option1, option2Offset, option3Offset) {
    _LalalaOptions.startLalalaOptions(builder);
    _LalalaOptions.addOption1(builder, option1);
    _LalalaOptions.addOption2(builder, option2Offset);
    _LalalaOptions.addOption3(builder, option3Offset);
    return _LalalaOptions.endLalalaOptions(builder);
  }
  unpack() {
    return new LalalaOptionsT(
      this.option1(),
      this.bb.createScalarList(this.option2.bind(this), this.option2Length()),
      this.bb.createScalarList(this.option3.bind(this), this.option3Length())
    );
  }
  unpackTo(_o) {
    _o.option1 = this.option1();
    _o.option2 = this.bb.createScalarList(this.option2.bind(this), this.option2Length());
    _o.option3 = this.bb.createScalarList(this.option3.bind(this), this.option3Length());
  }
};
var LalalaOptionsT = class {
  constructor(option1 = BigInt("0"), option2 = [], option3 = []) {
    this.option1 = option1;
    this.option2 = option2;
    this.option3 = option3;
  }
  pack(builder) {
    const option2 = LalalaOptions.createOption2Vector(builder, this.option2);
    const option3 = LalalaOptions.createOption3Vector(builder, this.option3);
    return LalalaOptions.createLalalaOptions(
      builder,
      this.option1,
      option2,
      option3
    );
  }
};

// optional_values/optional-values.ts
var flatbuffers2 = __toESM(require("flatbuffers"), 1);
var OptionalValues = class _OptionalValues {
  constructor() {
    this.bb = void 0;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsOptionalValues(bb, obj) {
    return (obj || new _OptionalValues()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsOptionalValues(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers2.SIZE_PREFIX_LENGTH);
    return (obj || new _OptionalValues()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  fooString(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : void 0;
  }
  bar1Number() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.readUint16(this.bb_pos + offset) : 0;
  }
  bar2Number() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? this.bb.readUint16(this.bb_pos + offset) : 1234;
  }
  bar3Number() {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? this.bb.readUint16(this.bb_pos + offset) : void 0;
  }
  lalalaOptions1(obj) {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? (obj || new LalalaOptions()).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : void 0;
  }
  static startOptionalValues(builder) {
    builder.startObject(5);
  }
  static addFooString(builder, fooStringOffset) {
    builder.addFieldOffset(0, fooStringOffset, 0);
  }
  static addBar1Number(builder, bar1Number) {
    builder.addFieldInt16(1, bar1Number, 0);
  }
  static addBar2Number(builder, bar2Number) {
    builder.addFieldInt16(2, bar2Number, 1234);
  }
  static addBar3Number(builder, bar3Number) {
    builder.addFieldInt16(3, bar3Number, void 0);
  }
  static addLalalaOptions1(builder, lalalaOptions1Offset) {
    builder.addFieldOffset(4, lalalaOptions1Offset, 0);
  }
  static endOptionalValues(builder) {
    const offset = builder.endObject();
    return offset;
  }
  unpack() {
    return new OptionalValuesT(
      this.fooString(),
      this.bar1Number(),
      this.bar2Number(),
      this.bar3Number(),
      this.lalalaOptions1() !== void 0 ? this.lalalaOptions1().unpack() : void 0
    );
  }
  unpackTo(_o) {
    _o.fooString = this.fooString();
    _o.bar1Number = this.bar1Number();
    _o.bar2Number = this.bar2Number();
    _o.bar3Number = this.bar3Number();
    _o.lalalaOptions1 = this.lalalaOptions1() !== void 0 ? this.lalalaOptions1().unpack() : void 0;
  }
};
var OptionalValuesT = class {
  constructor(fooString = void 0, bar1Number = 0, bar2Number = 1234, bar3Number = void 0, lalalaOptions1 = void 0) {
    this.fooString = fooString;
    this.bar1Number = bar1Number;
    this.bar2Number = bar2Number;
    this.bar3Number = bar3Number;
    this.lalalaOptions1 = lalalaOptions1;
  }
  pack(builder) {
    const fooString = this.fooString !== void 0 ? builder.createString(this.fooString) : 0;
    const lalalaOptions1 = this.lalalaOptions1 !== void 0 ? this.lalalaOptions1.pack(builder) : 0;
    OptionalValues.startOptionalValues(builder);
    OptionalValues.addFooString(builder, fooString);
    OptionalValues.addBar1Number(builder, this.bar1Number);
    OptionalValues.addBar2Number(builder, this.bar2Number);
    if (this.bar3Number !== void 0)
      OptionalValues.addBar3Number(builder, this.bar3Number);
    OptionalValues.addLalalaOptions1(builder, lalalaOptions1);
    return OptionalValues.endOptionalValues(builder);
  }
};
