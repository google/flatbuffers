export const int32 = new Int32Array(2);
export const float32 = Float32Array.wrap(int32.buffer);
export const float64 = Float64Array.wrap(int32.buffer);
// export const isLittleEndian =
//   new Uint16Array(new Uint8Array([1, 0]).buffer)[0] === 1;

// TODO: need to figure out how to convert above logic into AS compatible one.
// however, in our current use cases we're always little endian based.
export const isLittleEndian = true;
