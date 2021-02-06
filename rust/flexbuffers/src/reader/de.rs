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

use std::marker::PhantomData;

use super::Error;
use crate::{FlexBuffer, FlexBufferType, Reader, ReaderIterator};
use serde::de::{
    DeserializeSeed, Deserializer, EnumAccess, IntoDeserializer, MapAccess, SeqAccess,
    VariantAccess, Visitor,
};

pub struct ReaderDeserializer<'de, B> {
    inner: Reader<B>,
    lifetime: PhantomData<&'de B>
}

impl<'de, B: 'de + FlexBuffer> ReaderDeserializer<'de, B> {
    pub fn new(reader: Reader<B>) -> Self {
        Self {
            inner: reader,
            lifetime: PhantomData
        }
    }

    /// Pointer returned is ensured to live long enough b/c of the PhantomData struct.
    #[inline]
    pub fn inner(&self) -> &'de Reader<B> {
        let inner: *const Reader<B> = &self.inner;
        unsafe { inner.as_ref().unwrap() } // Value will never be `null`
    }

    /// Mutable pointer returned is ensured to live long enough b/c of the PhantomData struct.
    #[inline]
    pub fn inner_mut(&mut self) -> &'de mut Reader<B> {
        let inner: *mut Reader<B> = &mut self.inner;
        unsafe { inner.as_mut().unwrap() } // Value will never be `null`
    }
}

struct ReaderDeserializerIterator<'de, B>{
    inner: ReaderIterator<B>,
    lifetime: PhantomData<&'de B>
}

impl<'de, B: 'de + FlexBuffer> ReaderDeserializerIterator<'de, B> {
    pub fn new(iterator: ReaderIterator<B>) -> Self {
        Self {
            inner: iterator,
            lifetime: PhantomData
        }
    }
} 

impl<'de, B: 'de + FlexBuffer> Iterator for ReaderDeserializerIterator<'de, B> {
    type Item = ReaderDeserializer<'de, B>;
    
    fn next(&mut self) -> Option<Self::Item> {
        if let Some(reader) = self.inner.next() {
            Some(ReaderDeserializer::new(reader))
        } else {
            None
        }
    }
}

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

impl<'de, B: 'de + FlexBuffer> SeqAccess<'de> for ReaderDeserializerIterator<'de, B> {
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
        Some(self.inner.len())
    }
}

struct EnumReader<'de, B> {
    variant: &'de str,
    value: Option<Reader<B>>,
}

impl<'de, B: 'de + FlexBuffer> EnumAccess<'de> for EnumReader<'de, B> {
    type Error = DeserializationError;
    type Variant = ReaderDeserializer<'de, B>;

    fn variant_seed<V>(self, seed: V) -> Result<(V::Value, Self::Variant), Self::Error>
    where
        V: DeserializeSeed<'de>,
    {
        seed.deserialize(self.variant.into_deserializer())
            .map(|v| (v, ReaderDeserializer::new(self.value.unwrap_or_default())))
    }
}

struct MapAccessor<'de, B> {
    keys: ReaderDeserializerIterator<'de, B>,
    vals: ReaderDeserializerIterator<'de, B>,
}

impl<'de, B: 'de + FlexBuffer> MapAccess<'de> for MapAccessor<'de, B> {
    type Error = DeserializationError;

    fn next_key_seed<K>(&mut self, seed: K) -> Result<Option<K::Value>, Self::Error>
    where
        K: DeserializeSeed<'de>,
    {
        if let Some(k) = self.keys.next() {
            seed.deserialize(k).map(Some)
        } else {
            Ok(None)
        }
    }

    fn next_value_seed<V>(&mut self, seed: V) -> Result<V::Value, Self::Error>
    where
        V: DeserializeSeed<'de>,
    {
        let val = self.vals.next().ok_or(Error::IndexOutOfBounds)?;
        seed.deserialize(val)
    }
}

impl<'de, B: 'de + FlexBuffer> VariantAccess<'de> for ReaderDeserializer<'de, B> {
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
        visitor.visit_seq(ReaderDeserializerIterator::new(self.inner.as_vector().iter()))
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
        let m = self.inner.get_map()?;
        visitor.visit_map(MapAccessor {
            keys: ReaderDeserializerIterator::new(m.keys_vector().iter()),
            vals: ReaderDeserializerIterator::new(m.iter_values()),
        })
    }
}

impl<'de, B: 'de + FlexBuffer> Deserializer<'de> for ReaderDeserializer<'de, B> {
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
        match (self.inner.flexbuffer_type(), self.inner.bitwidth()) {
            (Bool, _) => visitor.visit_bool(self.inner.as_bool()),
            (UInt, W8) => visitor.visit_u8(self.inner.as_u8()),
            (UInt, W16) => visitor.visit_u16(self.inner.as_u16()),
            (UInt, W32) => visitor.visit_u32(self.inner.as_u32()),
            (UInt, W64) => visitor.visit_u64(self.inner.as_u64()),
            (Int, W8) => visitor.visit_i8(self.inner.as_i8()),
            (Int, W16) => visitor.visit_i16(self.inner.as_i16()),
            (Int, W32) => visitor.visit_i32(self.inner.as_i32()),
            (Int, W64) => visitor.visit_i64(self.inner.as_i64()),
            (Float, W32) => visitor.visit_f32(self.inner.as_f32()),
            (Float, W64) => visitor.visit_f64(self.inner.as_f64()),
            (Float, _) => Err(Error::InvalidPackedType.into()), // f8 and f16 are not supported.
            (Null, _) => visitor.visit_unit(),
            (String, _) | (Key, _) => visitor.visit_borrowed_str(self.inner().as_str()),
            // FIXME: Lifetime issue
            (Blob, _) => visitor.visit_bytes(self.inner.get_blob()?.0.as_ref()),
            (Map, _) => {
                let m = self.inner.get_map()?;
                visitor.visit_map(MapAccessor {
                    keys: ReaderDeserializerIterator::new(m.keys_vector().iter()),
                    vals: ReaderDeserializerIterator::new(m.iter_values()),
                })
            }
            (ty, _) if ty.is_vector() => visitor.visit_seq(ReaderDeserializerIterator::new(self.inner.as_vector().iter())),
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
        visitor.visit_char(self.inner.as_u8() as char)
    }

    fn deserialize_byte_buf<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        visitor.visit_byte_buf(self.inner.get_blob()?.0.as_ref().to_vec())
    }

    fn deserialize_option<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'de>,
    {
        if self.inner.flexbuffer_type() == FlexBufferType::Null {
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
        let (variant, value) = match self.inner.fxb_type {
            FlexBufferType::String => (self.inner().as_str(), None),
            FlexBufferType::Map => {
                let m = self.inner().get_map()?;
                let variant: *const str = m.keys_vector().idx(0).get_key()?;
                let value = Some(m.idx(0));
                // FIXME: Lifetime issue
                (unsafe { variant.as_ref().unwrap() }, value)
            }
            _ => {
                return Err(Error::UnexpectedFlexbufferType {
                    expected: FlexBufferType::Map,
                    actual: self.inner.fxb_type,
                }
                .into());
            }
        };
        visitor.visit_enum(EnumReader { variant, value })
    }
}
