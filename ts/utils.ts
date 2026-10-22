export const int32: Int32Array = new Int32Array(2);
export const float32: Float32Array = new Float32Array(int32.buffer);
export const float64: Float64Array = new Float64Array(int32.buffer);
export const isLittleEndian: boolean =
  new Uint16Array(new Uint8Array([1, 0]).buffer)[0] === 1;
