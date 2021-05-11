"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Builder = void 0;
var byte_buffer_1 = require("./byte-buffer");
var constants_1 = require("./constants");
var long_1 = require("./long");
var Builder = /** @class */ (function () {
    /**
     * Create a FlatBufferBuilder.
     */
    function Builder(opt_initial_size) {
        /** Minimum alignment encountered so far. */
        this.minalign = 1;
        /** The vtable for the current table. */
        this.vtable = null;
        /** The amount of fields we're actually using. */
        this.vtable_in_use = 0;
        /** Whether we are currently serializing a table. */
        this.isNested = false;
        /** Starting offset of the current struct/table. */
        this.object_start = 0;
        /** List of offsets of all vtables. */
        this.vtables = [];
        /** For the current vector being built. */
        this.vector_num_elems = 0;
        /** False omits default values from the serialized data */
        this.force_defaults = false;
        this.string_maps = null;
        var initial_size;
        if (!opt_initial_size) {
            initial_size = 1024;
        }
        else {
            initial_size = opt_initial_size;
        }
        /**
         * @type {ByteBuffer}
         * @private
         */
        this.bb = byte_buffer_1.ByteBuffer.allocate(initial_size);
        this.space = initial_size;
    }
    Builder.prototype.clear = function () {
        this.bb.clear();
        this.space = this.bb.capacity();
        this.minalign = 1;
        this.vtable = null;
        this.vtable_in_use = 0;
        this.isNested = false;
        this.object_start = 0;
        this.vtables = [];
        this.vector_num_elems = 0;
        this.force_defaults = false;
        this.string_maps = null;
    };
    /**
     * In order to save space, fields that are set to their default value
     * don't get serialized into the buffer. Forcing defaults provides a
     * way to manually disable this optimization.
     *
     * @param forceDefaults true always serializes default values
     */
    Builder.prototype.forceDefaults = function (forceDefaults) {
        this.force_defaults = forceDefaults;
    };
    /**
     * Get the ByteBuffer representing the FlatBuffer. Only call this after you've
     * called finish(). The actual data starts at the ByteBuffer's current position,
     * not necessarily at 0.
     */
    Builder.prototype.dataBuffer = function () {
        return this.bb;
    };
    /**
     * Get the bytes representing the FlatBuffer. Only call this after you've
     * called finish().
     */
    Builder.prototype.asUint8Array = function () {
        return this.bb.bytes().subarray(this.bb.position(), this.bb.position() + this.offset());
    };
    /**
     * Prepare to write an element of `size` after `additional_bytes` have been
     * written, e.g. if you write a string, you need to align such the int length
     * field is aligned to 4 bytes, and the string data follows it directly. If all
     * you need to do is alignment, `additional_bytes` will be 0.
     *
     * @param size This is the of the new element to write
     * @param additional_bytes The padding size
     */
    Builder.prototype.prep = function (size, additional_bytes) {
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
            this.bb = Builder.growByteBuffer(this.bb);
            this.space += this.bb.capacity() - old_buf_size;
        }
        this.pad(align_size);
    };
    Builder.prototype.pad = function (byte_size) {
        for (var i = 0; i < byte_size; i++) {
            this.bb.writeInt8(--this.space, 0);
        }
    };
    Builder.prototype.writeInt8 = function (value) {
        this.bb.writeInt8(this.space -= 1, value);
    };
    Builder.prototype.writeInt16 = function (value) {
        this.bb.writeInt16(this.space -= 2, value);
    };
    Builder.prototype.writeInt32 = function (value) {
        this.bb.writeInt32(this.space -= 4, value);
    };
    Builder.prototype.writeInt64 = function (value) {
        this.bb.writeInt64(this.space -= 8, value);
    };
    Builder.prototype.writeFloat32 = function (value) {
        this.bb.writeFloat32(this.space -= 4, value);
    };
    Builder.prototype.writeFloat64 = function (value) {
        this.bb.writeFloat64(this.space -= 8, value);
    };
    /**
     * Add an `int8` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param value The `int8` to add the the buffer.
     */
    Builder.prototype.addInt8 = function (value) {
        this.prep(1, 0);
        this.writeInt8(value);
    };
    /**
     * Add an `int16` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param value The `int16` to add the the buffer.
     */
    Builder.prototype.addInt16 = function (value) {
        this.prep(2, 0);
        this.writeInt16(value);
    };
    /**
     * Add an `int32` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param value The `int32` to add the the buffer.
     */
    Builder.prototype.addInt32 = function (value) {
        this.prep(4, 0);
        this.writeInt32(value);
    };
    /**
     * Add an `int64` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param value The `int64` to add the the buffer.
     */
    Builder.prototype.addInt64 = function (value) {
        this.prep(8, 0);
        this.writeInt64(value);
    };
    /**
     * Add a `float32` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param value The `float32` to add the the buffer.
     */
    Builder.prototype.addFloat32 = function (value) {
        this.prep(4, 0);
        this.writeFloat32(value);
    };
    /**
     * Add a `float64` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param value The `float64` to add the the buffer.
     */
    Builder.prototype.addFloat64 = function (value) {
        this.prep(8, 0);
        this.writeFloat64(value);
    };
    Builder.prototype.addFieldInt8 = function (voffset, value, defaultValue) {
        if (this.force_defaults || value != defaultValue) {
            this.addInt8(value);
            this.slot(voffset);
        }
    };
    Builder.prototype.addFieldInt16 = function (voffset, value, defaultValue) {
        if (this.force_defaults || value != defaultValue) {
            this.addInt16(value);
            this.slot(voffset);
        }
    };
    Builder.prototype.addFieldInt32 = function (voffset, value, defaultValue) {
        if (this.force_defaults || value != defaultValue) {
            this.addInt32(value);
            this.slot(voffset);
        }
    };
    Builder.prototype.addFieldInt64 = function (voffset, value, defaultValue) {
        if (this.force_defaults || !value.equals(defaultValue)) {
            this.addInt64(value);
            this.slot(voffset);
        }
    };
    Builder.prototype.addFieldFloat32 = function (voffset, value, defaultValue) {
        if (this.force_defaults || value != defaultValue) {
            this.addFloat32(value);
            this.slot(voffset);
        }
    };
    Builder.prototype.addFieldFloat64 = function (voffset, value, defaultValue) {
        if (this.force_defaults || value != defaultValue) {
            this.addFloat64(value);
            this.slot(voffset);
        }
    };
    Builder.prototype.addFieldOffset = function (voffset, value, defaultValue) {
        if (this.force_defaults || value != defaultValue) {
            this.addOffset(value);
            this.slot(voffset);
        }
    };
    /**
     * Structs are stored inline, so nothing additional is being added. `d` is always 0.
     */
    Builder.prototype.addFieldStruct = function (voffset, value, defaultValue) {
        if (value != defaultValue) {
            this.nested(value);
            this.slot(voffset);
        }
    };
    /**
     * Structures are always stored inline, they need to be created right
     * where they're used.  You'll get this assertion failure if you
     * created it elsewhere.
     */
    Builder.prototype.nested = function (obj) {
        if (obj != this.offset()) {
            throw new Error('FlatBuffers: struct must be serialized inline.');
        }
    };
    /**
     * Should not be creating any other object, string or vector
     * while an object is being constructed
     */
    Builder.prototype.notNested = function () {
        if (this.isNested) {
            throw new Error('FlatBuffers: object serialization must not be nested.');
        }
    };
    /**
     * Set the current vtable at `voffset` to the current location in the buffer.
     */
    Builder.prototype.slot = function (voffset) {
        if (this.vtable !== null)
            this.vtable[voffset] = this.offset();
    };
    /**
     * @returns Offset relative to the end of the buffer.
     */
    Builder.prototype.offset = function () {
        return this.bb.capacity() - this.space;
    };
    /**
     * Doubles the size of the backing ByteBuffer and copies the old data towards
     * the end of the new buffer (since we build the buffer backwards).
     *
     * @param bb The current buffer with the existing data
     * @returns A new byte buffer with the old data copied
     * to it. The data is located at the end of the buffer.
     *
     * uint8Array.set() formally takes {Array<number>|ArrayBufferView}, so to pass
     * it a uint8Array we need to suppress the type check:
     * @suppress {checkTypes}
     */
    Builder.growByteBuffer = function (bb) {
        var old_buf_size = bb.capacity();
        // Ensure we don't grow beyond what fits in an int.
        if (old_buf_size & 0xC0000000) {
            throw new Error('FlatBuffers: cannot grow buffer beyond 2 gigabytes.');
        }
        var new_buf_size = old_buf_size << 1;
        var nbb = byte_buffer_1.ByteBuffer.allocate(new_buf_size);
        nbb.setPosition(new_buf_size - old_buf_size);
        nbb.bytes().set(bb.bytes(), new_buf_size - old_buf_size);
        return nbb;
    };
    /**
     * Adds on offset, relative to where it will be written.
     *
     * @param offset The offset to add.
     */
    Builder.prototype.addOffset = function (offset) {
        this.prep(constants_1.SIZEOF_INT, 0); // Ensure alignment is already done.
        this.writeInt32(this.offset() - offset + constants_1.SIZEOF_INT);
    };
    /**
     * Start encoding a new object in the buffer.  Users will not usually need to
     * call this directly. The FlatBuffers compiler will generate helper methods
     * that call this method internally.
     */
    Builder.prototype.startObject = function (numfields) {
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
     * @returns The offset to the object inside `dataBuffer`
     */
    Builder.prototype.endObject = function () {
        if (this.vtable == null || !this.isNested) {
            throw new Error('FlatBuffers: endObject called without startObject');
        }
        this.addInt32(0);
        var vtableloc = this.offset();
        // Trim trailing zeroes.
        var i = this.vtable_in_use - 1;
        // eslint-disable-next-line no-empty
        for (; i >= 0 && this.vtable[i] == 0; i--) { }
        var trimmed_size = i + 1;
        // Write out the current vtable.
        for (; i >= 0; i--) {
            // Offset relative to the start of the table.
            this.addInt16(this.vtable[i] != 0 ? vtableloc - this.vtable[i] : 0);
        }
        var standard_fields = 2; // The fields below:
        this.addInt16(vtableloc - this.object_start);
        var len = (trimmed_size + standard_fields) * constants_1.SIZEOF_SHORT;
        this.addInt16(len);
        // Search for an existing vtable that matches the current one.
        var existing_vtable = 0;
        var vt1 = this.space;
        outer_loop: for (i = 0; i < this.vtables.length; i++) {
            var vt2 = this.bb.capacity() - this.vtables[i];
            if (len == this.bb.readInt16(vt2)) {
                for (var j = constants_1.SIZEOF_SHORT; j < len; j += constants_1.SIZEOF_SHORT) {
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
    /**
     * Finalize a buffer, poiting to the given `root_table`.
     */
    Builder.prototype.finish = function (root_table, opt_file_identifier, opt_size_prefix) {
        var size_prefix = opt_size_prefix ? constants_1.SIZE_PREFIX_LENGTH : 0;
        if (opt_file_identifier) {
            var file_identifier = opt_file_identifier;
            this.prep(this.minalign, constants_1.SIZEOF_INT +
                constants_1.FILE_IDENTIFIER_LENGTH + size_prefix);
            if (file_identifier.length != constants_1.FILE_IDENTIFIER_LENGTH) {
                throw new Error('FlatBuffers: file identifier must be length ' +
                    constants_1.FILE_IDENTIFIER_LENGTH);
            }
            for (var i = constants_1.FILE_IDENTIFIER_LENGTH - 1; i >= 0; i--) {
                this.writeInt8(file_identifier.charCodeAt(i));
            }
        }
        this.prep(this.minalign, constants_1.SIZEOF_INT + size_prefix);
        this.addOffset(root_table);
        if (size_prefix) {
            this.addInt32(this.bb.capacity() - this.space);
        }
        this.bb.setPosition(this.space);
    };
    /**
     * Finalize a size prefixed buffer, pointing to the given `root_table`.
     */
    Builder.prototype.finishSizePrefixed = function (root_table, opt_file_identifier) {
        this.finish(root_table, opt_file_identifier, true);
    };
    /**
     * This checks a required field has been set in a given table that has
     * just been constructed.
     */
    Builder.prototype.requiredField = function (table, field) {
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
     * @param elem_size The size of each element in the array
     * @param num_elems The number of elements in the array
     * @param alignment The alignment of the array
     */
    Builder.prototype.startVector = function (elem_size, num_elems, alignment) {
        this.notNested();
        this.vector_num_elems = num_elems;
        this.prep(constants_1.SIZEOF_INT, elem_size * num_elems);
        this.prep(alignment, elem_size * num_elems); // Just in case alignment > int.
    };
    /**
     * Finish off the creation of an array and all its elements. The array must be
     * created with `startVector`.
     *
     * @returns The offset at which the newly created array
     * starts.
     */
    Builder.prototype.endVector = function () {
        this.writeInt32(this.vector_num_elems);
        return this.offset();
    };
    /**
     * Encode the string `s` in the buffer using UTF-8. If the string passed has
     * already been seen, we return the offset of the already written string
     *
     * @param s The string to encode
     * @return The offset in the buffer where the encoded string starts
     */
    Builder.prototype.createSharedString = function (s) {
        if (!s) {
            return 0;
        }
        if (!this.string_maps) {
            this.string_maps = new Map();
        }
        if (this.string_maps.has(s)) {
            return this.string_maps.get(s);
        }
        var offset = this.createString(s);
        this.string_maps.set(s, offset);
        return offset;
    };
    /**
     * Encode the string `s` in the buffer using UTF-8. If a Uint8Array is passed
     * instead of a string, it is assumed to contain valid UTF-8 encoded data.
     *
     * @param s The string to encode
     * @return The offset in the buffer where the encoded string starts
     */
    Builder.prototype.createString = function (s) {
        if (!s) {
            return 0;
        }
        var utf8;
        if (s instanceof Uint8Array) {
            utf8 = s;
        }
        else {
            utf8 = [];
            var i = 0;
            while (i < s.length) {
                var codePoint = void 0;
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
     */
    Builder.prototype.createLong = function (low, high) {
        return long_1.Long.create(low, high);
    };
    /**
     * A helper function to pack an object
     *
     * @returns offset of obj
     */
    Builder.prototype.createObjectOffset = function (obj) {
        if (obj === null) {
            return 0;
        }
        if (typeof obj === 'string') {
            return this.createString(obj);
        }
        else {
            return obj.pack(this);
        }
    };
    /**
     * A helper function to pack a list of object
     *
     * @returns list of offsets of each non null object
     */
    Builder.prototype.createObjectOffsetList = function (list) {
        var ret = [];
        for (var i = 0; i < list.length; ++i) {
            var val = list[i];
            if (val !== null) {
                ret.push(this.createObjectOffset(val));
            }
            else {
                throw new Error('FlatBuffers: Argument for createObjectOffsetList cannot contain null.');
            }
        }
        return ret;
    };
    Builder.prototype.createStructOffsetList = function (list, startFunc) {
        startFunc(this, list.length);
        this.createObjectOffsetList(list);
        return this.endVector();
    };
    return Builder;
}());
exports.Builder = Builder;
