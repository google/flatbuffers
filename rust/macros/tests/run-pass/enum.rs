#![feature(plugin)]
#![plugin(flatbuffers_macros)]

extern crate flatbuffers_macros;
extern crate flatbuffers;

flatbuffers_object!{Enum => Color {Red = 1, Blue = 2} as byte}

fn main() {
    let x = Color::Red;
    let y = Color::from(2).unwrap();
}
