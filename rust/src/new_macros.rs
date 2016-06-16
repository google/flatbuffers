//! Macro for defining flatbuffer Tables, Structs, Enums and Unions.
//!
//! The `flatbuffers_object!` macro is used to create the necessary
//! biolerplate for using flatbuffers. The recommended use is to
//! generate these macros using the `flatc --rust` command.

//! # Example Enums
//!
//! 
use types::*;

#[macro_export]
macro_rules! flatbuffers_object {
    // Table slot accessor functions
    (@table_accessor $i:ident [$ty:ty] $slot:expr,) => {
        ($i.0).get_slot_vector::<$ty>($slot)
    };
    (@table_accessor $i:ident &str $slot:expr,) => {
        ($i.0).get_slot_str($slot)
    };
    // enum
    (@table_accessor $i:ident $ty:ident($si:ident) $slot:expr, $default:expr) => {
        let v = flatbuffers_object!(@table_accessor $i $si $slot, $default);
        $ty::from(v)
    };
    // Union
    (@table_accessor $i:ident Union($ty:ident = $fi:ident) $slot:expr, $default:expr) => {
        let ty = $i.$fi();
        let table =  ($i.0).get_slot_table($slot)
        $ty::new(table, ty)
    };
    (@table_accessor $i:ident u8 $slot:expr, $default:expr) => {
        ($i.0).get_slot_u8($slot, $default)
    };
    (@table_accessor $i:ident i8 $slot:expr, $default:expr) => {
        ($i.0).get_slot_i8($slot, $default)
    };
    (@table_accessor $i:ident u16 $slot:expr, $default:expr) => {
        ($i.0).get_slot_u16($slot, $default)
    };
    (@table_accessor $i:ident i16 $slot:expr, $default:expr) => {
        ($i.0).get_slot_i16($slot, $default)
    };
    (@table_accessor $i:ident u32 $slot:expr, $default:expr) => {
        ($i.0).get_slot_u32($slot, $default)
    };
    (@table_accessor $i:ident i32 $slot:expr, $default:expr) => {
        ($i.0).get_slot_i32($slot, $default)
    };
    (@table_accessor $i:ident u64 $slot:expr, $default:expr) => {
        ($i.0).get_slot_u64($slot, $default)
    };
    (@table_accessor $i:ident i64 $slot:expr, $default:expr) => {
        ($i.0).get_slot_i64($slot, $default)
    };
    (@table_accessor $i:ident f32 $slot:expr, $default:expr) => {
        ($i.0).get_slot_f32($slot, $default)
    };
    (@table_accessor $i:ident f64 $slot:expr, $default:expr) => {
        ($i.0).get_slot_f64($slot, $default)
    };
    (@table_accessor $i:ident bool $slot:expr, $default:expr) => {
        ($i.0).get_slot_bool($slot, $default)
    };
    (@table_accessor $i:ident $ty:ident $slot:expr,) => {
        ($i.0).get_slot_struct($slot)
    };
    // Struct slot accessor functions
    (@table_accessor $i:ident &str $slot:expr,) => {
        ($i.0).get_str($slot)
    };
    (@struct_accessor $i:ident u8 $slot:expr) => {
        ($i.0).get_u8($slot)
    };
    (@struct_accessor $i:ident i8) $slot:expr => {
        ($i.0).get_i8($slot)
    };
    (@struct_accessor $i:ident u16 $slot:expr) => {
        ($i.0).get_u16($slot)
    };
    (@struct_accessor $i:ident i16 $slot:expr) => {
        ($i.0).get_i16($slot)
    };
    (@struct_accessor $i:ident u32 $slot:expr) => {
        ($i.0).get_u32($slot)
    };
    (@struct_accessor $i:ident i32 $slot:expr) => {
        ($i.0).get_i32($slot)
    };
    (@struct_accessor $i:ident u64 $slot:expr) => {
        ($i.0).get_u64($slot)
    };
    (@struct_accessor $i:ident i64 $slot:expr) => {
        ($i.0).get_i64($slot)
    };
    (@struct_accessor $i:ident f32 $slot:expr) => {
        ($i.0).get_f32($slot)
    };
    (@struct_accessor $i:ident f64 $slot:expr) => {
        ($i.0).get_f64($slot)
    };
    (@struct_accessor $i:ident bool $slot:expr) => {
        ($i.0).get_bool($slot)
    };
    (@struct_accessor $i:ident $ty:tt $slot:expr,) => {
        ($i.0).get_struct($slot)
    };
    // enum
    (@struct_accessor $i:ident $ty:ident($si:ident) $slot:expr, $default:expr) => {
        let v = flatbuffers_object!(@table_accessor $i $si $slot, $default);
        $ty::from(v)
    };
    // For each @field build 
    //  1. table accessor functions,
    //   2. builder trait definition,
    //   3. builder functions
    // and then recurse until no more fields.
    // (@field { field {  } }, ($($table:tt)*), ($($traits:tt)*), ($($builder:tt)*))
    // => {
    // flatbuffers_object!{@field ($($table:tt)*), ($($traits:tt)*),   ($($builder:tt)*)}
    //  };
    (@table_fun { }) => {};
    (@table_fun { field => }) => {};
    (@table_fun { field => { name = $name:ident,
                             typeOf = $ret_ty:ty,
                             slot = $slot:expr
                             $(, default = $default:expr)*
                             $(, comment = $comment:expr)*}
                  $(, field => $body:tt)* $(,)*}) => {
        $( #[doc = $comment] )*
        fn $name(&self) -> $ret_ty {
            flatbuffers_object!(@table_accessor self $ret_ty $slot, $($default)*)
        }
        flatbuffers_object!(@table_fun { field => $($body)* });
    };
    (@struct_fun { }) => {};
    (@struct_fun { field => }) => {};
    (@struct_fun { field => { name = $name:ident,
                             typeOf = $ret_ty:ty,
                             slot = $slot:expr
                             $(, default = $default:expr)*
                             $(, comment = $comment:expr)*}
                  $(, field => $body:tt)* $(,)*}) => {
        $( #[doc = $comment] )*
            fn $name(&self) -> $ret_ty {
                flatbuffers_object!(@struct_accessor self $ret_ty $slot)
            }
        flatbuffers_object!(@struct_fun { field => $($body)* });
    };
    (@from_table $name:ident) => {
        impl<T: AsRef<[u8]>> From<$crate::Table<T>> for $name<T> {
            fn from(table: $crate::Table) -> $name {
                $name(table)
            }
        }
    }
    (@vector_type $name:ident $size:expr) => {
        impl<T: AsRef<[u8]>> VectorType for $name<T> {
            fn inline_size() -> usize {
                $size
            }

            fn read_next(buffer: &[u8]) -> $name {
                Table::get_indirect_root(buffer, 0).into();
            }
        }
    };
    // Table
    ($name:ident =>
     $body:tt ) => {

        #[derive(Debug, Clone, PartialEq, Eq)]
        pub struct $name<T: AsRef<[u8]>>($crate::Table<T>);

        impl<T: AsRef<[u8]>> $name<T> {
            flatbuffers_object!(@table_fun $body);
        }

        flatbuffers_object!(@from_table $name);

        flatbuffers_object!(@vector_type $name UOFFSETT_SIZE);

    };
    // Struct
    ($name:ident =>
     fixed_size = $inline_size:expr,
     $body:tt ) => {

        #[derive(Debug, Clone, PartialEq, Eq)]
        pub struct $name<T: AsRef<[u8]>>($crate::Table<T>);

        impl<T: AsRef<[u8]>> $name<T> {
            flatbuffers_object!(@struct_fun $body);
        }

        flatbuffers_object!(@from_table $name);

        flatbuffers_object!(@vector_type $name $inline_size);
    };
    // Simple Enum
    ($name:ident => Enum {$( ($e_name:ident = $value:expr) ),+ }
     as $repr:ty) => {
        #[derive(PartialEq, Eq, Clone, Copy, Debug, Hash)]
        #[repr($repr)]
        pub enum $name {
            $( $e_name = $value ),+
        }

        impl $name {
            pub fn from(value: $repr) -> Option<$name> {
                match value {
                    $( $value => Some($name::$e_name) ),+,
                    _ => None
                }
            }
        }
    };
    // Union
    ($name:ident => $ty:ident {$( ($e_name:ident = $value:expr) ),+ }
     as $repr:ty) => {
        #[derive(Debug)]
        pub enum $name {
            None,
            $( $e_name( $ty ) ),+
        }

        impl $name {
            pub fn new(table: $crate::Table, utype: Option<$type_name>) -> Option<$name> {
                match utype {
                    $( Some($type_name::$e_name) => Some( $name::$e_name( table.into() ) ), )*
                        _ => None
                }
            }
        }

        flatbuffers_object!($ty => Enum {$( ($e_name = $value) ),+ } as  $repr);
    }
}

// use table::Table;

// pub struct Vec3;

// impl<'a> From<Table<'a>> for Vec3 {
//     fn from(table: Table) -> Vec3 {
//         Vec3
//     }
// }

// /// an example documentation comment: monster object
// flatbuffers_object!{Monster =>
//        { field => { name = pos,
//                     typeOf = Vec3,
//                     slot = 4,
//                     comment = "///Testing all my ways are dead\ntestomg"},
//          field => { name = mana,
//                     typeOf = i16,
//                     slot = 6,
//                     default = 150,
//                     comment = "///Testing all my ways are dead\ntestomg",
//                     comment = "///Testing all my ways are dead\ntestomg"},
//          field => { name = color,
//                     typeOf = Color(i8),
//                     slot = 6,
//                     default = 8,
//                     comment = "///Testing all my ways are dead\ntestomg",
//                     comment = "///Testing all my ways are dead\ntestomg"},
//          field => { name = test_type,
//                     typeOf = AnyType(u8),
//                     slot = 8,
//                     default = 0,
//                     comment = "///Testing all my ways are dead\ntestomg",
//                     comment = "///Testing all my ways are dead\ntestomg"},
//          field => { name = test,
//                     typeOf = Union(Any = test_type),
//                     slot = 10,
//                     default = 0,
//                     comment = "///Testing all my ways are dead\ntestomg",
//                     comment = "///Testing all my ways are dead\ntestomg"},
//        }
// }

// flatbuffers_object!{Color => Enum{ Red = 1, Blue = 2} as i8}

// flatbuffers_object!{Any => AnyType{   } as u8}
