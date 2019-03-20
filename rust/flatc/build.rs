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

extern crate cmake;

use std::{env, fs, path::Path, process::Command};

fn main() {
    let git_sha = if let Ok(output) = Command::new("git")
        .args(&["rev-parse", "--short=7", "HEAD"])
        .output()
    {
        String::from_utf8(output.stdout).expect("utf8 sha")
    } else {
        "unavailable".to_owned()
    };

    println!("cargo:rustc-env=FLATBUFFERS_COMMIT_SHA={}", git_sha.trim());

    if let Ok(output) = Command::new("cmake").arg("--version").output() {
        println!("\n\n{:?}\n\n", output);
    }

    let flatc_root = Path::new(&env::var("CARGO_MANIFEST_DIR").unwrap())
        .join("..")
        .join("..");

    // CI workaround
    let cmake_cache = flatc_root.join("CMakeCache.txt");
    if cmake_cache.exists() {
        fs::remove_file(cmake_cache).expect("remove cmake cache");
    }

    let mut cmake = ::cmake::Config::new(flatc_root);

    cmake.define("CMAKE_BUILD_TYPE", "Release");
    cmake.define("CMAKE_INSTALL_BINDIR", "bin");
    cmake.define("FLATBUFFERS_BUILD_TESTS", "OFF");
    cmake.define("FLATBUFFERS_BUILD_FLATLIB", "OFF");
    cmake.define("FLATBUFFERS_BUILD_FLATHASH", "OFF");
    #[cfg(windows)]
    cmake.cxxflag("/EHsc");

    #[cfg(windows)]
    let _ = if let Ok(cmake_vs_version) = env::var("CMAKE_VS_VERSION") {
        cmake.generator(format!("Visual Studio {}", cmake_vs_version));
    };

    cmake.build();
}
