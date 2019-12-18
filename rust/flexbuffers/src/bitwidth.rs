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

use crate::bitwidth::BitWidth::*;
use std::slice::Iter;

/// This represents the size of Flexbuffers data. For numbers the bitwidth refers to
/// precision. Flexbuffers automatically compresses numbers to the smallest possible width
/// (`250u64` is stored as `250u8`).
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, num_enum::TryFromPrimitive)]
#[repr(u8)]
pub enum BitWidth {
    W8 = 0,
    W16 = 1,
    W32 = 2,
    W64 = 3,
}
impl BitWidth {
    pub(crate) fn iter() -> Iter<'static, Self> {
        [W8, W16, W32, W64].iter()
    }
    pub fn n_bytes(self) -> usize {
        1 << self as usize
    }
}

impl Default for BitWidth {
    fn default() -> Self {
        W8
    }
}

macro_rules! impl_bitwidth_from {
    ($from: ident, $w64: ident, $w32: ident, $w16: ident, $w8: ident) => {
        impl From<$from> for BitWidth {
            fn from(x: $from) -> BitWidth {
                let x = x as $w64;
                if x >= $w8::min_value() as $w64 && x <= $w8::max_value() as $w64 {
                    return W8;
                }
                if x >= $w16::min_value() as $w64 && x <= $w16::max_value() as $w64 {
                    return W16;
                }
                if x >= $w32::min_value() as $w64 && x <= $w32::max_value() as $w64 {
                    return W32;
                }
                W64
            }
        }
    };
}
impl_bitwidth_from!(u64, u64, u32, u16, u8);
impl_bitwidth_from!(usize, u64, u32, u16, u8);
impl_bitwidth_from!(i64, i64, i32, i16, i8);

#[allow(clippy::cast_lossless)]
impl From<f64> for BitWidth {
    fn from(x: f64) -> BitWidth {
        if (x - x as f32 as f64).abs() >= std::f64::EPSILON {
            W64
        } else {
            W32
        }
    }
}
impl From<f32> for BitWidth {
    fn from(_: f32) -> BitWidth {
        W32
    }
}

/// Zero pad `v` until `T` will be byte aligned when pushed.
pub fn align(buffer: &mut Vec<u8>, width: BitWidth) {
    let bytes = 1 << width as u8;
    let alignment = (bytes - buffer.len() % bytes) % bytes;
    // Profiling reveals the loop is faster than Vec::resize.
    for _ in 0..alignment as usize {
        buffer.push(0);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn align_8byte() {
        let mut v = vec![1, 2, 3];
        align(&mut v, W64);
        assert_eq!(v, vec![1, 2, 3, 0, 0, 0, 0, 0]);
    }
    #[test]
    fn align_4byte() {
        let mut v = vec![1, 2, 3];
        align(&mut v, W32);
        assert_eq!(v, vec![1, 2, 3, 0]);
    }
    #[test]
    fn align_2byte() {
        let mut v = vec![1];
        align(&mut v, W16);
        assert_eq!(v, vec![1, 0]);
    }
    #[test]
    fn align_1byte() {
        let mut v = vec![1];
        align(&mut v, W8);
        assert_eq!(v, vec![1]);
    }
    #[test]
    fn test_f32_width() {
        assert_eq!(W32, BitWidth::from(std::f32::consts::FRAC_1_PI));
        assert_eq!(W32, BitWidth::from(std::f32::consts::E));
        assert_eq!(W32, BitWidth::from(std::f32::consts::SQRT_2));
    }
    #[test]
    fn test_f64_width() {
        println!(
            "{:?} {:?}",
            std::f64::consts::FRAC_1_PI.to_le_bytes(),
            (std::f64::consts::FRAC_1_PI as f32 as f64).to_le_bytes()
        );
        assert_eq!(W64, BitWidth::from(std::f64::consts::FRAC_1_PI));
        assert_eq!(W64, BitWidth::from(std::f64::consts::E));
        assert_eq!(W64, BitWidth::from(std::f64::consts::SQRT_2));
    }
}
