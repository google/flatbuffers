use std::iter::{Iterator};

use std::ops::{RangeBounds, Bound, Index};
use std::slice::SliceIndex;
use std::fmt::Debug;

use crate::reader::ReadLE;

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct FlexBufferSlice<'buf> {
    buf: &'buf [u8],
}

pub trait FlexBuffer<Unsigned = u8>: Debug + PartialEq + IntoIterator + AsRef<[Unsigned]> + Default + Clone {
    /// Gets the length of the current buffer.
    fn len(&self) -> usize;

    /// Retrieves a slice of memory based on RangeBounds.
    fn slice(&self, range: impl RangeBounds<usize>) -> Self;

    /// Similar to [T]::get, get a value at an index.
    fn get<I>(&self, index: I) -> Option<&<I as SliceIndex<[u8]>>::Output>
    where
        I: SliceIndex<[u8]>;

    /// Returns true if the flexbuffer is aligned to 8 bytes. This guarantees, for valid
    /// flexbuffers, that the data is correctly aligned in memory and slices can be read directly
    /// e.g. with `get_f64s` or `get_i16s`.
    fn is_aligned(&self) -> bool;

    fn align_to<L: ReadLE>(&self) -> (&[u8], &[L], &[u8]);
}

impl FlexBuffer for &[u8] {
    /// Gets the length of the current buffer.
    #[inline]
    fn len(&self) -> usize {
        self.len()
    }

    /// Retrieves a slice of memory based on RangeBounds.
    fn slice(&self, range: impl RangeBounds<usize>) -> Self {
        self.slice(range)
    }

    /// Similar to [T]::get, get a value at an index.
    fn get<I>(&self, index: I) -> Option<&<I as SliceIndex<[u8]>>::Output>
        where I: SliceIndex<[u8]>
    {
        self.get(index)
    }

    /// Returns true if the flexbuffer is aligned to 8 bytes. This guarantees, for valid
    /// flexbuffers, that the data is correctly aligned in memory and slices can be read directly
    /// e.g. with `get_f64s` or `get_i16s`.
    #[inline]
    fn is_aligned(&self) -> bool {
        self.is_aligned()
    }

    fn align_to<L: ReadLE>(&self) -> (&[u8], &[L], &[u8]) {
        self.align_to::<L>()
    }
}

//impl<'buf> FlexBuffer for FlexBufferSlice<'buf> {
    //type BufferType = &'buf [u8];

    //#[inline]
    //fn len(&self) -> usize { self.buf.len() }

    //#[inline]
    //fn slice(&self, range: impl RangeBounds<usize>) -> Self {
        //// Taken from bytes::Bytes `slice` implementation.
        //let len = self.len();

        //let begin = match range.start_bound() {
            //Bound::Included(&n) => n,
            //Bound::Excluded(&n) => n + 1,
            //Bound::Unbounded => 0,
        //};

        //let end = match range.end_bound() {
            //Bound::Included(&n) => n.checked_add(1).expect("out of range"),
            //Bound::Excluded(&n) => n,
            //Bound::Unbounded => len,
        //};

        //assert!(
            //begin <= end,
            //"range start must not be greater than end: {:?} <= {:?}",
            //begin,
            //end,
        //);
        //assert!(
            //end <= len,
            //"range end out of bounds: {:?} <= {:?}",
            //end,
            //len,
        //);

        //Self {
            //buf: &self.buf[begin..end],
        //}
    //}
//}

//#[cfg(feature = "tokio-bytes")]
//pub mod tokio_bytes {
    //use std::ops::{RangeBounds};

    //use bytes::{Bytes};
    //use super::FlexBuffer;

    //#[derive(Clone, Debug, PartialEq)]
    //pub struct FlexBufferBytes {
        //bytes: Bytes
    //}

    //impl FlexBuffer for FlexBufferBytes {
        //type BufferType = Bytes;

        //#[inline]
        //fn len(&self) -> usize {
            //self.bytes.len()
        //}

        //#[inline]
        //fn slice(&self, range: impl RangeBounds<usize>) -> Self {
            //Self { bytes: self.bytes.slice(range) }
        //}
    //}
//}

