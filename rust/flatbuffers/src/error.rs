use std::error::Error;
use std::fmt;

#[derive(Copy, Clone, Ord, PartialOrd, Eq, PartialEq, Hash, Debug)]
pub enum ConvertError {
    InvalidValue,
}

impl fmt::Display for ConvertError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match *self {
            ConvertError::InvalidValue => write!(f, "invalid value in buffer"),
        }
    }
}

impl Error for ConvertError {}
