# Tests for Annotated Binaries

## Invalid Binary Tests

The following is a collection of manually corrupted binaries based off of
`..\annotated_binary.bin`. Each file changes some offset or length/size entry to
point so an invalid spot, and the generated annotated binaries demonstrate that
those corruptions can be spotted.

Each of these files were ran with the following command:

```sh
cd .tests/annotated_binary
../../flatc -annotate annotated_binary.fbs tests/{binary_file}...
```

### `invalid_root_offset.bin`

Changed first two bytes from `4400` to `FFFF` which produces an offset larger
than the binary.

### `invalid_root_table_vtable_offset.bin`

Changed two bytes at 0x0044 from `3A00` to `FFFF` which points to an offset
outside the binary.

### `invalid_root_table_too_short.bin`

Truncated the file to 0x46 bytes, as that cuts into the vtable offset field of
the root table.

```sh
truncate annotated_binary.bin --size=70 >> invalid_root_table_too_short.bin
```