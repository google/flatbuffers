/*
 * Copyright 2018 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#![allow(clippy::wrong_self_convention)]

use std::{
    convert::{TryFrom, TryInto},
    mem::size_of,
};

/// Trait for values that must be stored in little-endian byte order, but
/// might be represented in memory as big-endian. Every type that implements
/// EndianScalar is a valid FlatBuffers scalar value.
///
/// The Rust stdlib does not provide a trait to represent scalars, so this trait
/// serves that purpose, too.
///
/// Note that we do not use the num-traits crate for this, because it provides
/// "too much". For example, num-traits provides i128 support, but that is an
/// invalid FlatBuffers type.
pub trait EndianScalar: Sized + PartialEq + Copy + Clone {
    type Bytes: AsRef<[u8]> + for<'a> TryFrom<&'a [u8]>;
    const N: usize;
    fn to_le_bytes(self) -> Self::Bytes;
    fn from_le_bytes(v: Self::Bytes) -> Self;

    /// Reads a chunk into bytes, panicking if the chunk does not fit into `T`.
    #[inline]
    fn from_le_chunk(chunk: &[u8]) -> Self {
        let chunk: Self::Bytes = match chunk.try_into() {
            Ok(v) => v,
            Err(_) => panic!(),
        };
        Self::from_le_bytes(chunk)
    }
}

/// Macro for implementing an endian conversion using the stdlib `to_le` and
/// `from_le` functions. This is used for integer types. It is not used for
/// floats, because the `to_le` and `from_le` are not implemented for them in
/// the stdlib.
macro_rules! impl_endian_scalar {
    ($ty:ident) => {
        impl EndianScalar for $ty {
            type Bytes = [u8; size_of::<Self>()];
            const N: usize = size_of::<Self>();
            #[inline]
            fn to_le_bytes(self) -> Self::Bytes {
                <$ty>::to_le_bytes(self)
            }
            #[inline]
            fn from_le_bytes(bytes: Self::Bytes) -> Self {
                <$ty>::from_le_bytes(bytes)
            }
        }
    };
}

impl EndianScalar for bool {
    type Bytes = [u8; 1];
    const N: usize = size_of::<Self>();
    #[inline]
    fn to_le_bytes(self) -> Self::Bytes {
        [self as u8]
    }
    #[inline]
    fn from_le_bytes(bytes: Self::Bytes) -> Self {
        bytes[0] != 0
    }
}

//impl_endian_scalar!(bool);
impl_endian_scalar!(u8);
impl_endian_scalar!(i8);
impl_endian_scalar!(u16);
impl_endian_scalar!(u32);
impl_endian_scalar!(u64);
impl_endian_scalar!(i16);
impl_endian_scalar!(i32);
impl_endian_scalar!(i64);
impl_endian_scalar!(f32);
impl_endian_scalar!(f64);
