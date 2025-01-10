# Evolution

FlatBuffers enables the [schema](schema.md) to evolve over time while still
maintaining forwards and backwards compatibility with old flatbuffers.

Some rules must be followed to ensure the evolution of a schema is valid.

## Rules

Adding new tables, vectors, structs to the schema is always allowed. Its only
when you add a new field to a [`table`](schema.md#tables) that certain rules
must be followed.

### Addition

**New fields MUST be added to the end of the table definition.**

This allows older data to still be read correctly (giving you the default value
of the added field if accessed).

Older code will simply ignore the new field in the flatbuffer.

You can ignore this rule if you use the `id` attribute on all the fields of a
table.

### Removal

**You MUST not remove a field from the schema, even if you don't use it
anymore.** You simply stop writing them to the buffer.

Its encouraged to mark the field deprecated by adding the `deprecated`
attribute. This will skip the generation of accessors and setters in the code,
to enforce the field not to be used any more.

### Name Changes

Its generally OK to change the name of tables and fields, as these are not
serialized to the buffer. It may break code that would have to be refactored
with the updated name.

## Examples

The following examples uses a base schema and attempts to evolve it a few times.
The versions are tracked by `V1`, `V2`, etc.. and `CodeV1` means code compiled
against the `V1` schema.

### Table Evolution

Lets start with a simple table `T` with two fields.

```c++ title="Schema V1"
table T {
  a:int;
  b:int;
}
```

=== "Well Evolved"

    First lets extend the table with a new field.

    ```c++ title="Schema V2"
    table T {
      a:int;
      b:int;
      c:int;
    }
    ```

    This is OK. `CodeV1` reading `V2` data will simply ignore the presence of the
    new field `c`. `CodeV2` reading `V1` data will get a default value (0) when
    reading `c`.

    ```c++ title="Schema V3"
    table T {
      a:int (deprecated);
      b:int;
      c:int;
    }
    ```

    This is OK, removing field `a` via deprecation. `CodeV1`, `CodeV2` and `CodeV3`
    reading `V3` data will now always get the default value of `a`, since it is not
    present. `CodeV3` cannot write `a` anymore. `CodeV3` reading old data (`V1` or
    `V2`) will not be able to access the field anymore, since no generated accessors
    are omitted.

=== "Improper Addition"

    Add a new field, but this time at the beginning.

    ```c++ title="Schema V2"
    table T {
      c:int;
      a:int;
      b:int;
    }
    ```

    This is NOT OK, as it makes `V2` incompatible. `CodeV1` reading `V2` data
    will access `a` but will read `c` data.

    `CodeV2` reading `V1` data will access `c` but will read `a` data.

=== "Improper Deletion"

    Remove a field from the schema.

    ```c++ title="Schema V2"
    table T {
      b:int;
    }
    ```

    This is NOT OK. `CodeV1` reading `V2` data will access `a` but read `b` data.

    `CodeV2` reading `V1` data will access `b` but will read `a` data.

=== "Proper Reordering"

    Lets add a new field to the beginning, but use `id` attributes.

    ```c++ title="Schema V2"
    table T {
      c:int (id: 2);
      a:int (id: 0);
      b:int (id: 1);
    }
    ```

    This is OK. This adds the a new field in the beginning, but because all the
    `id` attributes were added, it is OK.

=== "Changing Types"

    Let change the types of the fields.

    ```c++ title="Schema V2"
    table T {
      a:uint;
      b:uint;
    }
    ```

    This is MAYBE OK, and only in the case where the type change is the same
    width. This is tricky if the `V1` data contained any negative numbers. So
    this should be done with care.

=== "Changing Defaults"

    Lets change the default values of the existing fields.

    ```c++ title="Schema V2"
    table T {
      a:int = 1;
      b:int = 2;
    }
    ```

    This is NOT OK. Any `V1` data that did not have a value written to the
    buffer relied on generated code to provide the default value.

    There MAY be cases where this is OK, if you control all the producers and
    consumers, and you can update them in tandem.

=== "Renaming Fields"

    Lets change the name of the fields

    ```c++ title="Schema V2"
    table T {
      aa:int;
      bb:int;
    }
    ```

    This is generally OK. You've renamed fields will break all code and JSON
    files that use this schema, but you can refactor those without affecting the
    binary data, since the binary only address fields by id and offset, not by
    names.

### Union Evolution

Lets start with a simple union `U` with two members.

```c++ title="Schema V1"
union U {
  A,
  B
}
```

=== "Well Evolved"

    Lets add a another variant to the end.

    ```c++ title="Schema V2"
    union U {
      A,
      B,
      another_a: A
    }
    ```

    This is OK. `CodeV1` will not recognize the `another_a`.

=== "Improper Evolved"

    Lets add a another variant to the middle.

    ```c++ title="Schema V2"
    union U {
      A,
      another_a: A,
      B
    }
    ```

    This is NOT OK. `CodeV1` reading `V2` data will interpret `B` as `another_a`.
    `CodeV2` reading `V1` data will interpret `another_a` as `B`.

=== "Evolved With Discriminant"

    Lets add a another variant to the middle, this time adding a union "discriminant".

    ```c++ title="Schema V2"
    union U {
      A = 1,
      another_a: A = 3,
      B = 2
    }
    ```

    This is OK. Its like you added it to the end, but using the discriminant
    value to physically place it elsewhere in the union.

## Version Control

FlatBuffers relies on new field declarations being added at the end, and earlier
declarations to not be removed, but be marked deprecated when needed. We think
this is an improvement over the manual number assignment that happens in
Protocol Buffers (and which is still an option using the `id` attribute
mentioned above).

One place where this is possibly problematic however is source control. If user
`A` adds a field, generates new binary data with this new schema, then tries to
commit both to source control after user `B` already committed a new field also,
and just auto-merges the schema, the binary files are now invalid compared to
the new schema.

The solution of course is that you should not be generating binary data before
your schema changes have been committed, ensuring consistency with the rest of
the world. If this is not practical for you, use explicit field `id`s, which
should always generate a merge conflict if two people try to allocate the same
id.
