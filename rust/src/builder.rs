//! Flatbuffer Builder.
//!
//! Builder is a state machine for creating FlatBuffer objects.
//! Use a Builder to construct object(s) starting from leaf nodes.
use std::mem;
use byteorder::{ByteOrder, LittleEndian};

use types::*;

/// Flatbuffer builder.
///
/// A Builder constructs byte buffers in a last-first manner for simplicity and
/// performance.
#[derive(Debug)]
pub struct Builder {
    // Where the FlatBuffer is constructed.
    bytes: Vec<u8>,
    // Minimum alignment encountered so far.
    min_align: usize,
    // Starting offset of the current struct/table.
    object_start: usize,
    // Whether we are currently serializing a table.
    nested: bool,
    // Whether the buffer is finished.
    finished: bool,
    // The vtable for the current table.
    vtable: Vec<UOffsetT>,
    // The number of vtable slots required by the current object.
    vtable_in_use: usize,
    // Offsets of other Vtables in the buffer.
    vtables: Vec<UOffsetT>,
    // Unused space in the buffer. Size from beggining of buffer to first vtable
    space: usize,
    // Number of elements in the current vector
    vector_len: usize,
}

impl Builder {
    /// Start a new FlatBuffer `Builder` backed by a buffer with an
    /// initial capacity of `size`.
    pub fn with_capacity(size: usize) -> Self {
        let mut bytes = Vec::with_capacity(size);
        unsafe { bytes.set_len(size) };
        Builder {
            bytes: bytes,
            min_align: 1,
            object_start: 0,
            nested: false,
            finished: false,
            vtable_in_use: 0,
            vtable: Vec::with_capacity(16),
            vtables: Vec::with_capacity(16),
            space: size,
            vector_len: 0,
        }
    }

    /// Start encoding a new object in the buffer.
    pub fn start_object(&mut self, num_fields: usize) {
        self.assert_not_nested();
        self.nested = true;
        self.finished = false;
        self.vtable.clear();
        self.vtable.resize(num_fields, 0);
        self.vtable_in_use = num_fields;
        self.object_start = self.offset();
        self.min_align = 1;
    }

    /// finish off writing the object that is under construction.
    ///
    /// Returns the offset of the object in the inside the buffer.
    pub fn end_object(&mut self) -> UOffsetT {
        self.assert_nested();
        self.nested = false;
        self.write_vtable()
    }

    /// Initializes bookkeeping for writing a new vector.
    pub fn start_vector(&mut self, elem_size: usize, num_elems: usize, alignment: usize) {
        self.assert_not_nested();
        self.nested = true;
        self.finished = false;
        self.vector_len = num_elems;
        self.prep(UOFFSETT_SIZE, elem_size * num_elems);
        self.prep(alignment, elem_size * num_elems);
    }

    /// finish off writing the current vector.
    pub fn end_vector(&mut self) -> UOffsetT {
        self.assert_nested();

        let len = self.vector_len;
        self.put_u32(len as u32);

        self.nested = false;
        self.offset() as UOffsetT
    }

    /// Create a string in the buffer from an already encoded UTF-8 `String`.
    pub fn create_string(&mut self, value: &str) -> UOffsetT {
        let len = value.len();
        self.add_u8(0);
        self.start_vector(1, len, 1);
        self.space -= len;
        for (i, c) in value.bytes().enumerate() {
            let pos = self.space + i;
            self.place_u8(pos, c);
        }
        self.end_vector()
    }

    /// Finalize a buffer, pointing to the given `root_table`.
    pub fn finish(&mut self, root_table: UOffsetT) {
        if !self.finished {
            self.assert_not_nested();
            let align = self.min_align;
            self.prep(align, UOFFSETT_SIZE);
            self.add_uoffset(root_table);
            self.finished = true;
        }
    }

    /// Consume the builder and return the finished flatbuffer.
    pub fn get_bytes(&self) -> &[u8] {
        assert!(self.finished, "Flatbuffer is not been finished");
        &self.bytes
    }

    /// Returns the length of the buffer.
    pub fn len(&self) -> usize {
        self.bytes.len()
    }

    /// Returns the current finished buffer and replaces it with
    /// a `new_buffer`.
    ///
    /// Thie function facilitates some reuse of the `Builder` object.
    /// Use `into()` if the `Builder` is no longer required.
    pub fn swap_out(&mut self, mut new_buffer: Vec<u8>) -> Vec<u8> {
        mem::swap(&mut self.bytes, &mut new_buffer);
        self.reset();
        new_buffer
    }

    /// Resets the builder.
    ///
    /// Clears the buffer without resizing. This allows for reuse
    /// without allocating new memeory.
    pub fn reset(&mut self) {
        self.min_align = 1;
        self.object_start = 0;
        self.nested = false;
        self.finished = false;
        self.vtable_in_use = 0;
        unsafe {
            self.vtable.set_len(0);
            self.vtables.set_len(0);
        }
        self.space = self.bytes.capacity();
        self.vector_len = 0;
    }

    /// Offset relative to the end of the buffer
    #[inline(always)]
    pub fn offset(&self) -> usize {
        self.bytes.len() - self.space
    }

    /// prepare to write an element of `size` after `additional_bytes`
    /// have been written.
    pub fn prep(&mut self, size: usize, additional_bytes: usize) {
        if size > self.min_align {
            self.min_align = size
        }
        let align_size = self.offset() + additional_bytes + 1; 
        let align_size = ((align_size ^ 1)) & (size - 1);
        while self.space <= align_size + size + additional_bytes {
            self.grow();
        }
        self.pad(align_size)
    }

    /// pad places zeros at the current offset.
    pub fn pad(&mut self, n: usize) {
        for _ in 0..n {
            self.space -= 1;
            let pos = self.space;
            self.place_u8(pos, 0)
        }
    }
}

impl Builder {
    /// Add a `bool` to the buffer, backwards from the current location.
    /// Doesn't align nor check for space.
    pub fn put_bool(&mut self, boolean: bool) {
        self.space -= 1;
        let head = self.space;
        self.bytes[head] = if boolean {
            1
        } else {
            0
        };
    }

    /// Add a `byte` to the buffer, backwards from the current location.
    /// Doesn't align nor check for space.
    pub fn put_u8(&mut self, byte: u8) {
        self.space -= 1;
        let head = self.space;
        self.bytes[head] = byte;
    }

    /// Add a `value` of type `i8` to the buffer, backwards from the current
    /// location. Doesn't align nor check for space.
    pub fn put_i8(&mut self, value: i8) {
        self.put_u8(value as u8)
    }

    /// Add a `value` of type `u16` to the buffer, backwards from the current
    /// location. Doesn't align nor check for space.
    pub fn put_u16(&mut self, value: u16) {
        self.space -= 2;
        let head = self.space;
        LittleEndian::write_u16(&mut self.bytes[head..head + 2], value);
    }

    /// Add a `value` of type `i16` to the buffer, backwards from the current
    /// location. Doesn't align nor check for space.
    pub fn put_i16(&mut self, value: i16) {
        self.put_u16(value as u16)
    }

    /// Add a `value` of type `u32` to the buffer, backwards from the current
    /// location. Doesn't align nor check for space.
    pub fn put_u32(&mut self, value: u32) {
        self.space -= 4;
        let head = self.space;
        LittleEndian::write_u32(&mut self.bytes[head..head + 4], value);
    }

    /// Add a `value` of type `i32` to the buffer, backwards from the current
    /// location. Doesn't align nor check for space.
    pub fn put_i32(&mut self, value: i32) {
        self.put_u32(value as u32)
    }

    /// Add a `value` of type `u64` to the buffer, backwards from the current
    /// location. Doesn't align nor check for space.
    pub fn put_u64(&mut self, value: u64) {
        self.space -= 8;
        let head = self.space;
        LittleEndian::write_u64(&mut self.bytes[head..head + 8], value);

    }

    /// Add a `value` of type `i64` to the buffer, backwards from the current
    /// location. Doesn't align nor check for space.
    pub fn put_i64(&mut self, value: i64) {
        self.put_u64(value as u64)
    }

    /// Add a `value` of type `f32` to the buffer, backwards from the current
    /// location. Doesn't align nor check for space.
    pub fn put_f32(&mut self, value: f32) {
        self.space -= 4;
        let head = self.space;
        LittleEndian::write_f32(&mut self.bytes[head..head + 4], value);
    }

    /// Add a `value` of type `f64` to the buffer, backwards from the current
    /// location. Doesn't align nor check for space.
    pub fn put_f64(&mut self, value: f64) {
        self.space -= 8;
        let head = self.space;
        LittleEndian::write_f64(&mut self.bytes[head..head + 8], value);
    }

    /// Add a `value` of type `UOffsetT` to the buffer, backwards from the
    /// current location. Doesn't align nor check for space.
    pub fn put_uoffset(&mut self, value: UOffsetT) {
        self.space -= 4;
        let head = self.space;
        LittleEndian::write_u32(&mut self.bytes[head..head + 4], value);
    }

    /// Add a `bool` to the buffer, properly aligned, and grows the
    /// buffer (if necessary).
    pub fn add_bool(&mut self, value: bool) {
        self.prep(1, 0);
        self.put_bool(value);
    }

    /// Add a value of type `u8` to the buffer, properly aligned, and grows the
    /// buffer (if necessary).
    pub fn add_u8(&mut self, value: u8) {
        self.prep(1, 0);
        self.put_u8(value);
    }

    /// Add a value of type `i8` to the buffer, properly aligned, and grows the
    /// buffer (if necessary).
    pub fn add_i8(&mut self, value: i8) {
        self.add_u8(value as u8);
    }

    /// Add a value of type `u16` to the buffer, properly aligned, and grows the
    /// buffer (if necessary).
    pub fn add_u16(&mut self, value: u16) {
        self.prep(2, 0);
        self.put_u16(value);
    }

    /// Add a value of type `i16` to the buffer, properly aligned, and grows the
    /// buffer (if necessary).
    pub fn add_i16(&mut self, value: i16) {
        self.add_u16(value as u16);
    }

    /// Add a value of type `u32` to the buffer, properly aligned, and grows the
    /// buffer (if necessary).
    pub fn add_u32(&mut self, value: u32) {
        self.prep(4, 0);
        self.put_u32(value);
    }

    /// Add a value of type `i32` to the buffer, properly aligned, and grows the
    /// buffer (if necessary).
    pub fn add_i32(&mut self, value: i32) {
        self.add_u32(value as u32);
    }

    /// Add a value of type `u64` to the buffer, properly aligned, and grows the
    /// buffer (if necessary).
    pub fn add_u64(&mut self, value: u64) {
        self.prep(8, 0);
        self.put_u64(value);
    }

    /// Add a value of type `i64` to the buffer, properly aligned, and grows the
    /// buffer (if necessary).
    pub fn add_i64(&mut self, value: i64) {
        self.add_u64(value as u64);
    }

    /// Add a value of type `f32` to the buffer, properly aligned, and grows the
    /// buffer (if necessary).
    pub fn add_f32(&mut self, value: f32) {
        self.prep(4, 0);
        self.put_f32(value);
    }

    /// Add a value of type `f64` to the buffer, properly aligned, and grows the
    /// buffer (if necessary).
    pub fn add_f64(&mut self, value: f64) {
        self.prep(8, 0);
        self.put_f64(value);
    }

    /// Add a value of type `f64` to the buffer, properly aligned, and grows the
    /// buffer (if necessary).
    /// prepends an UOffsetT, relative to where it will be written.
    pub fn add_uoffset(&mut self, value: UOffsetT) {
        self.prep(UOFFSETT_SIZE, 0);
		assert!(value <= self.offset() as UOffsetT, "unreachable: off <= boffset();");
        let relative = self.offset() as u32 - value as u32 + UOFFSETT_SIZE as u32;
        self.put_uoffset(relative);
    }

    /// Slot sets the vtable key `voffset` to the current location in the buffer.
    pub fn slot(&mut self, slot: usize) {
        self.vtable[slot] = self.offset() as UOffsetT;
    }

    /// Add a `bool` onto the object at vtable slot `o`. If value `x` equals
    /// default `d`, then the slot will be set to zero and no other data
    /// will be written.
    pub fn add_slot_bool(&mut self, o: usize, value: bool, d: bool) {
        if value != d {
            self.add_bool(value);
            self.slot(o)
        }
    }

    /// Add a value of type `u8` onto the object at vtable slot `o`. If value
    /// `x` equals default `d`, then the slot will be set to zero and no other
    /// data will be written.
    pub fn add_slot_u8(&mut self, o: usize, value: u8, d: u8) {
        if value != d {
            self.add_u8(value);
            self.slot(o)
        }
    }

    /// Add a value of type `i8` onto the object at vtable slot `o`. If value
    /// `x` equals default `d`, then the slot will be set to zero and no other
    /// data will be written.
    pub fn add_slot_i8(&mut self, o: usize, value: i8, d: i8) {
        self.add_slot_u8(o, value as u8, d as u8);
    }

    /// Add a value of type `u16` onto the object at vtable slot `o`. If value
    /// `x` equals default `d`, then the slot will be set to zero and no other
    /// data will be written.
    pub fn add_slot_u16(&mut self, o: usize, value: u16, d: u16) {
        if value != d {
            self.add_u16(value);
            self.slot(o)
        }
    }

    /// Add a value of type `i16` onto the object at vtable slot `o`. If value
    /// `x` equals default `d`, then the slot will be set to zero and no other
    /// data will be written.
    pub fn add_slot_i16(&mut self, o: usize, value: i16, d: i16) {
        self.add_slot_u16(o, value as u16, d as u16);
    }

    /// Add a value of type `u32` onto the object at vtable slot `o`. If value
    /// `x` equals default `d`, then the slot will be set to zero and no other
    /// data will be written.
    pub fn add_slot_u32(&mut self, o: usize, value: u32, d: u32) {
        if value != d {
            self.add_u32(value);
            self.slot(o)
        }
    }

    /// Add a value of type `i32` onto the object at vtable slot `o`. If value
    /// `x` equals default `d`, then the slot will be set to zero and no other
    /// data will be written.
    pub fn add_slot_i32(&mut self, o: usize, value: i32, d: i32) {
        self.add_slot_u32(o, value as u32, d as u32);
    }

    /// Add a value of type `u64` onto the object at vtable slot `o`. If value
    /// `x` equals default `d`, then the slot will be set to zero and no other
    /// data will be written.
    pub fn add_slot_u64(&mut self, o: usize, value: u64, d: u64) {
        if value != d {
            self.add_u64(value);
            self.slot(o)
        }
    }

    /// Add a value of type `i64` onto the object at vtable slot `o`. If value
    /// `x` equals default `d`, then the slot will be set to zero and no other
    /// data will be written.
    pub fn add_slot_i64(&mut self, o: usize, value: i64, d: i64) {
        self.add_slot_u64(o, value as u64, d as u64);
    }

    /// Add a value of type `f32` onto the object at vtable slot `o`. If value
    /// `x` equals default `d`, then the slot will be set to zero and no other
    /// data will be written.
    pub fn add_slot_f32(&mut self, o: usize, value: f32, d: f32) {
        if value != d {
            self.add_f32(value);
            self.slot(o)
        }
    }

    /// Add a value of type `f64` onto the object at vtable slot `o`. If value
    /// `x` equals default `d`, then the slot will be set to zero and no other
    /// data will be written.
    pub fn add_slot_f64(&mut self, o: usize, value: f64, d: f64) {
        if value != d {
            self.add_f64(value);
            self.slot(o)
        }
    }

    /// Add a value of type `UOffsetT` onto the object at vtable slot `o`. If
    /// value `x` equals default `d`, then the slot will be set to zero and no
    /// other data will be written.
    ///
    /// prepends an UOffsetT, relative to where it will be written.
    pub fn add_slot_uoffset(&mut self, o: usize, value: UOffsetT, d: UOffsetT) {
        if value != d {
            self.add_uoffset(value);
            self.slot(o)
        }
    }

    /// PrependStructSlot prepends a struct onto the object at vtable slot `o`.
    /// Structs are stored inline, so nothing additional is being added.
    /// In generated code, `d` is always 0.
    pub fn add_slot_struct(&mut self, o: usize, value: UOffsetT, d: UOffsetT) {
        if value != d {
            self.assert_nested();
		    assert!(value == self.offset() as u32, "Inline data write outside of object");
		    self.slot(o)
        }
    }

    // helper method to read a VOffsetT
    fn get_u16(&self, pos: usize) -> VOffsetT {
        LittleEndian::read_u16(&self.bytes[pos..pos + 2])
    }
}

impl Builder {
    // WriteVtable serializes the vtable for the current object.
    //
    // Before writing out the vtable, this checks pre-existing vtables for
    // equality to this one. If an equal vtable is found, point the object to
    // the existing vtable and return.
    //
    // Because vtable values are sensitive to alignment of object data, not all
    // logically-equal vtables will be deduplicated.
    fn write_vtable(&mut self) -> UOffsetT {
        self.add_uoffset(0);
        let vtableloc = self.offset();
        // Write out the current vtable.
        for i in (0..self.vtable_in_use).rev() {
            // Offset relative to the start of the table.
            let offset = if self.vtable[i] != 0 {
                vtableloc - self.vtable[i] as usize
            } else {
                0
            };
            self.add_u16(offset as u16);
        }
        // write the metadata
        let total_obj_size = vtableloc - self.object_start;
        let vtable_size = (self.vtable_in_use + VTABLE_METADATA_FIEDS) * VOFFSETT_SIZE;
        self.add_u16(total_obj_size as u16);
        self.add_u16(vtable_size as u16);
        // check for identical vtable
        let mut existing_vt = 0;
        let mut this_vt = self.space;
        'outer: for (i, vt_offset) in self.vtables.iter().rev().enumerate() {
            let vt_start = self.bytes.capacity() - *vt_offset as usize;
            let vt_len = self.get_u16(vt_start);
            if vt_len != self.get_u16(this_vt) {
                continue;
            }
            let vt_end = vt_start + vt_len as usize;
            let vt_bytes = &self.bytes[vt_start + VTABLE_METADATA_SIZE..vt_end];
            for (j, chunk) in vt_bytes.chunks(2).enumerate() {

                let this_start = this_vt + (j * VOFFSETT_SIZE) as usize;
                let this_end = this_start + VOFFSETT_SIZE;
                let this_chunk = &self.bytes[this_start..this_end];
                if chunk != this_chunk {
                    continue 'outer;
                }
            }
            existing_vt = self.vtables[i] as usize;
            break 'outer;
        }

        if existing_vt != 0 {
            // found a match
            self.space = self.bytes.capacity() - vtableloc;
            let pos = self.space;
            self.place_u32(pos, (existing_vt - vtableloc) as u32);
        } else {
            // no match
            this_vt = self.offset();
            self.vtables.push(this_vt as u32);
            let capacity = self.bytes.capacity();
            self.place_u32(capacity - vtableloc, (this_vt - vtableloc) as u32);
        }
        vtableloc as UOffsetT
    }

    /// Doubles the size of the buffer.
    ///
    /// copies the old data towards the end of the new buffer (since we build
    /// the buffer backwards).
    pub fn grow(&mut self) {
        let old_capacity = if self.bytes.capacity() == 0 {
            1
        } else {
            self.bytes.capacity()
        };
        let size_test = old_capacity & 0xC0000000;
        assert!(size_test == 0,
                "FlatBuffers: cannot grow buffer beyond 2 gigabytes.");
        let new_capacity = old_capacity << 1;
        let mut nbytes = Vec::with_capacity(new_capacity);
        for _ in 0..old_capacity {
            nbytes.push(0)
        }
        self.space = new_capacity - self.offset();
        nbytes.extend_from_slice(&self.bytes);
        unsafe { nbytes.set_len(new_capacity) };
        self.bytes = nbytes;
    }


    fn place_u8(&mut self, pos: usize, value: u8) {
        self.bytes[pos] = value;
    }

    fn place_u32(&mut self, pos: usize, value: u32) {
        LittleEndian::write_u32(&mut self.bytes[pos..pos + 4], value);
    }

    fn assert_not_nested(&self) {
        assert!(!self.nested,
                "FlatBuffers: object serialization must not be nested.");
    }

    fn assert_nested(&self) {
        assert!(self.nested,
                "FlatBuffers: struct must be serialized inline.");
    }
}

impl Default for Builder {
    fn default() -> Builder {
        Builder::with_capacity(1024)
    }
}

impl Into<Vec<u8>> for Builder {
    fn into(self) -> Vec<u8> {
        self.bytes
    }
}

/// A trait used by generated object builders to facilitate
/// using the same flatbuffer `Builder`.
pub trait ObjectBuilder: Into<Builder> + From<Builder> {
    /// Convert from one `ObjectBuilder` instance to another.
    #[inline(always)]
    fn from_other<T: ObjectBuilder>(x: T) -> Self {
        x.into().into()
    }
}

#[cfg(test)]
mod test {
    use super::Builder;

    #[test]
    fn grow_buffer() {
        let mut b = Builder::with_capacity(0);
        b.grow();
        assert_eq!(b.space, 2);
        assert_eq!(b.offset(), 0);
        assert_eq!(b.len(), 2);

        let mut b = Builder::with_capacity(1);
        b.grow();
        assert_eq!(b.space, 2);
        assert_eq!(b.offset(), 0);
        assert_eq!(b.len(), 2);

        let mut b = Builder::with_capacity(2);
        b.grow();
        assert_eq!(b.space, 4);
        assert_eq!(b.offset(), 0);
        assert_eq!(b.len(), 4);
    }

    #[test]
    fn grow_buffer2() {
        let mut builder = Builder::with_capacity(2);
        builder.add_u8(4);
        builder.add_u8(2);
        let old_capacity = builder.bytes.capacity();
        builder.grow();
        // capacity doubled
        assert_eq!(builder.bytes.capacity(), old_capacity * 2);
        assert_eq!(builder.bytes.len(), old_capacity * 2);
        // bytes moved to end of new buffer
        assert_eq!(builder.bytes[(old_capacity * 2) - 2], 2u8);
        assert_eq!(builder.bytes[(old_capacity * 2) - 1], 4u8);
    }

    #[test]
    fn check_bytes() {
        fn check(b: &Builder, i: u8, want: &[u8]) {
            let got = &b.bytes;
            assert!(got.ends_with(want),
                    "case {}: want\n{:?}\nbut got\n{:?}\n",
                    i,
                    want,
                    got)
        };
        let mut b = Builder::with_capacity(0);
        check(&b, 1, &[]);

        b.add_bool(true);
        check(&b, 2, &[1]);
        b.add_i8(-127);
        check(&b, 3, &[129, 1]);
        b.add_u8(255);
        check(&b, 4, &[255, 129, 1]);
        b.add_i16(-32222);
        check(&b, 5, &[0x22, 0x82, 0, 255, 129, 1]); // first pad
        b.add_u16(0xFEEE);
        check(&b, 6, &[0xEE, 0xFE, 0x22, 0x82, 0, 255, 129, 1]); // no pad this time
        b.add_i32(-53687092);
        check(&b,
              7,
              &[204, 204, 204, 252, 0xEE, 0xFE, 0x22, 0x82, 0, 255, 129, 1]);
        b.add_u32(0x98765432);
        check(&b,
              8,
              &[0x32, 0x54, 0x76, 0x98, 204, 204, 204, 252, 0xEE, 0xFE, 0x22, 0x82, 0, 255, 129,
                1]);

        b = Builder::with_capacity(0);
        b.add_u64(0x1122334455667788);
        check(&b, 9, &[0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11]);

        // test 2: 1xbyte vector
        b = Builder::with_capacity(0);
        check(&b, 10, &[]);
        b.start_vector(1, 1, 1);
        check(&b, 11, &[0, 0, 0]); // align to 4bytes
        b.add_u8(1);
        check(&b, 12, &[1, 0, 0, 0]);
        b.end_vector();
        check(&b, 13, &[1, 0, 0, 0, 1, 0, 0, 0]); // padding

        // test 3: 2xbyte vector
        b = Builder::with_capacity(0);
        b.start_vector(1, 2, 1);
        check(&b, 14, &[0,0]); // align to 4bytes
        b.add_u8(1);
        check(&b, 15, &[1,0,0]);
        b.add_u8(2);
        check(&b, 16, &[2,1,0,0]);
        b.end_vector();
        check(&b, 17, &[2, 0, 0, 0, 2, 1, 0, 0]); // padding

        // test 3b: 11xbyte vector matches builder size
        b = Builder::with_capacity(12);
        b.start_vector(1, 8, 1);
        check(&b, 18, &[]);
        for i in 0..11 {
            b.add_u8(i);
        }
        b.end_vector();
        // says size is 8 not 11 
        check(&b, 19, &[8, 0, 0, 0, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0]);

        // test 4: 1xuint16 vector
        b = Builder::with_capacity(0);
        b.start_vector(2, 1, 1);
        check(&b, 20, &[0,0]); // align to 4bytes
        b.add_u16(1);
        check(&b, 21, &[1,0,0,0]); 
        b.end_vector();
        check(&b, 22, &[1,0,0,0,1,0,0,0]);  // padding

        // test 5: 2xuint16 vector
        b = Builder::with_capacity(0);
        b.start_vector(2, 2, 1);
        check(&b, 23, &[]); // align to 4bytes
        b.add_u16(0xABCD);
        check(&b, 24, &[0xCD, 0xAB]);
        b.add_u16(0xDCBA);
        check(&b, 25, &[0xBA, 0xDC,0xCD, 0xAB]);
        b.end_vector();
        check(&b, 25, &[2, 0, 0, 0, 0xBA, 0xDC, 0xCD, 0xAB]);

        // create string
        b = Builder::with_capacity(0);
        b.create_string("foo");
        check(&b, 26, &[3, 0, 0, 0, 102, 111, 111, 0]); // 0-terminated, no pad
        b.create_string("moop");
        check(&b, 27, &[4, 0, 0, 0, 109, 111, 111, 112, 0, 0, 0, 0, // 0-terminated, 3-byte pad
                        3, 0, 0, 0, 102, 111, 111, 0]);

        // test 7: empty vtable
        b = Builder::with_capacity(0);
        b.start_object(0);
        check(&b, 28, &[]);
        b.end_object();
        check(&b, 29, &[4, 0, 4, 0, 4, 0, 0, 0]);

        // test 8: vtable with one true bool
        b = Builder::with_capacity(0);
        b.start_object(1);
        check(&b, 30, &[]);
        b.add_slot_bool(0, true, false);
        b.end_object();
        check(&b, 31, &[
            6, 0, // vtable bytes
            8, 0, // length of object including vtable offset
            7, 0, // start of bool value
            6, 0, 0, 0, // offset for start of vtable (int32)
            0, 0, 0, // padded to 4 bytes
            1, // bool value
        ]);

        // test 9: vtable with one default bool
        b = Builder::with_capacity(0);
        b.start_object(1);
        check(&b, 32, &[]);
        b.add_slot_bool(0, false, false);
        b.end_object();
        check(&b, 33, &[
            6, 0, // vtable bytes
            4, 0, // end of object from here
            0, 0, // entry 1 is zero
            6, 0, 0, 0, // offset for start of vtable (int32)
        ]);

        // test 10: vtable with one int16
        b = Builder::with_capacity(0);
        b.start_object(1);
        b.add_slot_i16(0, 0x789A, 0);
        b.end_object();
        check(&b, 34, &[
            6, 0, // vtable bytes
            8, 0, // end of object from here
            6, 0, // offset to value
            6, 0, 0, 0, // offset for start of vtable (int32)
            0, 0, // padding to 4 bytes
            0x9A, 0x78,
        ]);

        // test 11: vtable with two int16
        b = Builder::with_capacity(0);
        b.start_object(2);
        b.add_slot_i16(0, 0x3456, 0);
        b.add_slot_i16(1, 0x789A, 0);
        b.end_object();
        check(&b, 35, &[
            8, 0, // vtable bytes
            8, 0, // end of object from here
            6, 0, // offset to value 0
            4, 0, // offset to value 1
            8, 0, 0, 0, // offset for start of vtable (int32)
            0x9A, 0x78, // value 1
            0x56, 0x34, // value 0
        ]);

        // test 12a: vtable with int16 and bool
        b = Builder::with_capacity(0);
        b.start_object(2);
        b.add_slot_i16(0, 0x3456, 0);
        b.add_slot_bool(1, true, false);
        b.end_object();
        check(&b, 36, &[
            8, 0, // vtable bytes
            8, 0, // end of object from here
            6, 0, // offset to value 0
            5, 0, // offset to value 1
            8, 0, 0, 0, // offset for start of vtable (int32)
            0,          // padding
            1,          // value 1
            0x56, 0x34, // value 0
        ]);

        // test 12b: vtable with empty vector
        b = Builder::with_capacity(0);
        b.start_vector(1, 0, 1);
        let vecend = b.end_vector();
        b.start_object(1);
        b.add_slot_uoffset(0, vecend, 0);
        b.end_object();
        check(&b, 37, &[
            6, 0, // vtable bytes
            8, 0,
            4, 0, // offset to vector offset
            6, 0, 0, 0, // offset for start of vtable (int32)
            4, 0, 0, 0,
            0, 0, 0, 0, // length of vector (not in struct)
        ]);

        // test 12c: vtable with empty vector of byte and some scalars
        b = Builder::with_capacity(0);
        b.start_vector(1, 0, 1);
        let vecend = b.end_vector();
        b.start_object(2);
        b.add_slot_i16(0, 55, 0);
        b.add_slot_uoffset(1, vecend, 0);
        b.end_object();
        check(&b, 38, &[
            8, 0, // vtable bytes
            12, 0,
            10, 0, // offset to value 0
            4, 0, // offset to vector offset
            8, 0, 0, 0, // vtable loc
            8, 0, 0, 0, // value 1
            0, 0, 55, 0, // value 0    
            0, 0, 0, 0, // length of vector (not in struct)
        ]);

        // test 13: vtable with 1 int16 and 2-vector of int16
        b = Builder::with_capacity(0);
        b.start_vector(2, 2, 1);
        b.add_i16(0x1234);
        b.add_i16(0x5678);
        let vecend = b.end_vector();
        b.start_object(2);
        b.add_slot_uoffset(1, vecend, 0);
        b.add_slot_i16(0, 55, 0);
        b.end_object();
        check(&b, 39, &[
                8, 0, // vtable bytes
                12, 0, // length of object
                6, 0, // start of value 0 from end of vtable
                8, 0, // start of value 1 from end of buffer
                8, 0, 0, 0, // offset for start of vtable (int32)
                0, 0, // padding
                55, 0, // value 0
                4, 0, 0, 0, // vector position from here
                2, 0, 0, 0, // length of vector (uint32)
                0x78, 0x56, // vector value 1
                0x34, 0x12, // vector value 0
        ]);

        // test 14: vtable with 1 struct of 1 int8, 1 int16, 1 int32
        b = Builder::with_capacity(0);
        b.start_object(1);
        b.prep(4+4+4, 0);
        b.add_i8(55);
        b.pad(3);
        b.add_i16(0x1234);
        b.pad(2);
        b.add_i32(0x12345678);
        let struct_start = b.offset();
        b.add_slot_struct(0, struct_start as u32, 0);
        b.end_object();
        check(&b, 40, &[
                6, 0, // vtable bytes
                16, 0, // end of object from here
                4, 0, // start of struct from here
                6, 0, 0, 0, // offset for start of vtable (int32)
                0x78, 0x56, 0x34, 0x12, // value 2
                0, 0, // padding
                0x34, 0x12, // value 1
                0, 0, 0, // padding
                55, // value 0
        ]);

       // test 15: vtable with 1 vector of 2 struct of 2 int8
        b = Builder::with_capacity(0);
        b.start_vector(1*2, 2, 1);
        b.add_i8(33);
        b.add_i8(44);
        b.add_i8(55);
        b.add_i8(66);
        let vecend = b.end_vector();
        b.start_object(1);
        b.add_slot_uoffset(0, vecend, 0);
        b.end_object();
        check(&b, 41, &[
                6, 0, // vtable bytes
                8, 0,
                4, 0, // offset of vector offset
                6, 0, 0, 0, // offset for start of vtable (int32)
                4, 0, 0, 0, // vector start offset

                2, 0, 0, 0, // vector length
                66, // vector value 1,1
                55, // vector value 1,0
                44, // vector value 0,1
                33, // vector value 0,0
        ]);

        // test 16: table with some elements
        b = Builder::with_capacity(0);
        b.start_object(2);
        b.add_slot_i8(0, 33, 0);
        b.add_slot_i16(1, 66, 0);
        let off = b.end_object();
        b.finish(off);

        check(&b, 42, &[
                12, 0, 0, 0, // root of table: points to vtable offset

                8, 0, // vtable bytes
                8, 0, // end of object from here
                7, 0, // start of value 0
                4, 0, // start of value 1

                8, 0, 0, 0, // offset for start of vtable (int32)

                66, 0, // value 1
                0,  // padding
                33, // value 0
        ]);

          // test 17: one unfinished table and one finished table
        b = Builder::with_capacity(0);
        b.start_object(2);
        b.add_slot_i8(0, 33, 0);
        b.add_slot_i8(1, 44, 0);
        let off = b.end_object();
        b.finish(off);

        b.start_object(3);
        b.add_slot_i8(0, 55, 0);
        b.add_slot_i8(1, 66, 0);
        b.add_slot_i8(2, 77, 0);
        let off = b.end_object();
        b.finish(off);

        check(&b, 43, &[
                16, 0, 0, 0, // root of table: points to object
                0, 0, // padding

                10, 0, // vtable bytes
                8, 0, // size of object
                7, 0, // start of value 0
                6, 0, // start of value 1
                5, 0, // start of value 2
                10, 0, 0, 0, // offset for start of vtable (int32)
                0,  // padding
                77, // value 2
                66, // value 1
                55, // value 0

                12, 0, 0, 0, // root of table: points to object

                8, 0, // vtable bytes
                8, 0, // size of object
                7, 0, // start of value 0
                6, 0, // start of value 1
                8, 0, 0, 0, // offset for start of vtable (int32)
                0, 0, // padding
                44, // value 1
                33, // value 0
        ]);

        // test 18: a bunch of bools
        b = Builder::with_capacity(0);
        b.start_object(8);
        b.add_slot_bool(0, true, false);
        b.add_slot_bool(1, true, false);
        b.add_slot_bool(2, true, false);
        b.add_slot_bool(3, true, false);
        b.add_slot_bool(4, true, false);
        b.add_slot_bool(5, true, false);
        b.add_slot_bool(6, true, false);
        b.add_slot_bool(7, true, false);
        let off = b.end_object();
        b.finish(off);

        check(&b, 44, &[
                24, 0, 0, 0, // root of table: points to vtable offset

                20, 0, // vtable bytes
                12, 0, // size of object
                11, 0, // start of value 0
                10, 0, // start of value 1
                9, 0, // start of value 2
                8, 0, // start of value 3
                7, 0, // start of value 4
                6, 0, // start of value 5
                5, 0, // start of value 6
                4, 0, // start of value 7
                20, 0, 0, 0, // vtable offset

                1, // value 7
                1, // value 6
                1, // value 5
                1, // value 4
                1, // value 3
                1, // value 2
                1, // value 1
                1, // value 0
        ]);


        // test 19: three bools
        b = Builder::with_capacity(0);
        b.start_object(3);
        b.add_slot_bool(0, true, false);
        b.add_slot_bool(1, true, false);
        b.add_slot_bool(2, true, false);
        let off = b.end_object();
        b.finish(off);

        check(&b, 45, &[
                16, 0, 0, 0, // root of table: points to vtable offset

                0, 0, // padding

                10, 0, // vtable bytes
                8, 0, // size of object
                7, 0, // start of value 0
                6, 0, // start of value 1
                5, 0, // start of value 2
                10, 0, 0, 0, // vtable offset from here

                0, // padding
                1, // value 2
                1, // value 1
                1, // value 0
        ]);

        // test 20: some floats
        b = Builder::with_capacity(0);
        b.start_object(1);
        b.add_slot_f32(0, 1.0, 0.0);
        b.end_object();

        check(&b, 46, &[
                6, 0, // vtable bytes
                8, 0, // size of object
                4, 0, // start of value 0
                6, 0, 0, 0, // vtable offset

                0, 0, 128, 63, // value 0
        ]);
    }
}
