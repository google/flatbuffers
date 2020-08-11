// automatically generated by the FlatBuffers compiler, do not modify



use crate::include_test2_generated::*;
use std::mem;
use std::cmp::Ordering;

extern crate flatbuffers;
use self::flatbuffers::EndianScalar;

pub enum TableAOffset {}
#[derive(Copy, Clone, Debug, PartialEq)]

pub struct TableA<'a> {
  pub _tab: flatbuffers::Table<'a>,
}

impl<'a> flatbuffers::Follow<'a> for TableA<'a> {
    type Inner = TableA<'a>;
    #[inline]
    fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
        Self { _tab: flatbuffers::Table { buf, loc } }
    }
}

impl<'a> TableA<'a> {
    #[inline]
    pub fn init_from_table(table: flatbuffers::Table<'a>) -> Self {
        TableA { _tab: table }
    }
    #[allow(unused_mut)]
    pub fn create<'bldr: 'args, 'args: 'mut_bldr, 'mut_bldr>(
        _fbb: &'mut_bldr mut flatbuffers::FlatBufferBuilder<'bldr>,
        args: &'args TableAArgs<'args>) -> flatbuffers::WIPOffset<TableA<'bldr>> {
      let mut builder = TableABuilder::new(_fbb);
      if let Some(x) = args.b { builder.add_b(x); }
      builder.finish()
    }

    pub const VT_B: flatbuffers::VOffsetT = 4;

  #[inline]
  pub fn b(&self) -> Option<my_game::other_name_space::TableB<'a>> {
    self._tab.get::<flatbuffers::ForwardsUOffset<my_game::other_name_space::TableB<'a>>>(TableA::VT_B, None)
  }
}

pub struct TableAArgs<'a> {
    pub b: Option<flatbuffers::WIPOffset<my_game::other_name_space::TableB<'a>>>,
}
impl<'a> Default for TableAArgs<'a> {
    #[inline]
    fn default() -> Self {
        TableAArgs {
            b: None,
        }
    }
}
pub struct TableABuilder<'a: 'b, 'b> {
  fbb_: &'b mut flatbuffers::FlatBufferBuilder<'a>,
  start_: flatbuffers::WIPOffset<flatbuffers::TableUnfinishedWIPOffset>,
}
impl<'a: 'b, 'b> TableABuilder<'a, 'b> {
  #[inline]
  pub fn add_b(&mut self, b: flatbuffers::WIPOffset<my_game::other_name_space::TableB<'b >>) {
    self.fbb_.push_slot_always::<flatbuffers::WIPOffset<my_game::other_name_space::TableB>>(TableA::VT_B, b);
  }
  #[inline]
  pub fn new(_fbb: &'b mut flatbuffers::FlatBufferBuilder<'a>) -> TableABuilder<'a, 'b> {
    let start = _fbb.start_table();
    TableABuilder {
      fbb_: _fbb,
      start_: start,
    }
  }
  #[inline]
  pub fn finish(self) -> flatbuffers::WIPOffset<TableA<'a>> {
    let o = self.fbb_.end_table(self.start_);
    flatbuffers::WIPOffset::new(o.value())
  }
}

