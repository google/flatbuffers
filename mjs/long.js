export function createLong(low, high) {
    return Long.create(low, high);
}
export class Long {
    constructor(low, high) {
        this.low = low | 0;
        this.high = high | 0;
    }
    static create(low, high) {
        // Special-case zero to avoid GC overhead for default values
        return low == 0 && high == 0 ? Long.ZERO : new Long(low, high);
    }
    toFloat64() {
        return (this.low >>> 0) + this.high * 0x100000000;
    }
    equals(other) {
        return this.low == other.low && this.high == other.high;
    }
}
Long.ZERO = new Long(0, 0);
