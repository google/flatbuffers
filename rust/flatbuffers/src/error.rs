use std::fmt::{Debug, Display};

use thiserror::Error;

#[derive(Error, Copy, Clone, Ord, PartialOrd, Eq, PartialEq, Hash, Debug)]
pub enum ConvertError<T: Debug + Display> {
    #[error("invalid value in buffer: {0}")]
    InvalidValue(T),
}
