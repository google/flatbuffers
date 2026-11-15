"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.toByteWidth = toByteWidth;
exports.iwidth = iwidth;
exports.fwidth = fwidth;
exports.uwidth = uwidth;
exports.fromByteWidth = fromByteWidth;
exports.paddingSize = paddingSize;
const bit_width_js_1 = require("./bit-width.js");
function toByteWidth(bitWidth) {
    return 1 << bitWidth;
}
function iwidth(value) {
    if (value >= -128 && value <= 127)
        return bit_width_js_1.BitWidth.WIDTH8;
    if (value >= -32768 && value <= 32767)
        return bit_width_js_1.BitWidth.WIDTH16;
    if (value >= -2147483648 && value <= 2147483647)
        return bit_width_js_1.BitWidth.WIDTH32;
    return bit_width_js_1.BitWidth.WIDTH64;
}
function fwidth(value) {
    return value === Math.fround(value) ? bit_width_js_1.BitWidth.WIDTH32 : bit_width_js_1.BitWidth.WIDTH64;
}
function uwidth(value) {
    if (value <= 255)
        return bit_width_js_1.BitWidth.WIDTH8;
    if (value <= 65535)
        return bit_width_js_1.BitWidth.WIDTH16;
    if (value <= 4294967295)
        return bit_width_js_1.BitWidth.WIDTH32;
    return bit_width_js_1.BitWidth.WIDTH64;
}
function fromByteWidth(value) {
    if (value === 1)
        return bit_width_js_1.BitWidth.WIDTH8;
    if (value === 2)
        return bit_width_js_1.BitWidth.WIDTH16;
    if (value === 4)
        return bit_width_js_1.BitWidth.WIDTH32;
    return bit_width_js_1.BitWidth.WIDTH64;
}
function paddingSize(bufSize, scalarSize) {
    return (~bufSize + 1) & (scalarSize - 1);
}
