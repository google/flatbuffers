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
#[deprecated(since = "2.0.0", note = "Use associated constants instead. This will no longer be generated in 2021.")]
pub const ENUM_MIN_GAME_MESSAGE: u8 = 0;
#[deprecated(since = "2.0.0", note = "Use associated constants instead. This will no longer be generated in 2021.")]
pub const ENUM_MAX_GAME_MESSAGE: u8 = 3;
#[deprecated(since = "2.0.0", note = "Use associated constants instead. This will no longer be generated in 2021.")]
#[allow(non_camel_case_types)]
pub const ENUM_VALUES_GAME_MESSAGE: [GameMessage; 4] = [
  GameMessage::NONE,
  GameMessage::PlayerStatEvent,
  GameMessage::PlayerSpectate,
  GameMessage::PlayerInputChange,
];

#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Default)]
#[repr(transparent)]
pub struct GameMessage(pub u8);
#[allow(non_upper_case_globals)]
impl GameMessage {
  pub const NONE: Self = Self(0);
  pub const PlayerStatEvent: Self = Self(1);
  pub const PlayerSpectate: Self = Self(2);
  pub const PlayerInputChange: Self = Self(3);

  pub const ENUM_MIN: u8 = 0;
  pub const ENUM_MAX: u8 = 3;
  pub const ENUM_VALUES: &'static [Self] = &[
    Self::NONE,
    Self::PlayerStatEvent,
    Self::PlayerSpectate,
    Self::PlayerInputChange,
  ];
  /// Returns the variant's name or "" if unknown.
  pub fn variant_name(self) -> Option<&'static str> {
    match self {
      Self::NONE => Some("NONE"),
      Self::PlayerStatEvent => Some("PlayerStatEvent"),
      Self::PlayerSpectate => Some("PlayerSpectate"),
      Self::PlayerInputChange => Some("PlayerInputChange"),
      _ => None,
    }
  }
}
impl core::fmt::Debug for GameMessage {
  fn fmt(&self, f: &mut core::fmt::Formatter) -> core::fmt::Result {
    if let Some(name) = self.variant_name() {
      f.write_str(name)
    } else {
      f.write_fmt(format_args!("<UNKNOWN {:?}>", self.0))
    }
  }
}
impl<'a> flatbuffers::Follow<'a> for GameMessage {
  type Inner = Self;
  #[inline]
  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
    let b = unsafe { flatbuffers::read_scalar_at::<u8>(buf, loc) };
    Self(b)
  }
}

impl flatbuffers::Push for GameMessage {
    type Output = GameMessage;
    #[inline]
    unsafe fn push(&self, dst: &mut [u8], _written_len: usize) {
        unsafe { flatbuffers::emplace_scalar::<u8>(dst, self.0) };
    }
}

impl flatbuffers::EndianScalar for GameMessage {
  type Scalar = u8;
  #[inline]
  fn to_little_endian(self) -> u8 {
    self.0.to_le()
  }
  #[inline]
  #[allow(clippy::wrong_self_convention)]
  fn from_little_endian(v: u8) -> Self {
    let b = u8::from_le(v);
    Self(b)
  }
}

impl<'a> flatbuffers::Verifiable for GameMessage {
  #[inline]
  fn run_verifier(
    v: &mut flatbuffers::Verifier, pos: usize
  ) -> Result<(), flatbuffers::InvalidFlatbuffer> {
    use self::flatbuffers::Verifiable;
    u8::run_verifier(v, pos)
  }
}

impl flatbuffers::SimpleToVerifyInSlice for GameMessage {}
pub struct GameMessageUnionTableOffset {}

#[allow(clippy::upper_case_acronyms)]
#[non_exhaustive]
#[derive(Debug, Clone, PartialEq)]
pub enum GameMessageT {
  NONE,
  PlayerStatEvent(Box<PlayerStatEventT>),
  PlayerSpectate(Box<PlayerSpectateT>),
  PlayerInputChange(Box<PlayerInputChangeT>),
}
impl Default for GameMessageT {
  fn default() -> Self {
    Self::NONE
  }
}
impl GameMessageT {
  pub fn game_message_type(&self) -> GameMessage {
    match self {
      Self::NONE => GameMessage::NONE,
      Self::PlayerStatEvent(_) => GameMessage::PlayerStatEvent,
      Self::PlayerSpectate(_) => GameMessage::PlayerSpectate,
      Self::PlayerInputChange(_) => GameMessage::PlayerInputChange,
    }
  }
  pub fn pack<'b, A: flatbuffers::Allocator + 'b>(&self, fbb: &mut flatbuffers::FlatBufferBuilder<'b, A>) -> Option<flatbuffers::WIPOffset<flatbuffers::UnionWIPOffset>> {
    match self {
      Self::NONE => None,
      Self::PlayerStatEvent(v) => Some(v.pack(fbb).as_union_value()),
      Self::PlayerSpectate(v) => Some(v.pack(fbb).as_union_value()),
      Self::PlayerInputChange(v) => Some(v.pack(fbb).as_union_value()),
    }
  }
  /// If the union variant matches, return the owned PlayerStatEventT, setting the union to NONE.
  pub fn take_player_stat_event(&mut self) -> Option<Box<PlayerStatEventT>> {
    if let Self::PlayerStatEvent(_) = self {
      let v = core::mem::replace(self, Self::NONE);
      if let Self::PlayerStatEvent(w) = v {
        Some(w)
      } else {
        unreachable!()
      }
    } else {
      None
    }
  }
  /// If the union variant matches, return a reference to the PlayerStatEventT.
  pub fn as_player_stat_event(&self) -> Option<&PlayerStatEventT> {
    if let Self::PlayerStatEvent(v) = self { Some(v.as_ref()) } else { None }
  }
  /// If the union variant matches, return a mutable reference to the PlayerStatEventT.
  pub fn as_player_stat_event_mut(&mut self) -> Option<&mut PlayerStatEventT> {
    if let Self::PlayerStatEvent(v) = self { Some(v.as_mut()) } else { None }
  }
  /// If the union variant matches, return the owned PlayerSpectateT, setting the union to NONE.
  pub fn take_player_spectate(&mut self) -> Option<Box<PlayerSpectateT>> {
    if let Self::PlayerSpectate(_) = self {
      let v = core::mem::replace(self, Self::NONE);
      if let Self::PlayerSpectate(w) = v {
        Some(w)
      } else {
        unreachable!()
      }
    } else {
      None
    }
  }
  /// If the union variant matches, return a reference to the PlayerSpectateT.
  pub fn as_player_spectate(&self) -> Option<&PlayerSpectateT> {
    if let Self::PlayerSpectate(v) = self { Some(v.as_ref()) } else { None }
  }
  /// If the union variant matches, return a mutable reference to the PlayerSpectateT.
  pub fn as_player_spectate_mut(&mut self) -> Option<&mut PlayerSpectateT> {
    if let Self::PlayerSpectate(v) = self { Some(v.as_mut()) } else { None }
  }
  /// If the union variant matches, return the owned PlayerInputChangeT, setting the union to NONE.
  pub fn take_player_input_change(&mut self) -> Option<Box<PlayerInputChangeT>> {
    if let Self::PlayerInputChange(_) = self {
      let v = core::mem::replace(self, Self::NONE);
      if let Self::PlayerInputChange(w) = v {
        Some(w)
      } else {
        unreachable!()
      }
    } else {
      None
    }
  }
  /// If the union variant matches, return a reference to the PlayerInputChangeT.
  pub fn as_player_input_change(&self) -> Option<&PlayerInputChangeT> {
    if let Self::PlayerInputChange(v) = self { Some(v.as_ref()) } else { None }
  }
  /// If the union variant matches, return a mutable reference to the PlayerInputChangeT.
  pub fn as_player_input_change_mut(&mut self) -> Option<&mut PlayerInputChangeT> {
    if let Self::PlayerInputChange(v) = self { Some(v.as_mut()) } else { None }
  }
}
