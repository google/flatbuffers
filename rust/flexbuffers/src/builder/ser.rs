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

use crate::Builder;
use serde::ser;
use serde::ser::*;
use std::fmt::Display;

// This struct internally tracks the nested vectors representing
// nested structs and such.
// TODO: Add an option field names in a map.
// TODO: Rename this to something better.
/// Flexbuffer Serializer. This should be used to serialize structs.
#[derive(Debug, Default)]
pub struct FlexbufferSerializer {
    builder: Builder,
    nesting: Vec<Option<usize>>,
}
impl FlexbufferSerializer {
    pub fn new() -> Self {
        Self::default()
    }
    pub fn view(&self) -> &[u8] {
        self.builder.view()
    }
    fn finish_if_not_nested(&mut self) -> Result<(), Error> {
        if self.nesting.is_empty() {
            self.unnest()
        } else {
            Ok(())
        }
    }
    fn unnest(&mut self) -> Result<(), Error> {
        match self.nesting.pop() {
            // Singleton root.
            None => {
                let root = self.builder.values.pop().unwrap();
                super::store_root(&mut self.builder.buffer, root);
            }
            // Nested vector.
            Some(previous_end) => {
                self.builder.end_map_or_vector(false, previous_end);
            }
        }
        Ok(())
    }
    fn nest(&mut self) {
        if self.builder.values.is_empty() {
            // The root is a vector.
            self.nesting.push(None);
        } else {
            // This is a nested vector.
            self.nesting.push(Some(self.builder.values.len()));
        }
    }
}

// TODO: What actual errors will we have?
#[derive(Debug)]
pub struct Error(String);

impl std::fmt::Display for Error {
    fn fmt(&self, _: &mut std::fmt::Formatter) -> Result<(), std::fmt::Error> {
        unimplemented!()
    }
}
impl std::error::Error for Error {}
impl ser::Error for Error {
    fn custom<T>(msg: T) -> Self
    where
        T: Display,
    {
        Error(format!("{}", msg))
    }
}
impl<'a> ser::SerializeSeq for &mut FlexbufferSerializer {
    type Ok = ();
    type Error = Error;
    fn serialize_element<T: ?Sized>(&mut self, value: &T) -> Result<(), Self::Error>
    where
        T: Serialize,
    {
        value.serialize(&mut **self)
    }
    fn end(self) -> Result<Self::Ok, Self::Error> {
        self.unnest()
    }
}
// This is unlike a flexbuffers map which requires CString like keys.
// Its implemented as alternating keys and values (hopefully).
impl<'a> ser::SerializeMap for &'a mut FlexbufferSerializer {
    type Ok = ();
    type Error = Error;
    fn serialize_key<T: ?Sized>(&mut self, key: &T) -> Result<(), Self::Error>
    where
        T: Serialize,
    {
        key.serialize(&mut **self)
    }
    fn serialize_value<T: ?Sized>(&mut self, value: &T) -> Result<(), Self::Error>
    where
        T: Serialize,
    {
        value.serialize(&mut **self)
    }
    fn end(self) -> Result<Self::Ok, Self::Error> {
        self.unnest()
    }
}
impl<'a> ser::SerializeTuple for &mut FlexbufferSerializer {
    type Ok = ();
    type Error = Error;
    fn serialize_element<T: ?Sized>(&mut self, value: &T) -> Result<(), Self::Error>
    where
        T: Serialize,
    {
        value.serialize(&mut **self)
    }
    fn end(self) -> Result<Self::Ok, Self::Error> {
        self.unnest()
    }
}
impl<'a> ser::SerializeTupleStruct for &mut FlexbufferSerializer {
    type Ok = ();
    type Error = Error;
    fn serialize_field<T: ?Sized>(&mut self, value: &T) -> Result<(), Self::Error>
    where
        T: Serialize,
    {
        value.serialize(&mut **self)
    }
    fn end(self) -> Result<Self::Ok, Self::Error> {
        self.unnest()
    }
}
impl<'a> ser::SerializeStruct for &mut FlexbufferSerializer {
    type Ok = ();
    type Error = Error;
    fn serialize_field<T: ?Sized>(
        &mut self,
        _key: &'static str,
        value: &T,
    ) -> Result<(), Self::Error>
    where
        T: Serialize,
    {
        value.serialize(&mut **self)
    }
    fn end(self) -> Result<Self::Ok, Self::Error> {
        self.unnest()
    }
}
impl<'a> ser::SerializeTupleVariant for &mut FlexbufferSerializer {
    type Ok = ();
    type Error = Error;
    fn serialize_field<T: ?Sized>(&mut self, value: &T) -> Result<(), Self::Error>
    where
        T: Serialize,
    {
        value.serialize(&mut **self)
    }
    fn end(self) -> Result<Self::Ok, Self::Error> {
        self.unnest()
    }
}
impl<'a> ser::SerializeStructVariant for &mut FlexbufferSerializer {
    type Ok = ();
    type Error = Error;
    fn serialize_field<T: ?Sized>(
        &mut self,
        _key: &'static str,
        value: &T,
    ) -> Result<(), Self::Error>
    where
        T: Serialize,
    {
        value.serialize(&mut **self)
    }
    fn end(self) -> Result<Self::Ok, Self::Error> {
        self.unnest()
    }
    // TODO: skip field?
}

impl<'a> ser::Serializer for &'a mut FlexbufferSerializer {
    type SerializeSeq = &'a mut FlexbufferSerializer;
    type SerializeTuple = &'a mut FlexbufferSerializer;
    type SerializeTupleStruct = &'a mut FlexbufferSerializer;
    type SerializeTupleVariant = &'a mut FlexbufferSerializer;
    type SerializeMap = &'a mut FlexbufferSerializer;
    type SerializeStruct = &'a mut FlexbufferSerializer;
    type SerializeStructVariant = &'a mut FlexbufferSerializer;
    type Ok = ();
    type Error = Error;
    fn serialize_bool(self, v: bool) -> Result<Self::Ok, Self::Error> {
        self.builder.push(v);
        self.finish_if_not_nested()
    }
    fn serialize_i8(self, v: i8) -> Result<Self::Ok, Self::Error> {
        self.builder.push(v);
        self.finish_if_not_nested()
    }
    fn serialize_i16(self, v: i16) -> Result<Self::Ok, Self::Error> {
        self.builder.push(v);
        self.finish_if_not_nested()
    }
    fn serialize_i32(self, v: i32) -> Result<Self::Ok, Self::Error> {
        self.builder.push(v);
        self.finish_if_not_nested()
    }
    fn serialize_i64(self, v: i64) -> Result<Self::Ok, Self::Error> {
        self.builder.push(v);
        self.finish_if_not_nested()
    }
    fn serialize_u8(self, v: u8) -> Result<Self::Ok, Self::Error> {
        self.builder.push(v);
        self.finish_if_not_nested()
    }
    fn serialize_u16(self, v: u16) -> Result<Self::Ok, Self::Error> {
        self.builder.push(v);
        self.finish_if_not_nested()
    }
    fn serialize_u32(self, v: u32) -> Result<Self::Ok, Self::Error> {
        self.builder.push(v);
        self.finish_if_not_nested()
    }
    fn serialize_u64(self, v: u64) -> Result<Self::Ok, Self::Error> {
        self.builder.push(v);
        self.finish_if_not_nested()
    }
    fn serialize_f32(self, v: f32) -> Result<Self::Ok, Self::Error> {
        self.builder.push(v);
        self.finish_if_not_nested()
    }
    fn serialize_f64(self, v: f64) -> Result<Self::Ok, Self::Error> {
        self.builder.push(v);
        self.finish_if_not_nested()
    }
    fn serialize_char(self, v: char) -> Result<Self::Ok, Self::Error> {
        self.builder.push(v as u8);
        self.finish_if_not_nested()
    }
    fn serialize_str(self, v: &str) -> Result<Self::Ok, Self::Error> {
        self.builder.push(v);
        self.finish_if_not_nested()
    }
    fn serialize_bytes(self, v: &[u8]) -> Result<Self::Ok, Self::Error> {
        self.builder.push(v);
        self.finish_if_not_nested()
    }
    fn serialize_none(self) -> Result<Self::Ok, Self::Error> {
        self.builder.push(());
        self.finish_if_not_nested()
    }
    fn serialize_some<T: ?Sized>(self, t: &T) -> Result<Self::Ok, Self::Error>
    where
        T: Serialize,
    {
        t.serialize(self)
    }
    fn serialize_unit(self) -> Result<Self::Ok, Self::Error> {
        self.builder.push(());
        self.finish_if_not_nested()
    }
    fn serialize_unit_struct(self, _name: &'static str) -> Result<Self::Ok, Self::Error> {
        self.builder.push(());
        self.finish_if_not_nested()
    }
    fn serialize_unit_variant(
        self,
        _name: &'static str,
        variant_index: u32,
        _variant: &'static str,
    ) -> Result<Self::Ok, Self::Error> {
        self.builder.push(variant_index);
        self.finish_if_not_nested()
    }
    fn serialize_newtype_struct<T: ?Sized>(
        self,
        _name: &'static str,
        value: &T,
    ) -> Result<Self::Ok, Self::Error>
    where
        T: Serialize,
    {
        value.serialize(self)
    }
    fn serialize_newtype_variant<T: ?Sized>(
        self,
        _name: &'static str,
        variant_index: u32,
        _variant: &'static str,
        value: &T,
    ) -> Result<Self::Ok, Self::Error>
    where
        T: Serialize,
    {
        self.nest();
        self.builder.push(variant_index);
        value.serialize(&mut *self)?;
        self.unnest()
    }
    fn serialize_seq(self, _len: Option<usize>) -> Result<Self::SerializeSeq, Self::Error> {
        self.nest();
        Ok(self)
    }
    fn serialize_tuple(self, _len: usize) -> Result<Self::SerializeTuple, Self::Error> {
        self.nest();
        Ok(self)
    }
    fn serialize_tuple_struct(
        self,
        _name: &'static str,
        _len: usize,
    ) -> Result<Self::SerializeTupleStruct, Self::Error> {
        self.nest();
        Ok(self)
    }
    fn serialize_tuple_variant(
        self,
        _name: &'static str,
        variant_index: u32,
        _variant: &'static str,
        _len: usize,
    ) -> Result<Self::SerializeTupleVariant, Self::Error> {
        self.nest();
        self.builder.push(variant_index);
        Ok(self)
    }
    fn serialize_map(self, _len: Option<usize>) -> Result<Self::SerializeMap, Self::Error> {
        self.nest();
        Ok(self)
    }
    fn serialize_struct(
        self,
        _name: &'static str,
        _len: usize,
    ) -> Result<Self::SerializeStruct, Self::Error> {
        self.nest();
        Ok(self)
    }
    fn serialize_struct_variant(
        self,
        _name: &'static str,
        variant_index: u32,
        _variant: &'static str,
        _len: usize,
    ) -> Result<Self::SerializeStructVariant, Self::Error> {
        self.nest();
        self.builder.push(variant_index);
        Ok(self)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn unit() {
        #[derive(Serialize)]
        struct Foo;
        let mut s = FlexbufferSerializer::new();
        Foo.serialize(&mut s).unwrap();
        assert_eq!(s.view(), &[0, 0, 0]);
    }

    #[test]
    fn i8() {
        let mut s = FlexbufferSerializer::new();
        13i8.serialize(&mut s).unwrap();
        assert_eq!(s.view(), &[13, 4, 0]);
    }
    #[test]
    fn tuple_struct_i8() {
        #[derive(Serialize)]
        struct Foo(i32);
        let mut s = FlexbufferSerializer::new();
        Foo(13).serialize(&mut s).unwrap();
        assert_eq!(s.view(), &[13, 4, 0]);
    }
    #[test]
    fn tuple_tuple_struct_i8_is_inlined() {
        #[derive(Serialize)]
        struct Foo(i32);
        #[derive(Serialize)]
        struct Bar(Foo);
        let mut s = FlexbufferSerializer::new();
        Bar(Foo(13)).serialize(&mut s).unwrap();
        assert_eq!(s.view(), &[13, 4, 0]);
    }
    #[test] // TODO: should this be inlined or not?
    fn struct_i8() {
        #[derive(Serialize)]
        struct Foo {
            a: i32,
        };
        let mut s = FlexbufferSerializer::new();
        Foo { a: 13 }.serialize(&mut s).unwrap();
        // VecInt
        assert_eq!(s.view(), &[1, 13, 1, 44, 0]);
    }
    #[test]
    #[rustfmt::skip]
    fn struct_4u16() {
        #[derive(Serialize)]
        struct Foo {
            a: u8,
            b: u16,
            c: u32,
            d: u64,
        }
        let mut s = FlexbufferSerializer::new();
        let res = Foo {
            a: 4,
            b: 16,
            c: 64,
            d: 256,
        }
        .serialize(&mut s);
        res.unwrap();
        assert_eq!(
            s.view(),
            &[
                4, 0, 16, 0, 64, 0, 0, 1, // Data
                8,           // Vector offset.
                23 << 2 | 1, // (VectorUInt, W16 - referring to data).
                0,           // Root width W8 - referring to vector.
            ]
        );
    }
    #[test]
    #[rustfmt::skip]
    fn array_in_struct() {
        #[derive(Serialize)]
        struct Foo {
            a: u64,
            b: [u32; 3],
            c: i64,
        }
        let mut s = FlexbufferSerializer::new();
        Foo {
            a: 0,
            b: [1, 2, 3],
            c: -42,
        }
        .serialize(&mut s)
        .unwrap();
        assert_eq!(
            s.view(),
            &[
                1, 2, 3,      // Nested vector
                3, 0, 5, 214, // Root Vector: size, v[0], v[1] (offset), v[2] as u8
                2 << 2 | 0,   // v[0]: (UInt, W8)
                20 << 2 | 0,  // v[1]: (VectorUInt3, W8)
                1 << 2 | 0,   // v[2]: (Int, W8)
                6,            // Root points to Root vector
                10 << 2 | 0,  // Root type and width (Vector, W8)
                0,            // Root bit width
            ]
        );
    }
    #[test]
    #[rustfmt::skip]
    fn enum_struct() {
        #[derive(Serialize)]
        enum Foo {
            _A, B { a: u32, b: [i16; 3], c: String }, _C
        }
        let mut s = FlexbufferSerializer::new();
        Foo::B {
            a: 12,
            b: [1, 2, 3],
            c: "hello".to_string(),
        }
        .serialize(&mut s)
        .unwrap();
        assert_eq!(
            s.view(),
            [
                1, 2, 3,
                5, b'h', b'e', b'l', b'l', b'o', b'\0',
                4,
                1,  // variant (Foo::B)
                12, // a
                13, // offset to b
                10, // offset to c
                2 << 2,  // variant: UInt
                2 << 2,  // a: UInt
                19 << 2, // b: VectorInt3
                5 << 2,  // String
                8,       // offset to Vector
                10 << 2, // root: Vector
                0,       // root width
            ]
        )
    }
}
