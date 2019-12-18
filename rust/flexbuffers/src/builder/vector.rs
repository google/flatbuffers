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

use super::{Builder, MapBuilder, Pushable};

/// Builds a Flexbuffer vector. When this is dropped, or `end` is called, the vector is
/// commited to the buffer. If this vector is the root of the flexbuffer then the
/// root is written and the flexbuffer is complete. The FlexBufferType of this vector
/// is determined by the pushed values when this is dropped.
pub struct VectorBuilder<'a> {
    pub(crate) builder: &'a mut Builder,
    // If the root is this vector then start == None. Otherwise start is the
    // number of values in the 'values stack' before adding this vector.
    pub(crate) start: Option<usize>,
}
impl<'a> VectorBuilder<'a> {
    /// Pushes `p` onto the vector.
    pub fn push<P: Pushable>(&mut self, p: P) {
        self.builder.push(p);
    }
    /// Starts a nested vector which is pushed onto this vector when dropped.
    pub fn nest_vector(&mut self) -> VectorBuilder {
        let start = Some(self.builder.values.len());
        VectorBuilder {
            builder: &mut self.builder,
            start,
        }
    }
    /// Starts a nested map which is pushed onto this vector when dropped.
    pub fn nest_map(&mut self) -> MapBuilder {
        let start = Some(self.builder.values.len());
        MapBuilder {
            builder: &mut self.builder,
            start,
        }
    }
    /// `end` determines the type of the vector and writes it to the buffer.
    /// This will happen automatically if the VectorBuilder is dropped.
    pub fn end(self) {}
}
impl<'a> Drop for VectorBuilder<'a> {
    fn drop(&mut self) {
        self.builder.end_map_or_vector(false, self.start);
    }
}
