# Use in Python

There's experimental support for reading FlatBuffers in Python. Generate
code for Python with the `-p` option to `flatc`.

See `py_test.py` for an example. You import the generated code, read a
FlatBuffer binary file into a `bytearray`, which you pass to the
`GetRootAsMonster` function:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
    import MyGame.Example as example
    import flatbuffers

    buf = open('monster.dat', 'rb').read()
    buf = bytearray(buf)
    monster = example.GetRootAsMonster(buf, 0)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now you can access values like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
    hp = monster.Hp()
    pos = monster.Pos()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To access vectors you pass an extra index to the
vector field accessor. Then a second method with the same name suffixed
by `Length` let's you know the number of elements you can access:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
    for i in xrange(monster.InventoryLength()):
        monster.Inventory(i) # do something here
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can also construct these buffers in Python using the functions found
in the generated code, and the FlatBufferBuilder class:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
    builder = flatbuffers.NewBuilder(0)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create strings:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
    s = builder.CreateString("MyMonster")
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a table with a struct contained therein:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
    example.MonsterStart(builder)
    example.MonsterAddPos(builder, example.CreateVec3(builder, 1.0, 2.0, 3.0, 3.0, 4, 5, 6))
    example.MonsterAddHp(builder, 80)
    example.MonsterAddName(builder, str)
    example.MonsterAddInventory(builder, inv)
    example.MonsterAddTest_Type(builder, 1)
    example.MonsterAddTest(builder, mon2)
    example.MonsterAddTest4(builder, test4s)
    mon = example.MonsterEnd(builder)

    final_flatbuffer = bulder.Output()
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
    example.MonsterStartInventoryVector(builder, 5)
    i = 4
    while i >= 0:
        builder.PrependByte(byte(i))
        i -= 1

    inv = builder.EndVector(5)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The generated method 'StartInventoryVector' is provided as a convenience
function which calls 'StartVector' with the correct element size of the vector
type which in this case is 'ubyte' or 1 byte per vector element.
You pass the number of elements you want to write.
You write the elements backwards since the buffer
is being constructed back to front. Use the correct `Prepend` call for the type,
or `PrependUOffsetT` for offsets. You then pass `inv` to the corresponding
`Add` call when you construct the table containing it afterwards.

There are `Prepend` functions for all the scalar types. You use
`PrependUOffset` for any previously constructed objects (such as other tables,
strings, vectors). For structs, you use the appropriate `create` function
in-line, as shown above in the `Monster` example.

Once you're done constructing a buffer, you call `Finish` with the root object
offset (`mon` in the example above). Your data now resides in Builder.Bytes.
Important to note is that the real data starts at the index indicated by Head(),
for Offset() bytes (this is because the buffer is constructed backwards).
If you wanted to read the buffer right after creating it (using
`GetRootAsMonster` above), the second argument, instead of `0` would thus
also be `Head()`.

## Text Parsing

There currently is no support for parsing text (Schema's and JSON) directly
from Python, though you could use the C++ parser through SWIG or ctypes. Please
see the C++ documentation for more on text parsing.

