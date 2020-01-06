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

use crate::{
    deserialize::traits::{FromTableField, FromTableFieldUnion},
    errors::InvalidFlatbuffer,
};
use core::convert::TryInto;

#[derive(Copy, Clone)]
#[doc(hidden)]
pub struct Table<'a> {
    buf: &'a [u8],
    offset: usize,
    vtable: &'a [u8],
}

impl<'a> FromTableField<'a> for Table<'a> {
    const INLINE_SIZE: usize = u32::INLINE_SIZE;

    #[inline]
    fn from_field(buf: &'a [u8], offset: usize) -> Result<Table<'a>, InvalidFlatbuffer> {
        let offset = offset + u32::from_field(buf, offset)? as usize;
        let vtable_offset = (offset as u32).wrapping_sub(u32::from_field(buf, offset)?) as usize;
        let vtable_size = u16::from_field(buf, vtable_offset)? as usize;
        let vtable = buf
            .get(vtable_offset..vtable_offset + vtable_size)
            .ok_or(InvalidFlatbuffer)?;
        Ok(Table {
            buf,
            offset,
            vtable,
        })
    }
}

impl<'a> Table<'a> {
    #[inline]
    fn get_field_offset(&self, field: usize) -> Option<usize> {
        let field = self.vtable.get(field..field + 2)?;
        let field: &[u8; 2] = field.try_into().unwrap();
        let offset = u16::from_le_bytes(*field) as usize;
        if offset != 0 {
            Some(self.offset + offset)
        } else {
            None
        }
    }

    #[inline]
    pub fn get_field<T: FromTableField<'a>>(
        &self,
        field: usize,
    ) -> Result<Option<T>, InvalidFlatbuffer> {
        if let Some(offset) = self.get_field_offset(field) {
            T::from_field(self.buf, offset).map(Some)
        } else {
            Ok(None)
        }
    }

    #[inline]
    pub fn get_field_union<T: FromTableFieldUnion<'a>>(
        &self,
        tag_field: usize,
        value_field: usize,
    ) -> Result<Option<T>, InvalidFlatbuffer> {
        if let Some(tag_offset) = self.get_field_offset(tag_field) {
            if let Some(value_offset) = self.get_field_offset(value_field) {
                return T::from_field_union(self.buf, tag_offset, value_offset);
            }
        }
        Ok(None)
    }
}
