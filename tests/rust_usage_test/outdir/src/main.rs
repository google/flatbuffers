// In this example, a build.rs file generates the code and then copies it into $OUT_DIR. 
extern crate flatbuffers;

#[cfg(target_family = "unix")]
#[allow(dead_code, unused_imports)]
mod generated {
    include!(concat!(env!("OUT_DIR"), "/monster_generated.rs"));
}

#[cfg(target_family = "windows")]
#[allow(dead_code, unused_imports)]
mod generated {
    include!(concat!(env!("OUT_DIR"), "\\monster_generated.rs"));
}

use generated::my_game::sample::{Monster, MonsterArgs};


fn main() {
    let mut fbb = flatbuffers::FlatBufferBuilder::new();
    let name = Some(fbb.create_string("bob"));
    let m = Monster::create(&mut fbb, &MonsterArgs {
        hp: 1,
        mana: 2,
        name,
        ..Default::default()
    });
    fbb.finish(m, None);
    let mon = flatbuffers::root::<Monster>(fbb.finished_data()).unwrap();
    assert_eq!(mon.hp(), 1);
    assert_eq!(mon.mana(), 2);
    assert_eq!(mon.name().unwrap(), "bob");
}

#[test]
fn test_main() {
    main()
}
