//! Automatically generated, do not modify.

#[derive(PartialEq, Eq, Clone, Debug, Hash)]
#[repr(i8)]
pub enum Color {
    Red = 1,
    Green = 2,
    Blue = 8,
}

/// A List of all `Color` enum variants.
pub const COLOR_LIST: [Color;3] = [Color::Red,Color::Green,Color::Blue,];

impl Color {
    /// Returns a `str` representation of a `Color` enum.
    pub fn name(&self) -> &'static str {
        match *self {
            Color::Red => "Red",
            Color::Green => "Green",
            Color::Blue => "Blue",
        }
    }
}

impl From<i8> for Color {
    fn from(value: i8) -> Color {
        match value {
            1 => Color::Red,
            2 => Color::Green,
            8 => Color::Blue,
            _ => unreachable!("Unable to create a `Color` from value {} ", value),
        }
    }
}

