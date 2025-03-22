// automatically generated by the FlatBuffers compiler, do not modify
// @generated
extern crate alloc;
use super::*;
// struct StructOfStructsOfStructs, aligned to 4
#[repr(transparent)]
#[derive(Clone, Copy, PartialEq)]
pub struct StructOfStructsOfStructs(pub [u8; 20]);
impl Default for StructOfStructsOfStructs { 
  fn default() -> Self { 
    Self([0; 20])
  }
}
impl ::core::fmt::Debug for StructOfStructsOfStructs {
  fn fmt(&self, f: &mut ::core::fmt::Formatter) -> ::core::fmt::Result {
    f.debug_struct("StructOfStructsOfStructs")
      .field("a", &self.a())
      .finish()
  }
}

impl ::flatbuffers::SimpleToVerifyInSlice for StructOfStructsOfStructs {}
impl<'a> ::flatbuffers::Follow<'a> for StructOfStructsOfStructs {
  type Inner = &'a StructOfStructsOfStructs;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    <&'a StructOfStructsOfStructs>::follow(buf, loc)
  }
}
impl<'a> ::flatbuffers::Follow<'a> for &'a StructOfStructsOfStructs {
  type Inner = &'a StructOfStructsOfStructs;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    ::flatbuffers::follow_cast_ref::<StructOfStructsOfStructs>(buf, loc)
  }
}
impl<'b> ::flatbuffers::Push for StructOfStructsOfStructs {
    type Output = StructOfStructsOfStructs;
    #[inline]
    unsafe fn push(&self, dst: &mut [u8], _written_len: usize) {
        let src = ::core::slice::from_raw_parts(self as *const StructOfStructsOfStructs as *const u8, <Self as ::flatbuffers::Push>::size());
        dst.copy_from_slice(src);
    }
    #[inline]
    fn alignment() -> ::flatbuffers::PushAlignment {
        ::flatbuffers::PushAlignment::new(4)
    }
}

impl<'a> ::flatbuffers::Verifiable for StructOfStructsOfStructs {
  #[inline]
  fn run_verifier(
    v: &mut ::flatbuffers::Verifier, pos: usize
  ) -> Result<(), ::flatbuffers::InvalidFlatbuffer> {
    use ::flatbuffers::Verifiable;
    v.in_buffer::<Self>(pos)
  }
}

impl<'a> StructOfStructsOfStructs {
  #[allow(clippy::too_many_arguments)]
  pub fn new(
    a: &StructOfStructs,
  ) -> Self {
    let mut s = Self([0; 20]);
    s.set_a(a);
    s
  }

  pub const fn get_fully_qualified_name() -> &'static str {
    "MyGame.Example.StructOfStructsOfStructs"
  }

  pub fn a(&self) -> &StructOfStructs {
    // Safety:
    // Created from a valid Table for this object
    // Which contains a valid struct in this slot
    unsafe { &*(self.0[0..].as_ptr() as *const StructOfStructs) }
  }

  #[allow(clippy::identity_op)]
  pub fn set_a(&mut self, x: &StructOfStructs) {
    self.0[0..0 + 20].copy_from_slice(&x.0)
  }

  pub fn unpack(&self) -> StructOfStructsOfStructsT {
    StructOfStructsOfStructsT {
      a: self.a().unpack(),
    }
  }
}

#[derive(Debug, Clone, PartialEq, Default)]
pub struct StructOfStructsOfStructsT {
  pub a: StructOfStructsT,
}
impl StructOfStructsOfStructsT {
  pub fn pack(&self) -> StructOfStructsOfStructs {
    StructOfStructsOfStructs::new(
      &self.a.pack(),
    )
  }
}

