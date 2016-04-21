module flatbuffers.flatbufferbuilder;

import flatbuffers.exception;
import flatbuffers.bytebuffer;

import std.exception;

final class FlatBufferBuilder
{
public: 
	this(int initsize)
	{
		if(initsize == 0)
			throw new ArgumentOutOfRangeException("initsize", initsize, "Must be greater than zero");
		_space = initsize;
		_buffer = new ByteBuffer(new ubyte[initsize]);
	}
	
	int offset() { return _buffer.length - _space; }
	
	void pad(int size)
	{
		for(int i=0; i<size; i++) {
			-- _space;
			_buffer.put!ubyte(_space, 0x00);
		}
	}
	
	///Doubles the size of the ByteBuffer, and copies the old data towards
	///the end of the new buffer (since we build the buffer backwards).
	void growBuffer()
	{
		auto oldBuf = _buffer.data;
		auto oldBufSize = oldBuf.length;
		if((oldBufSize & 0xC0000000) != 0)
			throw new Exception("FlatBuffers: cannot grow buffer beyond 2 gigabytes.");
		
		auto newBufSize = oldBufSize * 2;
		auto newBuf = new ubyte[](newBufSize);
		newBuf[(newBufSize-oldBufSize)..$] = oldBuf[];
		
		_buffer = new ByteBuffer(newBuf);
	}
	
	///Prepare to write an element of `size` after `additional_bytes`
	///have been written, e.g. if you write a string, you need to align
	///such the int length field is aligned to SIZEOF_INT, and the string
	///data follows it directly.
	///If all you need to do is align, `additional_bytes` will be 0.
	void prep(int size, int additionalBytes)
	{
		//Track the biggest thing we've ever aligned to.
		if(size > _minAlign)
			_minAlign = size;
		
		//Find the amount of alignment needed such that `size` is properly
		//aligned after `additional_bytes`.
		auto alignSize = ((~(cast(int)_buffer.length - _space + additionalBytes)) + 1) & (size - 1);
		
		//Reallocate the buffer if needed.
		while(_space < alignSize + size + additionalBytes)
		{
			auto oldBufSize = cast(int)_buffer.length;
			growBuffer();
			_space += cast(int)_buffer.length - oldBufSize;
		}
		if(alignSize > 0)
			pad(alignSize);
	}

	void put(T)(T x) if(is(T == bool) || is(T == byte) || is(T == ubyte) ||
		is(T == short) || is(T == ushort) || is(T == int) || is(T == uint) || is(T == long) 
		|| is(T == ulong) || is(T == float) || is(T == double)) 
	{
		static if (is(T == bool)) {
			_space -= 1;
		} else {
			_space -= T.sizeof;
		}
		_buffer.put!T(_space, x);
	}

	///Adds a scalar to the buffer, properly aligned, and the buffer grown if needed.
	void addBool(bool x) { prep(byte.sizeof, 0); put!bool(x); }
	void addByte(byte x) { prep(byte.sizeof, 0); put!byte(x); }
	void addUbyte(ubyte x) { prep(ubyte.sizeof, 0); put!ubyte(x); }
	void addShort(short x) { prep(short.sizeof, 0); put!short(x); }
	void addUshort(ushort x) { prep(ushort.sizeof, 0); put!ushort(x); }
	void addInt(int x) { prep(int.sizeof, 0); put!int(x); }
	void addUint(uint x) { prep(uint.sizeof, 0); put!uint(x); }
	void addLong(long x) { prep(long.sizeof, 0); put!long(x); }
	void addUlong(ulong x) { prep(ulong.sizeof, 0); put!ulong(x); }
	void addFloat(float x) { prep(float.sizeof, 0); put!float(x); }
	void addDouble(double x) { prep(double.sizeof, 0); put!double(x); }
	
	///Adds on offset, relative to where it will be written.
	void addOffset(int off)
	{
		prep(int.sizeof, 0); //Ensure alignment is already done.
		if(off > offset())
			throw new ArgumentException("FlatBuffers: must be less than offset.", "off");
		
		off = offset() - off + cast(int)int.sizeof;
		put!int(off);
	}

	void startVector(int elemSize, int count, int alignment)
	{
		notNested();
		_vectorNumElems = count;
		prep(int.sizeof, elemSize * count);
		prep(alignment, elemSize * count); //Just in case alignment > int.
	}
	
	int endVector()
	{
		put!int(_vectorNumElems);
		return offset();
	}
	
	void nested(int obj)
	{
		//Structs are always stored inline, so need to be created right
		//where they are used. You'll get this assert if you created it
		//elsewhere.
		if(obj != offset())
			throw new Exception("FlatBuffers: struct must be serialized inline.");
	}
	
	void notNested()
	{
		//You should not be creating any other objects or strings/vectors
		//while an object is being constructed.
		if(_vtable)
			throw new Exception("FlatBuffers: object serialization must not be nested.");
	}
	
	void startObject(int numfields)
	{
		notNested();
		_vtable = new int[](numfields);
		_objectStart = offset();
	}
	
	///Set the current vtable at `voffset` to the current location in the buffer.
	void slot(int voffset)
	{
		_vtable[voffset] = offset();
	}
	
	///Add a scalar to a table at `o` into its vtable, with value `x` and default `d`.
	void addBool(int o, bool x, bool d) { if(x != d) { addBool(x); slot(o); } }
	void addByte(int o, byte x, byte d) { if(x != d) { addByte(x); slot(o); } }
	void addUbyte(int o, ubyte x, ubyte d) { if(x != d) { addUbyte(x); slot(o); } }
	void addShort(int o, short x, int d) { if(x != d) { addShort(x); slot(o); } }
	void addUshort(int o, ushort x, ushort d) { if(x != d) { addUshort(x); slot(o); } }
	void addInt(int o, int x, int d) { if(x != d) { addInt(x); slot(o); } }
	void addUint(int o, uint x, uint d) { if(x != d) { addUint(x); slot(o); } }
	void addLong(int o, long x, long d) { if(x != d) { addLong(x); slot(o); } }
	void addUlong(int o, ulong x, ulong d) { if(x != d) { addUlong(x); slot(o); } }
	void addFloat(int o, float x, double d) { if(x != d) { addFloat(x); slot(o); } }
	void addDouble(int o, double x, double d) { if(x != d) { addDouble(x); slot(o); } }
	void addOffset(int o, int x, int d) { if(x != d) { addOffset(x); slot(o); } }
	
	int createString(string s)
	{
		notNested();
		auto utf8 = cast(ubyte[])s;
		addUbyte(cast(ubyte)0);
		startVector(1, cast(int)utf8.length, 1);
		_space -= utf8.length;
		_buffer.data[_space.._space + utf8.length] = utf8[];
		return endVector();
	}
	
	///Structs are stored inline, so nothing additional is being added.
	///`d` is always 0.
	void addStruct(int voffset, int x, int d)
	{
		if(x != d)
		{
			nested(x);
			slot(voffset);
		}
	}
	
	int endObject()
	{
		if(!_vtable)
			throw new InvalidOperationException("Flatbuffers: calling endObject without a startObject");
		
		addInt(cast(int)0);
		auto vtableloc = offset();
		
		//Write out the current vtable.
		for(int i=cast(int)_vtable.length-1; i>=0; i--)
		{
			//Offset relative to the start of the table.
			short off = cast(short)(_vtable[i] != 0? vtableloc - _vtable[i] : 0);
			addShort(off);
		}
		
		const int standardFields = 2; //The fields below:
		addShort(cast(short)(vtableloc - _objectStart));
		addShort(cast(short)((_vtable.length + standardFields) * short.sizeof));
		
		///Search for an existing vtable that matches the current one.
		int existingVtable = 0;

		ubyte[] data = _buffer.data();

	     for(int i=0; i<_numVtables; i++)
		{
			int vt1 = _buffer.length - _vtables[i];
			int vt2 = _space;
			short vt1len = _buffer.get!short(vt1);
			short vt2len = _buffer.get!short(vt2);

			if(vt1len != vt2len || data[vt1..(vt1 + vt1len)] != data[vt2..(vt2 + vt2len)]) continue;
			existingVtable = _vtables[i];
		}
		
		if(existingVtable != 0)
		{
			//Found a match:
			//Remove the current vtable.
			_space = _buffer.length - vtableloc;
			//Point table to existing vtable.
			_buffer.put!int(_space, existingVtable - vtableloc);
		}
		else
		{
			//No match:
			//Add the location of the current vtable to the list of vtables.
			if(_numVtables == _vtables.length)
				_vtables.length *= 2;
			_vtables[_numVtables++] = offset();
			//Point table to current vtable.
			_buffer.put!int(_buffer.length - vtableloc, offset() - vtableloc);
		}
		
		destroy(_vtable);
		_vtable = null;
		return vtableloc;
	}
	
	///This checks a required field has been set in a given table that has
	///just been constructed.
	void required(int table, int field)
	{
		import std.string;
		int table_start = _buffer.length - table;
		int vtable_start = table_start - _buffer.get!int(table_start);
		bool ok = _buffer.get!short(vtable_start + field) != 0;
		//If this fails, the caller will show what field needs to be set.
		if(!ok)
			throw new InvalidOperationException(format("FlatBuffers: field %s must be set.", field));
	}
	
	void finish(int rootTable)
	{
		prep(_minAlign, int.sizeof);
		addOffset(rootTable);
	}
	
	ByteBuffer dataBuffer() { return _buffer; }
	
	///Utility function for copying a byte array that starts at 0.
	ubyte[] sizedByteArray()
	{
		return _buffer.data[_buffer.position..$];
	}
	
	void finish(int rootTable, string fileIdentifier)
	{
		import std.string;
		prep(_minAlign, int.sizeof + fileIdentifierLength);
		if(fileIdentifier.length != fileIdentifierLength)
			throw new ArgumentException(format("FlatBuffers: file identifier must be length %s.", fileIdentifierLength), "fileIdentifier");
		for(int i=fileIdentifierLength-1; i>=0; i--)
			addByte(cast(ubyte)fileIdentifier[i]);
		addOffset(rootTable);
	}
	
private: 
	int _space;
	ByteBuffer _buffer;
	int _minAlign = 1;
	
	///The vtable for the current table, null otherwise.
	int[] _vtable;
	///Starting offset of the current struct/table.
	int _objectStart;
	///List of offsets of all vtables.
	int[] _vtables = new int[](16);
	///Number of entries in `vtables` in use.
	int _numVtables = 0;
	///For the current vector being built.
	int _vectorNumElems = 0;
}

