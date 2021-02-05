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

use super::Error;
use crate::{FlexBuffer, FlexBufferType, Reader, ReaderIterator};
use serde::de::{
    DeserializeSeed, Deserializer, EnumAccess, IntoDeserializer, MapAccess, SeqAccess,
    VariantAccess, Visitor,
};

type BufferSlice<'de> = &'de [u8];

struct ReaderRef<'de, B>(&'de Reader<B>);

/// Errors that may happen when deserializing a flexbuffer with serde.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum DeserializationError {
    Reader(Error),
    Serde(String),
}

impl std::error::Error for DeserializationError {}
impl std::fmt::Display for DeserializationError {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> Result<(), std::fmt::Error> {
        match self {
            Self::Reader(r) => write!(f, "Flexbuffer Read Error: {:?}", r),
            Self::Serde(s) => write!(f, "Serde Error: {}", s),
        }
    }
}
impl serde::de::Error for DeserializationError {
    fn custom<T>(msg: T) -> Self
    where
        T: std::fmt::Display,
    {
        Self::Serde(format!("{}", msg))
    }
}
impl std::convert::From<super::Error> for DeserializationError {
    fn from(e: super::Error) -> Self {
        Self::Reader(e)
    }
}

struct ReaderIteratorRef<'de, B>(&'de ReaderIterator<B>);

impl<'de, B: FlexBuffer> Iterator for ReaderIteratorRef<'de, B> {
    type Item = ReaderRef<'de, B>;
    
    fn next(&mut self) -> Option<Self::Item> {
        if let Some(reader) = self.0.next() {
            Some(ReaderRef(&reader))
        } else {
            None
        }
    }
}

impl<'de, B: FlexBuffer> SeqAccess<'de> for ReaderIteratorRef<'de, B> {
    type Error = DeserializationError;

    fn next_element_seed<T>(
        &mut self,
        seed: T,
    ) -> Result<Option<<T as DeserializeSeed<'de>>::Value>, Self::Error>
    where
        T: DeserializeSeed<'de>,
    {
        if let Some(elem) = self.next() {
            //todo!("Lifetime issue with serde iteration");
            seed.deserialize(elem).map(Some)
        } else {
            Ok(None)
        }
    }

    fn size_hint(&self) -> Option<usize> {
        Some(self.0.len())
    }
}

struct EnumReader<'de, B> {
    variant: &'de str,
    value: Option<Reader<B>>,
}

impl<'de, B: 'de + FlexBuffer> EnumAccess<'de> for EnumReader<'de, B> {
    type Error = DeserializationError;
    type Variant = ReaderRef<'de, B>;

    fn variant_seed<V>(self, seed: V) -> Result<(V::Value, Self::Variant), Self::Error>
    where
        V: DeserializeSeed<'de>,
    {
        seed.deserialize(self.variant.into_deserializer())
            .map(|v| (v, ReaderRef(&self.value.unwrap_or_default())))
    }
}

struct MapAccessor<B> {
    keys: ReaderIterator<B>,
    vals: ReaderIterator<B>,
}

struct MapAccessorRef<'de, B>(&'de MapAccessor<B>);

impl<'de, B: FlexBuffer> MapAccess<'de> for MapAccessorRef<'de, B> {
    type Error = DeserializationError;

    fn next_key_seed<K>(&mut self, seed: K) -> Result<Option<K::Value>, Self::Error>
    where
        K: DeserializeSeed<'de>,
    {
        if let Some(k) = self.0.keys.next() {
            seed.deserialize(ReaderRef(&k)).map(Some)
        } else {
            Ok(None)
        }
    }

    fn next_value_seed<V>(&mut self, seed: V) -> Result<V::Value, Self::Error>
    where
        V: DeserializeSeed<'de>,
    {
        let val = self.0.vals.next().ok_or(Error::IndexOutOfBounds)?;
        seed.deserialize(ReaderRef(&val))
    }
}

impl<'de, B: FlexBuffer> VariantAccess<'de> for ReaderRef<'de, B> {
    type Error = DeserializationError;

    fn unit_variant(self) -> Result<(), Self::Error> {
        Ok(())
    }

    fn newtype_variant_seed<T>(self, seed: T) -> Result<T::Value, Self::Error>
    where
        T: DeserializeSeed<'de>,
    {
        seed.deserialize(self)
    }

    // Tuple variants have an internally tagged representation. They are vectors where Index 0 is
    // the discriminant and index N is field N-1.
    fn tuple_variant<V>(self, _len: usize, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        visitor.visit_seq(ReaderIteratorRef(&self.0.as_vector().iter()))
    }

    // Struct variants have an internally tagged representation. They are vectors where Index 0 is
    // the discriminant and index N is field N-1.
    fn struct_variant<V>(
        self,
        _fields: &'static [&'static str],
        visitor: V,
    ) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        let m = self.0.get_map()?;
        visitor.visit_map(MapAccessorRef(&MapAccessor {
            keys: m.keys_vector().iter(),
            vals: m.iter_values(),
        }))
    }
}

impl<'de, B: FlexBuffer> Deserializer<'de> for ReaderRef<'de, B> {
    type Error = DeserializationError;
    fn is_human_readable(&self) -> bool {
        cfg!(deserialize_human_readable)
    }

    fn deserialize_any<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        use crate::BitWidth::*;
        use crate::FlexBufferType::*;
        match (self.0.flexbuffer_type(), self.0.bitwidth()) {
            (Bool, _) => visitor.visit_bool(self.0.as_bool()),
            (UInt, W8) => visitor.visit_u8(self.0.as_u8()),
            (UInt, W16) => visitor.visit_u16(self.0.as_u16()),
            (UInt, W32) => visitor.visit_u32(self.0.as_u32()),
            (UInt, W64) => visitor.visit_u64(self.0.as_u64()),
            (Int, W8) => visitor.visit_i8(self.0.as_i8()),
            (Int, W16) => visitor.visit_i16(self.0.as_i16()),
            (Int, W32) => visitor.visit_i32(self.0.as_i32()),
            (Int, W64) => visitor.visit_i64(self.0.as_i64()),
            (Float, W32) => visitor.visit_f32(self.0.as_f32()),
            (Float, W64) => visitor.visit_f64(self.0.as_f64()),
            (Float, _) => Err(Error::InvalidPackedType.into()), // f8 and f16 are not supported.
            (Null, _) => visitor.visit_unit(),
            (String, _) | (Key, _) => visitor.visit_borrowed_str(self.0.as_str()),
            (Blob, _) => visitor.visit_borrowed_bytes(self.0.get_blob()?.0.as_ref()),
            (Map, _) => {
                let m = self.0.get_map()?;
                visitor.visit_map(MapAccessorRef(&MapAccessor {
                    keys: m.keys_vector().iter(),
                    vals: m.iter_values(),
                }))
            }
            (ty, _) if ty.is_vector() => visitor.visit_seq(ReaderIteratorRef(&self.0.as_vector().iter())),
            (ty, bw) => unreachable!("TODO deserialize_any {:?} {:?}.", ty, bw),
        }
    }

    serde::forward_to_deserialize_any! {
        bool i8 i16 i32 i64 u8 u16 u32 u64 f32 f64 str unit unit_struct bytes
        ignored_any map identifier struct tuple tuple_struct seq string
    }

    fn deserialize_char<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        visitor.visit_char(self.0.as_u8() as char)
    }

    fn deserialize_byte_buf<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        visitor.visit_byte_buf(self.0.get_blob()?.0.as_ref().to_vec())
    }

    fn deserialize_option<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        if self.0.flexbuffer_type() == FlexBufferType::Null {
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
        let (variant, value) = match self.0.fxb_type {
            FlexBufferType::String => (self.0.as_str(), None),
            FlexBufferType::Map => {
                let m = self.0.get_map()?;
                let variant = m.keys_vector().idx(0).get_key()?;
                let value = Some(m.idx(0));
                (variant, value)
            }
            _ => {
                return Err(Error::UnexpectedFlexbufferType {
                    expected: FlexBufferType::Map,
                    actual: self.0.fxb_type,
                }
                .into());
            }
        };
        visitor.visit_enum(EnumReader { variant, value })
    }
}
