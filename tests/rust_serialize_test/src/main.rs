#[allow(dead_code, unused_imports)]
#[path = "../../monster_test_serialize/mod.rs"]
mod monster_test_serialize_generated;
pub use monster_test_serialize_generated::my_game;

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
            inventory: Some(builder.create_vector_direct(&[0u8, 1, 2, 3, 4][..])),
            test4: Some(builder.create_vector_direct(&[my_game::example::Test::new(10, 20),
                                                       my_game::example::Test::new(30, 40)])),
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
    let _ = serde_json::to_string(&obj).unwrap();
}
