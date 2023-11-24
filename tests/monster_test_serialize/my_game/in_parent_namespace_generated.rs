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
pub enum InParentNamespaceOffset {}
#[derive(Copy, Clone, PartialEq)]

pub struct InParentNamespace<'a> {
  pub _tab: flatbuffers::Table<'a>,
}

impl<'a> flatbuffers::Follow<'a> for InParentNamespace<'a> {
  type Inner = InParentNamespace<'a>;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    Self { _tab: flatbuffers::Table::new(buf, loc) }
  }
}

impl<'a> InParentNamespace<'a> {

  pub const fn get_fully_qualified_name() -> &'static str {
    "MyGame.InParentNamespace"
  }

  #[inline]
  pub unsafe fn init_from_table(table: flatbuffers::Table<'a>) -> Self {
    InParentNamespace { _tab: table }
  }
  #[allow(unused_mut)]
  pub fn create<'bldr: 'args, 'args: 'mut_bldr, 'mut_bldr, A: flatbuffers::Allocator + 'bldr>(
    _fbb: &'mut_bldr mut flatbuffers::FlatBufferBuilder<'bldr, A>,
    _args: &'args InParentNamespaceArgs
  ) -> flatbuffers::WIPOffset<InParentNamespace<'bldr>> {
    let mut builder = InParentNamespaceBuilder::new(_fbb);
    builder.finish()
  }

  pub fn unpack(&self) -> InParentNamespaceT {
    InParentNamespaceT {
    }
  }
}

impl flatbuffers::Verifiable for InParentNamespace<'_> {
  #[inline]
  fn run_verifier(
    v: &mut flatbuffers::Verifier, pos: usize
  ) -> Result<(), flatbuffers::InvalidFlatbuffer> {
    use self::flatbuffers::Verifiable;
    v.visit_table(pos)?
     .finish();
    Ok(())
  }
}
pub struct InParentNamespaceArgs {
}
impl<'a> Default for InParentNamespaceArgs {
  #[inline]
  fn default() -> Self {
    InParentNamespaceArgs {
    }
  }
}

impl Serialize for InParentNamespace<'_> {
  fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
  where
    S: Serializer,
  {
    let s = serializer.serialize_struct("InParentNamespace", 0)?;
    s.end()
  }
}

pub struct InParentNamespaceBuilder<'a: 'b, 'b, A: flatbuffers::Allocator + 'a> {
  fbb_: &'b mut flatbuffers::FlatBufferBuilder<'a, A>,
  start_: flatbuffers::WIPOffset<flatbuffers::TableUnfinishedWIPOffset>,
}
impl<'a: 'b, 'b, A: flatbuffers::Allocator + 'a> InParentNamespaceBuilder<'a, 'b, A> {
  #[inline]
  pub fn new(_fbb: &'b mut flatbuffers::FlatBufferBuilder<'a, A>) -> InParentNamespaceBuilder<'a, 'b, A> {
    let start = _fbb.start_table();
    InParentNamespaceBuilder {
      fbb_: _fbb,
      start_: start,
    }
  }
  #[inline]
  pub fn finish(self) -> flatbuffers::WIPOffset<InParentNamespace<'a>> {
    let o = self.fbb_.end_table(self.start_);
    flatbuffers::WIPOffset::new(o.value())
  }
}

impl core::fmt::Debug for InParentNamespace<'_> {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    let mut ds = f.debug_struct("InParentNamespace");
      ds.finish()
  }
}
#[non_exhaustive]
#[derive(Debug, Clone, PartialEq)]
pub struct InParentNamespaceT {
}
impl Default for InParentNamespaceT {
  fn default() -> Self {
    Self {
    }
  }
}
impl InParentNamespaceT {
  pub fn pack<'b, A: flatbuffers::Allocator + 'b>(
    &self,
    _fbb: &mut flatbuffers::FlatBufferBuilder<'b, A>
  ) -> flatbuffers::WIPOffset<InParentNamespace<'b>> {
    InParentNamespace::create(_fbb, &InParentNamespaceArgs{
    })
  }
}
