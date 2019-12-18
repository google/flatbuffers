// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Read what you wrote.
use crate::*;
use quickcheck;
use serde::{Deserialize, Serialize};

// TODO(cneo): Upstream this to the quickcheck crate. Also, write a macro to derive Arbitrary.
#[derive(Clone, PartialEq, Eq, PartialOrd, Ord, Debug)]
struct CString(std::ffi::CString);
impl quickcheck::Arbitrary for CString {
    fn arbitrary<G: quickcheck::Gen>(g: &mut G) -> Self {
        let size = std::cmp::min(2048, usize::arbitrary(g));
        let mut vec = Vec::with_capacity(size);
        while vec.len() < size {
            let c = u8::arbitrary(g);
            if c != b'\0' {
                vec.push(c);
            }
        }
        // We checked for null byte ourselves.
        CString(unsafe { std::ffi::CString::from_vec_unchecked(vec) })
    }
}

quickcheck! {
    fn qc_vec_bool(xs: Vec<bool>) -> bool {
        let mut builder = Builder::default();
        let mut v = builder.build_vector();
        for &x in &xs {
            v.push(x);
        }
        v.end();
        let r = Reader::get_root(&builder.view()).unwrap().as_vector();
        xs.iter().enumerate().all(|(i, &x)| r.index(i).unwrap().get_bool().unwrap() == x)
    }
    fn qc_vec_uint(xs: Vec<u64>) -> bool {
        let mut builder = Builder::default();
        let mut v = builder.build_vector();
        for &x in &xs {
            v.push(x);
        }
        v.end();
        let r = Reader::get_root(&builder.view()).unwrap().as_vector();
        xs.iter().enumerate().all(|(i, &x)| r.idx(i).as_u64() == x)
    }
    fn qc_vec_int(xs: Vec<i64>) -> bool {
        let mut builder = Builder::default();
        let mut v = builder.build_vector();
        for &x in &xs {
            v.push(x);
        }
        v.end();
        let r = Reader::get_root(&builder.view()).unwrap().as_vector();
        xs.iter().enumerate().all(|(i, &x)| r.idx(i).as_i64() == x)
    }
    fn qc_vec_float(xs: Vec<f64>) -> bool {
        let mut builder = Builder::default();
        let mut v = builder.build_vector();
        for &x in &xs {
            v.push(x);
        }
        v.end();
        let r = Reader::get_root(&builder.view()).unwrap().as_vector();
        xs.iter().enumerate().all(|(i, &x)| (r.idx(i).as_f64() - x).abs() < std::f64::EPSILON)
    }
    fn qc_vec_string(xs: Vec<String>) -> bool {
        let mut builder = Builder::default();
        let mut v = builder.build_vector();
        for x in &xs {
            v.push(x as &str);
        }
        v.end();
        let r = Reader::get_root(&builder.view()).unwrap().as_vector();
        xs.iter().enumerate().all(|(i, x)| (r.idx(i).as_str() == x))
    }
    // We use CString since Flexbuffer keys are more or less Cstrings despite the API taking str.
    fn qc_map_int(xs: std::collections::BTreeMap<CString, i64>) -> bool {
        let mut builder = Builder::default();
        let mut m = builder.build_map();
        for (k, &v) in &xs {
            m.push(&k.0.to_str().unwrap(), v);
        }
        m.end();
        let r = Reader::get_root(&builder.view()).unwrap().as_map();
        xs.iter().enumerate().all(|(i, (k, &v))| {
            r.idx(i).as_i64() == v && r.idx(k.0.to_str().unwrap()).as_i64() == v
        })
    }
    // We use CString since Flexbuffer keys are more or less Cstrings despite the API taking str.
    fn qc_map_string(xs: std::collections::BTreeMap<CString, String>) -> bool {
        let mut builder = Builder::default();
        let mut m = builder.build_map();
        for (k, v) in &xs {
            m.push(&k.0.to_str().unwrap(), v as &str);
        }
        m.end();
        let r = Reader::get_root(&builder.view()).unwrap().as_map();
        xs.iter().enumerate().all(|(i, (k, v))| {
            r.idx(i).as_str() == v && r.idx(k.0.to_str().unwrap()).as_str() == v
        })
    }
    fn qc_blob(xs: Vec<Vec<u8>>) -> bool {
        let mut builder = Builder::default();
        let mut v = builder.build_vector();
        for x in &xs {
            v.push(Blob(&x));
        }
        v.end();
        let r = Reader::get_root(&builder.view()).unwrap().as_vector();
        xs.iter().enumerate().all(
            |(i, x)| r.idx(i).get_blob().unwrap().0.iter().eq(x.iter())
        )
    }
}

#[test]
fn string() {
    let mut builder = Builder::default();
    let mut v = builder.build_vector();
    v.push("foo");
    v.push("barrr");
    v.push("bazzzzzz");
    v.end();
    let r = Reader::get_root(&builder.view()).unwrap().as_vector();
    assert_eq!(r.idx(0).as_str(), "foo");
    assert_eq!(r.idx(1).as_str(), "barrr");
    assert_eq!(r.idx(2).as_str(), "bazzzzzz");
}

#[test]
fn store_13() {
    let finished = singleton::<i32>(13);
    let r = Reader::get_root(&finished).unwrap();
    assert_eq!(r.as_i32(), 13);
}
#[test]
fn singleton_vector_uint_4_16bit() {
    let mut builder = Builder::default();
    let mut v = builder.build_vector();
    v.push(2u8);
    v.push(3u8);
    v.push(5u8);
    v.end();
    let buf1 = builder.view();
    let buf2 = singleton(&[2u8, 3, 5]);
    assert_eq!(buf1, buf2.as_slice());

    let r = Reader::get_root(&buf1).unwrap().as_vector();
    assert_eq!(r.idx(0).get_u8(), Ok(2u8));
    assert_eq!(r.idx(1).get_u8(), Ok(3u8));
    assert_eq!(r.idx(2).get_u8(), Ok(5u8));
    assert_eq!(r.index(3).unwrap_err(), reader::Error::IndexOutOfBounds);
}
#[test]
fn vector_uint4() {
    let mut fxb = Builder::default();
    let mut v = fxb.build_vector();
    v.push(2u8);
    v.push(3u8);
    v.push(5u8);
    v.push(7u8);
    v.end();
    let r = Reader::get_root(&fxb.view()).unwrap();
    let v = r.as_vector();
    assert_eq!(v.idx(0).get_u8(), Ok(2u8));
    assert_eq!(v.idx(1).get_u8(), Ok(3u8));
    assert_eq!(v.idx(2).get_u8(), Ok(5u8));
    assert_eq!(v.idx(3).get_u8(), Ok(7u8));
    assert!(v.index(4).is_err());
    assert_eq!(r.get_u8s().unwrap(), [2, 3, 5, 7]);
}
#[test]
fn store_and_read_blob() {
    let mut fxb = Builder::default();
    let mut v = fxb.build_vector();
    v.push(Blob(&[1, 2, 3, 4]));
    v.push(Blob(&[5, 6, 7]));
    v.end();

    let r = Reader::get_root(&fxb.view()).unwrap().as_vector();
    assert_eq!(r.idx(0).get_blob(), Ok(Blob(&[1, 2, 3, 4])));
    assert_eq!(r.idx(1).get_blob(), Ok(Blob(&[5, 6, 7])));
}
#[test]
fn map_64bit() {
    let mut fxb = Builder::default();
    let mut m = fxb.build_map();
    m.push("a", 257u16);
    m.push("b", u64::max_value() - 3);
    m.end();

    let r = Reader::get_root(&fxb.view()).unwrap().as_map();
    assert_eq!(r.idx("a").as_u16(), 257);
    assert_eq!(r.idx("b").as_u64(), u64::max_value() - 3);
}
#[test]
fn index_map() {
    let mut fxb = Builder::default();
    let mut m = fxb.build_map();
    m.push("foo", 17u8);
    m.push("bar", 33u16);
    m.push("baz", 41u32);
    m.end();

    let r = Reader::get_root(fxb.view()).unwrap().as_map();
    assert_eq!(r.idx(0).get_u8(), Ok(33u8));
    assert_eq!(r.idx(1).get_u8(), Ok(41u8));
    assert_eq!(r.idx(2).as_u8(), 17);
    assert_eq!(r.index(3).unwrap_err(), reader::Error::IndexOutOfBounds);

    assert_eq!(r.idx("bar").as_u64(), 33);
    assert_eq!(r.idx("baz").as_u32(), 41);
    assert_eq!(r.idx("foo").as_u16(), 17);
    assert_eq!(r.index("???").unwrap_err(), reader::Error::KeyNotFound);
}

#[test]
fn map_strings() {
    let mut fxb = Builder::default();
    {
        let mut m = fxb.build_map();
        let mut a = m.nest_vector("a");
        for &s in ["b", "c", "d", "e"].iter() {
            a.push(s);
        }
        a.end();
        let mut f = m.nest_vector("f");
        for &s in ["gh", "ij"].iter() {
            f.push(s);
        }
    }
    let r = Reader::get_root(fxb.view()).unwrap().as_map();
    let a = r.idx("a").as_vector();

    assert_eq!(a.idx(0).as_str(), "b");
    assert_eq!(a.idx(1).as_str(), "c");
    assert_eq!(a.idx(2).as_str(), "d");
    assert_eq!(a.idx(3).as_str(), "e");

    let f = r.idx("f").as_vector();
    assert_eq!(f.idx(0).as_str(), "gh");
    assert_eq!(f.idx(1).as_str(), "ij");

    // Defaults to empty string for index errors.
    assert_eq!(r.idx("a").as_vector().idx(4).as_str(), "");
    assert_eq!(r.idx("b").as_vector().idx(2).as_str(), "");
    assert_eq!(r.idx("c").as_str(), "");
}

#[test]
fn store_u64() {
    let finished = singleton(u64::max_value() - 10);
    let r = Reader::get_root(&finished).unwrap();
    assert_eq!(r.get_u64(), Ok(u64::max_value() - 10));
}
#[test]
fn store_indirects() {
    let mut b = Builder::new();
    let mut v = b.build_vector();
    v.push(IndirectInt(-42));
    v.push(IndirectUInt(9000));
    v.push(IndirectFloat(3.14));
    v.end();
    let r = Reader::get_root(b.view()).unwrap().as_vector();
    assert_eq!(r.idx(0).get_i8().unwrap(), -42);
    assert_eq!(r.idx(1).get_u16().unwrap(), 9000);
    assert_eq!(r.idx(2).get_f64().unwrap(), 3.14);
}

#[derive(Serialize, Deserialize, Debug, PartialEq)]
struct Foo {
    a: i8,
    b: f64,
    c: Vec<u32>,
    d: String,
}
quickcheck! {
    fn serde_foo(a: i8,
    b: f64,
    c: Vec<u32>,
    d: String) -> bool {
        let mut s = crate::FlexbufferSerializer::new();
        let data = Foo { a, b, c, d };
        data.serialize(&mut s).unwrap();

        let read = Foo::deserialize(Reader::get_root(s.view()).unwrap()).unwrap();
        data == read
    }
}

#[test]
fn serde_serious() {
    #[derive(Debug, PartialEq, Serialize, Deserialize)]
    enum MyEnum {
        Unit,
        NewType([i32; 3]),
        Tuple(f32, f64),
        Struct { a: u8, b: u16, c: u32 },
    }
    #[derive(Debug, PartialEq, Serialize, Deserialize)]
    struct MyNewType;

    #[derive(Debug, PartialEq, Serialize, Deserialize)]
    struct MyStruct {
        a: u8,
        b: u16,
        c: u32,
        d: u64,
    };

    #[derive(Debug, PartialEq, Serialize, Deserialize)]
    struct MyUnitStruct(Vec<String>);

    #[derive(Debug, PartialEq, Serialize, Deserialize)]
    struct MyTupleStruct(MyNewType, MyUnitStruct, MyStruct, Vec<MyEnum>);

    let data = MyTupleStruct(
        MyNewType,
        MyUnitStruct(vec!["Hello".to_string(), "World".to_string()]),
        MyStruct {
            a: 2,
            b: 4,
            c: 8,
            d: 16,
        },
        vec![
            MyEnum::Unit,
            MyEnum::NewType([-1, 0, 1]),
            MyEnum::Unit,
            MyEnum::Tuple(3.14, 2.71),
            MyEnum::Struct {
                a: 32,
                b: 64,
                c: 128,
            },
        ],
    );

    let mut s = crate::FlexbufferSerializer::new();
    data.serialize(&mut s).unwrap();

    let reader = Reader::get_root(s.view()).unwrap();
    let read = MyTupleStruct::deserialize(reader).unwrap();
    assert_eq!(data, read);
}
