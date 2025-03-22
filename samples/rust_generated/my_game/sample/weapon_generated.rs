// automatically generated by the FlatBuffers compiler, do not modify
// @generated
extern crate alloc;
use super::*;
pub enum WeaponOffset {}
#[derive(Copy, Clone, PartialEq)]

pub struct Weapon<'a> {
  pub _tab: ::flatbuffers::Table<'a>,
}

impl<'a> ::flatbuffers::Follow<'a> for Weapon<'a> {
  type Inner = Weapon<'a>;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    Self { _tab: ::flatbuffers::Table::new(buf, loc) }
  }
}

impl<'a> Weapon<'a> {
  pub const VT_NAME: ::flatbuffers::VOffsetT = 4;
  pub const VT_DAMAGE: ::flatbuffers::VOffsetT = 6;

  pub const fn get_fully_qualified_name() -> &'static str {
    "MyGame.Sample.Weapon"
  }

  #[inline]
  pub unsafe fn init_from_table(table: ::flatbuffers::Table<'a>) -> Self {
    Weapon { _tab: table }
  }
  #[allow(unused_mut)]
  pub fn create<'bldr: 'args, 'args: 'mut_bldr, 'mut_bldr, A: ::flatbuffers::Allocator + 'bldr>(
    _fbb: &'mut_bldr mut ::flatbuffers::FlatBufferBuilder<'bldr, A>,
    args: &'args WeaponArgs<'args>
  ) -> ::flatbuffers::WIPOffset<Weapon<'bldr>> {
    let mut builder = WeaponBuilder::new(_fbb);
    if let Some(x) = args.name { builder.add_name(x); }
    builder.add_damage(args.damage);
    builder.finish()
  }

  pub fn unpack(&self) -> WeaponT {
    let name = self.name().map(|x| {
      alloc::string::ToString::to_string(x)
    });
    let damage = self.damage();
    WeaponT {
      name,
      damage,
    }
  }

  #[inline]
  pub fn name(&self) -> Option<&'a str> {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<::flatbuffers::ForwardsUOffset<&str>>(Weapon::VT_NAME, None)}
  }
  #[inline]
  pub fn damage(&self) -> i16 {
    // Safety:
    // Created from valid Table for this object
    // which contains a valid value in this slot
    unsafe { self._tab.get::<i16>(Weapon::VT_DAMAGE, Some(0)).unwrap()}
  }
}

impl ::flatbuffers::Verifiable for Weapon<'_> {
  #[inline]
  fn run_verifier(
    v: &mut ::flatbuffers::Verifier, pos: usize
  ) -> Result<(), ::flatbuffers::InvalidFlatbuffer> {
    v.visit_table(pos)?
     .visit_field::<::flatbuffers::ForwardsUOffset<&str>>("name", Self::VT_NAME, false)?
     .visit_field::<i16>("damage", Self::VT_DAMAGE, false)?
     .finish();
    Ok(())
  }
}
pub struct WeaponArgs<'a> {
    pub name: Option<::flatbuffers::WIPOffset<&'a str>>,
    pub damage: i16,
}
impl<'a> Default for WeaponArgs<'a> {
  #[inline]
  fn default() -> Self {
    WeaponArgs {
      name: None,
      damage: 0,
    }
  }
}

pub struct WeaponBuilder<'a: 'b, 'b, A: ::flatbuffers::Allocator + 'a> {
  fbb_: &'b mut ::flatbuffers::FlatBufferBuilder<'a, A>,
  start_: ::flatbuffers::WIPOffset<::flatbuffers::TableUnfinishedWIPOffset>,
}
impl<'a: 'b, 'b, A: ::flatbuffers::Allocator + 'a> WeaponBuilder<'a, 'b, A> {
  #[inline]
  pub fn add_name(&mut self, name: ::flatbuffers::WIPOffset<&'b  str>) {
    self.fbb_.push_slot_always::<::flatbuffers::WIPOffset<_>>(Weapon::VT_NAME, name);
  }
  #[inline]
  pub fn add_damage(&mut self, damage: i16) {
    self.fbb_.push_slot::<i16>(Weapon::VT_DAMAGE, damage, 0);
  }
  #[inline]
  pub fn new(_fbb: &'b mut ::flatbuffers::FlatBufferBuilder<'a, A>) -> WeaponBuilder<'a, 'b, A> {
    let start = _fbb.start_table();
    WeaponBuilder {
      fbb_: _fbb,
      start_: start,
    }
  }
  #[inline]
  pub fn finish(self) -> ::flatbuffers::WIPOffset<Weapon<'a>> {
    let o = self.fbb_.end_table(self.start_);
    ::flatbuffers::WIPOffset::new(o.value())
  }
}

impl ::core::fmt::Debug for Weapon<'_> {
  fn fmt(&self, f: &mut ::core::fmt::Formatter<'_>) -> ::core::fmt::Result {
    let mut ds = f.debug_struct("Weapon");
      ds.field("name", &self.name());
      ds.field("damage", &self.damage());
      ds.finish()
  }
}
#[non_exhaustive]
#[derive(Debug, Clone, PartialEq)]
pub struct WeaponT {
  pub name: Option<alloc::string::String>,
  pub damage: i16,
}
impl Default for WeaponT {
  fn default() -> Self {
    Self {
      name: None,
      damage: 0,
    }
  }
}
impl WeaponT {
  pub fn pack<'b, A: ::flatbuffers::Allocator + 'b>(
    &self,
    _fbb: &mut ::flatbuffers::FlatBufferBuilder<'b, A>
  ) -> ::flatbuffers::WIPOffset<Weapon<'b>> {
    let name = self.name.as_ref().map(|x|{
      _fbb.create_string(x)
    });
    let damage = self.damage;
    Weapon::create(_fbb, &WeaponArgs{
      name,
      damage,
    })
  }
}
