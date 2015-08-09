// automatically generated, do not modify

// namespace: Example

var flatBuffers = require('flatbuffers');

var imports = {};
imports.test = require('./test');


exports.Vec3 = function(fb, pos) {
    this._fb = fb;
    this._pos = pos;
};

exports.Vec3Builder = {};

exports.Vec3.prototype.init = function(fb, pos) {
    this._fb = fb;
    this._pos = pos;
    return this;
};

exports.Vec3.prototype.constructor = exports.Vec3;

exports.Vec3.prototype.getX = function() {
    return this._fb.getFloat32(this._pos + 0);
};

exports.Vec3.prototype.getY = function() {
    return this._fb.getFloat32(this._pos + 4);
};

exports.Vec3.prototype.getZ = function() {
    return this._fb.getFloat32(this._pos + 8);
};

exports.Vec3.prototype.getTest1 = function() {
    return this._fb.getFloat64(this._pos + 16);
};

exports.Vec3.prototype.getTest2 = function() {
    return this._fb.getInt8(this._pos + 24);
};

exports.Vec3.prototype.getTest3 = function(obj) {
    if (typeof obj === 'undefined') {
        obj = new imports.test.Test();
    };
    obj.init(this._fb, this._pos + 26)
    return obj;
};

exports.createVec3 = function(builder, x, y, z, test1, test2, test3_a, test3_b) {
    builder.prep(16, 32);
    builder.pad(2);
    builder.prep(2, 4);
    builder.pad(1);
    builder.prependInt8(test3_b);
    builder.prependInt16(test3_a);
    builder.pad(1);
    builder.prependInt8(test2);
    builder.prependFloat64(test1);
    builder.pad(4);
    builder.prependFloat32(z);
    builder.prependFloat32(y);
    builder.prependFloat32(x);
    return builder.getOffset();
};
