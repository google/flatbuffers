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

use crate::reader::ReaderIterator;
use crate::Error;
use serde::de::{DeserializeSeed, Deserializer, EnumAccess, SeqAccess, VariantAccess, Visitor};

impl<'de> SeqAccess<'de> for ReaderIterator<'de> {
    type Error = crate::Error;
    fn next_element_seed<T>(
        &mut self,
        seed: T,
    ) -> Result<Option<<T as DeserializeSeed<'de>>::Value>, Error>
    where
        T: serde::de::DeserializeSeed<'de>,
    {
        if let Some(elem) = self.next() {
            seed.deserialize(elem).map(Some)
        } else {
            Ok(None)
        }
    }
    fn size_hint(&self) -> Option<usize> {
        Some(self.len())
    }
}

impl<'de> EnumAccess<'de> for ReaderIterator<'de> {
    type Error = Error;
    type Variant = Self;
    fn variant_seed<V>(self, seed: V) -> Result<(V::Value, Self::Variant), Self::Error>
    where
        V: DeserializeSeed<'de>,
    {
        let val = seed.deserialize(self.reader.reader())?;
        Ok((val, self))
    }
}
impl<'de> VariantAccess<'de> for ReaderIterator<'de> {
    type Error = Error;
    fn unit_variant(self) -> Result<(), Error> {
        Ok(())
    }
    fn newtype_variant_seed<T>(self, seed: T) -> Result<T::Value, Error>
    where
        T: DeserializeSeed<'de>,
    {
        // index 0 is discriminant, index 1 is the newtype.
        seed.deserialize(self.reader.index(1)?)
    }
    // Tuple variants have an internally tagged representation. They are vectors where Index 0 is
    // the discriminant and index N is field N-1.
    fn tuple_variant<V>(mut self, _len: usize, visitor: V) -> Result<V::Value, Error>
    where
        V: Visitor<'de>,
    {
        self.front += 1;
        visitor.visit_seq(self)
    }
    // Struct variants have an internally tagged representation. They are vectors where Index 0 is
    // the discriminant and index N is field N-1.
    fn struct_variant<V>(
        mut self,
        _fields: &'static [&'static str],
        visitor: V,
    ) -> Result<V::Value, Error>
    where
        V: Visitor<'de>,
    {
        self.front += 1;
        visitor.visit_seq(self)
    }
}

impl<'de> Deserializer<'de> for crate::Reader<'de> {
    type Error = crate::Error;
    fn deserialize_any<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        use crate::BitWidth::*;
        use crate::FlexBufferType::*;
        match (self.flexbuffer_type(), self.bitwidth()) {
            (Bool, _) => visitor.visit_bool(self.as_bool()),
            (UInt, W8) => visitor.visit_u8(self.as_u8()),
            (UInt, W16) => visitor.visit_u16(self.as_u16()),
            (UInt, W32) => visitor.visit_u32(self.as_u32()),
            (UInt, W64) => visitor.visit_u64(self.as_u64()),
            (Int, W8) => visitor.visit_i8(self.as_i8()),
            (Int, W16) => visitor.visit_i16(self.as_i16()),
            (Int, W32) => visitor.visit_i32(self.as_i32()),
            (Int, W64) => visitor.visit_i64(self.as_i64()),
            (Float, W32) => visitor.visit_f32(self.as_f32()),
            (Float, W64) => visitor.visit_f64(self.as_f64()),
            (Float, _) => Err(Error::InvalidPackedType), // f8 and f16 are not supported.
            (Null, _) => visitor.visit_unit(),
            (String, _) => visitor.visit_borrowed_str(self.as_str()),
            (Blob, _) => visitor.visit_borrowed_bytes(self.get_blob()?.0),
            (ty, bw) => unimplemented!("TODO deserialize_any {:?} {:?}.", ty, bw),
        }
    }
    // TODO(cneo): Use type hints instead of deserialize_any for tiny efficiency gains.
    serde::forward_to_deserialize_any! {
        bool i8 i16 i32 i64 u8 u16 u32 u64 f32 f64 str unit unit_struct bytes
        ignored_any
    }
    fn deserialize_char<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        visitor.visit_char(self.as_u8() as char)
    }
    fn deserialize_string<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        visitor.visit_string(self.as_str().to_string())
    }
    fn deserialize_byte_buf<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        visitor.visit_byte_buf(self.get_blob()?.0.to_vec())
    }
    fn deserialize_option<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        if self.flexbuffer_type() == crate::FlexBufferType::Null {
            visitor.visit_none()
        } else {
            visitor.visit_some(self)
        }
    }
    fn deserialize_newtype_struct<V>(
        self,
        _name: &'static str,
        visitor: V,
    ) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        visitor.visit_newtype_struct(self)
    }
    fn deserialize_seq<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        visitor.visit_seq(self.iter())
    }
    fn deserialize_tuple<V>(self, _len: usize, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        self.deserialize_seq(visitor)
    }
    fn deserialize_tuple_struct<V>(
        self,
        _name: &'static str,
        _len: usize,
        visitor: V,
    ) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        self.deserialize_seq(visitor)
    }
    fn deserialize_map<V>(self, _visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        unimplemented!()
    }
    fn deserialize_struct<V>(
        self,
        _name: &'static str,
        _fields: &'static [&'static str],
        visitor: V,
    ) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        self.deserialize_seq(visitor)
    }
    fn deserialize_enum<V>(
        self,
        _name: &'static str,
        _variants: &'static [&'static str],
        visitor: V,
    ) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        visitor.visit_enum(self.iter())
    }
    fn deserialize_identifier<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        let discriminant = if self.flexbuffer_type().is_vector() {
            self.get_vector()?.index(0)?.as_u8()
        } else {
            self.as_u8()
        };
        visitor.visit_u8(discriminant)
    }
}

#[cfg(test)]
mod tests {
    use crate::serde::Deserialize;
    use crate::Reader;
    #[test]
    fn newtype_i8() {
        #[derive(Deserialize)]
        struct Foo(u8);
        let data = [13, 4, 0];
        let r = Reader::get_root(&data).unwrap();
        let foo = Foo::deserialize(r).unwrap();
        assert_eq!(foo.0, 13);
    }
    #[test]
    fn newtype_str() {
        #[derive(Deserialize)]
        struct Foo<'a>(&'a str);
        let data = [5, b'h', b'e', b'l', b'l', b'o', b'\0', 6, 5 << 2, 0];
        let r = Reader::get_root(&data).unwrap();
        let foo = Foo::deserialize(r).unwrap();
        assert_eq!(foo.0, "hello");
    }
    #[test]
    #[rustfmt::skip]
    fn tuple_struct_to_vec_uint4() {
        #[derive(Deserialize)]
        struct Foo(u8, u16, u32, u64);
        let data = [
            4, 0, 16, 0, 64, 0, 0, 1, // Data
            8,              // Vector offset.
            23 << 2 | 1,    // (VectorUInt4, W16 - referring to data).
            0,              // Root width W8 - referring to vector.
        ];
        let r = Reader::get_root(&data).unwrap();
        let foo = Foo::deserialize(r).unwrap();
        assert_eq!(foo.0, 4);
        assert_eq!(foo.1, 16);
        assert_eq!(foo.2, 64);
        assert_eq!(foo.3, 256);

        let data = [
            1, 2, 3, 4, // The vector.
            4,          // Root data (offset).
            23 << 2,    // Root type: VectorUInt4, W8.
            0,          // Root width: W8.
        ];
        let r = Reader::get_root(&data).unwrap();
        let foo = Foo::deserialize(r).unwrap();
        assert_eq!(foo.0, 1);
        assert_eq!(foo.1, 2);
        assert_eq!(foo.2, 3);
        assert_eq!(foo.3, 4);
    }
    #[test]
    #[rustfmt::skip]
    fn enum_struct_to_vec_uint4() {
        #[derive(Deserialize, PartialEq, Debug)]
        enum Foo {
            _A(u32),
            B(u8, u16, u32),
            _C
        };
        let data = [
            1, 2, 3, 4, // The vector.
            4,          // Root data (offset).
            23 << 2,    // Root type: VectorUInt4, W8.
            0,          // Root width: W8.
        ];
        let r = Reader::get_root(&data).unwrap();
        let foo = Foo::deserialize(r).unwrap();
        assert_eq!(foo, Foo::B(2, 3, 4));
    }
}
