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

use flexbuffers::{BitWidth, Builder, Error, FlexBufferType, Reader};

fn main() {
    // Create a new Flexbuffer builder.
    let mut builder = Builder::new();

    // The root of the builder can be a singleton, map or vector.
    // Our monster will be represented with a map.
    let mut monster = builder.build_map();

    // Use `push` to add elements to a vector or map. Note that it up to the programmer to ensure
    // duplicate keys are avoided and the key has no null bytes.
    monster.push("hp", 80);
    monster.push("mana", 200);
    monster.push("enraged", true);

    // Let's give our monster some weapons. Use `nest_vector` to store a vector.
    let mut weapons = monster.nest_vector("weapons");

    // The first weapon is a fist which has no damage so we'll store it as a string.
    // Strings in Flexbuffers are utf8 encoded and are distinct from map Keys which are c strings.
    weapons.push("fist");

    // The monster also has an axe. We'll store it as a map to make it more interesting.
    let mut axe = weapons.nest_map();
    axe.push("name", "great axe");
    axe.push("damage", 15);
    // We're done adding to the axe.
    axe.end();

    // The monster also has a hammer.
    {
        let mut hammer = weapons.nest_map();
        hammer.push("name", "hammer");
        hammer.push("damage", 5);
        // Instead of calling `hammer.end()`, we can just drop the `hammer` for the same effect.
        // Nested vectors and maps are completed and serialized when their builders are dropped.
    }

    // We're done adding weapons.
    weapons.end();

    // Give the monster some money. Flexbuffers has typed vectors which are smaller than
    // heterogenous vectors. Elements of typed vectors can be pushed one at a time, as above, or
    // they can be passed as a slice. This will be stored as a `FlexBufferType::VectorInt`.
    monster.push("coins", &[5, 10, 25, 25, 25, 100]);

    // Flexbuffer has special types for fixed-length-typed-vectors (if the length is 3 or 4 and the
    // type is int, uint, or float). They're even more compact than typed vectors.
    // The monster's position and Velocity will be stored as `FlexbufferType::VectorFloat3`.
    monster.push("position", &[0.0; 3]);
    monster.push("velocity", &[1.0, 0.0, 0.0]);

    // Give the monster bright red skin. In rust, numbers are assumed integers until proven
    // otherwise. We annotate u8 to tell flexbuffers to store it as a FlexbufferType::VectorUInt4.
    monster.push("color", &[255, 0, 0, 255u8]);

    // End the map at the root of the builder. This finishes the Flexbuffer.
    monster.end();

    // Now the buffer is free to be reused. Let's see the final buffer.
    let data = builder.view();
    assert_eq!(data.len(), 201); // Bytes.

    // Let's read and verify the data.
    let root = Reader::get_root(data).unwrap();
    let read_monster = root.as_map();

    // What attributes does this monster have?
    let attrs: Vec<_> = read_monster.iter_keys().collect();
    assert_eq!(
        attrs,
        vec!["coins", "color", "enraged", "hp", "mana", "position", "velocity", "weapons"]
    );

    // index into a vector or map with the `idx` method.
    let read_hp = read_monster.idx("hp");
    let read_mana = read_monster.idx("mana");
    // If `idx` fails it will return a Null flexbuffer Reader

    // Use `as_T` to cast the data to your desired type.
    assert_eq!(read_hp.as_u8(), 80);
    assert_eq!(read_hp.as_f32(), 80.0);
    // If it fails it will return T::default().
    assert_eq!(read_hp.as_str(), ""); // Its not a string.
    assert_eq!(read_mana.as_i8(), 0); // 200 is not representable in i8.
    assert!(read_mana.as_vector().is_empty()); // Its not a vector.
    assert_eq!(read_monster.idx("foo").as_i32(), 0); // `foo` is not a monster attribute.

    // To examine how your data is stored, check the flexbuffer type and bitwidth.
    assert_eq!(read_hp.flexbuffer_type(), FlexBufferType::Int);
    assert_eq!(read_mana.flexbuffer_type(), FlexBufferType::Int);
    // Note that mana=200 is bigger than the maximum i8 so everything in the top layer of the
    // monster map is stored in 16 bits.
    assert_eq!(read_hp.bitwidth(), BitWidth::W16);
    assert_eq!(read_monster.idx("mana").bitwidth(), BitWidth::W16);

    // Use get_T functions if you want to ensure the type and bitwidth match what you expect.
    assert_eq!(read_hp.get_i16(), Ok(80));
    assert!(read_hp.get_u16().is_err());
    assert!(read_hp.get_vector().is_err());

    // Analogously, the `index` method is the safe version of `idx`.
    assert!(read_monster.index("hp").is_ok());
    assert_eq!(read_monster.index("foo").unwrap_err(), Error::KeyNotFound);

    // Maps can also be indexed by usize. They're stored by key so `coins` are the first element.
    let monster_coins = read_monster.idx(0);
    // Maps and Vectors can be iterated over.
    assert!(monster_coins
        .iter()
        .map(|r| r.as_u8())
        .eq(vec![5, 10, 25, 25, 25, 100].into_iter()));
    // For very speed sensitive applications, you can directly read the slice if all of the
    // following are true:
    //
    // *   The provided data buffer contains a valid flexbuffer.
    // *   You correctly specify the flexbuffer type and width.
    // *   The host machine is little endian.
    // *   The provided data buffer itself is aligned in memory to 8 bytes.
    //
    // Vec<u8> has alignment 1 so special care is needed to get your buffer's alignment to 8.
    #[cfg(target_endian = "little")]
    {
        if monster_coins.is_aligned() {
            assert_eq!(monster_coins.as_i8s(), &[5, 10, 25, 25, 25, 100]);
        }
    }

    // Build the answer to life the universe and everything. Reusing a builder resets it. The
    // reused internals won't need to reallocate leading to a potential 2x speedup.
    builder.build_singleton(42);

    // The monster is now no more.
    assert_eq!(builder.view().len(), 3); // Bytes.

    let the_answer = Reader::get_root(builder.view()).unwrap();
    assert_eq!(the_answer.as_i32(), 42);
}

#[test]
fn test_main() {
    main()
}
