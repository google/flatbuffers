#![feature(plugin)]
#![plugin(flatbuffers_macros)]

extern crate flatbuffers_macros;

flatbuffers_object!{Table => Monster [
    field => { slot = me }
    //~^ Error: Slot must be an integer
]}

fn main() {}
