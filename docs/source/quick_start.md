# Quick Start

This will quickly go over the parts of using FlatBuffers to serialize some data.
See the [Tutorial](tutorial.md) for a more in depth guide.

1. **Build the compiler for FlatBuffers ([`flatc`](flatc.md))**

    ```sh
    cmake -G "Unix Makefiles"
    make -j
    ```

2. **Define your FlatBuffer [schema](schema.md) (`.fbs`)**

    ```c title="monster.fbs" linenums="1"
    table Monster {
      name:string;
      health:int;
    }

    root_type Monster;
    ```

    See [monster.fbs](https://github.com/google/flatbuffers/blob/master/samples/monster.fbs)
    for an complete example.

3. **Generate code for your language(s)**

    Use the `flatc` compiler to take your schema and generate language-specific
    code:

    ```sh
    ./flatc --cpp --rust mosnter.fbs
    ```

    Which generates `monster_generated.h` and `monster_generated.rs` files.

4. **Serialize data**

    Use the generated code files, as well as the `FlatBufferBuilder` to construct
    your serialized buffer.

    ```c++ title="my_monster_factory.cc" linenums="1"
    #include "flatbuffers.h"
    #include "monster_generated.h"

    int main() { 
      // Used to build the flatbuffer
      FlatBufferBuilder builder;

      // Auto-generated function emitted from `flatc` and the input 
      // `monster.fbs` schema.
      auto monster = CreateMonsterDirect(builder, "Abominable Snowman", 100);

      // Finalize the buffer.
      builder.Finish(monster);
    }
    ```

    See complete [C++ Example](https://github.com/google/flatbuffers/blob/master/samples/sample_binary.cpp#L24-L56).

5.  **Transmit/Store the serialized FlatBuffer**
  
    Use your serialized buffer however you want. Send it to someone, save if for
    later, etc...

    ```c++ title="my_monster_factory.cc" linenums="13"
    // Get a pointer to the flatbuffer.
    const uint8_t* flatbuffer = builder.GetBufferPointer();
    ```

6.  **Read the data**

    Use the generated accessors to read the data from the serialized buffer.

    It doesn't need to be the same language, or even schema version (see 
    [Evolving](evolution.md)), FlatBuffers ensures the data is readable across
    languages and schema versions. 
    
    ```c++ title="my_monster_factory.cc" linenums="15"
    // Get a view of the root monster from the flatbuffer.
    const Monster snowman = GetMonster(flatbuffer);

    // Access the monster's fields directly.
    ASSERT_EQ(snowman.name(), "Abominable Snowman");
    ASSERT_EQ(snowman.health(), 100);
    ```    
    
    See [`Rust` examples](https://github.com/google/flatbuffers/blob/master/samples/sample_binary.rs#L92-L106)
    for reading the data written by `C++`.
