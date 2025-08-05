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

// monster_test.ts
var monster_test_exports = {};
__export(monster_test_exports, {
  MyGame: () => MyGame_exports,
  TableA: () => TableA,
  TableAT: () => TableAT
});
module.exports = __toCommonJS(monster_test_exports);

// TableA.js
var flatbuffers2 = __toESM(require("flatbuffers"), 1);

// MyGame/OtherNameSpace/TableB.js
var flatbuffers = __toESM(require("flatbuffers"), 1);
var TableB = class _TableB {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsTableB(bb, obj) {
    return (obj || new _TableB()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsTableB(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
    return (obj || new _TableB()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  a(obj) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? (obj || new TableA()).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : null;
  }
  static getFullyQualifiedName() {
    return "MyGame.OtherNameSpace.TableB";
  }
  static startTableB(builder) {
    builder.startObject(1);
  }
  static add_a(builder, aOffset) {
    builder.addFieldOffset(0, aOffset, 0);
  }
  static endTableB(builder) {
    const offset = builder.endObject();
    return offset;
  }
  static createTableB(builder, aOffset) {
    _TableB.startTableB(builder);
    _TableB.add_a(builder, aOffset);
    return _TableB.endTableB(builder);
  }
  serialize() {
    return this.bb.bytes();
  }
  static deserialize(buffer) {
    return _TableB.getRootAsTableB(new flatbuffers.ByteBuffer(buffer));
  }
  unpack() {
    return new TableBT(this.a() !== null ? this.a().unpack() : null);
  }
  unpackTo(_o) {
    _o.a = this.a() !== null ? this.a().unpack() : null;
  }
};
var TableBT = class {
  constructor(a = null) {
    this.a = a;
  }
  pack(builder) {
    const a = this.a !== null ? this.a.pack(builder) : 0;
    return TableB.createTableB(builder, a);
  }
};

// TableA.js
var TableA = class _TableA {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsTableA(bb, obj) {
    return (obj || new _TableA()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsTableA(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers2.SIZE_PREFIX_LENGTH);
    return (obj || new _TableA()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  b(obj) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? (obj || new TableB()).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : null;
  }
  static getFullyQualifiedName() {
    return "TableA";
  }
  static startTableA(builder) {
    builder.startObject(1);
  }
  static add_b(builder, bOffset) {
    builder.addFieldOffset(0, bOffset, 0);
  }
  static endTableA(builder) {
    const offset = builder.endObject();
    return offset;
  }
  static createTableA(builder, bOffset) {
    _TableA.startTableA(builder);
    _TableA.add_b(builder, bOffset);
    return _TableA.endTableA(builder);
  }
  serialize() {
    return this.bb.bytes();
  }
  static deserialize(buffer) {
    return _TableA.getRootAsTableA(new flatbuffers2.ByteBuffer(buffer));
  }
  unpack() {
    return new TableAT(this.b() !== null ? this.b().unpack() : null);
  }
  unpackTo(_o) {
    _o.b = this.b() !== null ? this.b().unpack() : null;
  }
};
var TableAT = class {
  constructor(b = null) {
    this.b = b;
  }
  pack(builder) {
    const b = this.b !== null ? this.b.pack(builder) : 0;
    return TableA.createTableA(builder, b);
  }
};

// MyGame.js
var MyGame_exports = {};
__export(MyGame_exports, {
  Example: () => Example_exports,
  Example2: () => Example2_exports,
  InParentNamespace: () => InParentNamespace,
  InParentNamespaceT: () => InParentNamespaceT,
  OtherNameSpace: () => OtherNameSpace_exports
});

// MyGame/InParentNamespace.js
var flatbuffers3 = __toESM(require("flatbuffers"), 1);
var InParentNamespace = class _InParentNamespace {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsInParentNamespace(bb, obj) {
    return (obj || new _InParentNamespace()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsInParentNamespace(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers3.SIZE_PREFIX_LENGTH);
    return (obj || new _InParentNamespace()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getFullyQualifiedName() {
    return "MyGame.InParentNamespace";
  }
  static startInParentNamespace(builder) {
    builder.startObject(0);
  }
  static endInParentNamespace(builder) {
    const offset = builder.endObject();
    return offset;
  }
  static createInParentNamespace(builder) {
    _InParentNamespace.startInParentNamespace(builder);
    return _InParentNamespace.endInParentNamespace(builder);
  }
  serialize() {
    return this.bb.bytes();
  }
  static deserialize(buffer) {
    return _InParentNamespace.getRootAsInParentNamespace(new flatbuffers3.ByteBuffer(buffer));
  }
  unpack() {
    return new InParentNamespaceT();
  }
  unpackTo(_o) {
  }
};
var InParentNamespaceT = class {
  constructor() {
  }
  pack(builder) {
    return InParentNamespace.createInParentNamespace(builder);
  }
};

// MyGame/Example.js
var Example_exports = {};
__export(Example_exports, {
  Ability: () => Ability,
  AbilityT: () => AbilityT,
  Any: () => Any,
  AnyAmbiguousAliases: () => AnyAmbiguousAliases,
  AnyUniqueAliases: () => AnyUniqueAliases,
  Color: () => Color,
  LongEnum: () => LongEnum,
  Monster: () => Monster2,
  MonsterT: () => MonsterT2,
  Race: () => Race,
  Referrable: () => Referrable,
  ReferrableT: () => ReferrableT,
  Stat: () => Stat,
  StatT: () => StatT,
  StructOfStructs: () => StructOfStructs,
  StructOfStructsOfStructs: () => StructOfStructsOfStructs,
  StructOfStructsOfStructsT: () => StructOfStructsOfStructsT,
  StructOfStructsT: () => StructOfStructsT,
  Test: () => Test,
  TestSimpleTableWithEnum: () => TestSimpleTableWithEnum,
  TestSimpleTableWithEnumT: () => TestSimpleTableWithEnumT,
  TestT: () => TestT,
  TypeAliases: () => TypeAliases,
  TypeAliasesT: () => TypeAliasesT,
  Vec3: () => Vec3,
  Vec3T: () => Vec3T
});

// MyGame/Example/Ability.js
var Ability = class {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  id() {
    return this.bb.readUint32(this.bb_pos);
  }
  mutate_id(value) {
    this.bb.writeUint32(this.bb_pos + 0, value);
    return true;
  }
  distance() {
    return this.bb.readUint32(this.bb_pos + 4);
  }
  mutate_distance(value) {
    this.bb.writeUint32(this.bb_pos + 4, value);
    return true;
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.Ability";
  }
  static sizeOf() {
    return 8;
  }
  static createAbility(builder, id, distance) {
    builder.prep(4, 8);
    builder.writeInt32(distance);
    builder.writeInt32(id);
    return builder.offset();
  }
  unpack() {
    return new AbilityT(this.id(), this.distance());
  }
  unpackTo(_o) {
    _o.id = this.id();
    _o.distance = this.distance();
  }
};
var AbilityT = class {
  constructor(id = 0, distance = 0) {
    this.id = id;
    this.distance = distance;
  }
  pack(builder) {
    return Ability.createAbility(builder, this.id, this.distance);
  }
};

// MyGame/Example2/Monster.js
var flatbuffers4 = __toESM(require("flatbuffers"), 1);
var Monster = class _Monster {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsMonster(bb, obj) {
    return (obj || new _Monster()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsMonster(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers4.SIZE_PREFIX_LENGTH);
    return (obj || new _Monster()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getFullyQualifiedName() {
    return "MyGame.Example2.Monster";
  }
  static startMonster(builder) {
    builder.startObject(0);
  }
  static endMonster(builder) {
    const offset = builder.endObject();
    return offset;
  }
  static createMonster(builder) {
    _Monster.startMonster(builder);
    return _Monster.endMonster(builder);
  }
  serialize() {
    return this.bb.bytes();
  }
  static deserialize(buffer) {
    return _Monster.getRootAsMonster(new flatbuffers4.ByteBuffer(buffer));
  }
  unpack() {
    return new MonsterT();
  }
  unpackTo(_o) {
  }
};
var MonsterT = class {
  constructor() {
  }
  pack(builder) {
    return Monster.createMonster(builder);
  }
};

// MyGame/Example/Monster.js
var flatbuffers8 = __toESM(require("flatbuffers"), 1);

// MyGame/Example/AnyAmbiguousAliases.js
var AnyAmbiguousAliases;
(function(AnyAmbiguousAliases2) {
  AnyAmbiguousAliases2[AnyAmbiguousAliases2["NONE"] = 0] = "NONE";
  AnyAmbiguousAliases2[AnyAmbiguousAliases2["M1"] = 1] = "M1";
  AnyAmbiguousAliases2[AnyAmbiguousAliases2["M2"] = 2] = "M2";
  AnyAmbiguousAliases2[AnyAmbiguousAliases2["M3"] = 3] = "M3";
})(AnyAmbiguousAliases || (AnyAmbiguousAliases = {}));
function unionToAnyAmbiguousAliases(type, accessor) {
  switch (AnyAmbiguousAliases[type]) {
    case "NONE":
      return null;
    case "M1":
      return accessor(new Monster2());
    case "M2":
      return accessor(new Monster2());
    case "M3":
      return accessor(new Monster2());
    default:
      return null;
  }
}

// MyGame/Example/TestSimpleTableWithEnum.js
var flatbuffers5 = __toESM(require("flatbuffers"), 1);

// MyGame/Example/Color.js
var Color;
(function(Color2) {
  Color2[Color2["Red"] = 1] = "Red";
  Color2[Color2["Green"] = 2] = "Green";
  Color2[Color2["Blue"] = 8] = "Blue";
})(Color || (Color = {}));

// MyGame/Example/TestSimpleTableWithEnum.js
var TestSimpleTableWithEnum = class _TestSimpleTableWithEnum {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsTestSimpleTableWithEnum(bb, obj) {
    return (obj || new _TestSimpleTableWithEnum()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsTestSimpleTableWithEnum(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers5.SIZE_PREFIX_LENGTH);
    return (obj || new _TestSimpleTableWithEnum()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  color() {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.readUint8(this.bb_pos + offset) : Color.Green;
  }
  mutate_color(value) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint8(this.bb_pos + offset, value);
    return true;
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.TestSimpleTableWithEnum";
  }
  static startTestSimpleTableWithEnum(builder) {
    builder.startObject(1);
  }
  static add_color(builder, color) {
    builder.addFieldInt8(0, color, Color.Green);
  }
  static endTestSimpleTableWithEnum(builder) {
    const offset = builder.endObject();
    return offset;
  }
  static createTestSimpleTableWithEnum(builder, color) {
    _TestSimpleTableWithEnum.startTestSimpleTableWithEnum(builder);
    _TestSimpleTableWithEnum.add_color(builder, color);
    return _TestSimpleTableWithEnum.endTestSimpleTableWithEnum(builder);
  }
  serialize() {
    return this.bb.bytes();
  }
  static deserialize(buffer) {
    return _TestSimpleTableWithEnum.getRootAsTestSimpleTableWithEnum(new flatbuffers5.ByteBuffer(buffer));
  }
  unpack() {
    return new TestSimpleTableWithEnumT(this.color());
  }
  unpackTo(_o) {
    _o.color = this.color();
  }
};
var TestSimpleTableWithEnumT = class {
  constructor(color = Color.Green) {
    this.color = color;
  }
  pack(builder) {
    return TestSimpleTableWithEnum.createTestSimpleTableWithEnum(builder, this.color);
  }
};

// MyGame/Example/AnyUniqueAliases.js
var AnyUniqueAliases;
(function(AnyUniqueAliases2) {
  AnyUniqueAliases2[AnyUniqueAliases2["NONE"] = 0] = "NONE";
  AnyUniqueAliases2[AnyUniqueAliases2["M"] = 1] = "M";
  AnyUniqueAliases2[AnyUniqueAliases2["TS"] = 2] = "TS";
  AnyUniqueAliases2[AnyUniqueAliases2["M2"] = 3] = "M2";
})(AnyUniqueAliases || (AnyUniqueAliases = {}));
function unionToAnyUniqueAliases(type, accessor) {
  switch (AnyUniqueAliases[type]) {
    case "NONE":
      return null;
    case "M":
      return accessor(new Monster2());
    case "TS":
      return accessor(new TestSimpleTableWithEnum());
    case "M2":
      return accessor(new Monster());
    default:
      return null;
  }
}

// MyGame/Example/Race.js
var Race;
(function(Race2) {
  Race2[Race2["None"] = -1] = "None";
  Race2[Race2["Human"] = 0] = "Human";
  Race2[Race2["Dwarf"] = 1] = "Dwarf";
  Race2[Race2["Elf"] = 2] = "Elf";
})(Race || (Race = {}));

// MyGame/Example/Referrable.js
var flatbuffers6 = __toESM(require("flatbuffers"), 1);
var Referrable = class _Referrable {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsReferrable(bb, obj) {
    return (obj || new _Referrable()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsReferrable(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers6.SIZE_PREFIX_LENGTH);
    return (obj || new _Referrable()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  id() {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.readUint64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_id(value) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint64(this.bb_pos + offset, value);
    return true;
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.Referrable";
  }
  static startReferrable(builder) {
    builder.startObject(1);
  }
  static add_id(builder, id) {
    builder.addFieldInt64(0, id, BigInt("0"));
  }
  static endReferrable(builder) {
    const offset = builder.endObject();
    return offset;
  }
  static createReferrable(builder, id) {
    _Referrable.startReferrable(builder);
    _Referrable.add_id(builder, id);
    return _Referrable.endReferrable(builder);
  }
  serialize() {
    return this.bb.bytes();
  }
  static deserialize(buffer) {
    return _Referrable.getRootAsReferrable(new flatbuffers6.ByteBuffer(buffer));
  }
  unpack() {
    return new ReferrableT(this.id());
  }
  unpackTo(_o) {
    _o.id = this.id();
  }
};
var ReferrableT = class {
  constructor(id = BigInt("0")) {
    this.id = id;
  }
  pack(builder) {
    return Referrable.createReferrable(builder, this.id);
  }
};

// MyGame/Example/Stat.js
var flatbuffers7 = __toESM(require("flatbuffers"), 1);
var Stat = class _Stat {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsStat(bb, obj) {
    return (obj || new _Stat()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsStat(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers7.SIZE_PREFIX_LENGTH);
    return (obj || new _Stat()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  id(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  val() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.readInt64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_val(value) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt64(this.bb_pos + offset, value);
    return true;
  }
  count() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? this.bb.readUint16(this.bb_pos + offset) : 0;
  }
  mutate_count(value) {
    const offset = this.bb.__offset(this.bb_pos, 8);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint16(this.bb_pos + offset, value);
    return true;
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.Stat";
  }
  static startStat(builder) {
    builder.startObject(3);
  }
  static add_id(builder, idOffset) {
    builder.addFieldOffset(0, idOffset, 0);
  }
  static add_val(builder, val) {
    builder.addFieldInt64(1, val, BigInt("0"));
  }
  static add_count(builder, count) {
    builder.addFieldInt16(2, count, 0);
  }
  static endStat(builder) {
    const offset = builder.endObject();
    return offset;
  }
  static createStat(builder, idOffset, val, count) {
    _Stat.startStat(builder);
    _Stat.add_id(builder, idOffset);
    _Stat.add_val(builder, val);
    _Stat.add_count(builder, count);
    return _Stat.endStat(builder);
  }
  serialize() {
    return this.bb.bytes();
  }
  static deserialize(buffer) {
    return _Stat.getRootAsStat(new flatbuffers7.ByteBuffer(buffer));
  }
  unpack() {
    return new StatT(this.id(), this.val(), this.count());
  }
  unpackTo(_o) {
    _o.id = this.id();
    _o.val = this.val();
    _o.count = this.count();
  }
};
var StatT = class {
  constructor(id = null, val = BigInt("0"), count = 0) {
    this.id = id;
    this.val = val;
    this.count = count;
  }
  pack(builder) {
    const id = this.id !== null ? builder.createString(this.id) : 0;
    return Stat.createStat(builder, id, this.val, this.count);
  }
};

// MyGame/Example/Test.js
var Test = class {
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
    return this.bb.readInt16(this.bb_pos);
  }
  mutate_a(value) {
    this.bb.writeInt16(this.bb_pos + 0, value);
    return true;
  }
  b() {
    return this.bb.readInt8(this.bb_pos + 2);
  }
  mutate_b(value) {
    this.bb.writeInt8(this.bb_pos + 2, value);
    return true;
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.Test";
  }
  static sizeOf() {
    return 4;
  }
  static createTest(builder, a, b) {
    builder.prep(2, 4);
    builder.pad(1);
    builder.writeInt8(b);
    builder.writeInt16(a);
    return builder.offset();
  }
  unpack() {
    return new TestT(this.a(), this.b());
  }
  unpackTo(_o) {
    _o.a = this.a();
    _o.b = this.b();
  }
};
var TestT = class {
  constructor(a = 0, b = 0) {
    this.a = a;
    this.b = b;
  }
  pack(builder) {
    return Test.createTest(builder, this.a, this.b);
  }
};

// MyGame/Example/Vec3.js
var Vec3 = class {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  x() {
    return this.bb.readFloat32(this.bb_pos);
  }
  mutate_x(value) {
    this.bb.writeFloat32(this.bb_pos + 0, value);
    return true;
  }
  y() {
    return this.bb.readFloat32(this.bb_pos + 4);
  }
  mutate_y(value) {
    this.bb.writeFloat32(this.bb_pos + 4, value);
    return true;
  }
  z() {
    return this.bb.readFloat32(this.bb_pos + 8);
  }
  mutate_z(value) {
    this.bb.writeFloat32(this.bb_pos + 8, value);
    return true;
  }
  test1() {
    return this.bb.readFloat64(this.bb_pos + 16);
  }
  mutate_test1(value) {
    this.bb.writeFloat64(this.bb_pos + 16, value);
    return true;
  }
  test2() {
    return this.bb.readUint8(this.bb_pos + 24);
  }
  mutate_test2(value) {
    this.bb.writeUint8(this.bb_pos + 24, value);
    return true;
  }
  test3(obj) {
    return (obj || new Test()).__init(this.bb_pos + 26, this.bb);
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.Vec3";
  }
  static sizeOf() {
    return 32;
  }
  static createVec3(builder, x, y, z, test1, test2, test3_a, test3_b) {
    builder.prep(8, 32);
    builder.pad(2);
    builder.prep(2, 4);
    builder.pad(1);
    builder.writeInt8(test3_b);
    builder.writeInt16(test3_a);
    builder.pad(1);
    builder.writeInt8(test2);
    builder.writeFloat64(test1);
    builder.pad(4);
    builder.writeFloat32(z);
    builder.writeFloat32(y);
    builder.writeFloat32(x);
    return builder.offset();
  }
  unpack() {
    return new Vec3T(this.x(), this.y(), this.z(), this.test1(), this.test2(), this.test3() !== null ? this.test3().unpack() : null);
  }
  unpackTo(_o) {
    _o.x = this.x();
    _o.y = this.y();
    _o.z = this.z();
    _o.test1 = this.test1();
    _o.test2 = this.test2();
    _o.test3 = this.test3() !== null ? this.test3().unpack() : null;
  }
};
var Vec3T = class {
  constructor(x = 0, y = 0, z = 0, test1 = 0, test2 = Color.Red, test3 = null) {
    this.x = x;
    this.y = y;
    this.z = z;
    this.test1 = test1;
    this.test2 = test2;
    this.test3 = test3;
  }
  pack(builder) {
    return Vec3.createVec3(builder, this.x, this.y, this.z, this.test1, this.test2, this.test3?.a ?? 0, this.test3?.b ?? 0);
  }
};

// MyGame/Example/Monster.js
var Monster2 = class _Monster {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsMonster(bb, obj) {
    return (obj || new _Monster()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsMonster(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers8.SIZE_PREFIX_LENGTH);
    return (obj || new _Monster()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static bufferHasIdentifier(bb) {
    return bb.__has_identifier("MONS");
  }
  pos(obj) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? (obj || new Vec3()).__init(this.bb_pos + offset, this.bb) : null;
  }
  mana() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.readInt16(this.bb_pos + offset) : 150;
  }
  mutate_mana(value) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt16(this.bb_pos + offset, value);
    return true;
  }
  hp() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? this.bb.readInt16(this.bb_pos + offset) : 100;
  }
  mutate_hp(value) {
    const offset = this.bb.__offset(this.bb_pos, 8);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt16(this.bb_pos + offset, value);
    return true;
  }
  name(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  inventory(index) {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? this.bb.readUint8(this.bb.__vector(this.bb_pos + offset) + index) : 0;
  }
  inventory_Length() {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  inventory_Array() {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? new Uint8Array(this.bb.bytes().buffer, this.bb.bytes().byteOffset + this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : null;
  }
  color() {
    const offset = this.bb.__offset(this.bb_pos, 16);
    return offset ? this.bb.readUint8(this.bb_pos + offset) : Color.Blue;
  }
  mutate_color(value) {
    const offset = this.bb.__offset(this.bb_pos, 16);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint8(this.bb_pos + offset, value);
    return true;
  }
  test_type() {
    const offset = this.bb.__offset(this.bb_pos, 18);
    return offset ? this.bb.readUint8(this.bb_pos + offset) : Any.NONE;
  }
  test(obj) {
    const offset = this.bb.__offset(this.bb_pos, 20);
    return offset ? this.bb.__union(obj, this.bb_pos + offset) : null;
  }
  test4(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 22);
    return offset ? (obj || new Test()).__init(this.bb.__vector(this.bb_pos + offset) + index * 4, this.bb) : null;
  }
  test4_Length() {
    const offset = this.bb.__offset(this.bb_pos, 22);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  testarrayofstring(index, optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 24);
    return offset ? this.bb.__string(this.bb.__vector(this.bb_pos + offset) + index * 4, optionalEncoding) : null;
  }
  testarrayofstring_Length() {
    const offset = this.bb.__offset(this.bb_pos, 24);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  /**
   * an example documentation comment: this will end up in the generated code
   * multiline too
   */
  testarrayoftables(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 26);
    return offset ? (obj || new _Monster()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  testarrayoftables_Length() {
    const offset = this.bb.__offset(this.bb_pos, 26);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  enemy(obj) {
    const offset = this.bb.__offset(this.bb_pos, 28);
    return offset ? (obj || new _Monster()).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : null;
  }
  testnestedflatbuffer(index) {
    const offset = this.bb.__offset(this.bb_pos, 30);
    return offset ? this.bb.readUint8(this.bb.__vector(this.bb_pos + offset) + index) : 0;
  }
  testnestedflatbuffer_Length() {
    const offset = this.bb.__offset(this.bb_pos, 30);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  testnestedflatbuffer_Array() {
    const offset = this.bb.__offset(this.bb_pos, 30);
    return offset ? new Uint8Array(this.bb.bytes().buffer, this.bb.bytes().byteOffset + this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : null;
  }
  testempty(obj) {
    const offset = this.bb.__offset(this.bb_pos, 32);
    return offset ? (obj || new Stat()).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : null;
  }
  testbool() {
    const offset = this.bb.__offset(this.bb_pos, 34);
    return offset ? !!this.bb.readInt8(this.bb_pos + offset) : false;
  }
  mutate_testbool(value) {
    const offset = this.bb.__offset(this.bb_pos, 34);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt8(this.bb_pos + offset, +value);
    return true;
  }
  testhashs32_fnv1() {
    const offset = this.bb.__offset(this.bb_pos, 36);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : 0;
  }
  mutate_testhashs32_fnv1(value) {
    const offset = this.bb.__offset(this.bb_pos, 36);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt32(this.bb_pos + offset, value);
    return true;
  }
  testhashu32_fnv1() {
    const offset = this.bb.__offset(this.bb_pos, 38);
    return offset ? this.bb.readUint32(this.bb_pos + offset) : 0;
  }
  mutate_testhashu32_fnv1(value) {
    const offset = this.bb.__offset(this.bb_pos, 38);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint32(this.bb_pos + offset, value);
    return true;
  }
  testhashs64_fnv1() {
    const offset = this.bb.__offset(this.bb_pos, 40);
    return offset ? this.bb.readInt64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_testhashs64_fnv1(value) {
    const offset = this.bb.__offset(this.bb_pos, 40);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt64(this.bb_pos + offset, value);
    return true;
  }
  testhashu64_fnv1() {
    const offset = this.bb.__offset(this.bb_pos, 42);
    return offset ? this.bb.readUint64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_testhashu64_fnv1(value) {
    const offset = this.bb.__offset(this.bb_pos, 42);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint64(this.bb_pos + offset, value);
    return true;
  }
  testhashs32_fnv1a() {
    const offset = this.bb.__offset(this.bb_pos, 44);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : 0;
  }
  mutate_testhashs32_fnv1a(value) {
    const offset = this.bb.__offset(this.bb_pos, 44);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt32(this.bb_pos + offset, value);
    return true;
  }
  testhashu32_fnv1a() {
    const offset = this.bb.__offset(this.bb_pos, 46);
    return offset ? this.bb.readUint32(this.bb_pos + offset) : 0;
  }
  mutate_testhashu32_fnv1a(value) {
    const offset = this.bb.__offset(this.bb_pos, 46);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint32(this.bb_pos + offset, value);
    return true;
  }
  testhashs64_fnv1a() {
    const offset = this.bb.__offset(this.bb_pos, 48);
    return offset ? this.bb.readInt64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_testhashs64_fnv1a(value) {
    const offset = this.bb.__offset(this.bb_pos, 48);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt64(this.bb_pos + offset, value);
    return true;
  }
  testhashu64_fnv1a() {
    const offset = this.bb.__offset(this.bb_pos, 50);
    return offset ? this.bb.readUint64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_testhashu64_fnv1a(value) {
    const offset = this.bb.__offset(this.bb_pos, 50);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint64(this.bb_pos + offset, value);
    return true;
  }
  testarrayofbools(index) {
    const offset = this.bb.__offset(this.bb_pos, 52);
    return offset ? !!this.bb.readInt8(this.bb.__vector(this.bb_pos + offset) + index) : false;
  }
  testarrayofbools_Length() {
    const offset = this.bb.__offset(this.bb_pos, 52);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  testarrayofbools_Array() {
    const offset = this.bb.__offset(this.bb_pos, 52);
    return offset ? new Int8Array(this.bb.bytes().buffer, this.bb.bytes().byteOffset + this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : null;
  }
  testf() {
    const offset = this.bb.__offset(this.bb_pos, 54);
    return offset ? this.bb.readFloat32(this.bb_pos + offset) : 3.14159;
  }
  mutate_testf(value) {
    const offset = this.bb.__offset(this.bb_pos, 54);
    if (offset === 0) {
      return false;
    }
    this.bb.writeFloat32(this.bb_pos + offset, value);
    return true;
  }
  testf2() {
    const offset = this.bb.__offset(this.bb_pos, 56);
    return offset ? this.bb.readFloat32(this.bb_pos + offset) : 3;
  }
  mutate_testf2(value) {
    const offset = this.bb.__offset(this.bb_pos, 56);
    if (offset === 0) {
      return false;
    }
    this.bb.writeFloat32(this.bb_pos + offset, value);
    return true;
  }
  testf3() {
    const offset = this.bb.__offset(this.bb_pos, 58);
    return offset ? this.bb.readFloat32(this.bb_pos + offset) : 0;
  }
  mutate_testf3(value) {
    const offset = this.bb.__offset(this.bb_pos, 58);
    if (offset === 0) {
      return false;
    }
    this.bb.writeFloat32(this.bb_pos + offset, value);
    return true;
  }
  testarrayofstring2(index, optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 60);
    return offset ? this.bb.__string(this.bb.__vector(this.bb_pos + offset) + index * 4, optionalEncoding) : null;
  }
  testarrayofstring2_Length() {
    const offset = this.bb.__offset(this.bb_pos, 60);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  testarrayofsortedstruct(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 62);
    return offset ? (obj || new Ability()).__init(this.bb.__vector(this.bb_pos + offset) + index * 8, this.bb) : null;
  }
  testarrayofsortedstruct_Length() {
    const offset = this.bb.__offset(this.bb_pos, 62);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  flex(index) {
    const offset = this.bb.__offset(this.bb_pos, 64);
    return offset ? this.bb.readUint8(this.bb.__vector(this.bb_pos + offset) + index) : 0;
  }
  flex_Length() {
    const offset = this.bb.__offset(this.bb_pos, 64);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  flex_Array() {
    const offset = this.bb.__offset(this.bb_pos, 64);
    return offset ? new Uint8Array(this.bb.bytes().buffer, this.bb.bytes().byteOffset + this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : null;
  }
  test5(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 66);
    return offset ? (obj || new Test()).__init(this.bb.__vector(this.bb_pos + offset) + index * 4, this.bb) : null;
  }
  test5_Length() {
    const offset = this.bb.__offset(this.bb_pos, 66);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  vector_of_longs(index) {
    const offset = this.bb.__offset(this.bb_pos, 68);
    return offset ? this.bb.readInt64(this.bb.__vector(this.bb_pos + offset) + index * 8) : BigInt(0);
  }
  vector_of_longs_Length() {
    const offset = this.bb.__offset(this.bb_pos, 68);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  vector_of_doubles(index) {
    const offset = this.bb.__offset(this.bb_pos, 70);
    return offset ? this.bb.readFloat64(this.bb.__vector(this.bb_pos + offset) + index * 8) : 0;
  }
  vector_of_doubles_Length() {
    const offset = this.bb.__offset(this.bb_pos, 70);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  vector_of_doubles_Array() {
    const offset = this.bb.__offset(this.bb_pos, 70);
    return offset ? new Float64Array(this.bb.bytes().buffer, this.bb.bytes().byteOffset + this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : null;
  }
  parent_namespace_test(obj) {
    const offset = this.bb.__offset(this.bb_pos, 72);
    return offset ? (obj || new InParentNamespace()).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : null;
  }
  vector_of_referrables(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 74);
    return offset ? (obj || new Referrable()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  vector_of_referrables_Length() {
    const offset = this.bb.__offset(this.bb_pos, 74);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  single_weak_reference() {
    const offset = this.bb.__offset(this.bb_pos, 76);
    return offset ? this.bb.readUint64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_single_weak_reference(value) {
    const offset = this.bb.__offset(this.bb_pos, 76);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint64(this.bb_pos + offset, value);
    return true;
  }
  vector_of_weak_references(index) {
    const offset = this.bb.__offset(this.bb_pos, 78);
    return offset ? this.bb.readUint64(this.bb.__vector(this.bb_pos + offset) + index * 8) : BigInt(0);
  }
  vector_of_weak_references_Length() {
    const offset = this.bb.__offset(this.bb_pos, 78);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  vector_of_strong_referrables(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 80);
    return offset ? (obj || new Referrable()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  vector_of_strong_referrables_Length() {
    const offset = this.bb.__offset(this.bb_pos, 80);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  co_owning_reference() {
    const offset = this.bb.__offset(this.bb_pos, 82);
    return offset ? this.bb.readUint64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_co_owning_reference(value) {
    const offset = this.bb.__offset(this.bb_pos, 82);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint64(this.bb_pos + offset, value);
    return true;
  }
  vector_of_co_owning_references(index) {
    const offset = this.bb.__offset(this.bb_pos, 84);
    return offset ? this.bb.readUint64(this.bb.__vector(this.bb_pos + offset) + index * 8) : BigInt(0);
  }
  vector_of_co_owning_references_Length() {
    const offset = this.bb.__offset(this.bb_pos, 84);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  non_owning_reference() {
    const offset = this.bb.__offset(this.bb_pos, 86);
    return offset ? this.bb.readUint64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_non_owning_reference(value) {
    const offset = this.bb.__offset(this.bb_pos, 86);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint64(this.bb_pos + offset, value);
    return true;
  }
  vector_of_non_owning_references(index) {
    const offset = this.bb.__offset(this.bb_pos, 88);
    return offset ? this.bb.readUint64(this.bb.__vector(this.bb_pos + offset) + index * 8) : BigInt(0);
  }
  vector_of_non_owning_references_Length() {
    const offset = this.bb.__offset(this.bb_pos, 88);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  any_unique_type() {
    const offset = this.bb.__offset(this.bb_pos, 90);
    return offset ? this.bb.readUint8(this.bb_pos + offset) : AnyUniqueAliases.NONE;
  }
  any_unique(obj) {
    const offset = this.bb.__offset(this.bb_pos, 92);
    return offset ? this.bb.__union(obj, this.bb_pos + offset) : null;
  }
  any_ambiguous_type() {
    const offset = this.bb.__offset(this.bb_pos, 94);
    return offset ? this.bb.readUint8(this.bb_pos + offset) : AnyAmbiguousAliases.NONE;
  }
  any_ambiguous(obj) {
    const offset = this.bb.__offset(this.bb_pos, 96);
    return offset ? this.bb.__union(obj, this.bb_pos + offset) : null;
  }
  vector_of_enums(index) {
    const offset = this.bb.__offset(this.bb_pos, 98);
    return offset ? this.bb.readUint8(this.bb.__vector(this.bb_pos + offset) + index) : null;
  }
  vector_of_enums_Length() {
    const offset = this.bb.__offset(this.bb_pos, 98);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  vector_of_enums_Array() {
    const offset = this.bb.__offset(this.bb_pos, 98);
    return offset ? new Uint8Array(this.bb.bytes().buffer, this.bb.bytes().byteOffset + this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : null;
  }
  signed_enum() {
    const offset = this.bb.__offset(this.bb_pos, 100);
    return offset ? this.bb.readInt8(this.bb_pos + offset) : Race.None;
  }
  mutate_signed_enum(value) {
    const offset = this.bb.__offset(this.bb_pos, 100);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt8(this.bb_pos + offset, value);
    return true;
  }
  testrequirednestedflatbuffer(index) {
    const offset = this.bb.__offset(this.bb_pos, 102);
    return offset ? this.bb.readUint8(this.bb.__vector(this.bb_pos + offset) + index) : 0;
  }
  testrequirednestedflatbuffer_Length() {
    const offset = this.bb.__offset(this.bb_pos, 102);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  testrequirednestedflatbuffer_Array() {
    const offset = this.bb.__offset(this.bb_pos, 102);
    return offset ? new Uint8Array(this.bb.bytes().buffer, this.bb.bytes().byteOffset + this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : null;
  }
  scalar_key_sorted_tables(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 104);
    return offset ? (obj || new Stat()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  scalar_key_sorted_tables_Length() {
    const offset = this.bb.__offset(this.bb_pos, 104);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  native_inline(obj) {
    const offset = this.bb.__offset(this.bb_pos, 106);
    return offset ? (obj || new Test()).__init(this.bb_pos + offset, this.bb) : null;
  }
  long_enum_non_enum_default() {
    const offset = this.bb.__offset(this.bb_pos, 108);
    return offset ? this.bb.readUint64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_long_enum_non_enum_default(value) {
    const offset = this.bb.__offset(this.bb_pos, 108);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint64(this.bb_pos + offset, value);
    return true;
  }
  long_enum_normal_default() {
    const offset = this.bb.__offset(this.bb_pos, 110);
    return offset ? this.bb.readUint64(this.bb_pos + offset) : BigInt("2");
  }
  mutate_long_enum_normal_default(value) {
    const offset = this.bb.__offset(this.bb_pos, 110);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint64(this.bb_pos + offset, value);
    return true;
  }
  nan_default() {
    const offset = this.bb.__offset(this.bb_pos, 112);
    return offset ? this.bb.readFloat32(this.bb_pos + offset) : NaN;
  }
  mutate_nan_default(value) {
    const offset = this.bb.__offset(this.bb_pos, 112);
    if (offset === 0) {
      return false;
    }
    this.bb.writeFloat32(this.bb_pos + offset, value);
    return true;
  }
  inf_default() {
    const offset = this.bb.__offset(this.bb_pos, 114);
    return offset ? this.bb.readFloat32(this.bb_pos + offset) : Infinity;
  }
  mutate_inf_default(value) {
    const offset = this.bb.__offset(this.bb_pos, 114);
    if (offset === 0) {
      return false;
    }
    this.bb.writeFloat32(this.bb_pos + offset, value);
    return true;
  }
  positive_inf_default() {
    const offset = this.bb.__offset(this.bb_pos, 116);
    return offset ? this.bb.readFloat32(this.bb_pos + offset) : Infinity;
  }
  mutate_positive_inf_default(value) {
    const offset = this.bb.__offset(this.bb_pos, 116);
    if (offset === 0) {
      return false;
    }
    this.bb.writeFloat32(this.bb_pos + offset, value);
    return true;
  }
  infinity_default() {
    const offset = this.bb.__offset(this.bb_pos, 118);
    return offset ? this.bb.readFloat32(this.bb_pos + offset) : Infinity;
  }
  mutate_infinity_default(value) {
    const offset = this.bb.__offset(this.bb_pos, 118);
    if (offset === 0) {
      return false;
    }
    this.bb.writeFloat32(this.bb_pos + offset, value);
    return true;
  }
  positive_infinity_default() {
    const offset = this.bb.__offset(this.bb_pos, 120);
    return offset ? this.bb.readFloat32(this.bb_pos + offset) : Infinity;
  }
  mutate_positive_infinity_default(value) {
    const offset = this.bb.__offset(this.bb_pos, 120);
    if (offset === 0) {
      return false;
    }
    this.bb.writeFloat32(this.bb_pos + offset, value);
    return true;
  }
  negative_inf_default() {
    const offset = this.bb.__offset(this.bb_pos, 122);
    return offset ? this.bb.readFloat32(this.bb_pos + offset) : -Infinity;
  }
  mutate_negative_inf_default(value) {
    const offset = this.bb.__offset(this.bb_pos, 122);
    if (offset === 0) {
      return false;
    }
    this.bb.writeFloat32(this.bb_pos + offset, value);
    return true;
  }
  negative_infinity_default() {
    const offset = this.bb.__offset(this.bb_pos, 124);
    return offset ? this.bb.readFloat32(this.bb_pos + offset) : -Infinity;
  }
  mutate_negative_infinity_default(value) {
    const offset = this.bb.__offset(this.bb_pos, 124);
    if (offset === 0) {
      return false;
    }
    this.bb.writeFloat32(this.bb_pos + offset, value);
    return true;
  }
  double_inf_default() {
    const offset = this.bb.__offset(this.bb_pos, 126);
    return offset ? this.bb.readFloat64(this.bb_pos + offset) : Infinity;
  }
  mutate_double_inf_default(value) {
    const offset = this.bb.__offset(this.bb_pos, 126);
    if (offset === 0) {
      return false;
    }
    this.bb.writeFloat64(this.bb_pos + offset, value);
    return true;
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.Monster";
  }
  static startMonster(builder) {
    builder.startObject(62);
  }
  static add_pos(builder, posOffset) {
    builder.addFieldStruct(0, posOffset, 0);
  }
  static add_mana(builder, mana) {
    builder.addFieldInt16(1, mana, 150);
  }
  static add_hp(builder, hp) {
    builder.addFieldInt16(2, hp, 100);
  }
  static add_name(builder, nameOffset) {
    builder.addFieldOffset(3, nameOffset, 0);
  }
  static add_inventory(builder, inventoryOffset) {
    builder.addFieldOffset(5, inventoryOffset, 0);
  }
  static create_inventory_Vector(builder, data) {
    builder.startVector(1, data.length, 1);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addInt8(data[i]);
    }
    return builder.endVector();
  }
  static start_inventory_Vector(builder, numElems) {
    builder.startVector(1, numElems, 1);
  }
  static add_color(builder, color) {
    builder.addFieldInt8(6, color, Color.Blue);
  }
  static add_test_type(builder, test_type) {
    builder.addFieldInt8(7, test_type, Any.NONE);
  }
  static add_test(builder, testOffset) {
    builder.addFieldOffset(8, testOffset, 0);
  }
  static add_test4(builder, test4Offset) {
    builder.addFieldOffset(9, test4Offset, 0);
  }
  static start_test4_Vector(builder, numElems) {
    builder.startVector(4, numElems, 2);
  }
  static add_testarrayofstring(builder, testarrayofstringOffset) {
    builder.addFieldOffset(10, testarrayofstringOffset, 0);
  }
  static create_testarrayofstring_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_testarrayofstring_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_testarrayoftables(builder, testarrayoftablesOffset) {
    builder.addFieldOffset(11, testarrayoftablesOffset, 0);
  }
  static create_testarrayoftables_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_testarrayoftables_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_enemy(builder, enemyOffset) {
    builder.addFieldOffset(12, enemyOffset, 0);
  }
  static add_testnestedflatbuffer(builder, testnestedflatbufferOffset) {
    builder.addFieldOffset(13, testnestedflatbufferOffset, 0);
  }
  static create_testnestedflatbuffer_Vector(builder, data) {
    builder.startVector(1, data.length, 1);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addInt8(data[i]);
    }
    return builder.endVector();
  }
  static start_testnestedflatbuffer_Vector(builder, numElems) {
    builder.startVector(1, numElems, 1);
  }
  static add_testempty(builder, testemptyOffset) {
    builder.addFieldOffset(14, testemptyOffset, 0);
  }
  static add_testbool(builder, testbool) {
    builder.addFieldInt8(15, +testbool, 0);
  }
  static add_testhashs32_fnv1(builder, testhashs32_fnv1) {
    builder.addFieldInt32(16, testhashs32_fnv1, 0);
  }
  static add_testhashu32_fnv1(builder, testhashu32_fnv1) {
    builder.addFieldInt32(17, testhashu32_fnv1, 0);
  }
  static add_testhashs64_fnv1(builder, testhashs64_fnv1) {
    builder.addFieldInt64(18, testhashs64_fnv1, BigInt("0"));
  }
  static add_testhashu64_fnv1(builder, testhashu64_fnv1) {
    builder.addFieldInt64(19, testhashu64_fnv1, BigInt("0"));
  }
  static add_testhashs32_fnv1a(builder, testhashs32_fnv1a) {
    builder.addFieldInt32(20, testhashs32_fnv1a, 0);
  }
  static add_testhashu32_fnv1a(builder, testhashu32_fnv1a) {
    builder.addFieldInt32(21, testhashu32_fnv1a, 0);
  }
  static add_testhashs64_fnv1a(builder, testhashs64_fnv1a) {
    builder.addFieldInt64(22, testhashs64_fnv1a, BigInt("0"));
  }
  static add_testhashu64_fnv1a(builder, testhashu64_fnv1a) {
    builder.addFieldInt64(23, testhashu64_fnv1a, BigInt("0"));
  }
  static add_testarrayofbools(builder, testarrayofboolsOffset) {
    builder.addFieldOffset(24, testarrayofboolsOffset, 0);
  }
  static create_testarrayofbools_Vector(builder, data) {
    builder.startVector(1, data.length, 1);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addInt8(+data[i]);
    }
    return builder.endVector();
  }
  static start_testarrayofbools_Vector(builder, numElems) {
    builder.startVector(1, numElems, 1);
  }
  static add_testf(builder, testf) {
    builder.addFieldFloat32(25, testf, 3.14159);
  }
  static add_testf2(builder, testf2) {
    builder.addFieldFloat32(26, testf2, 3);
  }
  static add_testf3(builder, testf3) {
    builder.addFieldFloat32(27, testf3, 0);
  }
  static add_testarrayofstring2(builder, testarrayofstring2Offset) {
    builder.addFieldOffset(28, testarrayofstring2Offset, 0);
  }
  static create_testarrayofstring2_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_testarrayofstring2_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_testarrayofsortedstruct(builder, testarrayofsortedstructOffset) {
    builder.addFieldOffset(29, testarrayofsortedstructOffset, 0);
  }
  static start_testarrayofsortedstruct_Vector(builder, numElems) {
    builder.startVector(8, numElems, 4);
  }
  static add_flex(builder, flexOffset) {
    builder.addFieldOffset(30, flexOffset, 0);
  }
  static create_flex_Vector(builder, data) {
    builder.startVector(1, data.length, 1);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addInt8(data[i]);
    }
    return builder.endVector();
  }
  static start_flex_Vector(builder, numElems) {
    builder.startVector(1, numElems, 1);
  }
  static add_test5(builder, test5Offset) {
    builder.addFieldOffset(31, test5Offset, 0);
  }
  static start_test5_Vector(builder, numElems) {
    builder.startVector(4, numElems, 2);
  }
  static add_vector_of_longs(builder, vector_of_longsOffset) {
    builder.addFieldOffset(32, vector_of_longsOffset, 0);
  }
  static create_vector_of_longs_Vector(builder, data) {
    builder.startVector(8, data.length, 8);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addInt64(data[i]);
    }
    return builder.endVector();
  }
  static start_vector_of_longs_Vector(builder, numElems) {
    builder.startVector(8, numElems, 8);
  }
  static add_vector_of_doubles(builder, vector_of_doublesOffset) {
    builder.addFieldOffset(33, vector_of_doublesOffset, 0);
  }
  static create_vector_of_doubles_Vector(builder, data) {
    builder.startVector(8, data.length, 8);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addFloat64(data[i]);
    }
    return builder.endVector();
  }
  static start_vector_of_doubles_Vector(builder, numElems) {
    builder.startVector(8, numElems, 8);
  }
  static add_parent_namespace_test(builder, parent_namespace_testOffset) {
    builder.addFieldOffset(34, parent_namespace_testOffset, 0);
  }
  static add_vector_of_referrables(builder, vector_of_referrablesOffset) {
    builder.addFieldOffset(35, vector_of_referrablesOffset, 0);
  }
  static create_vector_of_referrables_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_vector_of_referrables_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_single_weak_reference(builder, single_weak_reference) {
    builder.addFieldInt64(36, single_weak_reference, BigInt("0"));
  }
  static add_vector_of_weak_references(builder, vector_of_weak_referencesOffset) {
    builder.addFieldOffset(37, vector_of_weak_referencesOffset, 0);
  }
  static create_vector_of_weak_references_Vector(builder, data) {
    builder.startVector(8, data.length, 8);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addInt64(data[i]);
    }
    return builder.endVector();
  }
  static start_vector_of_weak_references_Vector(builder, numElems) {
    builder.startVector(8, numElems, 8);
  }
  static add_vector_of_strong_referrables(builder, vector_of_strong_referrablesOffset) {
    builder.addFieldOffset(38, vector_of_strong_referrablesOffset, 0);
  }
  static create_vector_of_strong_referrables_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_vector_of_strong_referrables_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_co_owning_reference(builder, co_owning_reference) {
    builder.addFieldInt64(39, co_owning_reference, BigInt("0"));
  }
  static add_vector_of_co_owning_references(builder, vector_of_co_owning_referencesOffset) {
    builder.addFieldOffset(40, vector_of_co_owning_referencesOffset, 0);
  }
  static create_vector_of_co_owning_references_Vector(builder, data) {
    builder.startVector(8, data.length, 8);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addInt64(data[i]);
    }
    return builder.endVector();
  }
  static start_vector_of_co_owning_references_Vector(builder, numElems) {
    builder.startVector(8, numElems, 8);
  }
  static add_non_owning_reference(builder, non_owning_reference) {
    builder.addFieldInt64(41, non_owning_reference, BigInt("0"));
  }
  static add_vector_of_non_owning_references(builder, vector_of_non_owning_referencesOffset) {
    builder.addFieldOffset(42, vector_of_non_owning_referencesOffset, 0);
  }
  static create_vector_of_non_owning_references_Vector(builder, data) {
    builder.startVector(8, data.length, 8);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addInt64(data[i]);
    }
    return builder.endVector();
  }
  static start_vector_of_non_owning_references_Vector(builder, numElems) {
    builder.startVector(8, numElems, 8);
  }
  static add_any_unique_type(builder, any_unique_type) {
    builder.addFieldInt8(43, any_unique_type, AnyUniqueAliases.NONE);
  }
  static add_any_unique(builder, any_uniqueOffset) {
    builder.addFieldOffset(44, any_uniqueOffset, 0);
  }
  static add_any_ambiguous_type(builder, any_ambiguous_type) {
    builder.addFieldInt8(45, any_ambiguous_type, AnyAmbiguousAliases.NONE);
  }
  static add_any_ambiguous(builder, any_ambiguousOffset) {
    builder.addFieldOffset(46, any_ambiguousOffset, 0);
  }
  static add_vector_of_enums(builder, vector_of_enumsOffset) {
    builder.addFieldOffset(47, vector_of_enumsOffset, 0);
  }
  static create_vector_of_enums_Vector(builder, data) {
    builder.startVector(1, data.length, 1);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addInt8(data[i]);
    }
    return builder.endVector();
  }
  static start_vector_of_enums_Vector(builder, numElems) {
    builder.startVector(1, numElems, 1);
  }
  static add_signed_enum(builder, signed_enum) {
    builder.addFieldInt8(48, signed_enum, Race.None);
  }
  static add_testrequirednestedflatbuffer(builder, testrequirednestedflatbufferOffset) {
    builder.addFieldOffset(49, testrequirednestedflatbufferOffset, 0);
  }
  static create_testrequirednestedflatbuffer_Vector(builder, data) {
    builder.startVector(1, data.length, 1);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addInt8(data[i]);
    }
    return builder.endVector();
  }
  static start_testrequirednestedflatbuffer_Vector(builder, numElems) {
    builder.startVector(1, numElems, 1);
  }
  static add_scalar_key_sorted_tables(builder, scalar_key_sorted_tablesOffset) {
    builder.addFieldOffset(50, scalar_key_sorted_tablesOffset, 0);
  }
  static create_scalar_key_sorted_tables_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_scalar_key_sorted_tables_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_native_inline(builder, native_inlineOffset) {
    builder.addFieldStruct(51, native_inlineOffset, 0);
  }
  static add_long_enum_non_enum_default(builder, long_enum_non_enum_default) {
    builder.addFieldInt64(52, long_enum_non_enum_default, BigInt("0"));
  }
  static add_long_enum_normal_default(builder, long_enum_normal_default) {
    builder.addFieldInt64(53, long_enum_normal_default, BigInt("2"));
  }
  static add_nan_default(builder, nan_default) {
    builder.addFieldFloat32(54, nan_default, NaN);
  }
  static add_inf_default(builder, inf_default) {
    builder.addFieldFloat32(55, inf_default, Infinity);
  }
  static add_positive_inf_default(builder, positive_inf_default) {
    builder.addFieldFloat32(56, positive_inf_default, Infinity);
  }
  static add_infinity_default(builder, infinity_default) {
    builder.addFieldFloat32(57, infinity_default, Infinity);
  }
  static add_positive_infinity_default(builder, positive_infinity_default) {
    builder.addFieldFloat32(58, positive_infinity_default, Infinity);
  }
  static add_negative_inf_default(builder, negative_inf_default) {
    builder.addFieldFloat32(59, negative_inf_default, -Infinity);
  }
  static add_negative_infinity_default(builder, negative_infinity_default) {
    builder.addFieldFloat32(60, negative_infinity_default, -Infinity);
  }
  static add_double_inf_default(builder, double_inf_default) {
    builder.addFieldFloat64(61, double_inf_default, Infinity);
  }
  static endMonster(builder) {
    const offset = builder.endObject();
    builder.requiredField(offset, 10);
    return offset;
  }
  static finishMonsterBuffer(builder, offset) {
    builder.finish(offset, "MONS");
  }
  static finishSizePrefixedMonsterBuffer(builder, offset) {
    builder.finish(offset, "MONS", true);
  }
  serialize() {
    return this.bb.bytes();
  }
  static deserialize(buffer) {
    return _Monster.getRootAsMonster(new flatbuffers8.ByteBuffer(buffer));
  }
  unpack() {
    return new MonsterT2(this.pos() !== null ? this.pos().unpack() : null, this.mana(), this.hp(), this.name(), this.bb.createScalarList(this.inventory.bind(this), this.inventory_Length()), this.color(), this.test_type(), (() => {
      const temp = unionToAny(this.test_type(), this.test.bind(this));
      if (temp === null) {
        return null;
      }
      return temp.unpack();
    })(), this.bb.createObjList(this.test4.bind(this), this.test4_Length()), this.bb.createScalarList(this.testarrayofstring.bind(this), this.testarrayofstring_Length()), this.bb.createObjList(this.testarrayoftables.bind(this), this.testarrayoftables_Length()), this.enemy() !== null ? this.enemy().unpack() : null, this.bb.createScalarList(this.testnestedflatbuffer.bind(this), this.testnestedflatbuffer_Length()), this.testempty() !== null ? this.testempty().unpack() : null, this.testbool(), this.testhashs32_fnv1(), this.testhashu32_fnv1(), this.testhashs64_fnv1(), this.testhashu64_fnv1(), this.testhashs32_fnv1a(), this.testhashu32_fnv1a(), this.testhashs64_fnv1a(), this.testhashu64_fnv1a(), this.bb.createScalarList(this.testarrayofbools.bind(this), this.testarrayofbools_Length()), this.testf(), this.testf2(), this.testf3(), this.bb.createScalarList(this.testarrayofstring2.bind(this), this.testarrayofstring2_Length()), this.bb.createObjList(this.testarrayofsortedstruct.bind(this), this.testarrayofsortedstruct_Length()), this.bb.createScalarList(this.flex.bind(this), this.flex_Length()), this.bb.createObjList(this.test5.bind(this), this.test5_Length()), this.bb.createScalarList(this.vector_of_longs.bind(this), this.vector_of_longs_Length()), this.bb.createScalarList(this.vector_of_doubles.bind(this), this.vector_of_doubles_Length()), this.parent_namespace_test() !== null ? this.parent_namespace_test().unpack() : null, this.bb.createObjList(this.vector_of_referrables.bind(this), this.vector_of_referrables_Length()), this.single_weak_reference(), this.bb.createScalarList(this.vector_of_weak_references.bind(this), this.vector_of_weak_references_Length()), this.bb.createObjList(this.vector_of_strong_referrables.bind(this), this.vector_of_strong_referrables_Length()), this.co_owning_reference(), this.bb.createScalarList(this.vector_of_co_owning_references.bind(this), this.vector_of_co_owning_references_Length()), this.non_owning_reference(), this.bb.createScalarList(this.vector_of_non_owning_references.bind(this), this.vector_of_non_owning_references_Length()), this.any_unique_type(), (() => {
      const temp = unionToAnyUniqueAliases(this.any_unique_type(), this.any_unique.bind(this));
      if (temp === null) {
        return null;
      }
      return temp.unpack();
    })(), this.any_ambiguous_type(), (() => {
      const temp = unionToAnyAmbiguousAliases(this.any_ambiguous_type(), this.any_ambiguous.bind(this));
      if (temp === null) {
        return null;
      }
      return temp.unpack();
    })(), this.bb.createScalarList(this.vector_of_enums.bind(this), this.vector_of_enums_Length()), this.signed_enum(), this.bb.createScalarList(this.testrequirednestedflatbuffer.bind(this), this.testrequirednestedflatbuffer_Length()), this.bb.createObjList(this.scalar_key_sorted_tables.bind(this), this.scalar_key_sorted_tables_Length()), this.native_inline() !== null ? this.native_inline().unpack() : null, this.long_enum_non_enum_default(), this.long_enum_normal_default(), this.nan_default(), this.inf_default(), this.positive_inf_default(), this.infinity_default(), this.positive_infinity_default(), this.negative_inf_default(), this.negative_infinity_default(), this.double_inf_default());
  }
  unpackTo(_o) {
    _o.pos = this.pos() !== null ? this.pos().unpack() : null;
    _o.mana = this.mana();
    _o.hp = this.hp();
    _o.name = this.name();
    _o.inventory = this.bb.createScalarList(this.inventory.bind(this), this.inventory_Length());
    _o.color = this.color();
    _o.test_type = this.test_type();
    _o.test = (() => {
      const temp = unionToAny(this.test_type(), this.test.bind(this));
      if (temp === null) {
        return null;
      }
      return temp.unpack();
    })();
    _o.test4 = this.bb.createObjList(this.test4.bind(this), this.test4_Length());
    _o.testarrayofstring = this.bb.createScalarList(this.testarrayofstring.bind(this), this.testarrayofstring_Length());
    _o.testarrayoftables = this.bb.createObjList(this.testarrayoftables.bind(this), this.testarrayoftables_Length());
    _o.enemy = this.enemy() !== null ? this.enemy().unpack() : null;
    _o.testnestedflatbuffer = this.bb.createScalarList(this.testnestedflatbuffer.bind(this), this.testnestedflatbuffer_Length());
    _o.testempty = this.testempty() !== null ? this.testempty().unpack() : null;
    _o.testbool = this.testbool();
    _o.testhashs32_fnv1 = this.testhashs32_fnv1();
    _o.testhashu32_fnv1 = this.testhashu32_fnv1();
    _o.testhashs64_fnv1 = this.testhashs64_fnv1();
    _o.testhashu64_fnv1 = this.testhashu64_fnv1();
    _o.testhashs32_fnv1a = this.testhashs32_fnv1a();
    _o.testhashu32_fnv1a = this.testhashu32_fnv1a();
    _o.testhashs64_fnv1a = this.testhashs64_fnv1a();
    _o.testhashu64_fnv1a = this.testhashu64_fnv1a();
    _o.testarrayofbools = this.bb.createScalarList(this.testarrayofbools.bind(this), this.testarrayofbools_Length());
    _o.testf = this.testf();
    _o.testf2 = this.testf2();
    _o.testf3 = this.testf3();
    _o.testarrayofstring2 = this.bb.createScalarList(this.testarrayofstring2.bind(this), this.testarrayofstring2_Length());
    _o.testarrayofsortedstruct = this.bb.createObjList(this.testarrayofsortedstruct.bind(this), this.testarrayofsortedstruct_Length());
    _o.flex = this.bb.createScalarList(this.flex.bind(this), this.flex_Length());
    _o.test5 = this.bb.createObjList(this.test5.bind(this), this.test5_Length());
    _o.vector_of_longs = this.bb.createScalarList(this.vector_of_longs.bind(this), this.vector_of_longs_Length());
    _o.vector_of_doubles = this.bb.createScalarList(this.vector_of_doubles.bind(this), this.vector_of_doubles_Length());
    _o.parent_namespace_test = this.parent_namespace_test() !== null ? this.parent_namespace_test().unpack() : null;
    _o.vector_of_referrables = this.bb.createObjList(this.vector_of_referrables.bind(this), this.vector_of_referrables_Length());
    _o.single_weak_reference = this.single_weak_reference();
    _o.vector_of_weak_references = this.bb.createScalarList(this.vector_of_weak_references.bind(this), this.vector_of_weak_references_Length());
    _o.vector_of_strong_referrables = this.bb.createObjList(this.vector_of_strong_referrables.bind(this), this.vector_of_strong_referrables_Length());
    _o.co_owning_reference = this.co_owning_reference();
    _o.vector_of_co_owning_references = this.bb.createScalarList(this.vector_of_co_owning_references.bind(this), this.vector_of_co_owning_references_Length());
    _o.non_owning_reference = this.non_owning_reference();
    _o.vector_of_non_owning_references = this.bb.createScalarList(this.vector_of_non_owning_references.bind(this), this.vector_of_non_owning_references_Length());
    _o.any_unique_type = this.any_unique_type();
    _o.any_unique = (() => {
      const temp = unionToAnyUniqueAliases(this.any_unique_type(), this.any_unique.bind(this));
      if (temp === null) {
        return null;
      }
      return temp.unpack();
    })();
    _o.any_ambiguous_type = this.any_ambiguous_type();
    _o.any_ambiguous = (() => {
      const temp = unionToAnyAmbiguousAliases(this.any_ambiguous_type(), this.any_ambiguous.bind(this));
      if (temp === null) {
        return null;
      }
      return temp.unpack();
    })();
    _o.vector_of_enums = this.bb.createScalarList(this.vector_of_enums.bind(this), this.vector_of_enums_Length());
    _o.signed_enum = this.signed_enum();
    _o.testrequirednestedflatbuffer = this.bb.createScalarList(this.testrequirednestedflatbuffer.bind(this), this.testrequirednestedflatbuffer_Length());
    _o.scalar_key_sorted_tables = this.bb.createObjList(this.scalar_key_sorted_tables.bind(this), this.scalar_key_sorted_tables_Length());
    _o.native_inline = this.native_inline() !== null ? this.native_inline().unpack() : null;
    _o.long_enum_non_enum_default = this.long_enum_non_enum_default();
    _o.long_enum_normal_default = this.long_enum_normal_default();
    _o.nan_default = this.nan_default();
    _o.inf_default = this.inf_default();
    _o.positive_inf_default = this.positive_inf_default();
    _o.infinity_default = this.infinity_default();
    _o.positive_infinity_default = this.positive_infinity_default();
    _o.negative_inf_default = this.negative_inf_default();
    _o.negative_infinity_default = this.negative_infinity_default();
    _o.double_inf_default = this.double_inf_default();
  }
};
var MonsterT2 = class {
  constructor(pos = null, mana = 150, hp = 100, name = null, inventory = [], color = Color.Blue, test_type = Any.NONE, test = null, test4 = [], testarrayofstring = [], testarrayoftables = [], enemy = null, testnestedflatbuffer = [], testempty = null, testbool = false, testhashs32_fnv1 = 0, testhashu32_fnv1 = 0, testhashs64_fnv1 = BigInt("0"), testhashu64_fnv1 = BigInt("0"), testhashs32_fnv1a = 0, testhashu32_fnv1a = 0, testhashs64_fnv1a = BigInt("0"), testhashu64_fnv1a = BigInt("0"), testarrayofbools = [], testf = 3.14159, testf2 = 3, testf3 = 0, testarrayofstring2 = [], testarrayofsortedstruct = [], flex = [], test5 = [], vector_of_longs = [], vector_of_doubles = [], parent_namespace_test = null, vector_of_referrables = [], single_weak_reference = BigInt("0"), vector_of_weak_references = [], vector_of_strong_referrables = [], co_owning_reference = BigInt("0"), vector_of_co_owning_references = [], non_owning_reference = BigInt("0"), vector_of_non_owning_references = [], any_unique_type = AnyUniqueAliases.NONE, any_unique = null, any_ambiguous_type = AnyAmbiguousAliases.NONE, any_ambiguous = null, vector_of_enums = [], signed_enum = Race.None, testrequirednestedflatbuffer = [], scalar_key_sorted_tables = [], native_inline = null, long_enum_non_enum_default = BigInt("0"), long_enum_normal_default = BigInt("2"), nan_default = NaN, inf_default = Infinity, positive_inf_default = Infinity, infinity_default = Infinity, positive_infinity_default = Infinity, negative_inf_default = -Infinity, negative_infinity_default = -Infinity, double_inf_default = Infinity) {
    this.pos = pos;
    this.mana = mana;
    this.hp = hp;
    this.name = name;
    this.inventory = inventory;
    this.color = color;
    this.test_type = test_type;
    this.test = test;
    this.test4 = test4;
    this.testarrayofstring = testarrayofstring;
    this.testarrayoftables = testarrayoftables;
    this.enemy = enemy;
    this.testnestedflatbuffer = testnestedflatbuffer;
    this.testempty = testempty;
    this.testbool = testbool;
    this.testhashs32_fnv1 = testhashs32_fnv1;
    this.testhashu32_fnv1 = testhashu32_fnv1;
    this.testhashs64_fnv1 = testhashs64_fnv1;
    this.testhashu64_fnv1 = testhashu64_fnv1;
    this.testhashs32_fnv1a = testhashs32_fnv1a;
    this.testhashu32_fnv1a = testhashu32_fnv1a;
    this.testhashs64_fnv1a = testhashs64_fnv1a;
    this.testhashu64_fnv1a = testhashu64_fnv1a;
    this.testarrayofbools = testarrayofbools;
    this.testf = testf;
    this.testf2 = testf2;
    this.testf3 = testf3;
    this.testarrayofstring2 = testarrayofstring2;
    this.testarrayofsortedstruct = testarrayofsortedstruct;
    this.flex = flex;
    this.test5 = test5;
    this.vector_of_longs = vector_of_longs;
    this.vector_of_doubles = vector_of_doubles;
    this.parent_namespace_test = parent_namespace_test;
    this.vector_of_referrables = vector_of_referrables;
    this.single_weak_reference = single_weak_reference;
    this.vector_of_weak_references = vector_of_weak_references;
    this.vector_of_strong_referrables = vector_of_strong_referrables;
    this.co_owning_reference = co_owning_reference;
    this.vector_of_co_owning_references = vector_of_co_owning_references;
    this.non_owning_reference = non_owning_reference;
    this.vector_of_non_owning_references = vector_of_non_owning_references;
    this.any_unique_type = any_unique_type;
    this.any_unique = any_unique;
    this.any_ambiguous_type = any_ambiguous_type;
    this.any_ambiguous = any_ambiguous;
    this.vector_of_enums = vector_of_enums;
    this.signed_enum = signed_enum;
    this.testrequirednestedflatbuffer = testrequirednestedflatbuffer;
    this.scalar_key_sorted_tables = scalar_key_sorted_tables;
    this.native_inline = native_inline;
    this.long_enum_non_enum_default = long_enum_non_enum_default;
    this.long_enum_normal_default = long_enum_normal_default;
    this.nan_default = nan_default;
    this.inf_default = inf_default;
    this.positive_inf_default = positive_inf_default;
    this.infinity_default = infinity_default;
    this.positive_infinity_default = positive_infinity_default;
    this.negative_inf_default = negative_inf_default;
    this.negative_infinity_default = negative_infinity_default;
    this.double_inf_default = double_inf_default;
  }
  pack(builder) {
    const name = this.name !== null ? builder.createString(this.name) : 0;
    const inventory = Monster2.create_inventory_Vector(builder, this.inventory);
    const test = builder.createObjectOffset(this.test);
    const test4 = builder.createStructOffsetList(this.test4, Monster2.start_test4_Vector);
    const testarrayofstring = Monster2.create_testarrayofstring_Vector(builder, builder.createObjectOffsetList(this.testarrayofstring));
    const testarrayoftables = Monster2.create_testarrayoftables_Vector(builder, builder.createObjectOffsetList(this.testarrayoftables));
    const enemy = this.enemy !== null ? this.enemy.pack(builder) : 0;
    const testnestedflatbuffer = Monster2.create_testnestedflatbuffer_Vector(builder, this.testnestedflatbuffer);
    const testempty = this.testempty !== null ? this.testempty.pack(builder) : 0;
    const testarrayofbools = Monster2.create_testarrayofbools_Vector(builder, this.testarrayofbools);
    const testarrayofstring2 = Monster2.create_testarrayofstring2_Vector(builder, builder.createObjectOffsetList(this.testarrayofstring2));
    const testarrayofsortedstruct = builder.createStructOffsetList(this.testarrayofsortedstruct, Monster2.start_testarrayofsortedstruct_Vector);
    const flex = Monster2.create_flex_Vector(builder, this.flex);
    const test5 = builder.createStructOffsetList(this.test5, Monster2.start_test5_Vector);
    const vector_of_longs = Monster2.create_vector_of_longs_Vector(builder, this.vector_of_longs);
    const vector_of_doubles = Monster2.create_vector_of_doubles_Vector(builder, this.vector_of_doubles);
    const parent_namespace_test = this.parent_namespace_test !== null ? this.parent_namespace_test.pack(builder) : 0;
    const vector_of_referrables = Monster2.create_vector_of_referrables_Vector(builder, builder.createObjectOffsetList(this.vector_of_referrables));
    const vector_of_weak_references = Monster2.create_vector_of_weak_references_Vector(builder, this.vector_of_weak_references);
    const vector_of_strong_referrables = Monster2.create_vector_of_strong_referrables_Vector(builder, builder.createObjectOffsetList(this.vector_of_strong_referrables));
    const vector_of_co_owning_references = Monster2.create_vector_of_co_owning_references_Vector(builder, this.vector_of_co_owning_references);
    const vector_of_non_owning_references = Monster2.create_vector_of_non_owning_references_Vector(builder, this.vector_of_non_owning_references);
    const any_unique = builder.createObjectOffset(this.any_unique);
    const any_ambiguous = builder.createObjectOffset(this.any_ambiguous);
    const vector_of_enums = Monster2.create_vector_of_enums_Vector(builder, this.vector_of_enums);
    const testrequirednestedflatbuffer = Monster2.create_testrequirednestedflatbuffer_Vector(builder, this.testrequirednestedflatbuffer);
    const scalar_key_sorted_tables = Monster2.create_scalar_key_sorted_tables_Vector(builder, builder.createObjectOffsetList(this.scalar_key_sorted_tables));
    Monster2.startMonster(builder);
    Monster2.add_pos(builder, this.pos !== null ? this.pos.pack(builder) : 0);
    Monster2.add_mana(builder, this.mana);
    Monster2.add_hp(builder, this.hp);
    Monster2.add_name(builder, name);
    Monster2.add_inventory(builder, inventory);
    Monster2.add_color(builder, this.color);
    Monster2.add_test_type(builder, this.test_type);
    Monster2.add_test(builder, test);
    Monster2.add_test4(builder, test4);
    Monster2.add_testarrayofstring(builder, testarrayofstring);
    Monster2.add_testarrayoftables(builder, testarrayoftables);
    Monster2.add_enemy(builder, enemy);
    Monster2.add_testnestedflatbuffer(builder, testnestedflatbuffer);
    Monster2.add_testempty(builder, testempty);
    Monster2.add_testbool(builder, this.testbool);
    Monster2.add_testhashs32_fnv1(builder, this.testhashs32_fnv1);
    Monster2.add_testhashu32_fnv1(builder, this.testhashu32_fnv1);
    Monster2.add_testhashs64_fnv1(builder, this.testhashs64_fnv1);
    Monster2.add_testhashu64_fnv1(builder, this.testhashu64_fnv1);
    Monster2.add_testhashs32_fnv1a(builder, this.testhashs32_fnv1a);
    Monster2.add_testhashu32_fnv1a(builder, this.testhashu32_fnv1a);
    Monster2.add_testhashs64_fnv1a(builder, this.testhashs64_fnv1a);
    Monster2.add_testhashu64_fnv1a(builder, this.testhashu64_fnv1a);
    Monster2.add_testarrayofbools(builder, testarrayofbools);
    Monster2.add_testf(builder, this.testf);
    Monster2.add_testf2(builder, this.testf2);
    Monster2.add_testf3(builder, this.testf3);
    Monster2.add_testarrayofstring2(builder, testarrayofstring2);
    Monster2.add_testarrayofsortedstruct(builder, testarrayofsortedstruct);
    Monster2.add_flex(builder, flex);
    Monster2.add_test5(builder, test5);
    Monster2.add_vector_of_longs(builder, vector_of_longs);
    Monster2.add_vector_of_doubles(builder, vector_of_doubles);
    Monster2.add_parent_namespace_test(builder, parent_namespace_test);
    Monster2.add_vector_of_referrables(builder, vector_of_referrables);
    Monster2.add_single_weak_reference(builder, this.single_weak_reference);
    Monster2.add_vector_of_weak_references(builder, vector_of_weak_references);
    Monster2.add_vector_of_strong_referrables(builder, vector_of_strong_referrables);
    Monster2.add_co_owning_reference(builder, this.co_owning_reference);
    Monster2.add_vector_of_co_owning_references(builder, vector_of_co_owning_references);
    Monster2.add_non_owning_reference(builder, this.non_owning_reference);
    Monster2.add_vector_of_non_owning_references(builder, vector_of_non_owning_references);
    Monster2.add_any_unique_type(builder, this.any_unique_type);
    Monster2.add_any_unique(builder, any_unique);
    Monster2.add_any_ambiguous_type(builder, this.any_ambiguous_type);
    Monster2.add_any_ambiguous(builder, any_ambiguous);
    Monster2.add_vector_of_enums(builder, vector_of_enums);
    Monster2.add_signed_enum(builder, this.signed_enum);
    Monster2.add_testrequirednestedflatbuffer(builder, testrequirednestedflatbuffer);
    Monster2.add_scalar_key_sorted_tables(builder, scalar_key_sorted_tables);
    Monster2.add_native_inline(builder, this.native_inline !== null ? this.native_inline.pack(builder) : 0);
    Monster2.add_long_enum_non_enum_default(builder, this.long_enum_non_enum_default);
    Monster2.add_long_enum_normal_default(builder, this.long_enum_normal_default);
    Monster2.add_nan_default(builder, this.nan_default);
    Monster2.add_inf_default(builder, this.inf_default);
    Monster2.add_positive_inf_default(builder, this.positive_inf_default);
    Monster2.add_infinity_default(builder, this.infinity_default);
    Monster2.add_positive_infinity_default(builder, this.positive_infinity_default);
    Monster2.add_negative_inf_default(builder, this.negative_inf_default);
    Monster2.add_negative_infinity_default(builder, this.negative_infinity_default);
    Monster2.add_double_inf_default(builder, this.double_inf_default);
    return Monster2.endMonster(builder);
  }
};

// MyGame/Example/Any.js
var Any;
(function(Any2) {
  Any2[Any2["NONE"] = 0] = "NONE";
  Any2[Any2["Monster"] = 1] = "Monster";
  Any2[Any2["TestSimpleTableWithEnum"] = 2] = "TestSimpleTableWithEnum";
  Any2[Any2["MyGame_Example2_Monster"] = 3] = "MyGame_Example2_Monster";
})(Any || (Any = {}));
function unionToAny(type, accessor) {
  switch (Any[type]) {
    case "NONE":
      return null;
    case "Monster":
      return accessor(new Monster2());
    case "TestSimpleTableWithEnum":
      return accessor(new TestSimpleTableWithEnum());
    case "MyGame_Example2_Monster":
      return accessor(new Monster());
    default:
      return null;
  }
}

// MyGame/Example/LongEnum.js
var LongEnum;
(function(LongEnum2) {
  LongEnum2["LongOne"] = "2";
  LongEnum2["LongTwo"] = "4";
  LongEnum2["LongBig"] = "1099511627776";
})(LongEnum || (LongEnum = {}));

// MyGame/Example/StructOfStructs.js
var StructOfStructs = class {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  a(obj) {
    return (obj || new Ability()).__init(this.bb_pos, this.bb);
  }
  b(obj) {
    return (obj || new Test()).__init(this.bb_pos + 8, this.bb);
  }
  c(obj) {
    return (obj || new Ability()).__init(this.bb_pos + 12, this.bb);
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.StructOfStructs";
  }
  static sizeOf() {
    return 20;
  }
  static createStructOfStructs(builder, a_id, a_distance, b_a, b_b, c_id, c_distance) {
    builder.prep(4, 20);
    builder.prep(4, 8);
    builder.writeInt32(c_distance);
    builder.writeInt32(c_id);
    builder.prep(2, 4);
    builder.pad(1);
    builder.writeInt8(b_b);
    builder.writeInt16(b_a);
    builder.prep(4, 8);
    builder.writeInt32(a_distance);
    builder.writeInt32(a_id);
    return builder.offset();
  }
  unpack() {
    return new StructOfStructsT(this.a() !== null ? this.a().unpack() : null, this.b() !== null ? this.b().unpack() : null, this.c() !== null ? this.c().unpack() : null);
  }
  unpackTo(_o) {
    _o.a = this.a() !== null ? this.a().unpack() : null;
    _o.b = this.b() !== null ? this.b().unpack() : null;
    _o.c = this.c() !== null ? this.c().unpack() : null;
  }
};
var StructOfStructsT = class {
  constructor(a = null, b = null, c = null) {
    this.a = a;
    this.b = b;
    this.c = c;
  }
  pack(builder) {
    return StructOfStructs.createStructOfStructs(builder, this.a?.id ?? 0, this.a?.distance ?? 0, this.b?.a ?? 0, this.b?.b ?? 0, this.c?.id ?? 0, this.c?.distance ?? 0);
  }
};

// MyGame/Example/StructOfStructsOfStructs.js
var StructOfStructsOfStructs = class {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  a(obj) {
    return (obj || new StructOfStructs()).__init(this.bb_pos, this.bb);
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.StructOfStructsOfStructs";
  }
  static sizeOf() {
    return 20;
  }
  static createStructOfStructsOfStructs(builder, a_a_id, a_a_distance, a_b_a, a_b_b, a_c_id, a_c_distance) {
    builder.prep(4, 20);
    builder.prep(4, 20);
    builder.prep(4, 8);
    builder.writeInt32(a_c_distance);
    builder.writeInt32(a_c_id);
    builder.prep(2, 4);
    builder.pad(1);
    builder.writeInt8(a_b_b);
    builder.writeInt16(a_b_a);
    builder.prep(4, 8);
    builder.writeInt32(a_a_distance);
    builder.writeInt32(a_a_id);
    return builder.offset();
  }
  unpack() {
    return new StructOfStructsOfStructsT(this.a() !== null ? this.a().unpack() : null);
  }
  unpackTo(_o) {
    _o.a = this.a() !== null ? this.a().unpack() : null;
  }
};
var StructOfStructsOfStructsT = class {
  constructor(a = null) {
    this.a = a;
  }
  pack(builder) {
    return StructOfStructsOfStructs.createStructOfStructsOfStructs(builder, this.a?.a?.id ?? 0, this.a?.a?.distance ?? 0, this.a?.b?.a ?? 0, this.a?.b?.b ?? 0, this.a?.c?.id ?? 0, this.a?.c?.distance ?? 0);
  }
};

// MyGame/Example/TypeAliases.js
var flatbuffers9 = __toESM(require("flatbuffers"), 1);
var TypeAliases = class _TypeAliases {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsTypeAliases(bb, obj) {
    return (obj || new _TypeAliases()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsTypeAliases(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers9.SIZE_PREFIX_LENGTH);
    return (obj || new _TypeAliases()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  i8() {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.readInt8(this.bb_pos + offset) : 0;
  }
  mutate_i8(value) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt8(this.bb_pos + offset, value);
    return true;
  }
  u8() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.readUint8(this.bb_pos + offset) : 0;
  }
  mutate_u8(value) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint8(this.bb_pos + offset, value);
    return true;
  }
  i16() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? this.bb.readInt16(this.bb_pos + offset) : 0;
  }
  mutate_i16(value) {
    const offset = this.bb.__offset(this.bb_pos, 8);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt16(this.bb_pos + offset, value);
    return true;
  }
  u16() {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? this.bb.readUint16(this.bb_pos + offset) : 0;
  }
  mutate_u16(value) {
    const offset = this.bb.__offset(this.bb_pos, 10);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint16(this.bb_pos + offset, value);
    return true;
  }
  i32() {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : 0;
  }
  mutate_i32(value) {
    const offset = this.bb.__offset(this.bb_pos, 12);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt32(this.bb_pos + offset, value);
    return true;
  }
  u32() {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? this.bb.readUint32(this.bb_pos + offset) : 0;
  }
  mutate_u32(value) {
    const offset = this.bb.__offset(this.bb_pos, 14);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint32(this.bb_pos + offset, value);
    return true;
  }
  i64() {
    const offset = this.bb.__offset(this.bb_pos, 16);
    return offset ? this.bb.readInt64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_i64(value) {
    const offset = this.bb.__offset(this.bb_pos, 16);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt64(this.bb_pos + offset, value);
    return true;
  }
  u64() {
    const offset = this.bb.__offset(this.bb_pos, 18);
    return offset ? this.bb.readUint64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_u64(value) {
    const offset = this.bb.__offset(this.bb_pos, 18);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint64(this.bb_pos + offset, value);
    return true;
  }
  f32() {
    const offset = this.bb.__offset(this.bb_pos, 20);
    return offset ? this.bb.readFloat32(this.bb_pos + offset) : 0;
  }
  mutate_f32(value) {
    const offset = this.bb.__offset(this.bb_pos, 20);
    if (offset === 0) {
      return false;
    }
    this.bb.writeFloat32(this.bb_pos + offset, value);
    return true;
  }
  f64() {
    const offset = this.bb.__offset(this.bb_pos, 22);
    return offset ? this.bb.readFloat64(this.bb_pos + offset) : 0;
  }
  mutate_f64(value) {
    const offset = this.bb.__offset(this.bb_pos, 22);
    if (offset === 0) {
      return false;
    }
    this.bb.writeFloat64(this.bb_pos + offset, value);
    return true;
  }
  v8(index) {
    const offset = this.bb.__offset(this.bb_pos, 24);
    return offset ? this.bb.readInt8(this.bb.__vector(this.bb_pos + offset) + index) : 0;
  }
  v8_Length() {
    const offset = this.bb.__offset(this.bb_pos, 24);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  v8_Array() {
    const offset = this.bb.__offset(this.bb_pos, 24);
    return offset ? new Int8Array(this.bb.bytes().buffer, this.bb.bytes().byteOffset + this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : null;
  }
  vf64(index) {
    const offset = this.bb.__offset(this.bb_pos, 26);
    return offset ? this.bb.readFloat64(this.bb.__vector(this.bb_pos + offset) + index * 8) : 0;
  }
  vf64_Length() {
    const offset = this.bb.__offset(this.bb_pos, 26);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  vf64_Array() {
    const offset = this.bb.__offset(this.bb_pos, 26);
    return offset ? new Float64Array(this.bb.bytes().buffer, this.bb.bytes().byteOffset + this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : null;
  }
  static getFullyQualifiedName() {
    return "MyGame.Example.TypeAliases";
  }
  static startTypeAliases(builder) {
    builder.startObject(12);
  }
  static add_i8(builder, i8) {
    builder.addFieldInt8(0, i8, 0);
  }
  static add_u8(builder, u8) {
    builder.addFieldInt8(1, u8, 0);
  }
  static add_i16(builder, i16) {
    builder.addFieldInt16(2, i16, 0);
  }
  static add_u16(builder, u16) {
    builder.addFieldInt16(3, u16, 0);
  }
  static add_i32(builder, i32) {
    builder.addFieldInt32(4, i32, 0);
  }
  static add_u32(builder, u32) {
    builder.addFieldInt32(5, u32, 0);
  }
  static add_i64(builder, i64) {
    builder.addFieldInt64(6, i64, BigInt("0"));
  }
  static add_u64(builder, u64) {
    builder.addFieldInt64(7, u64, BigInt("0"));
  }
  static add_f32(builder, f32) {
    builder.addFieldFloat32(8, f32, 0);
  }
  static add_f64(builder, f64) {
    builder.addFieldFloat64(9, f64, 0);
  }
  static add_v8(builder, v8Offset) {
    builder.addFieldOffset(10, v8Offset, 0);
  }
  static create_v8_Vector(builder, data) {
    builder.startVector(1, data.length, 1);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addInt8(data[i]);
    }
    return builder.endVector();
  }
  static start_v8_Vector(builder, numElems) {
    builder.startVector(1, numElems, 1);
  }
  static add_vf64(builder, vf64Offset) {
    builder.addFieldOffset(11, vf64Offset, 0);
  }
  static create_vf64_Vector(builder, data) {
    builder.startVector(8, data.length, 8);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addFloat64(data[i]);
    }
    return builder.endVector();
  }
  static start_vf64_Vector(builder, numElems) {
    builder.startVector(8, numElems, 8);
  }
  static endTypeAliases(builder) {
    const offset = builder.endObject();
    return offset;
  }
  static createTypeAliases(builder, i8, u8, i16, u16, i32, u32, i64, u64, f32, f64, v8Offset, vf64Offset) {
    _TypeAliases.startTypeAliases(builder);
    _TypeAliases.add_i8(builder, i8);
    _TypeAliases.add_u8(builder, u8);
    _TypeAliases.add_i16(builder, i16);
    _TypeAliases.add_u16(builder, u16);
    _TypeAliases.add_i32(builder, i32);
    _TypeAliases.add_u32(builder, u32);
    _TypeAliases.add_i64(builder, i64);
    _TypeAliases.add_u64(builder, u64);
    _TypeAliases.add_f32(builder, f32);
    _TypeAliases.add_f64(builder, f64);
    _TypeAliases.add_v8(builder, v8Offset);
    _TypeAliases.add_vf64(builder, vf64Offset);
    return _TypeAliases.endTypeAliases(builder);
  }
  serialize() {
    return this.bb.bytes();
  }
  static deserialize(buffer) {
    return _TypeAliases.getRootAsTypeAliases(new flatbuffers9.ByteBuffer(buffer));
  }
  unpack() {
    return new TypeAliasesT(this.i8(), this.u8(), this.i16(), this.u16(), this.i32(), this.u32(), this.i64(), this.u64(), this.f32(), this.f64(), this.bb.createScalarList(this.v8.bind(this), this.v8_Length()), this.bb.createScalarList(this.vf64.bind(this), this.vf64_Length()));
  }
  unpackTo(_o) {
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
    _o.v8 = this.bb.createScalarList(this.v8.bind(this), this.v8_Length());
    _o.vf64 = this.bb.createScalarList(this.vf64.bind(this), this.vf64_Length());
  }
};
var TypeAliasesT = class {
  constructor(i8 = 0, u8 = 0, i16 = 0, u16 = 0, i32 = 0, u32 = 0, i64 = BigInt("0"), u64 = BigInt("0"), f32 = 0, f64 = 0, v8 = [], vf64 = []) {
    this.i8 = i8;
    this.u8 = u8;
    this.i16 = i16;
    this.u16 = u16;
    this.i32 = i32;
    this.u32 = u32;
    this.i64 = i64;
    this.u64 = u64;
    this.f32 = f32;
    this.f64 = f64;
    this.v8 = v8;
    this.vf64 = vf64;
  }
  pack(builder) {
    const v8 = TypeAliases.create_v8_Vector(builder, this.v8);
    const vf64 = TypeAliases.create_vf64_Vector(builder, this.vf64);
    return TypeAliases.createTypeAliases(builder, this.i8, this.u8, this.i16, this.u16, this.i32, this.u32, this.i64, this.u64, this.f32, this.f64, v8, vf64);
  }
};

// MyGame/Example2.js
var Example2_exports = {};
__export(Example2_exports, {
  Monster: () => Monster,
  MonsterT: () => MonsterT
});

// MyGame/OtherNameSpace.js
var OtherNameSpace_exports = {};
__export(OtherNameSpace_exports, {
  FromInclude: () => FromInclude,
  TableB: () => TableB,
  TableBT: () => TableBT,
  Unused: () => Unused,
  UnusedT: () => UnusedT
});

// MyGame/OtherNameSpace/FromInclude.js
var FromInclude;
(function(FromInclude2) {
  FromInclude2["IncludeVal"] = "0";
})(FromInclude || (FromInclude = {}));

// MyGame/OtherNameSpace/Unused.js
var Unused = class {
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
    return this.bb.readInt32(this.bb_pos);
  }
  mutate_a(value) {
    this.bb.writeInt32(this.bb_pos + 0, value);
    return true;
  }
  static getFullyQualifiedName() {
    return "MyGame.OtherNameSpace.Unused";
  }
  static sizeOf() {
    return 4;
  }
  static createUnused(builder, a) {
    builder.prep(4, 4);
    builder.writeInt32(a);
    return builder.offset();
  }
  unpack() {
    return new UnusedT(this.a());
  }
  unpackTo(_o) {
    _o.a = this.a();
  }
};
var UnusedT = class {
  constructor(a = 0) {
    this.a = a;
  }
  pack(builder) {
    return Unused.createUnused(builder, this.a);
  }
};
