// automatically generated by the FlatBuffers compiler, do not modify
extern crate flatbuffers;
extern crate serde;
use std::mem;
use std::cmp::Ordering;
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
  fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    let b = unsafe {
      flatbuffers::read_scalar_at::<u8>(buf, loc)
    };
    unsafe { Self::from_bits_unchecked(b) }
  }
}

impl flatbuffers::Push for Color {
    type Output = Color;
    #[inline]
    fn push(&self, dst: &mut [u8], _rest: &[u8]) {
        unsafe { flatbuffers::emplace_scalar::<u8>(dst, self.bits()); }
    }
}

impl flatbuffers::EndianScalar for Color {
  #[inline]
  fn to_little_endian(self) -> Self {
    let b = u8::to_le(self.bits());
    unsafe { Self::from_bits_unchecked(b) }
  }
  #[inline]
  #[allow(clippy::wrong_self_convention)]
  fn from_little_endian(self) -> Self {
    let b = u8::from_le(self.bits());
    unsafe { Self::from_bits_unchecked(b) }
  }
}

impl<'a> flatbuffers::Verifiable for Color {
  #[inline]
  fn run_verifier(
    v: &mut flatbuffers::Verifier, pos: usize
  ) -> Result<(), flatbuffers::InvalidFlatbuffer> {
    use self::flatbuffers::Verifiable;
    u8::run_verifier(v, pos)
  }
}

impl flatbuffers::SimpleToVerifyInSlice for Color {}
