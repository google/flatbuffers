export function createLong(low: i32, high: i32): Long {
  return Long.create(low, high);
}

export class Long {
  static readonly ZERO: Long = new Long(0, 0);
  low: i32;
  high: i32;
  constructor(low: i32, high: i32) {
    this.low = low | 0;
    this.high = high | 0;
  }
  static create(low: i32, high: i32): Long {
    // Special-case zero to avoid GC overhead for default values
    return low == 0 && high == 0 ? Long.ZERO : new Long(low, high);
  }
  toFloat64(): i32 {
    return (this.low >>> 0) + this.high * 0x100000000;
  }
  toInt64(): i64 {
    return (this.low >>> 0) + this.high * 0x100000000;
  }
  equals(other: Long): boolean {
    return this.low == other.low && this.high == other.high;
  }
}
