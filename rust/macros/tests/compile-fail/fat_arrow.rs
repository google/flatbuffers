#![feature(plugin)]
#![plugin(flatbuffers_macros)]

extern crate flatbuffers_macros;

flatbuffers_object!{Table}
//~^ ERROR: Expected '=>'

fn main() {}
