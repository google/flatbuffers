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

use flatbuffers::Follow;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Struct<'a> {
    buf: &'a [u8],
    loc: usize,
}

impl<'a> Struct<'a> {
    #[inline]
    pub fn buf(&self) -> &'a [u8] {
        self.buf
    }

    #[inline]
    pub fn loc(&self) -> usize {
        self.loc
    }

    /// # Safety
    ///
    /// [buf] must contain a valid struct at [loc]
    #[inline]
    pub unsafe fn new(buf: &'a [u8], loc: usize) -> Self {
        Struct { buf, loc }
    }

    /// Retrieves the value at the provided [byte_loc]. No field in [Struct] is optional.
    ///
    /// # Safety
    ///
    /// The value of the corresponding slot must have type T
    #[inline]
    pub unsafe fn get<T: Follow<'a> + 'a>(&self, byte_loc: usize) -> T::Inner {
        <T>::follow(self.buf, self.loc + byte_loc)
    }
}

impl<'a> Follow<'a> for Struct<'a> {
    type Inner = Struct<'a>;
    #[inline]
    unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
        Struct { buf, loc }
    }
}
