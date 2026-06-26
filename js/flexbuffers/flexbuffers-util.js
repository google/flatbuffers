"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.fromUTF8Array = fromUTF8Array;
exports.toUTF8Array = toUTF8Array;
function fromUTF8Array(data) {
    const decoder = new TextDecoder();
    return decoder.decode(data);
}
function toUTF8Array(str) {
    const encoder = new TextEncoder();
    return encoder.encode(str);
}
