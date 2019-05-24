#![no_std]

extern crate flatbuffers;

#[allow(dead_code, unused_imports)]
#[path = "../../monster_test/mod.rs"]
mod monster_test_generated;
pub use monster_test_generated::my_game;

static MONSTER_DATA: &[u8] = include_bytes!("../../monsterdata_test.mon");

// Note: Copied from `bin/monster_example.rs`
#[test]
fn no_std_example_data() {
    let monster = my_game::example::get_root_as_monster(MONSTER_DATA);

    assert_eq!(monster.hp(), 80);
    assert_eq!(monster.mana(), 150);
    assert_eq!(monster.name(), "MyMonster");
}

// Note: the rest was copied from the `roundtrip_generated_code` module in
// `tests/integration_test.rs`. Really just making sure that everything compiles
// here, the behavior has already been tested.

fn build_mon<'a, 'b>(
    builder: &'a mut flatbuffers::FlatBufferBuilder,
    args: &'b my_game::example::MonsterArgs,
) -> my_game::example::Monster<'a> {
    let mon = my_game::example::Monster::create(builder, &args);
    my_game::example::finish_monster_buffer(builder, mon);
    my_game::example::get_root_as_monster(builder.finished_data())
}

#[test]
fn vector_of_struct_store_with_type_inference() {
    let mut b = flatbuffers::FlatBufferBuilder::new();
    let v = b.create_vector(&[
        my_game::example::Test::new(127, -128),
        my_game::example::Test::new(3, 123),
        my_game::example::Test::new(100, 101),
    ]);
    let name = b.create_string("foo");
    let m = build_mon(
        &mut b,
        &my_game::example::MonsterArgs {
            name: Some(name),
            test4: Some(v),
            ..Default::default()
        },
    );
    assert_eq!(
        m.test4().unwrap(),
        &[
            my_game::example::Test::new(127, -128),
            my_game::example::Test::new(3, 123),
            my_game::example::Test::new(100, 101)
        ][..]
    );
}