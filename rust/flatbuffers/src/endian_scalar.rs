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
use core::mem::size_of;

mod private {
    /// Types that are trivially transmutable are those where any combination of bits
    /// represents a valid value of that type
    ///
    /// For example integral types are TriviallyTransmutable as all bit patterns are valid,
    /// however, `bool` is not trivially transmutable as only `0` and `1` are valid
    pub trait TriviallyTransmutable {
        fn read_ne(src: &[u8]) -> Self;
        fn write_ne(self, dst: &mut [u8]);
    }
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
        impl TriviallyTransmutable for $ty {
            #[inline]
            fn read_ne(src: &[u8]) -> Self {
                let mut bytes = [0; size_of::<$ty>()];
                bytes.copy_from_slice(&src[..size_of::<$ty>()]);
                Self::from_ne_bytes(bytes)
            }

            #[inline]
            fn write_ne(self, dst: &mut [u8]) {
                dst[..size_of::<$ty>()].copy_from_slice(&self.to_ne_bytes());
            }
        }

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
/// # Safety
/// Caller must provide space for the wire representation, `T::Scalar`.
#[inline]
pub unsafe fn emplace_scalar<T: EndianScalar>(s: &mut [u8], x: T) {
    x.to_little_endian().write_ne(s);
}

/// Read an EndianScalar from the provided byte slice at the specified location.
/// Performs endian conversion, if necessary.
/// # Safety
/// Caller must provide space for `T::Scalar` starting at `loc`.
#[inline]
pub unsafe fn read_scalar_at<T: EndianScalar>(s: &[u8], loc: usize) -> T {
    T::from_little_endian(T::Scalar::read_ne(&s[loc..]))
}

/// Read an EndianScalar from the provided byte slice. Performs endian
/// conversion, if necessary.
/// # Safety
/// Caller must provide space for the wire representation, `T::Scalar`.
#[inline]
pub unsafe fn read_scalar<T: EndianScalar>(s: &[u8]) -> T {
    T::from_little_endian(T::Scalar::read_ne(s))
}

#[cfg(test)]
mod tests {
    use super::*;

    fn check_wire_bytes<T: EndianScalar>(value: T, expected: &[u8]) -> (T, T) {
        // Write at an offset and check the bytes on either side of the scalar.
        let mut buf = [0xa5; 10];
        let end = 1 + expected.len();
        assert_eq!(expected.len(), size_of::<T::Scalar>());
        assert!(end < buf.len());
        // SAFETY: The destination slice has exactly the scalar's wire size.
        unsafe { emplace_scalar(&mut buf[1..end], value) };
        assert_eq!(&buf[1..end], expected);
        assert_eq!((buf[0], buf[end]), (0xa5, 0xa5));
        // SAFETY: Both reads cover the scalar just written, ending at `end`.
        unsafe { (read_scalar(&buf[1..end]), read_scalar_at(&buf[..end], 1)) }
    }

    #[test]
    fn integer_wire_bytes() {
        macro_rules! check {
            ($value:expr) => {{
                let value = $value;
                assert_eq!(
                    check_wire_bytes(value, &value.to_le_bytes()),
                    (value, value)
                );
            }};
        }
        check!(0xacu8);
        check!(0x1234u16);
        check!(0x12345678u32);
        check!(0x123456789abcdef0u64);
        check!(-7i8);
        check!(-1234i16);
        check!(-1234567i32);
        check!(-123456789i64);
    }

    #[test]
    fn boolean_and_float_wire_bytes() {
        assert_eq!(check_wire_bytes(false, &[0]), (false, false));
        assert_eq!(check_wire_bytes(true, &[1]), (true, true));
        for &bits in &[0x80000000u32, 0x7fc00042] {
            let (direct, at) = check_wire_bytes(f32::from_bits(bits), &bits.to_le_bytes());
            assert_eq!((direct.to_bits(), at.to_bits()), (bits, bits));
        }
        for &bits in &[0x8000000000000000u64, 0x7ff8000000000042] {
            let (direct, at) = check_wire_bytes(f64::from_bits(bits), &bits.to_le_bytes());
            assert_eq!((direct.to_bits(), at.to_bits()), (bits, bits));
        }
    }
}
