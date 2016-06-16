#![feature(plugin)]
#![plugin(flatbuffers_macros)]

extern crate flatbuffers_macros;

flatbuffers_object!{Monster, []}
//~^ ERROR: Expected one of 'Table', 'Struct', 'Enum' or 'Union'

fn main() {}
