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

extern crate flexbuffers;
extern crate serde;
#[macro_use]
extern crate serde_derive;
use serde::{Deserialize, Serialize};

#[derive(Debug, PartialEq, Serialize, Deserialize)]
enum Weapon {
    Fist,
    Equipment { name: String, damage: i32 },
}

#[derive(Debug, PartialEq, Serialize, Deserialize)]
struct Color(u8, u8, u8, u8);

#[derive(Debug, PartialEq, Serialize, Deserialize)]
struct Monster {
    hp: u32,
    mana: i32,
    enraged: bool,
    weapons: Vec<Weapon>,
    color: Color,
    position: [f64; 3],
    velocity: [f64; 3],
    coins: Vec<u32>,
}

fn main() {
    let monster = Monster {
        hp: 80,
        mana: 200,
        enraged: true,
        color: Color(255, 255, 255, 255),
        position: [0.0; 3],
        velocity: [1.0, 0.0, 0.0],
        weapons: vec![
            Weapon::Fist,
            Weapon::Equipment {
                name: "great axe".to_string(),
                damage: 15,
            },
            Weapon::Equipment {
                name: "hammer".to_string(),
                damage: 5,
            },
        ],
        coins: vec![5, 10, 25, 25, 25, 100],
    };
    let mut s = flexbuffers::FlexbufferSerializer::new();
    monster.serialize(&mut s).unwrap();

    // With serde, structs are stored in a vector (not a map) without field names. This example
    // is stored in fewer bytes than  the other monster example. However, this flexbuffer is less
    // descriptive, which could be a problem for your other applications that don't have access to
    // the underlying struct.
    //
    // Eventually, we plan on optionally storing the field and/or type names. For now, if language
    // neutral interoprability is a concern, we recommend building and reading the flexbuffer
    // without serde as per the other example.
    assert_eq!(s.view().len(), 105); // bytes

    let r = flexbuffers::Reader::get_root(s.view()).unwrap();
    let monster2 = Monster::deserialize(r).unwrap();

    assert_eq!(monster, monster2);
}

#[test]
fn test_main() {
    main()
}
