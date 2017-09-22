#![feature(plugin)]
#![plugin(flatbuffers_macros)]

extern crate flatbuffers_macros;

flatbuffers_object!{Table => Monster [
    field => { name = pos,
               //~^ Error: Unknown attribute: wierd
               typeOf = Vec3,
               wierd = true,
               slot = 4 }
]}

fn main() {}
