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
		_bigEndian = false; //faltbuffer 在（x86下）默认小端存储
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

	@property bigEndian(){return _bigEndian;}
	@property bigEndian(bool bigendian){_bigEndian = bigendian;}

	void put(T)(int offset, T value) if(is(T == bool) || is(T == byte) || is(T == ubyte) ||
		is(T == short) || is(T == ushort) || is(T == int) || is(T == uint) || is(T == long) 
		|| is(T == ulong) || is(T == float) || is(T == double)) 
	{
		static if(is(T == bool)) {
			if(offset < 0 || offset >= _buffer.length || (offset + 1) > _buffer.length)
				throw new RangeError();
			_buffer[offset] = (value ? 0x01 : 0x00); 
			_pos = offset;
		} else static if(is(T == byte) || is(T == ubyte) ) {
			if(offset < 0 || offset >= _buffer.length || (offset + 1 ) > _buffer.length)
				throw new RangeError();
			_buffer[offset] = value; 
			_pos = offset;
		} else {
			if(offset < 0 || offset >= _buffer.length || offset + T.sizeof  > _buffer.length)
				throw new RangeError();
			if(_bigEndian) {
				auto array = nativeToBigEndian!T(value);
				_buffer[offset..(offset + T.sizeof)] = array[];
			} else {
				auto array = nativeToLittleEndian!T(value);
				_buffer[offset..(offset + T.sizeof)] = array[];
			}
			_pos = offset;
		} 
	}


	T get(T)(int index) if(is(T == bool) || is(T == byte) || is(T == ubyte) ||
		is(T == short) || is(T == ushort) || is(T == int) || is(T == uint) || is(T == long) 
		|| is(T == ulong) || is(T == float) || is(T == double)) 
	{
		static if(is(T == byte) || is(T == ubyte) ) {
			return cast(T)_buffer[index];
		} else {
			ubyte[T.sizeof] buf = _buffer[index..(index + T.sizeof)];
			if(_bigEndian)
				return bigEndianToNative!(T,T.sizeof)(buf);
			else
				return  littleEndianToNative!(T,T.sizeof)(buf);
		}
	}

private: //Variables.
	ubyte[] _buffer;
	int _pos;  //Must track start of the buffer.
	bool _bigEndian;
}

