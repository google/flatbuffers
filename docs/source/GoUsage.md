# Use in Go

There's experimental support for reading FlatBuffers in Go. Generate code
for Go with the `-g` option to `flatc`.

See `go_test.go` for an example. You import the generated code, read a
FlatBuffer binary file into a `[]byte`, which you pass to the
`GetRootAsMonster` function:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.go}
    import (
       example "MyGame/Example"
       flatbuffers "github.com/google/flatbuffers/go"

       io/ioutil
    )

    buf, err := ioutil.ReadFile("monster.dat")
    // handle err
    monster := example.GetRootAsMonster(buf, 0)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now you can access values like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.go}
    hp := monster.Hp()
    pos := monster.Pos(nil)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note that whenever you access a new object like in the `Pos` example above,
a new temporary accessor object gets created. If your code is very performance
sensitive (you iterate through a lot of objects), you can replace nil with a
pointer to a `Vec3` object you've already created. This allows
you to reuse it across many calls and reduce the amount of object allocation
(and thus garbage collection) your program does.

To access vectors you pass an extra index to the
vector field accessor. Then a second method with the same name suffixed
by `Length` let's you know the number of elements you can access:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.go}
    for i := 0; i < monster.InventoryLength(); i++ {
        monster.Inventory(i) // do something here
    }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can also construct these buffers in Go using the functions found in the
generated code, and the FlatBufferBuilder class:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.go}
    builder := flatbuffers.NewBuilder(0)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create strings:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.go}
    str := builder.CreateString("MyMonster")
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a table with a struct contained therein:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.go}
    example.MonsterStart(builder)
    example.MonsterAddPos(builder, example.CreateVec3(builder, 1.0, 2.0, 3.0, 3.0, 4, 5, 6))
    example.MonsterAddHp(builder, 80)
    example.MonsterAddName(builder, str)
    example.MonsterAddInventory(builder, inv)
    example.MonsterAddTest_Type(builder, 1)
    example.MonsterAddTest(builder, mon2)
    example.MonsterAddTest4(builder, test4s)
    mon := example.MonsterEnd(builder)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Unlike C++, Go does not support table creation functions like 'createMonster()'.
This is to create the buffer without
using temporary object allocation (since the `Vec3` is an inline component of
`Monster`, it has to be created right where it is added, whereas the name and
the inventory are not inline).
Structs do have convenient methods that allow you to construct them in one call.
These also have arguments for nested structs, e.g. if a struct has a field `a`
and a nested struct field `b` (which has fields `c` and `d`), then the arguments
will be `a`, `c` and `d`.

Vectors also use this start/end pattern to allow vectors of both scalar types
and structs:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.go}
    example.MonsterStartInventoryVector(builder, 5)
    for i := 4; i >= 0; i-- {
        builder.PrependByte(byte(i))
    }
    inv := builder.EndVector(5)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The generated method 'StartInventoryVector' is provided as a convenience
function which calls 'StartVector' with the correct element size of the vector
type which in this case is 'ubyte' or 1 byte per vector element.
You pass the number of elements you want to write.
You write the elements backwards since the buffer
is being constructed back to front.

There are `Prepend` functions for all the scalar types. You use
`PrependUOffset` for any previously constructed objects (such as other tables,
strings, vectors). For structs, you use the appropriate `create` function
in-line, as shown above in the `Monster` example.

## Text Parsing

There currently is no support for parsing text (Schema's and JSON) directly
from Go, though you could use the C++ parser through cgo. Please see the
C++ documentation for more on text parsing.
