"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.ByteBuffer = void 0;
var constants_1 = require("./constants");
var long_1 = require("./long");
var utils_1 = require("./utils");
var encoding_1 = require("./encoding");
var ByteBuffer = /** @class */ (function () {
    /**
     * Create a new ByteBuffer with a given array of bytes (`Uint8Array`)
     */
    function ByteBuffer(bytes_) {
        this.bytes_ = bytes_;
        this.position_ = 0;
    }
    /**
     * Create and allocate a new ByteBuffer with a given size.
     */
    ByteBuffer.allocate = function (byte_size) {
        return new ByteBuffer(new Uint8Array(byte_size));
    };
    ByteBuffer.prototype.clear = function () {
        this.position_ = 0;
    };
    /**
     * Get the underlying `Uint8Array`.
     */
    ByteBuffer.prototype.bytes = function () {
        return this.bytes_;
    };
    /**
     * Get the buffer's position.
     */
    ByteBuffer.prototype.position = function () {
        return this.position_;
    };
    /**
     * Set the buffer's position.
     */
    ByteBuffer.prototype.setPosition = function (position) {
        this.position_ = position;
    };
    /**
     * Get the buffer's capacity.
     */
    ByteBuffer.prototype.capacity = function () {
        return this.bytes_.length;
    };
    ByteBuffer.prototype.readInt8 = function (offset) {
        return this.readUint8(offset) << 24 >> 24;
    };
    ByteBuffer.prototype.readUint8 = function (offset) {
        return this.bytes_[offset];
    };
    ByteBuffer.prototype.readInt16 = function (offset) {
        return this.readUint16(offset) << 16 >> 16;
    };
    ByteBuffer.prototype.readUint16 = function (offset) {
        return this.bytes_[offset] | this.bytes_[offset + 1] << 8;
    };
    ByteBuffer.prototype.readInt32 = function (offset) {
        return this.bytes_[offset] | this.bytes_[offset + 1] << 8 | this.bytes_[offset + 2] << 16 | this.bytes_[offset + 3] << 24;
    };
    ByteBuffer.prototype.readUint32 = function (offset) {
        return this.readInt32(offset) >>> 0;
    };
    ByteBuffer.prototype.readInt64 = function (offset) {
        return new long_1.Long(this.readInt32(offset), this.readInt32(offset + 4));
    };
    ByteBuffer.prototype.readUint64 = function (offset) {
        return new long_1.Long(this.readUint32(offset), this.readUint32(offset + 4));
    };
    ByteBuffer.prototype.readFloat32 = function (offset) {
        utils_1.int32[0] = this.readInt32(offset);
        return utils_1.float32[0];
    };
    ByteBuffer.prototype.readFloat64 = function (offset) {
        utils_1.int32[utils_1.isLittleEndian ? 0 : 1] = this.readInt32(offset);
        utils_1.int32[utils_1.isLittleEndian ? 1 : 0] = this.readInt32(offset + 4);
        return utils_1.float64[0];
    };
    ByteBuffer.prototype.writeInt8 = function (offset, value) {
        this.bytes_[offset] = value;
    };
    ByteBuffer.prototype.writeUint8 = function (offset, value) {
        this.bytes_[offset] = value;
    };
    ByteBuffer.prototype.writeInt16 = function (offset, value) {
        this.bytes_[offset] = value;
        this.bytes_[offset + 1] = value >> 8;
    };
    ByteBuffer.prototype.writeUint16 = function (offset, value) {
        this.bytes_[offset] = value;
        this.bytes_[offset + 1] = value >> 8;
    };
    ByteBuffer.prototype.writeInt32 = function (offset, value) {
        this.bytes_[offset] = value;
        this.bytes_[offset + 1] = value >> 8;
        this.bytes_[offset + 2] = value >> 16;
        this.bytes_[offset + 3] = value >> 24;
    };
    ByteBuffer.prototype.writeUint32 = function (offset, value) {
        this.bytes_[offset] = value;
        this.bytes_[offset + 1] = value >> 8;
        this.bytes_[offset + 2] = value >> 16;
        this.bytes_[offset + 3] = value >> 24;
    };
    ByteBuffer.prototype.writeInt64 = function (offset, value) {
        this.writeInt32(offset, value.low);
        this.writeInt32(offset + 4, value.high);
    };
    ByteBuffer.prototype.writeUint64 = function (offset, value) {
        this.writeUint32(offset, value.low);
        this.writeUint32(offset + 4, value.high);
    };
    ByteBuffer.prototype.writeFloat32 = function (offset, value) {
        utils_1.float32[0] = value;
        this.writeInt32(offset, utils_1.int32[0]);
    };
    ByteBuffer.prototype.writeFloat64 = function (offset, value) {
        utils_1.float64[0] = value;
        this.writeInt32(offset, utils_1.int32[utils_1.isLittleEndian ? 0 : 1]);
        this.writeInt32(offset + 4, utils_1.int32[utils_1.isLittleEndian ? 1 : 0]);
    };
    /**
     * Return the file identifier.   Behavior is undefined for FlatBuffers whose
     * schema does not include a file_identifier (likely points at padding or the
     * start of a the root vtable).
     */
    ByteBuffer.prototype.getBufferIdentifier = function () {
        if (this.bytes_.length < this.position_ + constants_1.SIZEOF_INT +
            constants_1.FILE_IDENTIFIER_LENGTH) {
            throw new Error('FlatBuffers: ByteBuffer is too short to contain an identifier.');
        }
        var result = "";
        for (var i = 0; i < constants_1.FILE_IDENTIFIER_LENGTH; i++) {
            result += String.fromCharCode(this.readInt8(this.position_ + constants_1.SIZEOF_INT + i));
        }
        return result;
    };
    /**
     * Look up a field in the vtable, return an offset into the object, or 0 if the
     * field is not present.
     */
    ByteBuffer.prototype.__offset = function (bb_pos, vtable_offset) {
        var vtable = bb_pos - this.readInt32(bb_pos);
        return vtable_offset < this.readInt16(vtable) ? this.readInt16(vtable + vtable_offset) : 0;
    };
    /**
     * Initialize any Table-derived type to point to the union at the given offset.
     */
    ByteBuffer.prototype.__union = function (t, offset) {
        t.bb_pos = offset + this.readInt32(offset);
        t.bb = this;
        return t;
    };
    /**
     * Create a JavaScript string from UTF-8 data stored inside the FlatBuffer.
     * This allocates a new string and converts to wide chars upon each access.
     *
     * To avoid the conversion to UTF-16, pass Encoding.UTF8_BYTES as
     * the "optionalEncoding" argument. This is useful for avoiding conversion to
     * and from UTF-16 when the data will just be packaged back up in another
     * FlatBuffer later on.
     *
     * @param offset
     * @param opt_encoding Defaults to UTF16_STRING
     */
    ByteBuffer.prototype.__string = function (offset, opt_encoding) {
        offset += this.readInt32(offset);
        var length = this.readInt32(offset);
        var result = '';
        var i = 0;
        offset += constants_1.SIZEOF_INT;
        if (opt_encoding === encoding_1.Encoding.UTF8_BYTES) {
            return this.bytes_.subarray(offset, offset + length);
        }
        while (i < length) {
            var codePoint = void 0;
            // Decode UTF-8
            var a = this.readUint8(offset + i++);
            if (a < 0xC0) {
                codePoint = a;
            }
            else {
                var b = this.readUint8(offset + i++);
                if (a < 0xE0) {
                    codePoint =
                        ((a & 0x1F) << 6) |
                            (b & 0x3F);
                }
                else {
                    var c = this.readUint8(offset + i++);
                    if (a < 0xF0) {
                        codePoint =
                            ((a & 0x0F) << 12) |
                                ((b & 0x3F) << 6) |
                                (c & 0x3F);
                    }
                    else {
                        var d = this.readUint8(offset + i++);
                        codePoint =
                            ((a & 0x07) << 18) |
                                ((b & 0x3F) << 12) |
                                ((c & 0x3F) << 6) |
                                (d & 0x3F);
                    }
                }
            }
            // Encode UTF-16
            if (codePoint < 0x10000) {
                result += String.fromCharCode(codePoint);
            }
            else {
                codePoint -= 0x10000;
                result += String.fromCharCode((codePoint >> 10) + 0xD800, (codePoint & ((1 << 10) - 1)) + 0xDC00);
            }
        }
        return result;
    };
    /**
     * Handle unions that can contain string as its member, if a Table-derived type then initialize it,
     * if a string then return a new one
     *
     * WARNING: strings are immutable in JS so we can't change the string that the user gave us, this
     * makes the behaviour of __union_with_string different compared to __union
     */
    ByteBuffer.prototype.__union_with_string = function (o, offset) {
        if (typeof o === 'string') {
            return this.__string(offset);
        }
        return this.__union(o, offset);
    };
    /**
     * Retrieve the relative offset stored at "offset"
     */
    ByteBuffer.prototype.__indirect = function (offset) {
        return offset + this.readInt32(offset);
    };
    /**
     * Get the start of data of a vector whose offset is stored at "offset" in this object.
     */
    ByteBuffer.prototype.__vector = function (offset) {
        return offset + this.readInt32(offset) + constants_1.SIZEOF_INT; // data starts after the length
    };
    /**
     * Get the length of a vector whose offset is stored at "offset" in this object.
     */
    ByteBuffer.prototype.__vector_len = function (offset) {
        return this.readInt32(offset + this.readInt32(offset));
    };
    ByteBuffer.prototype.__has_identifier = function (ident) {
        if (ident.length != constants_1.FILE_IDENTIFIER_LENGTH) {
            throw new Error('FlatBuffers: file identifier must be length ' +
                constants_1.FILE_IDENTIFIER_LENGTH);
        }
        for (var i = 0; i < constants_1.FILE_IDENTIFIER_LENGTH; i++) {
            if (ident.charCodeAt(i) != this.readInt8(this.position() + constants_1.SIZEOF_INT + i)) {
                return false;
            }
        }
        return true;
    };
    /**
     * A helper function to avoid generated code depending on this file directly.
     */
    ByteBuffer.prototype.createLong = function (low, high) {
        return long_1.Long.create(low, high);
    };
    /**
     * A helper function for generating list for obj api
     */
    ByteBuffer.prototype.createScalarList = function (listAccessor, listLength) {
        var ret = [];
        for (var i = 0; i < listLength; ++i) {
            if (listAccessor(i) !== null) {
                ret.push(listAccessor(i));
            }
        }
        return ret;
    };
    /**
     * A helper function for generating list for obj api
     * @param listAccessor function that accepts an index and return data at that index
     * @param listLength listLength
     * @param res result list
     */
    ByteBuffer.prototype.createObjList = function (listAccessor, listLength) {
        var ret = [];
        for (var i = 0; i < listLength; ++i) {
            var val = listAccessor(i);
            if (val !== null) {
                ret.push(val.unpack());
            }
        }
        return ret;
    };
    return ByteBuffer;
}());
exports.ByteBuffer = ByteBuffer;
