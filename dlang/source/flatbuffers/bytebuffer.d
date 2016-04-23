module flatbuffers.bytebuffer;

import std.exception;
import std.bitmanip;
import std.exception;
import core.exception;

final class ByteBuffer
{
public:
	this(ubyte[] buffer)
	{
		_buffer = buffer;
		_pos = 0;
	}

	@property uint length()
	{
		return cast(uint)_buffer.length;
	}

	@property ubyte[] data()
	{
		return _buffer;
	}

	@property int position()
	{
		return _pos;
	}

	void put(T)(int offset, T value) if(is(T == bool))
	{
		put!ubyte(offset,(value ? 0x01 : 0x00));
	}


	void put(T)(int offset, T value) if(isByte!T)
	{
		mixin(verifyOffset("1"));
		_buffer[offset] = value;
		_pos = offset;
	}

	void put(T)(int offset, T value) if(isNum!T)
	{
		mixin(verifyOffset("T.sizeof"));
		version (FLATBUFFER_BIGENDIAN) {
			auto array = nativeToBigEndian!T(value);
			_buffer[offset..(offset + T.sizeof)] = array[];
		} else {
			auto array = nativeToLittleEndian!T(value);
			_buffer[offset..(offset + T.sizeof)] = array[];
		}
		_pos = offset;
	}


	T get(T)(int index) if(isByte!T)
	{
		return cast(T)_buffer[index];
	}

	T get(T)(int index) if(isNum!T)
	{
		ubyte[T.sizeof] buf = _buffer[index..(index + T.sizeof)];
		version (FLATBUFFER_BIGENDIAN)
			return bigEndianToNative!(T,T.sizeof)(buf);
		else
			return littleEndianToNative!(T,T.sizeof)(buf);
	}

private: //Variables.
	ubyte[] _buffer;
	int _pos; //Must track start of the buffer.
}

unittest {
	ByteBuffer buf = new ByteBuffer(new ubyte[50]);
	int a = 10;
	buf.put(5,a);
	short t = 4;
	buf.put(9,t);

	assert(buf.get!int(5) == 10);
	assert(buf.get!short(9) == 4);

}

private:
string verifyOffset(string length)
{
	return "if(offset < 0 || offset >= _buffer.length || (offset +" ~ length ~ ") > _buffer.length) throw new RangeError(); ";
}

template isNum(T)
{
	static if (is(T == short) || is(T == ushort) || is(T == int) || is(T == uint) || is(T == long)
		|| is(T == ulong) || is(T == float) || is(T == double))
		enum isNum = true;
	else
		enum isNum = false;
}

template isByte(T)
{
	static if (is(T == byte) || is(T == ubyte))
		enum isByte = true;
	else
		enum isByte = false;
}
