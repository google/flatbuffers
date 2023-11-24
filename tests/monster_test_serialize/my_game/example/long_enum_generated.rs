// automatically generated by the FlatBuffers compiler, do not modify
// @generated
extern crate alloc;
extern crate flatbuffers;
#[allow(unused_imports)]
use alloc::boxed::Box;
#[allow(unused_imports)]
use alloc::string::{String, ToString};
#[allow(unused_imports)]
use alloc::vec::Vec;
#[allow(unused_imports)]
use core::mem;
#[allow(unused_imports)]
use core::cmp::Ordering;
extern crate serde;
#[allow(unused_imports)]
use self::serde::ser::{Serialize, Serializer, SerializeStruct};
#[allow(unused_imports)]
use self::flatbuffers::{EndianScalar, Follow};
#[allow(unused_imports)]
use super::*;
#[allow(non_upper_case_globals)]
mod bitflags_long_enum {
  flatbuffers::bitflags::bitflags! {
    #[derive(Default)]
    pub struct LongEnum: u64 {
      const LongOne = 2;
      const LongTwo = 4;
      const LongBig = 1099511627776;
    }
  }
}
pub use self::bitflags_long_enum::LongEnum;

impl Serialize for LongEnum {
  fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
  where
    S: Serializer,
  {
    serializer.serialize_u32(self.bits() as u32)
  }
}

impl<'a> flatbuffers::Follow<'a> for LongEnum {
  type Inner = Self;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    let b = flatbuffers::read_scalar_at::<u64>(buf, loc);
    // Safety:
    // This is safe because we know bitflags is implemented with a repr transparent uint of the correct size.
    // from_bits_unchecked will be replaced by an equivalent but safe from_bits_retain in bitflags 2.0
    // https://github.com/bitflags/bitflags/issues/262
    Self::from_bits_unchecked(b)
  }
}

impl flatbuffers::Push for LongEnum {
    type Output = LongEnum;
    #[inline]
    unsafe fn push(&self, dst: &mut [u8], _written_len: usize) {
        flatbuffers::emplace_scalar::<u64>(dst, self.bits());
    }
}

impl flatbuffers::EndianScalar for LongEnum {
  type Scalar = u64;
  #[inline]
  fn to_little_endian(self) -> u64 {
    self.bits().to_le()
  }
  #[inline]
  #[allow(clippy::wrong_self_convention)]
  fn from_little_endian(v: u64) -> Self {
    let b = u64::from_le(v);
    // Safety:
    // This is safe because we know bitflags is implemented with a repr transparent uint of the correct size.
    // from_bits_unchecked will be replaced by an equivalent but safe from_bits_retain in bitflags 2.0
    // https://github.com/bitflags/bitflags/issues/262
    unsafe { Self::from_bits_unchecked(b) }
  }
}

impl<'a> flatbuffers::Verifiable for LongEnum {
  #[inline]
  fn run_verifier(
    v: &mut flatbuffers::Verifier, pos: usize
  ) -> Result<(), flatbuffers::InvalidFlatbuffer> {
    use self::flatbuffers::Verifiable;
    u64::run_verifier(v, pos)
  }
}

impl flatbuffers::SimpleToVerifyInSlice for LongEnum {}
