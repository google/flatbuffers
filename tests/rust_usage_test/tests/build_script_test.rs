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

extern crate syn;

use std::{env, fs, path::Path};

#[test]
fn build_script_wrapper() {
    let output = Path::new(&env::var("OUT_DIR").unwrap()).join("monster_generated.rs");

    let src = fs::read_to_string(&output).expect("Failed to read generated code");
    let _ = ::syn::parse_file(&src).expect("Invalid output generated");
}
