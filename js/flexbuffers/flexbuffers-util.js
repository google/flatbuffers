"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.toUTF8Array = exports.fromUTF8Array = void 0;
function fromUTF8Array(data) {
    var decoder = new TextDecoder();
    return decoder.decode(data);
}
exports.fromUTF8Array = fromUTF8Array;
function toUTF8Array(str) {
    var encoder = new TextEncoder();
    return encoder.encode(str);
}
exports.toUTF8Array = toUTF8Array;
