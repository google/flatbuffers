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

// arrays_test_complex/MyGame/Example.ts
var Example_exports = {};
__export(Example_exports, {
  ArrayStruct: () => ArrayStruct,
  ArrayStructT: () => ArrayStructT,
  ArrayTable: () => ArrayTable,
  ArrayTableT: () => ArrayTableT,
  InnerStruct: () => InnerStruct,
  InnerStructT: () => InnerStructT,
  NestedStruct: () => NestedStruct,
  NestedStructT: () => NestedStructT,
  OuterStruct: () => OuterStruct,
  OuterStructT: () => OuterStructT,
  TestEnum: () => TestEnum
});
module.exports = __toCommonJS(Example_exports);

// arrays_test_complex/MyGame/Example/InnerStruct.js
var InnerStruct = class {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  a() {
    return this.bb.readFloat64(this.bb_pos);
  }
  b(index) {
    return this.bb.readUint8(this.bb_pos + 8 + index);
  }
  c() {
    return this.bb.readInt8(this.bb_pos + 21);
  }
  d_underscore() {
    return this.bb.readInt64(this.bb_pos + 24);
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.InnerStruct";
  }
  static sizeOf() {
    return 32;
  }
  static createInnerStruct(builder, a, b, c, d_underscore) {
    builder.prep(8, 32);
    builder.writeInt64(BigInt(d_underscore ?? 0));
    builder.pad(2);
    builder.writeInt8(c);
    for (let i = 12; i >= 0; --i) {
      builder.writeInt8(b?.[i] ?? 0);
    }
    builder.writeFloat64(a);
    return builder.offset();
  }
  unpack() {
    return new InnerStructT(this.a(), this.bb.createScalarList(this.b.bind(this), 13), this.c(), this.d_underscore());
  }
  unpackTo(_o) {
    _o.a = this.a();
    _o.b = this.bb.createScalarList(this.b.bind(this), 13);
    _o.c = this.c();
    _o.d_underscore = this.d_underscore();
  }
};
var InnerStructT = class {
  constructor(a = 0, b = [], c = 0, d_underscore = BigInt("0")) {
    this.a = a;
    this.b = b;
    this.c = c;
    this.d_underscore = d_underscore;
  }
  pack(builder) {
    return InnerStruct.createInnerStruct(builder, this.a, this.b, this.c, this.d_underscore);
  }
};

// arrays_test_complex/MyGame/Example/OuterStruct.js
var OuterStruct = class {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  a() {
    return !!this.bb.readInt8(this.bb_pos);
  }
  b() {
    return this.bb.readFloat64(this.bb_pos + 8);
  }
  c_underscore(obj) {
    return (obj || new InnerStruct()).__init(this.bb_pos + 16, this.bb);
  }
  d(index, obj) {
    return (obj || new InnerStruct()).__init(this.bb_pos + 48 + index * 32, this.bb);
  }
  e(obj) {
    return (obj || new InnerStruct()).__init(this.bb_pos + 144, this.bb);
  }
  f(index) {
    return this.bb.readFloat64(this.bb_pos + 176 + index * 8);
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.OuterStruct";
  }
  static sizeOf() {
    return 208;
  }
  static createOuterStruct(builder, a, b, c_underscore_a, c_underscore_b, c_underscore_c, c_underscore_d_underscore, d, e_a, e_b, e_c, e_d_underscore, f) {
    builder.prep(8, 208);
    for (let i = 3; i >= 0; --i) {
      builder.writeFloat64(f?.[i] ?? 0);
    }
    builder.prep(8, 32);
    builder.writeInt64(BigInt(e_d_underscore ?? 0));
    builder.pad(2);
    builder.writeInt8(e_c);
    for (let i = 12; i >= 0; --i) {
      builder.writeInt8(e_b?.[i] ?? 0);
    }
    builder.writeFloat64(e_a);
    for (let i = 2; i >= 0; --i) {
      const item = d?.[i];
      if (item instanceof InnerStructT) {
        item.pack(builder);
        continue;
      }
      InnerStruct.createInnerStruct(builder, item?.a, item?.b, item?.c, item?.d_underscore);
    }
    builder.prep(8, 32);
    builder.writeInt64(BigInt(c_underscore_d_underscore ?? 0));
    builder.pad(2);
    builder.writeInt8(c_underscore_c);
    for (let i = 12; i >= 0; --i) {
      builder.writeInt8(c_underscore_b?.[i] ?? 0);
    }
    builder.writeFloat64(c_underscore_a);
    builder.writeFloat64(b);
    builder.pad(7);
    builder.writeInt8(Number(Boolean(a)));
    return builder.offset();
  }
  unpack() {
    return new OuterStructT(this.a(), this.b(), this.c_underscore() !== null ? this.c_underscore().unpack() : null, this.bb.createObjList(this.d.bind(this), 3), this.e() !== null ? this.e().unpack() : null, this.bb.createScalarList(this.f.bind(this), 4));
  }
  unpackTo(_o) {
    _o.a = this.a();
    _o.b = this.b();
    _o.c_underscore = this.c_underscore() !== null ? this.c_underscore().unpack() : null;
    _o.d = this.bb.createObjList(this.d.bind(this), 3);
    _o.e = this.e() !== null ? this.e().unpack() : null;
    _o.f = this.bb.createScalarList(this.f.bind(this), 4);
  }
};
var OuterStructT = class {
  constructor(a = false, b = 0, c_underscore = null, d = [], e = null, f = []) {
    this.a = a;
    this.b = b;
    this.c_underscore = c_underscore;
    this.d = d;
    this.e = e;
    this.f = f;
  }
  pack(builder) {
    return OuterStruct.createOuterStruct(builder, this.a, this.b, this.c_underscore?.a ?? 0, this.c_underscore?.b ?? [], this.c_underscore?.c ?? 0, this.c_underscore?.d_underscore ?? BigInt(0), this.d, this.e?.a ?? 0, this.e?.b ?? [], this.e?.c ?? 0, this.e?.d_underscore ?? BigInt(0), this.f);
  }
};

// arrays_test_complex/MyGame/Example/TestEnum.js
var TestEnum;
(function(TestEnum2) {
  TestEnum2[TestEnum2["A"] = 0] = "A";
  TestEnum2[TestEnum2["B"] = 1] = "B";
  TestEnum2[TestEnum2["C"] = 2] = "C";
})(TestEnum || (TestEnum = {}));

// arrays_test_complex/MyGame/Example/NestedStruct.js
var NestedStruct = class {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  a(index) {
    return this.bb.readInt32(this.bb_pos + 0 + index * 4);
  }
  b() {
    return this.bb.readInt8(this.bb_pos + 8);
  }
  c_underscore(index) {
    return this.bb.readInt8(this.bb_pos + 9 + index);
  }
  d_outer(index, obj) {
    return (obj || new OuterStruct()).__init(this.bb_pos + 16 + index * 208, this.bb);
  }
  e(index) {
    return this.bb.readInt64(this.bb_pos + 1056 + index * 8);
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.NestedStruct";
  }
  static sizeOf() {
    return 1072;
  }
  static createNestedStruct(builder, a, b, c_underscore, d_outer, e) {
    builder.prep(8, 1072);
    for (let i = 1; i >= 0; --i) {
      builder.writeInt64(BigInt(e?.[i] ?? 0));
    }
    for (let i = 4; i >= 0; --i) {
      const item = d_outer?.[i];
      if (item instanceof OuterStructT) {
        item.pack(builder);
        continue;
      }
      OuterStruct.createOuterStruct(builder, item?.a, item?.b, item?.c_underscore?.a ?? 0, item?.c_underscore?.b ?? [], item?.c_underscore?.c ?? 0, item?.c_underscore?.d_underscore ?? BigInt(0), item?.d, item?.e?.a ?? 0, item?.e?.b ?? [], item?.e?.c ?? 0, item?.e?.d_underscore ?? BigInt(0), item?.f);
    }
    builder.pad(5);
    for (let i = 1; i >= 0; --i) {
      builder.writeInt8(c_underscore?.[i] ?? 0);
    }
    builder.writeInt8(b);
    for (let i = 1; i >= 0; --i) {
      builder.writeInt32(a?.[i] ?? 0);
    }
    return builder.offset();
  }
  unpack() {
    return new NestedStructT(this.bb.createScalarList(this.a.bind(this), 2), this.b(), this.bb.createScalarList(this.c_underscore.bind(this), 2), this.bb.createObjList(this.d_outer.bind(this), 5), this.bb.createScalarList(this.e.bind(this), 2));
  }
  unpackTo(_o) {
    _o.a = this.bb.createScalarList(this.a.bind(this), 2);
    _o.b = this.b();
    _o.c_underscore = this.bb.createScalarList(this.c_underscore.bind(this), 2);
    _o.d_outer = this.bb.createObjList(this.d_outer.bind(this), 5);
    _o.e = this.bb.createScalarList(this.e.bind(this), 2);
  }
};
var NestedStructT = class {
  constructor(a = [], b = TestEnum.A, c_underscore = [TestEnum.A, TestEnum.A], d_outer = [], e = []) {
    this.a = a;
    this.b = b;
    this.c_underscore = c_underscore;
    this.d_outer = d_outer;
    this.e = e;
  }
  pack(builder) {
    return NestedStruct.createNestedStruct(builder, this.a, this.b, this.c_underscore, this.d_outer, this.e);
  }
};

// arrays_test_complex/MyGame/Example/ArrayStruct.js
var ArrayStruct = class {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  a_underscore() {
    return this.bb.readFloat32(this.bb_pos);
  }
  b_underscore(index) {
    return this.bb.readInt32(this.bb_pos + 4 + index * 4);
  }
  c() {
    return this.bb.readInt8(this.bb_pos + 64);
  }
  d(index, obj) {
    return (obj || new NestedStruct()).__init(this.bb_pos + 72 + index * 1072, this.bb);
  }
  e() {
    return this.bb.readInt32(this.bb_pos + 2216);
  }
  f(index, obj) {
    return (obj || new OuterStruct()).__init(this.bb_pos + 2224 + index * 208, this.bb);
  }
  g(index) {
    return this.bb.readInt64(this.bb_pos + 2640 + index * 8);
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.ArrayStruct";
  }
  static sizeOf() {
    return 2656;
  }
  static createArrayStruct(builder, a_underscore, b_underscore, c, d, e, f, g) {
    builder.prep(8, 2656);
    for (let i = 1; i >= 0; --i) {
      builder.writeInt64(BigInt(g?.[i] ?? 0));
    }
    for (let i = 1; i >= 0; --i) {
      const item = f?.[i];
      if (item instanceof OuterStructT) {
        item.pack(builder);
        continue;
      }
      OuterStruct.createOuterStruct(builder, item?.a, item?.b, item?.c_underscore?.a ?? 0, item?.c_underscore?.b ?? [], item?.c_underscore?.c ?? 0, item?.c_underscore?.d_underscore ?? BigInt(0), item?.d, item?.e?.a ?? 0, item?.e?.b ?? [], item?.e?.c ?? 0, item?.e?.d_underscore ?? BigInt(0), item?.f);
    }
    builder.pad(4);
    builder.writeInt32(e);
    for (let i = 1; i >= 0; --i) {
      const item = d?.[i];
      if (item instanceof NestedStructT) {
        item.pack(builder);
        continue;
      }
      NestedStruct.createNestedStruct(builder, item?.a, item?.b, item?.c_underscore, item?.d_outer, item?.e);
    }
    builder.pad(7);
    builder.writeInt8(c);
    for (let i = 14; i >= 0; --i) {
      builder.writeInt32(b_underscore?.[i] ?? 0);
    }
    builder.writeFloat32(a_underscore);
    return builder.offset();
  }
  unpack() {
    return new ArrayStructT(this.a_underscore(), this.bb.createScalarList(this.b_underscore.bind(this), 15), this.c(), this.bb.createObjList(this.d.bind(this), 2), this.e(), this.bb.createObjList(this.f.bind(this), 2), this.bb.createScalarList(this.g.bind(this), 2));
  }
  unpackTo(_o) {
    _o.a_underscore = this.a_underscore();
    _o.b_underscore = this.bb.createScalarList(this.b_underscore.bind(this), 15);
    _o.c = this.c();
    _o.d = this.bb.createObjList(this.d.bind(this), 2);
    _o.e = this.e();
    _o.f = this.bb.createObjList(this.f.bind(this), 2);
    _o.g = this.bb.createScalarList(this.g.bind(this), 2);
  }
};
var ArrayStructT = class {
  constructor(a_underscore = 0, b_underscore = [], c = 0, d = [], e = 0, f = [], g = []) {
    this.a_underscore = a_underscore;
    this.b_underscore = b_underscore;
    this.c = c;
    this.d = d;
    this.e = e;
    this.f = f;
    this.g = g;
  }
  pack(builder) {
    return ArrayStruct.createArrayStruct(builder, this.a_underscore, this.b_underscore, this.c, this.d, this.e, this.f, this.g);
  }
};

// arrays_test_complex/MyGame/Example/ArrayTable.js
var flatbuffers = __toESM(require("flatbuffers"), 1);
var ArrayTable = class _ArrayTable {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsArrayTable(bb, obj) {
    return (obj || new _ArrayTable()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsArrayTable(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
    return (obj || new _ArrayTable()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static bufferHasIdentifier(bb) {
    return bb.__has_identifier("RHUB");
  }
  a(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  c_underscore(obj) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? (obj || new ArrayStruct()).__init(this.bb_pos + offset, this.bb) : null;
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.ArrayTable";
  }
  static startArrayTable(builder) {
    builder.startObject(2);
  }
  static add_a(builder, aOffset) {
    builder.addFieldOffset(0, aOffset, 0);
  }
  static add_c_underscore(builder, c_underscoreOffset) {
    builder.addFieldStruct(1, c_underscoreOffset, 0);
  }
  static endArrayTable(builder) {
    const offset = builder.endObject();
    return offset;
  }
  static finishArrayTableBuffer(builder, offset) {
    builder.finish(offset, "RHUB");
  }
  static finishSizePrefixedArrayTableBuffer(builder, offset) {
    builder.finish(offset, "RHUB", true);
  }
  unpack() {
    return new ArrayTableT(this.a(), this.c_underscore() !== null ? this.c_underscore().unpack() : null);
  }
  unpackTo(_o) {
    _o.a = this.a();
    _o.c_underscore = this.c_underscore() !== null ? this.c_underscore().unpack() : null;
  }
};
var ArrayTableT = class {
  constructor(a = null, c_underscore = null) {
    this.a = a;
    this.c_underscore = c_underscore;
  }
  pack(builder) {
    const a = this.a !== null ? builder.createString(this.a) : 0;
    ArrayTable.startArrayTable(builder);
    ArrayTable.add_a(builder, a);
    ArrayTable.add_c_underscore(builder, this.c_underscore !== null ? this.c_underscore.pack(builder) : 0);
    return ArrayTable.endArrayTable(builder);
  }
};
