# Use in Kotlin

FlatBuffers supports reading and writing binary FlatBuffers in Kotlin.
Generate code for Kotlin with the `-k` option to `flatc`.

See `KotlinTest.kt` for an example. Essentially, you read a FlatBuffer binary
file into a `ByteBuffer`, which you either pass to the `wrap` method 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.kotlin}
    val monster = Monster()
    monster.wrap(byteBuffer)  // reusing monster, no allocation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

or which you use to construct a new accessor

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.kotlin}
    val monster = Monster(byteBuffer) // allocates a Monster accessor
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Then you can access values :

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.kotlin}
    val hp = monster.hp   // a short
    val pos = monster.pos // a Vec3? struct
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note that whenever you access a new object like in the `pos` example above,
a new temporary accessor object gets created. If your code is very performance
sensitive (you iterate through a lot of objects), you can pass a `Vec3` object 
you've already created. This allows you to reuse it across many calls and reduce 
the amount of object allocation (and thus garbage collection) your program does.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.kotlin}
    val reuse = Vec3() // allocate once, reuse a lot
    val pos = monster.pos(reuse) 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Kotlin does not support unsigned scalars. This means that any unsigned types you
use in your schema will actually be represented as a signed value (an Int or a Long). 
This means all bits are still present, but may represent a negative value when used. For example, to read a Byte `b` as an unsigned number, you can do:
`b.toInt().and(0xFF)`

The default string accessor (e.g. `monster.name`) currently creates
a new `String` when accessed, which means that the utf8 bytes get converted 
and copied in a CharArray inside the newly created String. Alternatively, 
use `monster.nameAsByteBuffer` which returns a `ByteBuffer` referring to the UTF-8 data in the original
`ByteBuffer`, which is much more efficient. The `ByteBuffer`'s `position`
points to the first character, and its `limit` to just after the last.

Vector access is also a bit different from C++: you pass an extra index
to the vector field accessor. Then a second method with the same name
suffixed by `Size` let's you know the number of elements you can access:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.kotlin}
    for (i in 0 until monster.inventorySize)
        monster.inventory(i); // do something here
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Alternatively, much like strings, you can use `monster.inventoryAsByteBuffer`
to get a `ByteBuffer` referring to the whole vector. Use `ByteBuffer` methods
like `asFloatBuffer` to get specific views if needed.

If you specified a file_indentifier in the schema, you can query if the
buffer is of the desired type before accessing it using:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.kotlin}
    if (Monster.hasIdentifier(byteBuffer)) ...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


## Buffer construction in Kotlin

You can also construct these buffers in Kotlin using the static methods found
in the generated code, and the FlatBufferBuilder class:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.kotlin}
    val fbb = FlatBufferBuilder()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create strings:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.kotlin}
    val stringOffset = fbb.of("MyMonster")
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The generated methods to construct an object are extension functions 
of it's Companion object (to avoid namespace pollution) and they need 
a FlatBufferBuilder receiver (fluent api). 
It is convenient ( and necessary as of Kotlin beta 1.0) 
to use the scoping method `with` to set their receivers. 


Create a table with a struct contained therein:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.kotlin}
        with (fbb) {
            with(Monster) { 
                val mon = monsterOf(
                        nameOf = of("MyMonster"), 
                	testbool = false,
                	hp = 80.toShort(),
                	inventoryOf = inventoryOf(0, 1, 2, 3, 4),
                	testType = Example.Any.Monster,
                	testOf = monsterOf(of("Fred")), 
			posDef = vec3Def(1.0f, 2.0f, 3.0f, 3.0, Color.Green, testDef(5.toShort(), 6.toByte()))
                 )
            }
        }

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For most types, you can use a convenient `typeOf` function that allows 
you to construct tables in one function call and that returns an offset
to the constructed type. 

Structs have a `structOf` function as well that is usefull when creating 
arrays of structs and a deffered `structDef` function that allows inlining 
nested structs inside tables 

Structs do have convenient methods that even have arguments for nested structs.

As of now, references to other objects (e.g. the string above) are simple Ints, 
and thus do not have the type-safety of the Offset type in C++. 
Extra care must thus be taken that you set the right offset on the right field.
(we could use generics to add type safety but that would need an allocation 
for each offset)

Arrays can be created with an `arrayOf` method that uses vararg like so:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.kotlin}
    val inv = with(Monster) { fbb.inventoryOf(0, 1, 2, 3, 4) }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This works for arrays of scalars and (Int) offsets to strings/tables,
but not structs. If you want to write structs, or what you want to write
does not sit in an array, you can also use a lambda 
to write data from back to front:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.kotlin}
    val inv = with(Monster) { fbb.inventoryOf(5) {
        	for (i in 4 downTo 0) addByte(i.toByte())
        }
    }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can use the generated method `inventoryOf` with the right element size. 
You pass the number of elements you want to write. 
Note how you write the elements backwards since
the buffer is being constructed back to front. You then pass `inv` to the
corresponding `inentory` call when you construct the table containing it afterwards.


For structs you can use the `structOf` method inside the lambda

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.kotlin}
    val test4 = with(Monster) { test4Of(2) {
          testOf(10.toShort(), 20.toByte())
          testOf(30.toShort(), 40.toByte())
        }
    }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To finish the buffer, call:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.kotlin}
    with(Monster) { fbb.finishBuffer(mon)}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The buffer is now ready to be transmitted. It is contained in the `ByteBuffer`
which you can obtain from `fbb.dataBuffer()`. Importantly, the valid data does
not start from offset 0 in this buffer, but from `fbb.dataBuffer().position()`
(this is because the data was built backwards in memory).
It ends at `fbb.capacity()`.

## Text parsing

There currently is no support for parsing text (Schema's and JSON) directly
from Kotlin, though you could use the C++ parser through native call
interfaces available to each language. Please see the
C++ documentation for more on text parsing.

### Mutating FlatBuffers

As you saw above, typically once you have created a FlatBuffer, it is
read-only from that moment on. There are however cases where you have just
received a FlatBuffer, and you'd like to modify something about it before
sending it on to another recipient. With the above functionality, you'd have
to generate an entirely new FlatBuffer, while tracking what you modify in your
own data structures. This is inconvenient.

For this reason FlatBuffers can also be mutated in-place. While this is great
for making small fixes to an existing buffer, you generally want to create
buffers from scratch whenever possible, since it is much more efficient and
the API is much more general purpose.

To get non-const accessors, invoke `flatc` with `--gen-mutable`.

You now can:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.kotlin}
    val monster = Monster(bb)
    monster.mutateHp(10) // Set table field // hp is a val.
    monster.pos?.z = 4   // Set struct field // z is a var.
    monster.mutateInventory(0, 1)  // Set vector element.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We use the somewhat verbose term `mutate` instead of `set` to indicate that
this is a special use case, not to be confused with the default way of
constructing FlatBuffer data.

After the above mutations, you can send on the FlatBuffer to a new recipient
without any further work!

Note that any `mutate` functions on tables return a boolean, which is false
if the field we're trying to set isn't present in the buffer. Fields are not
present if they weren't set, or even if they happen to be equal to the
default value. For example, in the creation code above we set the `mana` field
to `150`, which is the default value, so it was never stored in the buffer.
Trying to call mutateMana() on such data will return false, and the value won't
actually be modified!

One way to solve this is to call `forceDefaults()` on a
`FlatBufferBuilder` to force all fields you set to actually be written. This
of course increases the size of the buffer somewhat, but this may be
acceptable for a mutable buffer.git add

