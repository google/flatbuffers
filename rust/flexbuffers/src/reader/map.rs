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

use super::{deref_offset, unpack_type, Error, Reader, ReaderIterator, VectorReader};
use crate::BitWidth;
use std::cmp::Ordering;
use std::iter::{DoubleEndedIterator, ExactSizeIterator, FusedIterator, Iterator};

/// Allows indexing on a flexbuffer map.
#[derive(DebugStub, Default, Clone)]
pub struct MapReader<'de> {
    #[debug_stub = "&[..]"]
    pub(super) buffer: &'de [u8],
    pub(super) values_address: usize,
    pub(super) keys_address: usize,
    pub(super) values_width: BitWidth,
    pub(super) keys_width: BitWidth,
    pub(super) length: usize,
}

impl<'de> MapReader<'de> {
    /// Returns the number of key/value pairs are in the map.
    pub fn len(&self) -> usize {
        self.length
    }
    /// Returns true if the map has zero key/value pairs.
    pub fn is_empty(&self) -> bool {
        self.length == 0
    }
    // Using &CStr will eagerly compute the length of the key. &str needs length info AND utf8
    // validation. This version is faster than both.
    fn lazy_strcmp(&self, key_addr: usize, key: &str) -> Ordering {
        let k = self.buffer[key_addr..].iter().take_while(|&&b| b != b'\0');
        k.cmp(key.as_bytes().iter())
    }
    fn binary_search(&self, key: &str) -> Result<usize, Error> {
        let (mut low, mut high) = (0, self.length);
        while low < high {
            let i = (low + high) / 2;
            let key_offset_address = self.keys_address + i * self.keys_width.n_bytes();
            let key_address = deref_offset(self.buffer, key_offset_address, self.keys_width)?;
            match self.lazy_strcmp(key_address, key) {
                Ordering::Equal => return Ok(i),
                Ordering::Less => low = if i == low { i + 1 } else { i },
                Ordering::Greater => high = i,
            }
        }
        Err(Error::KeyNotFound)
    }
    /// Index into a map with a key or usize.
    pub fn index<I: MapReaderIndexer>(&self, i: I) -> Result<Reader<'de>, Error> {
        i.index_map_reader(self)
    }
    /// Index into a map with a key or usize. If any errors occur a Null reader is returned.
    pub fn idx<I: MapReaderIndexer>(&self, i: I) -> Reader<'de> {
        i.index_map_reader(self).unwrap_or_default()
    }
    fn usize_index(&self, i: usize) -> Result<Reader<'de>, Error> {
        if i >= self.length {
            return Err(Error::IndexOutOfBounds);
        }
        let data_address = self.values_address + self.values_width.n_bytes() * i;
        let type_address = self.values_address + self.values_width.n_bytes() * self.length + i;
        // TODO(cneo): This can totally be out of bounds.
        let (fxb_type, width) = unpack_type(self.buffer[type_address])?;
        Reader::new(
            &self.buffer,
            data_address,
            fxb_type,
            width,
            self.values_width,
        )
    }
    fn key_index(&self, k: &str) -> Result<Reader<'de>, Error> {
        let i = self.binary_search(k)?;
        self.usize_index(i)
    }
    /// Iterate over the values of the map. If any error occurs, Null Readers are returned.
    pub fn iter_values(&self) -> ReaderIterator<'de> {
        ReaderIterator::new(super::VectorReader {
            reader: Reader {
                buffer: self.buffer,
                fxb_type: crate::FlexBufferType::Map,
                width: self.values_width,
                address: self.values_address,
            },
            length: self.length,
        })
    }
    /// Iterate over the keys of the map. If any error occurs, empty strings are returned.
    pub fn iter_keys(
        &self,
    ) -> impl Iterator<Item = &'de str> + DoubleEndedIterator + ExactSizeIterator + FusedIterator
    {
        let ri = ReaderIterator::new(VectorReader {
            reader: Reader {
                buffer: self.buffer,
                fxb_type: crate::FlexBufferType::VectorKey,
                width: self.keys_width,
                address: self.keys_address,
            },
            length: self.length,
        });
        ri.map(|k| k.as_str())
    }
}
pub trait MapReaderIndexer {
    fn index_map_reader<'de>(self, r: &MapReader<'de>) -> Result<Reader<'de>, Error>;
}
impl MapReaderIndexer for usize {
    fn index_map_reader<'de>(self, r: &MapReader<'de>) -> Result<Reader<'de>, Error> {
        r.usize_index(self)
    }
}
impl MapReaderIndexer for &str {
    fn index_map_reader<'de>(self, r: &MapReader<'de>) -> Result<Reader<'de>, Error> {
        r.key_index(self)
    }
}
