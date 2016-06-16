
#![feature(plugin)]
#![plugin(flatbuffers_macros)]

extern crate flatbuffers_macros;
extern crate flatbuffers;

flatbuffers_object!{Struct => Vec3 ( size:32 ) []}

flatbuffers_object!{Table => Monster []}

flatbuffers_object!{Union => Any { Monster = 1, Vec3 = 2 }}

fn main() {
}
