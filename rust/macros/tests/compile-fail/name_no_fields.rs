#![feature(plugin)]
#![plugin(flatbuffers_macros)]

extern crate flatbuffers_macros;

flatbuffers_object!{Table => Monster,}
//~^ Error: Expected a list of fields

fn main() {}
