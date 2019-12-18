// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#![cfg_attr(test, feature(test))]
//! Flexbuffers is a high performance schemaless binary data format designed at Google.
//! It is complementary to the schema-ed format [Flatbuffers](http://docs.rs/flatbuffers/).
//! See [Flexbuffer Internals](https://google.github.io/flatbuffers/flatbuffers_internals.html)
//! for details on the binary format.
//!
//! * [See the examples for usage.](https://github.com/CasperN/flexbuffers/tree/master/examples)
//!
//! This rust implementation is in progress and breaking API changes may happen between minor
//! version changes.
// TODO(cneo): serde stuff are behind a default-on feature flag
//             Reader to Json is behind a default-off feature flag
//             Serializable structs are Pushable
//             Serde with maps - field names and type names.

extern crate byteorder;
#[cfg(test)]
#[macro_use]
extern crate serde_derive;
#[macro_use]
extern crate debug_stub_derive;
extern crate num_enum;
#[cfg(test)]
#[macro_use]
extern crate quickcheck;
extern crate serde;
#[cfg(test)]
extern crate test;

mod bitwidth;
mod builder;
mod flexbuffer_type;
mod reader;
pub use bitwidth::BitWidth;
pub use builder::{singleton, Builder, FlexbufferSerializer, MapBuilder, Pushable, VectorBuilder};
pub use flexbuffer_type::FlexBufferType;
pub use reader::{Error, MapReader, Reader, ReaderIterator, VectorReader};

mod private {
    pub trait Sealed {}
}

/// This struct, when pushed will be serialized as a `FlexBufferType::Blob`.
/// A `Blob` is a variable width `length` followed by that many bytes of data.
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub struct Blob<'a>(pub &'a [u8]);

/// This struct, when pushed, will be serialized as a `FlexBufferType::IndirectUInt`.
/// Unsigned integer stored by reference in the flexbuffer. This can reduce the
/// size of vectors and maps containing the `IndirectUInt`.
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub struct IndirectUInt(pub u64);

/// This struct, when pushed, will be serialized as a `FlexBufferType::IndirectInt`,
/// a signed integer stored by reference in the flexbuffer. This can reduce the
/// size of vectors and maps containing the `IndirectInt`.
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub struct IndirectInt(pub i64);

/// This struct, when pushed, will be serialized as a `FlexBufferType::IndirectFloat`,
/// a floating point stored by reference in the flexbuffer. This can reduce the
/// size of vectors and maps containing the `IndirectFloat`.
#[derive(Debug, Copy, Clone, PartialEq)]
pub struct IndirectFloat(pub f64);

#[cfg(test)]
mod tests {
    mod rwyw;
#[rustfmt::skip] // Manually formatting Arrays has better readability.
    mod binary_format;
    mod benches;
}
