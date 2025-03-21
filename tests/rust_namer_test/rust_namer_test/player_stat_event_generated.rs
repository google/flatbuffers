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
pub enum PlayerStatEventOffset {}
#[derive(Copy, Clone, PartialEq)]

pub struct PlayerStatEvent<'a> {
  pub _tab: flatbuffers::Table<'a>,
}

impl<'a> flatbuffers::Follow<'a> for PlayerStatEvent<'a> {
  type Inner = PlayerStatEvent<'a>;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    Self { _tab: unsafe { flatbuffers::Table::new(buf, loc) } }
  }
}

impl<'a> PlayerStatEvent<'a> {

  pub const fn get_fully_qualified_name() -> &'static str {
    "RustNamerTest.PlayerStatEvent"
  }

  #[inline]
  pub unsafe fn init_from_table(table: flatbuffers::Table<'a>) -> Self {
    PlayerStatEvent { _tab: table }
  }
  #[allow(unused_mut)]
  pub fn create<'bldr: 'args, 'args: 'mut_bldr, 'mut_bldr, A: flatbuffers::Allocator + 'bldr>(
    _fbb: &'mut_bldr mut flatbuffers::FlatBufferBuilder<'bldr, A>,
    _args: &'args PlayerStatEventArgs
  ) -> flatbuffers::WIPOffset<PlayerStatEvent<'bldr>> {
    let mut builder = PlayerStatEventBuilder::new(_fbb);
    builder.finish()
  }

  pub fn unpack(&self) -> PlayerStatEventT {
    PlayerStatEventT {
    }
  }
}

impl flatbuffers::Verifiable for PlayerStatEvent<'_> {
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
pub struct PlayerStatEventArgs {
}
impl<'a> Default for PlayerStatEventArgs {
  #[inline]
  fn default() -> Self {
    PlayerStatEventArgs {
    }
  }
}

pub struct PlayerStatEventBuilder<'a: 'b, 'b, A: flatbuffers::Allocator + 'a> {
  fbb_: &'b mut flatbuffers::FlatBufferBuilder<'a, A>,
  start_: flatbuffers::WIPOffset<flatbuffers::TableUnfinishedWIPOffset>,
}
impl<'a: 'b, 'b, A: flatbuffers::Allocator + 'a> PlayerStatEventBuilder<'a, 'b, A> {
  #[inline]
  pub fn new(_fbb: &'b mut flatbuffers::FlatBufferBuilder<'a, A>) -> PlayerStatEventBuilder<'a, 'b, A> {
    let start = _fbb.start_table();
    PlayerStatEventBuilder {
      fbb_: _fbb,
      start_: start,
    }
  }
  #[inline]
  pub fn finish(self) -> flatbuffers::WIPOffset<PlayerStatEvent<'a>> {
    let o = self.fbb_.end_table(self.start_);
    flatbuffers::WIPOffset::new(o.value())
  }
}

impl core::fmt::Debug for PlayerStatEvent<'_> {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    let mut ds = f.debug_struct("PlayerStatEvent");
      ds.finish()
  }
}
#[non_exhaustive]
#[derive(Debug, Clone, PartialEq)]
pub struct PlayerStatEventT {
}
impl Default for PlayerStatEventT {
  fn default() -> Self {
    Self {
    }
  }
}
impl PlayerStatEventT {
  pub fn pack<'b, A: flatbuffers::Allocator + 'b>(
    &self,
    _fbb: &mut flatbuffers::FlatBufferBuilder<'b, A>
  ) -> flatbuffers::WIPOffset<PlayerStatEvent<'b>> {
    PlayerStatEvent::create(_fbb, &PlayerStatEventArgs{
    })
  }
}
