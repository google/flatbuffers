# Use in Java/C-sharp

FlatBuffers supports reading and writing binary FlatBuffers in Java and C#.
Generate code for Java with the `-j` option to `flatc`, or for C# with `-n`
(think .Net).

Note that this document is from the perspective of Java. Code for both languages
is generated in the same way, with only minor differences. These differences
are [explained in a section below](#differences-in-c-sharp).

See `javaTest.java` for an example. Essentially, you read a FlatBuffer binary
file into a `byte[]`, which you then turn into a `ByteBuffer`, which you pass to
the `getRootAsMyRootType` function:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.java}
    ByteBuffer bb = ByteBuffer.wrap(data);
    Monster monster = Monster.getRootAsMonster(bb);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now you can access values much like C++:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.java}
    short hp = monster.hp();
    Vec3 pos = monster.pos();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note that whenever you access a new object like in the `pos` example above,
a new temporary accessor object gets created. If your code is very performance
sensitive (you iterate through a lot of objects), there's a second `pos()`
method to which you can pass a `Vec3` object you've already created. This allows
you to reuse it across many calls and reduce the amount of object allocation
(and thus garbage collection) your program does.

Java does not support unsigned scalars. This means that any unsigned types you
use in your schema will actually be represented as a signed value. This means
all bits are still present, but may represent a negative value when used.
For example, to read a `byte b` as an unsigned number, you can do:
`(short)(b & 0xFF)`

The default string accessor (e.g. `monster.name()`) currently always create
a new Java `String` when accessed, since FlatBuffer's UTF-8 strings can't be
used in-place by `String`. Alternatively, use `monster.nameAsByteBuffer()`
which returns a `ByteBuffer` referring to the UTF-8 data in the original
`ByteBuffer`, which is much more efficient. The `ByteBuffer`'s `position`
points to the first character, and its `limit` to just after the last.

Vector access is also a bit different from C++: you pass an extra index
to the vector field accessor. Then a second method with the same name
suffixed by `Length` let's you know the number of elements you can access:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.java}
    for (int i = 0; i < monster.inventoryLength(); i++)
        monster.inventory(i); // do something here
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Alternatively, much like strings, you can use `monster.inventoryAsByteBuffer()`
to get a `ByteBuffer` referring to the whole vector. Use `ByteBuffer` methods
like `asFloatBuffer` to get specific views if needed.

If you specified a file_indentifier in the schema, you can query if the
buffer is of the desired type before accessing it using:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.java}
    if (Monster.MonsterBufferHasIdentifier(bb)) ...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


## Buffer construction in Java

You can also construct these buffers in Java using the static methods found
in the generated code, and the FlatBufferBuilder class:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.java}
    FlatBufferBuilder fbb = new FlatBufferBuilder();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create strings:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.java}
    int str = fbb.createString("MyMonster");
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a table with a struct contained therein:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.java}
    Monster.startMonster(fbb);
    Monster.addPos(fbb, Vec3.createVec3(fbb, 1.0f, 2.0f, 3.0f, 3.0, (byte)4, (short)5, (byte)6));
    Monster.addHp(fbb, (short)80);
    Monster.addName(fbb, str);
    Monster.addInventory(fbb, inv);
    Monster.addTest_type(fbb, (byte)1);
    Monster.addTest(fbb, mon2);
    Monster.addTest4(fbb, test4s);
    int mon = Monster.endMonster(fbb);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For some simpler types, you can use a convenient `create` function call that
allows you to construct tables in one function call. This example definition
however contains an inline struct field, so we have to create the table
manually.
This is to create the buffer without using temporary object allocation.

It's important to understand that fields that are structs are inline (like
`Vec3` above), and MUST thus be created between the start and end calls of
a table. Everything else (other tables, strings, vectors) MUST be created
before the start of the table they are referenced in.

Structs do have convenient methods that even have arguments for nested structs.

As you can see, references to other objects (e.g. the string above) are simple
ints, and thus do not have the type-safety of the Offset type in C++. Extra
case must thus be taken that you set the right offset on the right field.

Vectors can be created from the corresponding Java array like so:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.java}
    int inv = Monster.createInventoryVector(fbb, new byte[] { 0, 1, 2, 3, 4 });
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This works for arrays of scalars and (int) offsets to strings/tables,
but not structs. If you want to write structs, or what you want to write
does not sit in an array, you can also use the start/end pattern:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.java}
    Monster.startInventoryVector(fbb, 5);
    for (byte i = 4; i >=0; i--) fbb.addByte(i);
    int inv = fbb.endVector();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can use the generated method `startInventoryVector` to conveniently call
`startVector` with the right element size. You pass the number of
elements you want to write. Note how you write the elements backwards since
the buffer is being constructed back to front. You then pass `inv` to the
corresponding `Add` call when you construct the table containing it afterwards.

There are `add` functions for all the scalar types. You use `addOffset` for
any previously constructed objects (such as other tables, strings, vectors).
For structs, you use the appropriate `create` function in-line, as shown
above in the `Monster` example.

To finish the buffer, call:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.java}
    Monster.finishMonsterBuffer(fbb, mon);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The buffer is now ready to be transmitted. It is contained in the `ByteBuffer`
which you can obtain from `fbb.dataBuffer()`. Importantly, the valid data does
not start from offset 0 in this buffer, but from `fbb.dataBuffer().position()`
(this is because the data was built backwards in memory).
It ends at `fbb.capacity()`.


## Differences in C-sharp

C# code works almost identically to Java, with only a few minor differences.
You can see an example of C# code in `tests/FlatBuffers.Test/FlatBuffersExampleTests.cs`.

First of all, naming follows standard C# style with `PascalCasing` identifiers,
e.g. `GetRootAsMyRootType`. Also, values (except vectors and unions) are available
as properties instead of parameterless accessor methods as in Java. The
performance-enhancing methods to which you can pass an already created object
are prefixed with `Get`, e.g.:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cs}
    // property
    var pos = monster.Pos;
    // method filling a preconstructed object
    var preconstructedPos = new Vec3();
    monster.GetPos(preconstructedPos);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


## Text parsing

There currently is no support for parsing text (Schema's and JSON) directly
from Java, though you could use the C++ parser through JNI. Please see the
C++ documentation for more on text parsing.
