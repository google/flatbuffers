/*
 *
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

#![no_std]

#[cfg(not(feature = "no_std"))]
extern crate std;
#[cfg(not(feature = "no_std"))]
use alloc::vec::Vec;

#[macro_use]
extern crate alloc;

use alloc::string::String;

#[cfg(feature = "no_std")]
#[global_allocator]
static ALLOCATOR: libc_alloc::LibcAlloc = libc_alloc::LibcAlloc;

#[macro_use]
#[cfg(not(miri))] // slow.
extern crate quickcheck;
extern crate flatbuffers;
extern crate flexbuffers;
extern crate rand;
extern crate serde;
#[macro_use]
extern crate serde_derive;
#[cfg(not(miri))] // slow.
#[macro_use]
extern crate quickcheck_derive;

mod flexbuffers_tests;
mod more_defaults_test;
mod optional_scalars_test;

#[allow(dead_code, unused_imports, clippy::all)]
#[path = "../../include_test1/mod.rs"]
pub mod include_test1_generated;

#[allow(dead_code, unused_imports, clippy::all)]
#[path = "../../include_test2/mod.rs"]
pub mod include_test2_generated;

#[allow(dead_code, unused_imports, clippy::all)]
#[path = "../../namespace_test/mod.rs"]
pub mod namespace_test_generated;

#[allow(dead_code, unused_imports, clippy::all)]
#[path = "../../monster_test/mod.rs"]
mod monster_test_generated;
pub use monster_test_generated::my_game;

#[allow(dead_code, unused_imports, clippy::all)]
#[path = "../../optional_scalars/mod.rs"]
mod optional_scalars_generated;

#[allow(dead_code, unused_imports, clippy::all)]
#[path = "../../arrays_test/mod.rs"]
mod arrays_test_generated;

// We use incorrect casing to test keywords.
#[allow(dead_code, unused_imports, non_camel_case_types, non_snake_case)]
#[path = "../../keyword_test/mod.rs"]
mod keyword_test_generated;

#[rustfmt::skip] // TODO: Use standard rust formatting and remove dead code.
#[allow(dead_code)]
mod flatbuffers_tests {
use super::*;

// Include simple random number generator to ensure results will be the
// same across platforms.
// http://en.wikipedia.org/wiki/Park%E2%80%93Miller_random_number_generator
struct LCG(u64);
impl LCG {
    fn new() -> Self {
        LCG { 0: 48271 }
    }
    fn next(&mut self) -> u64 {
        let old = self.0;
        self.0 = (self.0 * 279470273u64) % 4294967291u64;
        old
    }
    fn reset(&mut self) {
        self.0 = 48271
    }
}

// test helper macro to return an error if two expressions are not equal
macro_rules! check_eq {
    ($field_call:expr, $want:expr) => (
        if $field_call == $want {
            Ok(())
        } else {
            Err(stringify!($field_call))
        }
    )
}

#[test]
fn macro_check_eq() {
    assert!(check_eq!(1, 1).is_ok());
    assert!(check_eq!(1, 2).is_err());
}

// test helper macro to return an error if two expressions are equal
macro_rules! check_is_some {
    ($field_call:expr) => (
        if $field_call.is_some() {
            Ok(())
        } else {
            Err(stringify!($field_call))
        }
    )
}

#[test]
fn macro_check_is_some() {
    let some: Option<usize> = Some(0);
    let none: Option<usize> = None;
    assert!(check_is_some!(some).is_ok());
    assert!(check_is_some!(none).is_err());
}

#[test]
fn object_api_defaults() {
    use my_game::example::*;
    assert_eq!(
        Vec3T::default(), Vec3T {
        x: 0.0,
        y: 0.0,
        z: 0.0,
        test1: 0.0,
        test2: Color::empty(),
        test3: TestT {
            a: 0,
            b: 0
        }
    });
    let mut default_without_nan = MonsterT::default();
    default_without_nan.nan_default = 0.0;
    assert_eq!(
        default_without_nan,
        MonsterT {
            pos: None,
            hp: 100,
            mana: 150,
            name: String::new(),  // required string => default is empty string.
            color: Color::Blue,
            inventory: None,
            testarrayoftables: None,
            testarrayofstring: None,
            testarrayofstring2: None,
            testarrayofbools: None,
            testarrayofsortedstruct: None,
            enemy: None,
            test: AnyT::NONE,
            test4: None,
            test5: None,
            testnestedflatbuffer: None,
            testempty: None,
            testbool: false,
            testhashs32_fnv1: 0,
            testhashu32_fnv1: 0,
            testhashs64_fnv1: 0,
            testhashu64_fnv1: 0,
            testhashs32_fnv1a: 0,
            testhashu32_fnv1a: 0,
            testhashs64_fnv1a: 0,
            testhashu64_fnv1a: 0,
            testf: 3.14159,
            testf2: 3.0,
            testf3: 0.0,
            flex: None,
            vector_of_longs: None,
            vector_of_doubles: None,
            parent_namespace_test: None,
            vector_of_referrables: None,
            single_weak_reference: 0,
            vector_of_weak_references: None,
            vector_of_strong_referrables: None,
            co_owning_reference: 0,
            vector_of_co_owning_references: None,
            non_owning_reference: 0,
            vector_of_non_owning_references: None,
            any_unique: AnyUniqueAliasesT::NONE,
            any_ambiguous: AnyAmbiguousAliasesT::NONE,
            vector_of_enums: None,
            signed_enum: Race::None,
            testrequirednestedflatbuffer: None,  // despite the name, it is not required.
            scalar_key_sorted_tables: None,
            native_inline: None,
            long_enum_non_enum_default: Default::default(),
            long_enum_normal_default: LongEnum::LongOne,
            nan_default: 0.0,
            inf_default: f32::INFINITY,
            positive_inf_default: f32::INFINITY,
            infinity_default: f32::INFINITY,
            positive_infinity_default: f32::INFINITY,
            negative_inf_default: f32::NEG_INFINITY,
            negative_infinity_default: f32::NEG_INFINITY,
            double_inf_default: f64::INFINITY,
        }
    );
}

fn create_serialized_example_with_generated_code(builder: &mut flatbuffers::FlatBufferBuilder) {
    let mon = {
        let s0 = builder.create_string("test1");
        let s1 = builder.create_string("test2");
        let fred_name = builder.create_string("Fred");

        // can't inline creation of this Vec3 because we refer to it by reference, so it must live
        // long enough to be used by MonsterArgs.
        let pos = my_game::example::Vec3::new(1.0, 2.0, 3.0, 3.0, my_game::example::Color::Green, &my_game::example::Test::new(5i16, 6i8));

        let args = my_game::example::MonsterArgs{
            hp: 80,
            mana: 150,
            name: Some(builder.create_string("MyMonster")),
            pos: Some(&pos),
            test_type: my_game::example::Any::Monster,
            test: Some(my_game::example::Monster::create(builder, &my_game::example::MonsterArgs{
                name: Some(fred_name),
                ..Default::default()
            }).as_union_value()),
            inventory: Some(builder.create_vector(&[0u8, 1, 2, 3, 4])),
            test4: Some(builder.create_vector(&[my_game::example::Test::new(10, 20),
                                                       my_game::example::Test::new(30, 40)])),
            testarrayofstring: Some(builder.create_vector(&[s0, s1])),
            ..Default::default()
        };
        my_game::example::Monster::create(builder, &args)
    };
    my_game::example::finish_monster_buffer(builder, mon);
}

fn create_serialized_example_with_library_code(builder: &mut flatbuffers::FlatBufferBuilder) {
    let nested_union_mon = {
        let name = builder.create_string("Fred");
        let table_start = builder.start_table();
        builder.push_slot_always(my_game::example::Monster::VT_NAME, name);
        builder.end_table(table_start)
    };
    let pos = my_game::example::Vec3::new(1.0, 2.0, 3.0, 3.0, my_game::example::Color::Green, &my_game::example::Test::new(5i16, 6i8));
    let inv = builder.create_vector(&[0u8, 1, 2, 3, 4]);

    let test4 = builder.create_vector(&[my_game::example::Test::new(10, 20),
                                        my_game::example::Test::new(30, 40)][..]);

    let name = builder.create_string("MyMonster");

    let test1 = builder.create_string("test1");
    let test2 = builder.create_string("test2");

    let testarrayofstring = builder.create_vector(&[test1, test2]);

    // begin building

    let table_start = builder.start_table();
    builder.push_slot(my_game::example::Monster::VT_HP, 80i16, 100);
    builder.push_slot_always(my_game::example::Monster::VT_NAME, name);
    builder.push_slot_always(my_game::example::Monster::VT_POS, &pos);
    builder.push_slot(my_game::example::Monster::VT_TEST_TYPE, my_game::example::Any::Monster, my_game::example::Any::NONE);
    builder.push_slot_always(my_game::example::Monster::VT_TEST, nested_union_mon);
    builder.push_slot_always(my_game::example::Monster::VT_INVENTORY, inv);
    builder.push_slot_always(my_game::example::Monster::VT_TEST4, test4);
    builder.push_slot_always(my_game::example::Monster::VT_TESTARRAYOFSTRING, testarrayofstring);
    let root = builder.end_table(table_start);
    builder.finish(root, Some(my_game::example::MONSTER_IDENTIFIER));
}

fn serialized_example_is_accessible_and_correct(bytes: &[u8], identifier_required: bool, size_prefixed: bool) -> Result<(), &'static str> {

    if identifier_required {
        let correct = if size_prefixed {
            my_game::example::monster_size_prefixed_buffer_has_identifier(bytes)
        } else {
            my_game::example::monster_buffer_has_identifier(bytes)
        };
        check_eq!(correct, true)?;
    }

    let m = if size_prefixed {
        my_game::example::size_prefixed_root_as_monster(bytes).unwrap()
    } else {
        my_game::example::root_as_monster(bytes).unwrap()
    };

    check_eq!(m.hp(), 80)?;
    check_eq!(m.mana(), 150)?;
    check_eq!(m.name(), "MyMonster")?;

    let pos = m.pos().unwrap();
    check_eq!(pos.x(), 1.0f32)?;
    check_eq!(pos.y(), 2.0f32)?;
    check_eq!(pos.z(), 3.0f32)?;
    check_eq!(pos.test1(), 3.0f64)?;
    check_eq!(pos.test2(), my_game::example::Color::Green)?;

    let pos_test3 = pos.test3();
    check_eq!(pos_test3.a(), 5i16)?;
    check_eq!(pos_test3.b(), 6i8)?;

    check_eq!(m.test_type(), my_game::example::Any::Monster)?;
    check_is_some!(m.test())?;
    let table2 = m.test().unwrap();
    let monster2 = unsafe { my_game::example::Monster::init_from_table(table2) };

    check_eq!(monster2.name(), "Fred")?;

    check_is_some!(m.inventory())?;
    let inv = m.inventory().unwrap();
    check_eq!(inv.len(), 5)?;
    check_eq!(inv.iter().sum::<u8>(), 10u8)?;
    check_eq!(inv.iter().rev().sum::<u8>(), 10u8)?;

    check_is_some!(m.test4())?;
    let test4 = m.test4().unwrap();
    check_eq!(test4.len(), 2)?;
    check_eq!(test4.get(0).a() as i32 + test4.get(0).b() as i32 +
              test4.get(1).a() as i32 + test4.get(1).b() as i32, 100)?;

    check_is_some!(m.testarrayofstring())?;
    let testarrayofstring = m.testarrayofstring().unwrap();
    check_eq!(testarrayofstring.len(), 2)?;
    check_eq!(testarrayofstring.get(0), "test1")?;
    check_eq!(testarrayofstring.get(1), "test2")?;

    Ok(())
}

#[test]
fn test_object_api_reads_correctly() -> Result<(), &'static str>{
    let mut fbb = flatbuffers::FlatBufferBuilder::new();
    create_serialized_example_with_library_code(&mut fbb);

    let m = my_game::example::root_as_monster(fbb.finished_data()).unwrap().unpack();

    check_eq!(m.hp, 80)?;
    check_eq!(m.mana, 150)?;
    check_eq!(m.name, "MyMonster")?;

    let pos = m.pos.as_ref().unwrap();
    check_eq!(pos.x, 1.0f32)?;
    check_eq!(pos.y, 2.0f32)?;
    check_eq!(pos.z, 3.0f32)?;
    check_eq!(pos.test1, 3.0f64)?;
    check_eq!(pos.test2, my_game::example::Color::Green)?;

    let pos_test3 = &pos.test3;
    check_eq!(pos_test3.a, 5i16)?;
    check_eq!(pos_test3.b, 6i8)?;

    let monster2 = m.test.as_monster().unwrap();
    check_eq!(monster2.name, "Fred")?;

    let inv = m.inventory.as_ref().unwrap();
    check_eq!(inv.len(), 5)?;
    check_eq!(inv.iter().sum::<u8>(), 10u8)?;
    check_eq!(inv.iter().rev().sum::<u8>(), 10u8)?;

    let test4 = m.test4.as_ref().unwrap();
    check_eq!(test4.len(), 2)?;
    check_eq!(test4[0].a as i32 + test4[0].b as i32 +
              test4[1].a as i32 + test4[1].b as i32, 100)?;

    let testarrayofstring = m.testarrayofstring.as_ref().unwrap();
    check_eq!(testarrayofstring.len(), 2)?;
    check_eq!(testarrayofstring[0], "test1")?;
    check_eq!(testarrayofstring[1], "test2")?;
    Ok(())
}



// Disabled due to Windows CI limitations.
// #[test]
// fn builder_initializes_with_maximum_buffer_size() {
//     flatbuffers::FlatBufferBuilder::with_capacity(flatbuffers::FLATBUFFERS_MAX_BUFFER_SIZE);
// }

#[should_panic]
#[test]
fn builder_abort_with_greater_than_maximum_buffer_size() {
    flatbuffers::FlatBufferBuilder::with_capacity(flatbuffers::FLATBUFFERS_MAX_BUFFER_SIZE+1);
}

#[test]
fn builder_collapses_into_vec() {
    let mut b = flatbuffers::FlatBufferBuilder::new();
    create_serialized_example_with_generated_code(&mut b);
    let (backing_buf, head) = b.collapse();
    serialized_example_is_accessible_and_correct(&backing_buf[head..], true, false).unwrap();
}

#[test]
#[cfg(not(miri))]  // slow.
fn verifier_one_byte_errors_do_not_crash() {
    let mut b = flatbuffers::FlatBufferBuilder::new();
    create_serialized_example_with_library_code(&mut b);
    let mut badbuf = b.finished_data().to_vec();
    // If the verifier says a buffer is okay then using it won't cause a crash.
    // We use write_fmt since Debug visits all the fields - but there's no need to store anything.
    struct ForgetfulWriter;
    use core::fmt::Write;
    impl Write for ForgetfulWriter {
        fn write_str(&mut self, _: &str) -> Result<(), core::fmt::Error> {
            Ok(())
        }
    }
    let mut w = ForgetfulWriter;
    for d in 1..=255u8 {
        for i in 0..badbuf.len() {
            let orig = badbuf[i];
            badbuf[i] = badbuf[i].wrapping_add(d);
            if let Ok(m) = flatbuffers::root::<my_game::example::Monster>(&badbuf) {
                w.write_fmt(format_args!("{:?}", m)).unwrap()
            }
            badbuf[i] = orig;
        }
    }
}
#[test]
#[cfg(not(miri))]  // slow.
fn verifier_too_many_tables() {
    use my_game::example::*;
    let b = &mut flatbuffers::FlatBufferBuilder::new();
    let r = Referrable::create(b, &ReferrableArgs { id: 42 });
    let rs = b.create_vector(&vec![r; 500]);
    let name = Some(b.create_string("foo"));
    let m = Monster::create(b, &MonsterArgs {
        vector_of_referrables: Some(rs),
        name,  // required field.
        ..Default::default()
    });
    b.finish(m, None);

    let data = b.finished_data();
    let mut opts = flatbuffers::VerifierOptions::default();

    opts.max_tables = 500;
    let res = flatbuffers::root_with_opts::<Monster>(&opts, data);
    assert_eq!(res.unwrap_err(), flatbuffers::InvalidFlatbuffer::TooManyTables);

    opts.max_tables += 2;
    assert!(flatbuffers::root_with_opts::<Monster>(&opts, data).is_ok());
}
#[test]
#[cfg(not(miri))]  // slow.
fn verifier_apparent_size_too_large() {
    use my_game::example::*;
    let b = &mut flatbuffers::FlatBufferBuilder::new();
    let name = Some(b.create_string("foo"));
    // String amplification attack.
    let s = b.create_string(&(core::iter::repeat("X").take(1000).collect::<String>()));
    let testarrayofstring = Some(b.create_vector(&vec![s; 1000]));
    let m = Monster::create(b, &MonsterArgs {
        testarrayofstring,
        name,  // required field.
        ..Default::default()
    });
    b.finish(m, None);
    let data = b.finished_data();
    assert!(data.len() < 5200);  // est 4000 for the vector + 1000 for the string + 200 overhead.
    let mut opts = flatbuffers::VerifierOptions::default();
    opts.max_apparent_size = 1_000_000;

    let res = flatbuffers::root_with_opts::<Monster>(&opts, data);
    assert_eq!(res.unwrap_err(), flatbuffers::InvalidFlatbuffer::ApparentSizeTooLarge);

    opts.max_apparent_size += 20_000;
    assert!(flatbuffers::root_with_opts::<Monster>(&opts, data).is_ok());
}
#[test]
fn verifier_in_too_deep() {
    use my_game::example::*;
    let b = &mut flatbuffers::FlatBufferBuilder::new();
    let name = Some(b.create_string("foo"));
    let mut prev_monster = None;
    for _ in 0..11 {
        prev_monster = Some(Monster::create(b, &MonsterArgs {
            enemy: prev_monster,
            name,  // required field.
            ..Default::default()
        }));
    };
    b.finish(prev_monster.unwrap(), None);
    let mut opts = flatbuffers::VerifierOptions::default();
    opts.max_depth = 10;

    let data = b.finished_data();
    let res = flatbuffers::root_with_opts::<Monster>(&opts, data);
    assert_eq!(res.unwrap_err(), flatbuffers::InvalidFlatbuffer::DepthLimitReached);

    opts.max_depth += 1;
    assert!(flatbuffers::root_with_opts::<Monster>(&opts, data).is_ok());
}

#[cfg(test)]
mod generated_constants {
    extern crate flatbuffers;
    use super::my_game;

    #[test]
    fn monster_identifier() {
        assert_eq!("MONS", my_game::example::MONSTER_IDENTIFIER);
    }

    #[test]
    fn monster_file_extension() {
        assert_eq!("mon", my_game::example::MONSTER_EXTENSION);
    }

    #[test]
    fn enum_constants_are_public() {
        assert_eq!(-1, my_game::example::Race::ENUM_MIN);
        assert_eq!(2, my_game::example::Race::ENUM_MAX);
        assert_eq!(my_game::example::Race::ENUM_VALUES, [
            my_game::example::Race::None,
            my_game::example::Race::Human,
            my_game::example::Race::Dwarf,
            my_game::example::Race::Elf,
        ]);

        assert_eq!(0, my_game::example::Any::ENUM_MIN);
        assert_eq!(3, my_game::example::Any::ENUM_MAX);
        assert_eq!(my_game::example::Any::ENUM_VALUES, [
            my_game::example::Any::NONE,
            my_game::example::Any::Monster,
            my_game::example::Any::TestSimpleTableWithEnum,
            my_game::example::Any::MyGame_Example2_Monster,
        ]);

        assert_eq!(0, my_game::example::AnyUniqueAliases::ENUM_MIN);
        assert_eq!(3, my_game::example::AnyUniqueAliases::ENUM_MAX);
        assert_eq!(my_game::example::AnyUniqueAliases::ENUM_VALUES, [
            my_game::example::AnyUniqueAliases::NONE,
            my_game::example::AnyUniqueAliases::M,
            my_game::example::AnyUniqueAliases::TS,
            my_game::example::AnyUniqueAliases::M2,
        ]);

        assert_eq!(0, my_game::example::AnyAmbiguousAliases::ENUM_MIN);
        assert_eq!(3, my_game::example::AnyAmbiguousAliases::ENUM_MAX);
        assert_eq!(my_game::example::AnyAmbiguousAliases::ENUM_VALUES, [
            my_game::example::AnyAmbiguousAliases::NONE,
            my_game::example::AnyAmbiguousAliases::M1,
            my_game::example::AnyAmbiguousAliases::M2,
            my_game::example::AnyAmbiguousAliases::M3,
        ]);
    }
}

#[cfg(not(feature = "no_std"))]
#[cfg(test)]
mod lifetime_correctness {
    extern crate flatbuffers;

    use core::mem;

    use super::my_game;
    use super::load_file;

    #[test]
    fn table_get_field_from_static_buffer_1() {
        let buf = load_file("../monsterdata_test.mon").expect("missing monsterdata_test.mon");
        // create 'static slice
        let slice: &[u8] = &buf;
        let slice: &'static [u8] = unsafe { mem::transmute(slice) };
        // make sure values retrieved from the 'static buffer are themselves 'static
        let monster: my_game::example::Monster<'static> = my_game::example::root_as_monster(slice).unwrap();
        // this line should compile:
        let name: Option<&'static str> = unsafe { monster._tab.get::<flatbuffers::ForwardsUOffset<&str>>(my_game::example::Monster::VT_NAME, None) };
        assert_eq!(name, Some("MyMonster"));
    }

    #[test]
    fn table_get_field_from_static_buffer_2() {
        static DATA: [u8; 4] = [0, 0, 0, 0]; // some binary data
        let table: flatbuffers::Table<'static> = unsafe { flatbuffers::Table::new(&DATA, 0) };
        // this line should compile:
        unsafe { table.get::<&'static str>(0, None) };
    }

    #[test]
    fn table_object_self_lifetime_in_closure() {
        // This test is designed to ensure that lifetimes for temporary intermediate tables aren't inflated beyond where the need to be.
        let buf = load_file("../monsterdata_test.mon").expect("missing monsterdata_test.mon");
        let monster = my_game::example::root_as_monster(&buf).unwrap();
        let enemy: Option<my_game::example::Monster> = monster.enemy();
        // This line won't compile if "self" is required to live for the lifetime of buf above as the borrow disappears at the end of the closure.
        let enemy_of_my_enemy = enemy.map(|e| {
            // enemy (the Option) is consumed, and the enum's value is taken as a temporary (e) at the start of the closure
            let name = e.name();
            // ... the temporary dies here, so for this to compile name's lifetime must not be tied to the temporary
            name
            // If this test fails the error would be "`e` dropped here while still borrowed"
        });
        assert_eq!(enemy_of_my_enemy, Some("Fred"));
    }
}

#[cfg(test)]
mod roundtrip_generated_code {
    extern crate flatbuffers;

    use alloc::vec::Vec;

    use super::my_game;

    fn build_mon<'a, 'b>(builder: &'a mut flatbuffers::FlatBufferBuilder, args: &'b my_game::example::MonsterArgs) -> my_game::example::Monster<'a> {
        let mon = my_game::example::Monster::create(builder, &args);
        my_game::example::finish_monster_buffer(builder, mon);
        my_game::example::root_as_monster(builder.finished_data()).unwrap()
    }

    #[test]
    fn scalar_store() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{hp: 123, name: Some(name), ..Default::default()});
        assert_eq!(m.hp(), 123);
    }
    #[test]
    fn scalar_default() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{name: Some(name), ..Default::default()});
        assert_eq!(m.hp(), 100);
    }
    #[test]
    fn string_store() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let name = b.create_string("foobar");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{name: Some(name), ..Default::default()});
        assert_eq!(m.name(), "foobar");
    }
    #[test]
    fn struct_store() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{
            name: Some(name),
            pos: Some(&my_game::example::Vec3::new(1.0, 2.0, 3.0, 4.0,
                                                   my_game::example::Color::Green,
                                                   &my_game::example::Test::new(98, 99))),
            ..Default::default()
        });
        assert_eq!(m.pos(), Some(&my_game::example::Vec3::new(1.0, 2.0, 3.0, 4.0,
                                                              my_game::example::Color::Green,
                                                              &my_game::example::Test::new(98, 99))));
    }
    #[test]
    fn struct_default() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{name: Some(name), ..Default::default()});
        assert_eq!(m.pos(), None);
    }
    #[test]
    fn enum_store() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{name: Some(name), color: my_game::example::Color::Red, ..Default::default()});
        assert_eq!(m.color(), my_game::example::Color::Red);
    }
    #[test]
    fn enum_default() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{name: Some(name), ..Default::default()});
        assert_eq!(m.color(), my_game::example::Color::Blue);
    }
    #[test]
    fn union_store() {
        let b = &mut flatbuffers::FlatBufferBuilder::new();
        {
            let name_inner = b.create_string("foo");
            let name_outer = b.create_string("bar");

            let inner = my_game::example::Monster::create(b, &my_game::example::MonsterArgs{
                name: Some(name_inner),
                ..Default::default()
            });
            let outer = my_game::example::Monster::create(b, &my_game::example::MonsterArgs{
                name: Some(name_outer),
                test_type: my_game::example::Any::Monster,
                test: Some(inner.as_union_value()),
                ..Default::default()
            });
            my_game::example::finish_monster_buffer(b, outer);
        }

        let mon = my_game::example::root_as_monster(b.finished_data()).unwrap();
        assert_eq!(mon.name(), "bar");
        assert_eq!(mon.test_type(), my_game::example::Any::Monster);
        let name = unsafe { my_game::example::Monster::init_from_table(mon.test().unwrap()).name() };
        assert_eq!(name, "foo");
        assert_eq!(mon.test_as_monster().unwrap().name(), "foo");
        assert_eq!(mon.test_as_test_simple_table_with_enum(), None);
        assert_eq!(mon.test_as_my_game_example_2_monster(), None);
    }
    #[test]
    fn union_default() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{name: Some(name), ..Default::default()});
        assert_eq!(m.test_type(), my_game::example::Any::NONE);
        assert_eq!(m.test(), None);
    }
    #[test]
    fn table_full_namespace_store() {
        let b = &mut flatbuffers::FlatBufferBuilder::new();
        {
            let name_inner = b.create_string("foo");
            let name_outer = b.create_string("bar");

            let inner = my_game::example::Monster::create(b, &my_game::example::MonsterArgs{
                name: Some(name_inner),
                ..Default::default()
            });
            let outer = my_game::example::Monster::create(b, &my_game::example::MonsterArgs{
                name: Some(name_outer),
                enemy: Some(inner),
                ..Default::default()
            });
            my_game::example::finish_monster_buffer(b, outer);
        }

        let mon = my_game::example::root_as_monster(b.finished_data()).unwrap();
        assert_eq!(mon.name(), "bar");
        assert_eq!(mon.enemy().unwrap().name(), "foo");
    }
    #[test]
    fn table_full_namespace_default() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{name: Some(name), ..Default::default()});
        assert_eq!(m.enemy(), None);
    }
    #[test]
    fn table_store() {
        let b = &mut flatbuffers::FlatBufferBuilder::new();
        {
            let id_inner = b.create_string("foo");
            let name_outer = b.create_string("bar");

            let inner = my_game::example::Stat::create(b, &my_game::example::StatArgs{
                id: Some(id_inner),
                ..Default::default()
            });
            let outer = my_game::example::Monster::create(b, &my_game::example::MonsterArgs{
                name: Some(name_outer),
                testempty: Some(inner),
                ..Default::default()
            });
            my_game::example::finish_monster_buffer(b, outer);
        }

        let mon = my_game::example::root_as_monster(b.finished_data()).unwrap();
        assert_eq!(mon.name(), "bar");
        assert_eq!(mon.testempty().unwrap().id(), Some("foo"));
    }
    #[test]
    fn table_default() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{name: Some(name), ..Default::default()});
        assert_eq!(m.testempty(), None);
    }
    #[test]
    fn nested_flatbuffer_store() {
        let b0 = {
            let mut b0 = flatbuffers::FlatBufferBuilder::new();
            let args = my_game::example::MonsterArgs{
                hp: 123,
                name: Some(b0.create_string("foobar")),
                ..Default::default()
            };
            let mon = my_game::example::Monster::create(&mut b0, &args);
            my_game::example::finish_monster_buffer(&mut b0, mon);
            b0
        };

        let b1 = {
            let mut b1 = flatbuffers::FlatBufferBuilder::new();
            let args = my_game::example::MonsterArgs{
                testnestedflatbuffer: Some(b1.create_vector(b0.finished_data())),
                name: Some(b1.create_string("foo")),
                ..Default::default()
            };
            let mon = my_game::example::Monster::create(&mut b1, &args);
            my_game::example::finish_monster_buffer(&mut b1, mon);
            b1
        };

        let m = my_game::example::root_as_monster(b1.finished_data()).unwrap();

        assert!(m.testnestedflatbuffer().is_some());
        assert_eq!(m.testnestedflatbuffer().unwrap().bytes(), b0.finished_data());

        let m2_a = my_game::example::root_as_monster(m.testnestedflatbuffer().unwrap().bytes()).unwrap();
        assert_eq!(m2_a.hp(), 123);
        assert_eq!(m2_a.name(), "foobar");

        assert!(m.testnestedflatbuffer_nested_flatbuffer().is_some());
        let m2_b = m.testnestedflatbuffer_nested_flatbuffer().unwrap();

        assert_eq!(m2_b.hp(), 123);
        assert_eq!(m2_b.name(), "foobar");
    }
    #[test]
    fn nested_flatbuffer_default() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{name: Some(name), ..Default::default()});
        assert!(m.testnestedflatbuffer().is_none());
    }
    #[test]
    fn vector_of_string_store_helper_build() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let strings = &[b.create_string("foobar"), b.create_string("baz")];
        let v = b.create_vector(strings);
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{
            name: Some(name),
            testarrayofstring: Some(v), ..Default::default()});
        assert_eq!(m.testarrayofstring().unwrap().len(), 2);
        assert_eq!(m.testarrayofstring().unwrap().get(0), "foobar");
        assert_eq!(m.testarrayofstring().unwrap().get(1), "baz");

        let rust_vec_inst = m.testarrayofstring().unwrap();
        let rust_vec_iter_collect = rust_vec_inst.iter().collect::<Vec<_>>();
        assert_eq!(rust_vec_iter_collect.len(), 2);
        assert_eq!(rust_vec_iter_collect[0], "foobar");
        assert_eq!(rust_vec_iter_collect[1], "baz");

        let rust_vec_iter_rev_collect = rust_vec_inst.iter().rev().collect::<Vec<_>>();
        assert_eq!(rust_vec_iter_rev_collect.len(), 2);
        assert_eq!(rust_vec_iter_rev_collect[1], "foobar");
        assert_eq!(rust_vec_iter_rev_collect[0], "baz");

    }
    #[test]
    fn vector_of_string_store_manual_build() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let s0 = b.create_string("foobar");
        let s1 = b.create_string("baz");
        let v = b.create_vector(&[s0, s1]);
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{
            name: Some(name),
            testarrayofstring: Some(v), ..Default::default()});
        assert_eq!(m.testarrayofstring().unwrap().len(), 2);
        assert_eq!(m.testarrayofstring().unwrap().get(0), "foobar");
        assert_eq!(m.testarrayofstring().unwrap().get(1), "baz");

        let rust_vec_inst = m.testarrayofstring().unwrap();
        let rust_vec_iter_collect = rust_vec_inst.iter().collect::<Vec<_>>();
        assert_eq!(rust_vec_iter_collect.len(), 2);
        assert_eq!(rust_vec_iter_collect[0], "foobar");
        assert_eq!(rust_vec_iter_collect[1], "baz");

        let rust_vec_iter_rev_collect = rust_vec_inst.iter().rev().collect::<Vec<_>>();
        assert_eq!(rust_vec_iter_rev_collect.len(), 2);
        assert_eq!(rust_vec_iter_rev_collect[0], "baz");
        assert_eq!(rust_vec_iter_rev_collect[1], "foobar");
    }
    #[test]
    fn vector_of_ubyte_store() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let v = b.create_vector(&[123u8, 234u8][..]);
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{
            name: Some(name),
            inventory: Some(v), ..Default::default()
        });
        assert_eq!(m.inventory().unwrap().bytes(), &[123, 234]);
    }
    #[test]
    fn vector_of_bool_store() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let v = b.create_vector(&[false, true, false, true][..]);
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{
            name: Some(name),
            testarrayofbools: Some(v), ..Default::default()});

        let rust_vec_inst = m.testarrayofbools().unwrap();
        let rust_vec_iter_collect = rust_vec_inst.iter().collect::<Vec<_>>();
        assert_eq!(&rust_vec_iter_collect, &[false, true, false, true]);

        let rust_vec_iter_rev_collect = rust_vec_inst.iter().rev().collect::<Vec<_>>();
        assert_eq!(&rust_vec_iter_rev_collect, &[true, false, true, false]);
    }
    #[test]
    fn vector_of_f64_store() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let v = b.create_vector(&[3.14159265359f64][..]);
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{
            name: Some(name),
            vector_of_doubles: Some(v), ..Default::default()});
        assert_eq!(m.vector_of_doubles().unwrap().len(), 1);
        assert_eq!(m.vector_of_doubles().unwrap().get(0), 3.14159265359f64);

        let rust_vec_inst = m.vector_of_doubles().unwrap();
        let rust_vec_iter_collect = rust_vec_inst.iter().collect::<Vec<_>>();
        assert_eq!(rust_vec_iter_collect.len(), 1);
        assert_eq!(rust_vec_iter_collect[0], 3.14159265359f64);

        let rust_vec_iter_rev_collect = rust_vec_inst.iter().rev().collect::<Vec<_>>();
        assert_eq!(rust_vec_iter_rev_collect.len(), 1);
        assert_eq!(rust_vec_iter_rev_collect[0], 3.14159265359f64);
    }
    #[test]
    fn vector_of_struct_store() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let v = b.create_vector(&[my_game::example::Test::new(127, -128), my_game::example::Test::new(3, 123)][..]);
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{
            name: Some(name),
            test4: Some(v), ..Default::default()});

        let rust_vec_inst = m.test4().unwrap();
        let rust_vec_iter_collect = rust_vec_inst.iter().collect::<Vec<_>>();
        assert_eq!(rust_vec_iter_collect, &[&my_game::example::Test::new(127, -128), &my_game::example::Test::new(3, 123)][..]);

        let rust_vec_iter_rev_collect = rust_vec_inst.iter().rev().collect::<Vec<_>>();
        assert_eq!(rust_vec_iter_rev_collect, &[&my_game::example::Test::new(3, 123), &my_game::example::Test::new(127, -128)][..]);
    }
    #[test]
    fn vector_of_struct_store_with_type_inference() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let v = b.create_vector(&[my_game::example::Test::new(127, -128),
                                  my_game::example::Test::new(3, 123),
                                  my_game::example::Test::new(100, 101)]);
        let name = b.create_string("foo");
        let m = build_mon(&mut b, &my_game::example::MonsterArgs{
            name: Some(name),
            test4: Some(v), ..Default::default()});
        let vals: Vec<_> = m.test4().unwrap().iter().collect::<Vec<_>>();
        assert_eq!(vals, vec![&my_game::example::Test::new(127, -128), &my_game::example::Test::new(3, 123), &my_game::example::Test::new(100, 101)]);
    }
     #[test]
     fn vector_of_enums_store() {
         let mut b = flatbuffers::FlatBufferBuilder::new();
         let v = b.create_vector::<my_game::example::Color>(&[my_game::example::Color::Red, my_game::example::Color::Green][..]);
         let name = b.create_string("foo");
         let m = build_mon(&mut b, &my_game::example::MonsterArgs{
             name: Some(name),
             vector_of_enums: Some(v), ..Default::default()});
         assert_eq!(m.vector_of_enums().unwrap().len(), 2);
         assert_eq!(m.vector_of_enums().unwrap().get(0), my_game::example::Color::Red);
         assert_eq!(m.vector_of_enums().unwrap().get(1), my_game::example::Color::Green);
     }
    #[test]
    fn vector_of_table_store() {
        let b = &mut flatbuffers::FlatBufferBuilder::new();
        let t0 = {
            let name = b.create_string("foo");
            let args = my_game::example::MonsterArgs{hp: 55, name: Some(name), ..Default::default()};
            my_game::example::Monster::create(b, &args)
        };
        let t1 = {
            let name = b.create_string("bar");
            let args = my_game::example::MonsterArgs{name: Some(name), ..Default::default()};
            my_game::example::Monster::create(b, &args)
        };
        let v = b.create_vector(&[t0, t1][..]);
        let name = b.create_string("foo");
        let m = build_mon(b, &my_game::example::MonsterArgs{
            name: Some(name),
            testarrayoftables: Some(v), ..Default::default()});
        assert_eq!(m.testarrayoftables().unwrap().len(), 2);
        assert_eq!(m.testarrayoftables().unwrap().get(0).hp(), 55);
        assert_eq!(m.testarrayoftables().unwrap().get(0).name(), "foo");
        assert_eq!(m.testarrayoftables().unwrap().get(1).hp(), 100);
        assert_eq!(m.testarrayoftables().unwrap().get(1).name(), "bar");

        let rust_vec_inst = m.testarrayoftables().unwrap();
        let rust_vec_iter_collect = rust_vec_inst.iter().collect::<Vec<_>>();
        assert_eq!(rust_vec_iter_collect.len(), 2);
        assert_eq!(rust_vec_iter_collect[0].hp(), 55);
        assert_eq!(rust_vec_iter_collect[0].name(), "foo");
        assert_eq!(rust_vec_iter_collect[1].hp(), 100);
        assert_eq!(rust_vec_iter_collect[1].name(), "bar");

        let rust_vec_iter_rev_collect = rust_vec_inst.iter().rev().collect::<Vec<_>>();
        assert_eq!(rust_vec_iter_rev_collect.len(), 2);
        assert_eq!(rust_vec_iter_rev_collect[0].hp(), 100);
        assert_eq!(rust_vec_iter_rev_collect[0].name(), "bar");
        assert_eq!(rust_vec_iter_rev_collect[1].hp(), 55);
        assert_eq!(rust_vec_iter_rev_collect[1].name(), "foo");
    }
}

#[cfg(test)]
mod generated_code_alignment_and_padding {
    extern crate flatbuffers;
    use super::my_game;

    #[test]
    fn enum_color_is_1_byte() {
        assert_eq!(1, ::core::mem::size_of::<my_game::example::Color>());
    }

    #[test]
    fn union_any_is_1_byte() {
        assert_eq!(1, ::core::mem::size_of::<my_game::example::Any>());
    }

    #[test]
    fn union_any_is_aligned_to_1() {
        assert_eq!(1, ::core::mem::align_of::<my_game::example::Any>());
    }
    #[test]
    fn struct_test_is_4_bytes() {
        assert_eq!(4, ::core::mem::size_of::<my_game::example::Test>());
    }
    #[test]
    fn struct_vec3_is_32_bytes() {
        assert_eq!(32, ::core::mem::size_of::<my_game::example::Vec3>());
    }

    #[test]
    fn struct_vec3_is_written_with_correct_alignment_in_table() {
        let b = &mut flatbuffers::FlatBufferBuilder::new();
        {
            let name = b.create_string("foo");
            let mon = my_game::example::Monster::create(b, &my_game::example::MonsterArgs{
                name: Some(name),
                pos: Some(&my_game::example::Vec3::new(1.0, 2.0, 3.0, 4.0,
                                                       my_game::example::Color::Green,
                                                       &my_game::example::Test::new(98, 99))),
                                                       ..Default::default()});
            my_game::example::finish_monster_buffer(b, mon);
        }
        let buf = b.finished_data();
        let mon = my_game::example::root_as_monster(buf).unwrap();
        let vec3 = mon.pos().unwrap();

        let start_ptr = buf.as_ptr() as usize;
        let vec3_ptr = vec3 as *const my_game::example::Vec3 as usize;

        assert!(vec3_ptr > start_ptr);
        // Vec3 is aligned to 8 wrt the flatbuffer.
        assert_eq!((vec3_ptr - start_ptr) % 8, 0);
    }

    #[test]
    fn struct_ability_is_8_bytes() {
        assert_eq!(8, ::core::mem::size_of::<my_game::example::Ability>());
    }

    #[test]
    fn struct_ability_is_written_with_correct_alignment_in_table_vector() {
        let b = &mut flatbuffers::FlatBufferBuilder::new();
        {
            let name = b.create_string("foo");
            let v = b.create_vector(&[my_game::example::Ability::new(1, 2),
                                      my_game::example::Ability::new(3, 4),
                                      my_game::example::Ability::new(5, 6)]);
            let mon = my_game::example::Monster::create(b, &my_game::example::MonsterArgs{
                name: Some(name),
                testarrayofsortedstruct: Some(v),
                ..Default::default()});
            my_game::example::finish_monster_buffer(b, mon);
        }
        let buf = b.finished_data();
        let mon = my_game::example::root_as_monster(buf).unwrap();
        let abilities = mon.testarrayofsortedstruct().unwrap();

        let start_ptr = buf.as_ptr() as usize;
        for a in abilities.iter() {
            let a_ptr = a as *const my_game::example::Ability as usize;
            assert!(a_ptr > start_ptr);
            let aln = ::core::mem::align_of::<my_game::example::Ability>();
            assert_eq!((a_ptr - start_ptr) % aln, 0);
        }
        for a in abilities.iter().rev() {
            let a_ptr = a as *const my_game::example::Ability as usize;
            assert!(a_ptr > start_ptr);
            // Vec3 is aligned to 8 wrt the flatbuffer.
            assert_eq!((a_ptr - start_ptr) % 8, 0);
        }
    }
}

#[cfg(not(miri))]
quickcheck! {
  fn struct_of_structs(
    a_id: u32,
    a_distance: u32,
    b_a: i16,
    b_b: i8,
    c_id: u32,
    c_distance: u32
  ) -> bool {
    use my_game::example::*;
    let mut sos = StructOfStructs::default();
    let mut a = Ability::default();
    a.set_id(a_id);
    a.set_distance(a_distance);
    let mut b = Test::default();
    b.set_a(b_a);
    b.set_b(b_b);
    let mut c = Ability::default();
    c.set_id(c_id);
    c.set_distance(c_distance);
    sos.set_a(&a);
    sos.set_b(&b);
    sos.set_c(&c);

    sos.a().id() == a_id &&
    sos.a().distance() == a_distance &&
    sos.b().a() == b_a &&
    sos.b().b() == b_b &&
    sos.c().id() == c_id &&
    sos.c().distance() == c_distance
  }
}

#[cfg(not(miri))]  // slow.
#[cfg(test)]
mod roundtrip_vectors {

    #[cfg(test)]
    mod scalar {
        extern crate quickcheck;
        extern crate flatbuffers;

        use alloc::vec::Vec;

        const N: u64 = 20;

        fn prop<T>(xs: Vec<T>)
        where
            T: for<'a> flatbuffers::Follow<'a, Inner = T>
                + flatbuffers::EndianScalar
                + flatbuffers::Push
                + ::core::fmt::Debug,
        {
            use flatbuffers::Follow;

            let mut b = flatbuffers::FlatBufferBuilder::new();
            b.start_vector::<T>(xs.len());
            for i in (0..xs.len()).rev() {
                b.push::<T>(xs[i]);
            }
            let vecend = b.end_vector::<T>(xs.len());
            b.finish_minimal(vecend);

            let buf = b.finished_data();

            let got = unsafe { <flatbuffers::ForwardsUOffset<flatbuffers::Vector<T>>>::follow(&buf[..], 0) };
            let mut result_vec: Vec<T> = Vec::with_capacity(got.len());
            for i in 0..got.len() {
                result_vec.push(got.get(i));
            }
            assert_eq!(result_vec, xs);

            let rust_vec_iter = got.iter().collect::<Vec<T>>();
            assert_eq!(rust_vec_iter, xs);

            let mut rust_vec_rev_iter = got.iter().rev().collect::<Vec<T>>();
            rust_vec_rev_iter.reverse();
            assert_eq!(rust_vec_rev_iter, xs);
        }

        #[test]
        fn easy_u8() {
            prop::<u8>(vec![]);
            prop::<u8>(vec![1u8]);
            prop::<u8>(vec![1u8, 2u8]);
            prop::<u8>(vec![1u8, 2u8, 3u8]);
            prop::<u8>(vec![1u8, 2u8, 3u8, 4u8]);
        }

        #[test]
        fn fuzz_bool() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<bool> as fn(Vec<_>)); }
        #[test]
        fn fuzz_u8() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<u8> as fn(Vec<_>)); }
        #[test]
        fn fuzz_i8() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<i8> as fn(Vec<_>)); }
        #[test]
        fn fuzz_u16() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<u16> as fn(Vec<_>)); }
        #[test]
        fn fuzz_i16() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<i16> as fn(Vec<_>)); }
        #[test]
        fn fuzz_u32() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<u32> as fn(Vec<_>)); }
        #[test]
        fn fuzz_i32() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<i32> as fn(Vec<_>)); }
        #[test]
        fn fuzz_u64() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<u64> as fn(Vec<_>)); }
        #[test]
        fn fuzz_i64() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<i64> as fn(Vec<_>)); }
        #[test]
        fn fuzz_f32() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<f32> as fn(Vec<_>)); }
        #[test]
        fn fuzz_f64() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<f64> as fn(Vec<_>)); }
    }

    #[cfg(test)]
    mod string_manual_build {
        #[cfg(not(miri))]  // slow.
        extern crate quickcheck;
        extern crate flatbuffers;

        use alloc::string::String;
        use alloc::vec::Vec;

        fn prop(xs: Vec<String>) {
            use flatbuffers::Follow;

            let mut b = flatbuffers::FlatBufferBuilder::new();
            let mut offsets = Vec::new();
            for s in xs.iter().rev() {
                offsets.push(b.create_string(s.as_str()));
            }

            b.start_vector::<flatbuffers::WIPOffset<&str>>(xs.len());
            for &i in offsets.iter() {
                b.push(i);
            }
            let vecend = b.end_vector::<flatbuffers::WIPOffset<&str>>(xs.len());

            b.finish_minimal(vecend);

            let buf = b.finished_data();
            let got = unsafe { <flatbuffers::ForwardsUOffset<flatbuffers::Vector<flatbuffers::ForwardsUOffset<&str>>>>::follow(buf, 0) };

            assert_eq!(got.len(), xs.len());
            for i in 0..xs.len() {
                assert_eq!(got.get(i), &xs[i][..]);
            }
        }

        #[test]
        fn fuzz() {
            quickcheck::QuickCheck::new().max_tests(20).quickcheck(prop as fn(Vec<_>));
        }
    }

    #[cfg(test)]
    mod string_helper_build {
        #[cfg(not(miri))]  // slow.
        extern crate quickcheck;
        extern crate flatbuffers;

        use alloc::string::String;
        use alloc::vec::Vec;

        fn prop(input: Vec<String>) {
            use flatbuffers::Follow;

            let mut b = flatbuffers::FlatBufferBuilder::new();
            let xs: Vec<_> = input.iter().map(|s: &String| b.create_string(s)).collect();
            let vecend = b.create_vector(&xs);

            b.finish_minimal(vecend);

            let buf = b.finished_data();
            let got = unsafe { <flatbuffers::ForwardsUOffset<flatbuffers::Vector<flatbuffers::ForwardsUOffset<&str>>>>::follow(buf, 0) };

            assert_eq!(got.len(), xs.len());
            for (idx, s) in input.iter().enumerate() {
                assert_eq!(got.get(idx), s);
            }
        }

        #[test]
        fn fuzz() {
            quickcheck::QuickCheck::new().max_tests(100).quickcheck(prop as fn(Vec<_>));
        }
    }

    #[cfg(test)]
    mod ubyte {
        #[cfg(not(miri))]  // slow.
        extern crate quickcheck;
        extern crate flatbuffers;

        use alloc::vec::Vec;

        #[cfg(not(miri))]  // slow.
        #[test]
        fn fuzz_manual_build() {
            fn prop(vec: Vec<u8>) {
                let xs = &vec[..];

                let mut b1 = flatbuffers::FlatBufferBuilder::new();
                b1.start_vector::<u8>(xs.len());

                for i in (0..xs.len()).rev() {
                    b1.push(xs[i]);
                }
                b1.end_vector::<u8>(xs.len());

                let mut b2 = flatbuffers::FlatBufferBuilder::new();
                b2.create_vector(xs);
                assert_eq!(b1.unfinished_data(), b2.unfinished_data());
            }
            quickcheck::QuickCheck::new().max_tests(100).quickcheck(prop as fn(Vec<_>));
        }
    }
}

#[cfg(test)]
mod framing_format {
    extern crate flatbuffers;

    use super::my_game;

    #[test]
    fn test_size_prefixed_buffer() {
        // Create size prefixed buffer.
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let args = &my_game::example::MonsterArgs{
            mana: 200,
            hp: 300,
            name: Some(b.create_string("bob")),
            ..Default::default()
        };
        let mon = my_game::example::Monster::create(&mut b, &args);
        b.finish_size_prefixed(mon, None);

        // Access it.
        let buf = b.finished_data();
        let m = flatbuffers::size_prefixed_root::<my_game::example::Monster>(buf).unwrap();
        assert_eq!(m.mana(), 200);
        assert_eq!(m.hp(), 300);
        assert_eq!(m.name(), "bob");
    }
}

#[cfg(not(feature = "no_std"))]
#[cfg(test)]
mod roundtrip_table {
    use alloc::string::String;
    use alloc::vec::Vec;
    use std::collections::HashMap;

    extern crate flatbuffers;
    #[cfg(not(miri))]  // slow.
    extern crate quickcheck;

    use super::LCG;

    #[test]
    #[cfg(not(miri))]  // slow.
    fn table_of_mixed_scalars_fuzz() {
        // Values we're testing against: chosen to ensure no bits get chopped
        // off anywhere, and also be different from eachother.
        let bool_val: bool = true;
        let char_val: i8 = -127;  // 0x81
        let uchar_val: u8 = 0xFF;
        let short_val: i16 = -32222;  // 0x8222;
        let ushort_val: u16 = 0xFEEE;
        let int_val: i32 = unsafe { ::core::mem::transmute(0x83333333u32) };
        let uint_val: u32 = 0xFDDDDDDD;
        let long_val: i64 = unsafe { ::core::mem::transmute(0x8444444444444444u64) }; // TODO: byte literal?
        let ulong_val: u64 = 0xFCCCCCCCCCCCCCCCu64;
        let float_val: f32 = 3.14159;
        let double_val: f64 = 3.14159265359;

        let test_value_types_max: isize = 11;
        let max_fields_per_object: flatbuffers::VOffsetT = 100;
        let num_fuzz_objects: isize = 1000;  // The higher, the more thorough :)

        let mut builder = flatbuffers::FlatBufferBuilder::new();
        let mut lcg = LCG::new();

        let mut objects: Vec<flatbuffers::UOffsetT> = vec![0; num_fuzz_objects as usize];

        // Generate num_fuzz_objects random objects each consisting of
        // fields_per_object fields, each of a random type.
        for i in 0..(num_fuzz_objects as usize) {
            let fields_per_object = (lcg.next() % (max_fields_per_object as u64)) as flatbuffers::VOffsetT;
            let start = builder.start_table();

            for j in 0..fields_per_object {
                let choice = lcg.next() % (test_value_types_max as u64);

                let f = flatbuffers::field_index_to_field_offset(j);

                match choice {
                    0 => {builder.push_slot::<bool>(f, bool_val, false);}
                    1 => {builder.push_slot::<i8>(f, char_val, 0);}
                    2 => {builder.push_slot::<u8>(f, uchar_val, 0);}
                    3 => {builder.push_slot::<i16>(f, short_val, 0);}
                    4 => {builder.push_slot::<u16>(f, ushort_val, 0);}
                    5 => {builder.push_slot::<i32>(f, int_val, 0);}
                    6 => {builder.push_slot::<u32>(f, uint_val, 0);}
                    7 => {builder.push_slot::<i64>(f, long_val, 0);}
                    8 => {builder.push_slot::<u64>(f, ulong_val, 0);}
                    9 => {builder.push_slot::<f32>(f, float_val, 0.0);}
                    10 => {builder.push_slot::<f64>(f, double_val, 0.0);}
                    _ => { panic!("unknown choice: {}", choice); }
                }
            }
            objects[i] = builder.end_table(start).value();
        }

        // Do some bookkeeping to generate stats on fuzzes:
        let mut stats: HashMap<u64, u64> = HashMap::new();
        let mut values_generated: u64 = 0;

        // Embrace PRNG determinism:
        lcg.reset();

        // Test that all objects we generated are readable and return the
        // expected values. We generate random objects in the same order
        // so this is deterministic:
        for i in 0..(num_fuzz_objects as usize) {
            let table = {
                let buf = builder.unfinished_data();
                let loc = buf.len() as flatbuffers::UOffsetT - objects[i];
                unsafe { flatbuffers::Table::new(buf, loc as usize) }
            };

            let fields_per_object = (lcg.next() % (max_fields_per_object as u64)) as flatbuffers::VOffsetT;
            for j in 0..fields_per_object {
                let choice = lcg.next() % (test_value_types_max as u64);

                *stats.entry(choice).or_insert(0) += 1;
                values_generated += 1;

                let f = flatbuffers::field_index_to_field_offset(j);

                unsafe {
                    match choice {
                        0 => { assert_eq!(bool_val, table.get::<bool>(f, Some(false)).unwrap()); }
                        1 => { assert_eq!(char_val, table.get::<i8>(f, Some(0)).unwrap()); }
                        2 => { assert_eq!(uchar_val, table.get::<u8>(f, Some(0)).unwrap()); }
                        3 => { assert_eq!(short_val, table.get::<i16>(f, Some(0)).unwrap()); }
                        4 => { assert_eq!(ushort_val, table.get::<u16>(f, Some(0)).unwrap()); }
                        5 => { assert_eq!(int_val, table.get::<i32>(f, Some(0)).unwrap()); }
                        6 => { assert_eq!(uint_val, table.get::<u32>(f, Some(0)).unwrap()); }
                        7 => { assert_eq!(long_val, table.get::<i64>(f, Some(0)).unwrap()); }
                        8 => { assert_eq!(ulong_val, table.get::<u64>(f, Some(0)).unwrap()); }
                        9 => { assert_eq!(float_val, table.get::<f32>(f, Some(0.0)).unwrap()); }
                        10 => { assert_eq!(double_val, table.get::<f64>(f, Some(0.0)).unwrap()); }
                        _ => { panic!("unknown choice: {}", choice); }
                    }
                }
            }
        }

        // Assert that we tested all the fuzz cases enough:
        let min_tests_per_choice = 1000;
        assert!(values_generated > 0);
        assert!(min_tests_per_choice > 0);
        for i in 0..test_value_types_max as u64 {
            assert!(stats[&i] >= min_tests_per_choice, "inadequately-tested fuzz case: {}", i);
        }
    }

    #[test]
    #[cfg(not(miri))]  // slow.
    fn table_of_byte_strings_fuzz() {
        fn prop(vec: Vec<Vec<u8>>) {
            use flatbuffers::field_index_to_field_offset as fi2fo;
            use flatbuffers::Follow;

            let xs = &vec[..];

            // build
            let mut b = flatbuffers::FlatBufferBuilder::new();
            let str_offsets: Vec<flatbuffers::WIPOffset<_>> = xs.iter().map(|s| b.create_byte_string(&s[..])).collect();
            let table_start = b.start_table();

            for i in 0..xs.len() {
                b.push_slot_always(fi2fo(i as flatbuffers::VOffsetT), str_offsets[i]);
            }
            let root = b.end_table(table_start);
            b.finish_minimal(root);

            // use
            let buf = b.finished_data();
            let tab = unsafe { <flatbuffers::ForwardsUOffset<flatbuffers::Table>>::follow(buf, 0) };

            for i in 0..xs.len() {
                let v = unsafe { tab.get::<flatbuffers::ForwardsUOffset<flatbuffers::Vector<u8>>>(fi2fo(i as flatbuffers::VOffsetT), None) };
                assert!(v.is_some());
                let v2 = v.unwrap();
                assert_eq!(v2.bytes(), &xs[i]);
            }
        }
        prop(vec![vec![1,2,3]]);

        let n = 20;
        quickcheck::QuickCheck::new().max_tests(n).quickcheck(prop as fn(Vec<_>));
    }

    #[test]
    #[cfg(not(miri))]  // slow.
    fn fuzz_table_of_strings() {
        fn prop(vec: Vec<String>) {
            use flatbuffers::field_index_to_field_offset as fi2fo;
            use flatbuffers::Follow;

            let xs = &vec[..];

            // build
            let mut b = flatbuffers::FlatBufferBuilder::new();
            let str_offsets: Vec<flatbuffers::WIPOffset<_>> = xs.iter().map(|s| b.create_string(&s[..])).collect();
            let table_start = b.start_table();

            for i in 0..xs.len() {
                b.push_slot_always(fi2fo(i as flatbuffers::VOffsetT), str_offsets[i]);
            }
            let root = b.end_table(table_start);
            b.finish_minimal(root);

            // use
            let buf = b.finished_data();
            let tab = unsafe { <flatbuffers::ForwardsUOffset<flatbuffers::Table>>::follow(buf, 0) };

            for i in 0..xs.len() {
                let v = unsafe { tab.get::<flatbuffers::ForwardsUOffset<&str>>(fi2fo(i as flatbuffers::VOffsetT), None) };
                assert_eq!(v, Some(&xs[i][..]));
            }
        }
        let n = 20;
        quickcheck::QuickCheck::new().max_tests(n).quickcheck(prop as fn(Vec<String>));
    }

    #[cfg(not(miri))]  // slow.
    mod table_of_vectors_of_scalars {

        use alloc::vec::Vec;

        extern crate flatbuffers;
        #[cfg(not(miri))]  // slow.
        extern crate quickcheck;

        const N: u64 = 20;

        fn prop<T>(vecs: Vec<Vec<T>>)
        where
            T: for<'a> flatbuffers::Follow<'a, Inner = T>
                + flatbuffers::EndianScalar
                + flatbuffers::Push
                + ::core::fmt::Debug,
        {
            use flatbuffers::field_index_to_field_offset as fi2fo;
            use flatbuffers::Follow;

            // build
            let mut b = flatbuffers::FlatBufferBuilder::new();
            let mut offs = vec![];
            for vec in &vecs {
                b.start_vector::<T>(vec.len());

                let xs = &vec[..];
                for i in (0..xs.len()).rev() {
                    b.push::<T>(xs[i]);
                }
                let vecend = b.end_vector::<T>(xs.len());
                offs.push(vecend);
            }

            let table_start = b.start_table();

            for i in 0..vecs.len() {
                b.push_slot_always(fi2fo(i as flatbuffers::VOffsetT), offs[i]);
            }
            let root = b.end_table(table_start);
            b.finish_minimal(root);

            // use
            let buf = b.finished_data();
            let tab = unsafe { <flatbuffers::ForwardsUOffset<flatbuffers::Table>>::follow(buf, 0) };

            for i in 0..vecs.len() {
                let got = unsafe { tab.get::<flatbuffers::ForwardsUOffset<flatbuffers::Vector<T>>>(fi2fo(i as flatbuffers::VOffsetT), None) };
                assert!(got.is_some());
                let got2 = got.unwrap();
                let mut got3: Vec<T> = Vec::with_capacity(got2.len());
                for i in 0..got2.len() {
                    got3.push(got2.get(i));
                }
                assert_eq!(vecs[i], got3);
            }
        }

        #[test]
        fn fuzz_bool() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop as fn(Vec<Vec<bool>>)); }

        #[test]
        fn fuzz_u8() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop as fn(Vec<Vec<u8>>)); }
        #[test]
        fn fuzz_u16() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop as fn(Vec<Vec<u16>>)); }
        #[test]
        fn fuzz_u32() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop as fn(Vec<Vec<u32>>)); }
        #[test]
        fn fuzz_u64() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop as fn(Vec<Vec<u64>>)); }

        #[test]
        fn fuzz_i8() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop as fn(Vec<Vec<u8>>)); }
        #[test]
        fn fuzz_i16() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop as fn(Vec<Vec<u16>>)); }
        #[test]
        fn fuzz_i32() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop as fn(Vec<Vec<u32>>)); }
        #[test]
        fn fuzz_i64() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop as fn(Vec<Vec<u64>>)); }

        #[test]
        fn fuzz_f32() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop as fn(Vec<Vec<f32>>)); }
        #[test]
        fn fuzz_f64() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop as fn(Vec<Vec<f64>>)); }
    }
}

#[cfg(not(miri))]  // slow.
#[cfg(test)]
mod roundtrip_scalars {
    extern crate flatbuffers;
    #[cfg(not(miri))]  // slow.
    extern crate quickcheck;

    const N: u64 = 1000;

    fn prop<T: PartialEq + ::core::fmt::Debug + Copy + flatbuffers::EndianScalar>(x: T) {
        let mut buf = vec![0u8; ::core::mem::size_of::<T>()];
        let y = unsafe {
            flatbuffers::emplace_scalar(&mut buf[..], x);
            flatbuffers::read_scalar(&buf[..])
        };
        assert_eq!(x, y);
    }

    #[test]
    fn fuzz_bool() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<bool> as fn(_)); }
    #[test]
    fn fuzz_u8() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<u8> as fn(_)); }
    #[test]
    fn fuzz_i8() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<i8> as fn(_)); }

    #[test]
    fn fuzz_u16() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<u16> as fn(_)); }
    #[test]
    fn fuzz_i16() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<i16> as fn(_)); }

    #[test]
    fn fuzz_u32() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<u32> as fn(_)); }
    #[test]
    fn fuzz_i32() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<i32> as fn(_)); }

    #[test]
    fn fuzz_u64() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<u64> as fn(_)); }
    #[test]
    fn fuzz_i64() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<i64> as fn(_)); }

    #[test]
    fn fuzz_f32() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<f32> as fn(_)); }
    #[test]
    fn fuzz_f64() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop::<f64> as fn(_)); }
}

#[cfg(test)]
#[cfg(not(miri))]  // slow.
mod roundtrip_push_follow_scalars {
    extern crate flatbuffers;
    #[cfg(not(miri))]  // slow.
    extern crate quickcheck;

    use flatbuffers::Push;

    const N: u64 = 1000;

    // This uses a macro because lifetimes for a trait-bounded function get too
    // complicated.
    macro_rules! impl_prop {
        ($fn_name:ident, $ty:ident) => (
            fn $fn_name(x: $ty) {
                let mut buf = vec![0u8; ::core::mem::size_of::<$ty>()];
                unsafe { x.push(&mut buf[..], 0) };
                let fs: flatbuffers::FollowStart<$ty> = flatbuffers::FollowStart::new();
                assert_eq!(unsafe { fs.self_follow(&buf[..], 0) }, x);
            }
        )
    }

    impl_prop!(prop_bool, bool);
    impl_prop!(prop_u8, u8);
    impl_prop!(prop_i8, i8);
    impl_prop!(prop_u16, u16);
    impl_prop!(prop_i16, i16);
    impl_prop!(prop_u32, u32);
    impl_prop!(prop_i32, i32);
    impl_prop!(prop_u64, u64);
    impl_prop!(prop_i64, i64);
    impl_prop!(prop_f32, f32);
    impl_prop!(prop_f64, f64);

    #[test]
    fn fuzz_bool() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop_bool as fn(bool)); }
    #[test]
    fn fuzz_u8() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop_u8 as fn(u8)); }
    #[test]
    fn fuzz_i8() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop_i8 as fn(i8)); }
    #[test]
    fn fuzz_u16() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop_u16 as fn(u16)); }
    #[test]
    fn fuzz_i16() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop_i16 as fn(i16)); }
    #[test]
    fn fuzz_u32() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop_u32 as fn(u32)); }
    #[test]
    fn fuzz_i32() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop_i32 as fn(i32)); }
    #[test]
    fn fuzz_u64() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop_u64 as fn(u64)); }
    #[test]
    fn fuzz_i64() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop_i64 as fn(i64)); }
    #[test]
    fn fuzz_f32() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop_f32 as fn(f32)); }
    #[test]
    fn fuzz_f64() { quickcheck::QuickCheck::new().max_tests(N).quickcheck(prop_f64 as fn(f64)); }
}


#[cfg(test)]
mod write_and_read_examples {
    extern crate flatbuffers;

    use super::create_serialized_example_with_library_code;
    use super::create_serialized_example_with_generated_code;
    use super::serialized_example_is_accessible_and_correct;

    #[test]
    fn generated_code_creates_correct_example() {
        let b = &mut flatbuffers::FlatBufferBuilder::new();
        create_serialized_example_with_generated_code(b);
        let buf = b.finished_data();
        serialized_example_is_accessible_and_correct(&buf[..], true, false).unwrap();
    }

    #[test]
    fn generated_code_debug_prints_correctly() {
        let b = &mut flatbuffers::FlatBufferBuilder::new();
        create_serialized_example_with_generated_code(b);
        let buf = b.finished_data();
        serialized_example_is_accessible_and_correct(&buf, true, false).unwrap();
        let m = super::my_game::example::root_as_monster(buf).unwrap();
        assert_eq!(
            format!("{:.5?}", &m),
            "Monster { pos: Some(Vec3 { x: 1.00000, y: 2.00000, z: 3.00000, \
            test1: 3.00000, test2: Green, test3: Test { a: 5, b: 6 } }), \
            mana: 150, hp: 80, name: \"MyMonster\", \
            inventory: Some([0, 1, 2, 3, 4]), color: Blue, test_type: Monster, \
            test: Monster { pos: None, mana: 150, hp: 100, name: \"Fred\", \
            inventory: None, color: Blue, test_type: NONE, test: None, \
            test4: None, testarrayofstring: None, testarrayoftables: None, \
            enemy: None, testnestedflatbuffer: None, testempty: None, \
            testbool: false, testhashs32_fnv1: 0, testhashu32_fnv1: 0, \
            testhashs64_fnv1: 0, testhashu64_fnv1: 0, testhashs32_fnv1a: 0, \
            testhashu32_fnv1a: 0, testhashs64_fnv1a: 0, testhashu64_fnv1a: 0, \
            testarrayofbools: None, testf: 3.14159, testf2: 3.00000, testf3: 0.00000, \
            testarrayofstring2: None, testarrayofsortedstruct: None, flex: None, \
            test5: None, vector_of_longs: None, vector_of_doubles: None, \
            parent_namespace_test: None, vector_of_referrables: None, \
            single_weak_reference: 0, vector_of_weak_references: None, \
            vector_of_strong_referrables: None, co_owning_reference: 0, \
            vector_of_co_owning_references: None, non_owning_reference: 0, \
            vector_of_non_owning_references: None, any_unique_type: NONE, \
            any_unique: None, any_ambiguous_type: NONE, any_ambiguous: None, \
            vector_of_enums: None, signed_enum: None, \
            testrequirednestedflatbuffer: None, scalar_key_sorted_tables: None, \
            native_inline: None, long_enum_non_enum_default: (empty), \
            long_enum_normal_default: LongOne, nan_default: NaN, inf_default: \
            inf, positive_inf_default: inf, infinity_default: inf, \
            positive_infinity_default: inf, negative_inf_default: -inf, \
            negative_infinity_default: -inf, double_inf_default: inf }, \
            test4: Some([Test { a: 10, b: 20 }, Test { a: 30, b: 40 }]), \
            testarrayofstring: Some([\"test1\", \"test2\"]), \
            testarrayoftables: None, enemy: None, testnestedflatbuffer: None, \
            testempty: None, testbool: false, testhashs32_fnv1: 0, \
            testhashu32_fnv1: 0, testhashs64_fnv1: 0, testhashu64_fnv1: 0, \
            testhashs32_fnv1a: 0, testhashu32_fnv1a: 0, testhashs64_fnv1a: 0, \
            testhashu64_fnv1a: 0, testarrayofbools: None, testf: 3.14159, \
            testf2: 3.00000, testf3: 0.00000, testarrayofstring2: None, \
            testarrayofsortedstruct: None, flex: None, test5: None, \
            vector_of_longs: None, vector_of_doubles: None, \
            parent_namespace_test: None, vector_of_referrables: None, \
            single_weak_reference: 0, vector_of_weak_references: None, \
            vector_of_strong_referrables: None, co_owning_reference: 0, \
            vector_of_co_owning_references: None, non_owning_reference: 0, \
            vector_of_non_owning_references: None, any_unique_type: NONE, \
            any_unique: None, any_ambiguous_type: NONE, any_ambiguous: None, \
            vector_of_enums: None, signed_enum: None, \
            testrequirednestedflatbuffer: None, scalar_key_sorted_tables: None, \
            native_inline: None, long_enum_non_enum_default: (empty), \
            long_enum_normal_default: LongOne, nan_default: NaN, inf_default: \
            inf, positive_inf_default: inf, infinity_default: inf, \
            positive_infinity_default: inf, negative_inf_default: -inf, \
            negative_infinity_default: -inf, double_inf_default: inf }"
        );
    }

    #[test]
    #[cfg(not(miri))]  // slow.
    fn generated_code_creates_correct_example_repeatedly_with_reset() {
        let b = &mut flatbuffers::FlatBufferBuilder::new();
        for _ in 0..100 {
            create_serialized_example_with_generated_code(b);
            {
                let buf = b.finished_data();
                serialized_example_is_accessible_and_correct(&buf[..], true, false).unwrap();
            }
            b.reset();
        }
    }

    #[test]
    fn library_code_creates_correct_example() {
        let b = &mut flatbuffers::FlatBufferBuilder::new();
        create_serialized_example_with_library_code(b);
        let buf = b.finished_data();
        serialized_example_is_accessible_and_correct(&buf[..], true, false).unwrap();
    }

    #[test]
    #[cfg(not(miri))]  // slow.
    fn library_code_creates_correct_example_repeatedly_with_reset() {
        let b = &mut flatbuffers::FlatBufferBuilder::new();
        for _ in 0..100 {
            create_serialized_example_with_library_code(b);
            {
                let buf = b.finished_data();
                serialized_example_is_accessible_and_correct(&buf[..], true, false).unwrap();
            }
            b.reset();
        }
    }
}

#[cfg(not(feature = "no_std"))]
#[cfg(test)]
mod read_examples_from_other_language_ports {
    extern crate flatbuffers;

    use std::println;

    use super::load_file;
    use super::serialized_example_is_accessible_and_correct;

    #[test]
    fn gold_cpp_example_data_is_accessible_and_correct() {
        let buf = load_file("../monsterdata_test.mon").expect("missing monsterdata_test.mon");
        serialized_example_is_accessible_and_correct(&buf[..], true, false).unwrap();
    }
    #[test]
    fn java_wire_example_data_is_accessible_and_correct() {
        let buf = load_file("../monsterdata_java_wire.mon");
        if buf.is_err() {
            println!("skipping java wire test because it is not present");
            return;
        }
        let buf = buf.unwrap();
        serialized_example_is_accessible_and_correct(&buf[..], true, false).unwrap();
    }
    #[test]
    fn java_wire_size_prefixed_example_data_is_accessible_and_correct() {
        let buf = load_file("../monsterdata_java_wire_sp.mon");
        if buf.is_err() {
            println!("skipping java wire test because it is not present");
            return;
        }
        let buf = buf.unwrap();
        serialized_example_is_accessible_and_correct(&buf[..], true, true).unwrap();
    }
}

#[cfg(test)]
mod generated_code_asserts {
    extern crate flatbuffers;

    use super::my_game;

    #[test]
    #[should_panic]
    fn monster_builder_fails_when_name_is_missing() {
        let b = &mut flatbuffers::FlatBufferBuilder::new();
        my_game::example::Monster::create(b, &my_game::example::MonsterArgs{..Default::default()});
    }
}

#[cfg(test)]
mod generated_key_comparisons {
    extern crate flatbuffers;

    use super::my_game;

    #[test]
    fn struct_ability_key_compare_less_than() {
        let a = my_game::example::Ability::new(1, 2);
        let b = my_game::example::Ability::new(2, 1);
        let c = my_game::example::Ability::new(3, 3);

        assert_eq!(a.key_compare_less_than(&a), false);
        assert_eq!(b.key_compare_less_than(&b), false);
        assert_eq!(c.key_compare_less_than(&c), false);

        assert_eq!(a.key_compare_less_than(&b), true);
        assert_eq!(a.key_compare_less_than(&c), true);

        assert_eq!(b.key_compare_less_than(&a), false);
        assert_eq!(b.key_compare_less_than(&c), true);

        assert_eq!(c.key_compare_less_than(&a), false);
        assert_eq!(c.key_compare_less_than(&b), false);
    }

    #[test]
    fn struct_key_compare_with_value() {
        let a = my_game::example::Ability::new(1, 2);

        assert_eq!(a.key_compare_with_value(0), ::core::cmp::Ordering::Greater);
        assert_eq!(a.key_compare_with_value(1), ::core::cmp::Ordering::Equal);
        assert_eq!(a.key_compare_with_value(2), ::core::cmp::Ordering::Less);
    }

    #[test]
    fn struct_key_compare_less_than() {
        let a = my_game::example::Ability::new(1, 2);
        let b = my_game::example::Ability::new(2, 1);
        let c = my_game::example::Ability::new(3, 3);

        assert_eq!(a.key_compare_less_than(&a), false);
        assert_eq!(b.key_compare_less_than(&b), false);
        assert_eq!(c.key_compare_less_than(&c), false);

        assert_eq!(a.key_compare_less_than(&b), true);
        assert_eq!(a.key_compare_less_than(&c), true);

        assert_eq!(b.key_compare_less_than(&a), false);
        assert_eq!(b.key_compare_less_than(&c), true);

        assert_eq!(c.key_compare_less_than(&a), false);
        assert_eq!(c.key_compare_less_than(&b), false);
    }

    #[test]
    fn table_key_compare_with_value() {
        // setup
        let builder = &mut flatbuffers::FlatBufferBuilder::new();
        super::create_serialized_example_with_library_code(builder);
        let buf = builder.finished_data();
        let a = my_game::example::root_as_monster(buf).unwrap();

        // preconditions
        assert_eq!(a.name(), "MyMonster");

        assert_eq!(a.key_compare_with_value("AAA"), ::core::cmp::Ordering::Greater);
        assert_eq!(a.key_compare_with_value("MyMonster"), ::core::cmp::Ordering::Equal);
        assert_eq!(a.key_compare_with_value("ZZZ"), ::core::cmp::Ordering::Less);
    }

    #[test]
    fn table_key_compare_less_than() {
        // setup
        let builder = &mut flatbuffers::FlatBufferBuilder::new();
        super::create_serialized_example_with_library_code(builder);
        let buf = builder.finished_data();
        let a = my_game::example::root_as_monster(buf).unwrap();
        let b = a.test_as_monster().unwrap();

        // preconditions
        assert_eq!(a.name(), "MyMonster");
        assert_eq!(b.name(), "Fred");

        assert_eq!(a.key_compare_less_than(&a), false);
        assert_eq!(a.key_compare_less_than(&b), false);

        assert_eq!(b.key_compare_less_than(&a), true);
        assert_eq!(b.key_compare_less_than(&b), false);
    }
}

#[cfg(test)]
mod included_schema_generated_code {

    #[test]
    #[allow(unused_imports)]
    fn namespace_test_mod_is_importable() {
        use super::namespace_test_generated::{
            namespace_a,
            namespace_a::namespace_b,
            namespace_c,
        };

    }
}

#[cfg(test)]
mod builder_asserts {
    extern crate flatbuffers;

    #[test]
    #[should_panic]
    fn end_table_should_panic_when_not_in_table() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.end_table(flatbuffers::WIPOffset::new(0));
    }

    #[test]
    #[should_panic]
    fn create_string_should_panic_when_in_table() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.start_table();
        b.create_string("foo");
    }

    #[test]
    #[should_panic]
    fn create_byte_string_should_panic_when_in_table() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.start_table();
        b.create_byte_string(b"foo");
    }

    #[test]
    #[should_panic]
    fn push_struct_slot_should_panic_when_not_in_table() {
        #[derive(Copy, Clone, Debug, PartialEq)]
        #[repr(C, packed)]
        struct foo { }
        impl<'b> flatbuffers::Push for &'b foo {
            type Output = foo;
            unsafe fn push<'a>(&'a self, _dst: &'a mut [u8], _written_len: usize) { }
        }
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.push_slot_always(0, &foo{});
    }

    #[test]
    #[should_panic]
    fn finished_bytes_should_panic_when_table_is_not_finished() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.start_table();
        b.finished_data();
    }

    #[test]
    #[should_panic]
    fn required_panics_when_field_not_set() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let start = b.start_table();
        let o = b.end_table(start);
        b.required(o, 4 /* byte offset to first field */, "test field");
    }
}

#[cfg(test)]
mod follow_impls {
    extern crate flatbuffers;
    use flatbuffers::Follow;
    use flatbuffers::field_index_to_field_offset as fi2fo;

    use alloc::vec::Vec;

    // Define a test struct to use in a few tests. This replicates the work that the code generator
    // would normally do when defining a FlatBuffer struct. For reference, compare the following
    // `FooStruct` code with the code generated for the `Vec3` struct in
    // `../../monster_test/mod.rs`.
    use flatbuffers::EndianScalar;
    #[derive(Copy, Clone, Debug, PartialEq)]
    #[repr(C, packed)]
    struct FooStruct {
        a: i8,
        b: u8,
        c: i16,
    }
    impl FooStruct {
        fn new(_a: i8, _b: u8, _c: i16) -> Self {
            FooStruct {
                a: _a.to_little_endian(),
                b: _b.to_little_endian(),
                c: _c.to_little_endian(),
            }
        }
    }
    impl<'a> flatbuffers::Follow<'a> for FooStruct {
        type Inner = &'a FooStruct;
        #[inline(always)]
        unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
            <&'a FooStruct>::follow(buf, loc)
        }
    }
    impl<'a> flatbuffers::Follow<'a> for &'a FooStruct {
        type Inner = &'a FooStruct;
        #[inline(always)]
        unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
            flatbuffers::follow_cast_ref::<FooStruct>(buf, loc)
        }
    }

    #[test]
    fn to_u8() {
        let vec: Vec<u8> = vec![255, 3];
        let fs: flatbuffers::FollowStart<u8> = flatbuffers::FollowStart::new();
        assert_eq!(unsafe { fs.self_follow(&vec[..], 1) }, 3);
    }

    #[test]
    fn to_u16() {
        let vec: Vec<u8> = vec![255, 255, 3, 4];
        let fs: flatbuffers::FollowStart<u16> = flatbuffers::FollowStart::new();
        assert_eq!(unsafe { fs.self_follow(&vec[..], 2) }, 1027);
    }

    #[test]
    fn to_f32() {
        let vec: Vec<u8> = vec![255, 255, 255, 255, /* start of value */ 208, 15, 73, 64];
        let fs: flatbuffers::FollowStart<f32> = flatbuffers::FollowStart::new();
        assert_eq!(unsafe { fs.self_follow(&vec[..], 4) }, 3.14159);
    }

    #[test]
    fn to_string() {
        let vec: Vec<u8> = vec![255,255,255,255, 3, 0, 0, 0, 'f' as u8, 'o' as u8, 'o' as u8, 0];
        let off: flatbuffers::FollowStart<&str> = flatbuffers::FollowStart::new();
        assert_eq!(unsafe { off.self_follow(&vec[..], 4) }, "foo");
    }

    #[test]
    fn to_byte_slice() {
        let vec: Vec<u8> = vec![255, 255, 255, 255, 4, 0, 0, 0, 1, 2, 3, 4];
        let off: flatbuffers::FollowStart<flatbuffers::Vector<u8>> = flatbuffers::FollowStart::new();
        assert_eq!(unsafe { off.self_follow(&vec[..], 4).bytes() }, &[1, 2, 3, 4][..]);
    }

    #[test]
    fn to_vector_of_u16() {
        let vec: Vec<u8> = vec![255, 255, 255, 255, 2, 0, 0, 0, 1, 2, 3, 4];
        let off: flatbuffers::FollowStart<flatbuffers::Vector<u16>> = flatbuffers::FollowStart::new();
        assert_eq!(unsafe { off.self_follow(&vec[..], 4).len() }, 2);
        assert_eq!(unsafe { off.self_follow(&vec[..], 4).get(0) }, 513);
        assert_eq!(unsafe { off.self_follow(&vec[..], 4).get(1) }, 1027);
    }

    #[test]
    fn to_struct() {
        let vec: Vec<u8> = vec![255, 255, 255, 255, 1, 2, 3, 4];
        let off: flatbuffers::FollowStart<&FooStruct> = flatbuffers::FollowStart::new();
        assert_eq!(unsafe { *off.self_follow(&vec[..], 4) }, FooStruct::new(1, 2, 1027));
    }

    #[test]
    fn to_vector_of_offset_to_string_elements() {
        let buf: Vec<u8> = vec![/* vec len */ 1, 0, 0, 0, /* offset to string */ 4, 0, 0, 0, /* str length */ 3, 0, 0, 0, 'f' as u8, 'o' as u8, 'o' as u8, 0];
        let s: flatbuffers::FollowStart<flatbuffers::Vector<flatbuffers::ForwardsUOffset<&str>>> = flatbuffers::FollowStart::new();
        assert_eq!(unsafe {s.self_follow(&buf[..], 0).len() }, 1);
        assert_eq!(unsafe { s.self_follow(&buf[..], 0).get(0) }, "foo");
    }

    #[test]
    fn to_vector_of_struct_elements() {
        let buf: Vec<u8> = vec![1, 0, 0, 0, /* struct data */ 1, 2, 3, 4];
        let fs: flatbuffers::FollowStart<flatbuffers::Vector<FooStruct>> = flatbuffers::FollowStart::new();
        assert_eq!(unsafe { fs.self_follow(&buf[..], 0).len() }, 1);
        assert_eq!(unsafe { fs.self_follow(&buf[..], 0).get(0) }, &FooStruct::new(1, 2, 1027));
    }

    #[test]
    fn to_root_to_empty_table() {
        let buf: Vec<u8> = vec![
            12, 0, 0, 0, // offset to root table
            // enter vtable
            4, 0, // vtable len
            0, 0, // inline size
            255, 255, 255, 255, // canary
            // enter table
            8, 0, 0, 0, // vtable location
        ];
        unsafe {
            let fs: flatbuffers::FollowStart<flatbuffers::ForwardsUOffset<flatbuffers::Table>> = flatbuffers::FollowStart::new();
            assert_eq!(fs.self_follow(&buf[..], 0), flatbuffers::Table::new(&buf[..], 12));
        }
    }

    #[test]
    fn to_root_table_get_slot_scalar_u8() {
        let buf: Vec<u8> = vec![
            14, 0, 0, 0, // offset to root table
            // enter vtable
            6, 0, // vtable len
            2, 0, // inline size
            5, 0, // value loc
            255, 255, 255, 255, // canary
            // enter table
            10, 0, 0, 0, // vtable location
            0, 99 // value (with padding)
        ];
        unsafe {
            let fs: flatbuffers::FollowStart<flatbuffers::ForwardsUOffset<flatbuffers::Table>> = flatbuffers::FollowStart::new();
            let tab = fs.self_follow(&buf[..], 0);
            assert_eq!(tab.get::<u8>(fi2fo(0), Some(123)), Some(99));
        }
    }

    #[test]
    fn to_root_to_table_get_slot_scalar_u8_default_via_vtable_len() {
        let buf: Vec<u8> = vec![
            12, 0, 0, 0, // offset to root table
            // enter vtable
            4, 0, // vtable len
            2, 0, // inline size
            255, 255, 255, 255, // canary
            // enter table
            8, 0, 0, 0, // vtable location
        ];
        unsafe {
            let fs: flatbuffers::FollowStart<flatbuffers::ForwardsUOffset<flatbuffers::Table>> = flatbuffers::FollowStart::new();
            let tab = fs.self_follow(&buf[..], 0);
            assert_eq!(tab.get::<u8>(fi2fo(0), Some(123)), Some(123));
        }
    }

    #[test]
    fn to_root_to_table_get_slot_scalar_u8_default_via_vtable_zero() {
        let buf: Vec<u8> = vec![
            14, 0, 0, 0, // offset to root table
            // enter vtable
            6, 0, // vtable len
            2, 0, // inline size
            0, 0, // zero means use the default value
            255, 255, 255, 255, // canary
            // enter table
            10, 0, 0, 0, // vtable location
        ];
        unsafe {
            let fs: flatbuffers::FollowStart<flatbuffers::ForwardsUOffset<flatbuffers::Table>> = flatbuffers::FollowStart::new();
            let tab = fs.self_follow(&buf[..], 0);
            assert_eq!(tab.get::<u8>(fi2fo(0), Some(123)), Some(123));
        }
    }

    #[test]
    fn to_root_to_table_get_slot_string_multiple_types() {
        let buf: Vec<u8> = vec![
            14, 0, 0, 0, // offset to root table
            // enter vtable
            6, 0, // vtable len
            2, 0, // inline size
            4, 0, // value loc
            255, 255, 255, 255, // canary
            // enter table
            10, 0, 0, 0, // vtable location
            8, 0, 0, 0, // offset to string
            // leave table
            255, 255, 255, 255, // canary
            // enter string
            3, 0, 0, 0, 109, 111, 111, 0 // string length and contents
        ];
        unsafe {
            let tab = <flatbuffers::ForwardsUOffset<flatbuffers::Table>>::follow(&buf[..], 0);
            assert_eq!(tab.get::<flatbuffers::ForwardsUOffset<&str>>(fi2fo(0), None), Some("moo"));
            let byte_vec = tab.get::<flatbuffers::ForwardsUOffset<flatbuffers::Vector<u8>>>(fi2fo(0), None).unwrap().bytes();
            assert_eq!(byte_vec, &vec![109, 111, 111][..]);
            let v = tab.get::<flatbuffers::ForwardsUOffset<flatbuffers::Vector<u8>>>(fi2fo(0), None).unwrap();
            assert_eq!(v.len(), 3);
            assert_eq!(v.get(0), 109);
            assert_eq!(v.get(1), 111);
            assert_eq!(v.get(2), 111);
        }
    }

    #[test]
    fn to_root_to_table_get_slot_string_multiple_types_default_via_vtable_len() {
        let buf: Vec<u8> = vec![
            12, 0, 0, 0, // offset to root table
            // enter vtable
            4, 0, // vtable len
            4, 0, // table inline len
            255, 255, 255, 255, // canary
            // enter table
            8, 0, 0, 0, // vtable location
        ];

        unsafe {
            let tab = <flatbuffers::ForwardsUOffset<flatbuffers::Table>>::follow(&buf[..], 0);
            assert_eq!(tab.get::<flatbuffers::ForwardsUOffset<&str>>(fi2fo(0), Some("abc")), Some("abc"));
            #[cfg(target_endian = "little")]
            {
                assert_eq!(tab.get::<flatbuffers::ForwardsUOffset<&[u8]>>(fi2fo(0), Some(&vec![70, 71, 72][..])), Some(&vec![70, 71, 72][..]));
            }

            let default_vec_buf: Vec<u8> = vec![3, 0, 0, 0, 70, 71, 72, 0];
            let default_vec = flatbuffers::Vector::new(&default_vec_buf[..], 0);
            let v = tab.get::<flatbuffers::ForwardsUOffset<flatbuffers::Vector<u8>>>(fi2fo(0), Some(default_vec)).unwrap();
            assert_eq!(v.len(), 3);
            assert_eq!(v.get(0), 70);
            assert_eq!(v.get(1), 71);
            assert_eq!(v.get(2), 72);
        }
    }

    #[test]
    fn to_root_to_table_get_slot_string_multiple_types_default_via_vtable_zero() {
        let buf: Vec<u8> = vec![
            14, 0, 0, 0, // offset to root table
            // enter vtable
            6, 0, // vtable len
            2, 0, // inline size
            0, 0, // value loc
            255, 255, 255, 255, // canary
            // enter table
            10, 0, 0, 0, // vtable location
        ];
        unsafe {
            let tab = <flatbuffers::ForwardsUOffset<flatbuffers::Table>>::follow(&buf[..], 0);
            assert_eq!(tab.get::<flatbuffers::ForwardsUOffset<&str>>(fi2fo(0), Some("abc")), Some("abc"));
            #[cfg(target_endian = "little")]
            {
                assert_eq!(tab.get::<flatbuffers::ForwardsUOffset<&[u8]>>(fi2fo(0), Some(&vec![70, 71, 72][..])), Some(&vec![70, 71, 72][..]));
            }

            let default_vec_buf: Vec<u8> = vec![3, 0, 0, 0, 70, 71, 72, 0];
            let default_vec = flatbuffers::Vector::new(&default_vec_buf[..], 0);
            let v = tab.get::<flatbuffers::ForwardsUOffset<flatbuffers::Vector<u8>>>(fi2fo(0), Some(default_vec)).unwrap();
            assert_eq!(v.len(), 3);
            assert_eq!(v.get(0), 70);
            assert_eq!(v.get(1), 71);
            assert_eq!(v.get(2), 72);
        }
    }
}

#[cfg(test)]
mod push_impls {
    extern crate flatbuffers;

    use super::my_game;

    fn check<'a>(b: &'a flatbuffers::FlatBufferBuilder, want: &'a [u8]) {
        let got = b.unfinished_data();
        assert_eq!(want, got);
    }

    #[test]
    fn push_u8() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.push(123u8);
        check(&b, &[123]);
    }

    #[test]
    fn push_u64() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.push(0x12345678);
        check(&b, &[0x78, 0x56, 0x34, 0x12]);
    }

    #[test]
    fn push_f64() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.push(3.14159265359f64);
        check(&b, &[234, 46, 68, 84, 251, 33, 9, 64]);
    }

    #[test]
    fn push_generated_struct() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.push(my_game::example::Test::new(10, 20));
        check(&b, &[10, 0, 20, 0]);
    }

    #[test]
    fn push_u8_vector_with_offset_with_alignment() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let off = b.create_vector(&[1u8, 2, 3, 4, 5, 6, 7, 8, 9][..]);
        b.push(off);
        check(&b, &[/* loc */ 4, 0, 0, 0, /* len */ 9, 0, 0, 0, /* val */ 1, 2, 3, 4, 5, 6, 7, 8, 9, /* padding */ 0, 0, 0]);
    }

    #[test]
    fn push_u8_u16_alignment() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.push(1u8);
        b.push(2u16);
        check(&b, &[2, 0, 0, 1]);
    }

    #[test]
    fn push_u8_u32_alignment() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.push(1u8);
        b.push(2u32);
        check(&b, &[2, 0, 0, 0, 0, 0, 0, 1]);
    }

    #[test]
    fn push_u8_u64_alignment() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.push(1u8);
        b.push(2u64);
        check(&b, &[2, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 1]);
    }
}

#[cfg(test)]
mod vtable_deduplication {
    extern crate flatbuffers;
    use flatbuffers::field_index_to_field_offset as fi2fo;

    fn check<'a>(b: &'a flatbuffers::FlatBufferBuilder, want: &'a [u8]) {
        let got = b.unfinished_data();
        assert_eq!(want, got);
    }

    #[test]
    fn one_empty_table() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let start0 = b.start_table();
        b.end_table(start0);
        check(&b, &[
              4, 0, // vtable size in bytes
              4, 0, // object inline data in bytes

              4, 0, 0, 0, // backwards offset to vtable
        ]);
    }

    #[test]
    fn two_empty_tables_are_deduplicated() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let start0 = b.start_table();
        b.end_table(start0);
        let start1 = b.start_table();
        b.end_table(start1);
        check(&b, &[
              252, 255, 255, 255, // forwards offset to vtable

              4, 0, // vtable size in bytes
              4, 0, // object inline data in bytes

              4, 0, 0, 0, // backwards offset to vtable
        ]);
    }

    #[test]
    fn two_tables_with_two_conveniently_sized_inline_elements_are_deduplicated() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let start0 = b.start_table();
        b.push_slot::<u64>(fi2fo(0), 100, 0);
        b.push_slot::<u32>(fi2fo(1), 101, 0);
        b.end_table(start0);
        let start1 = b.start_table();
        b.push_slot::<u64>(fi2fo(0), 200, 0);
        b.push_slot::<u32>(fi2fo(1), 201, 0);
        b.end_table(start1);
        check(&b, &[
              240, 255, 255, 255, // forwards offset to vtable

              201, 0, 0, 0, // value #1
              200, 0, 0, 0, 0, 0, 0, 0, // value #0

              8, 0, // vtable size in bytes
              16, 0, // object inline data in bytes
              8, 0, // offset in object for value #0
              4, 0, // offset in object for value #1

              8, 0, 0, 0, // backwards offset to vtable
              101, 0, 0, 0, // value #1
              100, 0, 0, 0, 0, 0, 0, 0 // value #0
        ]);
    }

    #[cfg(not(miri))]  // slow.
    #[test]
    fn many_identical_tables_use_few_vtables() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        for _ in 0..1000 {
            let start = b.start_table();
            b.push_slot::<u8>(fi2fo(0), 100, 0);
            b.push_slot::<u32>(fi2fo(1), 101, 0);
            b.end_table(start);
        }
        assert!(b.num_written_vtables() <= 10);
    }
}

#[cfg(test)]
mod byte_layouts {
    extern crate flatbuffers;
    use flatbuffers::field_index_to_field_offset as fi2fo;

    fn check<'a>(b: &'a flatbuffers::FlatBufferBuilder, want: &'a [u8]) {
        let got = b.unfinished_data();
        assert_eq!(want, got);
    }

    #[test]
    fn layout_01_basic_numbers() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.push(true);
        check(&b, &[1]);
        b.push(-127i8);
        check(&b, &[129, 1]);
        b.push(255u8);
        check(&b, &[255, 129, 1]);
        b.push(-32222i16);
        check(&b, &[0x22, 0x82, 0, 255, 129, 1]); // first pad
        b.push(0xFEEEu16);
        check(&b, &[0xEE, 0xFE, 0x22, 0x82, 0, 255, 129, 1]); // no pad this time
        b.push(-53687092i32);
        check(&b, &[204, 204, 204, 252, 0xEE, 0xFE, 0x22, 0x82, 0, 255, 129, 1]);
        b.push(0x98765432u32);
        check(&b, &[0x32, 0x54, 0x76, 0x98, 204, 204, 204, 252, 0xEE, 0xFE, 0x22, 0x82, 0, 255, 129, 1]);
    }

    #[test]
    fn layout_01b_bigger_numbers() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.push(0x1122334455667788u64);
        check(&b, &[0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11]);
    }

    #[test]
    fn layout_02_1xbyte_vector() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        check(&b, &[]);
        b.start_vector::<u8>(1);
        check(&b, &[0, 0, 0]); // align to 4bytes
        b.push(1u8);
        check(&b, &[1, 0, 0, 0]);
        b.end_vector::<u8>(1);
        check(&b, &[1, 0, 0, 0, 1, 0, 0, 0]); // padding
    }

    #[test]
    fn layout_03_2xbyte_vector() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.start_vector::<u8>(2);
        check(&b, &[0, 0]); // align to 4bytes
        b.push(1u8);
        check(&b, &[1, 0, 0]);
        b.push(2u8);
        check(&b, &[2, 1, 0, 0]);
        b.end_vector::<u8>(2);
        check(&b, &[2, 0, 0, 0, 2, 1, 0, 0]); // padding
    }

    #[test]
    fn layout_03b_11xbyte_vector_matches_builder_size() {
        let mut b = flatbuffers::FlatBufferBuilder::with_capacity(12);
        b.start_vector::<u8>(8);

        let mut gold = vec![0u8; 0];
        check(&b, &gold[..]);

        for i in 1u8..=8 {
            b.push(i);
            gold.insert(0, i);
            check(&b, &gold[..]);
        }
        b.end_vector::<u8>(8);
        let want = vec![8u8, 0, 0, 0,  8, 7, 6, 5, 4, 3, 2, 1];
        check(&b, &want[..]);
    }
    #[test]
    fn layout_04_1xuint16_vector() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.start_vector::<u16>(1);
        check(&b, &[0, 0]); // align to 4bytes
        b.push(1u16);
        check(&b, &[1, 0, 0, 0]);
        b.end_vector::<u16>(1);
        check(&b, &[1, 0, 0, 0, 1, 0, 0, 0]); // padding
    }

    #[test]
    fn layout_05_2xuint16_vector() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let _off = b.start_vector::<u16>(2);
        check(&b, &[]); // align to 4bytes
        b.push(0xABCDu16);
        check(&b, &[0xCD, 0xAB]);
        b.push(0xDCBAu16);
        check(&b, &[0xBA, 0xDC, 0xCD, 0xAB]);
        b.end_vector::<u16>(2);
        check(&b, &[2, 0, 0, 0, 0xBA, 0xDC, 0xCD, 0xAB]);
    }

    #[test]
    fn layout_06_create_string() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let off0 = b.create_string("foo");
        assert_eq!(8, off0.value());
        check(&b, b"\x03\x00\x00\x00foo\x00"); // 0-terminated, no pad
        let off1 = b.create_string("moop");
        assert_eq!(20, off1.value());
        check(&b, b"\x04\x00\x00\x00moop\x00\x00\x00\x00\
                    \x03\x00\x00\x00foo\x00"); // 0-terminated, 3-byte pad
    }

    #[test]
    fn layout_06b_create_string_unicode() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        // These characters are chinese from blog.golang.org/strings
        // We use escape codes here so that editors without unicode support
        // aren't bothered:
        let uni_str = "\u{65e5}\u{672c}\u{8a9e}";
        let off0 = b.create_string(uni_str);
        assert_eq!(16, off0.value());
        check(&b, &[9, 0, 0, 0, 230, 151, 165, 230, 156, 172, 232, 170, 158, 0, //  null-terminated, 2-byte pad
                    0, 0]);
    }

    #[test]
    fn layout_06c_create_byte_string() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let off0 = b.create_byte_string(b"foo");
        assert_eq!(8, off0.value());
        check(&b, b"\x03\x00\x00\x00foo\x00"); // 0-terminated, no pad
        let off1 = b.create_byte_string(b"moop");
        assert_eq!(20, off1.value());
        check(&b, b"\x04\x00\x00\x00moop\x00\x00\x00\x00\
                    \x03\x00\x00\x00foo\x00"); // 0-terminated, 3-byte pad
    }

    #[test]
    fn layout_07_empty_vtable() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let off0 = b.start_table();
        check(&b, &[]);
        b.end_table(off0);
        check(&b, &[4, 0, // vtable length
                    4, 0, // length of table including vtable offset
                    4, 0, 0, 0]); // offset for start of vtable
    }

    #[test]
    fn layout_08_vtable_with_one_true_bool() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        check(&b, &[]);
        let off0 = b.start_table();
        assert_eq!(0, off0.value());
        check(&b, &[]);
        b.push_slot(fi2fo(0), true, false);
        check(&b, &[1]);
        let off1 = b.end_table(off0);
        assert_eq!(8, off1.value());
        check(&b, &[
              6, 0, // vtable bytes
              8, 0, // length of object including vtable offset
              7, 0, // start of bool value
              6, 0, 0, 0, // offset for start of vtable (int32)
              0, 0, 0, // padded to 4 bytes
              1, // bool value
        ]);
    }

    #[test]
    fn layout_09_vtable_with_one_default_bool() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        check(&b, &[]);
        let off = b.start_table();
        check(&b, &[]);
        b.push_slot(fi2fo(0), false, false);
        b.end_table(off);
        check(&b, &[
             4, 0, // vtable bytes
             4, 0, // end of object from here
             // entry 1 is zero and not stored.
             4, 0, 0, 0, // offset for start of vtable (int32)
        ]);
    }

    #[test]
    fn layout_09b_vtable_with_one_default_bool_force_defaults() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        check(&b, &[]);
        let off = b.start_table();
        check(&b, &[]);
        b.force_defaults(true);
        b.push_slot(fi2fo(0), false, false);
        b.end_table(off);
        check(&b, &[
            6, 0, // vtable bytes
            8, 0, // length of object including vtable offset
            7, 0, // start of bool value
            6, 0, 0, 0, // offset for start of vtable (int32)
            0, 0, 0, // padded to 4 bytes
            0, // bool value
      ]);
    }

    #[test]
    fn layout_10_vtable_with_one_int16() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        check(&b, &[]);
        let off = b.start_table();
        b.push_slot(fi2fo(0), 0x789Ai16, 0);
        b.end_table(off);
        check(&b, &[
              6, 0, // vtable bytes
              8, 0, // end of object from here
              6, 0, // offset to value
              6, 0, 0, 0, // offset for start of vtable (int32)
              0, 0, // padding to 4 bytes
              0x9A, 0x78,
        ]);
    }

    #[test]
    fn layout_11_vtable_with_two_int16() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let off = b.start_table();
        b.push_slot(fi2fo(0), 0x3456i16, 0);
        b.push_slot(fi2fo(1), 0x789Ai16, 0);
        b.end_table(off);
        check(&b, &[
              8, 0, // vtable bytes
              8, 0, // end of object from here
              6, 0, // offset to value 0
              4, 0, // offset to value 1
              8, 0, 0, 0, // offset for start of vtable (int32)
              0x9A, 0x78, // value 1
              0x56, 0x34, // value 0
        ]);
    }

    #[test]
    fn layout_12_vtable_with_int16_and_bool() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let off = b.start_table();
        b.push_slot(fi2fo(0), 0x3456i16, 0);
        b.push_slot(fi2fo(1), true, false);
        b.end_table(off);
        check(&b, &[
            8, 0, // vtable bytes
            8, 0, // end of object from here
            6, 0, // offset to value 0
            5, 0, // offset to value 1
            8, 0, 0, 0, // offset for start of vtable (int32)
            0,          // padding
            1,          // value 1
            0x56, 0x34, // value 0
        ]);
    }

    #[test]
    fn layout_12b_vtable_with_empty_vector() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.start_vector::<u8>(0);
        let vecend = b.end_vector::<u8>(0);
        let off = b.start_table();
        b.push_slot_always(fi2fo(0), vecend);
        b.end_table(off);
        check(&b, &[
              6, 0, // vtable bytes
              8, 0,
              4, 0, // offset to vector offset
              6, 0, 0, 0, // offset for start of vtable (int32)
              4, 0, 0, 0,
              0, 0, 0, 0, // length of vector (not in struct)
        ]);
    }

    #[test]
    fn layout_12c_vtable_with_empty_vector_of_byte_and_some_scalars() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.start_vector::<u8>(0);
        let vecend = b.end_vector::<u8>(0);
        let off = b.start_table();
        b.push_slot::<i16>(fi2fo(0), 55i16, 0);
        b.push_slot_always::<flatbuffers::WIPOffset<_>>(fi2fo(1), vecend);
        b.end_table(off);
        check(&b, &[
              8, 0, // vtable bytes
              12, 0,
              10, 0, // offset to value 0
              4, 0, // offset to vector offset
              8, 0, 0, 0, // vtable loc
              8, 0, 0, 0, // value 1
              0, 0, 55, 0, // value 0

              0, 0, 0, 0, // length of vector (not in struct)
        ]);
    }
    #[test]
    fn layout_13_vtable_with_1_int16_and_2_vector_of_i16() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.start_vector::<i16>(2);
        b.push(0x1234i16);
        b.push(0x5678i16);
        let vecend = b.end_vector::<i16>(2);
        let off = b.start_table();
        b.push_slot_always(fi2fo(1), vecend);
        b.push_slot(fi2fo(0), 55i16, 0);
        b.end_table(off);
        check(&b, &[
              8, 0, // vtable bytes
              12, 0, // length of object
              6, 0, // start of value 0 from end of vtable
              8, 0, // start of value 1 from end of buffer
              8, 0, 0, 0, // offset for start of vtable (int32)
              0, 0, // padding
              55, 0, // value 0
              4, 0, 0, 0, // vector position from here
              2, 0, 0, 0, // length of vector (uint32)
              0x78, 0x56, // vector value 1
              0x34, 0x12, // vector value 0
        ]);
    }
    #[test]
    fn layout_14_vtable_with_1_struct_of_int8_and_int16_and_int32() {
        #[derive(Copy, Clone, Debug, Eq, PartialEq)]
        #[repr(C, packed)]
        struct foo {
            a: i32,
            _pad0: [u8; 2],
            b: i16,
            _pad1: [u8; 3],
            c: i8,
            _pad2: [u8; 4],
        }
        assert_eq!(::core::mem::size_of::<foo>(), 16);
        impl<'b> flatbuffers::Push for &'b foo {
            type Output = foo;
            unsafe fn push<'a>(&'a self, dst: &'a mut [u8], _written_len: usize) {
                let src = ::core::slice::from_raw_parts(*self as *const foo as *const u8, ::core::mem::size_of::<foo>());
                dst.copy_from_slice(src);
            }
        }

        let mut b = flatbuffers::FlatBufferBuilder::new();
        let off = b.start_table();
        let x = foo{a: 0x12345678i32.to_le(), _pad0: [0,0], b: 0x1234i16.to_le(), _pad1: [0, 0, 0], c: 0x12i8.to_le(), _pad2: [0, 0, 0, 0]};
        b.push_slot_always(fi2fo(0), &x);
        b.end_table(off);
        check(&b, &[
              6, 0, // vtable bytes
              20, 0, // end of object from here
              4, 0, // start of struct from here
              6, 0, 0, 0, // offset for start of vtable (int32)

              0x78, 0x56, 0x34, 0x12, // value a
              0, 0, // padding
              0x34, 0x12, // value b
              0, 0, 0, // padding
              0x12, // value c
              0, 0, 0, 0, // padding
        ]);
    }
    #[test]
    fn layout_15_vtable_with_1_vector_of_4_int8() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        b.start_vector::<i8>(4);
        b.push(33i8);
        b.push(44i8);
        b.push(55i8);
        b.push(66i8);
        let vecend = b.end_vector::<i8>(4);
        let off = b.start_table();
        b.push_slot_always(fi2fo(0), vecend);
        b.end_table(off);
        check(&b, &[
              6, 0, // vtable bytes
              8, 0,
              4, 0, // offset of vector offset
              6, 0, 0, 0, // offset for start of vtable (int32)
              4, 0, 0, 0, // vector start offset

              4, 0, 0, 0, // vector length
              66, // vector value 1,1
              55, // vector value 1,0
              44, // vector value 0,1
              33, // vector value 0,0
        ]);
    }

    #[test]
    fn layout_16_table_with_some_elements() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let off = b.start_table();
        b.push_slot(fi2fo(0), 33i8, 0);
        b.push_slot(fi2fo(1), 66i16, 0);
        let off2 = b.end_table(off);
        b.finish_minimal(off2);

        check(&b, &[
              12, 0, 0, 0, // root of table: points to vtable offset

              8, 0, // vtable bytes
              8, 0, // end of object from here
              7, 0, // start of value 0
              4, 0, // start of value 1

              8, 0, 0, 0, // offset for start of vtable (int32)

              66, 0, // value 1
              0,  // padding
              33, // value 0
        ]);
    }

    #[test]
    fn layout_17_one_unfinished_table_and_one_finished_table() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        {
            let off = b.start_table();
            b.push_slot(fi2fo(0), 33i8, 0);
            b.push_slot(fi2fo(1), 44i8, 0);
            b.end_table(off);
        }

        {
            let off = b.start_table();
            b.push_slot(fi2fo(0), 55i8, 0);
            b.push_slot(fi2fo(1), 66i8, 0);
            b.push_slot(fi2fo(2), 77i8, 0);
            let off2 = b.end_table(off);
            b.finish_minimal(off2);
        }

        check(&b, &[
              16, 0, 0, 0, // root of table: points to object
              0, 0, // padding

              10, 0, // vtable bytes
              8, 0, // size of object
              7, 0, // start of value 0
              6, 0, // start of value 1
              5, 0, // start of value 2
              10, 0, 0, 0, // offset for start of vtable (int32)
              0,  // padding
              77, // value 2
              66, // value 1
              55, // value 0

              //12, 0, 0, 0, // root of table: points to object

              8, 0, // vtable bytes
              8, 0, // size of object
              7, 0, // start of value 0
              6, 0, // start of value 1
              8, 0, 0, 0, // offset for start of vtable (int32)
              0, 0, // padding
              44, // value 1
              33, // value 0
              ]);
    }

    #[test]
    fn layout_18_a_bunch_of_bools() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let off = b.start_table();
        b.push_slot(fi2fo(0), true, false);
        b.push_slot(fi2fo(1), true, false);
        b.push_slot(fi2fo(2), true, false);
        b.push_slot(fi2fo(3), true, false);
        b.push_slot(fi2fo(4), true, false);
        b.push_slot(fi2fo(5), true, false);
        b.push_slot(fi2fo(6), true, false);
        b.push_slot(fi2fo(7), true, false);
        let off2 = b.end_table(off);
        b.finish_minimal(off2);

        check(&b, &[
              24, 0, 0, 0, // root of table: points to vtable offset

              20, 0, // vtable bytes
              12, 0, // size of object
              11, 0, // start of value 0
              10, 0, // start of value 1
              9, 0, // start of value 2
              8, 0, // start of value 3
              7, 0, // start of value 4
              6, 0, // start of value 5
              5, 0, // start of value 6
              4, 0, // start of value 7
              20, 0, 0, 0, // vtable offset

              1, // value 7
              1, // value 6
              1, // value 5
              1, // value 4
              1, // value 3
              1, // value 2
              1, // value 1
              1, // value 0
              ]);
    }

    #[test]
    fn layout_19_three_bools() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let off = b.start_table();
        b.push_slot(fi2fo(0), true, false);
        b.push_slot(fi2fo(1), true, false);
        b.push_slot(fi2fo(2), true, false);
        let off2 = b.end_table(off);
        b.finish_minimal(off2);

        check(&b, &[
              16, 0, 0, 0, // root of table: points to vtable offset

              0, 0, // padding

              10, 0, // vtable bytes
              8, 0, // size of object
              7, 0, // start of value 0
              6, 0, // start of value 1
              5, 0, // start of value 2
              10, 0, 0, 0, // vtable offset from here

              0, // padding
              1, // value 2
              1, // value 1
              1, // value 0
        ]);
    }

    #[test]
    fn layout_20_some_floats() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let off = b.start_table();
        b.push_slot(fi2fo(0), 1.0f32, 0.0);
        b.end_table(off);

        check(&b, &[
              6, 0, // vtable bytes
              8, 0, // size of object
              4, 0, // start of value 0
              6, 0, 0, 0, // vtable offset

              0, 0, 128, 63, // value 0
        ]);
    }

    #[test]
    fn layout_21_vtable_defaults() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let off = b.start_table();
        b.push_slot::<i8>(fi2fo(0), 1, 1);
        b.push_slot::<i8>(fi2fo(1), 3, 2);
        b.push_slot::<i8>(fi2fo(2), 3, 3);
        b.end_table(off);
        check(&b, &[
              8, 0, // vtable size in bytes
              8, 0, // object inline data in bytes
              0, 0, // entry 1/3: 0 => default
              7, 0, // entry 2/3: 7 => table start + 7 bytes
              // entry 3/3: not present => default
              8, 0, 0, 0,
              0, 0, 0,
              3,
        ]);
    }

    #[test]
    fn layout_22_root() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let off = b.start_table();
        // skipped: b.push_slot_scalar::<i16>(0, 1, 1);
        b.push_slot::<i16>(fi2fo(1), 3, 2);
        b.push_slot::<i16>(fi2fo(2), 3, 3);
        let table_end = b.end_table(off);
        b.finish_minimal(table_end);
        check(&b, &[
              12, 0, 0, 0, // root

              8, 0, // vtable size in bytes
              8, 0, // object inline data in bytes
              0, 0, // entry 1/3: 0 => default
              6, 0, // entry 2/3: 6 => table start + 6 bytes
              // entry 3/3: not present => default
              8, 0, 0, 0, // size of table data in bytes
              0, 0, // padding
              3, 0, // value 2/3
        ]);
    }
    #[test]
    fn layout_23_varied_slots_and_root() {
        let mut b = flatbuffers::FlatBufferBuilder::new();
        let off = b.start_table();
        b.push_slot::<i16>(fi2fo(0), 1, 0);
        b.push_slot::<u8>(fi2fo(1), 2, 0);
        b.push_slot::<f32>(fi2fo(2), 3.0, 0.0);
        let table_end = b.end_table(off);
        b.finish_minimal(table_end);
        check(&b, &[
              16, 0, 0, 0, // root
              0, 0, // padding
              10, 0, // vtable bytes
              12, 0, // object inline data size
              10, 0, // offset to value #1 (i16)
              9, 0, // offset to value #2 (u8)
              4, 0, // offset to value #3 (f32)
              10, 0, // offset to vtable
              0, 0, // padding
              0, 0, 64, 64, // value #3 => 3.0 (float32)
              0, 2, // value #1 => 2 (u8)
              1, 0, // value #0 => 1 (int16)
        ]);
    }
}

#[cfg(test)]
mod copy_clone_traits {

    use alloc::vec::Vec;

    #[test]
    fn follow_types_implement_copy_and_clone() {
        static_assertions::assert_impl_all!(flatbuffers::WIPOffset<u32>: Copy, Clone);
        static_assertions::assert_impl_all!(flatbuffers::WIPOffset<Vec<u32>>: Copy, Clone);

        static_assertions::assert_impl_all!(flatbuffers::ForwardsUOffset<u32>: Copy, Clone);
        static_assertions::assert_impl_all!(flatbuffers::ForwardsUOffset<Vec<u32>>: Copy, Clone);

        static_assertions::assert_impl_all!(flatbuffers::Vector<'static, u32>: Copy, Clone);
        static_assertions::assert_impl_all!(flatbuffers::Vector<'static, Vec<u32>>: Copy, Clone);
    }
}

#[cfg(test)]
mod fully_qualified_name {
    #[test]
    fn fully_qualified_name_generated() {
        assert!(check_eq!(super::my_game::example::Monster::get_fully_qualified_name(), "MyGame.Example.Monster").is_ok());
        assert!(check_eq!(super::my_game::example_2::Monster::get_fully_qualified_name(), "MyGame.Example2.Monster").is_ok());

        assert!(check_eq!(super::my_game::example::Vec3::get_fully_qualified_name(), "MyGame.Example.Vec3").is_ok());
        assert!(check_eq!(super::my_game::example::Ability::get_fully_qualified_name(), "MyGame.Example.Ability").is_ok());
    }
}

// this is not technically a test, but we want to always keep this generated
// file up-to-date, and the simplest way to do that is to make sure that when
// tests are run, the file is generated.
#[cfg(not(feature = "no_std"))]
#[test]
fn write_example_wire_data_to_file() {
    let b = &mut flatbuffers::FlatBufferBuilder::new();
    create_serialized_example_with_generated_code(b);

    use ::std::io::Write;
    let mut f = std::fs::File::create("../monsterdata_rust_wire.mon").unwrap();
    f.write_all(b.finished_data()).unwrap();
}

#[cfg(not(feature = "no_std"))]
fn load_file(filename: &str) -> Result<Vec<u8>, std::io::Error> {
    use std::io::Read;
    let mut f = std::fs::File::open(filename)?;
    let mut buf = Vec::new();
    f.read_to_end(&mut buf)?;
    Ok(buf)
}

#[test]
fn test_shared_strings() {
    let mut builder = flatbuffers::FlatBufferBuilder::new();
    let offset1 = builder.create_shared_string("welcome to flatbuffers!!");
    let offset2 = builder.create_shared_string("welcome");
    let offset3 = builder.create_shared_string("welcome to flatbuffers!!");
    assert_ne!(offset2.value(), offset3.value());
    assert_eq!(offset1.value(), offset3.value());
    builder.reset();
    let offset4 = builder.create_shared_string("welcome");
    let offset5 = builder.create_shared_string("welcome to flatbuffers!!");
    assert_ne!(offset2.value(), offset4.value());
    assert_ne!(offset5.value(), offset1.value());
    builder.reset();

    // Checks if the shared string function would always work with
    // an object in between the writes
    let name = builder.create_shared_string("foo");
    let enemy = my_game::example::Monster::create(&mut builder, &my_game::example::MonsterArgs {
        name: Some(name),
        ..Default::default()
    });
    let secondary_name = builder.create_shared_string("foo");
    assert_eq!(name.value(), secondary_name.value());

    // Builds a new monster object and embeds enemy into it so we can verify
    // that shared strings are working.
    let args = my_game::example::MonsterArgs {
        name: Some(secondary_name),
        enemy: Some(enemy),
        testarrayofstring: Some(builder.create_vector(&[name, secondary_name])),
        ..Default::default()
    };
    // Building secondary monster
    let main_monster = my_game::example::Monster::create(&mut builder, &args);
    builder.finish(main_monster, None);
    let monster = my_game::example::root_as_monster(builder.finished_data()).unwrap();

    // Checks if the embedded object (Enemy) name is foo
    assert_eq!(monster.enemy().unwrap().name(), "foo");
    let string_vector = monster.testarrayofstring().unwrap();
    // Check if the vector will have the same string
    assert_eq!(string_vector.get(0), "foo");
    assert_eq!(string_vector.get(1), "foo");
}

}
