//! Automatically generated, do not modify.

use flatbuffers;
use super::*;

#[derive(PartialEq, Eq, Clone, Debug, Hash)]
#[repr(u8)]
pub enum Any {
    NONE = 0,
    Monster = 1,
    TestSimpleTableWithEnum = 2,
}

/// A List of all `Any` enum variants.
pub const ANY_LIST: [Any;3] = [Any::NONE,Any::Monster,Any::TestSimpleTableWithEnum,];

impl Any {
    /// Returns a `str` representation of a `Any` enum.
    pub fn name(&self) -> &'static str {
        match *self {
            Any::NONE => "NONE",
            Any::Monster => "Monster",
            Any::TestSimpleTableWithEnum => "TestSimpleTableWithEnum",
        }
    }
}

impl From<u8> for Any {
    fn from(value: u8) -> Any {
        match value {
            0 => Any::NONE,
            1 => Any::Monster,
            2 => Any::TestSimpleTableWithEnum,
            _ => unreachable!("Unable to create a `Any` from value {} ", value),
        }
    }
}

#[derive(Debug)]
pub enum AnyUnion<'a> {
    None,
    Monster(Monster<'a>),
    TestSimpleTableWithEnum(TestSimpleTableWithEnum<'a>),
}

impl<'a> AnyUnion<'a> {
    pub fn from_type(table: &'a flatbuffers::Table, t: Any, offset: usize) -> AnyUnion<'a> {
        match t {
            Any::NONE => {
                AnyUnion::None
            }
            Any::Monster => {
                AnyUnion::Monster(table.get_root(offset as u32).into())
            }
            Any::TestSimpleTableWithEnum => {
                AnyUnion::TestSimpleTableWithEnum(table.get_root(offset as u32).into())
            }
        }
    }
}

