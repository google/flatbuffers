/*
 * Copyright 2018 Google Inc. All rights reserved.
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

//! # flatc
//!
//! Companion package to [`flatbuffers`] to generate code using `flatc` (the
//! FlatBuffers schema compiler) at compile time.
//!
//! This library is intended to be used as a `build-dependencies` entry in
//! `Cargo.toml`:
//!
//! ```toml
//! [dependencies]
//! flatbuffers = "0.5"
//!
//! [build-dependencies]
//! flatc = "0.1"
//! ```
//!
//! # Examples
//!
//! Use the `Build` structure to compile schema files:
//!
//! ```no_run
//! extern crate flatc;
//!
//! fn main() {
//!     flatc::Build::new()
//!         .schema("schema/foo.fbs")
//!         .compile();
//! }
//! ```
//!
//! Include the generated Rust code:
//!
//! ```ignore
//! mod foo_generated {
//!     include!(concat!(env!("OUT_DIR"), "/foo_generated.rs"));
//! }
//! ```
//!
//! [`flatbuffers`]: https://docs.rs/flatbuffers/latest

#![cfg_attr(feature = "cargo-clippy", allow(new_without_default_derive))]

use std::path::{Path, PathBuf};
use std::process::Command;
use std::{env, fmt, io};

/// The commit SHA of [google/flatbuffers.git] that the crate-supplied copy of
/// `flatc` was compiled with.
///
/// [google/flatbuffers.git]: https://github.com/google/flatbuffers
const FLATBUFFERS_COMMIT_SHA: &str = env!("FLATBUFFERS_COMMIT_SHA");

/// The location of the copy of the `flatc` executable compiled by this crate.
const FLATC_EXECUTABLE: &str = concat!(env!("OUT_DIR"), "/bin/flatc");

/// An internal error that occured.
struct Error {
    /// The kind of error.
    kind: ErrorKind,
    /// A helpful message.
    message: String,
}

impl Error {
    /// Create a new instance of an `Error`.
    fn new<S: AsRef<str>>(kind: ErrorKind, message: S) -> Self {
        Self {
            kind,
            message: message.as_ref().to_owned(),
        }
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self.kind {
            ErrorKind::IO => {
                writeln!(
                    f,
                    "An I/O error occured during FlatBuffers compilation: {}\n",
                    self.message,
                )?;
            }
            ErrorKind::FlatC => {
                writeln!(
                    f,
"A flatc error occurred during schema compilation with the crate-supplied executable\n",
                )?;
            }
        }

        writeln!(f, "Error message:      {}", self.message)?;
        writeln!(f, "flatc executable:   {}", FLATC_EXECUTABLE)?;
        writeln!(f, "flatbuffers commit: {}", FLATBUFFERS_COMMIT_SHA)?;

        Ok(())
    }
}

impl From<io::Error> for Error {
    fn from(error: io::Error) -> Self {
        Self::new(ErrorKind::IO, format!("{}", error))
    }
}

/// The type of errors that can occur when using `flatc`.
enum ErrorKind {
    /// An I/O error occured.
    IO,
    /// An error occurred while using the schema compiler.
    FlatC,
}

/// A builder for compilation of one or multiple FlatBuffers schemas.
///
/// This is the core type of `flatc`. For further documentation, see the
/// individual methods for this structure.
pub struct Build {
    /// The path that compiled schema files will be written into.
    output_path: Option<PathBuf>,
    /// The paths that will be tried in `include` statements in schemas.
    include_paths: Vec<PathBuf>,
    /// The schema files that will be compiled.
    schema: Vec<PathBuf>,
}

impl Build {
    /// Create a new instance of a `flatc` compilation with no configuration.
    ///
    /// This is finished with the [`compile`](struct.Build.html#method.compile)
    /// function.
    pub fn new() -> Self {
        Self {
            output_path: None,
            include_paths: Vec::new(),
            schema: Vec::new(),
        }
    }

    /// Add a schema file to the files which will be compiled.
    ///
    /// ## Shell Command
    ///
    /// ```text
    /// $ flatc -r foo.fbs bar.fbs baz.fbs
    /// ```
    ///
    /// ## Rust Equivalent
    ///
    /// ```no_run
    /// extern crate flatc;
    ///
    /// fn main() {
    ///     flatc::Build::new()
    ///         .schema("foo.fbs")
    ///         .schema("bar.fbs")
    ///         .schema("baz.fbs")
    ///         .compile();
    /// }
    /// ```
    pub fn schema<P: AsRef<Path>>(&mut self, file: P) -> &mut Self {
        self.schema.push(file.as_ref().to_path_buf());
        self
    }

    /// Add include directories to the flatc compilation.
    ///
    /// The order in which directories are added will be preserved.
    ///
    /// ## Shell Command
    ///
    /// ```text
    /// $ flatc -r -I dir1 -I dir2 foo.fbs
    /// ```
    ///
    /// ## Rust equivalent
    ///
    /// ```no_run
    /// extern crate flatc;
    ///
    /// fn main() {
    ///     flatc::Build::new()
    ///         .schema("foo.fbs")
    ///         .include("dir1")
    ///         .include("dir2")
    ///         .compile();
    /// }
    /// ```
    pub fn include<P: AsRef<Path>>(&mut self, dir: P) -> &mut Self {
        self.include_paths.push(dir.as_ref().to_path_buf());
        self
    }

    /// Run the FlatBuffer schema compiler, generating one file for each of the
    /// schemas added to the `Build` with the
    /// [`schema`](struct.Build.html#method.schema) method.
    ///
    /// Output filenames are postfixed with "*_generated*", and the extension is
    /// set to "*.rs*". These files are placed in `OUT_DIR`. They can be
    /// included in your crate using compile time macros.
    ///
    /// # Example
    ///
    /// For a compilation of `foo.fbs`, you would include it in your main crate
    /// with:
    ///
    /// ```ignore
    /// mod foo_generated {
    ///     include!(concat!(env!("OUT_DIR"), "/foo_generated.rs"));
    /// }
    /// ```
    ///
    /// # Panics
    ///
    /// This function will panic if it encounters any error. This will be
    /// emitted as a compilation error when compiling your crate.
    pub fn compile(&self) {
        if let Err(e) = self.try_compile() {
            panic!("\n\n{}\n\n", e);
        }
    }
}

impl Build {
    fn try_compile(&self) -> Result<(), Error> {
        let output_path = self.output_path.as_ref().map_or_else(
            || env::var("OUT_DIR").unwrap(),
            |path| format!("{}", path.display()),
        );

        let include_paths = self
            .include_paths
            .iter()
            .map(|path| format!("-I {}", path.display()));

        let fbs_files = self.schema.iter();

        Command::new(FLATC_EXECUTABLE)
            .current_dir(env::var("CARGO_MANIFEST_DIR").unwrap())
            .arg("-r")
            .args(&["-o", &output_path])
            .args(include_paths)
            .args(fbs_files)
            .output()
            .map_err(Error::from)
            .and_then(|output| {
                if output.status.success() {
                    Ok(())
                } else {
                    Err(Error::new(
                        ErrorKind::FlatC,
                        String::from_utf8(output.stdout).unwrap().trim(),
                    ))
                }
            })
    }
}

impl Build {
    #[doc(hidden)]
    pub fn output<P: AsRef<Path>>(&mut self, dir: P) -> &mut Self {
        self.output_path = Some(dir.as_ref().to_path_buf());
        self
    }
}
