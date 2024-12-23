## EBNF

```ebnf
schema = include* ( namespace_decl | type_decl | enum_decl | root_decl |
           file_extension_decl | file_identifier_decl |
           attribute_decl | rpc_decl | object )*

include = `include` string_constant `;`

namespace_decl = `namespace` ident ( `.` ident )* `;`

attribute_decl = `attribute` ident | `"` ident `"` `;`

type_decl = ( `table` | `struct` ) ident metadata `{` field_decl+ `}`

enum_decl = ( `enum` ident `:` type | `union` ident )  metadata `{`
commasep( enumval_decl ) `}`

root_decl = `root_type` ident `;`

field_decl = ident `:` type [ `=` scalar ] metadata `;`

rpc_decl = `rpc_service` ident `{` rpc_method+ `}`

rpc_method = ident `(` ident `)` `:` ident metadata `;`

type = `bool` | `byte` | `ubyte` | `short` | `ushort` | `int` | `uint` |
       `float` | `long` | `ulong` | `double` | `int8` | `uint8` | `int16` |
       `uint16` | `int32` | `uint32`| `int64` | `uint64` | `float32` |
       `float64` | `string` | `[` type `]` | ident

enumval_decl = ident [ `=` integer_constant ] metadata

metadata = [ `(` commasep( ident [ `:` single_value ] ) `)` ]

scalar = boolean_constant | integer_constant | float_constant

object = `{` commasep( ident `:` value ) `}`

single_value = scalar | string_constant

value = single_value | object | `[` commasep( value ) `]`

commasep(x) = [ x ( `,` x )\* ]

file_extension_decl = `file_extension` string_constant `;`

file_identifier_decl = `file_identifier` string_constant `;`

string_constant = `\".*?\"`

ident = `[a-zA-Z_][a-zA-Z0-9_]*`

`[:digit:]` = `[0-9]`

`[:xdigit:]` = `[0-9a-fA-F]`

dec_integer_constant = `[-+]?[:digit:]+`

hex_integer_constant = `[-+]?0[xX][:xdigit:]+`

integer_constant = dec_integer_constant | hex_integer_constant

dec_float_constant = `[-+]?(([.][:digit:]+)|([:digit:]+[.][:digit:]*)|([:digit:]+))([eE][-+]?[:digit:]+)?`

hex_float_constant = `[-+]?0[xX](([.][:xdigit:]+)|([:xdigit:]+[.][:xdigit:]*)|([:xdigit:]+))([pP][-+]?[:digit:]+)`

special_float_constant = `[-+]?(nan|inf|infinity)`

float_constant = dec_float_constant | hex_float_constant | special_float_constant

boolean_constant = `true` | `false`
```
