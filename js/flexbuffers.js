"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.encode = exports.toObject = exports.builder = exports.toReference = void 0;
/* eslint-disable @typescript-eslint/no-namespace */
var builder_1 = require("./flexbuffers/builder");
var reference_1 = require("./flexbuffers/reference");
var reference_2 = require("./flexbuffers/reference");
Object.defineProperty(exports, "toReference", { enumerable: true, get: function () { return reference_2.toReference; } });
function builder() {
    return new builder_1.Builder();
}
exports.builder = builder;
function toObject(buffer) {
    return reference_1.toReference(buffer).toObject();
}
exports.toObject = toObject;
function encode(object, size, deduplicateStrings, deduplicateKeys, deduplicateKeyVectors) {
    if (size === void 0) { size = 2048; }
    if (deduplicateStrings === void 0) { deduplicateStrings = true; }
    if (deduplicateKeys === void 0) { deduplicateKeys = true; }
    if (deduplicateKeyVectors === void 0) { deduplicateKeyVectors = true; }
    var builder = new builder_1.Builder(size > 0 ? size : 2048, deduplicateStrings, deduplicateKeys, deduplicateKeyVectors);
    builder.add(object);
    return builder.finish();
}
exports.encode = encode;
