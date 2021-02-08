use std::ops::{RangeBounds, Bound};
use std::fmt::Debug;

/// The underlying buffer that is used by a flexbuffer Reader. 
pub trait FlexBuffer: Debug + PartialEq + IntoIterator + AsRef<[u8]> + Default + Clone {
    /// Retrieves a slice of memory based on RangeBounds.
    fn slice(&self, range: impl RangeBounds<usize>) -> Self;
}

impl FlexBuffer for &[u8] {
    /// Retrieves a slice of memory based on RangeBounds.
    #[inline]
    fn slice(&self, range: impl RangeBounds<usize>) -> Self {
        // Taken from bytes::Bytes `slice` implementation.
        let len = self.len();

        let begin = match range.start_bound() {
            Bound::Included(&n) => n,
            Bound::Excluded(&n) => n + 1,
            Bound::Unbounded => 0,
        };

        let end = match range.end_bound() {
            Bound::Included(&n) => n.checked_add(1).expect("out of range"),
            Bound::Excluded(&n) => n,
            Bound::Unbounded => len,
        };

        assert!(
            begin <= end,
            "range start must not be greater than end: {:?} <= {:?}",
            begin,
            end,
        );
        assert!(
            end <= len,
            "range end out of bounds: {:?} <= {:?}",
            end,
            len,
        );

        &self[begin..end]
    }
}

