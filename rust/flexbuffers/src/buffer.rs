use std::ops::{Deref, Range};

/// The underlying buffer that is used by a flexbuffer Reader. 
///
/// This allows for custom buffer implementations so long as they can be
/// viewed as a &[u8]. 
pub trait Buffer: Deref<Target = [u8]> + Clone + Default {
    type BufferString: Deref<Target = str> + Default;

    /// This method returns an instance of type Self. This allows for lifetimes
    /// to be tracked in cases of deserialization. 
    ///
    /// It also lets custom buffers manage reference counts. 
    ///
    /// Returns None if:
    /// - range start is greater than end
    /// - range end is out of bounds
    fn slice(&self, range: Range<usize>) -> Option<Self>;

    /// Attempts to convert the given buffer to a custom string type. 
    ///
    /// This should fail if the type does not have valid UTF-8 bytes. 
    fn buffer_str(&self) -> Result<Self::BufferString, std::str::Utf8Error>;
}

impl<'de> Buffer for &'de [u8] {
    type BufferString = &'de str;

    #[inline]
    fn slice(&self, range: Range<usize>) -> Option<Self> {
        self.get(range)
    }

    #[inline]
    fn buffer_str(&self) -> Result<Self::BufferString, std::str::Utf8Error> {
        Ok(std::str::from_utf8(self)?)
    }
}

