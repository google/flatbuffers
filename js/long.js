"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Long = exports.createLong = void 0;
function createLong(low, high) {
    return Long.create(low, high);
}
exports.createLong = createLong;
var Long = /** @class */ (function () {
    function Long(low, high) {
        this.low = low | 0;
        this.high = high | 0;
    }
    Long.create = function (low, high) {
        // Special-case zero to avoid GC overhead for default values
        return low == 0 && high == 0 ? Long.ZERO : new Long(low, high);
    };
    Long.prototype.toFloat64 = function () {
        return (this.low >>> 0) + this.high * 0x100000000;
    };
    Long.prototype.equals = function (other) {
        return this.low == other.low && this.high == other.high;
    };
    Long.ZERO = new Long(0, 0);
    return Long;
}());
exports.Long = Long;
