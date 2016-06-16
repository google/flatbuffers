#![feature(plugin)]
#![plugin(flatbuffers_macros)]

extern crate flatbuffers_macros;
extern crate flatbuffers;

flatbuffers_object!{Enum => Color {Red = 1, Blue = 2} as ubyte}

flatbuffers_object!{Struct => Vec3 ( size:32 ) [
    field => { name = x,
               typeOf = float,
               slot = 0,
               default = 0.0 }, 
    field => { name = y,
               typeOf = float,
               slot = 4,
               default = 0.0 }, 
    field => { name = z,
               typeOf = float,
               slot = 8,
               default = 0.0 }, 
    field => { name = test1,
               typeOf = double,
               slot = 16,
               default = 0.0 }, 
    field => { name = test2,
               typeOf = enum Color ubyte,
               slot = 24,
               default = 0 }, 
    field => { name = test3,
               typeOf = Vec3,
               slot = 26 }]}

fn main() {}
