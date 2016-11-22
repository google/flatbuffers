package com.google.flatbuffers;

public class Floats {

    private static final int FLOAT32_SIGN_MASK = 0x80000000;
    private static final long FLOAT64_SIGN_MASK = 0x8000000000000000L;

    private Floats() {
    }

    // Floating-point are represented as:
    // SIGN | EXP | MANTISSA, with mantissa being in the lower bits and sign being the highest.
    // When value is negative (SIGN=1), we need to flip bits of EXP and MANTISSA to preserve natural order.
    // (2 power -10 is smaller than 2 power -2)
    public static int asComparable( float value ) {
        final int floatBits = Float.floatToIntBits(value);
        // Creates the mask of bits to flip by propagating the sign bit.
        // This means that if sign = 0 (positive) then mask will also be 0 and no bit is changed
        final int mask = (floatBits >> 31) & ~FLOAT32_SIGN_MASK;
        final int comparable = floatBits ^ mask;
        return comparable;
    }

    public static long asComparable( double value ) {
        final long doubleBits = Double.doubleToLongBits(value);
        final long mask = (doubleBits >> 63) & ~FLOAT64_SIGN_MASK;
        final long comparable = doubleBits ^ mask;
        return comparable;
    }
}
