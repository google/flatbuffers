/*
 * Copyright 2020 Google Inc. All rights reserved.
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

use crate::errors::InvalidFlatbuffer;
use core::{convert::TryInto, mem};

#[doc(hidden)]
pub trait FromTableField<'a>: Sized {
    const INLINE_SIZE: usize;
    fn from_field(buffer: &'a [u8], offset: usize) -> Result<Self, InvalidFlatbuffer>;
}

#[doc(hidden)]
pub trait FromTableFieldUnion<'a>: Sized {
    fn from_field_union(
        buffer: &'a [u8],
        tag_offset: usize,
        value_offset: usize,
    ) -> Result<Option<Self>, InvalidFlatbuffer>;
}

#[doc(hidden)]
pub unsafe trait FromStructField<'a>: Sized {
    type Input; // Must be [u8; N]
    fn from_struct_field(slice: &'a Self::Input) -> Result<Self, InvalidFlatbuffer>;
}

macro_rules! from_buf_number {
    ($typ:ty) => {
        impl<'a> FromTableField<'a> for $typ {
            const INLINE_SIZE: usize = mem::size_of::<$typ>();

            #[inline]
            fn from_field(buf: &'a [u8], offset: usize) -> Result<$typ, InvalidFlatbuffer> {
                let buf = buf
                    .get(offset..offset + mem::size_of::<$typ>())
                    .ok_or(InvalidFlatbuffer)?;
                let buf: &[u8; mem::size_of::<$typ>()] =
                    buf.try_into().or(Err(InvalidFlatbuffer))?;
                Ok(<$typ>::from_le_bytes(*buf))
            }
        }

        unsafe impl<'a> FromStructField<'a> for $typ {
            type Input = [u8; mem::size_of::<$typ>()];

            #[inline]
            fn from_struct_field(slice: &'a Self::Input) -> Result<$typ, InvalidFlatbuffer> {
                Ok(Self::from_le_bytes(*slice))
            }
        }
    };
}

from_buf_number!(u8);
from_buf_number!(u16);
from_buf_number!(u32);
from_buf_number!(u64);

from_buf_number!(i8);
from_buf_number!(i16);
from_buf_number!(i32);
from_buf_number!(i64);

from_buf_number!(f32);
from_buf_number!(f64);

impl<'a> FromTableField<'a> for bool {
    const INLINE_SIZE: usize = mem::size_of::<bool>();

    #[inline]
    fn from_field(buf: &'a [u8], offset: usize) -> Result<bool, InvalidFlatbuffer> {
        Ok(*(buf.get(offset).ok_or(InvalidFlatbuffer)?) != 0)
    }
}

unsafe impl<'a> FromStructField<'a> for bool {
    type Input = [u8; 1];

    #[inline]
    fn from_struct_field(slice: &'a Self::Input) -> Result<bool, InvalidFlatbuffer> {
        Ok(slice[0] != 0)
    }
}

impl<'a> FromTableField<'a> for &'a [u8] {
    const INLINE_SIZE: usize = u32::INLINE_SIZE;

    #[inline]
    fn from_field(buf: &'a [u8], offset: usize) -> Result<&'a [u8], InvalidFlatbuffer> {
        let offset = offset + u32::from_field(buf, offset)? as usize;
        let count = u32::from_field(buf, offset)? as usize;
        let offset = offset + u32::INLINE_SIZE;
        buf.get(offset..offset + (count as usize))
            .ok_or(InvalidFlatbuffer)
    }
}

impl<'a> FromTableField<'a> for &'a str {
    const INLINE_SIZE: usize = u32::INLINE_SIZE;

    #[inline]
    fn from_field(buf: &'a [u8], offset: usize) -> Result<&'a str, InvalidFlatbuffer> {
        let slice: &'a [u8] = FromTableField::from_field(buf, offset)?;
        core::str::from_utf8(slice).or(Err(InvalidFlatbuffer))
    }
}
