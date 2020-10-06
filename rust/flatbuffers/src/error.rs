use std::fmt::{Debug, Display};

use thiserror::Error;

#[derive(Error, Copy, Clone, Ord, PartialOrd, Eq, PartialEq, Hash, Debug)]
pub enum ConvertError<T: Debug + Display> {
    #[error("unknown variant in buffer: {0}")]
    UnknownVariant(T),
}
