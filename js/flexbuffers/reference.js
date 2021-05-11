"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Reference = exports.toReference = void 0;
var bit_width_util_1 = require("./bit-width-util");
var value_type_1 = require("./value-type");
var value_type_util_1 = require("./value-type-util");
var reference_util_1 = require("./reference-util");
var flexbuffers_util_1 = require("./flexbuffers-util");
var bit_width_1 = require("./bit-width");
function toReference(buffer) {
    var len = buffer.byteLength;
    if (len < 3) {
        throw "Buffer needs to be bigger than 3";
    }
    var dataView = new DataView(buffer);
    var byteWidth = dataView.getUint8(len - 1);
    var packedType = dataView.getUint8(len - 2);
    var parentWidth = bit_width_util_1.fromByteWidth(byteWidth);
    var offset = len - byteWidth - 2;
    return new Reference(dataView, offset, parentWidth, packedType, "/");
}
exports.toReference = toReference;
var Reference = /** @class */ (function () {
    function Reference(dataView, offset, parentWidth, packedType, path) {
        this.dataView = dataView;
        this.offset = offset;
        this.parentWidth = parentWidth;
        this.packedType = packedType;
        this.path = path;
        this._length = -1;
        this.byteWidth = 1 << (packedType & 3);
        this.valueType = packedType >> 2;
    }
    Reference.prototype.isNull = function () { return this.valueType === value_type_1.ValueType.NULL; };
    Reference.prototype.isNumber = function () { return value_type_util_1.isNumber(this.valueType) || value_type_util_1.isIndirectNumber(this.valueType); };
    Reference.prototype.isFloat = function () { return value_type_1.ValueType.FLOAT === this.valueType || value_type_1.ValueType.INDIRECT_FLOAT === this.valueType; };
    Reference.prototype.isInt = function () { return this.isNumber() && !this.isFloat(); };
    Reference.prototype.isString = function () { return value_type_1.ValueType.STRING === this.valueType || value_type_1.ValueType.KEY === this.valueType; };
    Reference.prototype.isBool = function () { return value_type_1.ValueType.BOOL === this.valueType; };
    Reference.prototype.isBlob = function () { return value_type_1.ValueType.BLOB === this.valueType; };
    Reference.prototype.isVector = function () { return value_type_util_1.isAVector(this.valueType); };
    Reference.prototype.isMap = function () { return value_type_1.ValueType.MAP === this.valueType; };
    Reference.prototype.boolValue = function () {
        if (this.isBool()) {
            return reference_util_1.readInt(this.dataView, this.offset, this.parentWidth) > 0;
        }
        return null;
    };
    Reference.prototype.intValue = function () {
        if (this.valueType === value_type_1.ValueType.INT) {
            return reference_util_1.readInt(this.dataView, this.offset, this.parentWidth);
        }
        if (this.valueType === value_type_1.ValueType.UINT) {
            return reference_util_1.readUInt(this.dataView, this.offset, this.parentWidth);
        }
        if (this.valueType === value_type_1.ValueType.INDIRECT_INT) {
            return reference_util_1.readInt(this.dataView, reference_util_1.indirect(this.dataView, this.offset, this.parentWidth), bit_width_util_1.fromByteWidth(this.byteWidth));
        }
        if (this.valueType === value_type_1.ValueType.INDIRECT_UINT) {
            return reference_util_1.readUInt(this.dataView, reference_util_1.indirect(this.dataView, this.offset, this.parentWidth), bit_width_util_1.fromByteWidth(this.byteWidth));
        }
        return null;
    };
    Reference.prototype.floatValue = function () {
        if (this.valueType === value_type_1.ValueType.FLOAT) {
            return reference_util_1.readFloat(this.dataView, this.offset, this.parentWidth);
        }
        if (this.valueType === value_type_1.ValueType.INDIRECT_FLOAT) {
            return reference_util_1.readFloat(this.dataView, reference_util_1.indirect(this.dataView, this.offset, this.parentWidth), bit_width_util_1.fromByteWidth(this.byteWidth));
        }
        return null;
    };
    Reference.prototype.numericValue = function () { return this.floatValue() || this.intValue(); };
    Reference.prototype.stringValue = function () {
        if (this.valueType === value_type_1.ValueType.STRING || this.valueType === value_type_1.ValueType.KEY) {
            var begin = reference_util_1.indirect(this.dataView, this.offset, this.parentWidth);
            return flexbuffers_util_1.fromUTF8Array(new Uint8Array(this.dataView.buffer, begin, this.length()));
        }
        return null;
    };
    Reference.prototype.blobValue = function () {
        if (this.isBlob()) {
            var begin = reference_util_1.indirect(this.dataView, this.offset, this.parentWidth);
            return new Uint8Array(this.dataView.buffer, begin, this.length());
        }
        return null;
    };
    Reference.prototype.get = function (key) {
        var length = this.length();
        if (Number.isInteger(key) && value_type_util_1.isAVector(this.valueType)) {
            if (key >= length || key < 0) {
                throw "Key: [" + key + "] is not applicable on " + this.path + " of " + this.valueType + " length: " + length;
            }
            var _indirect = reference_util_1.indirect(this.dataView, this.offset, this.parentWidth);
            var elementOffset = _indirect + key * this.byteWidth;
            var _packedType = this.dataView.getUint8(_indirect + length * this.byteWidth + key);
            if (value_type_util_1.isTypedVector(this.valueType)) {
                var _valueType = value_type_util_1.typedVectorElementType(this.valueType);
                _packedType = value_type_util_1.packedType(_valueType, bit_width_1.BitWidth.WIDTH8);
            }
            else if (value_type_util_1.isFixedTypedVector(this.valueType)) {
                var _valueType = value_type_util_1.fixedTypedVectorElementType(this.valueType);
                _packedType = value_type_util_1.packedType(_valueType, bit_width_1.BitWidth.WIDTH8);
            }
            return new Reference(this.dataView, elementOffset, bit_width_util_1.fromByteWidth(this.byteWidth), _packedType, this.path + "[" + key + "]");
        }
        if (typeof key === 'string') {
            var index = reference_util_1.keyIndex(key, this.dataView, this.offset, this.parentWidth, this.byteWidth, length);
            if (index !== null) {
                return reference_util_1.valueForIndexWithKey(index, key, this.dataView, this.offset, this.parentWidth, this.byteWidth, length, this.path);
            }
        }
        throw "Key [" + key + "] is not applicable on " + this.path + " of " + this.valueType;
    };
    Reference.prototype.length = function () {
        var size;
        if (this._length > -1) {
            return this._length;
        }
        if (value_type_util_1.isFixedTypedVector(this.valueType)) {
            this._length = value_type_util_1.fixedTypedVectorElementSize(this.valueType);
        }
        else if (this.valueType === value_type_1.ValueType.BLOB
            || this.valueType === value_type_1.ValueType.MAP
            || value_type_util_1.isAVector(this.valueType)) {
            this._length = reference_util_1.readUInt(this.dataView, reference_util_1.indirect(this.dataView, this.offset, this.parentWidth) - this.byteWidth, bit_width_util_1.fromByteWidth(this.byteWidth));
        }
        else if (this.valueType === value_type_1.ValueType.NULL) {
            this._length = 0;
        }
        else if (this.valueType === value_type_1.ValueType.STRING) {
            var _indirect = reference_util_1.indirect(this.dataView, this.offset, this.parentWidth);
            var sizeByteWidth = this.byteWidth;
            size = reference_util_1.readUInt(this.dataView, _indirect - sizeByteWidth, bit_width_util_1.fromByteWidth(this.byteWidth));
            while (this.dataView.getInt8(_indirect + size) !== 0) {
                sizeByteWidth <<= 1;
                size = reference_util_1.readUInt(this.dataView, _indirect - sizeByteWidth, bit_width_util_1.fromByteWidth(this.byteWidth));
            }
            this._length = size;
        }
        else if (this.valueType === value_type_1.ValueType.KEY) {
            var _indirect = reference_util_1.indirect(this.dataView, this.offset, this.parentWidth);
            size = 1;
            while (this.dataView.getInt8(_indirect + size) !== 0) {
                size++;
            }
            this._length = size;
        }
        else {
            this._length = 1;
        }
        return this._length;
    };
    Reference.prototype.toObject = function () {
        var length = this.length();
        if (this.isVector()) {
            var result = [];
            for (var i = 0; i < length; i++) {
                result.push(this.get(i).toObject());
            }
            return result;
        }
        if (this.isMap()) {
            var result = {};
            for (var i = 0; i < length; i++) {
                var key = reference_util_1.keyForIndex(i, this.dataView, this.offset, this.parentWidth, this.byteWidth);
                result[key] = reference_util_1.valueForIndexWithKey(i, key, this.dataView, this.offset, this.parentWidth, this.byteWidth, length, this.path).toObject();
            }
            return result;
        }
        if (this.isNull()) {
            return null;
        }
        if (this.isBool()) {
            return this.boolValue();
        }
        if (this.isNumber()) {
            return this.numericValue();
        }
        return this.blobValue() || this.stringValue();
    };
    return Reference;
}());
exports.Reference = Reference;
