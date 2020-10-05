use thiserror::Error;

#[derive(Error, Copy, Clone, Ord, PartialOrd, Eq, PartialEq, Hash, Debug)]
pub enum ConvertError {
    #[error("invalid value in buffer")]
    InvalidValue,
}
