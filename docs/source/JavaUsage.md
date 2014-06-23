# Use in Java

There's experimental support for reading FlatBuffers in Java. Generate code
for Java with the `-j` option to `flatc`.

See `javaTest.java` for an example. Essentially, you read a FlatBuffer binary
file into a `byte[]`, which you then turn into a `ByteBuffer`, which you pass to
the `getRootAsMonster` function:

    ByteBuffer bb = ByteBuffer.wrap(data);
    Monster monster = Monster.getRootAsMonster(bb);

Now you can access values much like C++:

    short hp = monster.hp();
    Vec3 pos = monster.pos();

Note that whenever you access a new object like in the `pos` example above,
a new temporary accessor object gets created. If your code is very performance
sensitive (you iterate through a lot of objects), there's a second `pos()`
method to which you can pass a `Vec3` object you've already created. This allows
you to reuse it across many calls and reduce the amount of object allocation (and
thus garbage collection) your program does.

Java does not support unsigned scalars. This means that any unsigned types you
use in your schema will actually be represented as a signed value. This means
all bits are still present, but may represent a negative value when used.
For example, to read a `byte b` as an unsigned number, you can do:
`(short)(b & 0xFF)`

Sadly the string accessors currently always create a new string when accessed,
since FlatBuffer's UTF-8 strings can't be read in-place by Java.

Vector access is also a bit different from C++: you pass an extra index
to the vector field accessor. Then a second method with the same name
suffixed by `_length` let's you know the number of elements you can access:

    for (int i = 0; i < monster.inventory_length(); i++)
        monster.inventory(i); // do something here

You can also construct these buffers in Java using the static methods found
in the generated code, and the FlatBufferBuilder class:

    FlatBufferBuilder fbb = new FlatBufferBuilder();

Create strings:

    int str = fbb.createString("MyMonster");

Create a table with a struct contained therein:

    Monster.startMonster(fbb);
    Monster.addPos(fbb, Vec3.createVec3(fbb, 1.0f, 2.0f, 3.0f, 3.0, (byte)4, (short)5, (byte)6));
    Monster.addHp(fbb, (short)80);
    Monster.addName(fbb, str);
    Monster.addInventory(fbb, inv);
    Monster.addTest_type(fbb, (byte)1);
    Monster.addTest(fbb, mon2);
    Monster.addTest4(fbb, test4s);
    int mon = Monster.endMonster(fbb);

As you can see, the Java code for tables does not use a convenient
`createMonster` call like the C++ code. This is to create the buffer without
using temporary object allocation (since the `Vec3` is an inline component of
`Monster`, it has to be created right where it is added, whereas the name and
the inventory are not inline).
Structs do have convenient methods that even have arguments for nested structs.

Vectors also use this start/end pattern to allow vectors of both scalar types
and structs:

    Monster.startInventoryVector(fbb, 5);
    for (byte i = 4; i >=0; i--) fbb.addByte(i);
    int inv = fbb.endVector();

You can use the generated method `startInventoryVector` to conveniently call
`startVector` with the right element size. You pass the number of
elements you want to write. You write the elements backwards since the buffer
is being constructed back to front.

There are `add` functions for all the scalar types. You use `addOffset` for
any previously constructed objects (such as other tables, strings, vectors).
For structs, you use the appropriate `create` function in-line, as shown
above in the `Monster` example.

## Text Parsing

There currently is no support for parsing text (Schema's and JSON) directly
from Java, though you could use the C++ parser through JNI. Please see the
C++ documentation for more on text parsing.
