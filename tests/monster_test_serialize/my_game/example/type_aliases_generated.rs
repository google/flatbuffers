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
pub enum TypeAliasesOffset {}
#[derive(Copy, Clone, PartialEq)]

pub struct TypeAliases<'a> {
  pub _tab: flatbuffers::Table<'a>,
}

impl<'a> flatbuffers::Follow<'a> for TypeAliases<'a> {
  type Inner = TypeAliases<'a>;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    Self { _tab: unsafe { flatbuffers::Table::new(buf, loc) } }
  }
}

impl<'a> TypeAliases<'a> {
  pub const VT_I8_: flatbuffers::VOffsetT = 4;
  pub const VT_U8_: flatbuffers::VOffsetT = 6;
  pub const VT_I16_: flatbuffers::VOffsetT = 8;
  pub const VT_U16_: flatbuffers::VOffsetT = 10;
  pub const VT_I32_: flatbuffers::VOffsetT = 12;
  pub const VT_U32_: flatbuffers::VOffsetT = 14;
  pub const VT_I64_: flatbuffers::VOffsetT = 16;
  pub const VT_U64_: flatbuffers::VOffsetT = 18;
  pub const VT_F32_: flatbuffers::VOffsetT = 20;
  pub const VT_F64_: flatbuffers::VOffsetT = 22;
  pub const VT_V8: flatbuffers::VOffsetT = 24;
  pub const VT_VF64: flatbuffers::VOffsetT = 26;

  pub const fn get_fully_qualified_name() -> &'static str {
    "MyGame.Example.TypeAliases"
  }

  #[inline]
  pub unsafe fn init_from_table(table: flatbuffers::Table<'a>) -> Self {
    TypeAliases { _tab: table }
  }
  #[allow(unused_mut)]
  pub fn create<'bldr: 'args, 'args: 'mut_bldr, 'mut_bldr, A: flatbuffers::Allocator + 'bldr>(
    _fbb: &'mut_bldr mut flatbuffers::FlatBufferBuilder<'bldr, A>,
    args: &'args TypeAliasesArgs<'args>
  ) -> flatbuffers::WIPOffset<TypeAliases<'bldr>> {
    let mut builder = TypeAliasesBuilder::new(_fbb);
    builder.add_f64_(args.f64_);
    builder.add_u64_(args.u64_);
    builder.add_i64_(args.i64_);
    if let Some(x) = args.vf64 { builder.add_vf64(x); }
    if let Some(x) = args.v8 { builder.add_v8(x); }
    builder.add_f32_(args.f32_);
    builder.add_u32_(args.u32_);
    builder.add_i32_(args.i32_);
    builder.add_u16_(args.u16_);
    builder.add_i16_(args.i16_);
    builder.add_u8_(args.u8_);
    builder.add_i8_(args.i8_);
    builder.finish()
  }

  pub fn unpack(&self) -> TypeAliasesT {
    let i8_ = self.i8_();
    let u8_ = self.u8_();
    let i16_ = self.i16_();
    let u16_ = self.u16_();
    let i32_ = self.i32_();
    let u32_ = self.u32_();
    let i64_ = self.i64_();
    let u64_ = self.u64_();
    let f32_ = self.f32_();
    let f64_ = self.f64_();
    let v8 = self.v8().map(|x| {
      x.into_iter().collect()
    });
    let vf64 = self.vf64().map(|x| {
      x.into_iter().collect()
    });
    TypeAliasesT {
      i8_,
      u8_,
      i16_,
      u16_,
      i32_,
      u32_,
      i64_,
      u64_,
      f32_,
      f64_,
      v8,
      vf64,
    }
  }

  #[inline]
  pub fn i8_(&self) -> i8 {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<i8>(TypeAliases::VT_I8_, Some(0)).unwrap()}
  }
  #[inline]
  pub fn u8_(&self) -> u8 {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<u8>(TypeAliases::VT_U8_, Some(0)).unwrap()}
  }
  #[inline]
  pub fn i16_(&self) -> i16 {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<i16>(TypeAliases::VT_I16_, Some(0)).unwrap()}
  }
  #[inline]
  pub fn u16_(&self) -> u16 {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<u16>(TypeAliases::VT_U16_, Some(0)).unwrap()}
  }
  #[inline]
  pub fn i32_(&self) -> i32 {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<i32>(TypeAliases::VT_I32_, Some(0)).unwrap()}
  }
  #[inline]
  pub fn u32_(&self) -> u32 {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<u32>(TypeAliases::VT_U32_, Some(0)).unwrap()}
  }
  #[inline]
  pub fn i64_(&self) -> i64 {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<i64>(TypeAliases::VT_I64_, Some(0)).unwrap()}
  }
  #[inline]
  pub fn u64_(&self) -> u64 {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<u64>(TypeAliases::VT_U64_, Some(0)).unwrap()}
  }
  #[inline]
  pub fn f32_(&self) -> f32 {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<f32>(TypeAliases::VT_F32_, Some(0.0)).unwrap()}
  }
  #[inline]
  pub fn f64_(&self) -> f64 {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<f64>(TypeAliases::VT_F64_, Some(0.0)).unwrap()}
  }
  #[inline]
  pub fn v8(&self) -> Option<flatbuffers::Vector<'a, i8>> {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<flatbuffers::ForwardsUOffset<flatbuffers::Vector<'a, i8>>>(TypeAliases::VT_V8, None)}
  }
  #[inline]
  pub fn vf64(&self) -> Option<flatbuffers::Vector<'a, f64>> {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<flatbuffers::ForwardsUOffset<flatbuffers::Vector<'a, f64>>>(TypeAliases::VT_VF64, None)}
  }
}

impl flatbuffers::Verifiable for TypeAliases<'_> {
  #[inline]
  fn run_verifier(
    v: &mut flatbuffers::Verifier, pos: usize
  ) -> Result<(), flatbuffers::InvalidFlatbuffer> {
    use self::flatbuffers::Verifiable;
    v.visit_table(pos)?
     .visit_field::<i8>("i8_", Self::VT_I8_, false)?
     .visit_field::<u8>("u8_", Self::VT_U8_, false)?
     .visit_field::<i16>("i16_", Self::VT_I16_, false)?
     .visit_field::<u16>("u16_", Self::VT_U16_, false)?
     .visit_field::<i32>("i32_", Self::VT_I32_, false)?
     .visit_field::<u32>("u32_", Self::VT_U32_, false)?
     .visit_field::<i64>("i64_", Self::VT_I64_, false)?
     .visit_field::<u64>("u64_", Self::VT_U64_, false)?
     .visit_field::<f32>("f32_", Self::VT_F32_, false)?
     .visit_field::<f64>("f64_", Self::VT_F64_, false)?
     .visit_field::<flatbuffers::ForwardsUOffset<flatbuffers::Vector<'_, i8>>>("v8", Self::VT_V8, false)?
     .visit_field::<flatbuffers::ForwardsUOffset<flatbuffers::Vector<'_, f64>>>("vf64", Self::VT_VF64, false)?
     .finish();
    Ok(())
  }
}
pub struct TypeAliasesArgs<'a> {
    pub i8_: i8,
    pub u8_: u8,
    pub i16_: i16,
    pub u16_: u16,
    pub i32_: i32,
    pub u32_: u32,
    pub i64_: i64,
    pub u64_: u64,
    pub f32_: f32,
    pub f64_: f64,
    pub v8: Option<flatbuffers::WIPOffset<flatbuffers::Vector<'a, i8>>>,
    pub vf64: Option<flatbuffers::WIPOffset<flatbuffers::Vector<'a, f64>>>,
}
impl<'a> Default for TypeAliasesArgs<'a> {
  #[inline]
  fn default() -> Self {
    TypeAliasesArgs {
      i8_: 0,
      u8_: 0,
      i16_: 0,
      u16_: 0,
      i32_: 0,
      u32_: 0,
      i64_: 0,
      u64_: 0,
      f32_: 0.0,
      f64_: 0.0,
      v8: None,
      vf64: None,
    }
  }
}

impl Serialize for TypeAliases<'_> {
  fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
  where
    S: Serializer,
  {
    let mut s = serializer.serialize_struct("TypeAliases", 12)?;
      s.serialize_field("i8_", &self.i8_())?;
      s.serialize_field("u8_", &self.u8_())?;
      s.serialize_field("i16_", &self.i16_())?;
      s.serialize_field("u16_", &self.u16_())?;
      s.serialize_field("i32_", &self.i32_())?;
      s.serialize_field("u32_", &self.u32_())?;
      s.serialize_field("i64_", &self.i64_())?;
      s.serialize_field("u64_", &self.u64_())?;
      s.serialize_field("f32_", &self.f32_())?;
      s.serialize_field("f64_", &self.f64_())?;
      if let Some(f) = self.v8() {
        s.serialize_field("v8", &f)?;
      } else {
        s.skip_field("v8")?;
      }
      if let Some(f) = self.vf64() {
        s.serialize_field("vf64", &f)?;
      } else {
        s.skip_field("vf64")?;
      }
    s.end()
  }
}

pub struct TypeAliasesBuilder<'a: 'b, 'b, A: flatbuffers::Allocator + 'a> {
  fbb_: &'b mut flatbuffers::FlatBufferBuilder<'a, A>,
  start_: flatbuffers::WIPOffset<flatbuffers::TableUnfinishedWIPOffset>,
}
impl<'a: 'b, 'b, A: flatbuffers::Allocator + 'a> TypeAliasesBuilder<'a, 'b, A> {
  #[inline]
  pub fn add_i8_(&mut self, i8_: i8) {
    self.fbb_.push_slot::<i8>(TypeAliases::VT_I8_, i8_, 0);
  }
  #[inline]
  pub fn add_u8_(&mut self, u8_: u8) {
    self.fbb_.push_slot::<u8>(TypeAliases::VT_U8_, u8_, 0);
  }
  #[inline]
  pub fn add_i16_(&mut self, i16_: i16) {
    self.fbb_.push_slot::<i16>(TypeAliases::VT_I16_, i16_, 0);
  }
  #[inline]
  pub fn add_u16_(&mut self, u16_: u16) {
    self.fbb_.push_slot::<u16>(TypeAliases::VT_U16_, u16_, 0);
  }
  #[inline]
  pub fn add_i32_(&mut self, i32_: i32) {
    self.fbb_.push_slot::<i32>(TypeAliases::VT_I32_, i32_, 0);
  }
  #[inline]
  pub fn add_u32_(&mut self, u32_: u32) {
    self.fbb_.push_slot::<u32>(TypeAliases::VT_U32_, u32_, 0);
  }
  #[inline]
  pub fn add_i64_(&mut self, i64_: i64) {
    self.fbb_.push_slot::<i64>(TypeAliases::VT_I64_, i64_, 0);
  }
  #[inline]
  pub fn add_u64_(&mut self, u64_: u64) {
    self.fbb_.push_slot::<u64>(TypeAliases::VT_U64_, u64_, 0);
  }
  #[inline]
  pub fn add_f32_(&mut self, f32_: f32) {
    self.fbb_.push_slot::<f32>(TypeAliases::VT_F32_, f32_, 0.0);
  }
  #[inline]
  pub fn add_f64_(&mut self, f64_: f64) {
    self.fbb_.push_slot::<f64>(TypeAliases::VT_F64_, f64_, 0.0);
  }
  #[inline]
  pub fn add_v8(&mut self, v8: flatbuffers::WIPOffset<flatbuffers::Vector<'b , i8>>) {
    self.fbb_.push_slot_always::<flatbuffers::WIPOffset<_>>(TypeAliases::VT_V8, v8);
  }
  #[inline]
  pub fn add_vf64(&mut self, vf64: flatbuffers::WIPOffset<flatbuffers::Vector<'b , f64>>) {
    self.fbb_.push_slot_always::<flatbuffers::WIPOffset<_>>(TypeAliases::VT_VF64, vf64);
  }
  #[inline]
  pub fn new(_fbb: &'b mut flatbuffers::FlatBufferBuilder<'a, A>) -> TypeAliasesBuilder<'a, 'b, A> {
    let start = _fbb.start_table();
    TypeAliasesBuilder {
      fbb_: _fbb,
      start_: start,
    }
  }
  #[inline]
  pub fn finish(self) -> flatbuffers::WIPOffset<TypeAliases<'a>> {
    let o = self.fbb_.end_table(self.start_);
    flatbuffers::WIPOffset::new(o.value())
  }
}

impl core::fmt::Debug for TypeAliases<'_> {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    let mut ds = f.debug_struct("TypeAliases");
      ds.field("i8_", &self.i8_());
      ds.field("u8_", &self.u8_());
      ds.field("i16_", &self.i16_());
      ds.field("u16_", &self.u16_());
      ds.field("i32_", &self.i32_());
      ds.field("u32_", &self.u32_());
      ds.field("i64_", &self.i64_());
      ds.field("u64_", &self.u64_());
      ds.field("f32_", &self.f32_());
      ds.field("f64_", &self.f64_());
      ds.field("v8", &self.v8());
      ds.field("vf64", &self.vf64());
      ds.finish()
  }
}
#[non_exhaustive]
#[derive(Debug, Clone, PartialEq)]
pub struct TypeAliasesT {
  pub i8_: i8,
  pub u8_: u8,
  pub i16_: i16,
  pub u16_: u16,
  pub i32_: i32,
  pub u32_: u32,
  pub i64_: i64,
  pub u64_: u64,
  pub f32_: f32,
  pub f64_: f64,
  pub v8: Option<Vec<i8>>,
  pub vf64: Option<Vec<f64>>,
}
impl Default for TypeAliasesT {
  fn default() -> Self {
    Self {
      i8_: 0,
      u8_: 0,
      i16_: 0,
      u16_: 0,
      i32_: 0,
      u32_: 0,
      i64_: 0,
      u64_: 0,
      f32_: 0.0,
      f64_: 0.0,
      v8: None,
      vf64: None,
    }
  }
}
impl TypeAliasesT {
  pub fn pack<'b, A: flatbuffers::Allocator + 'b>(
    &self,
    _fbb: &mut flatbuffers::FlatBufferBuilder<'b, A>
  ) -> flatbuffers::WIPOffset<TypeAliases<'b>> {
    let i8_ = self.i8_;
    let u8_ = self.u8_;
    let i16_ = self.i16_;
    let u16_ = self.u16_;
    let i32_ = self.i32_;
    let u32_ = self.u32_;
    let i64_ = self.i64_;
    let u64_ = self.u64_;
    let f32_ = self.f32_;
    let f64_ = self.f64_;
    let v8 = self.v8.as_ref().map(|x|{
      _fbb.create_vector(x)
    });
    let vf64 = self.vf64.as_ref().map(|x|{
      _fbb.create_vector(x)
    });
    TypeAliases::create(_fbb, &TypeAliasesArgs{
      i8_,
      u8_,
      i16_,
      u16_,
      i32_,
      u32_,
      i64_,
      u64_,
      f32_,
      f64_,
      v8,
      vf64,
    })
  }
}
