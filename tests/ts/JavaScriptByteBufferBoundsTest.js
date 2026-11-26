// Run as part of TypeScriptTest.py
import assert from 'assert'
import {ByteBuffer} from 'flatbuffers'

function main() {
  // A minimal, valid 16-byte buffer used as the trusted base.
  const bytes = new Uint8Array([
    0x10, 0x00, 0x00, 0x00, // root offset = 16 -> past the end (malformed)
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  ]);
  const bb = new ByteBuffer(bytes);

  // Scalar reads past the end previously returned `undefined` and NaN-
  // propagated through later arithmetic; they must now throw.
  assert.throws(() => bb.readInt32(13), RangeError);
  assert.throws(() => bb.readUint16(15), RangeError);
  assert.throws(() => bb.readUint8(16), RangeError);
  assert.throws(() => bb.readInt64(13), RangeError);

  // Negative offsets previously wrapped around on the underlying typed
  // array (`bytes[-1]` is `undefined`), silently producing garbage.
  assert.throws(() => bb.readInt32(-4), RangeError);
  assert.throws(() => bb.readUint16(-2), RangeError);

  // In-bounds reads still work.
  assert.strictEqual(bb.readInt32(0), 16);
  assert.strictEqual(bb.readUint16(0), 0x0010);
  assert.strictEqual(bb.readUint8(0), 0x10);

  // __string with a length that runs past the end previously returned a
  // silently clamped (truncated) string via subarray.
  // Layout: at offset 4 a forward offset 0, then length 100, then "ab".
  const stringBytes = new Uint8Array([
    0x00, 0x00, 0x00, 0x00, // offset to string data = 0 (relative)
    0x64, 0x00, 0x00, 0x00, // length = 100, buffer only holds 2 more bytes
    0x61, 0x62, // "ab"
  ]);
  const stringBb = new ByteBuffer(stringBytes);
  assert.throws(() => stringBb.__string(4), RangeError);

  // __string with a negative length previously produced an empty string.
  const negativeLengthBytes = new Uint8Array([
    0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, // length = -1
    0x61, 0x62,
  ]);
  const negativeBb = new ByteBuffer(negativeLengthBytes);
  assert.throws(() => negativeBb.__string(4), RangeError);

  // A well-formed short string still reads back correctly.
  // __string(4): forward offset at [4..7] points to the string header;
  // header = int32 length followed by the bytes.
  const validStringBytes = new Uint8Array([
    0x00, 0x00, 0x00, 0x00, // padding
    0x04, 0x00, 0x00, 0x00, // forward offset = 4 -> header at 8
    0x02, 0x00, 0x00, 0x00, // length = 2
    0x61, 0x62, // "ab" at 12
  ]);
  const validBb = new ByteBuffer(validStringBytes);
  assert.strictEqual(validBb.__string(4), 'ab');

  // __vector with a start beyond the buffer previously returned the
  // out-of-range start silently; later element reads then produced
  // undefined values.
  const vectorBytes = new Uint8Array([
    0x00, 0x00, 0x00, 0x00,
    0x10, 0x00, 0x00, 0x00, // forward offset 16 -> vector start past end
    0x01, 0x00, 0x00, 0x00, // length = 1
  ]);
  const vectorBb = new ByteBuffer(vectorBytes);
  assert.throws(() => vectorBb.__vector(4), RangeError);

  console.log('ByteBuffer bounds tests: PASSED');
}

main();
