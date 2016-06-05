//! Rust support for Flatbuffers.
//!
//! FlatBuffers is an efficient cross platform serialization library for games
//! and other memory constrained apps. It allows you to directly access
//! serialized data without unpacking/parsing it first, while still having great
//! forwards/backwards compatibility.
//! 
#![doc(html_logo_url="http://google.github.io/flatbuffers/fpl_logo_small.png")]

#![deny(unused_qualifications, missing_debug_implementations, trivial_casts,
        missing_copy_implementations, unused_import_braces, missing_docs)]

extern crate byteorder;

mod builder;
mod types;
mod table;
mod iter;

#[macro_use]
pub mod macros;

pub use self::builder::Builder;
pub use self::types::{UOffsetT, VOffsetT, SOffsetT};
pub use self::table::*;
pub use self::iter::*;


