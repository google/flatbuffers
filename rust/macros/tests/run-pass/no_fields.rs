#![feature(plugin)]
#![plugin(flatbuffers_macros)]

extern crate flatbuffers_macros;
extern crate flatbuffers;

flatbuffers_object!{Table => Vec3 []}
//~^ WARNING: Expected a list of fields


flatbuffers_object!{Table => Monster}
//~^ WARNING: Expected a list of fields

fn main() {}
