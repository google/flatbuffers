/// @file
/// @addtogroup flatbuffers_javascript_api
/// @{
/// @cond FLATBUFFERS_INTERNAL
/**
 * @const
 * @namespace
 */
//var flatbuffers = {};
"use strict";
exports.__esModule = true;
////////////////////////////////////////////////////////////////////////////////
/**
 * @constructor
 * @param {number} high
 * @param {number} low
 */
exports["default"] = flatbuffers;
var flatbuffers;
(function (flatbuffers) {
    /**
   * @type {number}
   * @const
   */
    var SIZEOF_SHORT = 2;
    /**
     * @type {number}
     * @const
     */
    var SIZEOF_INT = 4;
    /**
     * @type {number}
     * @const
     */
    var FILE_IDENTIFIER_LENGTH = 4;
    /**
     * @enum {number}
     */
    (function (Encoding) {
        Encoding[Encoding["UTF8_BYTES"] = 1] = "UTF8_BYTES";
        Encoding[Encoding["UTF16_STRING"] = 2] = "UTF16_STRING";
    })(flatbuffers.Encoding || (flatbuffers.Encoding = {}));
    var Encoding = flatbuffers.Encoding;
    ;
    /**
     * @type {Int32Array}
     * @const
     */
    var int32 = new Int32Array(2);
    /**
     * @type {Float32Array}
     * @const
     */
    var float32 = new Float32Array(int32.buffer);
    /**
     * @type {Float64Array}
     * @const
     */
    var float64 = new Float64Array(int32.buffer);
    /**
     * @type {boolean}
     * @const
     */
    var isLittleEndian = new Uint16Array(new Uint8Array([1, 0]).buffer)[0] === 1;
    /**
   * @typedef {{
   *   bb: flatbuffers.ByteBuffer,
   *   bb_pos: number
   * }}
   */
    var Table = (function () {
        function Table() {
        }
        return Table;
    }());
    flatbuffers.Table = Table;
    ;
    var Long = (function () {
        function Long(low, high) {
            /**
             * @returns {number}
             */
            this.toFloat64 = function () {
                return this.low + this.high * 0x100000000;
            };
            /**
             * @param {flatbuffers.Long} other
             * @returns {boolean}
             */
            this.equals = function (other) {
                return this.low == other.low && this.high == other.high;
            };
            /**
             * @type {number}
             * @const
             */
            this.low = low | 0;
            /**
             * @type {number}
             * @const
             */
            this.high = high | 0;
        }
        ;
        return Long;
    }());
    /**
     * @param {number} high
     * @param {number} low
     * @returns {flatbuffers.Long}
     */
    Long.create = function (low, high) {
        // Special-case zero to avoid GC overhead for default values
        return low == 0 && high == 0 ? Long.ZERO : new flatbuffers.Long(low, high);
    };
    flatbuffers.Long = Long;
    (function (Long) {
        Long.ZERO = new flatbuffers.Long(0, 0);
    })(Long = flatbuffers.Long || (flatbuffers.Long = {}));
    /// @endcond
    ////////////////////////////////////////////////////////////////////////////////
    /**
     * Create a FlatBufferBuilder.
     *
     * @constructor
     * @param {number=} initial_size
     */
    var Builder = (function () {
        function Builder(initial_size) {
            /**
             * In order to save space, fields that are set to their default value
             * don't get serialized into the buffer. Forcing defaults provides a
             * way to manually disable this optimization.
             *
             * @param {boolean} forceDefaults true always serializes default values
             */
            this.forceDefaults = function (forceDefaults) {
                this.force_defaults = forceDefaults;
            };
            /**
             * Get the ByteBuffer representing the FlatBuffer. Only call this after you've
             * called finish(). The actual data starts at the ByteBuffer's current position,
             * not necessarily at 0.
             *
             * @returns {flatbuffers.ByteBuffer}
             */
            this.dataBuffer = function () {
                return this.bb;
            };
            /**
             * Get the bytes representing the FlatBuffer. Only call this after you've
             * called finish().
             *
             * @returns {Uint8Array}
             */
            this.asUint8Array = function () {
                return this.bb.bytes().subarray(this.bb.position(), this.bb.position() + this.offset());
            };
            /// @cond FLATBUFFERS_INTERNAL
            /**
             * Prepare to write an element of `size` after `additional_bytes` have been
             * written, e.g. if you write a string, you need to align such the int length
             * field is aligned to 4 bytes, and the string data follows it directly. If all
             * you need to do is alignment, `additional_bytes` will be 0.
             *
             * @param {number} size This is the of the new element to write
             * @param {number} additional_bytes The padding size
             */
            this.prep = function (size, additional_bytes) {
                // Track the biggest thing we've ever aligned to.
                if (size > this.minalign) {
                    this.minalign = size;
                }
                // Find the amount of alignment needed such that `size` is properly
                // aligned after `additional_bytes`
                var align_size = ((~(this.bb.capacity() - this.space + additional_bytes)) + 1) & (size - 1);
                // Reallocate the buffer if needed.
                while (this.space < align_size + size + additional_bytes) {
                    var old_buf_size = this.bb.capacity();
                    this.bb = this.Builder.growByteBuffer(this.bb);
                    this.space += this.bb.capacity() - old_buf_size;
                }
                this.pad(align_size);
            };
            /**
             * @param {number} byte_size
             */
            this.pad = function (byte_size) {
                for (var i = 0; i < byte_size; i++) {
                    this.bb.writeInt8(--this.space, 0);
                }
            };
            /**
             * @param {number} value
             */
            this.writeInt8 = function (value) {
                this.bb.writeInt8(this.space -= 1, value);
            };
            /**
             * @param {number} value
             */
            this.writeInt16 = function (value) {
                this.bb.writeInt16(this.space -= 2, value);
            };
            /**
             * @param {number} value
             */
            this.writeInt32 = function (value) {
                this.bb.writeInt32(this.space -= 4, value);
            };
            /**
             * @param {flatbuffers.Long} value
             */
            this.writeInt64 = function (value) {
                this.bb.writeInt64(this.space -= 8, value);
            };
            /**
             * @param {number} value
             */
            this.writeFloat32 = function (value) {
                this.bb.writeFloat32(this.space -= 4, value);
            };
            /**
             * @param {number} value
             */
            this.writeFloat64 = function (value) {
                this.bb.writeFloat64(this.space -= 8, value);
            };
            /// @endcond
            /**
             * Add an `int8` to the buffer, properly aligned, and grows the buffer (if necessary).
             * @param {number} value The `int8` to add the the buffer.
             */
            this.addInt8 = function (value) {
                this.prep(1, 0);
                this.writeInt8(value);
            };
            /**
             * Add an `int16` to the buffer, properly aligned, and grows the buffer (if necessary).
             * @param {number} value The `int16` to add the the buffer.
             */
            this.addInt16 = function (value) {
                this.prep(2, 0);
                this.writeInt16(value);
            };
            /**
             * Add an `int32` to the buffer, properly aligned, and grows the buffer (if necessary).
             * @param {number} value The `int32` to add the the buffer.
             */
            this.addInt32 = function (value) {
                this.prep(4, 0);
                this.writeInt32(value);
            };
            /**
             * Add an `int64` to the buffer, properly aligned, and grows the buffer (if necessary).
             * @param {flatbuffers.Long} value The `int64` to add the the buffer.
             */
            this.addInt64 = function (value) {
                this.prep(8, 0);
                this.writeInt64(value);
            };
            /**
             * Add a `float32` to the buffer, properly aligned, and grows the buffer (if necessary).
             * @param {number} value The `float32` to add the the buffer.
             */
            this.addFloat32 = function (value) {
                this.prep(4, 0);
                this.writeFloat32(value);
            };
            /**
             * Add a `float64` to the buffer, properly aligned, and grows the buffer (if necessary).
             * @param {number} value The `float64` to add the the buffer.
             */
            this.addFloat64 = function (value) {
                this.prep(8, 0);
                this.writeFloat64(value);
            };
            /// @cond FLATBUFFERS_INTERNAL
            /**
             * @param {number} voffset
             * @param {number} value
             * @param {number} defaultValue
             */
            this.addFieldInt8 = function (voffset, value, defaultValue) {
                if (this.force_defaults || value != defaultValue) {
                    this.addInt8(value);
                    this.slot(voffset);
                }
            };
            /**
             * @param {number} voffset
             * @param {number} value
             * @param {number} defaultValue
             */
            this.addFieldInt16 = function (voffset, value, defaultValue) {
                if (this.force_defaults || value != defaultValue) {
                    this.addInt16(value);
                    this.slot(voffset);
                }
            };
            /**
             * @param {number} voffset
             * @param {number} value
             * @param {number} defaultValue
             */
            this.addFieldInt32 = function (voffset, value, defaultValue) {
                if (this.force_defaults || value != defaultValue) {
                    this.addInt32(value);
                    this.slot(voffset);
                }
            };
            /**
             * @param {number} voffset
             * @param {flatbuffers.Long} value
             * @param {flatbuffers.Long} defaultValue
             */
            this.addFieldInt64 = function (voffset, value, defaultValue) {
                if (this.force_defaults || !value.equals(defaultValue)) {
                    this.addInt64(value);
                    this.slot(voffset);
                }
            };
            /**
             * @param {number} voffset
             * @param {number} value
             * @param {number} defaultValue
             */
            this.addFieldFloat32 = function (voffset, value, defaultValue) {
                if (this.force_defaults || value != defaultValue) {
                    this.addFloat32(value);
                    this.slot(voffset);
                }
            };
            /**
             * @param {number} voffset
             * @param {number} value
             * @param {number} defaultValue
             */
            this.addFieldFloat64 = function (voffset, value, defaultValue) {
                if (this.force_defaults || value != defaultValue) {
                    this.addFloat64(value);
                    this.slot(voffset);
                }
            };
            /**
             * @param {number} voffset
             * @param {flatbuffers.Offset} value
             * @param {flatbuffers.Offset} defaultValue
             */
            this.addFieldOffset = function (voffset, value, defaultValue) {
                if (this.force_defaults || value != defaultValue) {
                    this.addOffset(value);
                    this.slot(voffset);
                }
            };
            /**
             * Structs are stored inline, so nothing additional is being added. `d` is always 0.
             *
             * @param {number} voffset
             * @param {flatbuffers.Offset} value
             * @param {flatbuffers.Offset} defaultValue
             */
            this.addFieldStruct = function (voffset, value, defaultValue) {
                if (value != defaultValue) {
                    this.nested(value);
                    this.slot(voffset);
                }
            };
            /**
             * Structures are always stored inline, they need to be created right
             * where they're used.  You'll get this assertion failure if you
             * created it elsewhere.
             *
             * @param {flatbuffers.Offset} obj The offset of the created object
             */
            this.nested = function (obj) {
                if (obj != this.offset()) {
                    throw new Error('FlatBuffers: struct must be serialized inline.');
                }
            };
            /**
             * Should not be creating any other object, string or vector
             * while an object is being constructed
             */
            this.notNested = function () {
                if (this.isNested) {
                    throw new Error('FlatBuffers: object serialization must not be nested.');
                }
            };
            /**
             * Set the current vtable at `voffset` to the current location in the buffer.
             *
             * @param {number} voffset
             */
            this.slot = function (voffset) {
                this.vtable[voffset] = this.offset();
            };
            /**
             * @returns {flatbuffers.Offset} Offset relative to the end of the buffer.
             */
            this.offset = function () {
                return this.bb.capacity() - this.space;
            };
            /**
             * Doubles the size of the backing ByteBuffer and copies the old data towards
             * the end of the new buffer (since we build the buffer backwards).
             *
             * @param {flatbuffers.ByteBuffer} bb The current buffer with the existing data
             * @returns {flatbuffers.ByteBuffer} A new byte buffer with the old data copied
             * to it. The data is located at the end of the buffer.
             */
            this.growByteBuffer = function (bb) {
                var old_buf_size = bb.capacity();
                // Ensure we don't grow beyond what fits in an int.
                if (old_buf_size & 0xC0000000) {
                    throw new Error('FlatBuffers: cannot grow buffer beyond 2 gigabytes.');
                }
                var new_buf_size = old_buf_size << 1;
                var nbb = this.ByteBuffer.allocate(new_buf_size);
                nbb.setPosition(new_buf_size - old_buf_size);
                nbb.bytes().set(bb.bytes(), new_buf_size - old_buf_size);
                return nbb;
            };
            /// @endcond
            /**
             * Adds on offset, relative to where it will be written.
             *
             * @param {flatbuffers.Offset} offset The offset to add.
             */
            this.addOffset = function (offset) {
                this.prep(SIZEOF_INT, 0); // Ensure alignment is already done.
                this.writeInt32(this.offset() - offset + SIZEOF_INT);
            };
            /// @cond FLATBUFFERS_INTERNAL
            /**
             * Start encoding a new object in the buffer.  Users will not usually need to
             * call this directly. The FlatBuffers compiler will generate helper methods
             * that call this method internally.
             *
             * @param {number} numfields
             */
            this.startObject = function (numfields) {
                this.notNested();
                if (this.vtable == null) {
                    this.vtable = [];
                }
                this.vtable_in_use = numfields;
                for (var i = 0; i < numfields; i++) {
                    this.vtable[i] = 0; // This will push additional elements as needed
                }
                this.isNested = true;
                this.object_start = this.offset();
            };
            /**
             * Finish off writing the object that is under construction.
             *
             * @returns {flatbuffers.Offset} The offset to the object inside `dataBuffer`
             */
            this.endObject = function () {
                if (this.vtable == null || !this.isNested) {
                    throw new Error('FlatBuffers: endObject called without startObject');
                }
                this.addInt32(0);
                var vtableloc = this.offset();
                // Write out the current vtable.
                for (var i = this.vtable_in_use - 1; i >= 0; i--) {
                    // Offset relative to the start of the table.
                    this.addInt16(this.vtable[i] != 0 ? vtableloc - this.vtable[i] : 0);
                }
                var standard_fields = 2; // The fields below:
                this.addInt16(vtableloc - this.object_start);
                this.addInt16((this.vtable_in_use + standard_fields) * SIZEOF_SHORT);
                // Search for an existing vtable that matches the current one.
                var existing_vtable = 0;
                outer_loop: for (var i = 0; i < this.vtables.length; i++) {
                    var vt1 = this.bb.capacity() - this.vtables[i];
                    var vt2 = this.space;
                    var len = this.bb.readInt16(vt1);
                    if (len == this.bb.readInt16(vt2)) {
                        for (var j = SIZEOF_SHORT; j < len; j += SIZEOF_SHORT) {
                            if (this.bb.readInt16(vt1 + j) != this.bb.readInt16(vt2 + j)) {
                                continue outer_loop;
                            }
                        }
                        existing_vtable = this.vtables[i];
                        break;
                    }
                }
                if (existing_vtable) {
                    // Found a match:
                    // Remove the current vtable.
                    this.space = this.bb.capacity() - vtableloc;
                    // Point table to existing vtable.
                    this.bb.writeInt32(this.space, existing_vtable - vtableloc);
                }
                else {
                    // No match:
                    // Add the location of the current vtable to the list of vtables.
                    this.vtables.push(this.offset());
                    // Point table to current vtable.
                    this.bb.writeInt32(this.bb.capacity() - vtableloc, this.offset() - vtableloc);
                }
                this.isNested = false;
                return vtableloc;
            };
            /// @endcond
            /**
             * Finalize a buffer, poiting to the given `root_table`.
             *
             * @param {flatbuffers.Offset} root_table
             * @param {string=} file_identifier
             */
            this.finish = function (root_table, file_identifier) {
                if (file_identifier) {
                    this.prep(this.minalign, SIZEOF_INT +
                        FILE_IDENTIFIER_LENGTH);
                    if (file_identifier.length != FILE_IDENTIFIER_LENGTH) {
                        throw new Error('FlatBuffers: file identifier must be length ' +
                            FILE_IDENTIFIER_LENGTH);
                    }
                    for (var i = FILE_IDENTIFIER_LENGTH - 1; i >= 0; i--) {
                        this.writeInt8(file_identifier.charCodeAt(i));
                    }
                }
                this.prep(this.minalign, SIZEOF_INT);
                this.addOffset(root_table);
                this.bb.setPosition(this.space);
            };
            /// @cond FLATBUFFERS_INTERNAL
            /**
             * This checks a required field has been set in a given table that has
             * just been constructed.
             *
             * @param {flatbuffers.Offset} table
             * @param {number} field
             */
            this.requiredField = function (table, field) {
                var table_start = this.bb.capacity() - table;
                var vtable_start = table_start - this.bb.readInt32(table_start);
                var ok = this.bb.readInt16(vtable_start + field) != 0;
                // If this fails, the caller will show what field needs to be set.
                if (!ok) {
                    throw new Error('FlatBuffers: field ' + field + ' must be set');
                }
            };
            /**
             * Start a new array/vector of objects.  Users usually will not call
             * this directly. The FlatBuffers compiler will create a start/end
             * method for vector types in generated code.
             *
             * @param {number} elem_size The size of each element in the array
             * @param {number} num_elems The number of elements in the array
             * @param {number} alignment The alignment of the array
             */
            this.startVector = function (elem_size, num_elems, alignment) {
                this.notNested();
                this.vector_num_elems = num_elems;
                this.prep(SIZEOF_INT, elem_size * num_elems);
                this.prep(alignment, elem_size * num_elems); // Just in case alignment > int.
            };
            /**
             * Finish off the creation of an array and all its elements. The array must be
             * created with `startVector`.
             *
             * @returns {flatbuffers.Offset} The offset at which the newly created array
             * starts.
             */
            this.endVector = function () {
                this.writeInt32(this.vector_num_elems);
                return this.offset();
            };
            /// @endcond
            /**
             * Encode the string `s` in the buffer using UTF-8. If a Uint8Array is passed
             * instead of a string, it is assumed to contain valid UTF-8 encoded data.
             *
             * @param {string|Uint8Array} s The string to encode
             * @return {flatbuffers.Offset} The offset in the buffer where the encoded string starts
             */
            this.createString = function (s) {
                var utf8;
                if (s instanceof Uint8Array) {
                    utf8 = s;
                }
                else {
                    utf8 = [];
                    var i = 0;
                    while (i < s.length) {
                        var codePoint;
                        // Decode UTF-16
                        var a = s.charCodeAt(i++);
                        if (a < 0xD800 || a >= 0xDC00) {
                            codePoint = a;
                        }
                        else {
                            var b = s.charCodeAt(i++);
                            codePoint = (a << 10) + b + (0x10000 - (0xD800 << 10) - 0xDC00);
                        }
                        // Encode UTF-8
                        if (codePoint < 0x80) {
                            utf8.push(codePoint);
                        }
                        else {
                            if (codePoint < 0x800) {
                                utf8.push(((codePoint >> 6) & 0x1F) | 0xC0);
                            }
                            else {
                                if (codePoint < 0x10000) {
                                    utf8.push(((codePoint >> 12) & 0x0F) | 0xE0);
                                }
                                else {
                                    utf8.push(((codePoint >> 18) & 0x07) | 0xF0, ((codePoint >> 12) & 0x3F) | 0x80);
                                }
                                utf8.push(((codePoint >> 6) & 0x3F) | 0x80);
                            }
                            utf8.push((codePoint & 0x3F) | 0x80);
                        }
                    }
                }
                this.addInt8(0);
                this.startVector(1, utf8.length, 1);
                this.bb.setPosition(this.space -= utf8.length);
                for (var i = 0, offset = this.space, bytes = this.bb.bytes(); i < utf8.length; i++) {
                    bytes[offset++] = utf8[i];
                }
                return this.endVector();
            };
            /**
             * A helper function to avoid generated code depending on this file directly.
             *
             * @param {number} low
             * @param {number} high
             * @returns {flatbuffers.Long}
             */
            this.createLong = function (low, high) {
                return flatbuffers.Long.create(low, high);
            };
            if (!initial_size) {
                initial_size = 1024;
            }
            /**
             * @type {flatbuffers.ByteBuffer}
             * @private
             */
            this.bb = ByteBuffer.allocate(initial_size);
            /**
             * Remaining space in the ByteBuffer.
             *
             * @type {number}
             * @private
             */
            this.space = initial_size;
            /**
             * Minimum alignment encountered so far.
             *
             * @type {number}
             * @private
             */
            this.minalign = 1;
            /**
             * The vtable for the current table.
             *
             * @type {Array.<number>}
             * @private
             */
            this.vtable = null;
            /**
             * The amount of fields we're actually using.
             *
             * @type {number}
             * @private
             */
            this.vtable_in_use = 0;
            /**
             * Whether we are currently serializing a table.
             *
             * @type {boolean}
             * @private
             */
            this.isNested = false;
            /**
             * Starting offset of the current struct/table.
             *
             * @type {number}
             * @private
             */
            this.object_start = 0;
            /**
             * List of offsets of all vtables.
             *
             * @type {Array.<number>}
             * @private
             */
            this.vtables = [];
            /**
             * For the current vector being built.
             *
             * @type {number}
             * @private
             */
            this.vector_num_elems = 0;
            /**
             * False omits default values from the serialized data
             *
             * @type {boolean}
             * @private
             */
            this.force_defaults = false;
        }
        ;
        return Builder;
    }());
    flatbuffers.Builder = Builder;
    ////////////////////////////////////////////////////////////////////////////////
    /// @cond FLATBUFFERS_INTERNAL
    /**
     * Create a new ByteBuffer with a given array of bytes (`Uint8Array`).
     *
     * @constructor
     * @param {Uint8Array} bytes
     */
    var ByteBuffer = (function () {
        function ByteBuffer(bytes) {
            /**
             * Get the underlying `Uint8Array`.
             *
             * @returns {Uint8Array}
             */
            this.bytes = function () {
                return this.bytes_;
            };
            /**
             * Get the buffer's position.
             *
             * @returns {number}
             */
            this.position = function () {
                return this.position_;
            };
            /**
             * Set the buffer's position.
             *
             * @param {number} position
             */
            this.setPosition = function (position) {
                this.position_ = position;
            };
            /**
             * Get the buffer's capacity.
             *
             * @returns {number}
             */
            this.capacity = function () {
                return this.bytes_.length;
            };
            /**
             * @param {number} offset
             * @returns {number}
             */
            this.readInt8 = function (offset) {
                return this.readUint8(offset) << 24 >> 24;
            };
            /**
             * @param {number} offset
             * @returns {number}
             */
            this.readUint8 = function (offset) {
                return this.bytes_[offset];
            };
            /**
             * @param {number} offset
             * @returns {number}
             */
            this.readInt16 = function (offset) {
                return this.readUint16(offset) << 16 >> 16;
            };
            /**
             * @param {number} offset
             * @returns {number}
             */
            this.readUint16 = function (offset) {
                return this.bytes_[offset] | this.bytes_[offset + 1] << 8;
            };
            /**
             * @param {number} offset
             * @returns {number}
             */
            this.readInt32 = function (offset) {
                return this.bytes_[offset] | this.bytes_[offset + 1] << 8 | this.bytes_[offset + 2] << 16 | this.bytes_[offset + 3] << 24;
            };
            /**
             * @param {number} offset
             * @returns {number}
             */
            this.readUint32 = function (offset) {
                return this.readInt32(offset) >>> 0;
            };
            /**
             * @param {number} offset
             * @returns {flatbuffers.Long}
             */
            this.readInt64 = function (offset) {
                return new flatbuffers.Long(this.readInt32(offset), this.readInt32(offset + 4));
            };
            /**
             * @param {number} offset
             * @returns {flatbuffers.Long}
             */
            this.readUint64 = function (offset) {
                return new flatbuffers.Long(this.readUint32(offset), this.readUint32(offset + 4));
            };
            /**
             * @param {number} offset
             * @returns {number}
             */
            this.readFloat32 = function (offset) {
                int32[0] = this.readInt32(offset);
                return float32[0];
            };
            /**
             * @param {number} offset
             * @returns {number}
             */
            this.readFloat64 = function (offset) {
                int32[isLittleEndian ? 0 : 1] = this.readInt32(offset);
                int32[isLittleEndian ? 1 : 0] = this.readInt32(offset + 4);
                return float64[0];
            };
            /**
             * @param {number} offset
             * @param {number} value
             */
            this.writeInt8 = function (offset, value) {
                this.bytes_[offset] = value;
            };
            /**
             * @param {number} offset
             * @param {number} value
             */
            this.writeInt16 = function (offset, value) {
                this.bytes_[offset] = value;
                this.bytes_[offset + 1] = value >> 8;
            };
            /**
             * @param {number} offset
             * @param {number} value
             */
            this.writeInt32 = function (offset, value) {
                this.bytes_[offset] = value;
                this.bytes_[offset + 1] = value >> 8;
                this.bytes_[offset + 2] = value >> 16;
                this.bytes_[offset + 3] = value >> 24;
            };
            /**
             * @param {number} offset
             * @param {flatbuffers.Long} value
             */
            this.writeInt64 = function (offset, value) {
                this.writeInt32(offset, value.low);
                this.writeInt32(offset + 4, value.high);
            };
            /**
             * @param {number} offset
             * @param {number} value
             */
            this.writeFloat32 = function (offset, value) {
                float32[0] = value;
                this.writeInt32(offset, int32[0]);
            };
            /**
             * @param {number} offset
             * @param {number} value
             */
            this.writeFloat64 = function (offset, value) {
                float64[0] = value;
                this.writeInt32(offset, int32[isLittleEndian ? 0 : 1]);
                this.writeInt32(offset + 4, int32[isLittleEndian ? 1 : 0]);
            };
            /**
             * Look up a field in the vtable, return an offset into the object, or 0 if the
             * field is not present.
             *
             * @param {number} bb_pos
             * @param {number} vtable_offset
             * @returns {number}
             */
            this.__offset = function (bb_pos, vtable_offset) {
                var vtable = bb_pos - this.readInt32(bb_pos);
                return vtable_offset < this.readInt16(vtable) ? this.readInt16(vtable + vtable_offset) : 0;
            };
            /**
             * Initialize any Table-derived type to point to the union at the given offset.
             *
             * @param {flatbuffers.Table} t
             * @param {number} offset
             * @returns {flatbuffers.Table}
             */
            this.__union = function (t, offset) {
                t.bb_pos = offset + this.readInt32(offset);
                t.bb = this;
                return t;
            };
            /**
             * Create a JavaScript string from UTF-8 data stored inside the FlatBuffer.
             * This allocates a new string and converts to wide chars upon each access.
             *
             * To avoid the conversion to UTF-16, pass flatbuffers.Encoding.UTF8_BYTES as
             * the "optionalEncoding" argument. This is useful for avoiding conversion to
             * and from UTF-16 when the data will just be packaged back up in another
             * FlatBuffer later on.
             *
             * @param {number} offset
             * @param {flatbuffers.Encoding=} optionalEncoding Defaults to UTF16_STRING
             * @returns {string|Uint8Array}
             */
            this.__string = function (offset, optionalEncoding) {
                offset += this.readInt32(offset);
                var length = this.readInt32(offset);
                var result = '';
                var i = 0;
                offset += SIZEOF_INT;
                if (optionalEncoding === Encoding.UTF8_BYTES) {
                    return this.bytes_.subarray(offset, offset + length);
                }
                while (i < length) {
                    var codePoint;
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
             * Retrieve the relative offset stored at "offset"
             * @param {number} offset
             * @returns {number}
             */
            this.__indirect = function (offset) {
                return offset + this.readInt32(offset);
            };
            /**
             * Get the start of data of a vector whose offset is stored at "offset" in this object.
             *
             * @param {number} offset
             * @returns {number}
             */
            this.__vector = function (offset) {
                return offset + this.readInt32(offset) + SIZEOF_INT; // data starts after the length
            };
            /**
             * Get the length of a vector whose offset is stored at "offset" in this object.
             *
             * @param {number} offset
             * @returns {number}
             */
            this.__vector_len = function (offset) {
                return this.readInt32(offset + this.readInt32(offset));
            };
            /**
             * @param {string} ident
             * @returns {boolean}
             */
            this.__has_identifier = function (ident) {
                if (ident.length != FILE_IDENTIFIER_LENGTH) {
                    throw new Error('FlatBuffers: file identifier must be length ' +
                        FILE_IDENTIFIER_LENGTH);
                }
                for (var i = 0; i < FILE_IDENTIFIER_LENGTH; i++) {
                    if (ident.charCodeAt(i) != this.readInt8(this.position_ + SIZEOF_INT + i)) {
                        return false;
                    }
                }
                return true;
            };
            /**
             * A helper function to avoid generated code depending on this file directly.
             *
             * @param {number} low
             * @param {number} high
             * @returns {flatbuffers.Long}
             */
            this.createLong = function (low, high) {
                return flatbuffers.Long.create(low, high);
            };
            /**
             * @type {Uint8Array}
             * @private
             */
            this.bytes_ = bytes;
            /**
             * @type {number}
             * @private
             */
            this.position_ = 0;
        }
        ;
        return ByteBuffer;
    }());
    /**
     * Create and allocate a new ByteBuffer with a given size.
     *
     * @param {number} byte_size
     * @returns {flatbuffers.ByteBuffer}
     */
    ByteBuffer.allocate = function (byte_size) {
        return new flatbuffers.ByteBuffer(new Uint8Array(byte_size));
    };
    flatbuffers.ByteBuffer = ByteBuffer;
})(flatbuffers = exports.flatbuffers || (exports.flatbuffers = {}));
// Exports for Node.js and RequireJS
//this.flatbuffers = flatbuffers;
/// @endcond
/// @}
