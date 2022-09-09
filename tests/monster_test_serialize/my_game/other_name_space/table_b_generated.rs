// automatically generated by the FlatBuffers compiler, do not modify
// @generated
extern crate alloc;
extern crate flatbuffers;
use alloc::boxed::Box;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::mem;
use core::cmp::Ordering;
extern crate serde;
use self::serde::ser::{Serialize, Serializer, SerializeStruct};
use self::flatbuffers::{EndianScalar, Follow};
use super::*;
pub enum TableBOffset {}
#[derive(Copy, Clone, PartialEq)]

pub struct TableB<'a> {
  pub _tab: flatbuffers::Table<'a>,
}

impl<'a> flatbuffers::Follow<'a> for TableB<'a> {
  type Inner = TableB<'a>;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    Self { _tab: flatbuffers::Table::new(buf, loc) }
  }
}

impl<'a> TableB<'a> {
  pub const VT_A: flatbuffers::VOffsetT = 4;

  pub const fn get_fully_qualified_name() -> &'static str {
    "MyGame.OtherNameSpace.TableB"
  }

  #[inline]
  pub fn init_from_table(table: flatbuffers::Table<'a>) -> Self {
    TableB { _tab: table }
  }
  #[allow(unused_mut)]
  pub fn create<'bldr: 'args, 'args: 'mut_bldr, 'mut_bldr>(
    _fbb: &'mut_bldr mut flatbuffers::FlatBufferBuilder<'bldr>,
    args: &'args TableBArgs<'args>
  ) -> flatbuffers::WIPOffset<TableB<'bldr>> {
    let mut builder = TableBBuilder::new(_fbb);
    if let Some(x) = args.a { builder.add_a(x); }
    builder.finish()
  }

  pub fn unpack(&self) -> TableBT {
    let a = self.a().map(|x| {
      Box::new(x.unpack())
    });
    TableBT {
      a,
    }
  }

  #[inline]
  pub fn a(&self) -> Option<super::super::TableA<'a>> {
    self._tab.get::<flatbuffers::ForwardsUOffset<super::super::TableA>>(TableB::VT_A, None)
  }
}

impl flatbuffers::Verifiable for TableB<'_> {
  #[inline]
  fn run_verifier(
    v: &mut flatbuffers::Verifier, pos: usize
  ) -> Result<(), flatbuffers::InvalidFlatbuffer> {
    use self::flatbuffers::Verifiable;
    v.visit_table(pos)?
     .visit_field::<flatbuffers::ForwardsUOffset<super::super::TableA>>("a", Self::VT_A, false)?
     .finish();
    Ok(())
  }
}
pub struct TableBArgs<'a> {
    pub a: Option<flatbuffers::WIPOffset<super::super::TableA<'a>>>,
}
impl<'a> Default for TableBArgs<'a> {
  #[inline]
  fn default() -> Self {
    TableBArgs {
      a: None,
    }
  }
}

impl Serialize for TableB<'_> {
  fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
  where
    S: Serializer,
  {
    let mut s = serializer.serialize_struct("TableB", 1)?;
      if let Some(f) = self.a() {
        s.serialize_field("a", &f)?;
      } else {
        s.skip_field("a")?;
      }
    s.end()
  }
}

pub struct TableBBuilder<'a: 'b, 'b> {
  fbb_: &'b mut flatbuffers::FlatBufferBuilder<'a>,
  start_: flatbuffers::WIPOffset<flatbuffers::TableUnfinishedWIPOffset>,
}
impl<'a: 'b, 'b> TableBBuilder<'a, 'b> {
  #[inline]
  pub fn add_a(&mut self, a: flatbuffers::WIPOffset<super::super::TableA<'b >>) {
    self.fbb_.push_slot_always::<flatbuffers::WIPOffset<super::super::TableA>>(TableB::VT_A, a);
  }
  #[inline]
  pub fn new(_fbb: &'b mut flatbuffers::FlatBufferBuilder<'a>) -> TableBBuilder<'a, 'b> {
    let start = _fbb.start_table();
    TableBBuilder {
      fbb_: _fbb,
      start_: start,
    }
  }
  #[inline]
  pub fn finish(self) -> flatbuffers::WIPOffset<TableB<'a>> {
    let o = self.fbb_.end_table(self.start_);
    flatbuffers::WIPOffset::new(o.value())
  }
}

impl core::fmt::Debug for TableB<'_> {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    let mut ds = f.debug_struct("TableB");
      ds.field("a", &self.a());
      ds.finish()
  }
}
#[non_exhaustive]
#[derive(Debug, Clone, PartialEq)]
pub struct TableBT {
  pub a: Option<Box<super::super::TableAT>>,
}
impl Default for TableBT {
  fn default() -> Self {
    Self {
      a: None,
    }
  }
}
impl TableBT {
  pub fn pack<'b>(
    &self,
    _fbb: &mut flatbuffers::FlatBufferBuilder<'b>
  ) -> flatbuffers::WIPOffset<TableB<'b>> {
    let a = self.a.as_ref().map(|x|{
      x.pack(_fbb)
    });
    TableB::create(_fbb, &TableBArgs{
      a,
    })
  }
}
