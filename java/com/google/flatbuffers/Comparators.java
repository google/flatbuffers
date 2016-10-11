package com.google.flatbuffers;

/**
 * Utilities function to signed & unsigned compare values in their natural order
 */
public class Comparators {
    private static final int UBYTE_MASK = 0xff;
    private static final int USHORT_MASK = 0xffff;
    private static final long UINT_MASK = 0xffffffffL;
    private static final int UINT_SIGN_BIT = 0x80000000;
    private static final long ULONG_SIGN_BIT = 0x8000000000000000L;

    public static int compare(long a, long b) {
        return a > b ? 1 : a < b ? -1 : 0;
    }

    public static int compare(int a, int b) {
        return a > b ? 1 : a < b ? -1 : 0;
    }

    public static int compare(short a, short b) {
        return ((int)a) - ((int)b);
    }

    public static int compare(byte a, byte b) {
        return ((int)a) - ((int)b);
    }

    public static int compareUnsigned(long a, long b) {
        long delta = (a - b) ^ (a & ULONG_SIGN_BIT) ^ (b & ULONG_SIGN_BIT);
        return delta > 0 ? 1 : delta < 0 ? -1 : 0;
    }

    public static int compareUnsigned(int a, int b) {
//        long delta = (a & UINT_MASK) - (b & UINT_MASK);
//        return delta > 0 ? 1 : delta < 0 ? -1 : 0;
        return (a - b) ^ (a & UINT_SIGN_BIT) ^ (b & UINT_SIGN_BIT);
    }

    public static int compareUnsigned(short a, short b) {
        return (a & USHORT_MASK) - (b & USHORT_MASK);
    }

    public static int compareUnsigned(byte a, byte b) {
        return (a & UBYTE_MASK) - (b & UBYTE_MASK);
    }
}
