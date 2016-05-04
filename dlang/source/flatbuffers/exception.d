module flatbuffers.exception;

enum fileIdentifierLength = 4;

class ArgumentException : Error
{
    this(string msg, string argument) pure nothrow @safe
    {
        super(msg);
    }
}

class ArgumentOutOfRangeException : Error
{
    this(string argument, long value, string msg) pure nothrow @safe
    {
        import std.conv : to;

        super(argument ~ ' ' ~ msg ~ " (instead: " ~ value.to!string ~ ")");
    }
}

class InvalidOperationException : Error
{
    this(string msg) pure nothrow @safe
    {
        super(msg);
    }
}
