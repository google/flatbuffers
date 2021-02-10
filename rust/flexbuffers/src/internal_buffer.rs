use std::ops::{RangeBounds, Bound};

/// The underlying buffer that is used by a flexbuffer Reader. 
///
/// This allows for custom buffer implementations so long as they can be
/// viewed as a &[u8]. 
pub trait InternalBuffer: AsRef<[u8]> + Clone + Default {
    /// This method returns an instance of type Self. This allows for lifetimes
    /// to be tracked in cases of deserialization. 
    ///
    /// It also lets custom buffers manage reference counts. 
    fn slice(&self, range: impl RangeBounds<usize>) -> Self;
}

/// Helper function for getting the pair of points for the beginning / end
/// of the given range. 
#[inline]
fn get_slice_pair(range: impl RangeBounds<usize>, len: usize) -> (usize, usize) {
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

    debug_assert!(
        begin <= end,
        "range start must not be greater than end: {:?} <= {:?}",
        begin,
        end,
    );

    debug_assert!(
        end <= len,
        "range end out of bounds: {:?} <= {:?}",
        end,
        len,
    );

    (begin, end)
}

impl<'de> InternalBuffer for &'de [u8] {

    #[inline]
    fn slice(&self, range: impl RangeBounds<usize>) -> Self {
        let (begin, end) = get_slice_pair(range, self.len());
        &self[begin..end]
    }
}

