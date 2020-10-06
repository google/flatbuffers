// automatically generated by the FlatBuffers compiler, do not modify



use std::mem;
use std::cmp::Ordering;
use std::convert::TryFrom;
use std::convert::TryInto;

extern crate flatbuffers;
use self::flatbuffers::EndianScalar;

#[allow(unused_imports, dead_code)]
pub mod namespace_a {

  use std::mem;
  use std::cmp::Ordering;
  use std::convert::TryFrom;
  use std::convert::TryInto;

  extern crate flatbuffers;
  use self::flatbuffers::EndianScalar;
#[allow(unused_imports, dead_code)]
pub mod namespace_b {

  use std::mem;
  use std::cmp::Ordering;
  use std::convert::TryFrom;
  use std::convert::TryInto;

  extern crate flatbuffers;
  use self::flatbuffers::EndianScalar;

#[allow(non_camel_case_types)]
#[repr(i8)]
#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Debug)]
pub enum EnumInNestedNS {
  A = 0,
  B = 1,
  C = 2,

}

pub const ENUM_MIN_ENUM_IN_NESTED_NS: i8 = 0;
pub const ENUM_MAX_ENUM_IN_NESTED_NS: i8 = 2;

impl<'a> flatbuffers::Follow<'a> for EnumInNestedNS {
  type Inner = Self;
  #[inline]
  fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    flatbuffers::read_scalar_at::<Self>(buf, loc)
  }
}

impl flatbuffers::EndianScalar for EnumInNestedNS {
  #[inline]
  fn to_little_endian(self) -> Self {
    let n = i8::to_le(self as i8);
    let p = &n as *const i8 as *const EnumInNestedNS;
    unsafe { *p }
  }
  #[inline]
  fn from_little_endian(self) -> Self {
    let n = i8::from_le(self as i8);
    let p = &n as *const i8 as *const EnumInNestedNS;
    unsafe { *p }
  }
}

impl flatbuffers::Push for EnumInNestedNS {
    type Output = EnumInNestedNS;
    #[inline]
    fn push(&self, dst: &mut [u8], _rest: &[u8]) {
        flatbuffers::emplace_scalar::<EnumInNestedNS>(dst, *self);
    }
}

impl TryFrom<i8> for EnumInNestedNS {
    type Error = flatbuffers::ConvertError<i8>;

    #[inline]
    fn try_from(value: i8) -> Result<Self, Self::Error> {
        match value {
          0 => Ok(EnumInNestedNS::A),
          1 => Ok(EnumInNestedNS::B),
          2 => Ok(EnumInNestedNS::C),
          _ => Err(Self::Error::UnknownVariant(value))
        }
    }
}

#[allow(non_camel_case_types)]
pub const ENUM_VALUES_ENUM_IN_NESTED_NS: [EnumInNestedNS; 3] = [
  EnumInNestedNS::A,
  EnumInNestedNS::B,
  EnumInNestedNS::C
];

#[allow(non_camel_case_types)]
pub const ENUM_NAMES_ENUM_IN_NESTED_NS: [&str; 3] = [
    "A",
    "B",
    "C"
];

pub fn enum_name_enum_in_nested_ns(e: EnumInNestedNS) -> &'static str {
  let index = e as i8;
  ENUM_NAMES_ENUM_IN_NESTED_NS[index as usize]
}

// struct StructInNestedNS, aligned to 4
#[repr(C, align(4))]
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct StructInNestedNS {
  a_: i32,
  b_: i32,
} // pub struct StructInNestedNS
impl flatbuffers::SafeSliceAccess for StructInNestedNS {}
impl<'a> flatbuffers::Follow<'a> for StructInNestedNS {
  type Inner = &'a StructInNestedNS;
  #[inline]
  fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    <&'a StructInNestedNS>::follow(buf, loc)
  }
}
impl<'a> flatbuffers::Follow<'a> for &'a StructInNestedNS {
  type Inner = &'a StructInNestedNS;
  #[inline]
  fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    flatbuffers::follow_cast_ref::<StructInNestedNS>(buf, loc)
  }
}
impl<'b> flatbuffers::Push for StructInNestedNS {
    type Output = StructInNestedNS;
    #[inline]
    fn push(&self, dst: &mut [u8], _rest: &[u8]) {
        let src = unsafe {
            ::std::slice::from_raw_parts(self as *const StructInNestedNS as *const u8, Self::size())
        };
        dst.copy_from_slice(src);
    }
}
impl<'b> flatbuffers::Push for &'b StructInNestedNS {
    type Output = StructInNestedNS;

    #[inline]
    fn push(&self, dst: &mut [u8], _rest: &[u8]) {
        let src = unsafe {
            ::std::slice::from_raw_parts(*self as *const StructInNestedNS as *const u8, Self::size())
        };
        dst.copy_from_slice(src);
    }
}


impl StructInNestedNS {
  pub fn new(_a: i32, _b: i32) -> Self {
    StructInNestedNS {
      a_: _a.to_little_endian(),
      b_: _b.to_little_endian(),

    }
  }
    pub const fn get_fully_qualified_name() -> &'static str {
        "NamespaceA.NamespaceB.StructInNestedNS"
    }

  pub fn a(&self) -> i32 {
    self.a_.from_little_endian()
  }
  pub fn b(&self) -> i32 {
    self.b_.from_little_endian()
  }
}

pub enum TableInNestedNSOffset {}
#[derive(Copy, Clone, Debug, PartialEq)]

pub struct TableInNestedNS<'a> {
  pub _tab: flatbuffers::Table<'a>,
}

impl<'a> flatbuffers::Follow<'a> for TableInNestedNS<'a> {
    type Inner = TableInNestedNS<'a>;
    #[inline]
    fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
        Self { _tab: flatbuffers::Table { buf, loc } }
    }
}

impl<'a> TableInNestedNS<'a> {
    pub const fn get_fully_qualified_name() -> &'static str {
        "NamespaceA.NamespaceB.TableInNestedNS"
    }

    #[inline]
    pub fn init_from_table(table: flatbuffers::Table<'a>) -> Self {
        TableInNestedNS {
            _tab: table,
        }
    }
    #[allow(unused_mut)]
    pub fn create<'bldr: 'args, 'args: 'mut_bldr, 'mut_bldr>(
        _fbb: &'mut_bldr mut flatbuffers::FlatBufferBuilder<'bldr>,
        args: &'args TableInNestedNSArgs) -> flatbuffers::WIPOffset<TableInNestedNS<'bldr>> {
      let mut builder = TableInNestedNSBuilder::new(_fbb);
      builder.add_foo(args.foo);
      builder.finish()
    }

    pub const VT_FOO: flatbuffers::VOffsetT = 4;

  #[inline]
  pub fn foo(&self) -> i32 {
    self._tab.get::<i32>(TableInNestedNS::VT_FOO, Some(0)).unwrap()
  }
}

pub struct TableInNestedNSArgs {
    pub foo: i32,
}
impl<'a> Default for TableInNestedNSArgs {
    #[inline]
    fn default() -> Self {
        TableInNestedNSArgs {
            foo: 0,
        }
    }
}
pub struct TableInNestedNSBuilder<'a: 'b, 'b> {
  fbb_: &'b mut flatbuffers::FlatBufferBuilder<'a>,
  start_: flatbuffers::WIPOffset<flatbuffers::TableUnfinishedWIPOffset>,
}
impl<'a: 'b, 'b> TableInNestedNSBuilder<'a, 'b> {
  #[inline]
  pub fn add_foo(&mut self, foo: i32) {
    self.fbb_.push_slot::<i32>(TableInNestedNS::VT_FOO, foo, 0);
  }
  #[inline]
  pub fn new(_fbb: &'b mut flatbuffers::FlatBufferBuilder<'a>) -> TableInNestedNSBuilder<'a, 'b> {
    let start = _fbb.start_table();
    TableInNestedNSBuilder {
      fbb_: _fbb,
      start_: start,
    }
  }
  #[inline]
  pub fn finish(self) -> flatbuffers::WIPOffset<TableInNestedNS<'a>> {
    let o = self.fbb_.end_table(self.start_);
    flatbuffers::WIPOffset::new(o.value())
  }
}

}  // pub mod NamespaceB
}  // pub mod NamespaceA

