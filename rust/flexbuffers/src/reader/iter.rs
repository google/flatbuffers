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

use super::{Reader, VectorReader};
use std::iter::{DoubleEndedIterator, ExactSizeIterator, FusedIterator, Iterator};

// TODO(cneo): MapReader.iter_values, MapReader.iter_keys, VectorReader.iter
/// Iterates over a flexbuffer vector, typed vector, or map. Yields Readers.
pub struct ReaderIterator<'de> {
    pub(super) reader: VectorReader<'de>,
    pub(super) front: usize,
    end: usize, // TODO redundant?
}
impl<'de> ReaderIterator<'de> {
    pub(super) fn new(reader: VectorReader<'de>) -> Self {
        let end = reader.len();
        ReaderIterator {
            reader,
            front: 0,
            end,
        }
    }
}
impl<'de> Iterator for ReaderIterator<'de> {
    type Item = Reader<'de>;
    fn next(&mut self) -> Option<Self::Item> {
        if self.front < self.end {
            let r = self.reader.idx(self.front);
            self.front += 1;
            Some(r)
        } else {
            None
        }
    }
    fn size_hint(&self) -> (usize, Option<usize>) {
        let remaining = self.end - self.front;
        (remaining, Some(remaining))
    }
}
impl<'de> DoubleEndedIterator for ReaderIterator<'de> {
    fn next_back(&mut self) -> Option<Self::Item> {
        if self.front < self.end {
            self.end -= 1;
            Some(self.reader.idx(self.end))
        } else {
            None
        }
    }
}
impl<'de> ExactSizeIterator for ReaderIterator<'de> {}
impl<'de> FusedIterator for ReaderIterator<'de> {}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::Builder;

    #[test]
    fn iter() {
        let mut fxb = Builder::default();
        let mut m = fxb.build_map();
        m.push("a", "42");
        m.push("b", 250i64);
        m.push("c", 5000u16);
        m.end();

        let r = Reader::get_root(fxb.view()).unwrap();

        let v: Vec<u32> = r.iter().map(|x| x.as_u32()).collect();
        assert_eq!(&v, &[42, 250, 5000]);
    }
}
