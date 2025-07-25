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
pub enum SecondTableInAOffset {}
#[derive(Copy, Clone, PartialEq)]

pub struct SecondTableInA<'a> {
  pub _tab: flatbuffers::Table<'a>,
}

impl<'a> flatbuffers::Follow<'a> for SecondTableInA<'a> {
  type Inner = SecondTableInA<'a>;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    Self { _tab: unsafe { flatbuffers::Table::new(buf, loc) } }
  }
}

impl<'a> SecondTableInA<'a> {
  pub const VT_REFER_TO_C: flatbuffers::VOffsetT = 4;

  pub const fn get_fully_qualified_name() -> &'static str {
    "NamespaceA.SecondTableInA"
  }

  #[inline]
  pub unsafe fn init_from_table(table: flatbuffers::Table<'a>) -> Self {
    SecondTableInA { _tab: table }
  }
  #[allow(unused_mut)]
  pub fn create<'bldr: 'args, 'args: 'mut_bldr, 'mut_bldr, A: flatbuffers::Allocator + 'bldr>(
    _fbb: &'mut_bldr mut flatbuffers::FlatBufferBuilder<'bldr, A>,
    args: &'args SecondTableInAArgs<'args>
  ) -> flatbuffers::WIPOffset<SecondTableInA<'bldr>> {
    let mut builder = SecondTableInABuilder::new(_fbb);
    if let Some(x) = args.refer_to_c { builder.add_refer_to_c(x); }
    builder.finish()
  }

  pub fn unpack(&self) -> SecondTableInAT {
    let refer_to_c = self.refer_to_c().map(|x| {
      Box::new(x.unpack())
    });
    SecondTableInAT {
      refer_to_c,
    }
  }

  #[inline]
  pub fn refer_to_c(&self) -> Option<super::namespace_c::TableInC<'a>> {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<flatbuffers::ForwardsUOffset<super::namespace_c::TableInC>>(SecondTableInA::VT_REFER_TO_C, None)}
  }
}

impl flatbuffers::Verifiable for SecondTableInA<'_> {
  #[inline]
  fn run_verifier(
    v: &mut flatbuffers::Verifier, pos: usize
  ) -> Result<(), flatbuffers::InvalidFlatbuffer> {
    use self::flatbuffers::Verifiable;
    v.visit_table(pos)?
     .visit_field::<flatbuffers::ForwardsUOffset<super::namespace_c::TableInC>>("refer_to_c", Self::VT_REFER_TO_C, false)?
     .finish();
    Ok(())
  }
}
pub struct SecondTableInAArgs<'a> {
    pub refer_to_c: Option<flatbuffers::WIPOffset<super::namespace_c::TableInC<'a>>>,
}
impl<'a> Default for SecondTableInAArgs<'a> {
  #[inline]
  fn default() -> Self {
    SecondTableInAArgs {
      refer_to_c: None,
    }
  }
}

pub struct SecondTableInABuilder<'a: 'b, 'b, A: flatbuffers::Allocator + 'a> {
  fbb_: &'b mut flatbuffers::FlatBufferBuilder<'a, A>,
  start_: flatbuffers::WIPOffset<flatbuffers::TableUnfinishedWIPOffset>,
}
impl<'a: 'b, 'b, A: flatbuffers::Allocator + 'a> SecondTableInABuilder<'a, 'b, A> {
  #[inline]
  pub fn add_refer_to_c(&mut self, refer_to_c: flatbuffers::WIPOffset<super::namespace_c::TableInC<'b >>) {
    self.fbb_.push_slot_always::<flatbuffers::WIPOffset<super::namespace_c::TableInC>>(SecondTableInA::VT_REFER_TO_C, refer_to_c);
  }
  #[inline]
  pub fn new(_fbb: &'b mut flatbuffers::FlatBufferBuilder<'a, A>) -> SecondTableInABuilder<'a, 'b, A> {
    let start = _fbb.start_table();
    SecondTableInABuilder {
      fbb_: _fbb,
      start_: start,
    }
  }
  #[inline]
  pub fn finish(self) -> flatbuffers::WIPOffset<SecondTableInA<'a>> {
    let o = self.fbb_.end_table(self.start_);
    flatbuffers::WIPOffset::new(o.value())
  }
}

impl core::fmt::Debug for SecondTableInA<'_> {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    let mut ds = f.debug_struct("SecondTableInA");
      ds.field("refer_to_c", &self.refer_to_c());
      ds.finish()
  }
}
#[non_exhaustive]
#[derive(Debug, Clone, PartialEq)]
pub struct SecondTableInAT {
  pub refer_to_c: Option<Box<super::namespace_c::TableInCT>>,
}
impl Default for SecondTableInAT {
  fn default() -> Self {
    Self {
      refer_to_c: None,
    }
  }
}
impl SecondTableInAT {
  pub fn pack<'b, A: flatbuffers::Allocator + 'b>(
    &self,
    _fbb: &mut flatbuffers::FlatBufferBuilder<'b, A>
  ) -> flatbuffers::WIPOffset<SecondTableInA<'b>> {
    let refer_to_c = self.refer_to_c.as_ref().map(|x|{
      x.pack(_fbb)
    });
    SecondTableInA::create(_fbb, &SecondTableInAArgs{
      refer_to_c,
    })
  }
}
