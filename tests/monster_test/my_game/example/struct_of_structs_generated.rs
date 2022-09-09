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
// struct StructOfStructs, aligned to 4
#[repr(transparent)]
#[derive(Clone, Copy, PartialEq)]
pub struct StructOfStructs(pub [u8; 20]);
impl Default for StructOfStructs { 
  fn default() -> Self { 
    Self([0; 20])
  }
}
impl core::fmt::Debug for StructOfStructs {
  fn fmt(&self, f: &mut core::fmt::Formatter) -> core::fmt::Result {
    f.debug_struct("StructOfStructs")
      .field("a", &self.a())
      .field("b", &self.b())
      .field("c", &self.c())
      .finish()
  }
}

impl flatbuffers::SimpleToVerifyInSlice for StructOfStructs {}
impl flatbuffers::SafeSliceAccess for StructOfStructs {}
impl<'a> flatbuffers::Follow<'a> for StructOfStructs {
  type Inner = &'a StructOfStructs;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    <&'a StructOfStructs>::follow(buf, loc)
  }
}
impl<'a> flatbuffers::Follow<'a> for &'a StructOfStructs {
  type Inner = &'a StructOfStructs;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    flatbuffers::follow_cast_ref::<StructOfStructs>(buf, loc)
  }
}
impl<'b> flatbuffers::Push for StructOfStructs {
    type Output = StructOfStructs;
    #[inline]
    unsafe fn push(&self, dst: &mut [u8], _rest: &[u8]) {
        let src = ::core::slice::from_raw_parts(self as *const StructOfStructs as *const u8, Self::size());
        dst.copy_from_slice(src);
    }
}
impl<'b> flatbuffers::Push for &'b StructOfStructs {
    type Output = StructOfStructs;

    #[inline]
    unsafe fn push(&self, dst: &mut [u8], _rest: &[u8]) {
        let src = ::core::slice::from_raw_parts(*self as *const StructOfStructs as *const u8, Self::size());
        dst.copy_from_slice(src);
    }
}

impl<'a> flatbuffers::Verifiable for StructOfStructs {
  #[inline]
  fn run_verifier(
    v: &mut flatbuffers::Verifier, pos: usize
  ) -> Result<(), flatbuffers::InvalidFlatbuffer> {
    use self::flatbuffers::Verifiable;
    v.in_buffer::<Self>(pos)
  }
}

impl<'a> StructOfStructs {
  #[allow(clippy::too_many_arguments)]
  pub fn new(
    a: &Ability,
    b: &Test,
    c: &Ability,
  ) -> Self {
    let mut s = Self([0; 20]);
    s.set_a(a);
    s.set_b(b);
    s.set_c(c);
    s
  }

  pub const fn get_fully_qualified_name() -> &'static str {
    "MyGame.Example.StructOfStructs"
  }

  pub fn a(&self) -> &Ability {
    unsafe { &*(self.0[0..].as_ptr() as *const Ability) }
  }

  #[allow(clippy::identity_op)]
  pub fn set_a(&mut self, x: &Ability) {
    self.0[0..0 + 8].copy_from_slice(&x.0)
  }

  pub fn b(&self) -> &Test {
    unsafe { &*(self.0[8..].as_ptr() as *const Test) }
  }

  #[allow(clippy::identity_op)]
  pub fn set_b(&mut self, x: &Test) {
    self.0[8..8 + 4].copy_from_slice(&x.0)
  }

  pub fn c(&self) -> &Ability {
    unsafe { &*(self.0[12..].as_ptr() as *const Ability) }
  }

  #[allow(clippy::identity_op)]
  pub fn set_c(&mut self, x: &Ability) {
    self.0[12..12 + 8].copy_from_slice(&x.0)
  }

  pub fn unpack(&self) -> StructOfStructsT {
    StructOfStructsT {
      a: self.a().unpack(),
      b: self.b().unpack(),
      c: self.c().unpack(),
    }
  }
}

#[derive(Debug, Clone, PartialEq, Default)]
pub struct StructOfStructsT {
  pub a: AbilityT,
  pub b: TestT,
  pub c: AbilityT,
}
impl StructOfStructsT {
  pub fn pack(&self) -> StructOfStructs {
    StructOfStructs::new(
      &self.a.pack(),
      &self.b.pack(),
      &self.c.pack(),
    )
  }
}

