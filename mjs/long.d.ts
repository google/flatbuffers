export declare function createLong(low: number, high: number): Long;
export declare class Long {
    static readonly ZERO: Long;
    low: number;
    high: number;
    constructor(low: number, high: number);
    static create(low: number, high: number): Long;
    toFloat64(): number;
    equals(other: Long): boolean;
}
