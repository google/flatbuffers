# Use in Python

There's experimental support for reading FlatBuffers in Python. Generate
code for Python with the `-p` option to `flatc`.

See `py_test.py` for an example. You import the generated code, read a
FlatBuffer binary file into a `bytearray`, which you pass to the
`get_root_as_Monster` function:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
    from MyGame.Example import Monster, Vec3
    import flatbuffers

    buf = open('monster.dat', 'rb').read()
    buf = bytearray(buf)
    monster = Monster.get_root_as_Monster(buf, 0)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now you can access values like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
    hp = monster.hp()
    pos = monster.pos()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To access vectors you pass an extra index to the
vector field accessor. Then a second method with the same name suffixed
by `_length` let's you know the number of elements you can access:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
    for i in xrange(monster.inventory_length()):
        monster.inventory(i) # do something here
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can also construct these buffers in Python using the functions found
in the generated code, and the Builder class:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
    builder = flatbuffers.Builder(0)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create strings:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
    s = builder.create_string("MyMonster")
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a table with a struct contained therein:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
    Monster.start(builder)
    Monster.add_pos(builder, Vec3.create_Vec3(builder, 1.0, 2.0, 3.0, 3.0, 4, 5, 6))
    Monster.add_hp(builder, 80)
    Monster.add_name(builder, str)
    Monster.add_inventory(builder, inv)
    Monster.add_test_Type(builder, 1)
    Monster.add_Test(builder, mon2)
    Monster.add_Test4(builder, test4s)
    mon = Monster.end(builder)

    final_flatbuffer = builder.output()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Unlike C++, Python does not support table creation functions like 'createMonster()'.
This is to create the buffer without
using temporary object allocation (since the `Vec3` is an inline component of
`Monster`, it has to be created right where it is added, whereas the name and
the inventory are not inline, and **must** be created outside of the table
creation sequence).
Structs do have convenient methods that allow you to construct them in one call.
These also have arguments for nested structs, e.g. if a struct has a field `a`
and a nested struct field `b` (which has fields `c` and `d`), then the arguments
will be `a`, `c` and `d`.

Vectors also use this start/end pattern to allow vectors of both scalar types
and structs:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
    Monster.start_inventory_vector(builder, 5)
    i = 4
    while i >= 0:
        builder.prepend_Byte(byte(i))
        i -= 1

    inv = builder.end_vector(5)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The generated method 'start_inventory_vector' is provided as a convenience
function which calls 'start_vector' with the correct element size of the vector
type which in this case is 'ubyte' or 1 byte per vector element.
You pass the number of elements you want to write.
You write the elements backwards since the buffer
is being constructed back to front. Use the correct `prepend` call for the type,
or `prepend_UOffsetT` for offsets. You then pass `inv` to the corresponding
`add` call when you construct the table containing it afterwards.

There are `Builder.prepend` functions for all the scalar types. You use
`prepend_UOffset` for any previously constructed objects (such as other tables,
strings, vectors). For structs, you use the appropriate `create` function
in-line, as shown above in the `Monster` example.

Once you're done constructing a buffer, you call `Builder.finish()` with the root object
offset (`mon` in the example above). Your data now resides in Builder.Bytes.
Important to note is that the real data starts at the index indicated by `Builder.head`,
for offset() bytes (this is because the buffer is constructed backwards).
If you wanted to read the buffer right after creating it (using
`get_root_as_monster` above), the second argument, instead of `0` would thus
also be `Builder.head`.

## Text Parsing

There currently is no support for parsing text (Schema's and JSON) directly
from Python, though you could use the C++ parser through SWIG or ctypes. Please
see the C++ documentation for more on text parsing.

