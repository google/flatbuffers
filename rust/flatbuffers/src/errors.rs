/*
 * Copyright 2020 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/// Errors which occur when the message becomes too large.
///
/// This can both be because it is too large
/// for the a the buffer implementation or because
/// the serialize messages larger that maximum
/// flatbuffer message size.
#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct OutOfBufferSpace;

impl core::fmt::Display for OutOfBufferSpace {
    fn fmt(&self, formatter: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        write!(formatter, "Out of buffer space")
    }
}

#[cfg(feature = "std")]
impl std::error::Error for OutOfBufferSpace {
    fn description(&self) -> &str {
        "Out of buffer space"
    }
}

#[cfg(feature = "std")]
impl From<OutOfBufferSpace> for std::io::Error {
    fn from(value: OutOfBufferSpace) -> std::io::Error {
        std::io::Error::new(std::io::ErrorKind::Other, value)
    }
}

/// Errors which occur when trying decoding an enum, but
/// the value in the buffer does not match any known enum value.
#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct TryFromEnumError;

impl core::fmt::Display for TryFromEnumError {
    fn fmt(&self, formatter: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        write!(formatter, "Invalid enum representation")
    }
}

#[cfg(feature = "std")]
impl std::error::Error for TryFromEnumError {
    fn description(&self) -> &str {
        "Invalid enum representation"
    }
}

#[cfg(feature = "std")]
impl From<TryFromEnumError> for std::io::Error {
    fn from(value: TryFromEnumError) -> std::io::Error {
        std::io::Error::new(std::io::ErrorKind::InvalidData, value)
    }
}

/// Errors which occur when trying decode a flatbuffer,
/// but invalid data is encountered.
#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct InvalidFlatbuffer;

impl core::fmt::Display for InvalidFlatbuffer {
    fn fmt(&self, formatter: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        write!(formatter, "Invalid flatbuffer")
    }
}

#[cfg(feature = "std")]
impl std::error::Error for InvalidFlatbuffer {
    fn description(&self) -> &str {
        "Invalid flatbuffer"
    }
}

#[cfg(feature = "std")]
impl From<InvalidFlatbuffer> for std::io::Error {
    fn from(value: InvalidFlatbuffer) -> std::io::Error {
        std::io::Error::new(std::io::ErrorKind::InvalidData, value)
    }
}
