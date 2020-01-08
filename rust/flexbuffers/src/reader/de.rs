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

use crate::Error;
use crate::{FlexBufferType, Reader, ReaderIterator};
use serde::de::{
    DeserializeSeed, Deserializer, EnumAccess, IntoDeserializer, SeqAccess, VariantAccess, Visitor,
    MapAccess
};

impl<'de> SeqAccess<'de> for ReaderIterator<'de> {
    type Error = Error;
    fn next_element_seed<T>(
        &mut self,
        seed: T,
    ) -> Result<Option<<T as DeserializeSeed<'de>>::Value>, Error>
    where
        T: DeserializeSeed<'de>,
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

struct EnumReader<'de> {
    variant: &'de str,
    value: Option<Reader<'de>>,
}

impl<'de> EnumAccess<'de> for EnumReader<'de> {
    type Error = Error;
    type Variant = Reader<'de>;
    fn variant_seed<V>(self, seed: V) -> Result<(V::Value, Self::Variant), Self::Error>
    where
        V: DeserializeSeed<'de>,
    {
        seed.deserialize(self.variant.into_deserializer())
            .map(|v| (v, self.value.unwrap_or_default()))
    }
}

struct MapAccessor<'de, T>
    where
    T: Iterator<Item=&'de str>
 {
    keys: T,
    vals: ReaderIterator<'de>,
}
impl<'de, T: Iterator<Item=&'de str>> MapAccess<'de> for MapAccessor<'de, T> {
    type Error = Error;

    fn next_key_seed<K>(&mut self, seed: K) -> Result<Option<K::Value>, Error>
    where
        K: DeserializeSeed<'de>
    {
        if let Some(k) = self.keys.next() {
            seed.deserialize(k.into_deserializer()).map(Some)
        } else {
            Ok(None)
        }
    }
    fn next_value_seed<V>(&mut self, seed: V) -> Result<V::Value, Error>
    where
        V: DeserializeSeed<'de>,
    {
        let val = self.vals.next().ok_or(Error::IndexOutOfBounds)?;
        seed.deserialize(val)
    }
}

impl<'de> VariantAccess<'de> for Reader<'de> {
    type Error = Error;
    fn unit_variant(self) -> Result<(), Error> {
        Ok(())
    }
    fn newtype_variant_seed<T>(self, seed: T) -> Result<T::Value, Error>
    where
        T: DeserializeSeed<'de>,
    {
        seed.deserialize(self)
    }
    // Tuple variants have an internally tagged representation. They are vectors where Index 0 is
    // the discriminant and index N is field N-1.
    fn tuple_variant<V>(self, _len: usize, visitor: V) -> Result<V::Value, Error>
    where
        V: Visitor<'de>,
    {
        visitor.visit_seq(self.iter())
    }
    // Struct variants have an internally tagged representation. They are vectors where Index 0 is
    // the discriminant and index N is field N-1.
    fn struct_variant<V>(
        self,
        _fields: &'static [&'static str],
        visitor: V,
    ) -> Result<V::Value, Error>
    where
        V: Visitor<'de>,
    {
        let m = self.get_map()?;
        visitor.visit_map(MapAccessor{
            keys: m.iter_keys(),
            vals: m.iter_values()
        })
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
            (Map, _) => {
                let m = self.get_map()?;
                visitor.visit_map(MapAccessor{
                    keys: m.iter_keys(),
                    vals: m.iter_values()
                })
            }
            (ty, _) if ty.is_vector() => {
                visitor.visit_seq(self.iter())
            }
            (ty, bw) => unreachable!("TODO deserialize_any {:?} {:?}.", ty, bw),
        }
    }
    // TODO(cneo): Use type hints instead of deserialize_any for tiny efficiency gains.
    serde::forward_to_deserialize_any! {
        bool i8 i16 i32 i64 u8 u16 u32 u64 f32 f64 str unit unit_struct bytes
        ignored_any map identifier struct tuple tuple_struct seq string
    }
    fn deserialize_char<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        visitor.visit_char(self.as_u8() as char)
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
        if self.flexbuffer_type() == FlexBufferType::Null {
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
    fn deserialize_enum<V>(
        self,
        _name: &'static str,
        _variants: &'static [&'static str],
        visitor: V,
    ) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        let (variant, value) = match self.fxb_type {
            FlexBufferType::String => (self.as_str(), None),
            FlexBufferType::Map => {
                let ks = self.get_map_key_vector()?;
                let vs = self.get_vector()?;
                let variant = ks.idx(0).as_str();
                let value = Some(vs.idx(0));
                (variant, value)
            }
            _ => {
                return Err(Error::UnexpectedFlexbufferType {
                    expected: FlexBufferType::Map,
                    actual: self.fxb_type,
                });
            }
        };
        visitor.visit_enum(EnumReader { variant, value })
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
}
