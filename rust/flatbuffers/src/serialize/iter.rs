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

use crate::serialize::buffer::Buffer;

/// An iterator over the slices of a generated message
pub struct FlatbufferIter<'a, B: Buffer<'a>> {
    pub(crate) iter: Option<B::Iterator>,
}

impl<'a, B> Iterator for FlatbufferIter<'a, B>
where
    B: Buffer<'a>,
{
    type Item = &'a [u8];

    #[inline]
    fn next(&mut self) -> Option<&'a [u8]> {
        match self.iter.as_mut().and_then(|i| i.next()) {
            Some(value) => Some(value),
            None => {
                self.iter = None;
                None
            }
        }
    }

    #[inline]
    fn size_hint(&self) -> (usize, Option<usize>) {
        if let Some(iter) = &self.iter {
            iter.size_hint()
        } else {
            (0, Some(0))
        }
    }
}

impl<'a, B> core::iter::ExactSizeIterator for FlatbufferIter<'a, B>
where
    B: Buffer<'a>,
    B::Iterator: core::iter::ExactSizeIterator,
{
}

impl<'a, B> core::iter::FusedIterator for FlatbufferIter<'a, B> where B: Buffer<'a> {}

// The derived version would have the wrong generic bounds
impl<'a, B: Buffer<'a>> core::fmt::Debug for FlatbufferIter<'a, B>
where
    B::Iterator: core::fmt::Debug,
{
    fn fmt(&self, formatter: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        formatter
            .debug_struct(" FlatbufferIter")
            .field("iter", &self.iter)
            .finish()
    }
}
