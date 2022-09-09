// automatically generated by the FlatBuffers compiler, do not modify
// @generated
extern crate alloc;
extern crate flatbuffers;
use alloc::boxed::Box;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::mem;
use core::cmp::Ordering;
use self::flatbuffers::{EndianScalar, Follow};
use super::*;
#[deprecated(since = "2.0.0", note = "Use associated constants instead. This will no longer be generated in 2021.")]
pub const ENUM_MIN_EQUIPMENT: u8 = 0;
#[deprecated(since = "2.0.0", note = "Use associated constants instead. This will no longer be generated in 2021.")]
pub const ENUM_MAX_EQUIPMENT: u8 = 1;
#[deprecated(since = "2.0.0", note = "Use associated constants instead. This will no longer be generated in 2021.")]
#[allow(non_camel_case_types)]
pub const ENUM_VALUES_EQUIPMENT: [Equipment; 2] = [
  Equipment::NONE,
  Equipment::Weapon,
];

#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Default)]
#[repr(transparent)]
pub struct Equipment(pub u8);
#[allow(non_upper_case_globals)]
impl Equipment {
  pub const NONE: Self = Self(0);
  pub const Weapon: Self = Self(1);

  pub const ENUM_MIN: u8 = 0;
  pub const ENUM_MAX: u8 = 1;
  pub const ENUM_VALUES: &'static [Self] = &[
    Self::NONE,
    Self::Weapon,
  ];
  /// Returns the variant's name or "" if unknown.
  pub fn variant_name(self) -> Option<&'static str> {
    match self {
      Self::NONE => Some("NONE"),
      Self::Weapon => Some("Weapon"),
      _ => None,
    }
  }
}
impl core::fmt::Debug for Equipment {
  fn fmt(&self, f: &mut core::fmt::Formatter) -> core::fmt::Result {
    if let Some(name) = self.variant_name() {
      f.write_str(name)
    } else {
      f.write_fmt(format_args!("<UNKNOWN {:?}>", self.0))
    }
  }
}
impl<'a> flatbuffers::Follow<'a> for Equipment {
  type Inner = Self;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    let b = flatbuffers::read_scalar_at::<u8>(buf, loc);
    Self(b)
  }
}

impl flatbuffers::Push for Equipment {
    type Output = Equipment;
    #[inline]
    unsafe fn push(&self, dst: &mut [u8], _rest: &[u8]) {
        flatbuffers::emplace_scalar::<u8>(dst, self.0);
    }
}

impl flatbuffers::EndianScalar for Equipment {
  #[inline]
  fn to_little_endian(self) -> Self {
    let b = u8::to_le(self.0);
    Self(b)
  }
  #[inline]
  #[allow(clippy::wrong_self_convention)]
  fn from_little_endian(self) -> Self {
    let b = u8::from_le(self.0);
    Self(b)
  }
}

impl<'a> flatbuffers::Verifiable for Equipment {
  #[inline]
  fn run_verifier(
    v: &mut flatbuffers::Verifier, pos: usize
  ) -> Result<(), flatbuffers::InvalidFlatbuffer> {
    use self::flatbuffers::Verifiable;
    u8::run_verifier(v, pos)
  }
}

impl flatbuffers::SimpleToVerifyInSlice for Equipment {}
pub struct EquipmentUnionTableOffset {}

#[allow(clippy::upper_case_acronyms)]
#[non_exhaustive]
#[derive(Debug, Clone, PartialEq)]
pub enum EquipmentT {
  NONE,
  Weapon(Box<WeaponT>),
}
impl Default for EquipmentT {
  fn default() -> Self {
    Self::NONE
  }
}
impl EquipmentT {
  pub fn equipment_type(&self) -> Equipment {
    match self {
      Self::NONE => Equipment::NONE,
      Self::Weapon(_) => Equipment::Weapon,
    }
  }
  pub fn pack(&self, fbb: &mut flatbuffers::FlatBufferBuilder) -> Option<flatbuffers::WIPOffset<flatbuffers::UnionWIPOffset>> {
    match self {
      Self::NONE => None,
      Self::Weapon(v) => Some(v.pack(fbb).as_union_value()),
    }
  }
  /// If the union variant matches, return the owned WeaponT, setting the union to NONE.
  pub fn take_weapon(&mut self) -> Option<Box<WeaponT>> {
    if let Self::Weapon(_) = self {
      let v = core::mem::replace(self, Self::NONE);
      if let Self::Weapon(w) = v {
        Some(w)
      } else {
        unreachable!()
      }
    } else {
      None
    }
  }
  /// If the union variant matches, return a reference to the WeaponT.
  pub fn as_weapon(&self) -> Option<&WeaponT> {
    if let Self::Weapon(v) = self { Some(v.as_ref()) } else { None }
  }
  /// If the union variant matches, return a mutable reference to the WeaponT.
  pub fn as_weapon_mut(&mut self) -> Option<&mut WeaponT> {
    if let Self::Weapon(v) = self { Some(v.as_mut()) } else { None }
  }
}
