/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.flatbuffers;


import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

/**
 * This class can be used to parse FlexBuffer messages.
 * <p>
 * For generating FlexBuffer messages, use {@link FlexBuffersBuilder}.
 * <p>
 * Example of usage:
 * ByteBuffer bb = ... // load message from file or network
 * FlexBuffers.Reference r = FlexBuffers.getRoot(bb); // Reads the root element
 * FlexBuffers.Map map = r.asMap(); // We assumed root object is a map
 * System.out.println(map.get("name").asString()); // prints element with key "name"
 */
public class FlexBuffers {

    // These are used as the upper 6 bits of a type field to indicate the actual
    // type.
    public static final int FBT_NULL = 0;
    public static final int FBT_INT = 1;
    public static final int FBT_UINT = 2;
    public static final int FBT_FLOAT = 3; // Types above stored inline, types below store an offset.
    public static final int FBT_KEY = 4;
    public static final int FBT_STRING = 5;
    public static final int FBT_INDIRECT_INT = 6;
    public static final int FBT_INDIRECT_UINT = 7;
    public static final int FBT_INDIRECT_FLOAT = 8;
    public static final int FBT_MAP = 9;
    public static final int FBT_VECTOR = 10; // Untyped.
    public static final int FBT_VECTOR_INT = 11;  // Typed any size  = stores no type table).
    public static final int FBT_VECTOR_UINT = 12;
    public static final int FBT_VECTOR_FLOAT = 13;
    public static final int FBT_VECTOR_KEY = 14;
    public static final int FBT_VECTOR_STRING = 15;
    public static final int FBT_VECTOR_INT2 = 16;  // Typed tuple  = no type table; no size field).
    public static final int FBT_VECTOR_UINT2 = 17;
    public static final int FBT_VECTOR_FLOAT2 = 18;
    public static final int FBT_VECTOR_INT3 = 19;  // Typed triple  = no type table; no size field).
    public static final int FBT_VECTOR_UINT3 = 20;
    public static final int FBT_VECTOR_FLOAT3 = 21;
    public static final int FBT_VECTOR_INT4 = 22;  // Typed quad  = no type table; no size field).
    public static final int FBT_VECTOR_UINT4 = 23;
    public static final int FBT_VECTOR_FLOAT4 = 24;
    public static final int FBT_BLOB = 25;
    public static final int FBT_BOOL = 26;
    public static final int FBT_VECTOR_BOOL = 36;  // To Allow the same type of conversion of type to vector type
    private static final ByteBuffer EMPTY_BB = ByteBuffer.allocate(0).asReadOnlyBuffer();

    /**
     * Checks where a type is a typed vector
     *
     * @param type type to be checked
     * @return true if typed vector
     */
    static boolean isTypedVector(int type) {
        return (type >= FBT_VECTOR_INT && type <= FBT_VECTOR_STRING) || type == FBT_VECTOR_BOOL;
    }

    /**
     * Check whether you can access type directly (no indirection) or not.
     *
     * @param type type to be checked
     * @return true if inline type
     */
    static boolean isTypeInline(int type) {
        return type <= FBT_FLOAT || type == FBT_BOOL;
    }

    static int toTypedVectorElementType(int original_type) {
        return original_type - FBT_VECTOR_INT + FBT_INT;
    }

    /**
     * Return a vector type our of a original element type
     *
     * @param type        element type
     * @param fixedLength size of elment
     * @return typed vector type
     */
    static int toTypedVector(int type, int fixedLength) {
        assert (isTypedVectorElementType(type));
        switch (fixedLength) {
            case 0: return type - FBT_INT + FBT_VECTOR_INT;
            case 2: return type - FBT_INT + FBT_VECTOR_INT2;
            case 3: return type - FBT_INT + FBT_VECTOR_INT3;
            case 4: return type - FBT_INT + FBT_VECTOR_INT4;
            default:
                assert (false);
                return FBT_NULL;
        }
    }

    static boolean isTypedVectorElementType(int type) {
        return (type >= FBT_INT && type <= FBT_STRING) || type == FBT_BOOL;
    }

    // return position of the element that the offset is pointing to
    private static int indirect(ByteBuffer bb, int offset, int byteWidth) {
        //TODO: we assume all offset fits on a int, since ByteBuffer operates with that assumption
        return (int) (offset - readUInt(bb, offset, byteWidth));
    }

    // read unsigned int with size byteWidth and return as a 64-bit integer
    private static long readUInt(ByteBuffer buff, int end, int byteWidth) {
        switch (byteWidth) {
            case 1: return Byte.toUnsignedInt(buff.get(end));
            case 2: return Short.toUnsignedInt(buff.getShort(end));
            case 4: return Integer.toUnsignedLong(buff.getInt(end));
            //TODO: who should we treat a possible ulong? throw exception?
            case 8: {
                long l = buff.getLong(end);
                if (l < 0)
                    throw new FlexBufferException("This is probably a unsigned 64-bit integer. Use readULong if you store 64-bit longs");
                return l;
            }
            default:
                throw new FlexBufferException(String.format("Invalid bit width of %d for scalar int", byteWidth));
        }
    }

    // read signed int of size byteWidth and return as 32-bit int
    private static int readInt(ByteBuffer buff, int end, int byteWidth) {
        return (int) readLong(buff, end, byteWidth);
    }

    // read signed int of size byteWidth and return as 64-bit int
    private static long readLong(ByteBuffer buff, int end, int byteWidth) {
        switch (byteWidth) {
            case 1: return buff.get(end);
            case 2: return buff.getShort(end);
            case 4: return buff.getInt(end);
            case 8: return buff.getLong(end);
            default:
                throw new FlexBufferException(String.format("Invalid bit width of %d for scalar long", byteWidth));
        }
    }

    private static BigInteger readULong(ByteBuffer buff, int end, int byteWidth) {
        switch (byteWidth) {
            case 1: return BigInteger.valueOf(Byte.toUnsignedInt(buff.get(end)));
            case 2: return BigInteger.valueOf(Short.toUnsignedInt(buff.getShort(end)));
            case 4: return BigInteger.valueOf(Integer.toUnsignedLong(buff.getInt(end)));
            case 8: return new BigInteger(Long.toUnsignedString(buff.getLong(end))); // TODO: slow op
            default:
                throw new FlexBufferException(String.format("Invalid bit width of %d for scalar int", byteWidth));
        }
    }

    private static double readDouble(ByteBuffer buff, int end, int byteWidth) {
        switch (byteWidth) {
            case 4: return buff.getFloat(end);
            case 8: return buff.getDouble(end);
            default:
                throw new FlexBufferException(String.format("Invalid bit width of %d for scalar long", byteWidth));
        }
    }

    /**
     * Reads a FlexBuffer message in ByteBuffer and returns {@link Reference} to
     * the root element.
     * @param buffer ByteBuffer containing FlexBuffer message
     * @return {@link Reference} to the root object
     */
    public static Reference getRoot(ByteBuffer buffer) {
        // See Finish() below for the serialization counterpart of this.
        // The root ends at the end of the buffer, so we parse backwards from there.
        int end = buffer.limit();
        int byteWidth = buffer.get(--end);
        int packetType = Byte.toUnsignedInt(buffer.get(--end));
        end -= byteWidth;  // The root data item.
        return new Reference(buffer, end, byteWidth, packetType);
    }

    enum BitWidth {
        WIDTH_8(0),
        WIDTH_16(1),
        WIDTH_32(2),
        WIDTH_64(3);

        final int val;

        BitWidth(int val) {
            this.val = val;
        }

        public static BitWidth max(BitWidth a, BitWidth b) {
            return (a.val >= b.val) ? a : b;
        }
    }

    public static class Reference {

        private static final Reference NULL_REFERENCE = new Reference(EMPTY_BB, 0, 1, 0);
        private ByteBuffer bb;
        private int end;
        private int parentWidth;
        private int byteWidth;
        private int type;

        Reference(ByteBuffer bb, int end, int parentWidth, int packedType) {
            this(bb, end, parentWidth, (1 << (packedType & 3)), packedType >> 2);
        }

        Reference(ByteBuffer bb, int end, int parentWidth, int byteWidth, int type) {
            this.bb = bb;
            this.end = end;
            this.parentWidth = parentWidth;
            this.byteWidth = byteWidth;
            this.type = type;
        }

        public int getType() {
            return type;
        }

        public boolean isNull() {
            return type == FBT_NULL;
        }

        public boolean isBoolean() {
            return type == FBT_BOOL;
        }

        public boolean isNumeric() {
            return isIntOrUInt() || isFloat();
        }

        public boolean isIntOrUInt() {
            return isInt() || isUInt();
        }

        public boolean isFloat() {
            return type == FBT_FLOAT || type == FBT_INDIRECT_FLOAT;
        }

        public boolean isInt() {
            return type == FBT_INT || type == FBT_INDIRECT_INT;
        }

        public boolean isUInt() {
            return type == FBT_UINT || type == FBT_INDIRECT_UINT;
        }

        public boolean isString() {
            return type == FBT_STRING;
        }

        public boolean isKey() {
            return type == FBT_KEY;
        }

        public boolean isVector() {
            return type == FBT_VECTOR || type == FBT_MAP;
        }

        public boolean isTypedVector() {
            return (type >= FBT_VECTOR_INT && type <= FBT_VECTOR_STRING) ||
                    type == FBT_VECTOR_BOOL;
        }

        public boolean isMap() {
            return type == FBT_MAP;
        }

        public boolean isBlob() {
            return type == FBT_BLOB;
        }

        public int asInt() {
            if (type != FBT_INDIRECT_INT && type != FBT_INT)
                throw new InvalidTypeConversionFlexBufferException(type, FBT_INT);
            int pos = type == FBT_INDIRECT_INT ? indirect(bb, end, parentWidth) : end;
            return readInt(bb, pos, parentWidth);
        }

        public long asUInt() {
            if (type != FBT_INDIRECT_UINT && type != FBT_UINT)
                throw new InvalidTypeConversionFlexBufferException(type, FBT_UINT);
            int pos = type == FBT_INDIRECT_UINT ? indirect(bb, end, parentWidth) : end;
            return readUInt(bb, pos, parentWidth);
        }

        public BigInteger asUInt64() {
            if (type != FBT_INDIRECT_INT && type != FBT_INT)
                throw new InvalidTypeConversionFlexBufferException(type, FBT_INT);
            int pos = type == FBT_INDIRECT_INT ? indirect(bb, end, parentWidth) : end;
            return readULong(bb, pos, parentWidth);
        }

        public long asLong() {
            if (type != FBT_INDIRECT_INT && type != FBT_INT)
                throw new InvalidTypeConversionFlexBufferException(type, FBT_INT);
            int pos = type == FBT_INDIRECT_INT ? indirect(bb, end, parentWidth) : end;
            return readLong(bb, pos, parentWidth);
        }

        public BigInteger asULong() {
            if (type != FBT_INDIRECT_UINT && type != FBT_UINT)
                throw new InvalidTypeConversionFlexBufferException(type, FBT_UINT);
            int pos = type == FBT_INDIRECT_UINT ? indirect(bb, end, parentWidth) : end;
            return new BigInteger(Long.toUnsignedString(readLong(bb, pos, parentWidth)));
        }

        public float asFloat() {
            if (type != FBT_FLOAT && type != FBT_INDIRECT_FLOAT)
                throw new InvalidTypeConversionFlexBufferException(type, FBT_FLOAT);
            int pos = type == FBT_INDIRECT_FLOAT ? indirect(bb, end, parentWidth) : end;
            return (float) readDouble(bb, pos, parentWidth);
        }

        public double asDouble() {
            if (type != FBT_FLOAT && type != FBT_INDIRECT_FLOAT)
                throw new InvalidTypeConversionFlexBufferException(type, FBT_FLOAT);
            int pos = type == FBT_INDIRECT_FLOAT ? indirect(bb, end, parentWidth) : end;
            return readDouble(bb, pos, parentWidth);
        }

        public Key asKey() {
            if (type == FBT_KEY) {
                return new Key(bb, indirect(bb, end, parentWidth), byteWidth);
            } else {
                return Key.empty();
            }
        }

        public String asString() {
            if (type == FBT_KEY || type == FBT_STRING || type == FBT_BLOB) {
                int start = indirect(bb, end, byteWidth);
                for (int i = start; ; i++) {
                    if (bb.get(i) == 0) {
                        return Utf8.getDefault().decodeUtf8(bb, start, i - start);
                    }
                }
            } else {
                return "";
            }
        }

        public Map asMap() {
            if (type == FBT_MAP) {
                return new Map(bb, indirect(bb, end, parentWidth), byteWidth);
            } else {
                return Map.empty();
            }
        }

        public Vector asVector() {
            if (type == FBT_VECTOR || type == FBT_MAP) {
                return new Vector(bb, indirect(bb, end, parentWidth), byteWidth);
            } else if (FlexBuffers.isTypedVector(type)) {
                return new TypedVector(bb, indirect(bb, end, parentWidth), byteWidth, FlexBuffers.toTypedVectorElementType(type));
            } else {
                return Vector.empty();
            }
        }

        public Blob asBlob() {
            if (type == FBT_BLOB || type == FBT_STRING) {
                return new Blob(bb, indirect(bb, end, parentWidth), byteWidth);
            } else {
                return Blob.empty();
            }
        }

        public boolean asBoolean() {
            if (type == FBT_BOOL) {
                return bb.get(end) != 0;
            }
            throw new InvalidTypeConversionFlexBufferException(type, FBT_INT);
        }

        @Override
        public String toString() {
            return toString(new StringBuilder(128)).toString();
        }

        StringBuilder toString(StringBuilder sb) {
            //TODO: Original C++ implementation escape strings.
            // probably we should do it as well.
            switch (type) {
                case FBT_NULL:
                    return sb.append("null");
                case FBT_INT:
                case FBT_INDIRECT_INT:
                    return sb.append(asLong());
                case FBT_UINT:
                case FBT_INDIRECT_UINT:
                    return sb.append(asULong());
                case FBT_INDIRECT_FLOAT:
                case FBT_FLOAT:
                    return sb.append(asDouble());
                case FBT_KEY:
                    return asKey().toString(sb.append('"')).append('"');
                case FBT_STRING:
                    return sb.append('"').append(asString()).append('"');
                case FBT_MAP:
                    return asMap().toString(sb);
                case FBT_VECTOR:
                    return asVector().toString(sb);
                case FBT_BLOB:
                    return asBlob().toString(sb);
                case FBT_BOOL:
                    return sb.append(asBoolean());
                case FBT_VECTOR_INT:
                case FBT_VECTOR_UINT:
                case FBT_VECTOR_FLOAT:
                case FBT_VECTOR_KEY:
                case FBT_VECTOR_STRING:
                case FBT_VECTOR_BOOL:
                    return sb.append(asVector());
                case FBT_VECTOR_INT2:
                case FBT_VECTOR_UINT2:
                case FBT_VECTOR_FLOAT2:
                case FBT_VECTOR_INT3:
                case FBT_VECTOR_UINT3:
                case FBT_VECTOR_FLOAT3:
                case FBT_VECTOR_INT4:
                case FBT_VECTOR_UINT4:
                case FBT_VECTOR_FLOAT4:

                    throw new FlexBufferException("not_implemented:" + type);
                default:
                    return sb;
            }
        }
    }

    /**
     * Base class of all types below.
     * Points into the data buffer and allows access to one type.
     */
    private static abstract class Object {
        ByteBuffer bb;
        int end;
        int byteWidth;

        Object(ByteBuffer buff, int end, int byteWidth) {
            this.bb = buff;
            this.end = end;
            this.byteWidth = byteWidth;
        }

        @Override
        public String toString() {
            return toString(new StringBuilder(128)).toString();
        }

        public abstract StringBuilder toString(StringBuilder sb);
    }

    // Stores size in `byte_width_` bytes before end position.
    private static abstract class Sized extends Object {
        Sized(ByteBuffer buff, int end, int byteWidth) {
            super(buff, end, byteWidth);
        }

        public int size() {
            return readInt(bb, end - byteWidth, byteWidth);
        }
    }

    public static class Blob extends Sized {
        static final Blob EMPTY = new Blob(EMPTY_BB, 0, 1);

        Blob(ByteBuffer buff, int end, int byteWidth) {
            super(buff, end, byteWidth);
        }

        public static Blob empty() {
            return EMPTY;
        }

        /**
         * @return blob as a {@link ByteBuffer}
         */
        public ByteBuffer data() {
            ByteBuffer dup = bb.duplicate();
            dup.position(end);
            dup.limit(end + size());
            return dup.asReadOnlyBuffer().slice();
        }

        /**
         * @return blob as a byte array
         */
        public byte[] getBytes() {
            int size = size();
            byte[] result = new byte[size];
            for (int i = 0; i < size; i++) {
                result[i] = bb.get(end + i);
            }
            return result;
        }

        @Override
        public String toString() {
            return Utf8.getDefault().decodeUtf8(bb, end, size());
        }

        @Override
        public StringBuilder toString(StringBuilder sb) {
            sb.append('"');
            sb.append(Utf8.getDefault().decodeUtf8(bb, end, size()));
            return sb.append('"');
        }
    }

    public static class Key extends Object {

        private static final Key EMPTY = new Key(EMPTY_BB, 0, 0);

        Key(ByteBuffer buff, int end, int byteWidth) {
            super(buff, end, byteWidth);
        }

        public static Key empty() {
            return Key.EMPTY;
        }

        @Override
        public StringBuilder toString(StringBuilder sb) {
            int size;
            for (int i = end; ; i++) {
                if (bb.get(i) == 0) {
                    size = i - end;
                    break;
                }
            }
            sb.append(Utf8.getDefault().decodeUtf8(bb, end, size));
            return sb;
        }

        int compareTo(byte[] other) {
            int ia = end;
            int io = 0;
            byte c1, c2;
            do {
                c1 = bb.get(ia);
                c2 = other[io];
                if (c1 == '\0')
                    return c1 - c2;
                ia++;
                io++;
                if (io == other.length) {
                    // in our buffer we have an additional \0 byte
                    // but this does not exist in regular Java strings, so we return now
                    return c1 - c2;
                }
            }
            while (c1 == c2);
            return c1 - c2;
        }

        @Override
        public boolean equals(java.lang.Object obj) {
            if (!(obj instanceof Key))
                return false;

            return ((Key) obj).end == end && ((Key) obj).byteWidth == byteWidth;
        }
    }

    /**
     * Map object representing a set of key-value pairs.
     */
    public static class Map extends Vector {
        private static final Map EMPTY_MAP = new Map(EMPTY_BB, 0, 0);

        Map(ByteBuffer bb, int end, int byteWidth) {
            super(bb, end, byteWidth);
        }

        public static Map empty() {
            return EMPTY_MAP;
        }

        /**
         * @param key access key to element on map
         * @return reference to value in map
         */
        public Reference get(String key) {
            KeyVector keys = keys();
            int size = keys.size();
            int index = binarySearch(keys, key.getBytes(StandardCharsets.UTF_8));
            if (index >= 0 && index < size) {
                return get(index);
            }
            return Reference.NULL_REFERENCE;
        }

        /**
         * Get a vector or keys in the map
         *
         * @return vector of keys
         */
        public KeyVector keys() {
            final int num_prefixed_fields = 3;
            int keysOffset = end - (byteWidth * num_prefixed_fields);
            return new KeyVector(new TypedVector(bb,
                    indirect(bb, keysOffset, byteWidth),
                    readInt(bb, keysOffset + byteWidth, byteWidth),
                    FBT_KEY));
        }

        /**
         * @return {@code Vector} of values from map
         */
        public Vector values() {
            return new Vector(bb, end, byteWidth);
        }

        /**
         * Writes text (json) representation of map in a {@code StringBuilder}.
         *
         * @param builder {@code StringBuilder} to be appended to
         * @return Same {@code StringBuilder} with appended text
         */
        public StringBuilder toString(StringBuilder builder) {
            builder.append("{ ");
            KeyVector keys = keys();
            int size = (int) size();
            Vector vals = values();
            for (int i = 0; i < size; i++) {
                builder.append('"')
                        .append(keys.get(i).toString())
                        .append("\" : ");
                builder.append(vals.get(i).toString());
                if (i != size - 1)
                    builder.append(", ");
            }
            builder.append(" }");
            return builder;
        }

        // Performs a binary search on a key vector and return index of the key in key vector
        private int binarySearch(KeyVector keys, byte[] searchedKey) {
            int low = 0;
            int high = keys.size() - 1;

            while (low <= high) {
                int mid = (low + high) >>> 1;
                Key k = keys.get(mid);
                int cmp = k.compareTo(searchedKey);
                if (cmp < 0)
                    low = mid + 1;
                else if (cmp > 0)
                    high = mid - 1;
                else
                    return mid; // key found
            }
            return -(low + 1);  // key not found
        }
    }

    /**
     * Object that represents a set of elements in the buffer
     */
    public static class Vector extends Sized {

        private static final Vector EMPTY_VECTOR = new Vector(ByteBuffer.allocate(0), 1, 1);

        Vector(ByteBuffer bb, int end, int byteWidth) {
            super(bb, end, byteWidth);
        }

        public static Vector empty() {
            return EMPTY_VECTOR;
        }

        public boolean isEmpty() {
            return this == EMPTY_VECTOR;
        }

        @Override
        public StringBuilder toString(StringBuilder sb) {
            sb.append("[ ");
            int size = size();
            for (int i = 0; i < size; i++) {
                get(i).toString(sb);
                if (i != size - 1) {
                    sb.append(", ");
                }
            }
            sb.append(" ]");
            return sb;
        }

        /**
         * Get a element in a vector by index
         *
         * @param index position of the element
         * @return {@code Reference} to the element
         */
        public Reference get(int index) {
            long len = size();
            if (index >= len) {
                return Reference.NULL_REFERENCE;
            }
            int packedType = Byte.toUnsignedInt(bb.get((int) (end + (len * byteWidth) + index)));
            int obj_end = end + index * byteWidth;
            return new Reference(bb, obj_end, byteWidth, packedType);
        }
    }

    /**
     * Object that represents a set of elements with the same type
     */
    public static class TypedVector extends Vector {

        private static final TypedVector EMPTY_VECTOR = new TypedVector(EMPTY_BB, 0, 1, FBT_INT);

        private final int elemType;

        TypedVector(ByteBuffer bb, int end, int byteWidth, int elemType) {
            super(bb, end, byteWidth);
            this.elemType = elemType;
        }

        public static TypedVector empty() {
            return EMPTY_VECTOR;
        }

        /**
         * Returns whether the vector is empty
         *
         * @return true if empty
         */
        public boolean isEmptyVector() {
            return this == EMPTY_VECTOR;
        }

        /**
         * Return element type for all elements in the vector
         *
         * @return element type
         */
        public int getElemType() {
            return elemType;
        }

        /**
         * Get reference to an object in the {@code Vector}
         *
         * @param pos position of the object in {@code Vector}
         * @return reference to element
         */
        @Override
        public Reference get(int pos) {
            int len = size();
            if (pos >= len) return Reference.NULL_REFERENCE;
            int childPos = end + pos * byteWidth;
            return new Reference(bb, childPos, byteWidth, 1, elemType);
        }
    }

    /**
     * Represent a vector of keys in a map
     */
    public static class KeyVector {

        private final TypedVector vec;

        KeyVector(TypedVector vec) {
            this.vec = vec;
        }

        /**
         * Return key
         *
         * @param pos position of the key in key vector
         * @return key
         */
        public Key get(int pos) {
            return vec.get(pos).asKey();
        }

        /**
         * Returns size of key vector
         *
         * @return size
         */
        public int size() {
            return vec.size();
        }

        public String toString() {
            StringBuilder b = new StringBuilder();
            b.append('[');
            for (int i = 0; i < vec.size(); i++) {
                vec.get(i).toString(b);
                if (i != vec.size() - 1) {
                    b.append(", ");
                }
            }
            return b.append("]").toString();
        }
    }

    public static class FlexBufferException extends RuntimeException {
        FlexBufferException(String msg) {
            super(msg);
        }
    }

    public static class InvalidTypeConversionFlexBufferException extends FlexBufferException {
        InvalidTypeConversionFlexBufferException(int from, int to) {
            super(String.format("Unable to convert type %s to %s", from, to));
        }
    }
}
