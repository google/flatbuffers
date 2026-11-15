//! Chunked `FlatBuffer` builder for memory-constrained environments.
//!
//! Uses segmented allocation during the build phase, then coalesces
//! into a contiguous buffer at finish time. Reduces heap fragmentation
//! on ARM embedded targets.
//!
//! # Proof of concept
//!
//! This is a **proof-of-concept allocator**, not a drop-in replacement for
//! [`flatbuffers::FlatBufferBuilder`]. It demonstrates the segmented allocation
//! pattern that reduces heap fragmentation on ARM targets with limited RAM.
//!
//! ## Why not just use `FlatBufferBuilder`?
//!
//! `FlatBufferBuilder` uses a single `Vec<u8>` that doubles in size when full.
//! On memory-constrained ARM targets (64–256 KB RAM), this doubling can:
//!
//! - Request an allocation larger than any available contiguous block
//! - Fragment the heap by releasing the old buffer after copying to a new one
//!
//! `ChunkedBuilder` caps individual allocations at `segment_size`, reducing
//! peak memory and fragmentation at the cost of a single coalesce copy at
//! finish time.
//!
//! ## Production integration options
//!
//! In production, VTS could integrate this pattern by either:
//!
//! 1. Forking `FlatBufferBuilder` to accept a custom allocator trait, or
//! 2. Using `ChunkedBuilder` as a pre-allocation pool that hands one large
//!    contiguous block to `FlatBufferBuilder` (the pool absorbs fragmentation).

/// Default segment size: 32 KB.
pub const DEFAULT_SEGMENT_SIZE: usize = 32 * 1024;

/// A byte-buffer allocator that allocates in fixed-size segments.
///
/// During the build phase, memory is allocated in chunks of `segment_size`.
/// When [`ChunkedBuilder::coalesce`] is called, all segments are concatenated
/// into a single contiguous `Vec<u8>` containing the valid `FlatBuffer` bytes.
///
/// # Usage
///
/// ```rust
/// use flatbuffers_reflection::chunked_builder::ChunkedBuilder;
///
/// let mut builder = ChunkedBuilder::new(4096);
///
/// // Simulate writing several fields worth of data
/// let chunk = builder.allocate(128);
/// chunk.fill(0xAB);
///
/// let flat = builder.coalesce();
/// assert_eq!(flat.len(), 128);
/// assert!(flat.iter().all(|&b| b == 0xAB));
/// ```
pub struct ChunkedBuilder {
    /// Fully-filled segments in order of allocation.
    segments: Vec<Vec<u8>>,
    /// Maximum number of bytes per segment.
    segment_size: usize,
    /// The segment currently being written into.
    current_segment: Vec<u8>,
    /// Cursor within `current_segment`: bytes already committed.
    current_written: usize,
    /// Sum of bytes committed across all completed segments plus `current_written`.
    total_written: usize,
}

impl ChunkedBuilder {
    /// Create a new `ChunkedBuilder` with the given segment size.
    ///
    /// `segment_size` is the maximum number of bytes that will be allocated in
    /// a single heap block during the build phase.  Allocations larger than
    /// `segment_size` are handled by requesting an oversized segment for that
    /// allocation alone (see [`allocate`](ChunkedBuilder::allocate)).
    ///
    /// # Panics
    ///
    /// Panics if `segment_size` is zero.
    #[must_use]
    pub fn new(segment_size: usize) -> Self {
        assert!(segment_size > 0, "segment_size must be greater than zero");
        Self {
            segments: Vec::new(),
            segment_size,
            current_segment: Vec::with_capacity(segment_size),
            current_written: 0,
            total_written: 0,
        }
    }

    /// Create a `ChunkedBuilder` using [`DEFAULT_SEGMENT_SIZE`] (32 KB).
    #[must_use]
    pub fn with_default_segment_size() -> Self {
        Self::new(DEFAULT_SEGMENT_SIZE)
    }

    /// Allocate `len` bytes and return a mutable slice to write into.
    ///
    /// If `len` fits in the remaining capacity of the current segment, the
    /// bytes are taken from there.  Otherwise the current segment is sealed and
    /// a new one is opened.  If `len` exceeds `segment_size`, a single
    /// oversized segment is allocated to satisfy the request.
    ///
    /// # Panics
    ///
    /// Panics if `len` is zero.
    pub fn allocate(&mut self, len: usize) -> &mut [u8] {
        assert!(len > 0, "allocation length must be greater than zero");

        let remaining = self.segment_size.saturating_sub(self.current_written);

        if len > remaining {
            // Seal the current segment (trim to what was actually written).
            let written = self.current_written;
            self.current_segment.truncate(written);
            let sealed = std::mem::replace(
                &mut self.current_segment,
                Vec::with_capacity(len.max(self.segment_size)),
            );
            if !sealed.is_empty() {
                self.segments.push(sealed);
            }
            self.current_written = 0;
        }

        // Extend the current segment to hold the new bytes.
        let start = self.current_written;
        let new_len = start + len;
        if self.current_segment.len() < new_len {
            self.current_segment.resize(new_len, 0);
        }
        self.current_written += len;
        self.total_written += len;

        &mut self.current_segment[start..start + len]
    }

    /// Returns the total number of bytes written across all segments.
    #[must_use]
    pub const fn total_size(&self) -> usize {
        self.total_written
    }

    /// Returns the number of segments currently allocated (including the active one).
    #[must_use]
    pub const fn segment_count(&self) -> usize {
        self.segments.len() + 1
    }

    /// Coalesce all segments into a single contiguous `Vec<u8>`.
    ///
    /// This is the final step: segments are concatenated in allocation order to
    /// produce a byte-slice that is structurally equivalent to what a
    /// `FlatBufferBuilder`-backed buffer would contain.
    #[must_use]
    pub fn coalesce(self) -> Vec<u8> {
        let mut result = Vec::with_capacity(self.total_written);
        for seg in &self.segments {
            result.extend_from_slice(seg);
        }
        // Only extend the bytes that were actually written in the active segment.
        result.extend_from_slice(&self.current_segment[..self.current_written]);
        result
    }
}

impl Default for ChunkedBuilder {
    fn default() -> Self {
        Self::with_default_segment_size()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_basic_allocation() {
        let mut builder = ChunkedBuilder::new(1024);
        let buf = builder.allocate(64);
        assert_eq!(buf.len(), 64);
        assert_eq!(builder.total_size(), 64);
    }

    #[test]
    fn test_coalesce_produces_contiguous_output() {
        let mut builder = ChunkedBuilder::new(1024);

        let a = builder.allocate(4);
        a.copy_from_slice(&[1, 2, 3, 4]);
        let b = builder.allocate(4);
        b.copy_from_slice(&[5, 6, 7, 8]);

        let out = builder.coalesce();
        assert_eq!(out, vec![1, 2, 3, 4, 5, 6, 7, 8]);
    }

    #[test]
    fn test_segment_boundary_crossing() {
        // Use a small segment so we can easily force a boundary crossing.
        let segment_size = 8;
        let mut builder = ChunkedBuilder::new(segment_size);

        // Fill first segment completely.
        let a = builder.allocate(8);
        a.fill(0xAA);

        // This allocation crosses into a new segment.
        let b = builder.allocate(4);
        b.fill(0xBB);

        assert_eq!(builder.segment_count(), 2);
        assert_eq!(builder.total_size(), 12);

        let out = builder.coalesce();
        assert_eq!(out.len(), 12);
        assert!(out[..8].iter().all(|&b| b == 0xAA));
        assert!(out[8..].iter().all(|&b| b == 0xBB));
    }

    #[test]
    fn test_large_allocation_exceeding_segment() {
        // Request an allocation larger than the configured segment size.
        let segment_size = 16;
        let mut builder = ChunkedBuilder::new(segment_size);

        let big = builder.allocate(256);
        big.fill(0xFF);

        assert_eq!(builder.total_size(), 256);

        let out = builder.coalesce();
        assert_eq!(out.len(), 256);
        assert!(out.iter().all(|&b| b == 0xFF));
    }

    #[test]
    fn test_multiple_segments_coalesce_in_order() {
        let segment_size = 4;
        let mut builder = ChunkedBuilder::new(segment_size);

        for i in 0_u8..8 {
            let s = builder.allocate(4);
            s.fill(i);
        }

        let out = builder.coalesce();
        assert_eq!(out.len(), 32);
        for i in 0_u8..8 {
            let start = usize::from(i) * 4;
            assert!(
                out[start..start + 4].iter().all(|&b| b == i),
                "segment {i} contents wrong"
            );
        }
    }

    #[test]
    fn test_default_segment_size() {
        let builder = ChunkedBuilder::default();
        assert_eq!(builder.segment_size, DEFAULT_SEGMENT_SIZE);
        assert_eq!(builder.total_size(), 0);
        assert_eq!(builder.segment_count(), 1);
    }

    #[test]
    fn test_empty_coalesce() {
        let builder = ChunkedBuilder::new(1024);
        let out = builder.coalesce();
        assert!(out.is_empty());
    }

    #[test]
    fn test_total_size_tracks_across_segments() {
        let mut builder = ChunkedBuilder::new(10);
        builder.allocate(7);
        builder.allocate(7); // forces a new segment
        builder.allocate(7); // forces another
        assert_eq!(builder.total_size(), 21);
    }
}
