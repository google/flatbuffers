// automatically generated by the FlatBuffers compiler, do not modify
// @generated
extern crate alloc;
extern crate flatbuffers;
use alloc::boxed::Box;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::mem;
use core::cmp::Ordering;
extern crate serde;
use self::serde::ser::{Serialize, Serializer, SerializeStruct};
use self::flatbuffers::{EndianScalar, Follow};
use super::*;
#[allow(non_upper_case_globals)]
mod bitflags_color {
  flatbuffers::bitflags::bitflags! {
    /// Composite components of Monster color.
    #[derive(Default)]
    pub struct Color: u8 {
      const Red = 1;
      /// \brief color Green
      /// Green is bit_flag with value (1u << 1)
      const Green = 2;
      /// \brief color Blue (1u << 3)
      const Blue = 8;
    }
  }
}
pub use self::bitflags_color::Color;

impl Serialize for Color {
  fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
  where
    S: Serializer,
  {
    serializer.serialize_u32(self.bits() as u32)
  }
}

impl<'a> flatbuffers::Follow<'a> for Color {
  type Inner = Self;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    let b = flatbuffers::read_scalar_at::<u8>(buf, loc);
    // Safety:
    // This is safe because we know bitflags is implemented with a repr transparent uint of the correct size.
    // from_bits_unchecked will be replaced by an equivalent but safe from_bits_retain in bitflags 2.0
    // https://github.com/bitflags/bitflags/issues/262
    Self::from_bits_unchecked(b)
  }
}

impl flatbuffers::Push for Color {
    type Output = Color;
    #[inline]
    unsafe fn push(&self, dst: &mut [u8], _written_len: usize) {
        flatbuffers::emplace_scalar::<u8>(dst, self.bits());
    }
}

impl flatbuffers::EndianScalar for Color {
  type Scalar = u8;
  #[inline]
  fn to_little_endian(self) -> u8 {
    self.bits().to_le()
  }
  #[inline]
  #[allow(clippy::wrong_self_convention)]
  fn from_little_endian(v: u8) -> Self {
    let b = u8::from_le(v);
    // Safety:
    // This is safe because we know bitflags is implemented with a repr transparent uint of the correct size.
    // from_bits_unchecked will be replaced by an equivalent but safe from_bits_retain in bitflags 2.0
    // https://github.com/bitflags/bitflags/issues/262
    unsafe { Self::from_bits_unchecked(b) }
  }
}

impl flatbuffers::Verifiable for Color {
  #[inline]
  fn run_verifier(
    v: &mut flatbuffers::Verifier, pos: usize
  ) -> Result<(), flatbuffers::InvalidFlatbuffer> {
    use self::flatbuffers::Verifiable;
    u8::run_verifier(v, pos)
  }
}

impl flatbuffers::SimpleToVerifyInSlice for Color {}
