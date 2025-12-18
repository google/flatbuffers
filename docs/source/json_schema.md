# JSON Schema

`flatc` can generate [JSON Schema](https://json-schema.org/) for a FlatBuffers schema, and it can also import that JSON Schema back into FlatBuffers' schema IR.

## Generate JSON Schema

Generate `*.schema.json` from `*.fbs`:

```sh
flatc --jsonschema -o out_dir schema.fbs
```

This produces `out_dir/schema.schema.json`.

## Import JSON Schema

You can use a `*.schema.json` file anywhere `flatc` expects a schema file:

```sh
flatc --cpp -o out_dir schema.schema.json
```

This is primarily intended for round-tripping FlatBuffers schemas through JSON Schema tooling, and for downstream workflows that treat JSON Schema as the schema “source of truth”.

## `x-flatbuffers` metadata (optional)

JSON Schema cannot represent all FlatBuffers schema semantics (for example: struct vs table, exact scalar widths, union type mapping, field ids, and presence rules). To enable lossless round-trips, `flatc` can emit an optional vendor extension:

```sh
flatc --jsonschema --jsonschema-xflatbuffers -o out_dir schema.fbs
```

This adds `x-flatbuffers` objects at:

- The schema root (`root_type`, `file_identifier`, `file_extension`)
- Each definition (enum/union/table/struct metadata)
- Each field (exact FlatBuffers type + attributes)

Because `x-flatbuffers` uses the standard vendor-extension mechanism (`x-...`), most JSON Schema tools will ignore it and continue to work normally (for example QuickType and similar code generators).

### `x-flatbuffers` meta-schema

The allowed keys/values for the `x-flatbuffers` objects are described by:

- `docs/source/schemas/x-flatbuffers.schema.json:1`

This file is a meta-schema for the `x-flatbuffers` vendor extension itself (not a replacement for the JSON Schema metaschema).

