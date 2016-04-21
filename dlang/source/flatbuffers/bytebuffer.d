module flatbuffers.bytebuffer;

import std.exception;
import std.bitmanip;
import std.exception;
import core.exception;

final class ByteBuffer
{
private:
	bool _bigEndian = false; //faltbuffer in（x86） is Little Endian
public:
	@property bigEndian(){return _bigEndian;}
	@property bigEndian(bool bigendian){_bigEndian = bigendian;}
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


	void put(T)(int offset, T value) if(is(T == byte) || is(T == ubyte))
	{
		mixin(verifyOffset("1"));
		_buffer[offset] = value; 
		_pos = offset;
	}

	void put(T)(int offset, T value) if(is(T == short) || is(T == ushort) || is(T == int) || is(T == uint) || is(T == long) 
		|| is(T == ulong) || is(T == float) || is(T == double)) 
	{
		mixin(verifyOffset("T.sizeof"));
		if(_bigEndian) {
			auto array = nativeToBigEndian!T(value);
			_buffer[offset..(offset + T.sizeof)] = array[];
		} else {
			auto array = nativeToLittleEndian!T(value);
			_buffer[offset..(offset + T.sizeof)] = array[];
		}
		_pos = offset;
	}


	T get(T)(int index) if( is(T == byte) || is(T == ubyte) )
	{
		return cast(T)_buffer[index];
	}

	T get(T)(int index) if (is(T == short) || is(T == ushort) || is(T == int) || is(T == uint) || is(T == long) 
		|| is(T == ulong) || is(T == float) || is(T == double)) 
	{
		ubyte[T.sizeof] buf = _buffer[index..(index + T.sizeof)];
		if(_bigEndian)
			return bigEndianToNative!(T,T.sizeof)(buf);
		else
			return  littleEndianToNative!(T,T.sizeof)(buf);
	}

private: //Variables.
	ubyte[] _buffer;
	int _pos;  //Must track start of the buffer.
}

private:
string verifyOffset(string length)	
{
	return "if(offset < 0 || offset >= _buffer.length || (offset +" ~ length ~ ") > _buffer.length)   throw new RangeError(); ";
}
