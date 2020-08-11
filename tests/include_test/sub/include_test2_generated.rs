// automatically generated by the FlatBuffers compiler, do not modify



use crate::include_test1_generated::*;
use std::mem;
use std::cmp::Ordering;

extern crate flatbuffers;
use self::flatbuffers::EndianScalar;

#[allow(unused_imports, dead_code)]
pub mod my_game {

  use crate::include_test1_generated::*;
  use std::mem;
  use std::cmp::Ordering;

  extern crate flatbuffers;
  use self::flatbuffers::EndianScalar;
#[allow(unused_imports, dead_code)]
pub mod other_name_space {

  use crate::include_test1_generated::*;
  use std::mem;
  use std::cmp::Ordering;

  extern crate flatbuffers;
  use self::flatbuffers::EndianScalar;

#[non_exhaustive]
#[allow(non_camel_case_types)]
#[repr(i64)]
#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Debug)]
pub enum FromInclude {
  IncludeVal = 0,
}

pub const ENUM_MIN_FROM_INCLUDE: i64 = 0;
pub const ENUM_MAX_FROM_INCLUDE: i64 = 0;

impl<'a> flatbuffers::Follow<'a> for FromInclude {
  type Inner = Self;
  #[inline]
  fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    flatbuffers::read_scalar_at::<Self>(buf, loc)
  }
}

impl flatbuffers::EndianScalar for FromInclude {
  #[inline]
  fn to_little_endian(self) -> Self {
    let n = i64::to_le(self as i64);
    let p = &n as *const i64 as *const FromInclude;
    unsafe { *p }
  }
  #[inline]
  fn from_little_endian(self) -> Self {
    let n = i64::from_le(self as i64);
    let p = &n as *const i64 as *const FromInclude;
    unsafe { *p }
  }
}

impl flatbuffers::Push for FromInclude {
    type Output = FromInclude;
    #[inline]
    fn push(&self, dst: &mut [u8], _rest: &[u8]) {
        flatbuffers::emplace_scalar::<FromInclude>(dst, *self);
    }
}

#[allow(non_camel_case_types)]
pub const ENUM_VALUES_FROM_INCLUDE: [FromInclude; 1] = [
  FromInclude::IncludeVal
];

#[allow(non_camel_case_types)]
pub const ENUM_NAMES_FROM_INCLUDE: [&str; 1] = [
    "IncludeVal"
];

pub fn enum_name_from_include(e: FromInclude) -> &'static str {
  let index = e as i64;
  ENUM_NAMES_FROM_INCLUDE[index as usize]
}

// struct Unused, aligned to 4
#[repr(C, align(4))]
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Unused {
  a_: i32,
} // pub struct Unused
impl flatbuffers::SafeSliceAccess for Unused {}
impl<'a> flatbuffers::Follow<'a> for Unused {
  type Inner = &'a Unused;
  #[inline]
  fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    <&'a Unused>::follow(buf, loc)
  }
}
impl<'a> flatbuffers::Follow<'a> for &'a Unused {
  type Inner = &'a Unused;
  #[inline]
  fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    flatbuffers::follow_cast_ref::<Unused>(buf, loc)
  }
}
impl<'b> flatbuffers::Push for Unused {
    type Output = Unused;
    #[inline]
    fn push(&self, dst: &mut [u8], _rest: &[u8]) {
        let src = unsafe {
            ::std::slice::from_raw_parts(self as *const Unused as *const u8, Self::size())
        };
        dst.copy_from_slice(src);
    }
}
impl<'b> flatbuffers::Push for &'b Unused {
    type Output = Unused;

    #[inline]
    fn push(&self, dst: &mut [u8], _rest: &[u8]) {
        let src = unsafe {
            ::std::slice::from_raw_parts(*self as *const Unused as *const u8, Self::size())
        };
        dst.copy_from_slice(src);
    }
}


impl Unused {
  pub fn new(_a: i32) -> Self {
    Unused {
      a_: _a.to_little_endian(),

    }
  }
  pub fn a(&self) -> i32 {
    self.a_.from_little_endian()
  }
}

pub enum TableBOffset {}
#[derive(Copy, Clone, Debug, PartialEq)]

pub struct TableB<'a> {
  pub _tab: flatbuffers::Table<'a>,
}

impl<'a> flatbuffers::Follow<'a> for TableB<'a> {
    type Inner = TableB<'a>;
    #[inline]
    fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
        Self { _tab: flatbuffers::Table { buf, loc } }
    }
}

impl<'a> TableB<'a> {
    #[inline]
    pub fn init_from_table(table: flatbuffers::Table<'a>) -> Self {
        TableB { _tab: table }
    }
    #[allow(unused_mut)]
    pub fn create<'bldr: 'args, 'args: 'mut_bldr, 'mut_bldr>(
        _fbb: &'mut_bldr mut flatbuffers::FlatBufferBuilder<'bldr>,
        args: &'args TableBArgs<'args>) -> flatbuffers::WIPOffset<TableB<'bldr>> {
      let mut builder = TableBBuilder::new(_fbb);
      if let Some(x) = args.a { builder.add_a(x); }
      builder.finish()
    }

    pub const VT_A: flatbuffers::VOffsetT = 4;

  #[inline]
  pub fn a(&self) -> Option<super::super::TableA<'a>> {
    self._tab.get::<flatbuffers::ForwardsUOffset<super::super::TableA<'a>>>(TableB::VT_A, None)
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

}  // pub mod OtherNameSpace
}  // pub mod MyGame

