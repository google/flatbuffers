// #![feature(trace_macros)]
// #[macro_use]
// extern crate flatbuffers;

// fn main() {
    
// }

// #[cfg(test)]
// #[cfg(feature = "test_idl_gen")]
// mod test {
//     use std::rc::Rc;

//     #[allow(non_snake_case,dead_code,unused_imports, non_camel_case_types)]
//     mod MyGame;

//     use flatbuffers::*;
//     use self::MyGame::*;

//     #[test]
//     fn manual_monster_build() {
//         let mut b = Builder::with_capacity(0);
//         let str = b.create_string("MyMonster");

//         b.start_vector(1, 5, 1);
//         b.add_u8(4);
//         b.add_u8(3);
//         b.add_u8(2);
//         b.add_u8(1);
//         b.add_u8(0);
//         let inv = b.end_vector();

//         b.start_object(13);
//         b.add_slot_i16(2, 20, 100);
//         let mon2 = b.end_object();

//         // Test4Vector
//         b.start_vector(4, 2, 1);

//         // Test 0
//         b.prep(2, 4);
//         b.pad(1);
//         b.add_i8(20);
//         b.add_i16(10);

//         // Test 1
//         b.prep(2, 4);
//         b.pad(1);
//         b.add_i8(40);
//         b.add_i16(30);

//         // end testvector
//         let test4 = b.end_vector();

//         b.start_object(13);

//         // a vec3
//         b.prep(16, 32);
//         b.pad(2);
//         b.prep(2, 4);
//         b.pad(1);
//         b.add_u8(6);
//         b.add_i16(5);
//         b.pad(1);
//         b.add_u8(4);
//         b.add_f64(3.0);
//         b.pad(4);
//         b.add_f32(3.0);
//         b.add_f32(2.0);
//         b.add_f32(1.0);
//         let vec3_loc = b.offset();
//         // end vec3

//         b.add_slot_struct(0, vec3_loc as u32, 0); // vec3. noop
//         b.add_slot_i16(2, 80, 100);     // hp
//         b.add_slot_uoffset(3, str as u32, 0);
//         b.add_slot_uoffset(5, inv as u32, 0); // inventory
//         b.add_slot_u8(7, 1, 0);
//         b.add_slot_uoffset(8, mon2 as u32, 0);
//         b.add_slot_uoffset(9, test4 as u32, 0);
//         let mon = b.end_object();
        
//         b.finish_table(mon);
//     }

//     fn build_monster() {
//         use self::MyGame::example::monster::MonsterBuilder;

//         let mut b = Builder::with_capacity(0);
//         let name = b.create_string("MyMonster");
//         let test1 = b.create_string("test1");
//         let test2 = b.create_string("test2");
//         let fred = b.create_string("Fred");
//         b.start_inventory_vector(5);
//         b.add_u8(4);
//         b.add_u8(3);
//         b.add_u8(2);
//         b.add_u8(1);
//         b.add_u8(0);
//         let inv = b.end_vector();

//         b.start_monster();
//         b.add_name(fred);
//         let mon2 = b.end_object();


//         use self::MyGame::example::test::TestBuilder;
//         b.start_test4_vector(2);
//         b.build_test(10, 20);
//         b.build_test(30, 40);
//         let test4 = b.end_vector();

//         b.start_testarrayofstring_vector(2);
//         b.add_uoffset(test2);
//         b.add_uoffset(test1);
//         let test_array = b.end_vector();
//         use self::MyGame::example::stat::StatBuilder;
//         let mut stat = b;
//         let stat_id = stat.create_string("way out..statistical");
//         stat.start_stat();
//         stat.add_id(stat_id);
//         stat.add_val(20);
//         stat.add_count(0);
//         let stat_offset = stat.end_object();
//         b = stat;


//         use self::MyGame::example::vec3::Vec3Builder;
//         b.start_monster();
//         let pos = b.build_vec3(1.0, 2.0, 3.0, 3.0, 2, 5, 6);
//         b.add_pos(pos);

//         b.add_hp(80);
//         b.add_name(name);
//         b.add_inventory(inv);
//         b.add_test_type(1);
//         b.add_test(mon2);
//         b.add_test4(test4);
//         b.add_testempty(stat_offset);
//         b.add_testarrayofstring(test_array);
//         let mon = b.end_object();

//         b.finish_table(mon);
//     }

//     #[test]
//     fn build_monster_with_generated() {
//         build_monster();
//     }

//     #[test]
//     fn can_read_monster() {
//         use std::env;
//         use std::path::Path;
//         use std::io::{Read, BufReader};
//         use std::fs::File;
//         let mut data_path = env::current_exe().unwrap();
//         data_path.pop();
//         data_path.pop();
//         data_path.pop(); //create root
//         data_path.push(Path::new("monsterdata_test.mon"));
//         let f1 = File::open(data_path).unwrap();
//         let mut reader = BufReader::new(f1);
//         let mut buf: Vec<u8> = Vec::new();
//         reader.read_to_end(&mut buf).unwrap();
//         let rc = Rc::new(buf);
//         let table = Table::from_offset(rc.clone(), 0);
//         let monster = example::monster::Monster::new(table);
//         let got = monster.hp();
//         assert!(got == 80, "bad {}: want {:?} got {:?}", "HP", 80, got);
//         let got = monster.mana(); 
//         assert!(got == 150, "bad {}: want {:?} got {:?}", "Mana", 150, got);
//         let got = monster.name();
//         assert!(got == "MyMonster", "bad {}: want {:?} got {:?}", "Name", "MyMonster", got);
//         let vec3 = monster.pos();
//         assert!(vec3.is_some(), "bad {}: want {:?} got {:?}", "Pos", "Vec3", "None");
//         // verify the properties of the Vec3
//         let vec3 = vec3.unwrap();
//         let got = vec3.x();
//         assert!(got == 1.0, "bad {}: want {:?} got {:?}", "Vec3.x", "1.0", got);
//         let got = vec3.y();
//         assert!(got == 2.0, "bad {}: want {:?} got {:?}", "Vec3.x", "2.0", got);
//         let got = vec3.z();
//         assert!(got == 3.0, "bad {}: want {:?} got {:?}", "Vec3.x", "3.0", got);
//         let got = vec3.test1();
//         assert!(got == 3.0, "bad {}: want {:?} got {:?}", "Vec3.test1", "3.0", got);
//         let got = vec3.test2();
//         assert!(got == Some(example::Color::Green), "bad {}: want {:?} got {:?}",
//                 "Vec3.test2", "Color::Green", got);
//         // Verify properties of test3
//         let test3 = vec3.test3();
//         let got = test3.a();
//         assert!(got == 5, "bad {}: want {:?} got {:?}", "Test.a", "5", got);
//         let got = test3.b();
//         assert!(got == 6, "bad {}: want {:?} got {:?}", "Test.b", "6", got);
//         // Verify test type
//         let got = monster.test_type();
//         assert!(got == Some(example::AnyType::Monster), "bad {}: want {:?} got {:?}", "TestType", "Monster", got);
//         // initialize a Table from a union field
//         let got = monster.test();
//         if let Some(example::Any::Monster(monster2)) = got {
//             let got = monster2.name();
//             assert!(got == "Fred", "bad {}: want {:?} got {:?}", "Name", "Fred", got);
//         } else {
//             panic!( "bad {}: want {:?} got {:?}", "Test", "Monster Union", got)
//         }
//         let got = monster.inventory();

//         assert!(got.size_hint() == (5, Some(5)), "bad {}: want {:?} got {:?}",
//                 "Inventory", "Byte Vector of length 5", got);
//         use std::ops::Add;
//         let got: u8 = got.fold(0, Add::add);
//         assert!(got == 10, "bad {}: want {:?} got {:?}", "Inventory Sum", "10", got);
//         // Test4
//         let mut got = monster.test4();
//         assert!(got.size_hint() == (2, Some(2)), "bad {}: want {:?} got {:?}", "Test4 vector length", "4", got.size_hint());
//         let test1 = got.next().unwrap();
//         let test2 = got.next().unwrap();
//         assert!(test1.a() == 10, "bad {}: want {:?} got {:?}", "Test4 array test1.a", "10", test1.a());
//         assert!(test1.b() == 20, "bad {}: want {:?} got {:?}", "Test4 array test1.b", "20", test1.b());
//         assert!(test2.a() == 30, "bad {}: want {:?} got {:?}", "Test4 array test2.a", "30", test2.a());
//         assert!(test2.b() == 40, "bad {}: want {:?} got {:?}", "Test4 array test2.b", "40", test2.b());
//         // Test Array of string
//         let mut got = monster.testarrayofstring();
//         assert!(got.size_hint() == (2, Some(2)), "bad {}: want {:?} got {:?}", "Test4 string vector length", "2", got.size_hint());
//         let str1 = got.next().unwrap();
//         let str2 = got.next().unwrap();
//         assert!(str1 == "test1",  "bad {}: want {:?} got {:?}", "Test string array 1", "test1", str1);
//         assert!(str2 == "test2",  "bad {}: want {:?} got {:?}", "Test string array 2", "test2", str2);
//         // array of tables
//         let got = monster.testarrayoftables();
//         assert_eq!(got.size_hint(),(0, Some(0)));
//     }

    

//     // until benchmark tests are stablized in Rust...
//     #[test]
//     fn bench_build() {
//         use std::time::Instant;
//         let now = Instant::now();
//         for _ in 1..10000 {
//             build_monster()
//         }
//         println!("Building 1000 took {:?}", now.elapsed());
//     }
// }
