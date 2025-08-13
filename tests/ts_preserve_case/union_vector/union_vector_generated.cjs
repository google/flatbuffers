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

// union_vector/union_vector.ts
var union_vector_exports = {};
__export(union_vector_exports, {
  Attacker: () => Attacker,
  AttackerT: () => AttackerT,
  BookReader: () => BookReader,
  BookReaderT: () => BookReaderT,
  Character: () => Character,
  FallingTub: () => FallingTub,
  FallingTubT: () => FallingTubT,
  Gadget: () => Gadget,
  HandFan: () => HandFan,
  HandFanT: () => HandFanT,
  Movie: () => Movie,
  MovieT: () => MovieT,
  Rapunzel: () => Rapunzel,
  RapunzelT: () => RapunzelT
});
module.exports = __toCommonJS(union_vector_exports);

// union_vector/Attacker.js
var flatbuffers = __toESM(require("flatbuffers"), 1);
var Attacker = class _Attacker {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsAttacker(bb, obj) {
    return (obj || new _Attacker()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsAttacker(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
    return (obj || new _Attacker()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  sword_attack_damage() {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : 0;
  }
  mutate_sword_attack_damage(value) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt32(this.bb_pos + offset, value);
    return true;
  }
  static getFullyQualifiedName() {
    return "Attacker";
  }
  static startAttacker(builder) {
    builder.startObject(1);
  }
  static add_sword_attack_damage(builder, sword_attack_damage) {
    builder.addFieldInt32(0, sword_attack_damage, 0);
  }
  static endAttacker(builder) {
    const offset = builder.endObject();
    return offset;
  }
  static createAttacker(builder, sword_attack_damage) {
    _Attacker.startAttacker(builder);
    _Attacker.add_sword_attack_damage(builder, sword_attack_damage);
    return _Attacker.endAttacker(builder);
  }
  unpack() {
    return new AttackerT(this.sword_attack_damage());
  }
  unpackTo(_o) {
    _o.sword_attack_damage = this.sword_attack_damage();
  }
};
var AttackerT = class {
  constructor(sword_attack_damage = 0) {
    this.sword_attack_damage = sword_attack_damage;
  }
  pack(builder) {
    return Attacker.createAttacker(builder, this.sword_attack_damage);
  }
};

// union_vector/BookReader.js
var BookReader = class {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  books_read() {
    return this.bb.readInt32(this.bb_pos);
  }
  mutate_books_read(value) {
    this.bb.writeInt32(this.bb_pos + 0, value);
    return true;
  }
  static getFullyQualifiedName() {
    return "BookReader";
  }
  static sizeOf() {
    return 4;
  }
  static createBookReader(builder, books_read) {
    builder.prep(4, 4);
    builder.writeInt32(books_read);
    return builder.offset();
  }
  unpack() {
    return new BookReaderT(this.books_read());
  }
  unpackTo(_o) {
    _o.books_read = this.books_read();
  }
};
var BookReaderT = class {
  constructor(books_read = 0) {
    this.books_read = books_read;
  }
  pack(builder) {
    return BookReader.createBookReader(builder, this.books_read);
  }
};

// union_vector/Rapunzel.js
var Rapunzel = class {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  hair_length() {
    return this.bb.readInt32(this.bb_pos);
  }
  mutate_hair_length(value) {
    this.bb.writeInt32(this.bb_pos + 0, value);
    return true;
  }
  static getFullyQualifiedName() {
    return "Rapunzel";
  }
  static sizeOf() {
    return 4;
  }
  static createRapunzel(builder, hair_length) {
    builder.prep(4, 4);
    builder.writeInt32(hair_length);
    return builder.offset();
  }
  unpack() {
    return new RapunzelT(this.hair_length());
  }
  unpackTo(_o) {
    _o.hair_length = this.hair_length();
  }
};
var RapunzelT = class {
  constructor(hair_length = 0) {
    this.hair_length = hair_length;
  }
  pack(builder) {
    return Rapunzel.createRapunzel(builder, this.hair_length);
  }
};

// union_vector/Character.js
var Character;
(function(Character2) {
  Character2[Character2["NONE"] = 0] = "NONE";
  Character2[Character2["MuLan"] = 1] = "MuLan";
  Character2[Character2["Rapunzel"] = 2] = "Rapunzel";
  Character2[Character2["Belle"] = 3] = "Belle";
  Character2[Character2["BookFan"] = 4] = "BookFan";
  Character2[Character2["Other"] = 5] = "Other";
  Character2[Character2["Unused"] = 6] = "Unused";
})(Character || (Character = {}));
function unionToCharacter(type, accessor) {
  switch (Character[type]) {
    case "NONE":
      return null;
    case "MuLan":
      return accessor(new Attacker());
    case "Rapunzel":
      return accessor(new Rapunzel());
    case "Belle":
      return accessor(new BookReader());
    case "BookFan":
      return accessor(new BookReader());
    case "Other":
      return accessor("");
    case "Unused":
      return accessor("");
    default:
      return null;
  }
}
function unionListToCharacter(type, accessor, index) {
  switch (Character[type]) {
    case "NONE":
      return null;
    case "MuLan":
      return accessor(index, new Attacker());
    case "Rapunzel":
      return accessor(index, new Rapunzel());
    case "Belle":
      return accessor(index, new BookReader());
    case "BookFan":
      return accessor(index, new BookReader());
    case "Other":
      return accessor(index, "");
    case "Unused":
      return accessor(index, "");
    default:
      return null;
  }
}

// union_vector/FallingTub.js
var FallingTub = class {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  weight() {
    return this.bb.readInt32(this.bb_pos);
  }
  mutate_weight(value) {
    this.bb.writeInt32(this.bb_pos + 0, value);
    return true;
  }
  static getFullyQualifiedName() {
    return "FallingTub";
  }
  static sizeOf() {
    return 4;
  }
  static createFallingTub(builder, weight) {
    builder.prep(4, 4);
    builder.writeInt32(weight);
    return builder.offset();
  }
  unpack() {
    return new FallingTubT(this.weight());
  }
  unpackTo(_o) {
    _o.weight = this.weight();
  }
};
var FallingTubT = class {
  constructor(weight = 0) {
    this.weight = weight;
  }
  pack(builder) {
    return FallingTub.createFallingTub(builder, this.weight);
  }
};

// union_vector/HandFan.js
var flatbuffers2 = __toESM(require("flatbuffers"), 1);
var HandFan = class _HandFan {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsHandFan(bb, obj) {
    return (obj || new _HandFan()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsHandFan(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers2.SIZE_PREFIX_LENGTH);
    return (obj || new _HandFan()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  length() {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : 0;
  }
  mutate_length(value) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt32(this.bb_pos + offset, value);
    return true;
  }
  static getFullyQualifiedName() {
    return "HandFan";
  }
  static startHandFan(builder) {
    builder.startObject(1);
  }
  static add_length(builder, length) {
    builder.addFieldInt32(0, length, 0);
  }
  static endHandFan(builder) {
    const offset = builder.endObject();
    return offset;
  }
  static createHandFan(builder, length) {
    _HandFan.startHandFan(builder);
    _HandFan.add_length(builder, length);
    return _HandFan.endHandFan(builder);
  }
  unpack() {
    return new HandFanT(this.length());
  }
  unpackTo(_o) {
    _o.length = this.length();
  }
};
var HandFanT = class {
  constructor(length = 0) {
    this.length = length;
  }
  pack(builder) {
    return HandFan.createHandFan(builder, this.length);
  }
};

// union_vector/Gadget.js
var Gadget;
(function(Gadget2) {
  Gadget2[Gadget2["NONE"] = 0] = "NONE";
  Gadget2[Gadget2["FallingTub"] = 1] = "FallingTub";
  Gadget2[Gadget2["HandFan"] = 2] = "HandFan";
})(Gadget || (Gadget = {}));

// union_vector/Movie.js
var flatbuffers3 = __toESM(require("flatbuffers"), 1);
var Movie = class _Movie {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsMovie(bb, obj) {
    return (obj || new _Movie()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsMovie(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers3.SIZE_PREFIX_LENGTH);
    return (obj || new _Movie()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static bufferHasIdentifier(bb) {
    return bb.__has_identifier("MOVI");
  }
  main_character_type() {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.readUint8(this.bb_pos + offset) : Character.NONE;
  }
  main_character(obj) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.__union_with_string(obj, this.bb_pos + offset) : null;
  }
  characters_type(index) {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? this.bb.readUint8(this.bb.__vector(this.bb_pos + offset) + index) : null;
  }
  characters_type_Length() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  characters_type_Array() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? new Uint8Array(this.bb.bytes().buffer, this.bb.bytes().byteOffset + this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : null;
  }
  characters(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? this.bb.__union_with_string(obj, this.bb.__vector(this.bb_pos + offset) + index * 4) : null;
  }
  characters_Length() {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  static getFullyQualifiedName() {
    return "Movie";
  }
  static startMovie(builder) {
    builder.startObject(4);
  }
  static add_main_character_type(builder, main_character_type) {
    builder.addFieldInt8(0, main_character_type, Character.NONE);
  }
  static add_main_character(builder, main_characterOffset) {
    builder.addFieldOffset(1, main_characterOffset, 0);
  }
  static add_characters_type(builder, characters_typeOffset) {
    builder.addFieldOffset(2, characters_typeOffset, 0);
  }
  static create_characters_type_Vector(builder, data) {
    builder.startVector(1, data.length, 1);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addInt8(data[i]);
    }
    return builder.endVector();
  }
  static start_characters_type_Vector(builder, numElems) {
    builder.startVector(1, numElems, 1);
  }
  static add_characters(builder, charactersOffset) {
    builder.addFieldOffset(3, charactersOffset, 0);
  }
  static create_characters_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_characters_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static endMovie(builder) {
    const offset = builder.endObject();
    return offset;
  }
  static finishMovieBuffer(builder, offset) {
    builder.finish(offset, "MOVI");
  }
  static finishSizePrefixedMovieBuffer(builder, offset) {
    builder.finish(offset, "MOVI", true);
  }
  static createMovie(builder, main_character_type, main_characterOffset, characters_typeOffset, charactersOffset) {
    _Movie.startMovie(builder);
    _Movie.add_main_character_type(builder, main_character_type);
    _Movie.add_main_character(builder, main_characterOffset);
    _Movie.add_characters_type(builder, characters_typeOffset);
    _Movie.add_characters(builder, charactersOffset);
    return _Movie.endMovie(builder);
  }
  unpack() {
    return new MovieT(this.main_character_type(), (() => {
      const temp = unionToCharacter(this.main_character_type(), this.main_character.bind(this));
      if (temp === null) {
        return null;
      }
      if (typeof temp === "string") {
        return temp;
      }
      return temp.unpack();
    })(), this.bb.createScalarList(this.characters_type.bind(this), this.characters_type_Length()), (() => {
      const ret = [];
      for (let targetEnumIndex = 0; targetEnumIndex < this.characters_type_Length(); ++targetEnumIndex) {
        const targetEnum = this.characters_type(targetEnumIndex);
        if (targetEnum === null || Character[targetEnum] === "NONE") {
          continue;
        }
        const temp = unionListToCharacter(targetEnum, this.characters.bind(this), targetEnumIndex);
        if (temp === null) {
          continue;
        }
        if (typeof temp === "string") {
          ret.push(temp);
          continue;
        }
        ret.push(temp.unpack());
      }
      return ret;
    })());
  }
  unpackTo(_o) {
    _o.main_character_type = this.main_character_type();
    _o.main_character = (() => {
      const temp = unionToCharacter(this.main_character_type(), this.main_character.bind(this));
      if (temp === null) {
        return null;
      }
      if (typeof temp === "string") {
        return temp;
      }
      return temp.unpack();
    })();
    _o.characters_type = this.bb.createScalarList(this.characters_type.bind(this), this.characters_type_Length());
    _o.characters = (() => {
      const ret = [];
      for (let targetEnumIndex = 0; targetEnumIndex < this.characters_type_Length(); ++targetEnumIndex) {
        const targetEnum = this.characters_type(targetEnumIndex);
        if (targetEnum === null || Character[targetEnum] === "NONE") {
          continue;
        }
        const temp = unionListToCharacter(targetEnum, this.characters.bind(this), targetEnumIndex);
        if (temp === null) {
          continue;
        }
        if (typeof temp === "string") {
          ret.push(temp);
          continue;
        }
        ret.push(temp.unpack());
      }
      return ret;
    })();
  }
};
var MovieT = class {
  constructor(main_character_type = Character.NONE, main_character = null, characters_type = [], characters = []) {
    this.main_character_type = main_character_type;
    this.main_character = main_character;
    this.characters_type = characters_type;
    this.characters = characters;
  }
  pack(builder) {
    const main_character = builder.createObjectOffset(this.main_character);
    const characters_type = Movie.create_characters_type_Vector(builder, this.characters_type);
    const characters = Movie.create_characters_Vector(builder, builder.createObjectOffsetList(this.characters));
    return Movie.createMovie(builder, this.main_character_type, main_character, characters_type, characters);
  }
};
