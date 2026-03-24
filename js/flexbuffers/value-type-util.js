"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isInline = isInline;
exports.isNumber = isNumber;
exports.isIndirectNumber = isIndirectNumber;
exports.isTypedVectorElement = isTypedVectorElement;
exports.isTypedVector = isTypedVector;
exports.isFixedTypedVector = isFixedTypedVector;
exports.isAVector = isAVector;
exports.toTypedVector = toTypedVector;
exports.typedVectorElementType = typedVectorElementType;
exports.fixedTypedVectorElementType = fixedTypedVectorElementType;
exports.fixedTypedVectorElementSize = fixedTypedVectorElementSize;
exports.packedType = packedType;
const value_type_js_1 = require("./value-type.js");
function isInline(value) {
    return value === value_type_js_1.ValueType.BOOL || value <= value_type_js_1.ValueType.FLOAT;
}
function isNumber(value) {
    return value >= value_type_js_1.ValueType.INT && value <= value_type_js_1.ValueType.FLOAT;
}
function isIndirectNumber(value) {
    return value >= value_type_js_1.ValueType.INDIRECT_INT && value <= value_type_js_1.ValueType.INDIRECT_FLOAT;
}
function isTypedVectorElement(value) {
    return (value === value_type_js_1.ValueType.BOOL ||
        (value >= value_type_js_1.ValueType.INT && value <= value_type_js_1.ValueType.STRING));
}
function isTypedVector(value) {
    return (value === value_type_js_1.ValueType.VECTOR_BOOL ||
        (value >= value_type_js_1.ValueType.VECTOR_INT &&
            value <= value_type_js_1.ValueType.VECTOR_STRING_DEPRECATED));
}
function isFixedTypedVector(value) {
    return value >= value_type_js_1.ValueType.VECTOR_INT2 && value <= value_type_js_1.ValueType.VECTOR_FLOAT4;
}
function isAVector(value) {
    return (isTypedVector(value) ||
        isFixedTypedVector(value) ||
        value === value_type_js_1.ValueType.VECTOR);
}
function toTypedVector(valueType, length) {
    if (length === 0)
        return valueType - value_type_js_1.ValueType.INT + value_type_js_1.ValueType.VECTOR_INT;
    if (length === 2)
        return valueType - value_type_js_1.ValueType.INT + value_type_js_1.ValueType.VECTOR_INT2;
    if (length === 3)
        return valueType - value_type_js_1.ValueType.INT + value_type_js_1.ValueType.VECTOR_INT3;
    if (length === 4)
        return valueType - value_type_js_1.ValueType.INT + value_type_js_1.ValueType.VECTOR_INT4;
    throw 'Unexpected length ' + length;
}
function typedVectorElementType(valueType) {
    return valueType - value_type_js_1.ValueType.VECTOR_INT + value_type_js_1.ValueType.INT;
}
function fixedTypedVectorElementType(valueType) {
    return ((valueType - value_type_js_1.ValueType.VECTOR_INT2) % 3) + value_type_js_1.ValueType.INT;
}
function fixedTypedVectorElementSize(valueType) {
    // The x / y >> 0 trick is to have an int division. Suppose to be faster than Math.floor()
    return (((valueType - value_type_js_1.ValueType.VECTOR_INT2) / 3) >> 0) + 2;
}
function packedType(valueType, bitWidth) {
    return bitWidth | (valueType << 2);
}
