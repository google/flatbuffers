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
pub enum StatOffset {}
#[derive(Copy, Clone, PartialEq)]

pub struct Stat<'a> {
  pub _tab: flatbuffers::Table<'a>,
}

impl<'a> flatbuffers::Follow<'a> for Stat<'a> {
  type Inner = Stat<'a>;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    Self { _tab: unsafe { flatbuffers::Table::new(buf, loc) } }
  }
}

impl<'a> Stat<'a> {
  pub const VT_ID: flatbuffers::VOffsetT = 4;
  pub const VT_VAL: flatbuffers::VOffsetT = 6;
  pub const VT_COUNT: flatbuffers::VOffsetT = 8;

  pub const fn get_fully_qualified_name() -> &'static str {
    "MyGame.Example.Stat"
  }

  #[inline]
  pub unsafe fn init_from_table(table: flatbuffers::Table<'a>) -> Self {
    Stat { _tab: table }
  }
  #[allow(unused_mut)]
  pub fn create<'bldr: 'args, 'args: 'mut_bldr, 'mut_bldr, A: flatbuffers::Allocator + 'bldr>(
    _fbb: &'mut_bldr mut flatbuffers::FlatBufferBuilder<'bldr, A>,
    args: &'args StatArgs<'args>
  ) -> flatbuffers::WIPOffset<Stat<'bldr>> {
    let mut builder = StatBuilder::new(_fbb);
    builder.add_val(args.val);
    if let Some(x) = args.id { builder.add_id(x); }
    builder.add_count(args.count);
    builder.finish()
  }

  pub fn unpack(&self) -> StatT {
    let id = self.id().map(|x| {
      x.to_string()
    });
    let val = self.val();
    let count = self.count();
    StatT {
      id,
      val,
      count,
    }
  }

  #[inline]
  pub fn id(&self) -> Option<&'a str> {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<flatbuffers::ForwardsUOffset<&str>>(Stat::VT_ID, None)}
  }
  #[inline]
  pub fn val(&self) -> i64 {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<i64>(Stat::VT_VAL, Some(0)).unwrap()}
  }
  #[inline]
  pub fn count(&self) -> u16 {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<u16>(Stat::VT_COUNT, Some(0)).unwrap()}
  }
  #[inline]
  pub fn key_compare_less_than(&self, o: &Stat) -> bool {
    self.count() < o.count()
  }

  #[inline]
  pub fn key_compare_with_value(&self, val: u16) -> ::core::cmp::Ordering {
    let key = self.count();
    key.cmp(&val)
  }
}

impl flatbuffers::Verifiable for Stat<'_> {
  #[inline]
  fn run_verifier(
    v: &mut flatbuffers::Verifier, pos: usize
  ) -> Result<(), flatbuffers::InvalidFlatbuffer> {
    use self::flatbuffers::Verifiable;
    v.visit_table(pos)?
     .visit_field::<flatbuffers::ForwardsUOffset<&str>>("id", Self::VT_ID, false)?
     .visit_field::<i64>("val", Self::VT_VAL, false)?
     .visit_field::<u16>("count", Self::VT_COUNT, false)?
     .finish();
    Ok(())
  }
}
pub struct StatArgs<'a> {
    pub id: Option<flatbuffers::WIPOffset<&'a str>>,
    pub val: i64,
    pub count: u16,
}
impl<'a> Default for StatArgs<'a> {
  #[inline]
  fn default() -> Self {
    StatArgs {
      id: None,
      val: 0,
      count: 0,
    }
  }
}

impl Serialize for Stat<'_> {
  fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
  where
    S: Serializer,
  {
    let mut s = serializer.serialize_struct("Stat", 3)?;
      if let Some(f) = self.id() {
        s.serialize_field("id", &f)?;
      } else {
        s.skip_field("id")?;
      }
      s.serialize_field("val", &self.val())?;
      s.serialize_field("count", &self.count())?;
    s.end()
  }
}

pub struct StatBuilder<'a: 'b, 'b, A: flatbuffers::Allocator + 'a> {
  fbb_: &'b mut flatbuffers::FlatBufferBuilder<'a, A>,
  start_: flatbuffers::WIPOffset<flatbuffers::TableUnfinishedWIPOffset>,
}
impl<'a: 'b, 'b, A: flatbuffers::Allocator + 'a> StatBuilder<'a, 'b, A> {
  #[inline]
  pub fn add_id(&mut self, id: flatbuffers::WIPOffset<&'b  str>) {
    self.fbb_.push_slot_always::<flatbuffers::WIPOffset<_>>(Stat::VT_ID, id);
  }
  #[inline]
  pub fn add_val(&mut self, val: i64) {
    self.fbb_.push_slot::<i64>(Stat::VT_VAL, val, 0);
  }
  #[inline]
  pub fn add_count(&mut self, count: u16) {
    self.fbb_.push_slot::<u16>(Stat::VT_COUNT, count, 0);
  }
  #[inline]
  pub fn new(_fbb: &'b mut flatbuffers::FlatBufferBuilder<'a, A>) -> StatBuilder<'a, 'b, A> {
    let start = _fbb.start_table();
    StatBuilder {
      fbb_: _fbb,
      start_: start,
    }
  }
  #[inline]
  pub fn finish(self) -> flatbuffers::WIPOffset<Stat<'a>> {
    let o = self.fbb_.end_table(self.start_);
    flatbuffers::WIPOffset::new(o.value())
  }
}

impl core::fmt::Debug for Stat<'_> {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    let mut ds = f.debug_struct("Stat");
      ds.field("id", &self.id());
      ds.field("val", &self.val());
      ds.field("count", &self.count());
      ds.finish()
  }
}
#[non_exhaustive]
#[derive(Debug, Clone, PartialEq)]
pub struct StatT {
  pub id: Option<String>,
  pub val: i64,
  pub count: u16,
}
impl Default for StatT {
  fn default() -> Self {
    Self {
      id: None,
      val: 0,
      count: 0,
    }
  }
}
impl StatT {
  pub fn pack<'b, A: flatbuffers::Allocator + 'b>(
    &self,
    _fbb: &mut flatbuffers::FlatBufferBuilder<'b, A>
  ) -> flatbuffers::WIPOffset<Stat<'b>> {
    let id = self.id.as_ref().map(|x|{
      _fbb.create_string(x)
    });
    let val = self.val;
    let count = self.count;
    Stat::create(_fbb, &StatArgs{
      id,
      val,
      count,
    })
  }
}
