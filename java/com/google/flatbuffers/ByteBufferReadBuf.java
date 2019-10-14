package com.google.flatbuffers;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class ByteBufferReadBuf implements ReadBuf {

    // cache for converting UTF-8 codepoints into
    // Java chars. Used to speed up String comparison
    private final char[] comparisonBuffer = new char[2];
    private final ByteBuffer buffer;

    public ByteBufferReadBuf(ByteBuffer bb) {
        this.buffer = bb;
    }

    @Override
    public boolean getBoolean(int index) {
        return get(index) != 0;
    }

    @Override
    public byte get(int index) {
        return buffer.get(index);
    }

    @Override
    public short getShort(int index) {
        return buffer.getShort(index);
    }

    @Override
    public int getInt(int index) {
        return buffer.getInt(index);
    }

    @Override
    public long getLong(int index) {
        return buffer.getLong(index);
    }

    @Override
    public float getFloat(int index) {
        return buffer.getFloat(index);
    }

    @Override
    public double getDouble(int index) {
        return buffer.getDouble(index);
    }

    @Override
    public String getString(int start, int size) {
        return Utf8.getDefault().decodeUtf8(buffer, start, size);
    }

    @Override
    public byte[] data() {
        return buffer.array();
    }

    @Override
    public int limit() {
        return buffer.limit();
    }

    @Override
    public int compareBytes(int start, byte[] other) {
        int l1 = start;
        int l2 = 0;
        byte c1, c2;
        do {
            c1 = buffer.get(l1);
            c2 = other[l2];
            if (c1 == '\0')
                return c1 - c2;
            l1++;
            l2++;
            if (l2 == other.length) {
                // in our buffer we have an additional \0 byte
                // but this does not exist in regular Java strings, so we return now
                return c1 - c2;
            }
        }
        while (c1 == c2);
        return c1 - c2;
    }

    @Override
    public int compareString(int start, String other) {
        int l1 = start;
        int l2 = 0;
        int limit = other.length();
        char c2;

        // special loop for ASCII characters. Most of keys should be ASCII only, so this
        // loop should be optimized for that.
        // breaks if a multi-byte character is found
        while (l2<limit) {
            byte b = buffer.get(l1);
            if (b == 0) {
                return -1;
            }
            if (b < 0) {
                break;
            }
            c2 = other.charAt(l2);
            if ((char)b != c2) {
                return b - c2;
            }
            l1++;
            l2++;
        }

        while (l2<limit) {
            int charSize = Utf8.decodeCodePoint(this, l1, comparisonBuffer);
            if (charSize == 0) {
                return -1;
            }
            c2 = other.charAt(l2);
            if (comparisonBuffer[0] != c2) {
                return comparisonBuffer[0] - c2;
            }

            if (charSize == 4) {
                c2 = other.charAt(l2++);
                if (comparisonBuffer[1] != c2) {
                    return comparisonBuffer[0] - c2;
                }
            }

            l1 += charSize;
            l2++;
        }

        return 0;
    }
}
