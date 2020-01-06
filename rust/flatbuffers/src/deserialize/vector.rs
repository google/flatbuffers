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
use core::marker::PhantomData;

/// Deserialized flatbuffer vector
pub struct Vector<'a, T> {
    buf: &'a [u8],
    offset: usize,
    count: usize,
    marker: PhantomData<&'a T>,
}

impl<'a, T: FromTableField<'a> + core::fmt::Debug> core::fmt::Debug for Vector<'a, T> {
    fn fmt(&self, formatter: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        formatter.debug_list().entries(self.iter()).finish()
    }
}

impl<'a, T: FromTableField<'a> + PartialEq> PartialEq for Vector<'a, T> {
    fn eq(&self, other: &Vector<'a, T>) -> bool {
        self.iter().eq(other.iter())
    }
}

impl<'a, T> Clone for Vector<'a, T> {
    #[inline]
    fn clone(&self) -> Vector<'a, T> {
        *self
    }
}

impl<'a, T> Copy for Vector<'a, T> {}

impl<'a, T: FromTableField<'a>> FromTableField<'a> for Vector<'a, T> {
    const INLINE_SIZE: usize = 4;

    #[inline]
    fn from_field(buf: &'a [u8], offset: usize) -> Result<Vector<'a, T>, InvalidFlatbuffer> {
        let offset = offset + u32::from_field(buf, offset)? as usize;
        let count = u32::from_field(buf, offset)? as usize;
        let offset = offset + u32::INLINE_SIZE;
        let slice = buf
            .get(offset..offset + count * T::INLINE_SIZE)
            .ok_or(InvalidFlatbuffer)?;

        assert_eq!(slice.len() % T::INLINE_SIZE, 0);
        assert_eq!(slice.len() / T::INLINE_SIZE, count);

        Ok(Vector {
            buf,
            offset,
            count,
            marker: PhantomData,
        })
    }
}

impl<'a, T: FromTableField<'a>> Vector<'a, T> {
    /// Returns the number of elements in the vector.
    #[inline]
    pub fn len(&self) -> usize {
        self.count
    }

    /// Returns `true` if the vector has a length of 0.
    #[inline]
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }

    /// Returns the first element of the slice. Return `None` if the vector is
    /// empty.
    ///
    /// See also the [try_first](#try_first) function.
    ///
    /// # Panics
    ///
    /// If the value cannot be deserialized.
    #[inline]
    pub fn first(&self) -> Option<T> {
        self.try_get(0).map(|value| value.unwrap())
    }

    /// Returns the first element of the slice. Returns `None` if the vector is
    /// empty or `Some(Err(InvalidFlatbuffer))` if the data cannot be
    /// deserialized correctly.
    #[inline]
    pub fn try_first(&self) -> Option<Result<T, InvalidFlatbuffer>> {
        self.try_get(0)
    }

    /// Returns the last element of the slice. Returns `None` if the vector is
    /// empty.
    ///
    /// # Panics
    ///
    /// If the value cannot be deserialized
    #[inline]
    pub fn last(&self) -> Option<T> {
        let last_idx = self.len().checked_sub(1)?;
        self.try_get(last_idx).map(|value| value.unwrap())
    }

    /// Returns the last element of the slice, or `None` if it is empty.
    #[inline]
    pub fn try_last(&self) -> Option<Result<T, InvalidFlatbuffer>> {
        let last_idx = self.len().checked_sub(1)?;
        self.try_get(last_idx)
    }

    /// Returns a reference to an element or subslice depending on the type of
    /// index.
    ///
    /// - If given a position, returns a the element at that position.
    /// - If given a range, returns the vector corresponding to that range.
    ///
    /// # Panics
    ///
    /// If the value is out of bounds or cannot be deserialized.
    #[inline]
    pub fn get<I>(&self, index: I) -> I::Output
    where
        I: VectorIndex<'a, T>,
    {
        index.get(self).unwrap().unwrap()
    }

    /// Returns a reference to an element or subslice depending on the type of
    /// index.
    ///
    /// - If given a position, returns a reference to the element at that
    ///   position or `None` if out of bounds.
    /// - If given a range, returns the vector corresponding to that range, or
    ///   `None` if out of bounds.
    #[inline]
    pub fn try_get<I>(&self, index: I) -> Option<Result<I::Output, InvalidFlatbuffer>>
    where
        I: VectorIndex<'a, T>,
    {
        index.get(self)
    }

    /// Returns an iterator over the vector.
    ///
    /// # Panics
    ///
    /// Will panic on calls to `next()`, if the returned value cannot be
    /// deserialized.
    #[inline]
    pub fn iter(&self) -> VectorIter<'a, T> {
        VectorIter(self.try_iter())
    }

    /// Returns an iterator over the vector.
    #[inline]
    pub fn try_iter(&self) -> VectorTryIter<'a, T> {
        VectorTryIter {
            buf: self.buf,
            count: self.count,
            offset: self.offset,
            marker: PhantomData,
        }
    }
}

pub trait VectorIndex<'a, T> {
    type Output;
    fn get(self, vector: &Vector<'a, T>) -> Option<Result<Self::Output, InvalidFlatbuffer>>;
}

impl<'a, T: FromTableField<'a>> VectorIndex<'a, T> for usize {
    type Output = T;

    #[inline]
    fn get(self, vector: &Vector<'a, T>) -> Option<Result<Self::Output, InvalidFlatbuffer>> {
        if self < vector.len() {
            Some(T::from_field(
                vector.buf,
                vector.offset + T::INLINE_SIZE * self,
            ))
        } else {
            None
        }
    }
}

impl<'a, T: 'a + FromTableField<'a>> VectorIndex<'a, T> for core::ops::Range<usize> {
    type Output = Vector<'a, T>;

    #[inline]
    fn get(self, vector: &Vector<'a, T>) -> Option<Result<Self::Output, InvalidFlatbuffer>> {
        if self.start <= self.end && self.end <= vector.len() {
            Some(Ok(Vector {
                buf: vector.buf,
                offset: self.start * T::INLINE_SIZE,
                count: (self.end - self.start),
                marker: PhantomData,
            }))
        } else {
            None
        }
    }
}

impl<'a, T: 'a + FromTableField<'a>> VectorIndex<'a, T> for core::ops::RangeFrom<usize> {
    type Output = Vector<'a, T>;

    #[inline]
    fn get(self, vector: &Vector<'a, T>) -> Option<Result<Self::Output, InvalidFlatbuffer>> {
        (self.start..vector.len()).get(vector)
    }
}

impl<'a, T: 'a + FromTableField<'a>> VectorIndex<'a, T> for core::ops::RangeInclusive<usize> {
    type Output = Vector<'a, T>;

    #[inline]
    fn get(self, vector: &Vector<'a, T>) -> Option<Result<Self::Output, InvalidFlatbuffer>> {
        if self.start() <= self.end() && *self.end() < vector.len() {
            Some(Ok(Vector {
                buf: vector.buf,
                offset: self.start() * T::INLINE_SIZE,
                count: (1 + self.end() - self.start()),
                marker: PhantomData,
            }))
        } else {
            None
        }
    }
}

impl<'a, T: 'a + FromTableField<'a>> VectorIndex<'a, T> for core::ops::RangeTo<usize> {
    type Output = Vector<'a, T>;

    #[inline]
    fn get(self, vector: &Vector<'a, T>) -> Option<Result<Self::Output, InvalidFlatbuffer>> {
        (0..self.end).get(vector)
    }
}

impl<'a, T: 'a + FromTableField<'a>> VectorIndex<'a, T> for core::ops::RangeToInclusive<usize> {
    type Output = Vector<'a, T>;

    #[inline]
    fn get(self, vector: &Vector<'a, T>) -> Option<Result<Self::Output, InvalidFlatbuffer>> {
        (0..=self.end).get(vector)
    }
}

impl<'a, T: 'a + FromTableField<'a>> VectorIndex<'a, T> for core::ops::RangeFull {
    type Output = Vector<'a, T>;

    #[inline]
    fn get(self, vector: &Vector<'a, T>) -> Option<Result<Self::Output, InvalidFlatbuffer>> {
        Some(Ok(*vector))
    }
}

pub struct VectorTryIter<'a, T> {
    buf: &'a [u8],
    offset: usize,
    count: usize,
    marker: PhantomData<&'a T>,
}

impl<'a, T: FromTableField<'a>> Iterator for VectorTryIter<'a, T> {
    type Item = Result<T, InvalidFlatbuffer>;

    #[inline]
    fn next(&mut self) -> Option<Result<T, InvalidFlatbuffer>> {
        if self.count == 0 {
            None
        } else {
            let result = T::from_field(self.buf, self.offset);
            self.offset += T::INLINE_SIZE;
            self.count -= 1;
            Some(result)
        }
    }

    #[inline]
    fn size_hint(&self) -> (usize, Option<usize>) {
        (self.count, Some(self.count))
    }
}

impl<'a, T: FromTableField<'a>> DoubleEndedIterator for VectorTryIter<'a, T> {
    #[inline]
    fn next_back(&mut self) -> Option<Result<T, InvalidFlatbuffer>> {
        if self.count > 0 {
            self.count -= 1;
            Some(T::from_field(
                self.buf,
                self.offset + self.count * T::INLINE_SIZE,
            ))
        } else {
            None
        }
    }
}

impl<'a, T: FromTableField<'a>> ExactSizeIterator for VectorTryIter<'a, T> {}
impl<'a, T: FromTableField<'a>> core::iter::FusedIterator for VectorTryIter<'a, T> {}

pub struct VectorIter<'a, T>(VectorTryIter<'a, T>);

impl<'a, T: FromTableField<'a>> Iterator for VectorIter<'a, T> {
    type Item = T;

    #[inline]
    fn next(&mut self) -> Option<T> {
        self.0.next().map(|value| value.unwrap())
    }

    #[inline]
    fn size_hint(&self) -> (usize, Option<usize>) {
        self.0.size_hint()
    }
}

impl<'a, T: FromTableField<'a>> DoubleEndedIterator for VectorIter<'a, T> {
    #[inline]
    fn next_back(&mut self) -> Option<T> {
        self.0.next_back().map(|value| value.unwrap())
    }
}

impl<'a, T: FromTableField<'a>> ExactSizeIterator for VectorIter<'a, T> {}
impl<'a, T: FromTableField<'a>> core::iter::FusedIterator for VectorIter<'a, T> {}
