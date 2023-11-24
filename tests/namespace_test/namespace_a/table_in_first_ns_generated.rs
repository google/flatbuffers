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
#[allow(unused_imports)]
use self::flatbuffers::{EndianScalar, Follow};
#[allow(unused_imports)]
use super::*;
pub enum TableInFirstNSOffset {}
#[derive(Copy, Clone, PartialEq)]

pub struct TableInFirstNS<'a> {
  pub _tab: flatbuffers::Table<'a>,
}

impl<'a> flatbuffers::Follow<'a> for TableInFirstNS<'a> {
  type Inner = TableInFirstNS<'a>;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    Self { _tab: flatbuffers::Table::new(buf, loc) }
  }
}

impl<'a> TableInFirstNS<'a> {
  pub const VT_FOO_TABLE: flatbuffers::VOffsetT = 4;
  pub const VT_FOO_ENUM: flatbuffers::VOffsetT = 6;
  pub const VT_FOO_UNION_TYPE: flatbuffers::VOffsetT = 8;
  pub const VT_FOO_UNION: flatbuffers::VOffsetT = 10;
  pub const VT_FOO_STRUCT: flatbuffers::VOffsetT = 12;

  pub const fn get_fully_qualified_name() -> &'static str {
    "NamespaceA.TableInFirstNS"
  }

  #[inline]
  pub unsafe fn init_from_table(table: flatbuffers::Table<'a>) -> Self {
    TableInFirstNS { _tab: table }
  }
  #[allow(unused_mut)]
  pub fn create<'bldr: 'args, 'args: 'mut_bldr, 'mut_bldr, A: flatbuffers::Allocator + 'bldr>(
    _fbb: &'mut_bldr mut flatbuffers::FlatBufferBuilder<'bldr, A>,
    args: &'args TableInFirstNSArgs<'args>
  ) -> flatbuffers::WIPOffset<TableInFirstNS<'bldr>> {
    let mut builder = TableInFirstNSBuilder::new(_fbb);
    if let Some(x) = args.foo_struct { builder.add_foo_struct(x); }
    if let Some(x) = args.foo_union { builder.add_foo_union(x); }
    if let Some(x) = args.foo_table { builder.add_foo_table(x); }
    builder.add_foo_union_type(args.foo_union_type);
    builder.add_foo_enum(args.foo_enum);
    builder.finish()
  }

  pub fn unpack(&self) -> TableInFirstNST {
    let foo_table = self.foo_table().map(|x| {
      Box::new(x.unpack())
    });
    let foo_enum = self.foo_enum();
    let foo_union = match self.foo_union_type() {
      namespace_b::UnionInNestedNS::NONE => namespace_b::UnionInNestedNST::NONE,
      namespace_b::UnionInNestedNS::TableInNestedNS => namespace_b::UnionInNestedNST::TableInNestedNS(Box::new(
        self.foo_union_as_table_in_nested_ns()
            .expect("Invalid union table, expected `namespace_b::UnionInNestedNS::TableInNestedNS`.")
            .unpack()
      )),
      _ => namespace_b::UnionInNestedNST::NONE,
    };
    let foo_struct = self.foo_struct().map(|x| {
      x.unpack()
    });
    TableInFirstNST {
      foo_table,
      foo_enum,
      foo_union,
      foo_struct,
    }
  }

  #[inline]
  pub fn foo_table(&self) -> Option<namespace_b::TableInNestedNS<'a>> {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<flatbuffers::ForwardsUOffset<namespace_b::TableInNestedNS>>(TableInFirstNS::VT_FOO_TABLE, None)}
  }
  #[inline]
  pub fn foo_enum(&self) -> namespace_b::EnumInNestedNS {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<namespace_b::EnumInNestedNS>(TableInFirstNS::VT_FOO_ENUM, Some(namespace_b::EnumInNestedNS::A)).unwrap()}
  }
  #[inline]
  pub fn foo_union_type(&self) -> namespace_b::UnionInNestedNS {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<namespace_b::UnionInNestedNS>(TableInFirstNS::VT_FOO_UNION_TYPE, Some(namespace_b::UnionInNestedNS::NONE)).unwrap()}
  }
  #[inline]
  pub fn foo_union(&self) -> Option<flatbuffers::Table<'a>> {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<flatbuffers::ForwardsUOffset<flatbuffers::Table<'a>>>(TableInFirstNS::VT_FOO_UNION, None)}
  }
  #[inline]
  pub fn foo_struct(&self) -> Option<&'a namespace_b::StructInNestedNS> {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<namespace_b::StructInNestedNS>(TableInFirstNS::VT_FOO_STRUCT, None)}
  }
  #[inline]
  #[allow(non_snake_case)]
  pub fn foo_union_as_table_in_nested_ns(&self) -> Option<namespace_b::TableInNestedNS<'a>> {
    if self.foo_union_type() == namespace_b::UnionInNestedNS::TableInNestedNS {
      self.foo_union().map(|t| {
       // Safety:
       // Created from a valid Table for this object
       // Which contains a valid union in this slot
       unsafe { namespace_b::TableInNestedNS::init_from_table(t) }
     })
    } else {
      None
    }
  }

}

impl flatbuffers::Verifiable for TableInFirstNS<'_> {
  #[inline]
  fn run_verifier(
    v: &mut flatbuffers::Verifier, pos: usize
  ) -> Result<(), flatbuffers::InvalidFlatbuffer> {
    use self::flatbuffers::Verifiable;
    v.visit_table(pos)?
     .visit_field::<flatbuffers::ForwardsUOffset<namespace_b::TableInNestedNS>>("foo_table", Self::VT_FOO_TABLE, false)?
     .visit_field::<namespace_b::EnumInNestedNS>("foo_enum", Self::VT_FOO_ENUM, false)?
     .visit_union::<namespace_b::UnionInNestedNS, _>("foo_union_type", Self::VT_FOO_UNION_TYPE, "foo_union", Self::VT_FOO_UNION, false, |key, v, pos| {
        match key {
          namespace_b::UnionInNestedNS::TableInNestedNS => v.verify_union_variant::<flatbuffers::ForwardsUOffset<namespace_b::TableInNestedNS>>("namespace_b::UnionInNestedNS::TableInNestedNS", pos),
          _ => Ok(()),
        }
     })?
     .visit_field::<namespace_b::StructInNestedNS>("foo_struct", Self::VT_FOO_STRUCT, false)?
     .finish();
    Ok(())
  }
}
pub struct TableInFirstNSArgs<'a> {
    pub foo_table: Option<flatbuffers::WIPOffset<namespace_b::TableInNestedNS<'a>>>,
    pub foo_enum: namespace_b::EnumInNestedNS,
    pub foo_union_type: namespace_b::UnionInNestedNS,
    pub foo_union: Option<flatbuffers::WIPOffset<flatbuffers::UnionWIPOffset>>,
    pub foo_struct: Option<&'a namespace_b::StructInNestedNS>,
}
impl<'a> Default for TableInFirstNSArgs<'a> {
  #[inline]
  fn default() -> Self {
    TableInFirstNSArgs {
      foo_table: None,
      foo_enum: namespace_b::EnumInNestedNS::A,
      foo_union_type: namespace_b::UnionInNestedNS::NONE,
      foo_union: None,
      foo_struct: None,
    }
  }
}

pub struct TableInFirstNSBuilder<'a: 'b, 'b, A: flatbuffers::Allocator + 'a> {
  fbb_: &'b mut flatbuffers::FlatBufferBuilder<'a, A>,
  start_: flatbuffers::WIPOffset<flatbuffers::TableUnfinishedWIPOffset>,
}
impl<'a: 'b, 'b, A: flatbuffers::Allocator + 'a> TableInFirstNSBuilder<'a, 'b, A> {
  #[inline]
  pub fn add_foo_table(&mut self, foo_table: flatbuffers::WIPOffset<namespace_b::TableInNestedNS<'b >>) {
    self.fbb_.push_slot_always::<flatbuffers::WIPOffset<namespace_b::TableInNestedNS>>(TableInFirstNS::VT_FOO_TABLE, foo_table);
  }
  #[inline]
  pub fn add_foo_enum(&mut self, foo_enum: namespace_b::EnumInNestedNS) {
    self.fbb_.push_slot::<namespace_b::EnumInNestedNS>(TableInFirstNS::VT_FOO_ENUM, foo_enum, namespace_b::EnumInNestedNS::A);
  }
  #[inline]
  pub fn add_foo_union_type(&mut self, foo_union_type: namespace_b::UnionInNestedNS) {
    self.fbb_.push_slot::<namespace_b::UnionInNestedNS>(TableInFirstNS::VT_FOO_UNION_TYPE, foo_union_type, namespace_b::UnionInNestedNS::NONE);
  }
  #[inline]
  pub fn add_foo_union(&mut self, foo_union: flatbuffers::WIPOffset<flatbuffers::UnionWIPOffset>) {
    self.fbb_.push_slot_always::<flatbuffers::WIPOffset<_>>(TableInFirstNS::VT_FOO_UNION, foo_union);
  }
  #[inline]
  pub fn add_foo_struct(&mut self, foo_struct: &namespace_b::StructInNestedNS) {
    self.fbb_.push_slot_always::<&namespace_b::StructInNestedNS>(TableInFirstNS::VT_FOO_STRUCT, foo_struct);
  }
  #[inline]
  pub fn new(_fbb: &'b mut flatbuffers::FlatBufferBuilder<'a, A>) -> TableInFirstNSBuilder<'a, 'b, A> {
    let start = _fbb.start_table();
    TableInFirstNSBuilder {
      fbb_: _fbb,
      start_: start,
    }
  }
  #[inline]
  pub fn finish(self) -> flatbuffers::WIPOffset<TableInFirstNS<'a>> {
    let o = self.fbb_.end_table(self.start_);
    flatbuffers::WIPOffset::new(o.value())
  }
}

impl core::fmt::Debug for TableInFirstNS<'_> {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    let mut ds = f.debug_struct("TableInFirstNS");
      ds.field("foo_table", &self.foo_table());
      ds.field("foo_enum", &self.foo_enum());
      ds.field("foo_union_type", &self.foo_union_type());
      match self.foo_union_type() {
        namespace_b::UnionInNestedNS::TableInNestedNS => {
          if let Some(x) = self.foo_union_as_table_in_nested_ns() {
            ds.field("foo_union", &x)
          } else {
            ds.field("foo_union", &"InvalidFlatbuffer: Union discriminant does not match value.")
          }
        },
        _ => {
          let x: Option<()> = None;
          ds.field("foo_union", &x)
        },
      };
      ds.field("foo_struct", &self.foo_struct());
      ds.finish()
  }
}
#[non_exhaustive]
#[derive(Debug, Clone, PartialEq)]
pub struct TableInFirstNST {
  pub foo_table: Option<Box<namespace_b::TableInNestedNST>>,
  pub foo_enum: namespace_b::EnumInNestedNS,
  pub foo_union: namespace_b::UnionInNestedNST,
  pub foo_struct: Option<namespace_b::StructInNestedNST>,
}
impl Default for TableInFirstNST {
  fn default() -> Self {
    Self {
      foo_table: None,
      foo_enum: namespace_b::EnumInNestedNS::A,
      foo_union: namespace_b::UnionInNestedNST::NONE,
      foo_struct: None,
    }
  }
}
impl TableInFirstNST {
  pub fn pack<'b, A: flatbuffers::Allocator + 'b>(
    &self,
    _fbb: &mut flatbuffers::FlatBufferBuilder<'b, A>
  ) -> flatbuffers::WIPOffset<TableInFirstNS<'b>> {
    let foo_table = self.foo_table.as_ref().map(|x|{
      x.pack(_fbb)
    });
    let foo_enum = self.foo_enum;
    let foo_union_type = self.foo_union.union_in_nested_ns_type();
    let foo_union = self.foo_union.pack(_fbb);
    let foo_struct_tmp = self.foo_struct.as_ref().map(|x| x.pack());
    let foo_struct = foo_struct_tmp.as_ref();
    TableInFirstNS::create(_fbb, &TableInFirstNSArgs{
      foo_table,
      foo_enum,
      foo_union_type,
      foo_union,
      foo_struct,
    })
  }
}
