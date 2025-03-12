use flatbuffers::{read_scalar_at, Follow, UOffsetT, Vector, SIZE_UOFFSET};

use crate::reflection::Object;

pub struct VectorOfAny<'a> {
    buf: &'a [u8],
    loc: usize,
}

impl<'a> VectorOfAny<'a> {
    /// # Safety
    ///
    /// [buf] must contain a valid vector at [loc]
    #[inline]
    pub unsafe fn new(buf: &'a [u8], loc: usize) -> Self {
        Self { buf, loc }
    }

    #[inline(always)]
    pub fn len(&self) -> usize {
        // Safety:
        // Valid vector at time of construction starting with UOffsetT element count
        unsafe { read_scalar_at::<UOffsetT>(self.buf, self.loc) as usize }
    }

    /// Get a slice of all the bytes from the start of the vector to the *end of the buffer*.
    ///
    /// We don't know the size of the elements in the vector, so we can't return just its contents.
    #[inline(always)]
    pub fn bytes(&self) -> &'a [u8] {
        &self.buf[self.loc + SIZE_UOFFSET..]
    }

    /// Get a slice of bytes corresponding to the struct at [index], assuming this is a vector of
    /// structs of type [obj].
    ///
    /// # Panics
    ///
    /// Panics if [index] is out of bounds or if [obj] is not a struct.
    #[inline(always)]
    pub fn get_struct(&self, index: usize, obj: Object<'_>) -> &'a [u8] {
        assert!(obj.is_struct());
        assert!(index < self.len());
        let bytesize = obj.bytesize() as usize;
        let start = self.loc + SIZE_UOFFSET + index * bytesize;
        let end = start + bytesize;
        assert!(end <= self.buf.len());
        &self.buf[start..end]
    }

    /// # Safety
    ///
    /// [buf] must contain a valid vector at [loc]
    pub unsafe fn as_vector<T: Follow<'a>>(&self) -> Vector<T> {
        Vector::new(self.buf, self.loc)
    }
}

impl<'a> Follow<'a> for VectorOfAny<'a> {
    type Inner = VectorOfAny<'a>;

    #[inline(always)]
    unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
        Self { buf, loc }
    }
}

impl Default for VectorOfAny<'_> {
    fn default() -> Self {
        Self { buf: &[], loc: 0 }
    }
}
