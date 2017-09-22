
/// A SOffsetT stores a signed offset into arbitrary data.
pub type SOffsetT = i32;

/// A UOffsetT stores an unsigned offset into vector data.
pub type UOffsetT = u32;

/// A VOffsetT stores an unsigned offset in a vtable.
pub type VOffsetT = u16;

/// Byte size of a `VOffsetT`.
pub const VOFFSETT_SIZE: usize = 2;
/// Byte size of a `UOffsetT`.
pub const UOFFSETT_SIZE: usize = 4;

/// Number of metadata fields in a `VTable`.
/// Vtable Len and Object Size.
pub const VTABLE_METADATA_FIEDS: usize = 2;
/// Byte size of `Vtable` metadata.
pub const VTABLE_METADATA_SIZE: usize = VTABLE_METADATA_FIEDS * VOFFSETT_SIZE;
