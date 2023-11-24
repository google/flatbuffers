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
pub enum FieldTableOffset {}
#[derive(Copy, Clone, PartialEq)]

pub struct FieldTable<'a> {
  pub _tab: flatbuffers::Table<'a>,
}

impl<'a> flatbuffers::Follow<'a> for FieldTable<'a> {
  type Inner = FieldTable<'a>;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    Self { _tab: flatbuffers::Table::new(buf, loc) }
  }
}

impl<'a> FieldTable<'a> {

  pub const fn get_fully_qualified_name() -> &'static str {
    "RustNamerTest.FieldTable"
  }

  #[inline]
  pub unsafe fn init_from_table(table: flatbuffers::Table<'a>) -> Self {
    FieldTable { _tab: table }
  }
  #[allow(unused_mut)]
  pub fn create<'bldr: 'args, 'args: 'mut_bldr, 'mut_bldr, A: flatbuffers::Allocator + 'bldr>(
    _fbb: &'mut_bldr mut flatbuffers::FlatBufferBuilder<'bldr, A>,
    _args: &'args FieldTableArgs
  ) -> flatbuffers::WIPOffset<FieldTable<'bldr>> {
    let mut builder = FieldTableBuilder::new(_fbb);
    builder.finish()
  }

  pub fn unpack(&self) -> FieldTableT {
    FieldTableT {
    }
  }
}

impl flatbuffers::Verifiable for FieldTable<'_> {
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
pub struct FieldTableArgs {
}
impl Default for FieldTableArgs {
  #[inline]
  fn default() -> Self {
    FieldTableArgs {
    }
  }
}

pub struct FieldTableBuilder<'a: 'b, 'b, A: flatbuffers::Allocator + 'a> {
  fbb_: &'b mut flatbuffers::FlatBufferBuilder<'a, A>,
  start_: flatbuffers::WIPOffset<flatbuffers::TableUnfinishedWIPOffset>,
}
impl<'a: 'b, 'b, A: flatbuffers::Allocator + 'a> FieldTableBuilder<'a, 'b, A> {
  #[inline]
  pub fn new(_fbb: &'b mut flatbuffers::FlatBufferBuilder<'a, A>) -> FieldTableBuilder<'a, 'b, A> {
    let start = _fbb.start_table();
    FieldTableBuilder {
      fbb_: _fbb,
      start_: start,
    }
  }
  #[inline]
  pub fn finish(self) -> flatbuffers::WIPOffset<FieldTable<'a>> {
    let o = self.fbb_.end_table(self.start_);
    flatbuffers::WIPOffset::new(o.value())
  }
}

impl core::fmt::Debug for FieldTable<'_> {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    let mut ds = f.debug_struct("FieldTable");
      ds.finish()
  }
}
#[non_exhaustive]
#[derive(Debug, Clone, PartialEq)]
pub struct FieldTableT {
}
impl Default for FieldTableT {
  fn default() -> Self {
    Self {
    }
  }
}
impl FieldTableT {
  pub fn pack<'b, A: flatbuffers::Allocator + 'b>(
    &self,
    _fbb: &mut flatbuffers::FlatBufferBuilder<'b, A>
  ) -> flatbuffers::WIPOffset<FieldTable<'b>> {
    FieldTable::create(_fbb, &FieldTableArgs{
    })
  }
}
