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
