use std::fmt::Debug;
use std::ops::{Deref, RangeBounds, Bound};

/// The underlying buffer that is used by a flexbuffer Reader. 
///
/// This allows for custom buffer implementations so long as they can be
/// viewed as a &[u8]. 
pub trait InternalBuffer: Deref<Target = [u8]> + AsRef<[u8]> + Clone + Default + Debug {
    type BufferString: Deref<Target = str> + AsRef<str> + Default + Debug;

    /// This method returns an instance of type Self. This allows for lifetimes
    /// to be tracked in cases of deserialization. 
    ///
    /// It also lets custom buffers manage reference counts. 
    ///
    /// Returns None if:
    /// - range start is greater than end
    /// - range end is out of bounds
    fn slice(&self, range: impl RangeBounds<usize>) -> Option<Self>;

    /// Attempts to convert the given buffer to a custom string type. 
    ///
    /// This should fail if the type does not have valid UTF-8 bytes. 
    fn as_str(&self) -> Result<Self::BufferString, std::str::Utf8Error>;
}

/// Helper function for getting the pair of points for the beginning / end
/// of the given range. 
///
/// Returns None if:
/// - range start is greater than end
/// - range end is out of bounds
#[inline]
fn get_slice_pair(range: impl RangeBounds<usize>, len: usize) -> Option<(usize, usize)> {
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

    if begin <= end && end <= len {
        Some((begin, end))
    } else {
        None
    }
}

impl<'de> InternalBuffer for &'de [u8] {
    type BufferString = &'de str;

    #[inline]
    fn slice(&self, range: impl RangeBounds<usize>) -> Option<Self> {
        if let Some((begin, end)) = get_slice_pair(range, self.len()) {
            Some(&self[begin..end])
        } else {
            None
        }
    }

    #[inline]
    fn as_str(&self) -> Result<Self::BufferString, std::str::Utf8Error> {
        Ok(std::str::from_utf8(self)?)
    }
}

