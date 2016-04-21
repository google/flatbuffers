module flatbuffers.table; 

import flatbuffers.exception;
import flatbuffers.bytebuffer;
public import std.typecons;

mixin template Struct(ParentType)
{
	static ParentType init_(int pos, ByteBuffer buffer)
	{
		return ParentType(buffer,pos);
	}

private: //Variables.
	@disable this();
	this(ByteBuffer buffer,int pos) {
		this._buffer = buffer;
		this._pos = pos;
	}
	ByteBuffer _buffer;
	int _pos;
}

mixin template Table(ParentType)
{
	static ParentType init_(int pos, ByteBuffer buffer)
	{
		return ParentType(buffer,pos);
	}

private: 
	ByteBuffer _buffer;
	int _pos;
	
private: //Methods.
	@disable this();
	this(ByteBuffer buffer,int pos) {
		this._buffer = buffer;
		this._pos = pos;
	}


	///Look up a field in the vtable, return an offset into the object, or 0 if the field is not present.
	int __offset(int vtableOffset)
	{
		int vtable = _pos - _buffer.get!int(_pos);
		return vtableOffset < _buffer.get!short(vtable)? cast(int)_buffer.get!short(vtable + vtableOffset) : 0;
	}
	
	///Retrieve the relative offset stored at "offset".
	int __indirect(int offset)
	{
		return offset + _buffer.get!int(offset);
	}
	
	///Create a D string from UTF-8 data stored inside the flatbuffer.
	string __string(int offset)
	{
		offset += _buffer.get!int(offset);
		auto len = _buffer.get!int(offset);
		auto startPos = offset + int.sizeof;
		return cast(string)_buffer.data[startPos..startPos+len];
	}
	
	///Get the length of a vector whose offset is stored at "offset" in this object.
	int __vector_len(int offset)
	{
		offset += _pos;
		offset += _buffer.get!int(offset);
		return _buffer.get!int(offset);
	}
	
	///Get the start of data of a vector whose offset is stored at "offset" in this object.
	int __dvector(int offset)
	{
		offset += _pos;
		return offset + _buffer.get!int(offset) + cast(int)int.sizeof; //Data starts after the length.
	}
	
	///Initialize any Table-derived type to point to the union at the given offset.
	T __union(T)(int offset)
	{
		offset += _pos;
		return T.init_((offset + _buffer.get!int(offset)), _buffer);
	}
	
	static bool __has_identifier(ByteBuffer bb, string ident)
	{
		import std.string;
		if(ident.length != fileIdentifierLength)
			throw new ArgumentException(format("FlatBuffers: file identifier must be length %s.", fileIdentifierLength), "ident");
		
		for(int i=0; i<fileIdentifierLength; i++)
		{
			if(ident[i] != cast(char)bb.get!byte(bb.position() + cast(int)int.sizeof + i))
				return false;
		}
		
		return true;
	}
}

import std.traits;

struct Iterator(ParentType, ReturnType, string accessor)
{
	static if(isScalarType!(ReturnType) || isSomeString!(ReturnType))
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
		mixin("return parent."~accessor~"Length;");
	}
	
	@property bool empty()
	{
		return index == length;
	}
	
	@property ReturnType popFront()
	{
		mixin("return parent."~accessor~"(++index);");
	}
	
	@property ReturnType front()
	{
		mixin("return parent."~accessor~"(index);");
	}
	
	ReturnType opIndex(int index)
	{
		mixin("return parent."~accessor~"(index);");
	}

	int opApply(int delegate(ApplyType) operations)
	{
		int result = 0;
		for(int number=0; number<length(); ++number)
		{
			static if(isScalarType!(ReturnType) || isSomeString!(ReturnType))
				result = operations(opIndex(number));
			else
				mixin("result = operations(parent."~accessor~"(number));");
			if(result)
				break;
		}
		return result;
	}
	
	int opApply(int delegate(int, ApplyType) operations)
	{
		int result = 0;
		for(int number=0; number<length(); ++number)
		{
			static if(isScalarType!(ReturnType) || isSomeString!(ReturnType))
				result = operations(number, opIndex(number));
			else
				mixin("result = operations(number,  parent."~accessor~"(number));");
			if(result)
				break;
		}
		return result;
	}
}
