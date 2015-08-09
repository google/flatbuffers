/*
 * Copyright 2015 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
require('buffer');

/*
 * FlatBuffer Builder - use to create a FlatBuffer
 */
exports.Builder = function(initialSize) {
    this._buffer = new Buffer(initialSize);
    this._buffer.fill(0); // initialize to null
    this._vtables = new Int32Array(16);
    this._vtablesCount = 0;
    this._vtable = null;
    this._objectStart = initialSize;
    this._minAlign = 1;
    this._head = initialSize;
  };

exports.Builder.prototype.constructor = exports.Builder;

exports.Builder.prototype.clear = function() {      
    this._buffer.fill(0); // initialize to null
    this._vtables = new Int32Array(16);
    this._vtablesCount = 0;
    this._vtable = null;  
    this._objectStart = this._buffer.length;
    this._minAlign = 1;
    this._head = this._buffer.length;  
};

exports.Builder.prototype.getOffset = function() {
    return this._buffer.length - this._head;
};

/*
 * returns the current head position
 */
exports.Builder.prototype.getHead = function() {
    return this._head;
};

/*
 * returns the raw byte buffer
 */
exports.Builder.prototype.getBuffer = function() {
    return this._buffer;
};

/*
 * Copy the written portion of the buffer, for export to files
 */
exports.Builder.prototype.copyData = function() {
    // create a new buffer from this, containing only the populated data
    var buf = new Buffer(this._buffer.length - this._head);
    this._buffer.copy(buf, 0, this._head);
    return buf;
};

/*
 * writes 'n' pad bytes to the buffer
 */
exports.Builder.prototype.pad = function(n) {
    for (var i = 0; i < n; ++i) {
        this.placeInt8(0, 1);
    }
};

/*
 * prepare the buffer for writing 'size' bytes, with 'additionalBytes' padding
 */
exports.Builder.prototype.prep = function(size, additionalBytes) {
    // Track the biggest alignment
    if (size > this._minAlign) {
      this._minAlign = size;
    }

    var alignSize = (~(this._buffer.length - this._head + additionalBytes)) + 1;
    alignSize &= (size - 1);
    
    while (this._head < alignSize + additionalBytes + size) {
      var oldBuffSize = this._buffer.length;
      this.growBuffer();
      this._head = this._buffer.length + this._head - oldBuffSize;
    }

    this.pad(alignSize);
};

/*
 * grows the buffer to a max of 2gb
 */
exports.Builder.prototype.growBuffer = function() {
    
  if ((this._buffer.length & 0xC0000000) != 0) {
      throw new Error('Flatbuffers: buffer cannot grow past 2gb');
  }

  var newSize = (this._buffer.length * 2 ) | 0;
  if (newSize === 0) {
      newSize = 1;
  }
  var newBuffer = new Buffer(newSize);
  newBuffer.fill(0, 0, this._buffer.length); // initialize to null
  this._buffer.copy(newBuffer, this._buffer.length);
  this._buffer = newBuffer;
};

/*
 * Methods which do in-place writes at head
 */
exports.Builder.prototype.placeBool = function(x) {
    this.placeInt8(x|0 ? 1 : 0);
};

exports.Builder.prototype.placeInt8 = function(x) {
    this._head -= 1;
    this._buffer.writeInt8(x|0, this._head);
};

exports.Builder.prototype.placeInt16 = function(x) {
    this._head -= 2;
    this._buffer.writeInt16LE(x|0, this._head);
};

exports.Builder.prototype.placeUInt16 = function(x) {
    this._head -= 2;
    this._buffer.writeUInt16LE(x|0, this._head);
};

exports.Builder.prototype.placeInt32 = function(x) {
    this._head -= 4;
    this._buffer.writeInt32LE(x|0, this._head);
};

exports.Builder.prototype.placeUInt32 = function(x) {
    this._head -= 4;
    this._buffer.writeUInt32LE(x|0, this._head);
};

exports.Builder.prototype.placeInt64 = function(x) {
    this._head -= 8;
    this._buffer.writeIntLE(x|0, this._head, 8);
};

exports.Builder.prototype.placeUInt64 = function(x) {
    this._head -= 8;
    this._buffer.writeUIntLE(x|0, this._head, 8);
};

exports.Builder.prototype.placeFloat32 = function(x) {
    this._head -= 4;
    this._buffer.writeFloatLE(+(x), this._head);
};

exports.Builder.prototype.placeFloat64 = function(x) {
    this._head -= 8;
    this._buffer.writeDoubleLE(+(x), this._head);
};


/*
 * Methods which adjust head, padding where required and write the value
 */

exports.Builder.prototype.prependBool = function (x) {
    this.prependInt8(x|0 ? 1 : 0);
};

exports.Builder.prototype.prependInt8 = function (x) {
    this.prep(1, 0);
    this.placeInt8(x);
};

exports.Builder.prototype.prependInt16 = function (x) {
    this.prep(2, 0);
    this.placeInt16(x);
};

exports.Builder.prototype.prependUInt16 = function (x) {
    this.prep(2, 0);
    this.placeUInt16(x);
};

exports.Builder.prototype.prependInt32 = function(x) {
    this.prep(4, 0);
    this.placeInt32(x);
};

exports.Builder.prototype.prependUInt32 = function(x) {
    this.prep(4, 0);
    this.placeUInt32(x);
};

exports.Builder.prototype.prependInt64 = function(x) {
    this.prep(8, 0);
    this.placeInt64(x);
};

exports.Builder.prototype.prependUInt64 = function(x) {
    this.prep(8, 0);
    this.placeUInt64(x);
};

exports.Builder.prototype.prependFloat32 = function(x) {
    this.prep(4, 0);
    this.placeFloat32(x);
};

exports.Builder.prototype.prependFloat64 = function(x) {
    this.prep(8, 0);
    this.placeFloat64(x);
};

exports.Builder.prototype.prependOffset = function(o) {
    o = o|0;
    this.prep(4, 0);
    if (o > this.getOffset())
    {
        throw new Error('invalid offset');
    }

    o = this.getOffset() - o + 4;
    this.placeInt32(o);
};

///////////////

exports.Builder.prototype._nested = function(o) {
    if(o != this.getOffset()) {
        throw new Error('FlatBuffers: struct must be serialized inline');
    }  
};

exports.Builder.prototype._notNested = function() {
    if(this._vtable != null) {
        throw new Error('FlatBuffers: object serialization must not be nested.');
    }  
};

exports.Builder.prototype._findExistingVTable = function() {
    // search for an existing vtable that matches the current
    var existingVtable = 0;
    var skipToNext = false;
    for(var i = 0; i < this._vtablesCount; ++i) {
        var vt1 = this._buffer.length - this._vtables[i];
        var vt2 = this._head;
        var len = this._buffer.readInt16LE(vt1);
        if (len == this._buffer.readInt16LE(vt2))
        {
            for(var j = 2; j < len; j += 2) {
                if(this._buffer.readInt16LE(vt1 + j) != this._buffer.readInt16LE(vt2 + j)) {
                    skipToNext = true;
                    break;
                }
            }
            if(!skipToNext) {
                existingVtable = this._vtables[i];
                break;
            }           
        }      
    }
    return existingVtable;
};

/*
 * Starts writing a vector
 */
exports.Builder.prototype.startVector = function(elemSize, count, alignment) {
    this._notNested();
    this.prep(4, elemSize * count);
    this.prep(alignment, elemSize * count);
    this._vectorElements = count;
    return this.getOffset();
};

/*
 * completes the writing of a vector
 */
exports.Builder.prototype.endVector = function() {
    this.prependInt32(this._vectorElements|0);
    return this.getOffset();
};

/*
 * Writes a vtable slot from the current offset
 */
exports.Builder.prototype.writeSlot = function(o) {
    this._vtable[o] = this.getOffset();
};

/*
 * start writing an object
 */
exports.Builder.prototype.startObject = function(numFields) {
    this._notNested();
    this._vtable = new Uint32Array(numFields|0);
    this._objectStart = this.getOffset();
    //this._minAlign = 1; // should we reset the alignment?
};

/*
 * completes an object
 */
exports.Builder.prototype.endObject = function() {
    if (this._vtable === null)
    {
        throw new Error('not in an object');
    }
    this.prependInt32(0);
    var vtableLoc = this.getOffset();
    for(var i = this._vtable.length - 1; i >= 0; --i) {
        var off = (this._vtable[i] != 0 ? vtableLoc - this._vtable[i] : 0);
        this.prependInt16(off);
    }
    var standardFields = 2;
    this.prependInt16(vtableLoc - this._objectStart);
    this.prependInt16((this._vtable.length + standardFields) * 2);   // sizeof int16
    
    // search for an existing vtable that matches the current
    var existingVtable = this._findExistingVTable();
    
    if (existingVtable != 0) {
        // found a vtable
        this._head = this._buffer.length - vtableLoc;
        this._buffer.writeInt32LE(existingVtable - vtableLoc, this._head);
    }
    else {
        // no vtable
        if (this._vtables.length == this._vtablesCount) {
            var newVtables = new Int32Array(this._vtablesCount * 2);
            newVtables.set(this._vtables);
            this._vtables = newVtables;
        }
        this._vtables[this._vtablesCount++] = this.getOffset();
        this._buffer.writeInt32LE(this.getOffset() - vtableLoc, this._buffer.length - vtableLoc);
    }
    
    this._vtable = null;
    return vtableLoc;
};


/*
 * finish the buffer
 */
exports.Builder.prototype.finish = function (root) {
    this.prep(this._minAlign, 4);
    this.prependOffset(root|0);
};

/*
 * creates a string in the buffer, returning the position of the string
 */
exports.Builder.prototype.createString = function (x) {
    this._notNested();
    var str;
    if (typeof x === "string") {
        str = x;
    } else {
        str = String(x);
    }

    var utf8 = new Buffer(str, 'utf8');
    // prepare for string + null
    this.prependInt8(0); // null terminate
    var len = utf8.length;
    this.startVector(1, len, 1);    
    utf8.copy(this._buffer, this._head - len)
    this._head -= len;
    return this.endVector(len);
};




/*
 * Slot functions; writes x and adds offset to vtable-slot o
 */
exports.Builder.prototype.addBool = function (o, x, d) {
    x = (x|0 ? 1 : 0);
    d = (d|0 ? 1 : 0);
    if (x != d) {
        this.prependBool(x);
        this.writeSlot(o|0);
    }
};

exports.Builder.prototype.addInt8 = function (o, x, d) {
    x = x|0;
    d = d|0;
    if (x != d) {
        this.prependInt8(x);
        this.writeSlot(o|0);
    }
};

exports.Builder.prototype.addInt16 = function (o, x, d) {
    x = x|0;
    d = d|0;
    if (x != d) {
        this.prependInt16(x);
        this.writeSlot(o|0);
    }
};

exports.Builder.prototype.addUInt16 = function (o, x, d) {
    x = x|0;
    d = d|0;
    if (x != d) {
        this.prependUInt16(x);
        this.writeSlot(o|0);
    }
};

exports.Builder.prototype.addInt32 = function (o, x, d) {
    x = x|0;
    d = d|0;
    if (x != d) {
        this.prependInt32(x);
        this.writeSlot(o|0);
    }
};

exports.Builder.prototype.addUInt32 = function (o, x, d) {
    x = x|0;
    d = d|0;
    if (x != d) {
        this.prependUInt32(x);
        this.writeSlot(o|0);
    }
}

exports.Builder.prototype.addInt64 = function (o, x, d) {
    x = x|0;
    d = d|0;
    if (x != d) {
        this.prependInt64(x);
        this.writeSlot(o|0);
    }
};

exports.Builder.prototype.addUInt64 = function (o, x, d) {
    x = x|0;
    d = d|0;
    if (x != d) {
        this.prependUInt64(x);
        this.writeSlot(o|0);
    }
};

exports.Builder.prototype.addFloat32 = function (o, x, d) {
    x = +(x);
    d = +(d);
    if (x != d) {
        this.prependFloat32(x);
        this.writeSlot(o|0);
    }
};

exports.Builder.prototype.addFloat64 = function (o, x, d) {
    x = +(x);
    d = +(d);
    if (x != d) {
        this.prependFloat64(x);
        this.writeSlot(o|0);
    }
};

exports.Builder.prototype.addStruct = function (voff, x, d) {
    x = x|0; d = d|0;
    if (x != d) {
        this._nested(x);
        this.writeSlot(voff);
    }
};

exports.Builder.prototype.addString = function (voff, x, d) {
    x = x|0; d = d|0;
    if (x != d) {
        this.prependInt32(x);
        this.writeSlot(voff);
    }
};

exports.Builder.prototype.addOffset = function(voff, x, d) {
    x = x|0; d = d|0;
    if (x != d) {
        this.prependOffset(x);
        this.writeSlot(voff);
    }
};

/*
 * Buffer Reader
 */
exports.Reader = function(buffer) {
   this._buffer = buffer;  
 };
 
exports.Reader.constructor = exports.Reader;
 
exports.Reader.prototype.getBuffer = function() {
  return this._buffer;  
};
 
exports.Reader.prototype.getInt8 = function(offset) {
    return this._buffer.readInt8(offset|0);
};

exports.Reader.prototype.getInt16 = function(offset) {
    return this._buffer.readInt16LE(offset|0);
};

exports.Reader.prototype.getUInt16 = function(offset) {
    return this._buffer.readUInt16LE(offset|0);
};

exports.Reader.prototype.getInt32 = function(offset) {
    return this._buffer.readInt32LE(offset|0);
};

exports.Reader.prototype.getUInt32 = function(offset) {
    return this._buffer.readUInt32LE(offset|0);
};

exports.Reader.prototype.getInt64 = function(offset) {
    return this._buffer.readInt64LE(offset|0);
};

exports.Reader.prototype.getUInt64 = function(offset) {
    return this._buffer.readUInt64LE(offset|0);
};

exports.Reader.prototype.getFloat32 = function(offset) {
    return this._buffer.readFloatLE(offset|0);
};

exports.Reader.prototype.getFloat64 = function(offset) {
    return this._buffer.readDoubleLE(offset|0);
};

exports.Reader.prototype.getString = function(offset) {
    offset = (offset|0);
    offset += this.getInt32(offset);
    var len = this.getInt32(offset);
    var startPos = offset + 4;
    return this._buffer.toString('utf8', startPos, startPos + len);
};


/*
 * Wraps a Flatbuffer and provides read access to the data
 */
exports.Table = function(fb, pos) {
  this._fb = fb;
  this._pos = pos|0;  
};

exports.Table.prototype.constructor = exports.Table;

exports.Table.prototype.getPos = function() {
    return this._pos;
}

exports.Table.prototype.getBuffer = function() {
    return this._fb;
}

exports.Table.prototype.getOffset = function(vtableOffset) {
    var vtable = this._pos - this._fb.getInt32(this._pos);
    var vtableEnd = this._fb.getInt16(vtable);
    if (vtableOffset < vtableEnd) {
        return this._fb.getInt16(vtable + vtableOffset);
    }
    return 0;
};

// Initialize any Table-derived type to point to the union at the given offset.
exports.Table.prototype.getUnion = function(obj, offset) {
    offset += this._pos;
    obj._pos = offset + this._fb.getInt32(offset);
    obj._fb = this._fb;
    return obj;
};

////////////
exports.Table.prototype.getString = function(offset) {
    offset = (offset|0);
    return this._fb.getString(offset);
};

exports.Table.prototype.getVector = function(offset) {
    offset = (offset|0) + this._pos;
    return offset + this._fb.getInt32(offset) + 4;                  
};

exports.Table.prototype.getVectorLen = function(offset) {
    offset = (offset|0) + this._pos;
    offset += this._fb.getInt32(offset);
    return this._fb.getInt32(offset);          
};


