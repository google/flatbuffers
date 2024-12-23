# Tutorial

This tutorial provides an example of how to work with FlatBuffers in a variety
of languages. The following topics will cover all the steps of using FlatBuffers
in your application.

1. Writing a FlatBuffers schema file (`.fbs`).
2. Using the `flatc` compiler to transform the schema into language-specific
   code.
3. Importing the generated code and libraries into your application.
4. Serializing data into a flatbuffer.
5. Deserializing a flatbuffer.

!!! note

    The tutorial is structured to be language agnostic, with language specifics
    in code blocks providing more context. Additionally, this tries to cover the
    major parts and type system of flatbuffers to give a general overview. It's
    not expected to be an exhaustive list of all features, or provide the best
    way to do things.

## FlatBuffers Schema (`.fbs`)

To start working with FlatBuffers, you first need to create a
[schema](schema.md) file which defines the format of the data structures you
wish to serialize. The schema is processed by the `flatc` compiler to generate
language-specific code that you use in your projects.

The following
[`monster.fbs`](https://github.com/google/flatbuffers/blob/master/samples/monster.fbs)
schema will be used for this tutorial. This is part of the FlatBuffers
[sample code](https://github.com/google/flatbuffers/tree/master/samples) to give
complete sample binaries demonstrations.

FlatBuffers schema is a Interface Definition Language (IDL) that has a couple
data structures, see the [schema](schema.md) documentation for a detail
description. Use the inline code annotations to get a brief synopsis of each
part of the schema.

```c title="monster.fbs" linenums="1"
// Example IDL file for our monster's schema.

namespace MyGame.Sample; //(1)!

enum Color:byte { Red = 0, Green, Blue = 2 } //(2)!

// Optionally add more tables.
union Equipment { Weapon } //(3)!

struct Vec3 { //(4)!
  x:float; //(5)!
  y:float;
  z:float;
}

table Monster { //(6)!
  pos:Vec3; //(7)!
  mana:short = 150; //(8)!
  hp:short = 100;
  name:string; //(9)!
  friendly:bool = false (deprecated); //(10)!
  inventory:[ubyte]; //(11)!
  color:Color = Blue;
  weapons:[Weapon]; //(12)!
  equipped:Equipment; //(13)!
  path:[Vec3];
}

table Weapon {
  name:string;
  damage:short;
}

root_type Monster; //(14)!
```

1. FlatBuffers has support for namespaces to place the generated code into.
   There is mixed level of support for namespaces (some languages don't have
   namespaces), but for the C family of languages, it is fully supported.

2. Enums definitions can be defined with the backing numerical type. Implicit
   numbering is supported, so that `Green` would have a value of 1.

3. A union represents a single value from a set of possible values. Its
   effectively an enum (to represent the type actually store) and a value,
   combined into one. In this example, the union is not very useful, since it
   only has a single type.

4. A struct is a collection of scalar fields with names. It is itself a scalar
   type, which uses less memory and has faster lookup. However, once a struct is
   defined, it cannot be changed. Use tables for data structures that can evolve
   over time.

5. FlatBuffers has the standard set of scalar numerical types (`int8`, `int16`,
   `int32`, `int64`, `uint8`, `uint16`, `uint32`, `uint64`, `float`, `double`),
   as well as `bool`. Note, scalars are fixed width, `varints` are not
   supported.

6. Tables are the main data structure for grouping data together. It can evolve
   by adding and deprecating fields over time, while preserving forward and
   backwards compatibility.

7. A field that happens to be a `struct`. This means the data of the `Vec3`
   struct will be serialized inline in the table without any need for offset.

8. Fields can be provided a default value. Default values can be configured to
   not be serialized at all while still providing the default value while
   deserializing. However, once set, a default value cannot be changed.

9. A `string` field which points to a serialized string external to the table.

10. A deprecated field that is no longer being used. This is used instead of
    removing the field outright.

11. A `vector` field that points to a vector of bytes. Like `strings`, the
    vector data is serialized elsewhere and this field just stores an offset to
    the vector.

12. Vector of `tables` and `structs` are also possible.

13. A field to a `union` type.

14. The root of the flatbuffer is always a `table`. This indicates the type of
    `table` the "entry" point of the flatbuffer will point to.

!!! bug "Get FlatBuffers schema syntax highlighting"

## Compiling Schema to Code (`flatc`)

After a schema file is written, you compile it to code in the languages you wish
to work with. This compilation is done by the [FlatBuffers Compiler](flatc.md)
(`flatc`) which is one of the binaries built in the repo.

### Building `flatc`

FlatBuffers uses [`cmake`](https://cmake.org/) to build projects files for your
environment.

=== "Unix"

    ```sh
    cmake -G "Unix Makefiles"
    make flatc
    ```

=== "Windows"

    ```sh
    cmake -G "Visual Studio 17 2022"
    msbuild.exe FlatBuffers.sln
    ```

See the documentation on [building](building.md) for more details and other
environments. Some languages also include a prebuilt `flatc` via their package
manager.

### Compiling Schema

To compile the schema, invoke `flatc` with the schema file and the language
flags you wish to generate code for. This compilation will generate files that
you include in your application code. These files provide convenient APIs for
serializing and deserializing the flatbuffer binary data.

=== "C++"

    ```sh
    flatc --cpp monster.fbs
    ```

=== "C#"

    ```sh
    flatc --csharp monster.fbs
    ```

!!! tip

    You can deserialize flatbuffers in languages that differ from the language
    that serialized it. For purpose of this tutorial, we assume one language
    is used for both serializing and deserializing.

## Application Integration

The generated files are then included in your project to be built into your
application. This is heavily dependent on your build system and language, but
generally involves two things:

1. Importing the generated code.
2. Importing the "runtime" libraries.

=== "C++"

    ```c++
    #include "monster_generated.h" // This was generated by `flatc`
    #include "flatbuffers.h" // The runtime library for C++

    // Simplifies naming in the following examples.
    using namespace MyGame::Sample; // Specified in the schema.
    ```

=== "C#"

    ```c#
    using Google.FlatBuffers; // The runtime library for C#
    using MyGame.Sample; // The generated files from `flatc`
    ```

For some languages the runtime libraries are just code files you compile into
your application. While other languages provide packaged libraries via their
package managers.

!!! note

    The generated files include APIs for both serializing and deserializing
    flatbuffers. So these steps are identical for both the consumer and
    producer.

## Serialization

Once all the files are included into your application, it's time to start
serializing some data!

With FlatBuffers, serialization can be a bit verbose, since each piece of data
must be serialized separately and in a particular order (depth-first, pre-order
traversal). The verbosity allows efficient serialization without heap
allocations, at the cost of more complex serialization APIs.

For example, any reference type (e.g. `table`, `vector`, `string`) must be
serialized before it can be referred to by other structures. So its typical to
serialize the data from leaf to root node, as will be shown below.

### FlatBufferBuilder

Most languages use a Builder object for managing the binary array that the data
is serialized into. It provides an API for serializing data, as well as keeps
track of some internal state. The generated code wraps methods on the Builder
object to provide an API tailored to the schema.

First instantiate a Builder (or reuse an existing one) and specify some memory
for it. The builder will automatically resize the backing buffer when necessary.

=== "C++"

    ```c++
    // Construct a Builder with 1024 byte backing array.
    flatbuffers::FlatBufferBuidler builder(1024);
    ```

=== "C#"

    ```c#
    // Construct a Builder with 1024 byte backing array.
    FlatBufferBuilder builder = new FlatBufferBuilder(1024);
    ```

Once a Builder is available, data can be serialized to it via the Builder APIs
and the generated code.

### Serializing Data

In this tutorial, we are building `Monsters` and `Weapons` for a computer game.
A `Weapon` is represented by a flatbuffer `table` with some fields. One field is
the `name` field, which is type `string`.

```c title="monster.fbs" linenums="28"
table Weapon {
  name:string;
  damage:short;
}
```

#### Strings

Since `string` is a reference type, we first need to serialize it before
assigning it to the `name` field of the `Weapon` table. This is done through the
Builder `CreateString` method:

=== "C++"

    ```c++
    flatbuffers::Offset<String> weapon_one_name = builder.CreateString("Sword");
    flatbuffers::Offset<String> weapon_two_name = builder.CreateString("Axe");
    ```

=== "C#"

    ```c#
    Offset<String> weaponOneName = builder.CreateString("Sword");
    Offset<String> weaponTwoName = builder.CreateString("Axe");
    ```

This performs the actual serialization (the string data is copied into the
backing array) and returns an offset. Think of the offset as a handle to that
reference. It's just a "typed" numerical offset to where that data resides in
the buffer.

#### Tables

Now that we have some names serialized, we can serialize `Weapons`. Here we will
use one of the generated helper functions that was emitted by `flatc`. The
`CreateWeapon` function takes in the Builder object, as well as the offset to
the weapon's name and a numerical value for the damage field.

=== "C++"

    ```c++
    short weapon_one_damage = 3;
    short weapon_two_damage = 5;

    // Use the `CreateWeapon()` shortcut to create Weapons with all the fields set.
    flatbuffers::Offset<Weapon> sword =
        CreateWeapon(builder, weapon_one_name, weapon_one_damage);
    flatbuffers::Offset<Weapon> axe =
        CreateWeapon(builder, weapon_two_name, weapon_two_damage);
    ```

=== "C#"

    ```c#
    short weaponOneDamage = 3;
    short weaponTwoDamage = 5;

    // Use the `CreateWeapon()` helper function to create the weapons, since we set every field.
    Offset<Weapon> sword =
        Weapon.CreateWeapon(builder, weaponOneName, weaponOneDamage);
    Offset<Weapon> axe =
        Weapon.CreateWeapon(builder, weaponTwoName, weaponTwoDamage);
    ```

!!! Tip

    The generated functions from `flatc`, like `CreateWeapon`, are just composed
    of various Builder API methods. So its not required to use the generated
    code, but it does make things much simpler and compact.

Just like the `CreateString` methods, the table serialization functions return
an offset to the location of the serialized `Weapon` table.

Now that we have some `Weapons` serialized, we can serialize a `Monster`.
Looking at the schema again, this table has a lot more fields of various types.
Some of these need to be serialized beforehand, for the same reason we
serialized the name string before the weapon table.

!!! note inline end

    There is no prescribed ordering of which table fields must be serialized
    first, you could serialize in any order you want. You can also not serialize
    a field to provide a `null` value, this is done by using an 0 offset value.

```c title="monster.fbs" linenums="15"
table Monster {
  pos:Vec3;
  mana:short = 150;
  hp:short = 100;
  name:string;
  friendly:bool = false (deprecated);
  inventory:[ubyte];
  color:Color = Blue;
  weapons:[Weapon];
  equipped:Equipment;
  path:[Vec3];
}
```

#### Vectors

The `weapons` field is a `vector` of `Weapon` tables. We already have two
`Weapons` serialized, so we just need to serialize a `vector` of those offsets.
The Builder provides multiple ways to create `vectors`.

=== "C++"

    ```c++
    // Create a std::vector of the offsets we had previous made.
    std::vector<flatbuffers::Offset<Weapon>> weapons_vector;
    weapons_vector.push_back(sword);
    weapons_vector.push_back(axe);

    // Then serialize that std::vector into the buffer and again get an Offset
    // to that vector. Use `auto` here since the full type is long, and it just
    // a "typed" number.
    auto weapons = builder.CreateVector(weapons_vector);
    ```

=== "C#"

    ```c#
    // Create an array of the two weapon offsets.
    var weaps = new Offset<Weapon>[2];
    weaps[0] = sword;
    weaps[1] = axe;

    // Pass the `weaps` array into the `CreateWeaponsVector()` method to create
    // a FlatBuffer vector.
    var weapons = Monster.CreateWeaponsVector(builder, weaps);
    ```

While we are at it, let us serialize the other two vector fields: the
`inventory` field is just a vector of scalars, and the `path` field is a vector
of structs (which are scalar data as well). So these vectors can be serialized a
bit more directly.

=== "C++"

    ```c++
    // Construct an array of two `Vec3` structs.
    Vec3 points[] = { Vec3(1.0f, 2.0f, 3.0f), Vec3(4.0f, 5.0f, 6.0f) };

    // Serialize it as a vector of structs.
    flatbuffers::Offset<flatbuffers::Vector<Vec3>> path =
        builder.CreateVectorOfStructs(points, 2);

    // Create a `vector` representing the inventory of the Orc. Each number
    // could correspond to an item that can be claimed after he is slain.
    unsigned char treasure[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    flatbuffers::Offset<flatbuffers::Vector<unsigned char>> inventory =
        builder.CreateVector(treasure, 10);

    ```

=== "C#"

    ```c#
    // Start building a path vector of length 2.
    Monster.StartPathVector(fbb, 2);

    // Serialize the individual Vec3 structs
    Vec3.CreateVec3(builder, 1.0f, 2.0f, 3.0f);
    Vec3.CreateVec3(builder, 4.0f, 5.0f, 6.0f);

    // End the vector to get the offset
    Offset<Vector<Vec3>> path = fbb.EndVector();

    // Create a `vector` representing the inventory of the Orc. Each number
    // could correspond to an item that can be claimed after he is slain.
    // Note: Since we prepend the bytes, this loop iterates in reverse order.
    Monster.StartInventoryVector(builder, 10);
    for (int i = 9; i >= 0; i--)
    {
        builder.AddByte((byte)i);
    }
    Offset<Vector<byte>> inventory = builder.EndVector();
    ```

#### Unions

The last non-scalar data for the `Monster` table is the `equipped` `union`
field. For this case, we will reuse an already serialized `Weapon` (the only
type in the union), without needing to reserialize it. Union fields implicitly
add a hidden `_type` field that stores the type of value stored in the union.
When serializing a union, you must explicitly set this type field, along with
providing the union value.

We will also serialize the other scalar data at the same time, since we have all
the necessary values and Offsets to make a `Monster`.

=== "C++"

    ```c++
    // Create the remaining data needed for the Monster.
    auto name = builder.CreateString("Orc");

    // Create the position struct
    auto position = Vec3(1.0f, 2.0f, 3.0f);

    // Set his hit points to 300 and his mana to 150.
    int hp = 300;
    int mana = 150;

    // Finally, create the monster using the `CreateMonster` helper function
    // to set all fields.
    //
    // Here we set the union field by using the `.Union()` method of the
    // `Offset<Weapon>` axe we already serialized above. We just have to specify
    // which type of object we put in the union, and do that with the
    // auto-generated `Equipment_Weapon` enum.
    flatbuffers::Offset<Monster> orc =
        CreateMonster(builder, &position, mana, hp, name, inventory,
                      Color_Red, weapons, Equipment_Weapon, axe.Union(),
                      path);

    ```

=== "C#"

    ```c#
    // Create the remaining data needed for the Monster.
    var name = builder.CreateString("Orc");

    // Create our monster using `StartMonster()` and `EndMonster()`.
    Monster.StartMonster(builder);
    Monster.AddPos(builder, Vec3.CreateVec3(builder, 1.0f, 2.0f, 3.0f));
    Monster.AddHp(builder, (short)300);
    Monster.AddName(builder, name);
    Monster.AddInventory(builder, inv);
    Monster.AddColor(builder, Color.Red);
    Monster.AddWeapons(builder, weapons);
    // For union fields, we explicitly add the auto-generated enum for the type
    // of value stored in the union.
    Monster.AddEquippedType(builder, Equipment.Weapon);
    // And we just use the `.Value` property of the already serialized axe.
    Monster.AddEquipped(builder, axe.Value); // Axe
    Monster.AddPath(builder, path);
    Offset<Monster> orc = Monster.EndMonster(builder);
    ```

!!! warning

    When serializing tables, you must fully serialize it before attempting to
    serialize another reference type. If you try to serialize in a nested
    manner, you will get an assert/exception/panic depending on your language.

### Finishing

At this point, we have serialized a `Monster` we've named "orc" to the
flatbuffer and have its offset. The `root_type` of the schema is also a
`Monster`, so we have everything we need to finish the serialization step.

This is done by calling the appropriate `finish` method on the Builder, passing
in the orc offset to indicate this `table` is the "entry" point when
deserializing the buffer later.

=== "C++"

    ```c++
    // Call `Finish()` to instruct the builder that this monster is complete.
    // You could also call `FinishMonsterBuffer(builder, orc);`
    builder.Finish(orc);
    ```

=== "C#"

    ```c#
    // Call `Finish()` to instruct the builder that this monster is complete.
    // You could also call `Monster.FinishMonsterBuffer(builder, orc);`
    builder.Finish(orc.Value);
    ```

Once you finish a Builder, you can no longer serialize more data to it.

#### Buffer Access

The flatbuffer is now ready to be stored somewhere, sent over the network,
compressed, or whatever you would like to do with it. You access the raw buffer
like so:

=== "C++"

    ```c++
    // This must be called after `Finish()`.
    uint8_t *buf = builder.GetBufferPointer();

    // Returns the size of the buffer that `GetBufferPointer()` points to.
    int size = builder.GetSize();
    ```

=== "C#"

    ```c#
    // This must be called after `Finish()`.
    //
    // The data in this ByteBuffer does NOT start at 0, but at buf.Position.
    // The end of the data is marked by buf.Length, so the size is
    // buf.Length - buf.Position.
    FlatBuffers.ByteBuffer dataBuffer = builder.DataBuffer;

    // Alternatively this copies the above data out of the ByteBuffer for you:
    byte[] buf = builder.SizedByteArray();
    ```

Now you can write the bytes to a file or send them over the network. The buffer
stays valid until the Builder is cleared or destroyed.

!!! warning "BINARY Mode"

    Make sure your file mode (or transfer protocol) is set to BINARY, and not
    TEXT. If you try to transfer a flatbuffer in TEXT mode, the buffer will be
    corrupted and be hard to diagnose.

## Deserialization

!!! note "Misnomer"

    Deserialization is a bit of a misnomer, since FlatBuffers doesn't
    deserialize the whole buffer when accessed. It just "decodes" the data that
    is requested, leaving all the other data untouched. It is up to the
    application to decide if the data is copied out or even read in the first
    place. However, we continue to use the word `deserialize` to mean accessing
    data from a binary flatbuffer.

Now that we have successfully create an orc FlatBuffer, the data can be saved,
sent over a network, etc. At some point, the buffer will be accessed to obtain
the underlying data.

The same application setup used for serialization is needed for deserialization
(see [application integration](#application-integration)).

### Root Access

All access to the data in the flatbuffer must first go through the root object.
There is only one root object per flatbuffer. The generated code provides
functions to get the root object given the buffer.

=== "C++"

    ```c++
    uint8_t *buffer_pointer = /* the data you just read */;

    // Get an view to the root object inside the buffer.
    Monster monster = GetMonster(buffer_pointer);
    ```

=== "C#"

    ```c#
    byte[] bytes = /* the data you just read */

    // Get an view to the root object inside the buffer.
    Monster monster = Monster.GetRootAsMonster(new ByteBuffer(bytes));
    ```

!!! warning "BINARY mode"

    Again, make sure you read the bytes in BINARY mode, otherwise the buffer may
    be corrupted.

In most languages, the returned object is just a "view" of the data with helpful
accessors. Data is typically not copied out of the backing buffer. This also
means the backing buffer must remain alive for the duration of the views.

### Table Access

If you look in the generated files emitted by `flatc`, you will see it generated
, for each `table`, accessors of all its non-`deprecated` fields. For example,
some of the accessors of the `Monster` root table would look like:

=== "C++"

    ```c++
    auto hp = monster->hp();
    auto mana = monster->mana();
    auto name = monster->name()->c_str();
    ```

=== "C#"

    ```c#
    // For C#, unlike most other languages support by FlatBuffers, most values
    // (except for vectors and unions) are available as properties instead of
    // accessor methods.
    var hp = monster.Hp;
    var mana = monster.Mana;
    var name = monster.Name;
    ```

These accessors should hold the values `300`, `150`, and `"Orc"` respectively.

!!! note "Default Values"

    The default value of `150` wasn't stored in the `mana` field, but we are
    still able to retrieve it. That is because the generated accessors return a
    hard-coded default value when it doesn't find the value in the buffer.

#### Nested Object Access

Accessing nested objects is very similar, with the nested field pointing to
another object type. Be careful, the field could be `null` if not present.

For example, accessing the `pos` `struct`, which is type `Vec3` you would do:

=== "C++"

    ```c++
    auto pos = monster->pos();
    auto x = pos->x();
    auto y = pos->y();
    auto z = pos->z();
    ```

=== "C#"

    ```c#
    var pos = monster.Pos.Value;
    var x = pos.X;
    var y = pos.Y;
    var z = pos.Z;
    ```

Where `x`, `y`, and `z` will contain `1.0`, `2.0`, and `3.0` respectively.

### Vector Access

Similarly, we can access elements of the `inventory` `vector` by indexing it.
You can also iterate over the length of the vector.

=== "C++"

    ```c++
    flatbuffers::Vector<unsigned char> inv = monster->inventory();
    auto inv_len = inv->size();
    auto third_item = inv->Get(2);
    ```

=== "C#"

    ```c#
    int invLength = monster.InventoryLength;
    var thirdItem = monster.Inventory(2);
    ```

For vectors of tables, you can access the elements like any other vector, except
you need to handle the result as a FlatBuffer table. Here we iterate over the
`weapons` vector that is houses `Weapon` `tables`.

=== "C++"

    ```c++
    flatbuffers::Vector<Weapon> weapons = monster->weapons();
    auto weapon_len = weapons->size();
    auto second_weapon_name = weapons->Get(1)->name()->str();
    auto second_weapon_damage = weapons->Get(1)->damage()
    ```

=== "C#"

    ```c#
    int weaponsLength = monster.WeaponsLength;
    var secondWeaponName = monster.Weapons(1).Name;
    var secondWeaponDamage = monster.Weapons(1).Damage;
    ```

### Union Access

Lastly , we can access our `equipped` `union` field. Just like when we created
the union, we need to get both parts of the union: the type and the data.

We can access the type to dynamically cast the data as needed (since the union
only stores a FlatBuffer `table`).

=== "C++"

    ```c++
    auto union_type = monster.equipped_type();
 
    if (union_type == Equipment_Weapon) {
         // Requires `static_cast` to type `const Weapon*`.
        auto weapon = static_cast<const Weapon*>(monster->equipped());
 
        auto weapon_name = weapon->name()->str(); // "Axe"
        auto weapon_damage = weapon->damage();    // 5
    }
    ```

=== "C#"

    ```c#
    var unionType = monster.EquippedType;
 
    if (unionType == Equipment.Weapon) {
        var weapon = monster.Equipped<Weapon>().Value;
 
        var weaponName = weapon.Name;     // "Axe"
        var weaponDamage = weapon.Damage; // 5
    }
    ```
