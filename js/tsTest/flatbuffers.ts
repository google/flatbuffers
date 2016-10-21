/// @file
/// @addtogroup flatbuffers_javascript_api
/// @{
/// @cond FLATBUFFERS_INTERNAL
/**
 * @const
 * @namespace
 */
//var flatbuffers = {};

////////////////////////////////////////////////////////////////////////////////

/**
 * @constructor
 * @param {number} high
 * @param {number} low
 */
export module flatbuffers {
  /**
 * @type {number}
 * @const
 */
  const SIZEOF_SHORT: number = 2;

  /**
   * @type {number}
   * @const
   */
  const SIZEOF_INT: number = 4;

  /**
   * @type {number}
   * @const
   */
  const FILE_IDENTIFIER_LENGTH: number = 4;

  /**
   * @enum {number}
   */
  export enum Encoding {
    UTF8_BYTES = 1,
    UTF16_STRING = 2
  };

  /**
   * @type {Int32Array}
   * @const
   */
  const int32 = new Int32Array(2);
  /**
   * @type {Float32Array}
   * @const
   */
  const float32 = new Float32Array(int32.buffer);

  /**
   * @type {Float64Array}
   * @const
   */
  const float64 = new Float64Array(int32.buffer);

  /**
   * @type {boolean}
   * @const
   */
  const isLittleEndian = new Uint16Array(new Uint8Array([1, 0]).buffer)[0] === 1;
  /**
 * @typedef {number}
 */
  export type Offset = number;
  /**
 * @typedef {{
 *   bb: flatbuffers.ByteBuffer,
 *   bb_pos: number
 * }}
 */
  export class Table {
    bb: flatbuffers.ByteBuffer
    bb_pos: number
  };
  export class Long {
    low: number
    high: number
    constructor(low: number, high: number) {

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

    };

    /**
     * @param {number} high
     * @param {number} low
     * @returns {flatbuffers.Long}
     */
    static create(low: number, high: number) {
      // Special-case zero to avoid GC overhead for default values
      return low == 0 && high == 0 ? Long.ZERO : new flatbuffers.Long(low, high);
    };

    /**
     * @returns {number}
     */
    toFloat64(): number {
      return this.low + this.high * 0x100000000;
    };

    /**
     * @param {flatbuffers.Long} other
     * @returns {boolean}
     */
    equals(other: flatbuffers.Long): boolean {
      return this.low == other.low && this.high == other.high;
    };

    /**
     * @type {flatbuffers.Long}
     * @const
     */
  }
  export module Long {
    export const ZERO = new flatbuffers.Long(0, 0);
  }


/// @endcond
////////////////////////////////////////////////////////////////////////////////
/**
 * Create a FlatBufferBuilder.
 *
 * @constructor
 * @param {number=} initial_size
 */
export class Builder {
  bb:flatbuffers.ByteBuffer
  space:number
  minalign:number
  vtable:number[]
  vtable_in_use:number
  isNested:boolean
  object_start:number
  vtables:number[]
  vector_num_elems:number
  force_defaults:boolean

  constructor(initial_size?:number) {
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
  };

  /**
   * In order to save space, fields that are set to their default value
   * don't get serialized into the buffer. Forcing defaults provides a
   * way to manually disable this optimization.
   *
   * @param {boolean} forceDefaults true always serializes default values
   */
  forceDefaults(forceDefaults: boolean) {
    this.force_defaults = forceDefaults;
  };

  /**
   * Get the ByteBuffer representing the FlatBuffer. Only call this after you've
   * called finish(). The actual data starts at the ByteBuffer's current position,
   * not necessarily at 0.
   *
   * @returns {flatbuffers.ByteBuffer}
   */
  dataBuffer(): flatbuffers.ByteBuffer {
    return this.bb;
  };

  /**
   * Get the bytes representing the FlatBuffer. Only call this after you've
   * called finish().
   *
   * @returns {Uint8Array}
   */
  asUint8Array(): Uint8Array {
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
  prep(size: number, additional_bytes: number) {
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
      this.bb = this.growByteBuffer(this.bb);
      this.space += this.bb.capacity() - old_buf_size;
    }

    this.pad(align_size);
  };

  /**
   * @param {number} byte_size
   */
  pad(byte_size: number) {
    for (var i = 0; i < byte_size; i++) {
      this.bb.writeInt8(--this.space, 0);
    }
  };

  /**
   * @param {number} value
   */
  writeInt8(value: number) {
    this.bb.writeInt8(this.space -= 1, value);
  };

  /**
   * @param {number} value
   */
  writeInt16(value: number) {
    this.bb.writeInt16(this.space -= 2, value);
  };

  /**
   * @param {number} value
   */
  writeInt32(value: number) {
    this.bb.writeInt32(this.space -= 4, value);
  };

  /**
   * @param {flatbuffers.Long} value
   */
  writeInt64(value: flatbuffers.Long) {
    this.bb.writeInt64(this.space -= 8, value);
  };

  /**
   * @param {number} value
   */
  writeFloat32(value: number) {
    this.bb.writeFloat32(this.space -= 4, value);
  };

  /**
   * @param {number} value
   */
  writeFloat64(value: number) {
    this.bb.writeFloat64(this.space -= 8, value);
  };
  /// @endcond

  /**
   * Add an `int8` to the buffer, properly aligned, and grows the buffer (if necessary).
   * @param {number} value The `int8` to add the the buffer.
   */
  addInt8(value: number) {
    this.prep(1, 0);
    this.writeInt8(value);
  };

  /**
   * Add an `int16` to the buffer, properly aligned, and grows the buffer (if necessary).
   * @param {number} value The `int16` to add the the buffer.
   */
  addInt16(value: number) {
    this.prep(2, 0);
    this.writeInt16(value);
  };

  /**
   * Add an `int32` to the buffer, properly aligned, and grows the buffer (if necessary).
   * @param {number} value The `int32` to add the the buffer.
   */
  addInt32(value: number) {
    this.prep(4, 0);
    this.writeInt32(value);
  };

  /**
   * Add an `int64` to the buffer, properly aligned, and grows the buffer (if necessary).
   * @param {flatbuffers.Long} value The `int64` to add the the buffer.
   */
  addInt64(value: flatbuffers.Long) {
    this.prep(8, 0);
    this.writeInt64(value);
  };

  /**
   * Add a `float32` to the buffer, properly aligned, and grows the buffer (if necessary).
   * @param {number} value The `float32` to add the the buffer.
   */
  addFloat32(value: number) {
    this.prep(4, 0);
    this.writeFloat32(value);
  };

  /**
   * Add a `float64` to the buffer, properly aligned, and grows the buffer (if necessary).
   * @param {number} value The `float64` to add the the buffer.
   */
  addFloat64(value: number) {
    this.prep(8, 0);
    this.writeFloat64(value);
  };

  /// @cond FLATBUFFERS_INTERNAL
  /**
   * @param {number} voffset
   * @param {number} value
   * @param {number} defaultValue
   */
  addFieldInt8(voffset: number, value: number, defaultValue: number) {
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
  addFieldInt16(voffset: number, value: number, defaultValue: number) {
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
  addFieldInt32(voffset: number, value: number, defaultValue: number) {
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
  addFieldInt64(voffset: number, value: flatbuffers.Long, defaultValue: flatbuffers.Long) {
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
  addFieldFloat32(voffset: number, value: number, defaultValue: number) {
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
  addFieldFloat64(voffset: number, value: number, defaultValue: number) {
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
  addFieldOffset(voffset: number, value: flatbuffers.Offset, defaultValue: flatbuffers.Offset) {
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
  addFieldStruct(voffset: number, value: flatbuffers.Offset, defaultValue: flatbuffers.Offset) {
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
  nested(obj: flatbuffers.Offset) {
    if (obj != this.offset()) {
      throw new Error('FlatBuffers: struct must be serialized inline.');
    }
  };

  /**
   * Should not be creating any other object, string or vector
   * while an object is being constructed
   */
  notNested() {
    if (this.isNested) {
      throw new Error('FlatBuffers: object serialization must not be nested.');
    }
  };

  /**
   * Set the current vtable at `voffset` to the current location in the buffer.
   *
   * @param {number} voffset
   */
  slot(voffset: number) {
    this.vtable[voffset] = this.offset();
  };

  /**
   * @returns {flatbuffers.Offset} Offset relative to the end of the buffer.
   */
  offset(): flatbuffers.Offset {
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
  growByteBuffer(bb: flatbuffers.ByteBuffer): flatbuffers.ByteBuffer {
    var old_buf_size = bb.capacity();

    // Ensure we don't grow beyond what fits in an int.
    if (old_buf_size & 0xC0000000) {
      throw new Error('FlatBuffers: cannot grow buffer beyond 2 gigabytes.');
    }

    var new_buf_size = old_buf_size << 1;
    var nbb = ByteBuffer.allocate(new_buf_size);
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
  addOffset(offset: flatbuffers.Offset) {
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
  startObject(numfields: number) {
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
  endObject(): flatbuffers.Offset {
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
    outer_loop:
    for (var i = 0; i < this.vtables.length; i++) {
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
    } else {
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
  finish(root_table: flatbuffers.Offset, file_identifier?: string) {
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
  requiredField(table: flatbuffers.Offset, field: number) {
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
  startVector(elem_size: number, num_elems: number, alignment: number) {
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
  endVector(): flatbuffers.Offset {
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
  createString(s: string | Uint8Array): flatbuffers.Offset {
    var utf8:number[]|Uint8Array
    if (s instanceof Uint8Array) {
      utf8 = s;
    } else {
      utf8 = [];
      var i = 0;

      while (i < s.length) {
        var codePoint;

        // Decode UTF-16
        var a = s.charCodeAt(i++);
        if (a < 0xD800 || a >= 0xDC00) {
          codePoint = a;
        } else {
          var b = s.charCodeAt(i++);
          codePoint = (a << 10) + b + (0x10000 - (0xD800 << 10) - 0xDC00);
        }

        // Encode UTF-8
        if (codePoint < 0x80) {
          utf8.push(codePoint);
        } else {
          if (codePoint < 0x800) {
            utf8.push(((codePoint >> 6) & 0x1F) | 0xC0);
          } else {
            if (codePoint < 0x10000) {
              utf8.push(((codePoint >> 12) & 0x0F) | 0xE0);
            } else {
              utf8.push(
                ((codePoint >> 18) & 0x07) | 0xF0,
                ((codePoint >> 12) & 0x3F) | 0x80);
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
  createLong(low, high) {
    return flatbuffers.Long.create(low, high);
  };
}

////////////////////////////////////////////////////////////////////////////////
/// @cond FLATBUFFERS_INTERNAL
/**
 * Create a new ByteBuffer with a given array of bytes (`Uint8Array`).
 *
 * @constructor
 * @param {Uint8Array} bytes
 */
export class ByteBuffer {
  bytes_:Uint8Array
  position_:number
  constructor(bytes: Uint8Array) {
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
  };

  /**
   * Create and allocate a new ByteBuffer with a given size.
   *
   * @param {number} byte_size
   * @returns {flatbuffers.ByteBuffer}
   */
  static allocate(byte_size: number) {
    return new flatbuffers.ByteBuffer(new Uint8Array(byte_size));
  };

  /**
   * Get the underlying `Uint8Array`.
   *
   * @returns {Uint8Array}
   */
  bytes(): Uint8Array {
    return this.bytes_;
  };

  /**
   * Get the buffer's position.
   *
   * @returns {number}
   */
  position(): number {
    return this.position_;
  };

  /**
   * Set the buffer's position.
   *
   * @param {number} position
   */
  setPosition(position: number) {
    this.position_ = position;
  };

  /**
   * Get the buffer's capacity.
   *
   * @returns {number}
   */
  capacity(): number {
    return this.bytes_.length;
  };

  /**
   * @param {number} offset
   * @returns {number}
   */
  readInt8(offset: number): number {
    return this.readUint8(offset) << 24 >> 24;
  };

  /**
   * @param {number} offset
   * @returns {number}
   */
  readUint8(offset: number): number {
    return this.bytes_[offset];
  };

  /**
   * @param {number} offset
   * @returns {number}
   */
  readInt16(offset: number): number {
    return this.readUint16(offset) << 16 >> 16;
  };

  /**
   * @param {number} offset
   * @returns {number}
   */
  readUint16(offset: number): number {
    return this.bytes_[offset] | this.bytes_[offset + 1] << 8;
  };

  /**
   * @param {number} offset
   * @returns {number}
   */
  readInt32(offset: number): number {
    return this.bytes_[offset] | this.bytes_[offset + 1] << 8 | this.bytes_[offset + 2] << 16 | this.bytes_[offset + 3] << 24;
  };

  /**
   * @param {number} offset
   * @returns {number}
   */
  readUint32(offset: number): number {
    return this.readInt32(offset) >>> 0;
  };

  /**
   * @param {number} offset
   * @returns {flatbuffers.Long}
   */
  readInt64(offset: number): flatbuffers.Long {
    return new flatbuffers.Long(this.readInt32(offset), this.readInt32(offset + 4));
  };

  /**
   * @param {number} offset
   * @returns {flatbuffers.Long}
   */
  readUint64(offset: number): flatbuffers.Long {
    return new flatbuffers.Long(this.readUint32(offset), this.readUint32(offset + 4));
  };

  /**
   * @param {number} offset
   * @returns {number}
   */
  readFloat32(offset: number): number {
    int32[0] = this.readInt32(offset);
    return float32[0];
  };

  /**
   * @param {number} offset
   * @returns {number}
   */
  readFloat64(offset: number): number {
    int32[isLittleEndian ? 0 : 1] = this.readInt32(offset);
    int32[isLittleEndian ? 1 : 0] = this.readInt32(offset + 4);
    return float64[0];
  };

  /**
   * @param {number} offset
   * @param {number} value
   */
  writeInt8(offset: number, value: number) {
    this.bytes_[offset] = value;
  };

  /**
   * @param {number} offset
   * @param {number} value
   */
  writeInt16(offset: number, value: number) {
    this.bytes_[offset] = value;
    this.bytes_[offset + 1] = value >> 8;
  };

  /**
   * @param {number} offset
   * @param {number} value
   */
  writeInt32(offset: number, value: number) {
    this.bytes_[offset] = value;
    this.bytes_[offset + 1] = value >> 8;
    this.bytes_[offset + 2] = value >> 16;
    this.bytes_[offset + 3] = value >> 24;
  };

  /**
   * @param {number} offset
   * @param {flatbuffers.Long} value
   */
  writeInt64(offset: number, value: flatbuffers.Long) {
    this.writeInt32(offset, value.low);
    this.writeInt32(offset + 4, value.high);
  };

  /**
   * @param {number} offset
   * @param {number} value
   */
  writeFloat32(offset: number, value: number) {
    float32[0] = value;
    this.writeInt32(offset, int32[0]);
  };

  /**
   * @param {number} offset
   * @param {number} value
   */
  writeFloat64(offset: number, value: number) {
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
  __offset(bb_pos: number, vtable_offset: number): number {
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
  __union<T extends flatbuffers.Table>(t: T, offset: number): T {
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
  __string(offset:number):string;
  __string(offset: number, optionalEncoding: flatbuffers.Encoding): string | Uint8Array;
  __string(offset: number, optionalEncoding?: Encoding): string | Uint8Array {
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
      } else {
        var b = this.readUint8(offset + i++);
        if (a < 0xE0) {
          codePoint =
            ((a & 0x1F) << 6) |
            (b & 0x3F);
        } else {
          var c = this.readUint8(offset + i++);
          if (a < 0xF0) {
            codePoint =
              ((a & 0x0F) << 12) |
              ((b & 0x3F) << 6) |
              (c & 0x3F);
          } else {
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
      } else {
        codePoint -= 0x10000;
        result += String.fromCharCode(
          (codePoint >> 10) + 0xD800,
          (codePoint & ((1 << 10) - 1)) + 0xDC00);
      }
    }

    return result;
  };

  /**
   * Retrieve the relative offset stored at "offset"
   * @param {number} offset
   * @returns {number}
   */
  __indirect(offset: number): number {
    return offset + this.readInt32(offset);
  };

  /**
   * Get the start of data of a vector whose offset is stored at "offset" in this object.
   *
   * @param {number} offset
   * @returns {number}
   */
  __vector(offset: number): number {
    return offset + this.readInt32(offset) + SIZEOF_INT; // data starts after the length
  };

  /**
   * Get the length of a vector whose offset is stored at "offset" in this object.
   *
   * @param {number} offset
   * @returns {number}
   */
  __vector_len(offset: number): number {
    return this.readInt32(offset + this.readInt32(offset));
  };

  /**
   * @param {string} ident
   * @returns {boolean}
   */
  __has_identifier(ident: string): boolean {
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
  createLong(low: number, high: number): flatbuffers.Long {
    return flatbuffers.Long.create(low, high);
  };
}
}

// Exports for Node.js and RequireJS
//this.flatbuffers = flatbuffers;

/// @endcond
/// @}
