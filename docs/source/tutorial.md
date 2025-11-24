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

The tutorial is structured to be language agnostic, with language specifics in
code blocks providing more context. Additionally, this tries to cover the major
parts and type system of flatbuffers to give a general overview. It's not
expected to be an exhaustive list of all features, or provide the best way to do
things.

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

```proto title="monster.fbs" linenums="1"
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

=== "C"

    !!! Note

        If you're working in C, you need to use the separate project
        [FlatCC](https://github.com/dvidelabs/flatcc) which contains a schema
        compiler and runtime library in C for C. See
        [flatcc build instructions](https://github.com/dvidelabs/flatcc#building).

        Please be aware of the difference between `flatc` and `flatcc` tools.

    ```sh
    cd flatcc
    mkdir -p build/tmp/samples/monster
    bin/flatcc -a -o build/tmp/samples/monster samples/monster/monster.fbs
    # or just
    flatcc/samples/monster/build.sh
    ```

=== "C#"

    ```sh
    flatc --csharp monster.fbs
    ```

=== "Dart"

    ```sh
    flatc --dart monster.fbs
    ```

=== "Go"

    ```sh
    flatc --go monster.fbs
    ```

=== "Java"

    ```sh
    flatc --java monster.fbs
    ```

=== "JavaScript"

    ```sh
    flatc --js monster.fbs
    ```

=== "Kotlin"

    ```sh
    flatc --kotlin monster.fbs
    ```

=== "Lobster"

    ```sh
    flatc --lobster monster.fbs
    ```

=== "Lua"

    ```sh
    flatc --lua monster.fbs
    ```

=== "PHP"

    ```sh
    flatc --php monster.fbs
    ```

=== "Python"

    ```sh
    flatc --python monster.fbs
    ```

=== "Rust"

    ```sh
    flatc --rust monster.fbs
    ```

=== "Swift"

    ```sh
    flatc --swift monster.fbs
    ```

=== "TypeScript"

    ```sh
    flatc --ts monster.fbs
    ```

You can deserialize flatbuffers in languages that differ from the language that
serialized it. For purpose of this tutorial, we assume one language is used for
both serializing and deserializing.

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

=== "C"

    ```c
    #include "monster_builder.h" // Generated by `flatcc`.

    // Convenient namespace macro to manage long namespace prefix.
    #undef ns
    // Specified in the schema.
    #define ns(x) FLATBUFFERS_WRAP_NAMESPACE(MyGame_Sample, x)

    // A helper to simplify creating vectors from C-arrays.
    #define c_vec_len(V) (sizeof(V)/sizeof((V)[0]))
    ```

=== "C#"

    ```c#
    using Google.FlatBuffers; // The runtime library for C#
    using MyGame.Sample; // The generated files from `flatc`
    ```

=== "Dart"

    ```dart
    import 'package:flat_buffers/flat_buffers.dart' as fb;

    // Generated by `flatc`.
    import 'monster_my_game.sample_generated.dart' as myGame;
    ```

=== "Go"

    ```go
    import (
          flatbuffers "github.com/google/flatbuffers/go"
          sample "MyGame/Sample"
    )
    ```

=== "Java"

    ```java
    import MyGame.Sample.*; //The `flatc` generated files. (Monster, Vec3, etc.)

    import com.google.flatbuffers.FlatBufferBuilder;
    ```

=== "JavaScript"

    ```javascript
    // The following code is an example - use your desired module flavor by
    // transpiling from TS.
    var flatbuffers = require('/js/flatbuffers').flatbuffers;
    var MyGame = require('./monster_generated').MyGame; // Generated by `flatc`.

    //------------------------------------------------------------------------//

    // The following code is for browser-based HTML/JavaScript. Use the above
    // code for JavaScript module loaders (e.g. Node.js).
    <script src="../js/flatbuffers.js"></script>
    <script src="monster_generated.js"></script> // Generated by `flatc`.
    ```

=== "Kotlin"

    ```kotlin
    import MyGame.Sample.* //The `flatc` generated files. (Monster, Vec3, etc.)

    import com.google.flatbuffers.FlatBufferBuilder
    ```

=== "Lobster"

    ```lobster
    import from "../lobster/"  // Where to find flatbuffers.lobster
    import monster_generated
    ```

=== "Lua"

    ```lua
    -- require the flatbuffers module
    local flatbuffers = require("flatbuffers")

    -- require the generated files from `flatc`.
    local color = require("MyGame.Sample.Color")
    local equipment = require("MyGame.Sample.Equipment")
    local monster = require("MyGame.Sample.Monster")
    local vec3 = require("MyGame.Sample.Vec3")
    local weapon = require("MyGame.Sample.Weapon")
    ```

=== "PHP"

    ```php
    // It is recommended that your use PSR autoload when using FlatBuffers in
    // PHP. Here is an example from `SampleBinary.php`:
    function __autoload($class_name) {
        // The last segment of the class name matches the file name.
        $class = substr($class_name, strrpos($class_name, "\\") + 1);
        // `flatbuffers` root.
        $root_dir = join(DIRECTORY_SEPARATOR, array(dirname(dirname(__FILE__))));

        // Contains the `*.php` files for the FlatBuffers library and the `flatc`
        // generated files.
        $paths = array(join(DIRECTORY_SEPARATOR, array($root_dir, "php")),
          join(DIRECTORY_SEPARATOR,
            array($root_dir, "samples", "MyGame", "Sample")));
        foreach ($paths as $path) {
        $file = join(DIRECTORY_SEPARATOR, array($path, $class . ".php"));
        if (file_exists($file)) {
            require($file);
            break;
        }
      }
    }
    ```

=== "Python"

    ```py
    import flatbuffers

    # Generated by `flatc`.
    import MyGame.Sample.Color
    import MyGame.Sample.Equipment
    import MyGame.Sample.Monster
    import MyGame.Sample.Vec3
    import MyGame.Sample.Weapon
    ```

=== "Rust"

    ```rust
    // import the flatbuffers runtime library
    extern crate flatbuffers;

    // import the generated code
    #[allow(dead_code, unused_imports)]
    #[path = "./monster_generated.rs"]
    mod monster_generated;
    pub use monster_generated::my_game::sample::{root_as_monster,
                                                Color, Equipment,
                                                Monster, MonsterArgs,
                                                Vec3,
                                                Weapon, WeaponArgs};
    ```

=== "Swift"

    ```swift
    /**
    // make sure that monster_generated.swift is included in your project
    */
    import Flatbuffers

    // typealiases for convenience
    typealias Monster = MyGame1_Sample_Monster
    typealias Weapon = MyGame1_Sample_Weapon
    typealias Color = MyGame1_Sample_Color
    typealias Vec3 = MyGame1_Sample_Vec3
    ```

=== "TypeScript"

    ```ts
    // note: import flatbuffers with your desired import method

    import { MyGame } from './monster_generated';
    ```

For some languages the runtime libraries are just code files you compile into
your application. While other languages provide packaged libraries via their
package managers.

The generated files include APIs for both serializing and deserializing
FlatBuffers. So these steps are identical for both the consumer and producer.

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
    flatbuffers::FlatBufferBuilder builder(1024);
    ```

=== "C"

    ```c
    flatcc_builder_t builder, *B;
    B = &builder;
    // Initialize the builder object.
    flatcc_builder_init(B);
    ```

=== "C#"

    ```c#
    // Construct a Builder with 1024 byte backing array.
    FlatBufferBuilder builder = new FlatBufferBuilder(1024);
    ```

=== "Dart"

    ```dart
    // Construct a Builder with 1024 byte backing array.
    var builder = new fb.Builder(initialSize: 1024);
    ```

=== "Go"

    ```go
    // Construct a Builder with 1024 byte backing array.
    builder := flatbuffers.NewBuilder(1024)
    ```

=== "Java"

    ```java
    // Construct a Builder with 1024 byte backing array.
    FlatBufferBuilder builder = new FlatBufferBuilder(1024);
    ```

=== "JavaScript"

    ```javascript
    // Construct a Builder with 1024 byte backing array.
    var builder = new flatbuffers.Builder(1024);
    ```

=== "Kotlin"

    ```kotlin
    // Construct a Builder with 1024 byte backing array.
    val builder = FlatBufferBuilder(1024)
    ```

=== "Lobster"

    ```lobster
    // Construct a Builder with 1024 byte backing array.
    let builder = flatbuffers_builder {}
    ```

=== "Lua"

    ```lua
    -- Construct a Builder with 1024 byte backing array.
    local builder = flatbuffers.Builder(1024)
    ```

=== "PHP"

    ```php
    // Construct a Builder with 1024 byte backing array.
    $builder = new Google\FlatBuffers\FlatbufferBuilder(1024);
    ```

=== "Python"

    ```py
    # Construct a Builder with 1024 byte backing array.
    builder = flatbuffers.Builder(1024)
    ```

=== "Rust"

    ```rust
    // Construct a Builder with 1024 byte backing array.
    let mut builder = flatbuffers::FlatBufferBuilder::with_capacity(1024);
    ```

=== "Swift"

    ```swift
    // Construct a Builder with 1024 byte backing array.
    let builder = FlatBufferBuilder(initialSize: 1024)
    ```

=== "TypeScript"

    ```ts
    // Construct a Builder with 1024 byte backing array.
    let builder = new flatbuffers.Builder(1024);
    ```

Once a Builder is available, data can be serialized to it via the Builder APIs
and the generated code.

### Serializing Data

In this tutorial, we are building `Monsters` and `Weapons` for a computer game.
A `Weapon` is represented by a flatbuffer `table` with some fields. One field is
the `name` field, which is type `string` and the other `damage` field is a
numerical scalar.

```c title="monster.fbs" linenums="28"
table Weapon {
  name:string;
  damage:short;
}
```

#### Strings

Since `string` is a reference type, we first need to serialize it before
assigning it to the `name` field of the `Weapon` table. This is done through the
Builder `CreateString` method.

Let's serialize two weapon strings.

=== "C++"

    ```c++
    flatbuffers::Offset<String> weapon_one_name = builder.CreateString("Sword");
    flatbuffers::Offset<String> weapon_two_name = builder.CreateString("Axe");
    ```

    `flatbuffers::Offset<>` is a just a "typed" integer tied to a particular
    type. It helps make the numerical offset more strongly typed.

=== "C"

    ```c
    flatbuffers_string_ref_t weapon_one_name
        = flatbuffers_string_create_str(B, "Sword");
    flatbuffers_string_ref_t weapon_two_name
        = flatbuffers_string_create_str(B, "Axe");
    ```

=== "C#"

    ```c#
    Offset<String> weaponOneName = builder.CreateString("Sword");
    Offset<String> weaponTwoName = builder.CreateString("Axe");
    ```

=== "Dart"

    ```dart
    final int weaponOneName = builder.writeString("Sword");
    final int weaponTwoName = builder.writeString("Axe");
    ```

=== "Go"

    ```go
    weaponOne := builder.CreateString("Sword")
    weaponTwo := builder.CreateString("Axe")
    ```

=== "Java"

    ```java
    int weaponOneName = builder.createString("Sword")
    int weaponTwoName = builder.createString("Axe");
    ```

=== "JavaScript"

    ```javascript
    var weaponOne = builder.createString('Sword');
    var weaponTwo = builder.createString('Axe');
    ```

=== "Kotlin"

    ```kotlin
    val weaponOneName = builder.createString("Sword")
    val weaponTwoName = builder.createString("Axe")
    ```

=== "Lobster"

    ```lobster
    let weapon_one = builder.CreateString("Sword")
    let weapon_two = builder.CreateString("Axe")
    ```

=== "Lua"

    ```lua
    local weaponOne = builder:CreateString("Sword")
    local weaponTwo = builder:CreateString("Axe")
    ```

=== "PHP"

    ```php
    $weapon_one_name = $builder->createString("Sword")
    $weapon_two_name = $builder->createString("Axe");
    ```

=== "Python"

    ```py
    weapon_one = builder.CreateString('Sword')
    weapon_two = builder.CreateString('Axe')
    ```

=== "Rust"

    ```rust
    let weapon_one_name = builder.create_string("Sword");
    let weapon_two_name = builder.create_string("Axe");
    ```

=== "Swift"

    ```swift
    let weapon1Name = builder.create(string: "Sword")
    let weapon2Name = builder.create(string: "Axe")
    ```

=== "TypeScript"

    ```ts
    let weaponOne = builder.createString('Sword');
    let weaponTwo = builder.createString('Axe');
    ```


This performs the actual serialization (the string data is copied into the
backing array) and returns an offset. Think of the offset as a handle to that
reference. It's just a "typed" numerical offset to where that data resides in
the buffer.

#### Tables

Now that we have some names serialized, we can serialize the `Weapon` tables.
Here we will use one of the generated helper functions that was emitted by
`flatc`. The `CreateWeapon` function takes in the Builder object, as well as the
offset to the weapon's name and a numerical value for the damage field.

=== "C++"

    ```c++
    short weapon_one_damage = 3;
    short weapon_two_damage = 5;

    // Use the `CreateWeapon()` shortcut to create Weapons with all the fields
    // set.
    flatbuffers::Offset<Weapon> sword =
        CreateWeapon(builder, weapon_one_name, weapon_one_damage);
    flatbuffers::Offset<Weapon> axe =
        CreateWeapon(builder, weapon_two_name, weapon_two_damage);
    ```

=== "C"

    ```c
    uint16_t weapon_one_damage = 3;
    uint16_t weapon_two_damage = 5;

    ns(Weapon_ref_t) sword
        = ns(Weapon_create(B, weapon_one_name, weapon_one_damage));
    ns(Weapon_ref_t) axe
        = ns(Weapon_create(B, weapon_two_name, weapon_two_damage));
    ```

=== "C#"

    ```c#
    short weaponOneDamage = 3;
    short weaponTwoDamage = 5;

    // Use the `CreateWeapon()` helper function to create the weapons, since we
    // set every field.
    Offset<Weapon> sword =
        Weapon.CreateWeapon(builder, weaponOneName, weaponOneDamage);
    Offset<Weapon> axe =
        Weapon.CreateWeapon(builder, weaponTwoName, weaponTwoDamage);
    ```

=== "Dart"

    ```dart
    final int weaponOneDamage = 3;
    final int weaponTwoDamage = 5;

    final swordBuilder = new myGame.WeaponBuilder(builder)
        ..begin()
        ..addNameOffset(weaponOneName)
        ..addDamage(weaponOneDamage);
    final int sword = swordBuilder.finish();

    final axeBuilder = new myGame.WeaponBuilder(builder)
        ..begin()
        ..addNameOffset(weaponTwoName)
        ..addDamage(weaponTwoDamage);
    final int axe = axeBuilder.finish();
    ```

    Note, as an alternative, the previous steps can be combined using the
    generative Builder classes.

    ```dart
    final myGame.WeaponBuilder sword = new myGame.WeaponObjectBuilder(
        name: "Sword",
        damage: 3,
    );

    final myGame.WeaponBuilder axe = new myGame.WeaponObjectBuilder(
        name: "Axe",
        damage: 5,
    );
    ```

=== "Go"

    ```go
    // Create the first `Weapon` ("Sword").
    sample.WeaponStart(builder)
    sample.WeaponAddName(builder, weaponOne)
    sample.WeaponAddDamage(builder, 3)
    sword := sample.WeaponEnd(builder)

    // Create the second `Weapon` ("Axe").
    sample.WeaponStart(builder)
    sample.WeaponAddName(builder, weaponTwo)
    sample.WeaponAddDamage(builder, 5)
    axe := sample.WeaponEnd(builder)
    ```

=== "Java"

    ```java
    short weaponOneDamage = 3;
    short weaponTwoDamage = 5;

    // Use the `createWeapon()` helper function to create the weapons, since we
    // set every field.
    int sword = Weapon.createWeapon(builder, weaponOneName, weaponOneDamage);
    int axe = Weapon.createWeapon(builder, weaponTwoName, weaponTwoDamage);
    ```

=== "JavaScript"

    ```javascript
    // Create the first `Weapon` ('Sword').
    MyGame.Sample.Weapon.startWeapon(builder);
    MyGame.Sample.Weapon.addName(builder, weaponOne);
    MyGame.Sample.Weapon.addDamage(builder, 3);
    var sword = MyGame.Sample.Weapon.endWeapon(builder);

    // Create the second `Weapon` ('Axe').
    MyGame.Sample.Weapon.startWeapon(builder);
    MyGame.Sample.Weapon.addName(builder, weaponTwo);
    MyGame.Sample.Weapon.addDamage(builder, 5);
    var axe = MyGame.Sample.Weapon.endWeapon(builder);
    ```

=== "Kotlin"

    ```kotlin
    val weaponOneDamage: Short = 3;
    val weaponTwoDamage: Short = 5;

    // Use the `createWeapon()` helper function to create the weapons, since we
    // set every field.
    val sword = Weapon.createWeapon(builder, weaponOneName, weaponOneDamage)
    val axe = Weapon.createWeapon(builder, weaponTwoName, weaponTwoDamage)
    ```

=== "Lobster"

    ```lobster
    let sword = MyGame_Sample_WeaponBuilder { b }
          .start()
          .add_name(weapon_one)
          .add_damage(3)
          .end()

    let axe = MyGame_Sample_WeaponBuilder { b }
          .start()
          .add_name(weapon_two)
          .add_damage(5)
          .end()
    ```

=== "Lua"

    ```lua
    -- Create the first 'Weapon'
    weapon.Start(builder)
    weapon.AddName(builder, weaponOne)
    weapon.AddDamage(builder, 3)
    local sword = weapon.End(builder)

    -- Create the second 'Weapon'
    weapon.Start(builder)
    weapon.AddName(builder, weaponTwo)
    weapon.AddDamage(builder, 5)
    local axe = weapon.End(builder)
    ```

=== "PHP"

    ```php
    $sword = \MyGame\Sample\Weapon::CreateWeapon($builder, $weapon_one_name, 3);
    $axe = \MyGame\Sample\Weapon::CreateWeapon($builder, $weapon_two_name, 5);
    ```

=== "Python"

    ```py
    # Create the first `Weapon` ('Sword').
    MyGame.Sample.Weapon.Start(builder)
    MyGame.Sample.Weapon.AddName(builder, weapon_one)
    MyGame.Sample.Weapon.AddDamage(builder, 3)
    sword = MyGame.Sample.Weapon.End(builder)

    # Create the second `Weapon` ('Axe').
    MyGame.Sample.Weapon.Start(builder)
    MyGame.Sample.Weapon.AddName(builder, weapon_two)
    MyGame.Sample.Weapon.AddDamage(builder, 5)
    axe = MyGame.Sample.Weapon.End(builder)
    ```

=== "Rust"

    ```rust
    // Use the `Weapon::create` shortcut to create Weapons with named field
    // arguments.
    let sword = Weapon::create(&mut builder, &WeaponArgs{
        name: Some(weapon_one_name),
        damage: 3,
    });
    let axe = Weapon::create(&mut builder, &WeaponArgs{
        name: Some(weapon_two_name),
        damage: 5,
    });
    ```

=== "Swift"

    ```swift
    // start creating the weapon by calling startWeapon
    let weapon1Start = Weapon.startWeapon(&builder)
    Weapon.add(name: weapon1Name, &builder)
    Weapon.add(damage: 3, &builder)
    // end the object by passing the start point for the weapon 1
    let sword = Weapon.endWeapon(&builder, start: weapon1Start)

    let weapon2Start = Weapon.startWeapon(&builder)
    Weapon.add(name: weapon2Name, &builder)
    Weapon.add(damage: 5, &builder)
    let axe = Weapon.endWeapon(&builder, start: weapon2Start)
    ```

=== "TypeScript"

    ```ts
    // Create the first `Weapon` ('Sword').
    MyGame.Sample.Weapon.startWeapon(builder);
    MyGame.Sample.Weapon.addName(builder, weaponOne);
    MyGame.Sample.Weapon.addDamage(builder, 3);
    let sword = MyGame.Sample.Weapon.endWeapon(builder);

    // Create the second `Weapon` ('Axe').
    MyGame.Sample.Weapon.startWeapon(builder);
    MyGame.Sample.Weapon.addName(builder, weaponTwo);
    MyGame.Sample.Weapon.addDamage(builder, 5);
    let axe = MyGame.Sample.Weapon.endWeapon(builder);
    ```


The generated functions from `flatc`, like `CreateWeapon`, are just composed of
various Builder API methods. So its not required to use the generated code, but
it does make things much simpler and compact.

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

=== "C"

    ```c
    // We use the internal builder stack to implement a dynamic vector.
    ns(Weapon_vec_start(B));
    ns(Weapon_vec_push(B, sword));
    ns(Weapon_vec_push(B, axe));
    ns(Weapon_vec_ref_t) weapons = ns(Weapon_vec_end(B));
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

=== "Dart"

    ```dart
    // If using the Builder classes, serialize the `[sword,axe]`
    final weapons = builder.writeList([sword, axe]);

    // If using the ObjectBuilders, just create an array from the two `Weapon`s
    final List<myGame.WeaponBuilder> weaps = [sword, axe];
    ```

=== "Go"

    ```go
    // Create a FlatBuffer vector and prepend the weapons.
    // Note: Since we prepend the data, prepend them in reverse order.
    sample.MonsterStartWeaponsVector(builder, 2)
    builder.PrependUOffsetT(axe)
    builder.PrependUOffsetT(sword)
    weapons := builder.EndVector(2)
    ```

=== "Java"

    ```java
    // Place the two weapons into an array, and pass it to the
    // `createWeaponsVector()` method to create a FlatBuffer vector.
    int[] weaps = new int[2];
    weaps[0] = sword;
    weaps[1] = axe;

    // Pass the `weaps` array into the `createWeaponsVector()` method to create
    // a FlatBuffer vector.
    int weapons = Monster.createWeaponsVector(builder, weaps);
    ```

=== "JavaScript"

    ```javascript
    // Create an array from the two `Weapon`s and pass it to the
    // `createWeaponsVector()` method to create a FlatBuffer vector.
    var weaps = [sword, axe];
    var weapons = MyGame.Sample.Monster.createWeaponsVector(builder, weaps);
    ```

=== "Kotlin"

    ```kotlin
    // Place the two weapons into an array, and pass it to the
    // `createWeaponsVector()` method to create a FlatBuffer vector.
    val weaps = intArrayOf(sword, axe)

    // Pass the `weaps` array into the `createWeaponsVector()` method to create
    // a FlatBuffer vector.
    val weapons = Monster.createWeaponsVector(builder, weaps)
    ```

=== "Lobster"

    ```lobster
    let weapons = builder.MyGame_Sample_MonsterCreateWeaponsVector([sword, axe])
    ```

=== "Lua"

    ```lua
    -- Create a FlatBuffer vector and prepend the weapons.
    -- Note: Since we prepend the data, prepend them in reverse order.
    monster.StartWeaponsVector(builder, 2)
    builder:PrependUOffsetTRelative(axe)
    builder:PrependUOffsetTRelative(sword)
    local weapons = builder:EndVector(2)
    ```

=== "PHP"

    ```php
    // Create an array from the two `Weapon`s and pass it to the
    // `CreateWeaponsVector()` method to create a FlatBuffer vector.
    $weaps = array($sword, $axe);
    $weapons = \MyGame\Sample\Monster::CreateWeaponsVector($builder, $weaps);
    ```

=== "Python"

    ```py
    # Create a FlatBuffer vector and prepend the weapons.
    # Note: Since we prepend the data, prepend them in reverse order.
    MyGame.Sample.Monster.StartWeaponsVector(builder, 2)
    builder.PrependUOffsetTRelative(axe)
    builder.PrependUOffsetTRelative(sword)
    weapons = builder.EndVector()
    ```

=== "Rust"

    ```rust
    // Create a FlatBuffer `vector` that contains offsets to the sword and axe
    // we created above.
    let weapons = builder.create_vector(&[sword, axe]);
    ```

=== "Swift"

    ```swift
    // Create a FlatBuffer `vector` that contains offsets to the sword and axe
    // we created above.
    let weaponsOffset = builder.createVector(ofOffsets: [sword, axe])
    ```

=== "TypeScript"

    ```ts
    // Create an array from the two `Weapon`s and pass it to the
    // `createWeaponsVector()` method to create a FlatBuffer vector.
    let weaps = [sword, axe];
    let weapons = MyGame.Sample.Monster.createWeaponsVector(builder, weaps);
    ```


While we are at it, let us serialize the other two vector fields: the
`inventory` field is just a vector of scalars, and the `path` field is a vector
of structs (which are scalar data as well). So these vectors can be serialized a
bit more directly.

=== "C++"

    ```c++
    // Create a `vector` representing the inventory of the Orc. Each number
    // could correspond to an item that can be claimed after he is slain.
    unsigned char treasure[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    flatbuffers::Offset<flatbuffers::Vector<unsigned char>> inventory =
        builder.CreateVector(treasure, 10);

    // Construct an array of two `Vec3` structs.
    Vec3 points[] = { Vec3(1.0f, 2.0f, 3.0f), Vec3(4.0f, 5.0f, 6.0f) };

    // Serialize it as a vector of structs.
    flatbuffers::Offset<flatbuffers::Vector<Vec3>> path =
        builder.CreateVectorOfStructs(points, 2);
    ```

=== "C"

    ```c
    // Create a `vector` representing the inventory of the Orc. Each number
    // could correspond to an item that can be claimed after he is slain.
    uint8_t treasure[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    flatbuffers_uint8_vec_ref_t inventory;
    // `c_vec_len` is the convenience macro we defined earlier.
    inventory = flatbuffers_uint8_vec_create(B, treasure, c_vec_len(treasure));
    ```

=== "C#"

    ```c#
    // Create a `vector` representing the inventory of the Orc. Each number
    // could correspond to an item that can be claimed after he is slain.
    // Note: Since we prepend the bytes, this loop iterates in reverse order.
    Monster.StartInventoryVector(builder, 10);
    for (int i = 9; i >= 0; i--)
    {
        builder.AddByte((byte)i);
    }
    Offset<Vector<byte>> inventory = builder.EndVector();

      // Start building a path vector of length 2.
    Monster.StartPathVector(fbb, 2);

    // Serialize the individual Vec3 structs
    Vec3.CreateVec3(builder, 1.0f, 2.0f, 3.0f);
    Vec3.CreateVec3(builder, 4.0f, 5.0f, 6.0f);

    // End the vector to get the offset
    Offset<Vector<Vec3>> path = fbb.EndVector();
    ```

=== "Dart"

    ```dart
    // Create a list representing the inventory of the Orc. Each number
    // could correspond to an item that can be claimed after he is slain.
    final List<int> treasure = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
    final inventory = builder.writeListUint8(treasure);

    // Using the Builder classes, you can write a list of structs like so:
    // Note that the intended order should be reversed if order is important.
    final vec3Builder = new myGame.Vec3Builder(builder);
    vec3Builder.finish(4.0, 5.0, 6.0);
    vec3Builder.finish(1.0, 2.0, 3.0);
    final int path = builder.endStructVector(2); // the length of the vector
    ```

=== "Go"

    ```go
    // Create a `vector` representing the inventory of the Orc. Each number
    // could correspond to an item that can be claimed after he is slain.
    // Note: Since we prepend the bytes, this loop iterates in reverse.
    sample.MonsterStartInventoryVector(builder, 10)
    for i := 9; i >= 0; i-- {
            builder.PrependByte(byte(i))
    }
    inv := builder.EndVector(10)

    sample.MonsterStartPathVector(builder, 2)
    sample.CreateVec3(builder, 1.0, 2.0, 3.0)
    sample.CreateVec3(builder, 4.0, 5.0, 6.0)
    path := builder.EndVector(2)
    ```

=== "Java"

    ```java
    // Create a `vector` representing the inventory of the Orc. Each number
    // could correspond to an item that can be claimed after he is slain.
    byte[] treasure = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int inv = Monster.createInventoryVector(builder, treasure);

    Monster.startPathVector(fbb, 2);
    Vec3.createVec3(builder, 1.0f, 2.0f, 3.0f);
    Vec3.createVec3(builder, 4.0f, 5.0f, 6.0f);
    int path = fbb.endVector();
    ```

=== "JavaScript"

    ```javascript
    // Create a `vector` representing the inventory of the Orc. Each number
    // could correspond to an item that can be claimed after he is slain.
    var treasure = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
    var inv = MyGame.Sample.Monster.createInventoryVector(builder, treasure);

    MyGame.Sample.Monster.startPathVector(builder, 2);
    MyGame.Sample.Vec3.createVec3(builder, 1.0, 2.0, 3.0);
    MyGame.Sample.Vec3.createVec3(builder, 4.0, 5.0, 6.0);
    var path = builder.endVector();
    ```

=== "Kotlin"

    ```kotlin
    // Create a `vector` representing the inventory of the Orc. Each number
    // could correspond to an item that can be claimed after he is slain.
    val treasure = byteArrayOf(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
    val inv = Monster.createInventoryVector(builder, treasure)

    Monster.startPathVector(fbb, 2)
    Vec3.createVec3(builder, 1.0f, 2.0f, 3.0f)
    Vec3.createVec3(builder, 4.0f, 5.0f, 6.0f)
    val path = fbb.endVector()
    ```

=== "Lobster"

    ```lobster
    // Inventory.
    let inv = builder.MyGame_Sample_MonsterCreateInventoryVector(map(10): _)

    builder.MyGame_Sample_MonsterStartPathVector(2)
    builder.MyGame_Sample_CreateVec3(1.0, 2.0, 3.0)
    builder.MyGame_Sample_CreateVec3(4.0, 5.0, 6.0)
    let path = builder.EndVector(2)
    ```

=== "Lua"

    ```lua
    -- Create a `vector` representing the inventory of the Orc. Each number
    -- could correspond to an item that can be claimed after he is slain.
    -- Note: Since we prepend the bytes, this loop iterates in reverse.
    monster.StartInventoryVector(builder, 10)
    for i=10,1,-1 do
        builder:PrependByte(i)
    end
    local inv = builder:EndVector(10)

    -- Create a FlatBuffer vector and prepend the path locations.
    -- Note: Since we prepend the data, prepend them in reverse order.
    monster.StartPathVector(builder, 2)
    vec3.CreateVec3(builder, 1.0, 2.0, 3.0)
    vec3.CreateVec3(builder, 4.0, 5.0, 6.0)
    local path = builder:EndVector(2)
    ```

=== "PHP"

    ```php
    // Create a `vector` representing the inventory of the Orc. Each number
    // could correspond to an item that can be claimed after he is slain.
    $treasure = array(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
    $inv = \MyGame\Sample\Monster::CreateInventoryVector($builder, $treasure);

    \MyGame\Example\Monster::StartPathVector($builder, 2);
    \MyGame\Sample\Vec3::CreateVec3($builder, 1.0, 2.0, 3.0);
    \MyGame\Sample\Vec3::CreateVec3($builder, 1.0, 2.0, 3.0);
    $path = $builder->endVector();
    ```

=== "Python"

    ```py
    # Create a `vector` representing the inventory of the Orc. Each number
    # could correspond to an item that can be claimed after he is slain.
    # Note: Since we prepend the bytes, this loop iterates in reverse.
    MyGame.Sample.Monster.StartInventoryVector(builder, 10)
    for i in reversed(range(0, 10)):
        builder.PrependByte(i)
    inv = builder.EndVector()

    MyGame.Sample.Monster.StartPathVector(builder, 2)
    MyGame.Sample.Vec3.CreateVec3(builder, 1.0, 2.0, 3.0)
    MyGame.Sample.Vec3.CreateVec3(builder, 4.0, 5.0, 6.0)
    path = builder.EndVector()
    ```

=== "Rust"

    ```rust
    // Inventory.
    let inventory = builder.create_vector(&[0u8, 1, 2, 3, 4, 5, 6, 7, 8, 9]);

    // Create the path vector of Vec3 objects.
    let x = Vec3::new(1.0, 2.0, 3.0);
    let y = Vec3::new(4.0, 5.0, 6.0);
    let path = builder.create_vector(&[x, y]);
    ```

=== "Swift"

    ```swift
    // create inventory
    let inventory: [Byte] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    let inventoryOffset = builder.createVector(inventory)

    let path = fbb.createVector(ofStructs: [
        Vec3(x: 1, y: 2, z: 3),
        Vec3(x: 4, y: 5, z: 6)
    ])
    ```

=== "TypeScript"

    ```ts
    // Create a `vector` representing the inventory of the Orc. Each number
    // could correspond to an item that can be claimed after he is slain.
    let treasure = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
    let inv = MyGame.Sample.Monster.createInventoryVector(builder, treasure);

    MyGame.Sample.Monster.startPathVector(builder, 2);
    MyGame.Sample.Vec3.createVec3(builder, 1.0, 2.0, 3.0);
    MyGame.Sample.Vec3.createVec3(builder, 4.0, 5.0, 6.0);
    let path = builder.endVector();
    ```


#### Unions

The last non-scalar data field for the `Monster` table is the `equipped` `union`
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

=== "C"

    ```c
    // Serialize a name for our monster, called "Orc".
    // The _str suffix indicates the source is an ascii-z string.
    flatbuffers_string_ref_t name = flatbuffers_string_create_str(B, "Orc");

    // Set his hit points to 300 and his mana to 150.
    uint16_t hp = 300;
    uint16_t mana = 150;

    // Define an equipment union. `create` calls in C has a single
    // argument for unions where C++ has both a type and a data argument.
    ns(Equipment_union_ref_t) equipped = ns(Equipment_as_Weapon(axe));
    ns(Vec3_t) pos = { 1.0f, 2.0f, 3.0f };
    ns(Monster_create_as_root(B, &pos, mana, hp, name, inventory, ns(Color_Red),
            weapons, equipped, path));
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

=== "Dart"

    ```dart
    // Serialize a name for our monster, called "Orc".
    final int name = builder.writeString('Orc');

    // Using the Builder API:
    // Set his hit points to 300 and his mana to 150.
    final int hp = 300;
    final int mana = 150;

    final monster = new myGame.MonsterBuilder(builder)
        ..begin()
        ..addNameOffset(name)
        ..addInventoryOffset(inventory)
        ..addWeaponsOffset(weapons)
        ..addEquippedType(myGame.EquipmentTypeId.Weapon)
        ..addEquippedOffset(axe)
        ..addHp(hp)
        ..addMana(mana)
        ..addPos(vec3Builder.finish(1.0, 2.0, 3.0))
        ..addPathOffset(path)
        ..addColor(myGame.Color.Red);

    final int orc = monster.finish();
    ```

=== "Go"

    ```go
    // Serialize a name for our monster, called "Orc".
    name := builder.CreateString("Orc")

    // Create our monster using `MonsterStart()` and `MonsterEnd()`.
    sample.MonsterStart(builder)
    sample.MonsterAddPos(builder, sample.CreateVec3(builder, 1.0, 2.0, 3.0))
    sample.MonsterAddHp(builder, 300)
    sample.MonsterAddName(builder, name)
    sample.MonsterAddInventory(builder, inv)
    sample.MonsterAddColor(builder, sample.ColorRed)
    sample.MonsterAddWeapons(builder, weapons)
    sample.MonsterAddEquippedType(builder, sample.EquipmentWeapon)
    sample.MonsterAddEquipped(builder, axe)
    sample.MonsterAddPath(builder, path)
    orc := sample.MonsterEnd(builder)
    ```

=== "Java"

    ```java
    // Serialize a name for our monster, called "Orc".
    int name = builder.createString("Orc");

    // Create our monster using `startMonster()` and `endMonster()`.
    Monster.startMonster(builder);
    Monster.addPos(builder, Vec3.createVec3(builder, 1.0f, 2.0f, 3.0f));
    Monster.addName(builder, name);
    Monster.addColor(builder, Color.Red);
    Monster.addHp(builder, (short)300);
    Monster.addInventory(builder, inv);
    Monster.addWeapons(builder, weapons);
    Monster.addEquippedType(builder, Equipment.Weapon);
    Monster.addEquipped(builder, axe);
    Monster.addPath(builder, path);
    int orc = Monster.endMonster(builder);
    ```

=== "JavaScript"

    ```javascript
    // Serialize a name for our monster, called 'Orc'.
    var name = builder.createString('Orc');

    // Create our monster by using `startMonster()` and `endMonster()`.
    MyGame.Sample.Monster.startMonster(builder);
    MyGame.Sample.Monster.addPos(builder,
       MyGame.Sample.Vec3.createVec3(builder, 1.0, 2.0, 3.0));
    MyGame.Sample.Monster.addHp(builder, 300);
    MyGame.Sample.Monster.addColor(builder, MyGame.Sample.Color.Red)
    MyGame.Sample.Monster.addName(builder, name);
    MyGame.Sample.Monster.addInventory(builder, inv);
    MyGame.Sample.Monster.addWeapons(builder, weapons);
    MyGame.Sample.Monster.addEquippedType(builder,
        MyGame.Sample.Equipment.Weapon);
    MyGame.Sample.Monster.addEquipped(builder, axe);
    MyGame.Sample.Monster.addPath(builder, path);
    var orc = MyGame.Sample.Monster.endMonster(builder);
    ```

=== "Kotlin"

    ```kotlin
    // Serialize a name for our monster, called "Orc".
    val name = builder.createString("Orc")

    // Create our monster using `startMonster()` and `endMonster()`.
    Monster.startMonster(builder)
    Monster.addPos(builder, Vec3.createVec3(builder, 1.0f, 2.0f, 3.0f))
    Monster.addName(builder, name)
    Monster.addColor(builder, Color.Red)
    Monster.addHp(builder, 300.toShort())
    Monster.addInventory(builder, inv)
    Monster.addWeapons(builder, weapons)
    Monster.addEquippedType(builder, Equipment.Weapon)
    Monster.addEquipped(builder, axe)
    Monster.addPath(builder, path)
    val orc = Monster.endMonster(builder)
    ```

=== "Lobster"

    ```lobster
    // Name of the monster.
    let name = builder.CreateString("Orc")

    let orc = MyGame_Sample_MonsterBuilder { b }
        .start()
        .add_pos(b.MyGame_Sample_CreateVec3(1.0, 2.0, 3.0))
        .add_hp(300)
        .add_name(name)
        .add_inventory(inv)
        .add_color(MyGame_Sample_Color_Red)
        .add_weapons(weapons)
        .add_equipped_type(MyGame_Sample_Equipment_Weapon)
        .add_equipped(weapon_offsets[1])
        .add_path(path)
        .end()
    ```

=== "Lua"

    ```lua
    -- Serialize a name for our monster, called 'orc'
    local name = builder:CreateString("Orc")

    -- Create our monster by using Start() andEnd()
    monster.Start(builder)
    monster.AddPos(builder, vec3.CreateVec3(builder, 1.0, 2.0, 3.0))
    monster.AddHp(builder, 300)
    monster.AddName(builder, name)
    monster.AddInventory(builder, inv)
    monster.AddColor(builder, color.Red)
    monster.AddWeapons(builder, weapons)
    monster.AddEquippedType(builder, equipment.Weapon)
    monster.AddEquipped(builder, axe)
    monster.AddPath(builder, path)
    local orc = monster.End(builder)
    ```

=== "PHP"

    ```php
    // Serialize a name for our monster, called "Orc".
    $name = $builder->createString("Orc");

        // Create our monster by using `StartMonster()` and `EndMonster()`.
    \MyGame\Sample\Monster::StartMonster($builder);
    \MyGame\Sample\Monster::AddPos($builder,
        \MyGame\Sample\Vec3::CreateVec3($builder, 1.0, 2.0, 3.0));
    \MyGame\Sample\Monster::AddHp($builder, 300);
    \MyGame\Sample\Monster::AddName($builder, $name);
    \MyGame\Sample\Monster::AddInventory($builder, $inv);
    \MyGame\Sample\Monster::AddColor($builder, \MyGame\Sample\Color::Red);
    \MyGame\Sample\Monster::AddWeapons($builder, $weapons);
    \MyGame\Sample\Monster::AddEquippedType($builder,
        \MyGame\Sample\Equipment::Weapon);
    \MyGame\Sample\Monster::AddEquipped($builder, $axe);
    \MyGame\Sample\Monster::AddPath($builder, $path);
    $orc = \MyGame\Sample\Monster::EndMonster($builder);
    ```

=== "Python"

    ```py
    # Serialize a name for our monster, called "Orc".
    name = builder.CreateString("Orc")

    # Create our monster by using `Monster.Start()` and `Monster.End()`.
    MyGame.Sample.Monster.Start(builder)
    MyGame.Sample.Monster.AddPos(builder,
        MyGame.Sample.Vec3.CreateVec3(builder, 1.0, 2.0, 3.0))
    MyGame.Sample.Monster.AddHp(builder, 300)
    MyGame.Sample.Monster.AddName(builder, name)
    MyGame.Sample.Monster.AddInventory(builder, inv)
    MyGame.Sample.Monster.AddColor(builder,
                                            MyGame.Sample.Color.Color().Red)
    MyGame.Sample.Monster.AddWeapons(builder, weapons)
    MyGame.Sample.Monster.AddEquippedType(
        builder, MyGame.Sample.Equipment.Equipment().Weapon)
    MyGame.Sample.Monster.AddEquipped(builder, axe)
    MyGame.Sample.Monster.AddPath(builder, path)
    orc = MyGame.Sample.Monster.End(builder)
    ```

=== "Rust"

    ```rust
    // Name of the Monster.
    let name = builder.create_string("Orc");

    // Create the monster using the `Monster::create` helper function. This
    // function accepts a `MonsterArgs` struct, which supplies all of the data
    // needed to build a `Monster`. To supply empty/default fields, just use the
    // Rust built-in `Default::default()` function, as demonstrated below.
    let orc = Monster::create(&mut builder, &MonsterArgs{
        pos: Some(&Vec3::new(1.0f32, 2.0f32, 3.0f32)),
        mana: 150,
        hp: 80,
        name: Some(name),
        inventory: Some(inventory),
        color: Color::Red,
        weapons: Some(weapons),
        equipped_type: Equipment::Weapon,
        equipped: Some(axe.as_union_value()),
        path: Some(path),
        ..Default::default()
    });
    ```

=== "Swift"

    ```swift
    // Name of the Monster.
    let name = builder.create(string: "Orc")

    let orc = Monster.createMonster(
        &builder,
        pos: MyGame_Sample_Vec3(x: 1, y: 2, z: 3),
        hp: 300,
        nameOffset: name,
        inventoryVectorOffset: inventoryOffset,
        color: .red,
        weaponsVectorOffset: weaponsOffset,
        equippedType: .weapon,
        equippedOffset: axe)
    ```

=== "TypeScript"

    ```ts
    // Serialize a name for our monster, called 'Orc'.
    let name = builder.createString('Orc');

    // Create our monster by using `startMonster()` and `endMonster()`.
    MyGame.Sample.Monster.startMonster(builder);
    MyGame.Sample.Monster.addPos(builder,
        MyGame.Sample.Vec3.createVec3(builder, 1.0, 2.0, 3.0));
    MyGame.Sample.Monster.addHp(builder, 300);
    MyGame.Sample.Monster.addColor(builder, MyGame.Sample.Color.Red)
    MyGame.Sample.Monster.addName(builder, name);
    MyGame.Sample.Monster.addInventory(builder, inv);
    MyGame.Sample.Monster.addWeapons(builder, weapons);
    MyGame.Sample.Monster.addEquippedType(builder,
        MyGame.Sample.Equipment.Weapon);
    MyGame.Sample.Monster.addEquipped(builder, axe);
    MyGame.Sample.Monster.addPath(builder, path);
    let orc = MyGame.Sample.Monster.endMonster(builder);
    ```


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

=== "C"

    ```c
    // Because we used `Monster_create_as_root`, we do not need a `finish` call
    // in C.
    ```

=== "C#"

    ```c#
    // Call `Finish()` to instruct the builder that this monster is complete.
    // You could also call `Monster.FinishMonsterBuffer(builder, orc);`
    builder.Finish(orc.Value);
    ```

=== "Dart"

    ```dart
    // Call `finish()` to instruct the builder that this monster is complete.
    // See the next code section, as in Dart `finish` will also return the byte
    // array.
    ```

=== "Go"

    ```go
    // Call `Finish()` to instruct the builder that this monster is complete.
    builder.Finish(orc)
    ```

=== "Java"

    ```java
    // Call `finish()` to instruct the builder that this monster is complete.
    builder.finish(orc);
    ```

=== "JavaScript"

    ```javascript
    // Call `finish()` to instruct the builder that this monster is complete.
    builder.finish(orc);
    ```

=== "Kotlin"

    ```kotlin
    // Call `finish()` to instruct the builder that this monster is complete.
    builder.finish(orc) ;
    ```

=== "Lobster"

    ```lobster
    // Call `Finish()` to instruct the builder that this monster is complete.
    builder.Finish(orc)
    ```

=== "Lua"

    ```lua
    -- Call 'Finish()' to instruct the builder that this monster is complete.
    builder:Finish(orc)
    ```

=== "PHP"

    ```php
    // Call `finish()` to instruct the builder that this monster is complete.
    $builder->finish($orc);
    ```

=== "Python"

    ```py
    # Call `Finish()` to instruct the builder that this monster is complete.
    builder.Finish(orc)
    ```

=== "Rust"

    ```rust
    // Call `finish()` to instruct the builder that this monster is complete.
    builder.finish(orc, None);
    ```

=== "Swift"

    ```swift
    // Call `finish()` to instruct the builder that this monster is complete.
    builder.finish(offset: orc)
    ```

=== "TypeScript"

    ```ts
    // Call `finish()` to instruct the builder that this monster is complete.
    builder.finish(orc);
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

=== "C"

    ```c
    uint8_t *buf;
    size_t size;

    // Allocate and extract a readable buffer from internal builder heap.
    // The returned buffer must be deallocated using `free`.
    // NOTE: Finalizing the buffer does NOT change the builder, it
    // just creates a snapshot of the builder content.
    buf = flatcc_builder_finalize_buffer(B, &size);
    // use buf
    free(buf);

    // Optionally reset builder to reuse builder without deallocating
    // internal stack and heap.
    flatcc_builder_reset(B);
    // build next buffer.
    // ...

    // Cleanup.
    flatcc_builder_clear(B);
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

=== "Dart"

    ```dart
    final Uint8List buf = builder.finish(orc);
    ```

=== "Go"

    ```go
    // This must be called after `Finish()`.
    buf := builder.FinishedBytes() // Of type `byte[]`.
    ```

=== "Java"

    ```java
    // This must be called after `finish()`.
    java.nio.ByteBuffer buf = builder.dataBuffer();
    // The data in this ByteBuffer does NOT start at 0, but at buf.position().
    // The number of bytes is buf.remaining().

    // Alternatively this copies the above data out of the ByteBuffer for you:
    byte[] buf = builder.sizedByteArray();
    ```

=== "JavaScript"

    ```javascript
    // This must be called after `finish()`.
    var buf = builder.asUint8Array(); // Of type `Uint8Array`.
    ```

=== "Kotlin"

    ```kotlin
    // This must be called after `finish()`.
    val buf = builder.dataBuffer()
    // The data in this ByteBuffer does NOT start at 0, but at buf.position().
    // The number of bytes is buf.remaining().

    // Alternatively this copies the above data out of the ByteBuffer for you:
    val buf = builder.sizedByteArray()
    ```

=== "Lobster"

    ```lobster
    // This must be called after `Finish()`.
    let buf = builder.SizedCopy() // Of type `string`.
    ```

=== "Lua"

    ```lua
    local bufAsString = builder:Output()
    ```

=== "PHP"

    ```php
    // This must be called after `finish()`.
    $buf = $builder->dataBuffer(); // Of type `Google\FlatBuffers\ByteBuffer`
    // The data in this ByteBuffer does NOT start at 0, but at
    // buf->getPosition().
    // The end of the data is marked by buf->capacity(), so the size is
    // buf->capacity() - buf->getPosition().
    ```

=== "Python"

    ```py
    # This must be called after `Finish()`.
    buf = builder.Output() // Of type `bytearray`.
    ```

=== "Rust"

    ```rust
    // This must be called after `finish()`.
    // `finished_data` returns a byte slice.
    let buf = builder.finished_data(); // Of type `&[u8]`
    ```

=== "Swift"

    ```swift
    // This must be called after `finish()`.
    // `sizedByteArray` returns the finished buf of type [UInt8].
    let buf = builder.sizedByteArray
    // or you can use to get an object of type Data
    let bufData = ByteBuffer(data: builder.data)
    // or
    let buf = builder.sizedBuffer
    ```

=== "TypeScript"

    ```ts
    // This must be called after `finish()`.
    let buf = builder.asUint8Array(); // Of type `Uint8Array`.
    ```


Now you can write the bytes to a file or send them over the network. The buffer
stays valid until the Builder is cleared or destroyed.

Make sure your file mode (or transfer protocol) is set to BINARY, and not TEXT.
If you try to transfer a flatbuffer in TEXT mode, the buffer will be corrupted
and be hard to diagnose.

## Deserialization

Deserialization is a bit of a misnomer, since FlatBuffers doesn't deserialize
the whole buffer when accessed. It just "decodes" the data that is requested,
leaving all the other data untouched. It is up to the application to decide if
the data is copied out or even read in the first place. However, we continue to
use the word `deserialize` to mean accessing data from a binary flatbuffer.

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

    // Get a view to the root object inside the buffer.
    Monster monster = GetMonster(buffer_pointer);
    ```

=== "C"

    ```c
    // Note that we use the `table_t` suffix when reading a table object
    // as opposed to the `ref_t` suffix used during the construction of
    // the buffer.
    ns(Monster_table_t) monster = ns(Monster_as_root(buffer));

    // Note: root object pointers are NOT the same as the `buffer` pointer.
    ```

=== "C#"

    ```c#
    byte[] bytes = /* the data you just read */

    // Get a view to the root object inside the buffer.
    Monster monster = Monster.GetRootAsMonster(new ByteBuffer(bytes));
    ```

=== "Dart"

    ```dart
    List<int> data = ... // the data, e.g. from file or network
    // A generated factory constructor that will read the data.
    myGame.Monster monster = new myGame.Monster(data);
    ```

=== "Go"

    ```go
    var buf []byte = /* the data you just read */

    // Get an accessor to the root object inside the buffer.
    monster := sample.GetRootAsMonster(buf, 0)

    // Note: We use `0` for the offset here, which is typical for most buffers
    // you would read. If you wanted to read from `builder.Bytes` directly, you
    // would need to pass in the offset of `builder.Head()`, as the builder
    // constructs the buffer backwards, so may not start at offset 0.
    ```

=== "Java"

    ```java
    byte[] bytes = /* the data you just read */
    java.nio.ByteBuffer buf = java.nio.ByteBuffer.wrap(bytes);

    // Get an accessor to the root object inside the buffer.
    Monster monster = Monster.getRootAsMonster(buf);
    ```

=== "JavaScript"

    ```javascript
    // the data you just read, as a `Uint8Array`
    // Note that the example here uses `readFileSync` from the built-in `fs`
    // module, but other methods for accessing the file contents will also work.
    var bytes = new Uint8Array(readFileSync('./monsterdata.bin'));

    var buf = new flatbuffers.ByteBuffer(bytes);

    // Get an accessor to the root object inside the buffer.
    var monster = MyGame.Sample.Monster.getRootAsMonster(buf);
    ```

=== "Kotlin"

    ```kotlin
    val bytes = /* the data you just read */
    val buf = java.nio.ByteBuffer.wrap(bytes)

    // Get an accessor to the root object inside the buffer.
    Monster monster = Monster.getRootAsMonster(buf)
    ```

=== "Lobster"

    ```lobster
    buf = /* the data you just read, in a string */

    // Get an accessor to the root object inside the buffer.
    let monster = MyGame_Sample_GetRootAsMonster(buf)
    ```

=== "Lua"

    ```lua
    local bufAsString =   -- The data you just read in

    -- Convert the string representation into binary array Lua structure
    local buf = flatbuffers.binaryArray.New(bufAsString)

    -- Get an accessor to the root object insert the buffer
    local mon = monster.GetRootAsMonster(buf, 0)
    ```

=== "PHP"

    ```php
    $bytes = /* the data you just read, in a string */
    $buf = Google\FlatBuffers\ByteBuffer::wrap($bytes);

    // Get an accessor to the root object inside the buffer.
    $monster = \MyGame\Sample\Monster::GetRootAsMonster($buf);
    ```

=== "Python"

    ```py
    buf = /* the data you just read, in an object of type "bytearray" */

    # Get an accessor to the root object inside the buffer.
    monster = MyGame.Sample.Monster.Monster.GetRootAs(buf, 0)

    # Note: We use `0` for the offset here, which is typical for most buffers
    # you would read.  If you wanted to read from the `builder.Bytes` directly,
    # you would need to pass in the offset of `builder.Head()`, as the builder
    # constructs the buffer backwards, so may not start at offset 0.
    ```

=== "Rust"

    ```rust
    let buf = /* the data you just read, in a &[u8] */

    // Get an accessor to the root object inside the buffer.
    let monster = root_as_monster(buf).unwrap();
    ```

=== "Swift"

    ```swift
    // create a ByteBuffer(:) from an [UInt8] or Data()
    var buf = // Get your data
    // Get an accessor to the root object inside the buffer.
    let monster: Monster = try! getCheckedRoot(byteBuffer: &byteBuffer)
    // let monster: Monster = getRoot(byteBuffer: &byteBuffer)
    ```

=== "TypeScript"

    ```ts
    // the data you just read, as a `Uint8Array`.
    // Note that the example here uses `readFileSync` from the built-in `fs`
    // module, but other methods for accessing the file contents will also work.
    let bytes = new Uint8Array(readFileSync('./monsterdata.bin'));

    let buf = new flatbuffers.ByteBuffer(bytes);

    // Get an accessor to the root object inside the buffer.
    let monster = MyGame.Sample.Monster.getRootAsMonster(buf);
    ```


Again, make sure you read the bytes in BINARY mode, otherwise the buffer may be
corrupted.

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

=== "C"

    ```c
    uint16_t hp = ns(Monster_hp(monster));
    uint16_t mana = ns(Monster_mana(monster));
    flatbuffers_string_t name = ns(Monster_name(monster));
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

=== "Dart"

    ```dart
    // For Dart, unlike other languages support by FlatBuffers, most values
    // are available as properties instead of accessor methods.
    var hp = monster.hp;
    var mana = monster.mana;
    var name = monster.name;
    ```

=== "Go"

    ```go
    hp := monster.Hp()
    mana := monster.Mana()
    name := string(monster.Name()) // Note: `monster.Name()` returns a byte[].
    ```

=== "Java"

    ```java
    short hp = monster.hp();
    short mana = monster.mana();
    String name = monster.name();
    ```

=== "JavaScript"

    ```javascript
    var hp = monster.hp();
    var mana = monster.mana();
    var name = monster.name();
    ```

=== "Kotlin"

    ```kotlin
    val hp = monster.hp
    val mana = monster.mana
    val name = monster.name
    ```

=== "Lobster"

    ```lobster
    let hp = monster.hp
    let mana = monster.mana
    let name = monster.name
    ```

=== "Lua"

    ```lua
    local hp = mon:Hp()
    local mana = mon:Mana()
    local name = mon:Name()
    ```

=== "PHP"

    ```php
    $hp = $monster->getHp();
    $mana = $monster->getMana();
    $name = monster->getName();
    ```

=== "Python"

    ```py
    hp = monster.Hp()
    mana = monster.Mana()
    name = monster.Name()
    ```

=== "Rust"

    ```rust
    // Get and test some scalar types from the FlatBuffer.
    let hp = monster.hp();
    let mana = monster.mana();
    let name = monster.name();
    ```

=== "Swift"

    ```swift
    let hp = monster.hp
    let mana = monster.mana
    let name = monster.name // returns an optional string
    ```

=== "TypeScript"

    ```ts
    let hp = monster.hp();
    let mana = monster.mana();
    let name = monster.name();
    ```


These accessors should hold the values `300`, `150`, and `"Orc"` respectively.

The default value of `150` wasn't stored in the `mana` field, but we are still
able to retrieve it. That is because the generated accessors return a hard-coded
default value when it doesn't find the value in the buffer.

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

=== "C"

    ```c
    ns(Vec3_struct_t) pos = ns(Monster_pos(monster));
    float x = ns(Vec3_x(pos));
    float y = ns(Vec3_y(pos));
    float z = ns(Vec3_z(pos));
    ```

=== "C#"

    ```c#
    var pos = monster.Pos.Value;
    var x = pos.X;
    var y = pos.Y;
    var z = pos.Z;
    ```

=== "Dart"

    ```dart
    myGame.Vec3 pos = monster.pos;
    double x = pos.x;
    double y = pos.y;
    double z = pos.z;
    ```

=== "Go"

    ```go
    pos := monster.Pos(nil)
    x := pos.X()
    y := pos.Y()
    z := pos.Z()

    // Note: Whenever you access a new object, like in `Pos()`, a new temporary
    // accessor object gets created. If your code is very performance sensitive,
    // you can pass in a pointer to an existing `Vec3` instead of `nil`. This
    // allows you to reuse it across many calls to reduce the amount of object
    // allocation/garbage collection.
    ```

=== "Java"

    ```java
    Vec3 pos = monster.pos();
    float x = pos.x();
    float y = pos.y();
    float z = pos.z();
    ```

=== "JavaScript"

    ```javascript
    var pos = monster.pos();
    var x = pos.x();
    var y = pos.y();
    var z = pos.z();
    ```

=== "Kotlin"

    ```kotlin
    val pos = monster.pos!!
    val x = pos.x
    val y = pos.y
    val z = pos.z
    ```

=== "Lobster"

    ```lobster
    let pos = monster.pos
    let x = pos.x
    let y = pos.y
    let z = pos.z
    ```

=== "Lua"

    ```lua
    local pos = mon:Pos()
    local x = pos:X()
    local y = pos:Y()
    local z = pos:Z()
    ```

=== "PHP"

    ```php
    $pos = $monster->getPos();
    $x = $pos->getX();
    $y = $pos->getY();
    $z = $pos->getZ();
    ```

=== "Python"

    ```py
    pos = monster.Pos()
    x = pos.X()
    y = pos.Y()
    z = pos.Z()
    ```

=== "Rust"

    ```rust
    let pos = monster.pos().unwrap();
    let x = pos.x();
    let y = pos.y();
    let z = pos.z();
    ```

=== "Swift"

    ```swift
    let pos = monster.pos
    let x = pos.x
    let y = pos.y
    let z = pos.z
    ```

=== "TypeScript"

    ```ts
    let pos = monster.pos();
    let x = pos.x();
    let y = pos.y();
    let z = pos.z();
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

=== "C"

    ```c
    // If `inv` hasn't been set, it will be null. It is valid get
    // the length of null which will be 0, useful for iteration.
    flatbuffers_uint8_vec_t inv = ns(Monster_inventory(monster));
    size_t inv_len = flatbuffers_uint8_vec_len(inv);
    ```

=== "C#"

    ```c#
    int invLength = monster.InventoryLength;
    var thirdItem = monster.Inventory(2);
    ```

=== "Dart"

    ```dart
    int invLength = monster.inventory.length;
    var thirdItem = monster.inventory[2];
    ```

=== "Go"

    ```go
    invLength := monster.InventoryLength()
    thirdItem := monster.Inventory(2)
    ```

=== "Java"

    ```java
    int invLength = monster.inventoryLength();
    byte thirdItem = monster.inventory(2);
    ```

=== "JavaScript"

    ```javascript
    var invLength = monster.inventoryLength();
    var thirdItem = monster.inventory(2);
    ```

=== "Kotlin"

    ```kotlin
    val invLength = monster.inventoryLength
    val thirdItem = monster.inventory(2)!!
    ```

=== "Lobster"

    ```lobster
    let inv_len = monster.inventory_length
    let third_item = monster.inventory(2)
    ```

=== "Lua"

    ```lua
    local invLength = mon:InventoryLength()
    local thirdItem = mon:Inventory(3) -- Lua is 1-based
    ```

=== "PHP"

    ```php
    $inv_len = $monster->getInventoryLength();
    $third_item = $monster->getInventory(2);
    ```

=== "Python"

    ```py
    inv_len = monster.InventoryLength()
    third_item = monster.Inventory(2)
    ```

=== "Rust"

    ```rust
    // Get and test an element from the `inventory` FlatBuffer's `vector`.
    let inv = monster.inventory().unwrap();

    // Note that this vector is returned as a slice, because direct access for
    // this type, a `u8` vector, is safe on all platforms:
    let third_item = inv[2];
    ```

=== "Swift"

    ```swift
    // Get a the count of objects in the vector
    let count = monster.inventoryCount

    // get item at index 4
    let object = monster.inventory(at: 4)

    // or you can fetch the entire array
    let inv = monster.inventory
    // inv[4] should equal object
    ```

=== "TypeScript"

    ```ts
    let invLength = monster.inventoryLength();
    let thirdItem = monster.inventory(2);
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

=== "C"

    ```c
    ns(Weapon_vec_t) weapons = ns(Monster_weapons(monster));
    size_t weapons_len = ns(Weapon_vec_len(weapons));
    // We can use `const char *` instead of `flatbuffers_string_t`.
    const char *second_weapon_name =
        ns(Weapon_name(ns(Weapon_vec_at(weapons, 1))));
    uint16_t second_weapon_damage =
        ns(Weapon_damage(ns(Weapon_vec_at(weapons, 1))));
    ```

=== "C#"

    ```c#
    int weaponsLength = monster.WeaponsLength;
    var secondWeaponName = monster.Weapons(1).Name;
    var secondWeaponDamage = monster.Weapons(1).Damage;
    ```

=== "Dart"

    ```dart
    int weaponsLength = monster.weapons.length;
    var secondWeaponName = monster.weapons[1].name;
    var secondWeaponDamage = monster.Weapons[1].damage;
    ```

=== "Go"

    ```go
    weaponLength := monster.WeaponsLength()
    // We need a `sample.Weapon` to pass into `monster.Weapons()`
    // to capture the output of the function.k
    weapon := new(sample.Weapon)
    if monster.Weapons(weapon, 1) {
            secondWeaponName := weapon.Name()
            secondWeaponDamage := weapon.Damage()
    }
    ```

=== "Java"

    ```java
    int weaponsLength = monster.weaponsLength();
    String secondWeaponName = monster.weapons(1).name();
    short secondWeaponDamage = monster.weapons(1).damage();
    ```

=== "JavaScript"

    ```javascript
    var weaponsLength = monster.weaponsLength();
    var secondWeaponName = monster.weapons(1).name();
    var secondWeaponDamage = monster.weapons(1).damage();
    ```

=== "Kotlin"

    ```kotlin
    val weaponsLength = monster.weaponsLength
    val secondWeaponName = monster.weapons(1)!!.name
    val secondWeaponDamage = monster.weapons(1)!!.damage
    ```

=== "Lobster"

    ```lobster
    let weapons_length = monster.weapons_length
    let second_weapon_name = monster.weapons(1).name
    let second_weapon_damage = monster.weapons(1).damage
    ```

=== "Lua"

    ```lua
    local weaponsLength = mon:WeaponsLength()
    local secondWeaponName = mon:Weapon(2):Name()
    local secondWeaponDamage = mon:Weapon(2):Damage()
    ```

=== "PHP"

    ```php
    $weapons_len = $monster->getWeaponsLength();
    $second_weapon_name = $monster->getWeapons(1)->getName();
    $second_weapon_damage = $monster->getWeapons(1)->getDamage();
    ```

=== "Python"

    ```py
    weapons_length = monster.WeaponsLength()
    second_weapon_name = monster.Weapons(1).Name()
    second_weapon_damage = monster.Weapons(1).Damage()
    ```

=== "Rust"

    ```rust
    // Get and test the `weapons` FlatBuffers's `vector`.
    let weps = monster.weapons().unwrap();
    let weps_len = weps.len();

    let wep2 = weps.get(1);
    let second_weapon_name = wep2.name();
    let second_weapon_damage = wep2.damage();
    ```

=== "Swift"

    ```swift
    // Get the count of weapon objects
    let wepsCount = monster.weaponsCount

    let weapon2 = monster.weapons(at: 1)
    let weaponName = weapon2.name
    let weaponDmg = weapon2.damage
    ```

=== "TypeScript"

    ```ts
    let weaponsLength = monster.weaponsLength();
    let secondWeaponName = monster.weapons(1).name();
    let secondWeaponDamage = monster.weapons(1).damage();
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

=== "C"

    ```c
    // Access union type field.
    if (ns(Monster_equipped_type(monster)) == ns(Equipment_Weapon)) {
        // Cast to appropriate type:
        // C allows for silent void pointer assignment, so we need no
        // explicit cast.
        ns(Weapon_table_t) weapon = ns(Monster_equipped(monster));
        const char *weapon_name = ns(Weapon_name(weapon)); // "Axe"
        uint16_t weapon_damage = ns(Weapon_damage(weapon)); // 5
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

=== "Dart"

    ```dart
    var unionType = monster.equippedType.value;

    if (unionType == myGame.EquipmentTypeId.Weapon.value) {
        myGame.Weapon weapon = mon.equipped as myGame.Weapon;

        var weaponName = weapon.name;     // "Axe"
        var weaponDamage = weapon.damage; // 5
    }
    ```

=== "Go"

    ```go
    // We need a `flatbuffers.Table` to capture the output of the
    // `monster.Equipped()` function.
    unionTable := new(flatbuffers.Table)

    if monster.Equipped(unionTable) {
        unionType := monster.EquippedType()

        if unionType == sample.EquipmentWeapon {
            // Create a `sample.Weapon` object that can be initialized with the
            // contents of the `flatbuffers.Table` (`unionTable`), which was
            // populated by `monster.Equipped()`.
            unionWeapon = new(sample.Weapon)
            unionWeapon.Init(unionTable.Bytes, unionTable.Pos)

            weaponName = unionWeapon.Name()
            weaponDamage = unionWeapon.Damage()
        }
    }
    ```

=== "Java"

    ```java
    int unionType = monster.EquippedType();

    if (unionType == Equipment.Weapon) {
        // Requires an explicit cast to `Weapon`.
        Weapon weapon = (Weapon)monster.equipped(new Weapon());

        String weaponName = weapon.name();    // "Axe"
        short weaponDamage = weapon.damage(); // 5
    }
    ```

=== "JavaScript"

    ```javascript
    var unionType = monster.equippedType();

    if (unionType == MyGame.Sample.Equipment.Weapon) {
        // 'Axe'
        var weaponName = monster.equipped(new MyGame.Sample.Weapon()).name();
        // 5
        var weaponDamage =
            monster.equipped(new MyGame.Sample.Weapon()).damage();
    }
    ```

=== "Kotlin"

    ```kotlin
    val unionType = monster.EquippedType

    if (unionType == Equipment.Weapon) {
        // Requires an explicit cast to `Weapon`.
        val weapon = monster.equipped(Weapon()) as Weapon

        val weaponName = weapon.name   // "Axe"
        val weaponDamage = weapon.damage // 5
    }
    ```

=== "Lobster"

    ```lobster
    union_type = monster.equipped_type

    if union_type == MyGame_Sample_Equipment_Weapon:
        // `monster.equipped_as_Weapon` returns a FlatBuffer handle much like
        // normal table fields, but this is only valid to call if we already
        // know it is the correct type.
        let union_weapon = monster.equipped_as_Weapon

        let weapon_name = union_weapon.name     // "Axe"
        let weapon_damage = union_weapon.damage // 5
    ```

=== "Lua"

    ```lua
    local unionType = mon:EquippedType()

    if unionType == equipment.Weapon then
        local unionWeapon = weapon.New()
        unionWeapon:Init(mon:Equipped().bytes, mon:Equipped().pos)

        local weaponName = unionWeapon:Name()     -- 'Axe'
        local weaponDamage = unionWeapon:Damage() -- 5
    end
    ```

=== "PHP"

    ```php
    $union_type = $monster->getEquippedType();

    if ($union_type == \MyGame\Sample\Equipment::Weapon) {
        // "Axe"
        $weapon_name =
            $monster->getEquipped(new \MyGame\Sample\Weapon())->getName();
        // 5
        $weapon_damage =
            $monster->getEquipped(new \MyGame\Sample\Weapon())->getDamage();
    }
    ```

=== "Python"

    ```py
    union_type = monster.EquippedType()

    if union_type == MyGame.Sample.Equipment.Equipment().Weapon:
        # `monster.Equipped()` returns a `flatbuffers.Table`, which can be used
        # to initialize a `MyGame.Sample.Weapon.Weapon()`.
        union_weapon = MyGame.Sample.Weapon.Weapon()
        union_weapon.Init(monster.Equipped().Bytes, monster.Equipped().Pos)

        weapon_name = union_weapon.Name()     // 'Axe'
        weapon_damage = union_weapon.Damage() // 5
    ```

=== "Rust"

    ```rust
    // Get and test the `Equipment` union (`equipped` field).
    // `equipped_as_weapon` returns a FlatBuffer handle much like normal table
    // fields, but this will return `None` if the union is not actually of that
    // type.
    if monster.equipped_type() == Equipment::Weapon {
        let equipped = monster.equipped_as_weapon().unwrap();
        let weapon_name = equipped.name();
        let weapon_damage = equipped.damage();
    ```

=== "Swift"

    ```swift
    // Get and check if the monster has an equipped item
    if monster.equippedType == .weapon {
        let _weapon = monster.equipped(type: Weapon.self)
        let name = _weapon.name // should return "Axe"
        let dmg = _weapon.damage // should return 5
    }
    ```

=== "TypeScript"

    ```ts
    let unionType = monster.equippedType();

    if (unionType == MyGame.Sample.Equipment.Weapon) {
        // 'Axe'
        let weaponName = monster.equipped(new MyGame.Sample.Weapon()).name();
        // 5
        let weaponDamage = monster.equipped(new MyGame.Sample.Weapon()).damage();
    }
    ```
