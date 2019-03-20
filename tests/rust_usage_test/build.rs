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

use std::{env, fs};

use tempfile::TempDir;

const MONSTER_FBS: &str = r#"
// Example IDL file for our monster's schema.

namespace MyGame.Sample;

enum Color:byte { Red = 0, Green, Blue = 2 }

union Equipment { Weapon } // Optionally add more tables.

struct Vec3 {
  x:float;
  y:float;
  z:float;
}

table Monster {
  pos:Vec3;
  mana:short = 150;
  hp:short = 100;
  name:string;
  friendly:bool = false (deprecated);
  inventory:[ubyte];
  color:Color = Blue;
  weapons:[Weapon];
  equipped:Equipment;
}

table Weapon {
  name:string;
  damage:short;
}

root_type Monster;"#;

fn main() {
    let tempdir = TempDir::new().expect("Failed to create temporary directory");
    let input = tempdir.path().join("monster.fbs");
    fs::write(&input, MONSTER_FBS).expect("Failed to write monster.fbs");

    flatc::Build::new().schema(&input).compile();

    // `OUT_DIR` undefined during `cargo test`
    println!("cargo:rustc-env=OUT_DIR={}", env::var("OUT_DIR").unwrap());
}
