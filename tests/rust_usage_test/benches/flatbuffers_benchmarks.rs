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

#[macro_use]
extern crate bencher;
use bencher::Bencher;

/*
#[allow(dead_code, unused_imports)]
#[path = "../../include_test/include_test1_generated.rs"]
pub mod include_test1_generated;

#[allow(dead_code, unused_imports)]
#[path = "../../include_test/sub/include_test2_generated.rs"]
pub mod include_test2_generated;
*/

#[allow(dead_code, unused_imports)]
#[path = "../../monster_test_generated.rs"]
mod monster_test_generated;
pub use monster_test_generated::my_game;

use flatbuffers::serialize::buffer::{Buffer, BumpaloBuffer};
use flatbuffers::serialize::cache::{Cache, HashMapCache};
use flatbuffers::{Flatbuffer, FlatbufferBuilder};

fn traverse_canonical_buffer(bench: &mut Bencher) {
    let owned_data = {
        let mut flatbuffer = Flatbuffer::new(BumpaloBuffer::default(), HashMapCache::default());
        create_serialized_example_with_generated_code(flatbuffer.new_message()).unwrap();
        flatbuffer.collect_last_message().unwrap()
    };
    let data = &owned_data[..];
    let n = data.len() as u64;
    bench.iter(|| {
        traverse_serialized_example_with_generated_code(data);
    });
    bench.bytes = n;
}

fn create_canonical_buffer_then_reset(bench: &mut Bencher) {
    let mut flatbuffer = Flatbuffer::new(BumpaloBuffer::default(), HashMapCache::default());
    create_serialized_example_with_generated_code(flatbuffer.new_message()).unwrap();
    create_serialized_example_with_generated_code(flatbuffer.new_message()).unwrap();
    let n = flatbuffer.last_message_size().unwrap() as u64;

    bench.iter(|| {
        create_serialized_example_with_generated_code(flatbuffer.new_message()).unwrap();
    });

    bench.bytes = n;
}

fn create_serialized_example_with_generated_code<'a, B, C>(
    mut builder: FlatbufferBuilder<'a, B, C>,
) -> Result<(), flatbuffers::errors::OutOfBufferSpace>
where
    B: Buffer<'a>,
    C: Cache<'a, 'a>,
{
    let s0 = builder.create_string("test1")?;
    let s1 = builder.create_string("test2")?;
    let t0_name = builder.create_string("Barney")?;
    let t1_name = builder.create_string("Fred")?;
    let t2_name = builder.create_string("Wilma")?;
    let t0 = builder.create_table(&my_game::example::Monster {
        hp: 1000,
        name: Some(t0_name),
        ..Default::default()
    })?;
    let t1 = builder.create_table(&my_game::example::Monster {
        name: Some(t1_name),
        ..Default::default()
    })?;
    let t2 = builder.create_table(&my_game::example::Monster {
        name: Some(t2_name),
        ..Default::default()
    })?;
    let mon = {
        let name = builder.create_string("MyMonster")?;
        let fred_name = builder.create_string("Fred")?;
        let inventory = builder.create_vector_direct(&[0u8, 1, 2, 3, 4])?;
        let test4 = builder.create_vector(&[
            my_game::example::Test { a: 10, b: 20 },
            my_game::example::Test { a: 30, b: 40 },
        ])?;
        let pos = my_game::example::Vec3 {
            x: 1.0,
            y: 2.0,
            z: 3.0,
            test1: 3.0,
            test2: my_game::example::Color::Green,
            test3: my_game::example::Test { a: 5i16, b: 6i8 },
        };
        let args = my_game::example::Monster {
            hp: 80,
            mana: 150,
            name: Some(name),
            pos: Some(pos),
            test: Some(my_game::example::Any::Monster(builder.create_table(
                &my_game::example::Monster {
                    name: Some(fred_name),
                    ..Default::default()
                },
            )?)),
            inventory: Some(inventory),
            test4: Some(test4),
            testarrayofstring: Some(builder.create_vector(&[s0, s1])?),
            testarrayoftables: Some(builder.create_vector(&[t0, t1, t2])?),
            ..Default::default()
        };
        builder.create_table(&args)?
    };
    builder.finish(mon)
}

#[inline(always)]
fn blackbox<T>(t: T) -> T {
    // encapsulate this in case we need to turn it into a noop
    bencher::black_box(t)
}

#[inline(always)]
fn traverse_serialized_example_with_generated_code(bytes: &[u8]) {
    let m = my_game::example::get_root_as_monster(bytes).unwrap();
    blackbox(m.hp());
    blackbox(m.mana());
    blackbox(m.name());
    let pos = m.pos().unwrap();
    blackbox(pos.x());
    blackbox(pos.y());
    blackbox(pos.z());
    blackbox(pos.test1());
    blackbox(pos.test2());
    let pos_test3 = pos.test3();
    blackbox(pos_test3.a());
    blackbox(pos_test3.b());
    let monster2 = m.test().unwrap().as_monster().unwrap();
    blackbox(monster2.name());
    blackbox(m.inventory());
    blackbox(m.test4());
    let testarrayoftables = m.testarrayoftables().unwrap();
    blackbox(testarrayoftables.get(0).hp());
    blackbox(testarrayoftables.get(0).name());
    blackbox(testarrayoftables.get(1).name());
    blackbox(testarrayoftables.get(2).name());
    let testarrayofstring = m.testarrayofstring().unwrap();
    blackbox(testarrayofstring.get(0));
    blackbox(testarrayofstring.get(1));
}

fn create_string_10(bench: &mut Bencher) {
    let mut flatbuffer = Flatbuffer::new(
        BumpaloBuffer::with_capacity(1 << 20),
        HashMapCache::default(),
    );
    let mut builder = flatbuffer.new_message();
    bench.iter(|| {
        builder.create_string("foobarbaz").unwrap(); // zero-terminated -> 10 bytes
    });

    bench.bytes = 10;
}

fn create_string_100(bench: &mut Bencher) {
    let mut flatbuffer = Flatbuffer::new(
        BumpaloBuffer::with_capacity(1 << 20),
        HashMapCache::default(),
    );
    let s_owned = (0..99).map(|_| "x").collect::<String>();
    let s: &str = &s_owned;

    bench.iter(|| {
        flatbuffer.new_message().create_string(s).unwrap(); // zero-terminated -> 100 bytes
    });

    bench.bytes = s.len() as u64;
}

fn create_string_100_no_reset(bench: &mut Bencher) {
    let mut flatbuffer = Flatbuffer::new(
        BumpaloBuffer::with_capacity(1 << 20),
        HashMapCache::default(),
    );
    let mut builder = flatbuffer.new_message();
    let s_owned = (0..99).map(|_| "x").collect::<String>();
    let s: &str = &s_owned;

    bench.iter(|| {
        builder.create_string(s).unwrap(); // zero-terminated -> 100 bytes
    });

    bench.bytes = s.len() as u64;
}

fn create_byte_vector_100_naive(bench: &mut Bencher) {
    let mut flatbuffer = Flatbuffer::new(
        BumpaloBuffer::with_capacity(1 << 20),
        HashMapCache::default(),
    );
    let v_owned = (0u8..100).map(|i| i).collect::<Vec<u8>>();
    let v: &[u8] = &v_owned;

    bench.iter(|| {
        flatbuffer.new_message().create_vector(v).unwrap(); // zero-terminated -> 100 bytes
    });

    bench.bytes = v.len() as u64;
}

fn create_byte_vector_100_naive_no_reset(bench: &mut Bencher) {
    let mut flatbuffer = Flatbuffer::new(
        BumpaloBuffer::with_capacity(1 << 20),
        HashMapCache::default(),
    );
    let mut builder = flatbuffer.new_message();
    let v_owned = (0u8..100).map(|i| i).collect::<Vec<u8>>();
    let v: &[u8] = &v_owned;

    bench.iter(|| {
        builder.create_vector(v).unwrap(); // zero-terminated -> 100 bytes
    });

    bench.bytes = v.len() as u64;
}

fn create_byte_vector_100_optimal(bench: &mut Bencher) {
    let mut flatbuffer = Flatbuffer::new(
        BumpaloBuffer::with_capacity(1 << 20),
        HashMapCache::default(),
    );
    let v_owned = (0u8..100).map(|i| i).collect::<Vec<u8>>();
    let v: &[u8] = &v_owned;

    bench.iter(|| {
        flatbuffer.new_message().create_vector_direct(v).unwrap(); // zero-terminated -> 100 bytes
    });

    bench.bytes = v.len() as u64;
}

fn create_byte_vector_100_optimal_no_reset(bench: &mut Bencher) {
    let mut flatbuffer = Flatbuffer::new(
        BumpaloBuffer::with_capacity(1 << 20),
        HashMapCache::default(),
    );
    let mut builder = flatbuffer.new_message();
    let v_owned = (0u8..100).map(|i| i).collect::<Vec<u8>>();
    let v: &[u8] = &v_owned;

    bench.iter(|| {
        builder.create_vector_direct(v).unwrap(); // zero-terminated -> 100 bytes
    });

    bench.bytes = v.len() as u64;
}

benchmark_group!(
    benches,
    create_byte_vector_100_naive,
    create_byte_vector_100_naive_no_reset,
    create_byte_vector_100_optimal,
    create_byte_vector_100_optimal_no_reset,
    traverse_canonical_buffer,
    create_canonical_buffer_then_reset,
    create_string_10,
    create_string_100,
    create_string_100_no_reset
);
benchmark_main!(benches);
