#[allow(dead_code, unused_imports)]
#[path = "../../monster_test_serialize/mod.rs"]
mod monster_test_serialize_generated;
pub use monster_test_serialize_generated::my_game;

use crate::my_game::example::AnyAmbiguousAliases;
use crate::my_game::example::{Color, MonsterT, TestT, Vec3T, AnyT};
use std::collections::HashMap;

fn create_serialized_example_with_generated_code(builder: &mut flatbuffers::FlatBufferBuilder) {
    let mon = {
        let s0 = builder.create_string("test1");
        let s1 = builder.create_string("test2");
        let fred_name = builder.create_string("Fred");

        // can't inline creation of this Vec3 because we refer to it by reference, so it must live
        // long enough to be used by MonsterArgs.
        let pos = my_game::example::Vec3::new(
            1.0,
            2.0,
            3.0,
            3.0,
            my_game::example::Color::Green,
            &my_game::example::Test::new(5i16, 6i8),
        );

        let args = my_game::example::MonsterArgs {
            hp: 80,
            mana: 150,
            name: Some(builder.create_string("MyMonster")),
            pos: Some(&pos),
            test_type: my_game::example::Any::Monster,
            test: Some(
                my_game::example::Monster::create(
                    builder,
                    &my_game::example::MonsterArgs {
                        name: Some(fred_name),
                        ..Default::default()
                    },
                )
                .as_union_value(),
            ),
            inventory: Some(builder.create_vector(&[0u8, 1, 2, 3, 4])),
            test4: Some(builder.create_vector(&[
                my_game::example::Test::new(10, 20),
                my_game::example::Test::new(30, 40),
            ])),
            testarrayofstring: Some(builder.create_vector(&[s0, s1])),
            ..Default::default()
        };
        my_game::example::Monster::create(builder, &args)
    };
    my_game::example::finish_monster_buffer(builder, mon);
}

fn main() {
    // This test is built into its own crate because it has a different set of
    // dependencies as the normal Rust tests; it requires that the `flatbuffer`
    // dependency have the "serialize" feature enabled. As this feature may
    // cause extra code gen and dependencies for those who do not need it, it
    // is disabled by default.
    let mut builder = flatbuffers::FlatBufferBuilder::new();
    create_serialized_example_with_generated_code(&mut builder);

    let data = builder.finished_data();
    let obj = my_game::example::root_as_monster(&data[..]).unwrap();
    let value = serde_json::to_value(&obj).unwrap();
    let o = value.as_object().unwrap();

    let pos = o.get("pos").unwrap().as_object().unwrap();
    assert_eq!(pos.get("x").unwrap().as_f64().unwrap(), 1.0);
    assert_eq!(pos.get("y").unwrap().as_f64().unwrap(), 2.0);
    assert_eq!(pos.get("z").unwrap().as_f64().unwrap(), 3.0);

    let mana = o.get("mana").unwrap();
    assert_eq!(mana.as_i64().unwrap(), 150);

    let hp = o.get("hp").unwrap();
    assert_eq!(hp.as_i64().unwrap(), 80);

    let name = o.get("name").unwrap();
    assert_eq!(name.as_str().unwrap(), "MyMonster");

    let test_type = o.get("test_type").unwrap();
    assert_eq!(test_type.as_str().unwrap(), "Monster");

    let testarrayofstring = o.get("testarrayofstring").unwrap().as_array().unwrap();
    let t0 = testarrayofstring[0].as_str().unwrap();
    assert_eq!(t0, "test1");

    let t1 = testarrayofstring[1].as_str().unwrap();
    assert_eq!(t1, "test2");

    let s = r#"{"val":"M1"}"#;
    let des = serde_json::from_str::<HashMap<String, AnyAmbiguousAliases>>(s).unwrap();
    assert_eq!(*des.get("val").unwrap(), AnyAmbiguousAliases::M1);

    // Test Object API deserialization (JSON -> MonsterT)
    test_object_api_json_roundtrip();
    test_object_api_full_pipeline();
    test_object_api_deserialize_from_json_string();
    test_bitflags_enum_deserialize();
    test_union_deserialize();
}

/// Test JSON round-trip: MonsterT -> JSON -> MonsterT
fn test_object_api_json_roundtrip() {
    let monster = MonsterT {
        name: "Orc".to_string(),
        hp: 300,
        mana: 150,
        pos: Some(Vec3T {
            x: 1.0,
            y: 2.0,
            z: 3.0,
            test1: 0.0,
            test2: Color::Red,
            test3: TestT { a: 10, b: 20 },
        }),
        inventory: Some(vec![1, 2, 3, 4, 5]),
        color: Color::Blue,
        test4: Some(vec![TestT { a: 10, b: 20 }]),
        testarrayofstring: Some(vec!["hello".to_string(), "world".to_string()]),
        testbool: true,
        // Override NaN/Inf defaults to normal values for JSON round-trip
        // (JSON format cannot represent NaN/Infinity)
        nan_default: 0.0,
        inf_default: 0.0,
        positive_inf_default: 0.0,
        infinity_default: 0.0,
        positive_infinity_default: 0.0,
        negative_inf_default: 0.0,
        negative_infinity_default: 0.0,
        double_inf_default: 0.0,
        ..Default::default()
    };

    let json = serde_json::to_string(&monster).unwrap();
    let deserialized: MonsterT = serde_json::from_str(&json).unwrap();
    assert_eq!(monster, deserialized, "Object API JSON round-trip failed");

    // Verify specific fields survived the round-trip
    assert_eq!(deserialized.name, "Orc");
    assert_eq!(deserialized.hp, 300);
    assert_eq!(deserialized.mana, 150);
    assert_eq!(deserialized.color, Color::Blue);
    assert_eq!(deserialized.testbool, true);
    assert!(deserialized.pos.is_some());
    let pos = deserialized.pos.unwrap();
    assert_eq!(pos.x, 1.0);
    assert_eq!(pos.y, 2.0);
    assert_eq!(pos.z, 3.0);
    assert_eq!(deserialized.inventory, Some(vec![1, 2, 3, 4, 5]));
    assert_eq!(deserialized.testarrayofstring, Some(vec!["hello".to_string(), "world".to_string()]));

    eprintln!("OK: Object API JSON round-trip passed");
}

/// Test full pipeline: MonsterT -> FlatBuffer -> MonsterT -> JSON -> MonsterT -> FlatBuffer
fn test_object_api_full_pipeline() {
    // Create a MonsterT with known values (avoiding NaN/Inf for JSON compat)
    let original = MonsterT {
        name: "PipelineMonster".to_string(),
        hp: 200,
        mana: 100,
        pos: Some(Vec3T {
            x: 5.0,
            y: 6.0,
            z: 7.0,
            test1: 1.0,
            test2: Color::Green,
            test3: TestT { a: 1, b: 2 },
        }),
        inventory: Some(vec![10, 20, 30]),
        testbool: true,
        nan_default: 0.0,
        inf_default: 0.0,
        positive_inf_default: 0.0,
        infinity_default: 0.0,
        positive_infinity_default: 0.0,
        negative_inf_default: 0.0,
        negative_infinity_default: 0.0,
        double_inf_default: 0.0,
        ..Default::default()
    };

    // Pack to FlatBuffer
    let mut builder = flatbuffers::FlatBufferBuilder::new();
    let offset = original.pack(&mut builder);
    builder.finish(offset, None);
    let data = builder.finished_data();

    // Unpack from FlatBuffer
    let fb_monster = my_game::example::root_as_monster(data).unwrap();
    let unpacked = fb_monster.unpack();
    assert_eq!(unpacked.name, "PipelineMonster");

    // Serialize to JSON
    let json = serde_json::to_string(&unpacked).unwrap();

    // Deserialize back from JSON
    let deserialized: MonsterT = serde_json::from_str(&json).unwrap();
    assert_eq!(unpacked, deserialized, "Full pipeline round-trip failed");

    // Repack to FlatBuffer
    let mut builder2 = flatbuffers::FlatBufferBuilder::new();
    let offset2 = deserialized.pack(&mut builder2);
    builder2.finish(offset2, None);
    let data2 = builder2.finished_data();

    // Verify the repacked FlatBuffer is readable and correct
    let fb_monster2 = my_game::example::root_as_monster(data2).unwrap();
    assert_eq!(fb_monster2.name(), "PipelineMonster");
    assert_eq!(fb_monster2.hp(), 200);
    assert_eq!(fb_monster2.mana(), 100);

    eprintln!("OK: Object API full pipeline (FlatBuffer -> JSON -> FlatBuffer) passed");
}

/// Test deserializing a hand-written JSON string into MonsterT
fn test_object_api_deserialize_from_json_string() {
    let json = r#"{
        "name": "Goblin",
        "hp": 50,
        "mana": 100,
        "color": 8,
        "test": { "type": "NONE" },
        "any_unique": { "type": "NONE" },
        "any_ambiguous": { "type": "NONE" },
        "testbool": false,
        "testhashs32_fnv1": 0,
        "testhashu32_fnv1": 0,
        "testhashs64_fnv1": 0,
        "testhashu64_fnv1": 0,
        "testhashs32_fnv1a": 0,
        "testhashu32_fnv1a": 0,
        "testhashs64_fnv1a": 0,
        "testhashu64_fnv1a": 0,
        "testf": 3.14159,
        "testf2": 3.0,
        "testf3": 0.0,
        "single_weak_reference": 0,
        "co_owning_reference": 0,
        "non_owning_reference": 0,
        "signed_enum": "None",
        "long_enum_non_enum_default": 0,
        "long_enum_normal_default": 2,
        "nan_default": 0.0,
        "inf_default": 0.0,
        "positive_inf_default": 0.0,
        "infinity_default": 0.0,
        "positive_infinity_default": 0.0,
        "negative_inf_default": 0.0,
        "negative_infinity_default": 0.0,
        "double_inf_default": 0.0
    }"#;

    let monster: MonsterT = serde_json::from_str(json).unwrap();
    assert_eq!(monster.name, "Goblin");
    assert_eq!(monster.hp, 50);
    assert_eq!(monster.mana, 100);
    assert_eq!(monster.color, Color::Blue);
    assert!(monster.pos.is_none());
    assert!(monster.inventory.is_none());

    eprintln!("OK: Object API deserialize from JSON string passed");
}

/// Test bitflags enum deserialization
fn test_bitflags_enum_deserialize() {
    // Bitflags serialize/deserialize as numeric values
    let json = serde_json::to_string(&Color::Green).unwrap();
    let deserialized: Color = serde_json::from_str(&json).unwrap();
    assert_eq!(deserialized, Color::Green);

    let json = serde_json::to_string(&Color::Blue).unwrap();
    let deserialized: Color = serde_json::from_str(&json).unwrap();
    assert_eq!(deserialized, Color::Blue);

    eprintln!("OK: Bitflags enum deserialization passed");
}

/// Test union type deserialization
fn test_union_deserialize() {
    // Test NONE union variant
    let none_union = AnyT::NONE;
    let json = serde_json::to_string(&none_union).unwrap();
    let deserialized: AnyT = serde_json::from_str(&json).unwrap();
    assert_eq!(deserialized, AnyT::NONE);

    // Test union with Monster variant via full round-trip
    let inner = MonsterT {
        name: "Inner".to_string(),
        hp: 42,
        // Override NaN/Inf defaults for JSON
        nan_default: 0.0,
        inf_default: 0.0,
        positive_inf_default: 0.0,
        infinity_default: 0.0,
        positive_infinity_default: 0.0,
        negative_inf_default: 0.0,
        negative_infinity_default: 0.0,
        double_inf_default: 0.0,
        ..Default::default()
    };
    let union_val = AnyT::Monster(Box::new(inner));
    let json = serde_json::to_string(&union_val).unwrap();
    let deserialized: AnyT = serde_json::from_str(&json).unwrap();
    assert_eq!(deserialized, union_val);

    eprintln!("OK: Union type deserialization passed");
}
