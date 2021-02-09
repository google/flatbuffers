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

extern crate tempfile;

use std::env;

fn main() {
    flatc::Build::new()
      .include("../include_test/")
      .schema("../monster_test.fbs")
      .schema("../include_test/include_test1.fbs")
      .schema("../include_test/sub/include_test2.fbs")
      .compile();

    // `OUT_DIR` undefined during `cargo test`
    println!("cargo:rustc-env=OUT_DIR={}", env::var("OUT_DIR").unwrap());
}
