#![feature(plugin)]
#![plugin(flatbuffers_macros)]

extern crate flatbuffers_macros;

flatbuffers_object!{Table => Monster [
    field => { typeOf = pos }
    //~^ Error: Expected default, name, slot, and typeOf, missing default, name, and slot
]}

fn main() {}
