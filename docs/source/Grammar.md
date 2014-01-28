# Formal Grammar of the schema language

schema = namespace\_decl | type\_decl | enum\_decl | root\_decl | object

namespace\_decl = `namespace` ident ( `.` ident )* `;`

type\_decl = ( `table` | `struct` ) ident metadata `{` field\_decl+ `}`

enum\_decl = ( `enum` | `union` ) ident [ `:` type ] metadata `{` commasep(
enumval\_decl ) `}`

root\_decl = `root_type` ident `;`

field\_decl = type `:` ident [ `=` scalar ] metadata `;`

type = `bool` | `byte` | `ubyte` | `short` | `ushort` | `int` | `uint` |
`float` | `long` | `ulong` | `double`
 | `string` | `[` type `]` | ident

enumval\_decl = ident [ `=` integer\_constant ]

metadata = [ `(` commasep( ident [ `:` scalar ] ) `)` ]

scalar = integer\_constant | float\_constant | `true` | `false`

object = { commasep( ident `:` value ) }

value = scalar | object | string\_constant | `[` commasep( value ) `]`

commasep(x) = [ x ( `,` x )\* ]
