// automatically generated, do not modify

// namespace: Example

var flatBuffers = require('flatbuffers');

var imports = {};
imports.vec3 = require('./vec3');
imports.test = require('./test');
imports.monster = require('./monster');
imports.stat = require('./stat');


exports.Monster = function(fb, pos) {
    this._fb = fb;
    this._pos = pos;
    this._table = new flatBuffers.Table(fb, pos);
};

exports.MonsterBuilder = {};

exports.Monster.prototype.init = function(fb, pos) {
    this._fb = fb;
    this._pos = pos;
    this._table = new flatBuffers.Table(fb, pos);
    return this;
};

exports.Monster.prototype.constructor = exports.Monster;

exports.getRootAsMonster = function(buf, offset) {
    return new exports.Monster(buf, buf.getInt32(offset) + offset);
}

exports.Monster.prototype.getPos = function(obj) {
    var o = this._table.getOffset(4);
    if (o != 0) {
        if (typeof obj === 'undefined') {
            obj = new imports.vec3.Vec3();
        };
        return obj.init(this._fb, o + this._pos);
    }
    return null;
};
exports.Monster.prototype.getMana = function() {
    var o = this._table.getOffset(6);
    if (o != 0) {
        return this._fb.getInt16(o + this._pos);
    }
    return 150;
};

exports.Monster.prototype.getHp = function() {
    var o = this._table.getOffset(8);
    if (o != 0) {
        return this._fb.getInt16(o + this._pos);
    }
    return 100;
};

exports.Monster.prototype.getName = function() {
    var o = this._table.getOffset(10);
    if (o != 0) {
        return this._fb.getString(o + this._pos);
    }
    return 0;
};

exports.Monster.prototype.getInventory = function(i) {
    var o = this._table.getOffset(14);
    if (o != 0) {
        return this._fb.getInt8(this._table.getVector(o) + i * 1);
    }
    return 0;
};
exports.Monster.prototype.getInventoryLength = function() {
    var o = this._table.getOffset(14);
    if (o != 0) {
        return this._table.getVectorLen(o);
    }
    return 0;
}

exports.Monster.prototype.getColor = function() {
    var o = this._table.getOffset(16);
    if (o != 0) {
        return this._fb.getInt8(o + this._pos);
    }
    return 8;
};

exports.Monster.prototype.getTestType = function() {
    var o = this._table.getOffset(18);
    if (o != 0) {
        return this._fb.getInt8(o + this._pos);
    }
    return 0;
};

exports.Monster.prototype.getTest = function(obj) {
    var o = this._table.getOffset(20);
    if (o != 0) {
        if (typeof obj === 'undefined') {
            obj = new flatBuffers.Table();
        };
        return this._table.getUnion(obj, o);
    }
    return null;
};

exports.Monster.prototype.getTest4 = function(i, obj) {
    var o = this._table.getOffset(22);
    if (o != 0) {
        if (typeof obj === 'undefined') {
            obj = new imports.test.Test();
        };
        return obj.init(this._fb, this._table.getVector(o) + i * 4);
    }
    return null;
}

exports.Monster.prototype.getTest4Length = function() {
    var o = this._table.getOffset(22);
    if (o != 0) {
        return this._table.getVectorLen(o);
    }
    return 0;
}

exports.Monster.prototype.getTestarrayofstring = function(i) {
    var o = this._table.getOffset(24);
    if (o != 0) {
        return this._fb.getString(this._table.getVector(o) + i * 4);
    }
    return '';
};
exports.Monster.prototype.getTestarrayofstringLength = function() {
    var o = this._table.getOffset(24);
    if (o != 0) {
        return this._table.getVectorLen(o);
    }
    return 0;
}

/// an example documentation comment: this will end up in the generated code
/// multiline too
exports.Monster.prototype.getTestarrayoftables = function(i, obj) {
    var o = this._table.getOffset(26);
    if (o != 0) {
        if (typeof obj === 'undefined') {
            obj = new imports.monster.Monster();
        };
        return obj.init(this._fb, this._table.getVector(o) + i * 4);
    }
    return null;
}

exports.Monster.prototype.getTestarrayoftablesLength = function() {
    var o = this._table.getOffset(26);
    if (o != 0) {
        return this._table.getVectorLen(o);
    }
    return 0;
}

exports.Monster.prototype.getEnemy = function(obj) {
    var o = this._table.getOffset(28);
    if (o != 0) {
        if (typeof obj === 'undefined') {
            obj = new imports.monster.Monster();
        };
        return obj.init(this._fb, this._table.getIndirect(o + this._pos));
    }
    return null;
};
exports.Monster.prototype.getTestnestedflatbuffer = function(i) {
    var o = this._table.getOffset(30);
    if (o != 0) {
        return this._fb.getInt8(this._table.getVector(o) + i * 1);
    }
    return 0;
};
exports.Monster.prototype.getTestnestedflatbufferLength = function() {
    var o = this._table.getOffset(30);
    if (o != 0) {
        return this._table.getVectorLen(o);
    }
    return 0;
}

exports.Monster.prototype.getTestempty = function(obj) {
    var o = this._table.getOffset(32);
    if (o != 0) {
        if (typeof obj === 'undefined') {
            obj = new imports.stat.Stat();
        };
        return obj.init(this._fb, this._table.getIndirect(o + this._pos));
    }
    return null;
};
exports.Monster.prototype.getTestbool = function() {
    var o = this._table.getOffset(34);
    if (o != 0) {
        return this._fb.getInt8(o + this._pos);
    }
    return 0;
};

exports.Monster.prototype.getTesthashs32Fnv1 = function() {
    var o = this._table.getOffset(36);
    if (o != 0) {
        return this._fb.getInt32(o + this._pos);
    }
    return 0;
};

exports.Monster.prototype.getTesthashu32Fnv1 = function() {
    var o = this._table.getOffset(38);
    if (o != 0) {
        return this._fb.getUInt23(o + this._pos);
    }
    return 0;
};

exports.Monster.prototype.getTesthashs64Fnv1 = function() {
    var o = this._table.getOffset(40);
    if (o != 0) {
        return this._fb.getInt64(o + this._pos);
    }
    return 0;
};

exports.Monster.prototype.getTesthashu64Fnv1 = function() {
    var o = this._table.getOffset(42);
    if (o != 0) {
        return this._fb.getUInt64(o + this._pos);
    }
    return 0;
};

exports.Monster.prototype.getTesthashs32Fnv1a = function() {
    var o = this._table.getOffset(44);
    if (o != 0) {
        return this._fb.getInt32(o + this._pos);
    }
    return 0;
};

exports.Monster.prototype.getTesthashu32Fnv1a = function() {
    var o = this._table.getOffset(46);
    if (o != 0) {
        return this._fb.getUInt23(o + this._pos);
    }
    return 0;
};

exports.Monster.prototype.getTesthashs64Fnv1a = function() {
    var o = this._table.getOffset(48);
    if (o != 0) {
        return this._fb.getInt64(o + this._pos);
    }
    return 0;
};

exports.Monster.prototype.getTesthashu64Fnv1a = function() {
    var o = this._table.getOffset(50);
    if (o != 0) {
        return this._fb.getUInt64(o + this._pos);
    }
    return 0;
};

exports.MonsterBuilder.start = function(builder) {
    builder.startObject(24);
};
exports.MonsterBuilder.addPos = function(builder, pos) {
    builder.addStruct(0, pos, 0);
};
exports.MonsterBuilder.addMana = function(builder, mana) {
    builder.addInt16(1, mana, 150);
};
exports.MonsterBuilder.addHp = function(builder, hp) {
    builder.addInt16(2, hp, 100);
};
exports.MonsterBuilder.addName = function(builder, name) {
    builder.addOffset(3, name, 0);
};
exports.MonsterBuilder.addInventory = function(builder, inventory) {
    builder.addOffset(5, inventory, 0);
};
exports.MonsterBuilder.startInventoryVector = function(builder, numElems) {
    return builder.startVector(1, numElems, 1);
};
exports.MonsterBuilder.addColor = function(builder, color) {
    builder.addInt8(6, color, 8);
};
exports.MonsterBuilder.addTestType = function(builder, testType) {
    builder.addInt8(7, testType, 0);
};
exports.MonsterBuilder.addTest = function(builder, test) {
    builder.addOffset(8, test, 0);
};
exports.MonsterBuilder.addTest4 = function(builder, test4) {
    builder.addOffset(9, test4, 0);
};
exports.MonsterBuilder.startTest4Vector = function(builder, numElems) {
    return builder.startVector(4, numElems, 2);
};
exports.MonsterBuilder.addTestarrayofstring = function(builder, testarrayofstring) {
    builder.addOffset(10, testarrayofstring, 0);
};
exports.MonsterBuilder.startTestarrayofstringVector = function(builder, numElems) {
    return builder.startVector(4, numElems, 4);
};
exports.MonsterBuilder.addTestarrayoftables = function(builder, testarrayoftables) {
    builder.addOffset(11, testarrayoftables, 0);
};
exports.MonsterBuilder.startTestarrayoftablesVector = function(builder, numElems) {
    return builder.startVector(4, numElems, 4);
};
exports.MonsterBuilder.addEnemy = function(builder, enemy) {
    builder.addOffset(12, enemy, 0);
};
exports.MonsterBuilder.addTestnestedflatbuffer = function(builder, testnestedflatbuffer) {
    builder.addOffset(13, testnestedflatbuffer, 0);
};
exports.MonsterBuilder.startTestnestedflatbufferVector = function(builder, numElems) {
    return builder.startVector(1, numElems, 1);
};
exports.MonsterBuilder.addTestempty = function(builder, testempty) {
    builder.addOffset(14, testempty, 0);
};
exports.MonsterBuilder.addTestbool = function(builder, testbool) {
    builder.addInt8(15, testbool, 0);
};
exports.MonsterBuilder.addTesthashs32Fnv1 = function(builder, testhashs32Fnv1) {
    builder.addInt32(16, testhashs32Fnv1, 0);
};
exports.MonsterBuilder.addTesthashu32Fnv1 = function(builder, testhashu32Fnv1) {
    builder.addUInt23(17, testhashu32Fnv1, 0);
};
exports.MonsterBuilder.addTesthashs64Fnv1 = function(builder, testhashs64Fnv1) {
    builder.addInt64(18, testhashs64Fnv1, 0);
};
exports.MonsterBuilder.addTesthashu64Fnv1 = function(builder, testhashu64Fnv1) {
    builder.addUInt64(19, testhashu64Fnv1, 0);
};
exports.MonsterBuilder.addTesthashs32Fnv1a = function(builder, testhashs32Fnv1a) {
    builder.addInt32(20, testhashs32Fnv1a, 0);
};
exports.MonsterBuilder.addTesthashu32Fnv1a = function(builder, testhashu32Fnv1a) {
    builder.addUInt23(21, testhashu32Fnv1a, 0);
};
exports.MonsterBuilder.addTesthashs64Fnv1a = function(builder, testhashs64Fnv1a) {
    builder.addInt64(22, testhashs64Fnv1a, 0);
};
exports.MonsterBuilder.addTesthashu64Fnv1a = function(builder, testhashu64Fnv1a) {
    builder.addUInt64(23, testhashu64Fnv1a, 0);
};
exports.MonsterBuilder.end = function(builder) {
    return builder.endObject();
};