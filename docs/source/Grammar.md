# Formal Grammar of the schema language

schema = include*
         ( namespace\_decl | type\_decl | enum\_decl | root\_decl |
           file_extension_decl | file_identifier_decl |
           attribute\_decl | object )*

include = `include` string\_constant `;`

namespace\_decl = `namespace` ident ( `.` ident )* `;`

attribute\_decl = `attribute` string\_constant `;`

type\_decl = ( `table` | `struct` ) ident metadata `{` field\_decl+ `}`

enum\_decl = ( `enum` | `union` ) ident [ `:` type ] metadata `{` commasep(
enumval\_decl ) `}`

root\_decl = `root_type` ident `;`

field\_decl = ident `:` type [ `=` scalar ] metadata `;`

type = `bool` | `byte` | `ubyte` | `short` | `ushort` | `int` | `uint` |
`float` | `long` | `ulong` | `double`
 | `string` | `[` type `]` | ident

enumval\_decl = ident [ `=` integer\_constant ]

metadata = [ `(` commasep( ident [ `:` scalar ] ) `)` ]

scalar = integer\_constant | float\_constant | `true` | `false`

object = { commasep( ident `:` value ) }

value = scalar | object | string\_constant | `[` commasep( value ) `]`

commasep(x) = [ x ( `,` x )\* ]

file_extension_decl = `file_extension` string\_constant `;`

file_identifier_decl = `file_identifier` string\_constant `;`

