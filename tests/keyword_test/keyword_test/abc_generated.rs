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
#[allow(unused_imports)]
use self::flatbuffers::{EndianScalar, Follow};
#[allow(unused_imports)]
use super::*;
#[deprecated(since = "2.0.0", note = "Use associated constants instead. This will no longer be generated in 2021.")]
pub const ENUM_MIN_ABC: i32 = 0;
#[deprecated(since = "2.0.0", note = "Use associated constants instead. This will no longer be generated in 2021.")]
pub const ENUM_MAX_ABC: i32 = 2;
#[deprecated(since = "2.0.0", note = "Use associated constants instead. This will no longer be generated in 2021.")]
#[allow(non_camel_case_types)]
pub const ENUM_VALUES_ABC: [ABC; 3] = [
  ABC::void,
  ABC::where_,
  ABC::stackalloc,
];

#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Default)]
#[repr(transparent)]
pub struct ABC(pub i32);
#[allow(non_upper_case_globals)]
impl ABC {
  pub const void: Self = Self(0);
  pub const where_: Self = Self(1);
  pub const stackalloc: Self = Self(2);

  pub const ENUM_MIN: i32 = 0;
  pub const ENUM_MAX: i32 = 2;
  pub const ENUM_VALUES: &'static [Self] = &[
    Self::void,
    Self::where_,
    Self::stackalloc,
  ];
  /// Returns the variant's name or "" if unknown.
  pub fn variant_name(self) -> Option<&'static str> {
    match self {
      Self::void => Some("void"),
      Self::where_ => Some("where_"),
      Self::stackalloc => Some("stackalloc"),
      _ => None,
    }
  }
}
impl core::fmt::Debug for ABC {
  fn fmt(&self, f: &mut core::fmt::Formatter) -> core::fmt::Result {
    if let Some(name) = self.variant_name() {
      f.write_str(name)
    } else {
      f.write_fmt(format_args!("<UNKNOWN {:?}>", self.0))
    }
  }
}
impl<'a> flatbuffers::Follow<'a> for ABC {
  type Inner = Self;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    let b = flatbuffers::read_scalar_at::<i32>(buf, loc);
    Self(b)
  }
}

impl flatbuffers::Push for ABC {
    type Output = ABC;
    #[inline]
    unsafe fn push(&self, dst: &mut [u8], _written_len: usize) {
        flatbuffers::emplace_scalar::<i32>(dst, self.0);
    }
}

impl flatbuffers::EndianScalar for ABC {
  type Scalar = i32;
  #[inline]
  fn to_little_endian(self) -> i32 {
    self.0.to_le()
  }
  #[inline]
  #[allow(clippy::wrong_self_convention)]
  fn from_little_endian(v: i32) -> Self {
    let b = i32::from_le(v);
    Self(b)
  }
}

impl<'a> flatbuffers::Verifiable for ABC {
  #[inline]
  fn run_verifier(
    v: &mut flatbuffers::Verifier, pos: usize
  ) -> Result<(), flatbuffers::InvalidFlatbuffer> {
    use self::flatbuffers::Verifiable;
    i32::run_verifier(v, pos)
  }
}

impl flatbuffers::SimpleToVerifyInSlice for ABC {}
