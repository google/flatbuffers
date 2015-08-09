// automatically generated, do not modify

// namespace: Example

var flatBuffers = require('flatbuffers');

var imports = {};

exports.Test = function(fb, pos) {
    this._fb = fb;
    this._pos = pos;
};

exports.TestBuilder = {};

exports.Test.prototype.init = function(fb, pos) {
    this._fb = fb;
    this._pos = pos;
    return this;
};

exports.Test.prototype.constructor = exports.Test;

exports.Test.prototype.getA = function() {
    return this._fb.getInt16(this._pos + 0);
};

exports.Test.prototype.getB = function() {
    return this._fb.getInt8(this._pos + 2);
};


exports.createTest = function(builder, a, b) {
    builder.prep(2, 4);
    builder.pad(1);
    builder.prependInt8(b);
    builder.prependInt16(a);
    return builder.getOffset();
};
