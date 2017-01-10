package com.google.flatbuffers;

/**
 * Utilities function to convert unsigned so that they can be compared as signed values.
 *
 * <pre> asComparable((byte)0xff) &lt; asComparable((byte)0x05) </pre>
 *
 */
public class Unsigneds {
    private static final int UINT_SIGN_BIT = 0x80000000;
    private static final long ULONG_SIGN_BIT = 0x8000000000000000L;

    public static long asComparable(long a) {
        return a ^ULONG_SIGN_BIT;
    }

    public static int asComparable(byte a) {
        return a & Constants.UBYTE_MASK;
    }

    public static int asComparable(short a) {
        return a & Constants.USHORT_MASK;

    }
    public static int asComparable(int a) {
        return a ^ UINT_SIGN_BIT;
    }
}
