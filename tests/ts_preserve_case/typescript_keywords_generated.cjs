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

// typescript_keywords.ts
var typescript_keywords_exports = {};
__export(typescript_keywords_exports, {
  foobar: () => foobar_exports,
  reflection: () => reflection_exports,
  typescript: () => typescript_exports
});
module.exports = __toCommonJS(typescript_keywords_exports);

// foobar.js
var foobar_exports = {};
__export(foobar_exports, {
  Abc: () => Abc
});

// foobar/Abc.js
var Abc;
(function(Abc2) {
  Abc2[Abc2["a"] = 0] = "a";
})(Abc || (Abc = {}));

// reflection.js
var reflection_exports = {};
__export(reflection_exports, {
  AdvancedFeatures: () => AdvancedFeatures,
  BaseType: () => BaseType,
  Enum: () => Enum,
  EnumT: () => EnumT,
  EnumVal: () => EnumVal,
  EnumValT: () => EnumValT,
  Field: () => Field,
  FieldT: () => FieldT,
  KeyValue: () => KeyValue,
  KeyValueT: () => KeyValueT,
  Object_: () => Object_,
  RPCCall: () => RPCCall,
  RPCCallT: () => RPCCallT,
  Schema: () => Schema,
  SchemaFile: () => SchemaFile,
  SchemaFileT: () => SchemaFileT,
  SchemaT: () => SchemaT,
  Service: () => Service,
  ServiceT: () => ServiceT,
  Type: () => Type,
  TypeT: () => TypeT
});

// reflection/AdvancedFeatures.js
var AdvancedFeatures;
(function(AdvancedFeatures2) {
  AdvancedFeatures2["AdvancedArrayFeatures"] = "1";
  AdvancedFeatures2["AdvancedUnionFeatures"] = "2";
  AdvancedFeatures2["OptionalScalars"] = "4";
  AdvancedFeatures2["DefaultVectorsAndStrings"] = "8";
})(AdvancedFeatures || (AdvancedFeatures = {}));

// reflection/BaseType.js
var BaseType;
(function(BaseType2) {
  BaseType2[BaseType2["None"] = 0] = "None";
  BaseType2[BaseType2["UType"] = 1] = "UType";
  BaseType2[BaseType2["Bool"] = 2] = "Bool";
  BaseType2[BaseType2["Byte"] = 3] = "Byte";
  BaseType2[BaseType2["UByte"] = 4] = "UByte";
  BaseType2[BaseType2["Short"] = 5] = "Short";
  BaseType2[BaseType2["UShort"] = 6] = "UShort";
  BaseType2[BaseType2["Int"] = 7] = "Int";
  BaseType2[BaseType2["UInt"] = 8] = "UInt";
  BaseType2[BaseType2["Long"] = 9] = "Long";
  BaseType2[BaseType2["ULong"] = 10] = "ULong";
  BaseType2[BaseType2["Float"] = 11] = "Float";
  BaseType2[BaseType2["Double"] = 12] = "Double";
  BaseType2[BaseType2["String"] = 13] = "String";
  BaseType2[BaseType2["Vector"] = 14] = "Vector";
  BaseType2[BaseType2["Obj"] = 15] = "Obj";
  BaseType2[BaseType2["Union"] = 16] = "Union";
  BaseType2[BaseType2["Array"] = 17] = "Array";
  BaseType2[BaseType2["Vector64"] = 18] = "Vector64";
  BaseType2[BaseType2["MaxBaseType"] = 19] = "MaxBaseType";
})(BaseType || (BaseType = {}));

// reflection/Enum.js
var flatbuffers4 = __toESM(require("flatbuffers"), 1);

// reflection/EnumVal.js
var flatbuffers3 = __toESM(require("flatbuffers"), 1);

// reflection/KeyValue.js
var flatbuffers = __toESM(require("flatbuffers"), 1);
var KeyValue = class _KeyValue {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsKeyValue(bb, obj) {
    return (obj || new _KeyValue()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsKeyValue(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
    return (obj || new _KeyValue()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  key(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  value(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  static getFullyQualifiedName() {
    return "reflection.KeyValue";
  }
  static startKeyValue(builder) {
    builder.startObject(2);
  }
  static add_key(builder, keyOffset) {
    builder.addFieldOffset(0, keyOffset, 0);
  }
  static add_value(builder, valueOffset) {
    builder.addFieldOffset(1, valueOffset, 0);
  }
  static endKeyValue(builder) {
    const offset = builder.endObject();
    builder.requiredField(offset, 4);
    return offset;
  }
  static createKeyValue(builder, keyOffset, valueOffset) {
    _KeyValue.startKeyValue(builder);
    _KeyValue.add_key(builder, keyOffset);
    _KeyValue.add_value(builder, valueOffset);
    return _KeyValue.endKeyValue(builder);
  }
  unpack() {
    return new KeyValueT(this.key(), this.value());
  }
  unpackTo(_o) {
    _o.key = this.key();
    _o.value = this.value();
  }
};
var KeyValueT = class {
  constructor(key = null, value = null) {
    this.key = key;
    this.value = value;
  }
  pack(builder) {
    const key = this.key !== null ? builder.createString(this.key) : 0;
    const value = this.value !== null ? builder.createString(this.value) : 0;
    return KeyValue.createKeyValue(builder, key, value);
  }
};

// reflection/Type.js
var flatbuffers2 = __toESM(require("flatbuffers"), 1);
var Type = class _Type {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsType(bb, obj) {
    return (obj || new _Type()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsType(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers2.SIZE_PREFIX_LENGTH);
    return (obj || new _Type()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  base_type() {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.readInt8(this.bb_pos + offset) : BaseType.None;
  }
  mutate_base_type(value) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt8(this.bb_pos + offset, value);
    return true;
  }
  element() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.readInt8(this.bb_pos + offset) : BaseType.None;
  }
  mutate_element(value) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt8(this.bb_pos + offset, value);
    return true;
  }
  index() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : -1;
  }
  mutate_index(value) {
    const offset = this.bb.__offset(this.bb_pos, 8);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt32(this.bb_pos + offset, value);
    return true;
  }
  fixed_length() {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? this.bb.readUint16(this.bb_pos + offset) : 0;
  }
  mutate_fixed_length(value) {
    const offset = this.bb.__offset(this.bb_pos, 10);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint16(this.bb_pos + offset, value);
    return true;
  }
  /**
   * The size (octets) of the `base_type` field.
   */
  base_size() {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? this.bb.readUint32(this.bb_pos + offset) : 4;
  }
  mutate_base_size(value) {
    const offset = this.bb.__offset(this.bb_pos, 12);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint32(this.bb_pos + offset, value);
    return true;
  }
  /**
   * The size (octets) of the `element` field, if present.
   */
  element_size() {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? this.bb.readUint32(this.bb_pos + offset) : 0;
  }
  mutate_element_size(value) {
    const offset = this.bb.__offset(this.bb_pos, 14);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint32(this.bb_pos + offset, value);
    return true;
  }
  static getFullyQualifiedName() {
    return "reflection.Type";
  }
  static startType(builder) {
    builder.startObject(6);
  }
  static add_base_type(builder, base_type) {
    builder.addFieldInt8(0, base_type, BaseType.None);
  }
  static add_element(builder, element) {
    builder.addFieldInt8(1, element, BaseType.None);
  }
  static add_index(builder, index) {
    builder.addFieldInt32(2, index, -1);
  }
  static add_fixed_length(builder, fixed_length) {
    builder.addFieldInt16(3, fixed_length, 0);
  }
  static add_base_size(builder, base_size) {
    builder.addFieldInt32(4, base_size, 4);
  }
  static add_element_size(builder, element_size) {
    builder.addFieldInt32(5, element_size, 0);
  }
  static endType(builder) {
    const offset = builder.endObject();
    return offset;
  }
  static createType(builder, base_type, element, index, fixed_length, base_size, element_size) {
    _Type.startType(builder);
    _Type.add_base_type(builder, base_type);
    _Type.add_element(builder, element);
    _Type.add_index(builder, index);
    _Type.add_fixed_length(builder, fixed_length);
    _Type.add_base_size(builder, base_size);
    _Type.add_element_size(builder, element_size);
    return _Type.endType(builder);
  }
  unpack() {
    return new TypeT(this.base_type(), this.element(), this.index(), this.fixed_length(), this.base_size(), this.element_size());
  }
  unpackTo(_o) {
    _o.base_type = this.base_type();
    _o.element = this.element();
    _o.index = this.index();
    _o.fixed_length = this.fixed_length();
    _o.base_size = this.base_size();
    _o.element_size = this.element_size();
  }
};
var TypeT = class {
  constructor(base_type = BaseType.None, element = BaseType.None, index = -1, fixed_length = 0, base_size = 4, element_size = 0) {
    this.base_type = base_type;
    this.element = element;
    this.index = index;
    this.fixed_length = fixed_length;
    this.base_size = base_size;
    this.element_size = element_size;
  }
  pack(builder) {
    return Type.createType(builder, this.base_type, this.element, this.index, this.fixed_length, this.base_size, this.element_size);
  }
};

// reflection/EnumVal.js
var EnumVal = class _EnumVal {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsEnumVal(bb, obj) {
    return (obj || new _EnumVal()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsEnumVal(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers3.SIZE_PREFIX_LENGTH);
    return (obj || new _EnumVal()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  name(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  value() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.readInt64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_value(value) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt64(this.bb_pos + offset, value);
    return true;
  }
  union_type(obj) {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? (obj || new Type()).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : null;
  }
  documentation(index, optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? this.bb.__string(this.bb.__vector(this.bb_pos + offset) + index * 4, optionalEncoding) : null;
  }
  documentation_Length() {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  attributes(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? (obj || new KeyValue()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  attributes_Length() {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  static getFullyQualifiedName() {
    return "reflection.EnumVal";
  }
  static startEnumVal(builder) {
    builder.startObject(6);
  }
  static add_name(builder, nameOffset) {
    builder.addFieldOffset(0, nameOffset, 0);
  }
  static add_value(builder, value) {
    builder.addFieldInt64(1, value, BigInt("0"));
  }
  static add_union_type(builder, union_typeOffset) {
    builder.addFieldOffset(3, union_typeOffset, 0);
  }
  static add_documentation(builder, documentationOffset) {
    builder.addFieldOffset(4, documentationOffset, 0);
  }
  static create_documentation_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_documentation_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_attributes(builder, attributesOffset) {
    builder.addFieldOffset(5, attributesOffset, 0);
  }
  static create_attributes_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_attributes_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static endEnumVal(builder) {
    const offset = builder.endObject();
    builder.requiredField(offset, 4);
    return offset;
  }
  unpack() {
    return new EnumValT(this.name(), this.value(), this.union_type() !== null ? this.union_type().unpack() : null, this.bb.createScalarList(this.documentation.bind(this), this.documentation_Length()), this.bb.createObjList(this.attributes.bind(this), this.attributes_Length()));
  }
  unpackTo(_o) {
    _o.name = this.name();
    _o.value = this.value();
    _o.union_type = this.union_type() !== null ? this.union_type().unpack() : null;
    _o.documentation = this.bb.createScalarList(this.documentation.bind(this), this.documentation_Length());
    _o.attributes = this.bb.createObjList(this.attributes.bind(this), this.attributes_Length());
  }
};
var EnumValT = class {
  constructor(name = null, value = BigInt("0"), union_type = null, documentation = [], attributes = []) {
    this.name = name;
    this.value = value;
    this.union_type = union_type;
    this.documentation = documentation;
    this.attributes = attributes;
  }
  pack(builder) {
    const name = this.name !== null ? builder.createString(this.name) : 0;
    const union_type = this.union_type !== null ? this.union_type.pack(builder) : 0;
    const documentation = EnumVal.create_documentation_Vector(builder, builder.createObjectOffsetList(this.documentation));
    const attributes = EnumVal.create_attributes_Vector(builder, builder.createObjectOffsetList(this.attributes));
    EnumVal.startEnumVal(builder);
    EnumVal.add_name(builder, name);
    EnumVal.add_value(builder, this.value);
    EnumVal.add_union_type(builder, union_type);
    EnumVal.add_documentation(builder, documentation);
    EnumVal.add_attributes(builder, attributes);
    return EnumVal.endEnumVal(builder);
  }
};

// reflection/Enum.js
var Enum = class _Enum {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsEnum(bb, obj) {
    return (obj || new _Enum()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsEnum(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers4.SIZE_PREFIX_LENGTH);
    return (obj || new _Enum()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  name(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  values(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? (obj || new EnumVal()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  values_Length() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  is_union() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? !!this.bb.readInt8(this.bb_pos + offset) : false;
  }
  mutate_is_union(value) {
    const offset = this.bb.__offset(this.bb_pos, 8);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt8(this.bb_pos + offset, +value);
    return true;
  }
  underlying_type(obj) {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? (obj || new Type()).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : null;
  }
  attributes(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? (obj || new KeyValue()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  attributes_Length() {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  documentation(index, optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? this.bb.__string(this.bb.__vector(this.bb_pos + offset) + index * 4, optionalEncoding) : null;
  }
  documentation_Length() {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  declaration_file(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 16);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  static getFullyQualifiedName() {
    return "reflection.Enum";
  }
  static startEnum(builder) {
    builder.startObject(7);
  }
  static add_name(builder, nameOffset) {
    builder.addFieldOffset(0, nameOffset, 0);
  }
  static add_values(builder, valuesOffset) {
    builder.addFieldOffset(1, valuesOffset, 0);
  }
  static create_values_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_values_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_is_union(builder, is_union) {
    builder.addFieldInt8(2, +is_union, 0);
  }
  static add_underlying_type(builder, underlying_typeOffset) {
    builder.addFieldOffset(3, underlying_typeOffset, 0);
  }
  static add_attributes(builder, attributesOffset) {
    builder.addFieldOffset(4, attributesOffset, 0);
  }
  static create_attributes_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_attributes_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_documentation(builder, documentationOffset) {
    builder.addFieldOffset(5, documentationOffset, 0);
  }
  static create_documentation_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_documentation_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_declaration_file(builder, declaration_fileOffset) {
    builder.addFieldOffset(6, declaration_fileOffset, 0);
  }
  static endEnum(builder) {
    const offset = builder.endObject();
    builder.requiredField(offset, 4);
    builder.requiredField(offset, 6);
    builder.requiredField(offset, 10);
    return offset;
  }
  unpack() {
    return new EnumT(this.name(), this.bb.createObjList(this.values.bind(this), this.values_Length()), this.is_union(), this.underlying_type() !== null ? this.underlying_type().unpack() : null, this.bb.createObjList(this.attributes.bind(this), this.attributes_Length()), this.bb.createScalarList(this.documentation.bind(this), this.documentation_Length()), this.declaration_file());
  }
  unpackTo(_o) {
    _o.name = this.name();
    _o.values = this.bb.createObjList(this.values.bind(this), this.values_Length());
    _o.is_union = this.is_union();
    _o.underlying_type = this.underlying_type() !== null ? this.underlying_type().unpack() : null;
    _o.attributes = this.bb.createObjList(this.attributes.bind(this), this.attributes_Length());
    _o.documentation = this.bb.createScalarList(this.documentation.bind(this), this.documentation_Length());
    _o.declaration_file = this.declaration_file();
  }
};
var EnumT = class {
  constructor(name = null, values = [], is_union = false, underlying_type = null, attributes = [], documentation = [], declaration_file = null) {
    this.name = name;
    this.values = values;
    this.is_union = is_union;
    this.underlying_type = underlying_type;
    this.attributes = attributes;
    this.documentation = documentation;
    this.declaration_file = declaration_file;
  }
  pack(builder) {
    const name = this.name !== null ? builder.createString(this.name) : 0;
    const values = Enum.create_values_Vector(builder, builder.createObjectOffsetList(this.values));
    const underlying_type = this.underlying_type !== null ? this.underlying_type.pack(builder) : 0;
    const attributes = Enum.create_attributes_Vector(builder, builder.createObjectOffsetList(this.attributes));
    const documentation = Enum.create_documentation_Vector(builder, builder.createObjectOffsetList(this.documentation));
    const declaration_file = this.declaration_file !== null ? builder.createString(this.declaration_file) : 0;
    Enum.startEnum(builder);
    Enum.add_name(builder, name);
    Enum.add_values(builder, values);
    Enum.add_is_union(builder, this.is_union);
    Enum.add_underlying_type(builder, underlying_type);
    Enum.add_attributes(builder, attributes);
    Enum.add_documentation(builder, documentation);
    Enum.add_declaration_file(builder, declaration_file);
    return Enum.endEnum(builder);
  }
};

// reflection/Field.js
var flatbuffers5 = __toESM(require("flatbuffers"), 1);
var Field = class _Field {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsField(bb, obj) {
    return (obj || new _Field()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsField(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers5.SIZE_PREFIX_LENGTH);
    return (obj || new _Field()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  name(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  type(obj) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? (obj || new Type()).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : null;
  }
  id() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? this.bb.readUint16(this.bb_pos + offset) : 0;
  }
  mutate_id(value) {
    const offset = this.bb.__offset(this.bb_pos, 8);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint16(this.bb_pos + offset, value);
    return true;
  }
  offset() {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? this.bb.readUint16(this.bb_pos + offset) : 0;
  }
  mutate_offset(value) {
    const offset = this.bb.__offset(this.bb_pos, 10);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint16(this.bb_pos + offset, value);
    return true;
  }
  default_integer() {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? this.bb.readInt64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_default_integer(value) {
    const offset = this.bb.__offset(this.bb_pos, 12);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt64(this.bb_pos + offset, value);
    return true;
  }
  default_real() {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? this.bb.readFloat64(this.bb_pos + offset) : 0;
  }
  mutate_default_real(value) {
    const offset = this.bb.__offset(this.bb_pos, 14);
    if (offset === 0) {
      return false;
    }
    this.bb.writeFloat64(this.bb_pos + offset, value);
    return true;
  }
  deprecated() {
    const offset = this.bb.__offset(this.bb_pos, 16);
    return offset ? !!this.bb.readInt8(this.bb_pos + offset) : false;
  }
  mutate_deprecated(value) {
    const offset = this.bb.__offset(this.bb_pos, 16);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt8(this.bb_pos + offset, +value);
    return true;
  }
  required() {
    const offset = this.bb.__offset(this.bb_pos, 18);
    return offset ? !!this.bb.readInt8(this.bb_pos + offset) : false;
  }
  mutate_required(value) {
    const offset = this.bb.__offset(this.bb_pos, 18);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt8(this.bb_pos + offset, +value);
    return true;
  }
  key() {
    const offset = this.bb.__offset(this.bb_pos, 20);
    return offset ? !!this.bb.readInt8(this.bb_pos + offset) : false;
  }
  mutate_key(value) {
    const offset = this.bb.__offset(this.bb_pos, 20);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt8(this.bb_pos + offset, +value);
    return true;
  }
  attributes(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 22);
    return offset ? (obj || new KeyValue()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  attributes_Length() {
    const offset = this.bb.__offset(this.bb_pos, 22);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  documentation(index, optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 24);
    return offset ? this.bb.__string(this.bb.__vector(this.bb_pos + offset) + index * 4, optionalEncoding) : null;
  }
  documentation_Length() {
    const offset = this.bb.__offset(this.bb_pos, 24);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  optional() {
    const offset = this.bb.__offset(this.bb_pos, 26);
    return offset ? !!this.bb.readInt8(this.bb_pos + offset) : false;
  }
  mutate_optional(value) {
    const offset = this.bb.__offset(this.bb_pos, 26);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt8(this.bb_pos + offset, +value);
    return true;
  }
  /**
   * Number of padding octets to always add after this field. Structs only.
   */
  padding() {
    const offset = this.bb.__offset(this.bb_pos, 28);
    return offset ? this.bb.readUint16(this.bb_pos + offset) : 0;
  }
  mutate_padding(value) {
    const offset = this.bb.__offset(this.bb_pos, 28);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint16(this.bb_pos + offset, value);
    return true;
  }
  /**
   * If the field uses 64-bit offsets.
   */
  offset64() {
    const offset = this.bb.__offset(this.bb_pos, 30);
    return offset ? !!this.bb.readInt8(this.bb_pos + offset) : false;
  }
  mutate_offset64(value) {
    const offset = this.bb.__offset(this.bb_pos, 30);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt8(this.bb_pos + offset, +value);
    return true;
  }
  static getFullyQualifiedName() {
    return "reflection.Field";
  }
  static startField(builder) {
    builder.startObject(14);
  }
  static add_name(builder, nameOffset) {
    builder.addFieldOffset(0, nameOffset, 0);
  }
  static add_type(builder, typeOffset) {
    builder.addFieldOffset(1, typeOffset, 0);
  }
  static add_id(builder, id) {
    builder.addFieldInt16(2, id, 0);
  }
  static add_offset(builder, offset) {
    builder.addFieldInt16(3, offset, 0);
  }
  static add_default_integer(builder, default_integer) {
    builder.addFieldInt64(4, default_integer, BigInt("0"));
  }
  static add_default_real(builder, default_real) {
    builder.addFieldFloat64(5, default_real, 0);
  }
  static add_deprecated(builder, deprecated) {
    builder.addFieldInt8(6, +deprecated, 0);
  }
  static add_required(builder, required) {
    builder.addFieldInt8(7, +required, 0);
  }
  static add_key(builder, key) {
    builder.addFieldInt8(8, +key, 0);
  }
  static add_attributes(builder, attributesOffset) {
    builder.addFieldOffset(9, attributesOffset, 0);
  }
  static create_attributes_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_attributes_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_documentation(builder, documentationOffset) {
    builder.addFieldOffset(10, documentationOffset, 0);
  }
  static create_documentation_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_documentation_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_optional(builder, optional) {
    builder.addFieldInt8(11, +optional, 0);
  }
  static add_padding(builder, padding) {
    builder.addFieldInt16(12, padding, 0);
  }
  static add_offset64(builder, offset64) {
    builder.addFieldInt8(13, +offset64, 0);
  }
  static endField(builder) {
    const offset = builder.endObject();
    builder.requiredField(offset, 4);
    builder.requiredField(offset, 6);
    return offset;
  }
  unpack() {
    return new FieldT(this.name(), this.type() !== null ? this.type().unpack() : null, this.id(), this.offset(), this.default_integer(), this.default_real(), this.deprecated(), this.required(), this.key(), this.bb.createObjList(this.attributes.bind(this), this.attributes_Length()), this.bb.createScalarList(this.documentation.bind(this), this.documentation_Length()), this.optional(), this.padding(), this.offset64());
  }
  unpackTo(_o) {
    _o.name = this.name();
    _o.type = this.type() !== null ? this.type().unpack() : null;
    _o.id = this.id();
    _o.offset = this.offset();
    _o.default_integer = this.default_integer();
    _o.default_real = this.default_real();
    _o.deprecated = this.deprecated();
    _o.required = this.required();
    _o.key = this.key();
    _o.attributes = this.bb.createObjList(this.attributes.bind(this), this.attributes_Length());
    _o.documentation = this.bb.createScalarList(this.documentation.bind(this), this.documentation_Length());
    _o.optional = this.optional();
    _o.padding = this.padding();
    _o.offset64 = this.offset64();
  }
};
var FieldT = class {
  constructor(name = null, type = null, id = 0, offset = 0, default_integer = BigInt("0"), default_real = 0, deprecated = false, required = false, key = false, attributes = [], documentation = [], optional = false, padding = 0, offset64 = false) {
    this.name = name;
    this.type = type;
    this.id = id;
    this.offset = offset;
    this.default_integer = default_integer;
    this.default_real = default_real;
    this.deprecated = deprecated;
    this.required = required;
    this.key = key;
    this.attributes = attributes;
    this.documentation = documentation;
    this.optional = optional;
    this.padding = padding;
    this.offset64 = offset64;
  }
  pack(builder) {
    const name = this.name !== null ? builder.createString(this.name) : 0;
    const type = this.type !== null ? this.type.pack(builder) : 0;
    const attributes = Field.create_attributes_Vector(builder, builder.createObjectOffsetList(this.attributes));
    const documentation = Field.create_documentation_Vector(builder, builder.createObjectOffsetList(this.documentation));
    Field.startField(builder);
    Field.add_name(builder, name);
    Field.add_type(builder, type);
    Field.add_id(builder, this.id);
    Field.add_offset(builder, this.offset);
    Field.add_default_integer(builder, this.default_integer);
    Field.add_default_real(builder, this.default_real);
    Field.add_deprecated(builder, this.deprecated);
    Field.add_required(builder, this.required);
    Field.add_key(builder, this.key);
    Field.add_attributes(builder, attributes);
    Field.add_documentation(builder, documentation);
    Field.add_optional(builder, this.optional);
    Field.add_padding(builder, this.padding);
    Field.add_offset64(builder, this.offset64);
    return Field.endField(builder);
  }
};

// reflection/Object.js
var flatbuffers6 = __toESM(require("flatbuffers"), 1);
var Object_ = class _Object_ {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsObject(bb, obj) {
    return (obj || new _Object_()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsObject(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers6.SIZE_PREFIX_LENGTH);
    return (obj || new _Object_()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  name(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  fields(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? (obj || new Field()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  fields_Length() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  is_struct() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? !!this.bb.readInt8(this.bb_pos + offset) : false;
  }
  mutate_is_struct(value) {
    const offset = this.bb.__offset(this.bb_pos, 8);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt8(this.bb_pos + offset, +value);
    return true;
  }
  minalign() {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : 0;
  }
  mutate_minalign(value) {
    const offset = this.bb.__offset(this.bb_pos, 10);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt32(this.bb_pos + offset, value);
    return true;
  }
  bytesize() {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : 0;
  }
  mutate_bytesize(value) {
    const offset = this.bb.__offset(this.bb_pos, 12);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt32(this.bb_pos + offset, value);
    return true;
  }
  attributes(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? (obj || new KeyValue()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  attributes_Length() {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  documentation(index, optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 16);
    return offset ? this.bb.__string(this.bb.__vector(this.bb_pos + offset) + index * 4, optionalEncoding) : null;
  }
  documentation_Length() {
    const offset = this.bb.__offset(this.bb_pos, 16);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  declaration_file(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 18);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  static getFullyQualifiedName() {
    return "reflection.Object";
  }
  static startObject(builder) {
    builder.startObject(8);
  }
  static add_name(builder, nameOffset) {
    builder.addFieldOffset(0, nameOffset, 0);
  }
  static add_fields(builder, fieldsOffset) {
    builder.addFieldOffset(1, fieldsOffset, 0);
  }
  static create_fields_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_fields_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_is_struct(builder, is_struct) {
    builder.addFieldInt8(2, +is_struct, 0);
  }
  static add_minalign(builder, minalign) {
    builder.addFieldInt32(3, minalign, 0);
  }
  static add_bytesize(builder, bytesize) {
    builder.addFieldInt32(4, bytesize, 0);
  }
  static add_attributes(builder, attributesOffset) {
    builder.addFieldOffset(5, attributesOffset, 0);
  }
  static create_attributes_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_attributes_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_documentation(builder, documentationOffset) {
    builder.addFieldOffset(6, documentationOffset, 0);
  }
  static create_documentation_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_documentation_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_declaration_file(builder, declaration_fileOffset) {
    builder.addFieldOffset(7, declaration_fileOffset, 0);
  }
  static endObject(builder) {
    const offset = builder.endObject();
    builder.requiredField(offset, 4);
    builder.requiredField(offset, 6);
    return offset;
  }
  static createObject(builder, nameOffset, fieldsOffset, is_struct, minalign, bytesize, attributesOffset, documentationOffset, declaration_fileOffset) {
    _Object_.startObject(builder);
    _Object_.add_name(builder, nameOffset);
    _Object_.add_fields(builder, fieldsOffset);
    _Object_.add_is_struct(builder, is_struct);
    _Object_.add_minalign(builder, minalign);
    _Object_.add_bytesize(builder, bytesize);
    _Object_.add_attributes(builder, attributesOffset);
    _Object_.add_documentation(builder, documentationOffset);
    _Object_.add_declaration_file(builder, declaration_fileOffset);
    return _Object_.endObject(builder);
  }
  unpack() {
    return new Object_T(this.name(), this.bb.createObjList(this.fields.bind(this), this.fields_Length()), this.is_struct(), this.minalign(), this.bytesize(), this.bb.createObjList(this.attributes.bind(this), this.attributes_Length()), this.bb.createScalarList(this.documentation.bind(this), this.documentation_Length()), this.declaration_file());
  }
  unpackTo(_o) {
    _o.name = this.name();
    _o.fields = this.bb.createObjList(this.fields.bind(this), this.fields_Length());
    _o.is_struct = this.is_struct();
    _o.minalign = this.minalign();
    _o.bytesize = this.bytesize();
    _o.attributes = this.bb.createObjList(this.attributes.bind(this), this.attributes_Length());
    _o.documentation = this.bb.createScalarList(this.documentation.bind(this), this.documentation_Length());
    _o.declaration_file = this.declaration_file();
  }
};
var Object_T = class {
  constructor(name = null, fields = [], is_struct = false, minalign = 0, bytesize = 0, attributes = [], documentation = [], declaration_file = null) {
    this.name = name;
    this.fields = fields;
    this.is_struct = is_struct;
    this.minalign = minalign;
    this.bytesize = bytesize;
    this.attributes = attributes;
    this.documentation = documentation;
    this.declaration_file = declaration_file;
  }
  pack(builder) {
    const name = this.name !== null ? builder.createString(this.name) : 0;
    const fields = Object_.create_fields_Vector(builder, builder.createObjectOffsetList(this.fields));
    const attributes = Object_.create_attributes_Vector(builder, builder.createObjectOffsetList(this.attributes));
    const documentation = Object_.create_documentation_Vector(builder, builder.createObjectOffsetList(this.documentation));
    const declaration_file = this.declaration_file !== null ? builder.createString(this.declaration_file) : 0;
    return Object_.createObject(builder, name, fields, this.is_struct, this.minalign, this.bytesize, attributes, documentation, declaration_file);
  }
};

// reflection/RPCCall.js
var flatbuffers7 = __toESM(require("flatbuffers"), 1);
var RPCCall = class _RPCCall {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsRPCCall(bb, obj) {
    return (obj || new _RPCCall()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsRPCCall(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers7.SIZE_PREFIX_LENGTH);
    return (obj || new _RPCCall()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  name(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  request(obj) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? (obj || new Object_()).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : null;
  }
  response(obj) {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? (obj || new Object_()).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : null;
  }
  attributes(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? (obj || new KeyValue()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  attributes_Length() {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  documentation(index, optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? this.bb.__string(this.bb.__vector(this.bb_pos + offset) + index * 4, optionalEncoding) : null;
  }
  documentation_Length() {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  static getFullyQualifiedName() {
    return "reflection.RPCCall";
  }
  static startRPCCall(builder) {
    builder.startObject(5);
  }
  static add_name(builder, nameOffset) {
    builder.addFieldOffset(0, nameOffset, 0);
  }
  static add_request(builder, requestOffset) {
    builder.addFieldOffset(1, requestOffset, 0);
  }
  static add_response(builder, responseOffset) {
    builder.addFieldOffset(2, responseOffset, 0);
  }
  static add_attributes(builder, attributesOffset) {
    builder.addFieldOffset(3, attributesOffset, 0);
  }
  static create_attributes_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_attributes_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_documentation(builder, documentationOffset) {
    builder.addFieldOffset(4, documentationOffset, 0);
  }
  static create_documentation_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_documentation_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static endRPCCall(builder) {
    const offset = builder.endObject();
    builder.requiredField(offset, 4);
    builder.requiredField(offset, 6);
    builder.requiredField(offset, 8);
    return offset;
  }
  unpack() {
    return new RPCCallT(this.name(), this.request() !== null ? this.request().unpack() : null, this.response() !== null ? this.response().unpack() : null, this.bb.createObjList(this.attributes.bind(this), this.attributes_Length()), this.bb.createScalarList(this.documentation.bind(this), this.documentation_Length()));
  }
  unpackTo(_o) {
    _o.name = this.name();
    _o.request = this.request() !== null ? this.request().unpack() : null;
    _o.response = this.response() !== null ? this.response().unpack() : null;
    _o.attributes = this.bb.createObjList(this.attributes.bind(this), this.attributes_Length());
    _o.documentation = this.bb.createScalarList(this.documentation.bind(this), this.documentation_Length());
  }
};
var RPCCallT = class {
  constructor(name = null, request = null, response = null, attributes = [], documentation = []) {
    this.name = name;
    this.request = request;
    this.response = response;
    this.attributes = attributes;
    this.documentation = documentation;
  }
  pack(builder) {
    const name = this.name !== null ? builder.createString(this.name) : 0;
    const request = this.request !== null ? this.request.pack(builder) : 0;
    const response = this.response !== null ? this.response.pack(builder) : 0;
    const attributes = RPCCall.create_attributes_Vector(builder, builder.createObjectOffsetList(this.attributes));
    const documentation = RPCCall.create_documentation_Vector(builder, builder.createObjectOffsetList(this.documentation));
    RPCCall.startRPCCall(builder);
    RPCCall.add_name(builder, name);
    RPCCall.add_request(builder, request);
    RPCCall.add_response(builder, response);
    RPCCall.add_attributes(builder, attributes);
    RPCCall.add_documentation(builder, documentation);
    return RPCCall.endRPCCall(builder);
  }
};

// reflection/Schema.js
var flatbuffers10 = __toESM(require("flatbuffers"), 1);

// reflection/SchemaFile.js
var flatbuffers8 = __toESM(require("flatbuffers"), 1);
var SchemaFile = class _SchemaFile {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsSchemaFile(bb, obj) {
    return (obj || new _SchemaFile()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsSchemaFile(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers8.SIZE_PREFIX_LENGTH);
    return (obj || new _SchemaFile()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  filename(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  included_filenames(index, optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.__string(this.bb.__vector(this.bb_pos + offset) + index * 4, optionalEncoding) : null;
  }
  included_filenames_Length() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  static getFullyQualifiedName() {
    return "reflection.SchemaFile";
  }
  static startSchemaFile(builder) {
    builder.startObject(2);
  }
  static add_filename(builder, filenameOffset) {
    builder.addFieldOffset(0, filenameOffset, 0);
  }
  static add_included_filenames(builder, included_filenamesOffset) {
    builder.addFieldOffset(1, included_filenamesOffset, 0);
  }
  static create_included_filenames_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_included_filenames_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static endSchemaFile(builder) {
    const offset = builder.endObject();
    builder.requiredField(offset, 4);
    return offset;
  }
  static createSchemaFile(builder, filenameOffset, included_filenamesOffset) {
    _SchemaFile.startSchemaFile(builder);
    _SchemaFile.add_filename(builder, filenameOffset);
    _SchemaFile.add_included_filenames(builder, included_filenamesOffset);
    return _SchemaFile.endSchemaFile(builder);
  }
  unpack() {
    return new SchemaFileT(this.filename(), this.bb.createScalarList(this.included_filenames.bind(this), this.included_filenames_Length()));
  }
  unpackTo(_o) {
    _o.filename = this.filename();
    _o.included_filenames = this.bb.createScalarList(this.included_filenames.bind(this), this.included_filenames_Length());
  }
};
var SchemaFileT = class {
  constructor(filename = null, included_filenames = []) {
    this.filename = filename;
    this.included_filenames = included_filenames;
  }
  pack(builder) {
    const filename = this.filename !== null ? builder.createString(this.filename) : 0;
    const included_filenames = SchemaFile.create_included_filenames_Vector(builder, builder.createObjectOffsetList(this.included_filenames));
    return SchemaFile.createSchemaFile(builder, filename, included_filenames);
  }
};

// reflection/Service.js
var flatbuffers9 = __toESM(require("flatbuffers"), 1);
var Service = class _Service {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsService(bb, obj) {
    return (obj || new _Service()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsService(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers9.SIZE_PREFIX_LENGTH);
    return (obj || new _Service()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  name(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  calls(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? (obj || new RPCCall()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  calls_Length() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  attributes(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? (obj || new KeyValue()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  attributes_Length() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  documentation(index, optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? this.bb.__string(this.bb.__vector(this.bb_pos + offset) + index * 4, optionalEncoding) : null;
  }
  documentation_Length() {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  declaration_file(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  static getFullyQualifiedName() {
    return "reflection.Service";
  }
  static startService(builder) {
    builder.startObject(5);
  }
  static add_name(builder, nameOffset) {
    builder.addFieldOffset(0, nameOffset, 0);
  }
  static add_calls(builder, callsOffset) {
    builder.addFieldOffset(1, callsOffset, 0);
  }
  static create_calls_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_calls_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_attributes(builder, attributesOffset) {
    builder.addFieldOffset(2, attributesOffset, 0);
  }
  static create_attributes_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_attributes_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_documentation(builder, documentationOffset) {
    builder.addFieldOffset(3, documentationOffset, 0);
  }
  static create_documentation_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_documentation_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_declaration_file(builder, declaration_fileOffset) {
    builder.addFieldOffset(4, declaration_fileOffset, 0);
  }
  static endService(builder) {
    const offset = builder.endObject();
    builder.requiredField(offset, 4);
    return offset;
  }
  static createService(builder, nameOffset, callsOffset, attributesOffset, documentationOffset, declaration_fileOffset) {
    _Service.startService(builder);
    _Service.add_name(builder, nameOffset);
    _Service.add_calls(builder, callsOffset);
    _Service.add_attributes(builder, attributesOffset);
    _Service.add_documentation(builder, documentationOffset);
    _Service.add_declaration_file(builder, declaration_fileOffset);
    return _Service.endService(builder);
  }
  unpack() {
    return new ServiceT(this.name(), this.bb.createObjList(this.calls.bind(this), this.calls_Length()), this.bb.createObjList(this.attributes.bind(this), this.attributes_Length()), this.bb.createScalarList(this.documentation.bind(this), this.documentation_Length()), this.declaration_file());
  }
  unpackTo(_o) {
    _o.name = this.name();
    _o.calls = this.bb.createObjList(this.calls.bind(this), this.calls_Length());
    _o.attributes = this.bb.createObjList(this.attributes.bind(this), this.attributes_Length());
    _o.documentation = this.bb.createScalarList(this.documentation.bind(this), this.documentation_Length());
    _o.declaration_file = this.declaration_file();
  }
};
var ServiceT = class {
  constructor(name = null, calls = [], attributes = [], documentation = [], declaration_file = null) {
    this.name = name;
    this.calls = calls;
    this.attributes = attributes;
    this.documentation = documentation;
    this.declaration_file = declaration_file;
  }
  pack(builder) {
    const name = this.name !== null ? builder.createString(this.name) : 0;
    const calls = Service.create_calls_Vector(builder, builder.createObjectOffsetList(this.calls));
    const attributes = Service.create_attributes_Vector(builder, builder.createObjectOffsetList(this.attributes));
    const documentation = Service.create_documentation_Vector(builder, builder.createObjectOffsetList(this.documentation));
    const declaration_file = this.declaration_file !== null ? builder.createString(this.declaration_file) : 0;
    return Service.createService(builder, name, calls, attributes, documentation, declaration_file);
  }
};

// reflection/Schema.js
var Schema = class _Schema {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsSchema(bb, obj) {
    return (obj || new _Schema()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsSchema(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers10.SIZE_PREFIX_LENGTH);
    return (obj || new _Schema()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static bufferHasIdentifier(bb) {
    return bb.__has_identifier("BFBS");
  }
  objects(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? (obj || new Object_()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  objects_Length() {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  enums(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? (obj || new Enum()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  enums_Length() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  file_ident(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  file_ext(optionalEncoding) {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
  }
  root_table(obj) {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? (obj || new Object_()).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : null;
  }
  services(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? (obj || new Service()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  services_Length() {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  advanced_features() {
    const offset = this.bb.__offset(this.bb_pos, 16);
    return offset ? this.bb.readUint64(this.bb_pos + offset) : BigInt("0");
  }
  mutate_advanced_features(value) {
    const offset = this.bb.__offset(this.bb_pos, 16);
    if (offset === 0) {
      return false;
    }
    this.bb.writeUint64(this.bb_pos + offset, value);
    return true;
  }
  /**
   * All the files used in this compilation. Files are relative to where
   * flatc was invoked.
   */
  fbs_files(index, obj) {
    const offset = this.bb.__offset(this.bb_pos, 18);
    return offset ? (obj || new SchemaFile()).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
  }
  fbs_files_Length() {
    const offset = this.bb.__offset(this.bb_pos, 18);
    return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
  }
  static getFullyQualifiedName() {
    return "reflection.Schema";
  }
  static startSchema(builder) {
    builder.startObject(8);
  }
  static add_objects(builder, objectsOffset) {
    builder.addFieldOffset(0, objectsOffset, 0);
  }
  static create_objects_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_objects_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_enums(builder, enumsOffset) {
    builder.addFieldOffset(1, enumsOffset, 0);
  }
  static create_enums_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_enums_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_file_ident(builder, file_identOffset) {
    builder.addFieldOffset(2, file_identOffset, 0);
  }
  static add_file_ext(builder, file_extOffset) {
    builder.addFieldOffset(3, file_extOffset, 0);
  }
  static add_root_table(builder, root_tableOffset) {
    builder.addFieldOffset(4, root_tableOffset, 0);
  }
  static add_services(builder, servicesOffset) {
    builder.addFieldOffset(5, servicesOffset, 0);
  }
  static create_services_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_services_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static add_advanced_features(builder, advanced_features) {
    builder.addFieldInt64(6, advanced_features, BigInt("0"));
  }
  static add_fbs_files(builder, fbs_filesOffset) {
    builder.addFieldOffset(7, fbs_filesOffset, 0);
  }
  static create_fbs_files_Vector(builder, data) {
    builder.startVector(4, data.length, 4);
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]);
    }
    return builder.endVector();
  }
  static start_fbs_files_Vector(builder, numElems) {
    builder.startVector(4, numElems, 4);
  }
  static endSchema(builder) {
    const offset = builder.endObject();
    builder.requiredField(offset, 4);
    builder.requiredField(offset, 6);
    return offset;
  }
  static finishSchemaBuffer(builder, offset) {
    builder.finish(offset, "BFBS");
  }
  static finishSizePrefixedSchemaBuffer(builder, offset) {
    builder.finish(offset, "BFBS", true);
  }
  unpack() {
    return new SchemaT(this.bb.createObjList(this.objects.bind(this), this.objects_Length()), this.bb.createObjList(this.enums.bind(this), this.enums_Length()), this.file_ident(), this.file_ext(), this.root_table() !== null ? this.root_table().unpack() : null, this.bb.createObjList(this.services.bind(this), this.services_Length()), this.advanced_features(), this.bb.createObjList(this.fbs_files.bind(this), this.fbs_files_Length()));
  }
  unpackTo(_o) {
    _o.objects = this.bb.createObjList(this.objects.bind(this), this.objects_Length());
    _o.enums = this.bb.createObjList(this.enums.bind(this), this.enums_Length());
    _o.file_ident = this.file_ident();
    _o.file_ext = this.file_ext();
    _o.root_table = this.root_table() !== null ? this.root_table().unpack() : null;
    _o.services = this.bb.createObjList(this.services.bind(this), this.services_Length());
    _o.advanced_features = this.advanced_features();
    _o.fbs_files = this.bb.createObjList(this.fbs_files.bind(this), this.fbs_files_Length());
  }
};
var SchemaT = class {
  constructor(objects = [], enums = [], file_ident = null, file_ext = null, root_table = null, services = [], advanced_features = BigInt("0"), fbs_files = []) {
    this.objects = objects;
    this.enums = enums;
    this.file_ident = file_ident;
    this.file_ext = file_ext;
    this.root_table = root_table;
    this.services = services;
    this.advanced_features = advanced_features;
    this.fbs_files = fbs_files;
  }
  pack(builder) {
    const objects = Schema.create_objects_Vector(builder, builder.createObjectOffsetList(this.objects));
    const enums = Schema.create_enums_Vector(builder, builder.createObjectOffsetList(this.enums));
    const file_ident = this.file_ident !== null ? builder.createString(this.file_ident) : 0;
    const file_ext = this.file_ext !== null ? builder.createString(this.file_ext) : 0;
    const root_table = this.root_table !== null ? this.root_table.pack(builder) : 0;
    const services = Schema.create_services_Vector(builder, builder.createObjectOffsetList(this.services));
    const fbs_files = Schema.create_fbs_files_Vector(builder, builder.createObjectOffsetList(this.fbs_files));
    Schema.startSchema(builder);
    Schema.add_objects(builder, objects);
    Schema.add_enums(builder, enums);
    Schema.add_file_ident(builder, file_ident);
    Schema.add_file_ext(builder, file_ext);
    Schema.add_root_table(builder, root_table);
    Schema.add_services(builder, services);
    Schema.add_advanced_features(builder, this.advanced_features);
    Schema.add_fbs_files(builder, fbs_files);
    return Schema.endSchema(builder);
  }
};

// typescript.js
var typescript_exports = {};
__export(typescript_exports, {
  Object_: () => Object_2,
  class_: () => class_2
});

// typescript/Object.js
var flatbuffers11 = __toESM(require("flatbuffers"), 1);

// foobar/class.js
var class_;
(function(class_3) {
  class_3[class_3["arguments_"] = 0] = "arguments_";
})(class_ || (class_ = {}));

// typescript/class.js
var class_2;
(function(class_3) {
  class_3[class_3["new_"] = 0] = "new_";
  class_3[class_3["instanceof_"] = 1] = "instanceof_";
})(class_2 || (class_2 = {}));

// typescript/Object.js
var Object_2 = class _Object_ {
  constructor() {
    this.bb = null;
    this.bb_pos = 0;
  }
  __init(i, bb) {
    this.bb_pos = i;
    this.bb = bb;
    return this;
  }
  static getRootAsObject(bb, obj) {
    return (obj || new _Object_()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  static getSizePrefixedRootAsObject(bb, obj) {
    bb.setPosition(bb.position() + flatbuffers11.SIZE_PREFIX_LENGTH);
    return (obj || new _Object_()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
  }
  return_() {
    const offset = this.bb.__offset(this.bb_pos, 4);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : 0;
  }
  mutate_return(value) {
    const offset = this.bb.__offset(this.bb_pos, 4);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt32(this.bb_pos + offset, value);
    return true;
  }
  if_() {
    const offset = this.bb.__offset(this.bb_pos, 6);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : 0;
  }
  mutate_if(value) {
    const offset = this.bb.__offset(this.bb_pos, 6);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt32(this.bb_pos + offset, value);
    return true;
  }
  switch_() {
    const offset = this.bb.__offset(this.bb_pos, 8);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : 0;
  }
  mutate_switch(value) {
    const offset = this.bb.__offset(this.bb_pos, 8);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt32(this.bb_pos + offset, value);
    return true;
  }
  enum_() {
    const offset = this.bb.__offset(this.bb_pos, 10);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : class_2.new_;
  }
  mutate_enum(value) {
    const offset = this.bb.__offset(this.bb_pos, 10);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt32(this.bb_pos + offset, value);
    return true;
  }
  enum2() {
    const offset = this.bb.__offset(this.bb_pos, 12);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : class_.arguments_;
  }
  mutate_enum2(value) {
    const offset = this.bb.__offset(this.bb_pos, 12);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt32(this.bb_pos + offset, value);
    return true;
  }
  enum3() {
    const offset = this.bb.__offset(this.bb_pos, 14);
    return offset ? this.bb.readInt32(this.bb_pos + offset) : Abc.a;
  }
  mutate_enum3(value) {
    const offset = this.bb.__offset(this.bb_pos, 14);
    if (offset === 0) {
      return false;
    }
    this.bb.writeInt32(this.bb_pos + offset, value);
    return true;
  }
  reflect(obj) {
    const offset = this.bb.__offset(this.bb_pos, 16);
    return offset ? (obj || new Schema()).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : null;
  }
  static getFullyQualifiedName() {
    return "typescript.Object";
  }
  static startObject(builder) {
    builder.startObject(7);
  }
  static add_return(builder, return_) {
    builder.addFieldInt32(0, return_, 0);
  }
  static add_if(builder, if_) {
    builder.addFieldInt32(1, if_, 0);
  }
  static add_switch(builder, switch_) {
    builder.addFieldInt32(2, switch_, 0);
  }
  static add_enum(builder, enum_) {
    builder.addFieldInt32(3, enum_, class_2.new_);
  }
  static add_enum2(builder, enum2) {
    builder.addFieldInt32(4, enum2, class_.arguments_);
  }
  static add_enum3(builder, enum3) {
    builder.addFieldInt32(5, enum3, Abc.a);
  }
  static add_reflect(builder, reflectOffset) {
    builder.addFieldOffset(6, reflectOffset, 0);
  }
  static endObject(builder) {
    const offset = builder.endObject();
    return offset;
  }
  unpack() {
    return new Object_T2(this.return_(), this.if_(), this.switch_(), this.enum_(), this.enum2(), this.enum3(), this.reflect() !== null ? this.reflect().unpack() : null);
  }
  unpackTo(_o) {
    _o.return_ = this.return_();
    _o.if_ = this.if_();
    _o.switch_ = this.switch_();
    _o.enum_ = this.enum_();
    _o.enum2 = this.enum2();
    _o.enum3 = this.enum3();
    _o.reflect = this.reflect() !== null ? this.reflect().unpack() : null;
  }
};
var Object_T2 = class {
  constructor(return_ = 0, if_ = 0, switch_ = 0, enum_ = class_2.new_, enum2 = class_.arguments_, enum3 = Abc.a, reflect = null) {
    this.return_ = return_;
    this.if_ = if_;
    this.switch_ = switch_;
    this.enum_ = enum_;
    this.enum2 = enum2;
    this.enum3 = enum3;
    this.reflect = reflect;
  }
  pack(builder) {
    const reflect = this.reflect !== null ? this.reflect.pack(builder) : 0;
    Object_2.startObject(builder);
    Object_2.add_return(builder, this.return_);
    Object_2.add_if(builder, this.if_);
    Object_2.add_switch(builder, this.switch_);
    Object_2.add_enum(builder, this.enum_);
    Object_2.add_enum2(builder, this.enum2);
    Object_2.add_enum3(builder, this.enum3);
    Object_2.add_reflect(builder, reflect);
    return Object_2.endObject(builder);
  }
};
