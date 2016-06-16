#![feature(plugin)]
#![plugin(flatbuffers_macros)]
extern crate flatbuffers_macros;

flatbuffers_object!{Table =>}
//~^ ERROR: Expected a name for the Table

fn main() {}
