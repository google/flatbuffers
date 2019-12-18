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

use super::{unpack_type, Error, Reader};
use crate::{BitWidth, FlexBufferType};

#[derive(Default, Clone)]
pub struct VectorReader<'de> {
    pub(super) reader: Reader<'de>,
    // Cache the length because read_usize can be slow.
    pub(super) length: usize,
}

/// Allows indexing on any flexbuffer vector type, (heterogenous vector, typed vector, or fixed
/// length typed vector).
impl<'de> VectorReader<'de> {
    /// Returns the number of elements in the vector.
    pub fn len(&self) -> usize {
        self.length
    }
    /// Returns true if there are 0 elements in the vector.
    pub fn is_empty(&self) -> bool {
        self.length == 0
    }
    fn get_elem_type(&self, i: usize) -> Result<(FlexBufferType, BitWidth), Error> {
        if let Some(ty) = self.reader.fxb_type.typed_vector_type() {
            Ok((ty, self.reader.width))
        } else {
            let types_addr = self.reader.address + self.length * self.reader.width.n_bytes();
            let packed = self.reader.buffer[types_addr + i];
            unpack_type(packed)
        }
    }
    /// Index into a flexbuffer vector. Any errors are defaulted to Null Readers.
    pub fn idx(&self, i: usize) -> Reader<'de> {
        self.index(i).unwrap_or_default()
    }
    /// Index into a flexbuffer.
    pub fn index(&self, i: usize) -> Result<Reader<'de>, Error> {
        if i >= self.length {
            return Err(Error::IndexOutOfBounds);
        }
        let (fxb_type, bw) = self.get_elem_type(i)?;
        let data_address = self.reader.address + self.reader.width.n_bytes() * i;
        Reader::new(
            self.reader.buffer,
            data_address,
            fxb_type,
            bw,
            self.reader.width,
        )
    }
    pub(super) fn reader(&self) -> Reader<'de> {
        self.reader.clone()
    }
}
