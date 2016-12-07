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
 
module flatbuffers.table;

import flatbuffers.exception;
import flatbuffers.bytebuffer;
public import std.typecons;

/// Mixin this template to all structs in the generated code derive , and add their own accessors.
mixin template Struct(ParentType)
{
    /**
        Create this Struct.
    */
    static ParentType init_(size_t pos, ByteBuffer buffer)
    {
        return ParentType(buffer, pos);
    }

private: // Variables.
    /**
        disable the constor.
    */
    @disable this();
	this(ByteBuffer buffer, size_t pos)
    {
        this._buffer = buffer;
        this._pos = pos;
    }

    ByteBuffer _buffer;
	size_t _pos;
}

/// Mixin this template to all  tables in the generated code derive , and add their own accessors.
mixin template Table(ParentType)
{
    /**
        Create this Struct as a Table.
    */
	static ParentType init_(size_t pos, ByteBuffer buffer)
    {
        return ParentType(buffer, pos);
    }

private:
    ByteBuffer _buffer;
	size_t _pos;

private: // Methods.
    @disable this();
	this(ByteBuffer buffer, size_t pos)
    {
        this._buffer = buffer;
        this._pos = pos;
    }

    /// Look up a field in the vtable, return an offset into the object, or 0 if the field is not present.
	uint __offset(size_t vtableOffset)
    {
        auto vtable = _pos - _buffer.get!uint(_pos);
        return vtableOffset < _buffer.get!short(vtable) ? cast(
            uint) _buffer.get!short(vtable + vtableOffset) : 0;
    }

    /// Retrieve the relative offset stored at "offset".
	uint __indirect(size_t offset)
    {
		return cast(uint)(offset + _buffer.get!uint(offset));
    }

    /// Create a D string from UTF-8 data stored inside the flatbuffer.
	string __string(size_t offset)
    {
        offset += _buffer.get!uint(offset);
        auto len = _buffer.get!uint(offset);
        auto startPos = offset + uint.sizeof;
        return cast(string) _buffer.data[startPos .. startPos + len];
    }

    /// Get the length of a vector whose offset is stored at "offset" in this object.
	uint __vector_len(size_t offset)
    {
        offset += _pos;
        offset += _buffer.get!uint(offset);
        return _buffer.get!uint(offset);
    }

    /// Get the start of data of a vector whose offset is stored at "offset" in this object.
	uint __dvector(size_t offset)
    {
        offset += _pos;
		return cast(uint)(offset + _buffer.get!uint(offset) + uint.sizeof); // Data starts after the length.
    }

    /// Initialize any Table-derived type to point to the union at the given offset.
	T __union(T)(size_t offset)
    {
        offset += _pos;
        return T.init_((offset + _buffer.get!uint(offset)), _buffer);
    }

    static bool __has_identifier(ByteBuffer bb, string ident)
    {
        import std.string;

        if (ident.length != fileIdentifierLength)
            throw new ArgumentException(
                format("FlatBuffers: file identifier must be length %s.", fileIdentifierLength),
                "ident");

        for (auto i = 0; i < fileIdentifierLength; i++)
        {
            if (ident[i] != cast(char) bb.get!byte(bb.position() + cast(uint)uint.sizeof + i))
                return false;
        }

        return true;
    }
}

import std.traits;

/**
    Iterator for the vector.
*/
struct Iterator(ParentType, ReturnType, string accessor)
{
    static if (isScalarType!(ReturnType) || isSomeString!(ReturnType))
        alias ApplyType = ReturnType;
    else
        alias ApplyType = Nullable!ReturnType;
private:
        ParentType parent;
    int index;
public:
    this(ParentType parent)
    {
        this.parent = parent;
    }

    @property int length()
    {
        mixin("return parent." ~ accessor ~ "Length;");
    }

    @property bool empty()
    {
        return index == length;
    }

    @property ReturnType popFront()
    {
        mixin("return parent." ~ accessor ~ "(++index);");
    }

    @property ReturnType front()
    {
        mixin("return parent." ~ accessor ~ "(index);");
    }

    ReturnType opIndex(int index)
    {
        mixin("return parent." ~ accessor ~ "(index);");
    }

    int opApply(int delegate(ApplyType) operations)
    {
        int result = 0;
        for (int number = 0; number < length(); ++number)
        {
            static if (isScalarType!(ReturnType) || isSomeString!(ReturnType))
                result = operations(opIndex(number));
            else
                mixin("result = operations(parent." ~ accessor ~ "(number));");
            if (result)
                break;
        }
        return result;
    }

    int opApply(int delegate(int, ApplyType) operations)
    {
        int result = 0;
        for (int number = 0; number < length(); ++number)
        {
            static if (isScalarType!(ReturnType) || isSomeString!(ReturnType))
                result = operations(number, opIndex(number));
            else
                mixin("result = operations(number, parent." ~ accessor ~ "(number));");
            if (result)
                break;
        }
        return result;
    }
}
