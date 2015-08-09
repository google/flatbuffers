// automatically generated, do not modify

// namespace: Example

var flatBuffers = require('flatbuffers');

var imports = {};

exports.Stat = function(fb, pos) {
    this._fb = fb;
    this._pos = pos;
    this._table = new flatBuffers.Table(fb, pos);
};

exports.StatBuilder = {};

exports.Stat.prototype.init = function(fb, pos) {
    this._fb = fb;
    this._pos = pos;
    this._table = new flatBuffers.Table(fb, pos);
    return this;
};

exports.Stat.prototype.constructor = exports.Stat;

exports.Stat.prototype.getId = function() {
    var o = this._table.getOffset(4);
    if (o != 0) {
        return this._fb.getString(o + this._pos);
    }
    return 0;
};

exports.Stat.prototype.getVal = function() {
    var o = this._table.getOffset(6);
    if (o != 0) {
        return this._fb.getInt64(o + this._pos);
    }
    return 0;
};

exports.Stat.prototype.getCount = function() {
    var o = this._table.getOffset(8);
    if (o != 0) {
        return this._fb.getUInt16(o + this._pos);
    }
    return 0;
};

exports.StatBuilder.start = function(builder) {
    builder.startObject(3);
};
exports.StatBuilder.addId = function(builder, id) {
    builder.addOffset(0, id, 0);
};
exports.StatBuilder.addVal = function(builder, val) {
    builder.addInt64(1, val, 0);
};
exports.StatBuilder.addCount = function(builder, count) {
    builder.addUInt16(2, count, 0);
};
exports.StatBuilder.end = function(builder) {
    return builder.endObject();
};