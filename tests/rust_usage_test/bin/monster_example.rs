extern crate flatbuffers;

#[allow(dead_code, unused_imports)]
#[path = "../../include_test/include_test1_generated.rs"]
pub mod include_test1_generated;

#[allow(dead_code, unused_imports)]
#[path = "../../include_test/sub/include_test2_generated.rs"]
pub mod include_test2_generated;

#[allow(dead_code, unused_imports)]
#[path = "../../monster_test_generated.rs"]
mod monster_test_generated;
pub use monster_test_generated::my_game;


use std::io::Read;

fn main() {
    let mut f = std::fs::File::open("../monsterdata_test.mon").unwrap();
    let mut buf = Vec::new();
    f.read_to_end(&mut buf).expect("file reading failed");

    let monster = my_game::example::get_root_as_monster(&buf[..]);
    println!("{}", monster.hp());     // `80`
    println!("{}", monster.mana());   // default value of `150`
    println!("{:?}", monster.name()); // Some("MyMonster")
}
