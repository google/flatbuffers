/*
 * Copyright 2016 Google Inc. All rights reserved.
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
 
module flatbuffers.flatbufferbuilder;

import flatbuffers.exception;
import flatbuffers.bytebuffer;

import std.exception;
import std.traits : isNumeric;

/**
    Responsible for building up and accessing a FlatBuffer formatted byte
*/
final class FlatBufferBuilder
{
    /**
        Create a FlatBufferBuilder with a given initial size.
        Params:
            initsize = The initial size to use for the internal buffer.
    */
    this(size_t initsize = 32)
	in{
		assert(initsize > 0);
	}body{
		this(new ByteBuffer(new ubyte[initsize]));
    }

	this(ByteBuffer buffer){
		_space = buffer.length;
		_buffer = buffer;
	}

    uint offset()
    {
        return cast(uint)(_buffer.length - _space);
    }

    void pad(size_t size)
    {
        for (int i = 0; i < size; i++)
        {
            --_space;
            _buffer.put!ubyte(_space, 0x00);
        }
    }

    /** Doubles the size of the ByteBuffer, and copies the old data towards
        the end of the new buffer (since we build the buffer backwards).
    */
    void growBuffer()
    {
        auto oldBuf = _buffer.data;
        auto oldBufSize = oldBuf.length;
        if ((oldBufSize & 0xC0000000) != 0)
            throw new Exception("FlatBuffers: cannot grow buffer beyond 2 gigabytes.");

		auto newBufSize = oldBufSize >= 32 ? oldBufSize * 2 : 64;
        auto newBuf = new ubyte[](newBufSize);
        newBuf[(newBufSize - oldBufSize) .. $] = oldBuf[];
		_buffer.restData(newBuf,0);
    }

    /**
        Prepare to write an element of `size` after `additional_bytes`
        have been written, e.g. if you write a string, you need to align
        such the int length field is aligned to SIZEOF_INT, and the string
        data follows it directly.
        If all you need to do is align, `additional_bytes` will be 0.
    */
    void prep(size_t size, size_t additionalBytes)
    {
        // Track the biggest thing we've ever aligned to.
        if (size > _minAlign)
            _minAlign = size;

        // Find the amount of alignment needed such that `size` is properly
        // aligned after `additional_bytes`.
        auto alignSize = ((~( _buffer.length - _space + additionalBytes)) + 1) & (size - 1);

        // Reallocate the buffer if needed.
        while (_space < alignSize + size + additionalBytes)
        {
            auto oldBufSize = cast(int) _buffer.length;
            growBuffer();
            _space += cast(int) _buffer.length - oldBufSize;
        }
        if (alignSize > 0)
            pad(alignSize);
    }

    /**
        put a value into the buffer.
    */
    void put(T)(T x) if (is(T == bool) || isNumeric!T)
    {
        static if (is(T == bool))
        {
            _space -= 1;
        }
        else
        {
            _space -= T.sizeof;
        }
        _buffer.put!T(_space, x);
    }

    /// Adds a scalar to the buffer, properly aligned, and the buffer grown if needed.
	void add(T)(T x)if (is(T == bool) || isNumeric!T)
    {
		static if (is(T == bool))
			prep(1, 0);
		else
			prep(T.sizeof, 0);
        put!T(x);
    }
    /// Adds on offset, relative to where it will be written.
    void addOffset(uint off)
    {
		prep(uint.sizeof, 0); // Ensure alignment is already done.
        if (off > offset())
            throw new ArgumentException("FlatBuffers: must be less than offset.", "off");

		off = offset() - off + cast(uint)uint.sizeof;
        put!uint(off);
    }

    void startVector(int elemSize, int count, int alignment)
    {
        notNested();
        _vectorNumElems = count;
        prep(int.sizeof, elemSize * count);
        prep(alignment, elemSize * count); // Just in case alignment > int.
    }

    uint endVector()
    {
        put!int(cast(int)_vectorNumElems);
        return offset();
    }

    void nested(int obj)
    {
        // Structs are always stored inline, so need to be created right
        // where they are used. You'll get this assert if you created it
        // elsewhere.
        if (obj != offset())
            throw new Exception("FlatBuffers: struct must be serialized inline.");
    }

    void notNested()
    {
        // You should not be creating any other objects or strings/vectors
        // while an object is being constructed.
        if (_vtable)
            throw new Exception("FlatBuffers: object serialization must not be nested.");
    }

    void startObject(int numfields)
    {
        if (numfields < 0)
            throw new ArgumentOutOfRangeException("numfields", numfields,
                "must be greater than zero");

        notNested();
        _vtable = new size_t[](numfields);
        _objectStart = offset();
    }

    /// Set the current vtable at `voffset` to the current location in the buffer.
	void slot(size_t voffset)
    {
        _vtable[voffset] = offset();
    }

    /// Add a scalar to a table at `o` into its vtable, with value `x` and default `d`.
    void add(T : bool)(size_t o, T x, T d)
    {
        if (x != d)
        {
            add!T(x);
            slot(o);
        }
    }
    /// ditto
	void add(T)(size_t o, T x, T d) if(isNumeric!T)
    {
        if (x != d)
        {
            add!T(x);
            slot(o);
        }
    }
    /// ditto
    void addOffset(int o, int x, int d)
    {
        if (x != d)
        {
            addOffset(x);
            slot(o);
        }
    }

	/** Structs are stored inline, so nothing additional is being added.
        `d` is always 0.
    */
	void addStruct(int voffset, int x, int d)
	{
		if (x != d)
		{
			nested(x);
			slot(voffset);
		}
	}

    /**
        Encode the string `s` in the buffer using UTF-8.
        Params:
            s = The string to encode.
        Returns:
            The offset in the buffer where the encoded string starts.
    */
    uint createString(string s)
    {
        notNested();
        auto utf8 = cast(ubyte[]) s;
        add!ubyte(cast(ubyte) 0);
        startVector(1, cast(int) utf8.length, 1);
        _space -= utf8.length;
        _buffer.data[_space .. _space + utf8.length] = utf8[];
        return endVector();
    }

    uint endObject()
    {
        if (!_vtable)
            throw new InvalidOperationException(
                "Flatbuffers: calling endObject without a startObject");

        add!int(cast(int) 0);
        auto vtableloc = offset();

        // Write out the current vtable.
        for (int i = cast(int) _vtable.length - 1; i >= 0; i--)
        {
            // Offset relative to the start of the table.
            short off = cast(short)(_vtable[i] != 0 ? vtableloc - _vtable[i] : 0);
            add!short(off);
        }

        const int standardFields = 2; // The fields below:
        add!short(cast(short)(vtableloc - _objectStart));
        add!short(cast(short)((_vtable.length + standardFields) * short.sizeof));

        /// Search for an existing vtable that matches the current one.
        size_t existingVtable = 0;

        ubyte[] data = _buffer.data();

        for (int i = 0; i < _numVtables; i++)
        {
            auto vt1 = _buffer.length - _vtables[i];
            auto vt2 = _space;
            short vt1len = _buffer.get!short(vt1);
            short vt2len = _buffer.get!short(vt2);

            if (vt1len != vt2len || data[vt1 .. (vt1 + vt1len)] != data[vt2 .. (vt2 + vt2len)])
                continue;
            existingVtable = _vtables[i];
        }

        if (existingVtable != 0)
        {
            // Found a match:
            // Remove the current vtable.
            _space = _buffer.length - vtableloc;
            // Point table to existing vtable.
            _buffer.put!int(_space, cast(int)(existingVtable - vtableloc));
        }
        else
        {
            // No match:
            // Add the location of the current vtable to the list of vtables.
            if (_numVtables == _vtables.length)
                _vtables.length *= 2;
            _vtables[_numVtables++] = offset();
            // Point table to current vtable.
            _buffer.put!int(_buffer.length - vtableloc, offset() - vtableloc);
        }

        destroy(_vtable);
        _vtable = null;
        return vtableloc;
    }

    /** This checks a required field has been set in a given table that has
        just been constructed.
    */
    void required(int table, int field)
    {
        import std.string;

        auto table_start = _buffer.length - table;
        auto vtable_start = table_start - _buffer.get!int(table_start);
        bool ok = _buffer.get!short(vtable_start + field) != 0;
        // If this fails, the caller will show what field needs to be set.
        if (!ok)
            throw new InvalidOperationException(format("FlatBuffers: field %s must be set.",
                field));
    }
    
    /**
        Finalize a buffer, pointing to the given `root_table`.
        Params:
            rootTable = An offset to be added to the buffer.
    */
    void finish(int rootTable)
    {
        prep(_minAlign, int.sizeof);
        addOffset(rootTable);
    }

    /**
        Get the ByteBuffer representing the FlatBuffer.
        Notes: his is typically only called after you call `Finish()`.
        Returns:
            Returns the ByteBuffer for this FlatBuffer.
    */
    ByteBuffer dataBuffer()
    {
        return _buffer;
    }

    /** 
        A utility function to copy and return the ByteBuffer data as a `ubyte[]`
        Retuens:
            the byte used in FlatBuffer data, it is not copy.
    */
    ubyte[] sizedByteArray()
    {
        return _buffer.data[_buffer.position .. $];
    }

    /**
        Finalize a buffer, pointing to the given `rootTable`.
        Params:
            rootTable = An offset to be added to the buffer.
            fileIdentifier = A FlatBuffer file identifier to be added to the buffer before `root_table`.
    */
    void finish(int rootTable, string fileIdentifier)
    {
        import std.string;

        prep(_minAlign, int.sizeof + fileIdentifierLength);
        if (fileIdentifier.length != fileIdentifierLength)
            throw new ArgumentException(
                format("FlatBuffers: file identifier must be length %s.", fileIdentifierLength),
                "fileIdentifier");
        for (int i = fileIdentifierLength - 1; i >= 0; i--)
            add!ubyte(cast(ubyte) fileIdentifier[i]);
        addOffset(rootTable);
    }

private:
    size_t _space;
    ByteBuffer _buffer;
    size_t _minAlign = 1;

    /// The vtable for the current table, null otherwise.
	size_t[] _vtable;
    /// Starting offset of the current struct/table.
    size_t _objectStart;
    /// List of offsets of all vtables.
	size_t[] _vtables = new int[](16);
    /// Number of entries in `vtables` in use.
	size_t _numVtables = 0;
    /// For the current vector being built.
	size_t _vectorNumElems = 0;
}
