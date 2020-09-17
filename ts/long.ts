export function createLong(low: number, high: number): Long {
    return Long.create(low, high);
}
  
export class Long {
    static readonly ZERO = new Long(0, 0)
    low: number
    high: number
    constructor(low: number, high: number) {
        this.low = low | 0;
        this.high = high | 0;
    }
    static create(low: number, high: number): Long {
        // Special-case zero to avoid GC overhead for default values
        return low == 0 && high == 0 ? Long.ZERO : new Long(low, high);
    }
    toFloat64(): number {
        return (this.low >>> 0) + this.high * 0x100000000;
    }
    equals(other: Long): boolean {
        return this.low == other.low && this.high == other.high;
    }
}