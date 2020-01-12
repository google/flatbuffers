use super::rwyw::NonNullString;
use quickcheck::{Arbitrary, Gen};
use serde::{Deserialize, Serialize};
use std::collections::BTreeMap;

#[derive(Debug, Clone, Arbitrary, PartialEq, Serialize, Deserialize)]
enum Enum {
    Unit,
    U8(u8),
    U16(u16),
    U32(u32),
    U64(u64),
    Us(u8, u16, u32, u64),
    I8(i8),
    I16(i16),
    I32(i32),
    I64(i64),
    Is(i8, i16, i32, i64),
    F32(f32),
    F64(f64),
    Fs(f32, f64),
    String(String),
    Strings(String, String),
    Everything(u8, u16, u32, u64, i8, i16, i32, i64, f32, f64, String),
    Arrays {
        a: Array3<u16>,
        b: Array4<i32>,
        c: Array2<f64>,
    },
}

#[derive(Debug, Clone, Arbitrary, PartialEq, Serialize, Deserialize)]
struct Unit;

#[derive(Debug, Clone, Arbitrary, PartialEq, Serialize, Deserialize)]
struct NewType(bool);

#[derive(Debug, Clone, Arbitrary, PartialEq, Serialize, Deserialize)]
struct Tuple(bool, u8, i16, f32, String);

#[derive(Debug, Clone, Arbitrary, PartialEq, Serialize, Deserialize)]
struct Struct {
    a: Vec<Enum>,
    b: BTreeMap<NonNullString, Enum>,
    c: Tuple,
    d: (Unit, Unit),
    e: Array4<NewType>,
}

#[derive(Debug, Clone, PartialOrd, Ord, PartialEq, Eq, Serialize, Deserialize)]
struct Array2<A: Arbitrary>([A; 2]);
#[derive(Debug, Clone, PartialOrd, Ord, PartialEq, Eq, Serialize, Deserialize)]
struct Array3<A: Arbitrary>([A; 3]);
#[derive(Debug, Clone, PartialOrd, Ord, PartialEq, Eq, Serialize, Deserialize)]
struct Array4<A: Arbitrary>([A; 4]);

impl<A: Arbitrary> Arbitrary for Array2<A> {
    fn arbitrary<G: Gen>(g: &mut G) -> Self {
        Array2([A::arbitrary(g), A::arbitrary(g)])
    }
}
impl<A: Arbitrary> Arbitrary for Array3<A> {
    fn arbitrary<G: Gen>(g: &mut G) -> Self {
        Array3([A::arbitrary(g), A::arbitrary(g), A::arbitrary(g)])
    }
}
impl<A: Arbitrary> Arbitrary for Array4<A> {
    fn arbitrary<G: Gen>(g: &mut G) -> Self {
        Array4([
            A::arbitrary(g),
            A::arbitrary(g),
            A::arbitrary(g),
            A::arbitrary(g),
        ])
    }
}

quickcheck! {
    fn qc_serious(x: Struct) -> bool {
        let mut s = crate::FlexbufferSerializer::new();
        x.serialize(&mut s).unwrap();
        let r = crate::Reader::get_root(s.view()).unwrap();
        println!("{}", r);
        let x2 = Struct::deserialize(r).unwrap();
        x == x2
    }
}
