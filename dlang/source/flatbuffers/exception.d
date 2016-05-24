module flatbuffers.exception;

import std.exception;

///The number of bytes in a file identifier.
enum fileIdentifierLength = 4;

///ArgumentException
class ArgumentException : Exception
{
    this(string msg, string argument) pure nothrow @safe
    {
        super(msg);
    }
}

///ArgumentOutOfRangeException
class ArgumentOutOfRangeException : Exception
{
    this(string argument, long value, string msg) pure nothrow @safe
    {
        import std.conv : to;

        super(argument ~ ' ' ~ msg ~ " (instead: " ~ value.to!string ~ ")");
    }
}

///InvalidOperationException
class InvalidOperationException : Exception
{
    this(string msg) pure nothrow @safe
    {
        super(msg);
    }
}
