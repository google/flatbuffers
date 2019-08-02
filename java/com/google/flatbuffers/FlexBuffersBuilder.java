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

import com.google.flatbuffers.FlexBuffers.BitWidth;

import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;

import static com.google.flatbuffers.FlexBuffers.*;
import static com.google.flatbuffers.FlexBuffers.BitWidth.*;

/**
 * A class that generates FlexBuffers
 * <p>
 * This class presents all necessary APIs to create FlexBuffers. The {@link ByteBuffer } buffer used to store the
 * data can be created internally, or passed down in the constructor.
 * <p>
 * Because it uses {@link ByteBuffer} internally, this impose some limitations in generating FlexBuffers. Mostly noted,
 * the maximum size limitation on FlexBuffer message, which is {@link Integer#MAX_VALUE}.
 *
 * <p>There is also some differences from the original implementation in C++. It can changed in future updates.
 * <ul>
 *
 *   <li><p>No support for mutations (might change in the future).</p></li>
 *
 *   <li><p>Size of message limited to {@link Integer#MAX_VALUE}</p></li>
 *
 *   <li><p>Since Java does not support unsigned type, all unsigned operations accepts a immediate higher representation
 *   of similar type. Unsigned long is not supported</p></li>
 * </ul>
 * </p>
 */
public class FlexBuffersBuilder {
    /**
     * No keys or strings will be shared
     */
    public static final int BUILDER_FLAG_NONE = 0;
    /**
     * Keys will be shared between elements. Identical keys will only be serialized once, thus possibly saving space.
     * But serialization performance might be slower and consumes more memory.
     */
    public static final int BUILDER_FLAG_SHARE_KEYS = 1;
    /**
     * Strings will be shared between elements. Identical strings will only be serialized once, thus possibly saving space.
     * But serialization performance might be slower and consumes more memory. This is ideal if you expect many repeated
     * strings on the message.
     */
    public static final int BUILDER_FLAG_SHARE_STRINGS = 1;
    /**
     * Strings and keys will be shared between elements.
     */
    public static final int BUILDER_FLAG_SHARE_KEYS_AND_STRINGS = 3;
    /**
     * Reserved for the future.
     */
    public static final int BUILDER_FLAG_SHARE_KEY_VECTORS = 4;
    /**
     * Reserved for the future.
     */
    public static final int BUILDER_FLAG_SHARE_ALL = 7;

    private final ByteBuffer bb;
    private final ArrayList<KeyValue> stack = new ArrayList<>();
    private final HashMap<String, Integer> keyPool = new HashMap<>();
    private final HashMap<String, Integer> stringPool = new HashMap<>();
    private final int flags;
    private boolean finished = false;

    /**
     * Constructs a newly allocated {@code FlexBuffersBuilder} with {@link #BUILDER_FLAG_SHARE_KEYS} set.
     */
    public FlexBuffersBuilder() {
        this(ByteBuffer.allocate(256), BUILDER_FLAG_SHARE_KEYS);
    }

    /**
     * Constructs a newly allocated {@code FlexBuffersBuilder}.
     *
     * @param bb    ByteBuffer that will hold the message
     * @param flags Share flags
     */
    public FlexBuffersBuilder(ByteBuffer bb, int flags) {
        this.bb = bb;
        this.flags = flags;
        bb.order(ByteOrder.LITTLE_ENDIAN);
        bb.position(0);
    }

    /**
     * Constructs a newly allocated {@code FlexBuffersBuilder}.
     *
     * @param bb ByteBuffer that will hold the message
     */
    public FlexBuffersBuilder(ByteBuffer bb) {
        this(bb, BUILDER_FLAG_SHARE_KEYS);
    }

    /**
     * Return {@code ByteBuffer} containing FlexBuffer message. {@code #finish()} must be called before calling this
     * function otherwise an assert will trigger.
     *
     * @return {@code ByteBuffer} with finished message
     */
    public ByteBuffer getBuffer() {
        assert (finished);
        return bb;
    }

    /**
     * Insert a single boolean into the buffer
     * @param val true or false
     */
    public void putBoolean(boolean val) {
        putBoolean(null, val);
    }

    private void putBoolean(String key, boolean val) {
        Value kKey = putKey(key);
        stack.add(new KeyValue(kKey, Value.bool(val)));
    }

    private Value putKey(String key) {
        if (key == null) {
            return null;
        }
        int pos = bb.position();
        if ((flags & BUILDER_FLAG_SHARE_KEYS) != 0) {
            if (keyPool.get(key) == null) {
                bb.put(key.getBytes(StandardCharsets.UTF_8));
                bb.put((byte) 0);
                keyPool.put(key, pos);
            } else {
                pos = keyPool.get(key);
            }
        } else {
            bb.put(key.getBytes(StandardCharsets.UTF_8));
            bb.put((byte) 0);
            keyPool.put(key, pos);
        }
        return Value.key(pos);
    }

    /**
     * Adds a integer into the buff
     * @param val integer
     */
    public void putInt(int val) {
        putInt(null, val);
    }

    private void putInt(String key, int val) {
        putInt(key, (long) val);
    }

    private void putInt(String key, long val) {
        Value vKey = putKey(key);
        Value vVal;
        if (Byte.MIN_VALUE <= val && val <= Byte.MAX_VALUE) {
            vVal = Value.int8((int) val);
        } else if (Short.MIN_VALUE <= val && val <= Short.MAX_VALUE) {
            vVal = Value.int16((int) val);
        } else if (Integer.MIN_VALUE <= val && val <= Integer.MAX_VALUE) {
            vVal = Value.int32((int) val);
        } else {
            vVal = Value.int64(val);
        }
        stack.add(new KeyValue(vKey, vVal));
    }

    /**
     * Adds a 64-bit integer into the buff
     * @param value integer
     */
    public void putInt(long value) {
        putInt(null, value);
    }

    /**
     * Adds a unsigned integer into the buff.
     * @param value integer representing unsigned value
     */
    public void putUInt(int value) {
        putUInt(null, (long) value);
    }

    /**
     * Adds a unsigned integer (stored in a signed 64-bit integer) into the buff.
     * @param value integer representing unsigned value
     */
    public void putUInt(long value) {
        putUInt(null, value);
    }

    /**
     * Adds a 64-bit unsigned integer (stored as {@link BigInteger}) into the buff.
     * Warning: This operation might be very slow.
     * @param value integer representing unsigned value
     */
    public void putUInt64(BigInteger value) {
        putUInt64(null, value.longValue());
    }

    private void putUInt64(String key, long value) {
        Value vKey = putKey(key);
        Value vVal = Value.uInt64(value);
        stack.add(new KeyValue(vKey, vVal));
    }

    private void putUInt(String key, long value) {
        Value vKey = putKey(key);
        Value vVal;

        BitWidth width = widthUInBits(value);

        if (width == WIDTH_8) {
            vVal = Value.uInt8((int)value);
        } else if (width == WIDTH_16) {
            vVal = Value.uInt16((int)value);
        } else if (width == WIDTH_32) {
            vVal = Value.uInt32((int)value);
        } else {
            vVal = Value.uInt64(value);
        }
        stack.add(new KeyValue(vKey, vVal));
    }

    /**
     * Adds a 32-bit float into the buff.
     * @param value float representing value
     */
    public void putFloat(float value) {
        putFloat(null, value);
    }

    private void putFloat(String key, float val) {
        Value vKey = putKey(key);
        stack.add(new KeyValue(vKey, Value.float32(val)));
    }

    /**
     * Adds a 64-bit float into the buff.
     * @param value float representing value
     */
    public void putFloat(double value) {
        putDouble(null, value);
    }

    private void putDouble(String key, double val) {
        Value vKey = putKey(key);
        stack.add(new KeyValue(vKey, Value.float64(val)));
    }

    /**
     * Adds a String into the buffer
     * @param value string
     * @return start position of string in the buffer
     */
    public int putString(String value) {
        return putString(null, value);
    }

    private int putString(String key, String val) {
        Value vKey = putKey(key);
        if ((flags & FlexBuffersBuilder.BUILDER_FLAG_SHARE_STRINGS) != 0) {
            Integer i = stringPool.get(val);
            if (i == null) {
                Value vValue = writeString(val);
                stringPool.put(val, (int) vValue.iValue);
                stack.add(new KeyValue(vKey, vValue));
                return (int) vValue.iValue;
            } else {
                BitWidth bitWidth = widthUInBits(val.length());
                Value vValue = Value.blob(i, FBT_STRING, bitWidth);
                stack.add(new KeyValue(vKey, vValue));
                return i;
            }
        } else {
            Value vValue = writeString(val);
            stack.add(new KeyValue(vKey, vValue));
            return (int) vValue.iValue;
        }
    }

    private Value writeString(String s) {
        return writeBlob(s.getBytes(StandardCharsets.UTF_8), FBT_STRING);
    }

    // in bits to fit a unsigned int
    private static BitWidth widthUInBits(long len) {
        if (len <= Byte.toUnsignedInt((byte)0xff)) return WIDTH_8;
        if (len <= Short.toUnsignedInt((short)0xffff)) return WIDTH_16;
        if (len <= Integer.toUnsignedLong(0xffff_ffff)) return WIDTH_32;
        return WIDTH_64;
    }

    private Value writeBlob(byte[] blob, int type) {
        BitWidth bitWidth = widthUInBits(blob.length);
        int byteWidth = align(bitWidth);
        writeInt(blob.length, byteWidth);
        int sloc = bb.position();
        bb.put(blob);
        if (type == FBT_STRING) {
            bb.put((byte) 0);
        }
        return Value.blob(sloc, type, bitWidth);
    }

    // Align to prepare for writing a scalar with a certain size.
    private int align(BitWidth alignment) {
        int byteWidth = 1 << alignment.val;
        int padBytes = Value.paddingBytes(bb.capacity(), byteWidth);
        while (padBytes-- != 0) {
            bb.put((byte) 0);
        }
        return byteWidth;
    }

    private void writeInt(long value, int byteWidth) {
        switch (byteWidth) {
            case 1: bb.put((byte) value); break;
            case 2: bb.putShort((short) value); break;
            case 4: bb.putInt((int) value); break;
            case 8: bb.putLong(value); break;
            default:
                throw new FlexBuffers.FlexBufferException(String.format("Invalid byte width of %d", byteWidth));
        }
    }

    /**
     * Adds a byte array into the message
     * @param value byte array
     * @return position in buffer as the start of byte array
     */
    public int putBlob(byte[] value) {
        return putBlob(null, value);
    }

    private int putBlob(String key, byte[] val) {
        Value vKey = putKey(key);
        Value vValue = writeBlob(val, FBT_BLOB);
        stack.add(new KeyValue(vKey, vValue));
        return (int) vValue.iValue;
    }

    /**
     * Adds a vector into the message
     * @param vbc callback where elements can be inserted into the vector
     * @return position in buffer as the start of the vector
     */
    public int putVector(VectorBuilderCallback vbc) {
        return putVector(null, vbc);
    }

    private int putVector(String key, VectorBuilderCallback vbc) {
        int start = stack.size();
        VectorBuilder vb = new VectorBuilder(this);
        vbc.run(vb);
        return endVector(key, start, vb.typed, false);
    }

    private int endVector(String key, int start, boolean typed, boolean fixed) {
        Value vKey = putKey(key);
        Value vec = createVector(start, stack.size() - start, typed, fixed, null);
        // Remove temp elements and return vector.
        while (stack.size() > start) {
            stack.remove(stack.size() - 1);
        }
        stack.add(new KeyValue(vKey, vec));
        return (int) vec.iValue;
    }

    /**
     * Adds a map into the message
     * @param vbc  callback where elements can be inserted into the map
     * @return position in buffer as the start of the map
     */
    public int putMap(MapBuilderCallback vbc) {
        return putMap(null, vbc);
    }

    /**
     * Finish writing the message into the buffer. After that no other element must
     * be inserted into the buffer. Also, you must call this function before start using the
     * FlexBuffer message
     * @return ByteBuffer containing the FlexBuffer message
     */
    public ByteBuffer finish() {
        // If you hit this assert, you likely have objects that were never included
        // in a parent. You need to have exactly one root to finish a buffer.
        // Check your Start/End calls are matched, and all objects are inside
        // some other object.
        assert (stack.size() == 1);
        // Write root value.
        int byteWidth = align(stack.get(0).value.elemWidth(bb.position(), 0));
        writeAny(stack.get(0).value, byteWidth);
        // Write root type.
        bb.put(stack.get(0).value.storedPackedType());
        // Write root size. Normally determined by parent, but root has no parent :)
        bb.put((byte) byteWidth);
        bb.limit(bb.position());
        this.finished = true;
        return bb;
    }

    /*
     * Create a vector based on the elements stored in the stack
     *
     * @param start  element in the stack
     * @param length size of the vector
     * @param typed  whether is TypedVector or not
     * @param fixed  whether is Fixed vector or not
     * @return Value representing the created vector
     */
    private Value createVector(int start, int length, boolean typed, boolean fixed, Value key) {
        assert (!fixed || typed); // typed=false, fixed=true combination is not supported.
        // Figure out smallest bit width we can store this vector with.
        BitWidth bitWidth = BitWidth.max(WIDTH_8, widthUInBits(length));
        int prefixElems = 1;
        if (key != null) {
            // If this vector is part of a map, we will pre-fix an offset to the keys
            // to this vector.
            bitWidth = BitWidth.max(bitWidth, key.elemWidth(bb.position(), 0));
            prefixElems += 2;
        }
        int vectorType = FBT_KEY;
        // Check bit widths and types for all elements.
        for (int i = start; i < stack.size(); i++) {
            BitWidth elemWidth = stack.get(i).value.elemWidth(bb.position(), i + prefixElems);
            bitWidth = BitWidth.max(bitWidth, elemWidth);
            if (typed) {
                if (i == start) {
                    vectorType = stack.get(i).value.type;
                } else {
                    // If you get this assert, you are writing a typed vector with
                    // elements that are not all the same type.
                    assert (vectorType == stack.get(i).value.type);
                }
            }
        }
        // If you get this assert, your fixed types are not one of:
        // Int / UInt / Float / Key.
        assert (!fixed || FlexBuffers.isTypedVectorElementType(vectorType));

        int byteWidth = align(bitWidth);
        // Write vector. First the keys width/offset if available, and size.
        if (key != null) {
            writeOffset(key.iValue, byteWidth);
            writeInt(1L << key.minBitWidth.val, byteWidth);
        }
        if (!fixed) {
            writeInt(length, byteWidth);
        }
        // Then the actual data.
        int vloc = bb.position();
        for (int i = start; i < stack.size(); i++) {
            writeAny(stack.get(i).value, byteWidth);
        }
        // Then the types.
        if (!typed) {
            for (int i = start; i < stack.size(); i++) {
                bb.put(stack.get(i).value.storedPackedType(bitWidth));
            }
        }
        return new Value(key != null ? FBT_MAP
                : (typed ? FlexBuffers.toTypedVector(vectorType, fixed ? length : 0)
                : FBT_VECTOR), bitWidth, vloc);
    }

    private void writeOffset(long val, int byteWidth) {
        int reloff = (int) (bb.position() - val);
        assert (byteWidth == 8 || reloff < 1L << (byteWidth * 8));
        writeInt(reloff, byteWidth);
    }

    private void writeAny(final Value val, int byteWidth) {
        switch (val.type) {
            case FBT_NULL:
            case FBT_BOOL:
            case FBT_INT:
            case FBT_UINT:
                writeInt(val.iValue, byteWidth);
                break;
            case FBT_FLOAT:
                writeDouble(val.dValue, byteWidth);
                break;
            default:
                writeOffset(val.iValue, byteWidth);
                break;
        }
    }

    private void writeDouble(double val, int byteWidth) {
        if (byteWidth == 4) {
            bb.putFloat((float) val);
        } else if (byteWidth == 8) {
            bb.putDouble(val);
        } else {
            throw new FlexBuffers.FlexBufferException(String.format("Invalid byte width of %d", byteWidth));
        }
    }

    private int putMap(String key, MapBuilderCallback vbc) {
        int start = stack.size();
        vbc.run(new MapBuilder(this));
        return endMap(key, start);
    }

    private int endMap(String key, int start) {
        Value vKey = putKey(key);

        // Make sure keys are all strings:
        for (int i = start; i < stack.size(); i++) {
            assert (stack.get(i) != null && stack.get(i).key.type == FBT_KEY);
        }

        Collections.sort(stack.subList(start, stack.size()));

        Value keys = createKeyVector(start, stack.size() - start);
        Value vec = createVector(start, stack.size() - start, false, false, keys);
        // Remove temp elements and return map.
        while (stack.size() > start) {
            stack.remove(stack.size() - 1);
        }
        stack.add(new KeyValue(vKey, vec));
        return (int) vec.iValue;
    }

    private Value createKeyVector(int start, int length) {
        // Figure out smallest bit width we can store this vector with.
        BitWidth bitWidth = BitWidth.max(WIDTH_8, widthUInBits(length));
        int prefixElems = 1;
        int vectorType = FBT_KEY;
        // Check bit widths and types for all elements.
        for (int i = start; i < stack.size(); i++) {
            BitWidth elemWidth = stack.get(i).key.elemWidth(bb.position(), i + prefixElems);
            bitWidth = BitWidth.max(bitWidth, elemWidth);
            if (i == start) {
                vectorType = stack.get(i).key.type;
            } else {
                // If you get this assert, you are writing a typed vector with
                // elements that are not all the same type.
                assert (vectorType == stack.get(i).key.type);
            }
        }

        int byteWidth = align(bitWidth);
        // Write vector. First the keys width/offset if available, and size.
        writeInt(length, byteWidth);
        // Then the actual data.
        int vloc = bb.position();
        for (int i = start; i < stack.size(); i++) {
            writeAny(stack.get(i).key, byteWidth);
        }
        // Then the types.
        return new Value(FlexBuffers.toTypedVector(vectorType,0), bitWidth, vloc);
    }

    /**
     * Callback interface used to add elements to a vector
     */
    public interface VectorBuilderCallback {
        void run(VectorBuilder vb);
    }

    /**
     * Callback interface used to add elements to a map
     */
    public interface MapBuilderCallback {
        void run(MapBuilder vb);
    }

    /**
     * This class will be use to construct vectors in a FlexBuffer message
     */
    public static class VectorBuilder {
        final FlexBuffersBuilder builder;
        boolean typed = true;
        int elementType = -1;

        private VectorBuilder(FlexBuffersBuilder builder) {
            this.builder = builder;
        }

        private void checkType(int nextElementType) {
            if (!typed) {
                // we already no its know typed, so no need to check
                return;
            }
            if (FlexBuffers.isTypedVectorElementType(nextElementType) && elementType == -1) {
                // we keep first element type, to check whether we can make the vector typed or not
                elementType = nextElementType;
                return;
            }
            if (!FlexBuffers.isTypedVectorElementType(nextElementType) || nextElementType != elementType) {
                typed = false;
            }
        }

        /**
         * Puts a boolean into the current vector
         *
         * @param value true or false
         */
        public void putBoolean(boolean value) {
            checkType(FBT_BOOL);
            builder.putBoolean(null, value);
        }

        /**
         * Puts a integer into the current vector
         *
         * @param value 32 bits or less int
         */
        public void putInt(int value) {
            checkType(FBT_INT);
            builder.putInt(null, value);
        }

        /**
         * Puts a long (64bits) into the current vector
         *
         * @param value 64 bits or less int
         */
        public void putInt(long value) {
            checkType(FBT_INT);
            builder.putInt(null, value);
        }

        /**
         * Put a 64bits float into the current vector
         *
         * @param value double value
         */
        public void putFloat(double value) {
            checkType(FBT_FLOAT);
            builder.putDouble(null, value);
        }

        /**
         * Put a 32bits float into the current vector
         *
         * @param value float value
         */
        public void putFloat(float value) {
            checkType(FBT_FLOAT);
            builder.putFloat(null, value);
        }

        /**
         * Puts a string into the current vector
         *
         * @param value string to be inserted
         * @return position in buffer where string is stored.
         */
        public int putString(String value) {
            checkType(FBT_STRING);
            return builder.putString(null, value);
        }

        /**
         * Puts a blob (array of bytes) into the current vector
         *
         * @param value byte array to be inserted
         * @return position in buffer where blob is stored.
         */
        public long putBlob(byte[] value) {
            checkType(FBT_BLOB);
            return builder.putBlob(null, value);
        }

        /**
         * Puts a new vector into the current vector
         *
         * @param vbc callback to be used to add elements to the inserted vector
         * @return position in buffer where vector is stored.
         */
        public int putVector(VectorBuilderCallback vbc) {
            return builder.putVector(null, vbc);
        }

        /**
         * Puts a new map into the current vector
         *
         * @param mbc callback to be used to add elements to the inserted map
         * @return position in buffer where map is stored.
         */
        public int putMap(MapBuilderCallback mbc) {
            checkType(FBT_MAP);
            int start = builder.stack.size();
            mbc.run(new MapBuilder(builder));
            return builder.endMap(null, start);
        }
    }

    /**
     * Helper class to add key value style map into the FlexBuffer message
     */
    public static class MapBuilder {
        final FlexBuffersBuilder builder;

        private MapBuilder(FlexBuffersBuilder builder) {
            this.builder = builder;
        }

        /**
         * Puts a boolean into the current map
         *
         * @param key key string associated
         * @param value true or false
         */
        public void putBoolean(String key, boolean value) {
            builder.putBoolean(key, value);
        }

        /**
         * Puts a integer into the current map
         *
         * @param key key string associated
         * @param value 32 bits or less int
         */
        public void putInt(String key, int value) {
            builder.putInt(key, value);
        }

        /**
         * Puts a long (64bits) into the current map
         *
         * @param key key string associated
         * @param value 64 bits or less int
         */
        public void putInt(String key, long value) {
            builder.putInt(key, value);
        }

        /**
         * Put a 32bits float into the current map
         *
         * @param key key string associated
         * @param value float value
         */
        public void putFloat(String key, float value) {
            builder.putFloat(key, value);
        }

        /**
         * Put a 64bits float into the current map
         *
         * @param key key string associated
         * @param value double value
         */
        public void putFloat(String key, double value) {
            builder.putDouble(key, value);
        }

        /**
         * Puts a string into the current map
         *
         * @param key key string associated
         * @param value string to be inserted
         * @return position in buffer where string is stored.
         */
        public void putString(String key, String value) {
            builder.putString(key, value);
        }

        /**
         * Puts a blob (array of bytes) into the current vector
         *
         * @param key key string associated
         * @param value byte array to be inserted
         * @return position in buffer where blob is stored.
         */
        public long putBlob(String key, byte[] value) {
            return builder.putBlob(key, value);
        }

        /**
         * Puts a new vector into the current vector
         *
         * @param key key string associated
         * @param vbc callback to be used to add elements to the inserted vector
         * @return position in buffer where vector is stored.
         */
        public int putVector(String key, VectorBuilderCallback vbc) {
            return builder.putVector(key, vbc);
        }

        /**
         * Puts a new map into the current vector
         *
         * @param key key string associated
         * @param mbc callback to be used to add elements to the inserted map
         * @return position in buffer where map is stored.
         */
        public int putMap(String key, MapBuilderCallback mbc) {
            int start = builder.stack.size();
            mbc.run(new MapBuilder(builder));
            return builder.endMap(key, start);
        }
    }

    public static class Value {
        final int type;
        // for scalars, represents scalar size in bytes
        // for vectors, represents the size
        // for string, length
        final BitWidth minBitWidth;
        // float value
        final double dValue;
        // integer value
        long iValue;

        Value(int type, BitWidth bitWidth, long iValue) {
            this.type = type;
            this.minBitWidth = bitWidth;
            this.iValue = iValue;
            this.dValue = Double.MIN_VALUE;
        }

        Value(int type, BitWidth bitWidth, double dValue) {
            this.type = type;
            this.minBitWidth = bitWidth;
            this.dValue = dValue;
            this.iValue = Long.MIN_VALUE;
        }

        static Value key(int position) {
            return new Value(FBT_KEY, WIDTH_8, position);
        }

        static Value bool(boolean b) {
            return new Value(FBT_BOOL, WIDTH_8, b ? 1 : 0);
        }

        static Value blob(int position, int type, BitWidth bitWidth) {
            return new Value(type, WIDTH_8, position);
        }

        static Value int8(int value) {
            return new Value(FBT_INT, WIDTH_8, value);
        }

        static Value int16(int value) {
            return new Value(FBT_INT, WIDTH_16, value);
        }

        static Value int32(int value) {
            return new Value(FBT_INT, WIDTH_32, value);
        }

        static Value int64(long value) {
            return new Value(FBT_INT, WIDTH_64, value);
        }

        static Value uInt8(int value) {
            return new Value(FBT_UINT, WIDTH_8, value);
        }

        static Value uInt16(int value) {
            return new Value(FBT_UINT, WIDTH_16, value);
        }

        static Value uInt32(int value) {
            return new Value(FBT_UINT, WIDTH_32, value);
        }

        static Value uInt64(long value) {
            return new Value(FBT_UINT, WIDTH_64, value);
        }

        static Value float32(float value) {
            return new Value(FBT_FLOAT, WIDTH_32, value);
        }

        static Value float64(double value) {
            return new Value(FBT_FLOAT, WIDTH_64, value);
        }

        private byte storedPackedType() {
            return storedPackedType(WIDTH_8);
        }

        private byte storedPackedType(BitWidth parentBitWidth) {
            return packedType(storedWidth(parentBitWidth), type);
        }

        private static byte packedType(BitWidth bitWidth, int type) {
            return (byte) (bitWidth.val | (type << 2));
        }

        private BitWidth storedWidth(BitWidth parentBitWidth) {
            if (FlexBuffers.isTypeInline(type)) {
                return BitWidth.max(minBitWidth, parentBitWidth);
            } else {
                return minBitWidth;
            }
        }

        private BitWidth elemWidth(int bufSize, int elemIndex) {
            if (FlexBuffers.isTypeInline(type)) {
                return minBitWidth;
            } else {
                // We have an absolute offset, but want to store a relative offset
                // elem_index elements beyond the current buffer end. Since whether
                // the relative offset fits in a certain byte_width depends on
                // the size of the elements before it (and their alignment), we have
                // to test for each size in turn.

                // Original implementation checks for largest scalar
                // which is long unsigned int
                for (int byteWidth = 1; byteWidth <= 32; byteWidth *= 2) {
                    // Where are we going to write this offset?
                    int offsetLoc = bufSize + paddingBytes(bufSize, byteWidth) + (elemIndex * byteWidth);
                    // Compute relative offset.
                    long offset = offsetLoc - iValue;
                    // Does it fit?
                    BitWidth bitWidth = widthUInBits((int) offset);
                    if (((1L) << bitWidth.val) == byteWidth)
                        return bitWidth;
                }
                assert (false);  // Must match one of the sizes above.
                return WIDTH_64;
            }
        }

        private static int paddingBytes(int bufSize, int scalarSize) {
            return ((~bufSize) + 1) & (scalarSize - 1);
        }
    }

    /**
     * Represents a FlexBuffer element.
     * can be an scalars, vector or strings
     */
    private class KeyValue implements Comparable<KeyValue> {
        Value key;
        Value value;

        KeyValue(Value key, Value value) {
            this.key = key;
            this.value = value;
        }

        @Override
        public int compareTo(KeyValue o) {
            int ia = (int) key.iValue;
            int io = (int) o.key.iValue;
            byte c1, c2;
            do {
                c1 = bb.get(ia);
                c2 = bb.get(io);
                if (c1 == '\0')
                    return c1 - c2;
                ia++;
                io++;
            }
            while (c1 == c2);
            return c1 - c2;
        }
    }
}
