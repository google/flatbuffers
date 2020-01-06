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

use crate::{deserialize::traits::FromTableField, errors::InvalidFlatbuffer};

#[cfg(feature = "std")]
use std::ffi::{CStr, FromBytesWithNulError};

/// Deserialized flatbuffer string
#[derive(Copy, Clone, PartialEq, Eq, Hash)]
pub struct Str<'a>(&'a [u8]);

impl<'a> Str<'a> {
    /// Interprets the flatbuffer string as a rust slice of bytes.
    #[inline]
    pub fn as_bytes(&self) -> &'a [u8] {
        if let Some((_, remaining)) = self.0.split_last() {
            remaining
        } else {
            &[]
        }
    }

    /// Interprets the flatbuffer string as a rust string.
    ///
    /// This checks at run-time that the contained slice is valid UTF-8.
    /// For valid flatbuffers, this function will always return `s`
    /// for some string `s`.
    ///
    /// # Panics
    ///
    /// If called on a malicious or otherwise invalid flatbuffer.
    #[inline]
    pub fn as_str(&self) -> &'a str {
        core::str::from_utf8(self.as_bytes()).unwrap()
    }

    /// Interprets the flatbuffer string as a rust string.
    ///
    /// This checks at run-time that the contained slice is valid UTF-8.
    /// For valid flatbuffers, this function will always return `Some(s)`
    /// for some string `s`.
    ///
    /// However if this function is called on a malicious or otherwise
    /// invalid flatbuffer, this function can return `None`.
    #[inline]
    pub fn try_as_str(&self) -> Option<&'a str> {
        core::str::from_utf8(self.as_bytes()).ok()
    }

    /// Interprets the flatbuffer string as a rust string without
    /// checking that string contains valid UTF-8.
    ///
    /// For a safe version, use `as_str`.
    ///
    /// # Safety
    ///
    /// This function is unsafe because it does not check that the bytes passed
    /// to it are valid UTF-8. If this constraint is violated, undefined
    /// behavior results, as the rest of Rust assumes that `&str`s are valid
    /// UTF-8.
    ///
    /// For valid flatbuffers, this function is always safe. However if
    /// this function is called on a malicious or otherwise invalid flatbuffer,
    /// the results is undefined behavior.
    #[inline]
    pub unsafe fn as_str_unchecked(&self) -> &'a str {
        core::str::from_utf8_unchecked(self.as_bytes())
    }

    /// Interprets the flatbuffer string as a c string.
    ///
    /// This checks at run-time that the contained slice does not contain any
    /// interior null bytes. If an interior null byte is found, and error is
    /// returned.
    #[inline]
    #[cfg(feature = "std")]
    pub fn as_cstr(&self) -> Result<&'a CStr, FromBytesWithNulError> {
        CStr::from_bytes_with_nul(self.0)
    }

    /// Interprets the flatbuffer string as a c string without checking for
    /// interior null bytes.
    ///
    /// In case the slice contains interior null bytes, a truncated string will
    /// be returned.
    #[inline]
    #[cfg(feature = "std")]
    pub fn as_cstr_truncated(&self) -> &'a CStr {
        // This is safe, because we already checked for null when we constructed
        // the Str value.
        unsafe { CStr::from_bytes_with_nul_unchecked(self.0) }
    }

    /// Returns the number of bytes the string.
    #[inline]
    pub fn len(&self) -> usize {
        // Do not count the null byte
        self.0.len() - 1
    }

    /// Returns `true` if the string has a length of 0.
    #[inline]
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }
}

impl<'a> core::fmt::Debug for Str<'a> {
    fn fmt(&self, formatter: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        let mut formatter = formatter.debug_tuple("Str");
        if let Some(s) = self.try_as_str() {
            formatter.field(&s);
        } else {
            formatter.field(&self.0);
        }
        formatter.finish()
    }
}

impl<'a> FromTableField<'a> for Str<'a> {
    const INLINE_SIZE: usize = 4;

    #[inline]
    fn from_field(buf: &'a [u8], offset: usize) -> Result<Str<'a>, InvalidFlatbuffer> {
        // Strings are serialized in the same way as vectors, i.e.
        // with a relative offset to a { count, [u8; count] } object.
        // However they must also include a trailing null byte, which
        // is not included in the count
        let offset = offset + u32::from_field(buf, offset)? as usize;
        let count = u32::from_field(buf, offset)? as usize;
        let offset = offset + u32::INLINE_SIZE; // Skip the count

        // Include null byte
        let slice = buf.get(offset..=offset + count).ok_or(InvalidFlatbuffer)?;

        // The check for a null byte is needed, because otherwise
        // the method as_cstr_truncated would be unsafe
        if slice.last() == Some(&0) {
            Ok(Str(slice))
        } else {
            Err(InvalidFlatbuffer)
        }
    }
}
