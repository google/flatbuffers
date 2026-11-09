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

use self::private::TriviallyTransmutable;

pub(crate) mod private {
    use core::convert::TryInto;

    /// Types that are trivially transmutable are those where any combination of bits
    /// represents a valid value of that type
    ///
    /// For example integral types are TriviallyTransmutable as all bit patterns are valid,
    /// however, `bool` is not trivially transmutable as only `0` and `1` are valid
    ///
    /// This sealed trait is the single place in the crate where raw bytes are
    /// converted to and from scalar values. All conversions are bounds-checked:
    /// short inputs panic instead of causing undefined behavior.
    pub trait TriviallyTransmutable: Sized {
        /// Loads the first `size_of::<Self>()` bytes of `bytes` as a
        /// native-endian value (i.e. the in-memory representation of the
        /// little-endian-encoded value).
        ///
        /// # Panics
        ///
        /// Panics if `bytes.len() < size_of::<Self>()`.
        fn load_ne(bytes: &[u8]) -> Self;

        /// Stores `self` in native-endian byte order (the in-memory
        /// representation of the little-endian-encoded value) into the first
        /// `size_of::<Self>()` bytes of `dst`.
        ///
        /// # Panics
        ///
        /// Panics if `dst.len() < size_of::<Self>()`.
        fn store_ne(&self, dst: &mut [u8]);
    }

    macro_rules! impl_trivially_transmutable {
        ($($ty:ident),*) => {
            $(
                impl TriviallyTransmutable for $ty {
                    #[inline]
                    fn load_ne(bytes: &[u8]) -> Self {
                        let n = core::mem::size_of::<Self>();
                        Self::from_ne_bytes(bytes[..n].try_into().expect("length mismatch"))
                    }

                    #[inline]
                    fn store_ne(&self, dst: &mut [u8]) {
                        let n = core::mem::size_of::<Self>();
                        dst[..n].copy_from_slice(&self.to_ne_bytes());
                    }
                }
            )*
        };
    }

    impl_trivially_transmutable!(u8, i8, u16, i16, u32, i32, u64, i64);
}

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
    type Scalar: private::TriviallyTransmutable;

    fn to_little_endian(self) -> Self::Scalar;

    fn from_little_endian(v: Self::Scalar) -> Self;
}

/// Macro for implementing an endian conversion using the stdlib `to_le` and
/// `from_le` functions. This is used for integer types. It is not used for
/// floats, because the `to_le` and `from_le` are not implemented for them in
/// the stdlib.
macro_rules! impl_endian_scalar {
    ($ty:ident) => {
        impl EndianScalar for $ty {
            type Scalar = Self;

            #[inline]
            fn to_little_endian(self) -> Self::Scalar {
                Self::to_le(self)
            }
            #[inline]
            fn from_little_endian(v: Self::Scalar) -> Self {
                Self::from_le(v)
            }
        }
    };
}

impl_endian_scalar!(u8);
impl_endian_scalar!(i8);
impl_endian_scalar!(u16);
impl_endian_scalar!(u32);
impl_endian_scalar!(u64);
impl_endian_scalar!(i16);
impl_endian_scalar!(i32);
impl_endian_scalar!(i64);

impl EndianScalar for bool {
    type Scalar = u8;

    fn to_little_endian(self) -> Self::Scalar {
        self as u8
    }

    fn from_little_endian(v: Self::Scalar) -> Self {
        v != 0
    }
}

impl EndianScalar for f32 {
    type Scalar = u32;
    /// Convert f32 from host endian-ness to little-endian.
    #[inline]
    fn to_little_endian(self) -> u32 {
        // Floats and Ints have the same endianness on all supported platforms.
        // <https://doc.rust-lang.org/std/primitive.f32.html#method.from_bits>
        self.to_bits().to_le()
    }
    /// Convert f32 from little-endian to host endian-ness.
    #[inline]
    fn from_little_endian(v: u32) -> Self {
        // Floats and Ints have the same endianness on all supported platforms.
        // <https://doc.rust-lang.org/std/primitive.f32.html#method.from_bits>
        f32::from_bits(u32::from_le(v))
    }
}

impl EndianScalar for f64 {
    type Scalar = u64;

    /// Convert f64 from host endian-ness to little-endian.
    #[inline]
    fn to_little_endian(self) -> u64 {
        // Floats and Ints have the same endianness on all supported platforms.
        // <https://doc.rust-lang.org/std/primitive.f64.html#method.from_bits>
        self.to_bits().to_le()
    }
    /// Convert f64 from little-endian to host endian-ness.
    #[inline]
    fn from_little_endian(v: u64) -> Self {
        // Floats and Ints have the same endianness on all supported platforms.
        // <https://doc.rust-lang.org/std/primitive.f64.html#method.from_bits>
        f64::from_bits(u64::from_le(v))
    }
}

/// Place an EndianScalar into the provided mutable byte slice. Performs
/// endian conversion, if necessary.
///
/// # Panics
///
/// Panics if `s.len() < size_of::<T>()`.
///
/// # Safety
///
/// Historically this function performed unchecked writes. It now bounds-checks
/// its input and panics on out-of-range access instead of causing undefined
/// behavior. No invariants are required of the caller; the `unsafe` marker is
/// retained for backwards compatibility with existing callers, including code
/// generated by `flatc`.
#[inline]
pub unsafe fn emplace_scalar<T: EndianScalar>(s: &mut [u8], x: T) {
    x.to_little_endian().store_ne(s);
}

/// Read an EndianScalar from the provided byte slice at the specified location.
/// Performs endian conversion, if necessary.
///
/// # Panics
///
/// Panics if `loc > s.len()` or `s.len() - loc < size_of::<T>()`.
///
/// # Safety
///
/// Historically this function performed unchecked reads. It now bounds-checks
/// its input and panics on out-of-range access instead of causing undefined
/// behavior. No invariants are required of the caller; the `unsafe` marker is
/// retained for backwards compatibility with existing callers, including code
/// generated by `flatc`.
#[inline]
pub unsafe fn read_scalar_at<T: EndianScalar>(s: &[u8], loc: usize) -> T {
    read_scalar(&s[loc..])
}

/// Read an EndianScalar from the provided byte slice. Performs endian
/// conversion, if necessary.
///
/// # Panics
///
/// Panics if `s.len() < size_of::<T>()`.
///
/// # Safety
///
/// Historically this function performed unchecked reads. It now bounds-checks
/// its input and panics on out-of-range access instead of causing undefined
/// behavior. No invariants are required of the caller; the `unsafe` marker is
/// retained for backwards compatibility with existing callers, including code
/// generated by `flatc`.
#[inline]
pub unsafe fn read_scalar<T: EndianScalar>(s: &[u8]) -> T {
    T::from_little_endian(<T::Scalar>::load_ne(s))
}
