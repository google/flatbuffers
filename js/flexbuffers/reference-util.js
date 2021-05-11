"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.keyForIndex = exports.valueForIndexWithKey = exports.diffKeys = exports.keyIndex = exports.indirect = exports.readFloat = exports.readUInt = exports.readInt = exports.validateOffset = void 0;
var bit_width_1 = require("./bit-width");
var bit_width_util_1 = require("./bit-width-util");
var flexbuffers_util_1 = require("./flexbuffers-util");
var reference_1 = require("./reference");
var long_1 = require("../long");
function validateOffset(dataView, offset, width) {
    if (dataView.byteLength <= offset + width || (offset & (bit_width_util_1.toByteWidth(width) - 1)) !== 0) {
        throw "Bad offset: " + offset + ", width: " + width;
    }
}
exports.validateOffset = validateOffset;
function readInt(dataView, offset, width) {
    if (width < 2) {
        if (width < 1) {
            return dataView.getInt8(offset);
        }
        else {
            return dataView.getInt16(offset, true);
        }
    }
    else {
        if (width < 3) {
            return dataView.getInt32(offset, true);
        }
        else {
            if (dataView.setBigInt64 === undefined) {
                return new long_1.Long(dataView.getUint32(offset, true), dataView.getUint32(offset + 4, true));
            }
            return dataView.getBigInt64(offset, true);
        }
    }
}
exports.readInt = readInt;
function readUInt(dataView, offset, width) {
    if (width < 2) {
        if (width < 1) {
            return dataView.getUint8(offset);
        }
        else {
            return dataView.getUint16(offset, true);
        }
    }
    else {
        if (width < 3) {
            return dataView.getUint32(offset, true);
        }
        else {
            if (dataView.getBigUint64 === undefined) {
                return new long_1.Long(dataView.getUint32(offset, true), dataView.getUint32(offset + 4, true));
            }
            return dataView.getBigUint64(offset, true);
        }
    }
}
exports.readUInt = readUInt;
function readFloat(dataView, offset, width) {
    if (width < bit_width_1.BitWidth.WIDTH32) {
        throw "Bad width: " + width;
    }
    if (width === bit_width_1.BitWidth.WIDTH32) {
        return dataView.getFloat32(offset, true);
    }
    return dataView.getFloat64(offset, true);
}
exports.readFloat = readFloat;
function indirect(dataView, offset, width) {
    var step = readUInt(dataView, offset, width);
    return offset - step;
}
exports.indirect = indirect;
function keyIndex(key, dataView, offset, parentWidth, byteWidth, length) {
    var input = flexbuffers_util_1.toUTF8Array(key);
    var keysVectorOffset = indirect(dataView, offset, parentWidth) - byteWidth * 3;
    var bitWidth = bit_width_util_1.fromByteWidth(byteWidth);
    var indirectOffset = keysVectorOffset - readUInt(dataView, keysVectorOffset, bitWidth);
    var _byteWidth = readUInt(dataView, keysVectorOffset + byteWidth, bitWidth);
    var low = 0;
    var high = length - 1;
    while (low <= high) {
        var mid = (high + low) >> 1;
        var dif = diffKeys(input, mid, dataView, indirectOffset, _byteWidth);
        if (dif === 0)
            return mid;
        if (dif < 0) {
            high = mid - 1;
        }
        else {
            low = mid + 1;
        }
    }
    return null;
}
exports.keyIndex = keyIndex;
function diffKeys(input, index, dataView, offset, width) {
    var keyOffset = offset + index * width;
    var keyIndirectOffset = keyOffset - readUInt(dataView, keyOffset, bit_width_util_1.fromByteWidth(width));
    for (var i = 0; i < input.length; i++) {
        var dif = input[i] - dataView.getUint8(keyIndirectOffset + i);
        if (dif !== 0) {
            return dif;
        }
    }
    return dataView.getUint8(keyIndirectOffset + input.length) === 0 ? 0 : -1;
}
exports.diffKeys = diffKeys;
function valueForIndexWithKey(index, key, dataView, offset, parentWidth, byteWidth, length, path) {
    var _indirect = indirect(dataView, offset, parentWidth);
    var elementOffset = _indirect + index * byteWidth;
    var packedType = dataView.getUint8(_indirect + length * byteWidth + index);
    return new reference_1.Reference(dataView, elementOffset, bit_width_util_1.fromByteWidth(byteWidth), packedType, path + "/" + key);
}
exports.valueForIndexWithKey = valueForIndexWithKey;
function keyForIndex(index, dataView, offset, parentWidth, byteWidth) {
    var keysVectorOffset = indirect(dataView, offset, parentWidth) - byteWidth * 3;
    var bitWidth = bit_width_util_1.fromByteWidth(byteWidth);
    var indirectOffset = keysVectorOffset - readUInt(dataView, keysVectorOffset, bitWidth);
    var _byteWidth = readUInt(dataView, keysVectorOffset + byteWidth, bitWidth);
    var keyOffset = indirectOffset + index * _byteWidth;
    var keyIndirectOffset = keyOffset - readUInt(dataView, keyOffset, bit_width_util_1.fromByteWidth(_byteWidth));
    var length = 0;
    while (dataView.getUint8(keyIndirectOffset + length) !== 0) {
        length++;
    }
    return flexbuffers_util_1.fromUTF8Array(new Uint8Array(dataView.buffer, keyIndirectOffset, length));
}
exports.keyForIndex = keyForIndex;
