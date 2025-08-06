# Overview

FlatBuffers is an efficient cross platform serialization library for C++, C#, C,
Go, Java, Kotlin, JavaScript, Lobster, Lua, TypeScript, PHP, Python, Rust and
Swift. It was originally created at Google for game development and other
performance-critical applications.

It is available as Open Source on
[GitHub](https://github.com/google/flatbuffers) under the Apache license v2.0.

## Why Use FlatBuffers?

<div class="grid cards" markdown>

- :material-clock-fast:{ .lg .middle } **Access to serialized data without
  parsing/unpacking**

    ---
    Access the data directly without unpacking or parsing.

- :material-memory:{ .lg .middle } **Memory Efficiency and Speed**

    ---
    The only memory needed to access your data is that of the buffer.
  No heap is required.

- :material-compare-horizontal:{ .lg .middle } **Backwards and Forwards
  Compatibility**

    ---
    FlatBuffers enables the schema to evolve over time while still maintaining
  forwards and backwards compatibility with old flatbuffers.

- :material-scale-off:{ .lg .middle } **Small Footprint**

    ---
    Minimal dependencies and small code footprint.

</div>

## Why not use...

=== "Protocol Buffers"

    Protocol Buffers is indeed relatively similar to FlatBuffers, with the primary
    difference being that FlatBuffers does not need a parsing/unpacking step to a
    secondary representation before you can access data, often coupled with
    per-object memory allocation. The code is an order of magnitude bigger, too.

=== "JSON"

    JSON is very readable (which is why we use it as our optional text format) and
    very convenient when used together with dynamically typed languages (such as
    JavaScript). When serializing data from statically typed languages, however,
    JSON not only has the obvious drawback of runtime inefficiency, but also forces
    you to write more code to access data (counterintuitively) due to its
    dynamic-typing serialization system. In this context, it is only a better choice
    for systems that have very little to no information ahead of time about what
    data needs to be stored.
