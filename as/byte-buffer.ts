import { FILE_IDENTIFIER_LENGTH, SIZEOF_INT } from './constants';
import { int32, isLittleEndian, float32, float64 } from './utils';
import { Offset, Table, IGeneratedObject } from './types';

export class ByteBuffer {
  private position_: i32 = 0;

  /**
   * Create a new ByteBuffer with a given array of bytes (`Uint8Array`)
   */
  constructor(private bytes_: Uint8Array) {}

  /**
   * Create and allocate a new ByteBuffer with a given size.
   */
  static allocate(byte_size: i32): ByteBuffer {
    return new ByteBuffer(new Uint8Array(byte_size));
  }

  clear(): void {
    this.position_ = 0;
  }

  /**
   * Get the underlying `Uint8Array`.
   */
  bytes(): Uint8Array {
    return this.bytes_;
  }

  /**
   * Get the buffer's position.
   */
  position(): i32 {
    return this.position_;
  }

  /**
   * Set the buffer's position.
   */
  setPosition(position: i32): void {
    this.position_ = position;
  }

  /**
   * Get the buffer's capacity.
   */
  capacity(): i32 {
    return this.bytes_.length;
  }

  readInt8(offset: i32): i32 {
    return this.readUint8(offset) as i32;
  }

  readUint8(offset: i32): i32 {
    return this.bytes_[offset];
  }

  readInt16(offset: i32): i32 {
    return this.readUint16(offset) as i32;
  }

  readUint16(offset: i32): i32 {
    return i32(this.bytes_[offset]) | (i32(this.bytes_[offset + 1]) << 8);
  }

  readInt32(offset: i32): i32 {
    return (
      i32(this.bytes_[offset]) |
      (i32(this.bytes_[offset + 1]) << 8) |
      (i32(this.bytes_[offset + 2]) << 16) |
      (i32(this.bytes_[offset + 3]) << 24)
    );
  }

  readUint32(offset: i32): u32 {
    return this.readInt32(offset);
  }

  readInt64(offset: i32): i32 {
    return this.readInt32(offset) | (this.readInt32(offset + 4) << 32);
  }

  readUint64(offset: i32): u64 {
    return this.readInt64(offset) as u64;
  }

  readFloat32(offset: i32): f32 {
    int32[0] = this.readInt32(offset);
    return float32[0];
  }

  readFloat64(offset: i32): f64 {
    int32[isLittleEndian ? 0 : 1] = this.readInt32(offset);
    int32[isLittleEndian ? 1 : 0] = this.readInt32(offset + 4);
    return float64[0];
  }

  writeInt8(offset: i32, value: i32): void {
    this.bytes_[offset] = value;
  }

  writeUint8(offset: i32, value: i32): void {
    this.bytes_[offset] = value;
  }

  writeInt16(offset: i32, value: i32): void {
    this.bytes_[offset] = value;
    this.bytes_[offset + 1] = value >> 8;
  }

  writeUint16(offset: i32, value: i32): void {
    this.bytes_[offset] = value;
    this.bytes_[offset + 1] = value >> 8;
  }

  writeInt32(offset: i32, value: i32): void {
    this.bytes_[offset] = value;
    this.bytes_[offset + 1] = value >> 8;
    this.bytes_[offset + 2] = value >> 16;
    this.bytes_[offset + 3] = value >> 24;
  }

  writeUint32(offset: i32, value: i32): void {
    this.bytes_[offset] = value;
    this.bytes_[offset + 1] = value >> 8;
    this.bytes_[offset + 2] = value >> 16;
    this.bytes_[offset + 3] = value >> 24;
  }

  writeInt64(offset: i32, value: i32): void {
    this.bytes_[offset] = value;
    this.bytes_[offset + 1] = value >> 8;
    this.bytes_[offset + 2] = value >> 16;
    this.bytes_[offset + 3] = value >> 24;
    this.bytes_[offset + 4] = value >> 32;
    this.bytes_[offset + 5] = value >> 38;
    this.bytes_[offset + 6] = value >> 46;
    this.bytes_[offset + 7] = value >> 54;
  }

  writeUint64(offset: i32, value: i32): void {
    this.bytes_[offset] = value;
    this.bytes_[offset + 1] = value >> 8;
    this.bytes_[offset + 2] = value >> 16;
    this.bytes_[offset + 3] = value >> 24;
    this.bytes_[offset + 4] = value >> 32;
    this.bytes_[offset + 5] = value >> 38;
    this.bytes_[offset + 6] = value >> 46;
    this.bytes_[offset + 7] = value >> 54;
  }

  writeFloat32(offset: i32, value: f32): void {
    float32[0] = value;
    this.writeInt32(offset, int32[0]);
  }

  writeFloat64(offset: i32, value: f64): void {
    float64[0] = value;
    this.writeInt32(offset, int32[isLittleEndian ? 0 : 1]);
    this.writeInt32(offset + 4, int32[isLittleEndian ? 1 : 0]);
  }

  /**
   * Return the file identifier.   Behavior is undefined for FlatBuffers whose
   * schema does not include a file_identifier (likely points at padding or the
   * start of a the root vtable).
   */
  getBufferIdentifier(): string {
    if (
      this.bytes_.length <
      this.position_ + SIZEOF_INT + FILE_IDENTIFIER_LENGTH
    ) {
      throw new Error(
        'FlatBuffers: ByteBuffer is too short to contain an identifier.'
      );
    }
    let result = '';
    for (let i = 0; i < FILE_IDENTIFIER_LENGTH; i++) {
      result += String.fromCharCode(
        this.readInt8(this.position_ + SIZEOF_INT + i)
      );
    }
    return result;
  }

  /**
   * Look up a field in the vtable, return an offset into the object, or 0 if the
   * field is not present.
   */
  __offset(bb_pos: i32, vtable_offset: i32): Offset {
    const vtable = bb_pos - this.readInt32(bb_pos);
    return vtable_offset < this.readInt16(vtable)
      ? this.readInt16(vtable + vtable_offset)
      : 0;
  }

  /**
   * Initialize any Table-derived type to point to the union at the given offset.
   */
  __union(t: Table, offset: i32): Table {
    t.bb_pos = offset + this.readInt32(offset);
    t.bb = this;
    return t;
  }

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
  __string(offset: i32): string {
    offset += this.readInt32(offset);

    const length = this.readInt32(offset);
    let result = '';
    let i = 0;

    offset += SIZEOF_INT;

    while (i < length) {
      let codePoint: i32;

      // Decode UTF-8
      const a = this.readUint8(offset + i++);
      if (a < 0xc0) {
        codePoint = a;
      } else {
        const b = this.readUint8(offset + i++);
        if (a < 0xe0) {
          codePoint = ((a & 0x1f) << 6) | (b & 0x3f);
        } else {
          const c = this.readUint8(offset + i++);
          if (a < 0xf0) {
            codePoint = ((a & 0x0f) << 12) | ((b & 0x3f) << 6) | (c & 0x3f);
          } else {
            const d = this.readUint8(offset + i++);
            codePoint =
              ((a & 0x07) << 18) |
              ((b & 0x3f) << 12) |
              ((c & 0x3f) << 6) |
              (d & 0x3f);
          }
        }
      }

      // Encode UTF-16
      if (codePoint < 0x10000) {
        result += String.fromCharCode(codePoint);
      } else {
        codePoint -= 0x10000;
        result += String.fromCharCode(
          (codePoint >> 10) + 0xd800,
          (codePoint & ((1 << 10) - 1)) + 0xdc00
        );
      }
    }

    return result;
  }

  /**
   * Handle unions that can contain string as its member, if a Table-derived type then initialize it,
   * if a string then return a new one
   *
   * WARNING: strings are immutable in JS so we can't change the string that the user gave us, this
   * makes the behaviour of __union_with_string different compared to __union
   */
  __union_with_string(offset: i32): string {
    return this.__string(offset) as string;
  }

  __union_with_table(o: Table, offset: i32): Table {
    return this.__union(o, offset);
  }

  /**
   * Retrieve the relative offset stored at "offset"
   */
  __indirect(offset: Offset): Offset {
    return offset + this.readInt32(offset);
  }

  /**
   * Get the start of data of a vector whose offset is stored at "offset" in this object.
   */
  __vector(offset: Offset): Offset {
    return offset + this.readInt32(offset) + SIZEOF_INT; // data starts after the length
  }

  /**
   * Get the length of a vector whose offset is stored at "offset" in this object.
   */
  __vector_len(offset: Offset): Offset {
    return this.readInt32(offset + this.readInt32(offset));
  }

  __has_identifier(ident: string): boolean {
    if (ident.length != FILE_IDENTIFIER_LENGTH) {
      throw new Error(
        'FlatBuffers: file identifier must be length ' + FILE_IDENTIFIER_LENGTH
      );
    }
    for (let i = 0; i < FILE_IDENTIFIER_LENGTH; i++) {
      if (
        ident.charCodeAt(i) != this.readInt8(this.position() + SIZEOF_INT + i)
      ) {
        return false;
      }
    }
    return true;
  }

  /**
   * A helper function for generating list for obj api
   */
  createScalarList(
    listAccessor: (i: i32) => unknown,
    listLength: i32
  ): unknown[] {
    const ret: unknown[] = [];
    for (let i = 0; i < listLength; ++i) {
      if (listAccessor(i) !== null) {
        ret.push(listAccessor(i));
      }
    }

    return ret;
  }

  /**
   * This function is here only to get around typescript type system
   */
  createStringList(
    listAccessor: (i: i32) => unknown,
    listLength: i32
  ): unknown[] {
    return this.createScalarList(listAccessor, listLength);
  }

  /**
   * A helper function for generating list for obj api
   * @param listAccessor function that accepts an index and return data at that index
   * @param listLength listLength
   * @param res result list
   */
  createObjList(
    listAccessor: (i: i32) => IGeneratedObject,
    listLength: i32
  ): IGeneratedObject[] {
    const ret: IGeneratedObject[] = [];
    for (let i = 0; i < listLength; ++i) {
      const val = listAccessor(i);
      if (val !== null) {
        ret.push(val.unpack());
      }
    }

    return ret;
  }
}
