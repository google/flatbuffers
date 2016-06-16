#![feature(plugin)]
#![plugin(flatbuffers_macros)]

extern crate flatbuffers_macros;

flatbuffers_object!{Table => Monster [
    field => {}
    //~^ Error: Invalid field definition
]}

fn main() {}
