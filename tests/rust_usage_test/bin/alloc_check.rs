// define a passthrough allocator that tracks alloc calls.
// (note that we can't drop this in to the usual test suite, because it's a big
// global variable).
use std::alloc::{GlobalAlloc, Layout, System};

static mut N_ALLOCS: usize = 0;

struct TrackingAllocator;

impl TrackingAllocator {
    fn n_allocs(&self) -> usize {
        unsafe { N_ALLOCS }
    }
}
unsafe impl GlobalAlloc for TrackingAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        N_ALLOCS += 1;
        System.alloc(layout)
    }
    unsafe fn dealloc(&self, ptr: *mut u8, layout: Layout) {
        System.dealloc(ptr, layout)
    }
}

// use the tracking allocator:
#[global_allocator]
static A: TrackingAllocator = TrackingAllocator;

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

use flatbuffers::serialize::buffer::{Buffer, BumpaloBuffer, SliceBuffer};
use flatbuffers::serialize::cache::{Cache, HashMapCache, UluruCache};
use flatbuffers::serialize::{Flatbuffer, FlatbufferBuilder};

// verbatim from the test suite:
fn create_serialized_example_with_generated_code<'a, B, C>(
    mut builder: FlatbufferBuilder<'a, B, C>,
) -> Result<(), flatbuffers::errors::OutOfBufferSpace>
where
    B: Buffer<'a>,
    C: Cache<'a, 'a>,
{
    let mon = {
        let strings = [
            builder.create_string("these")?,
            builder.create_string("unused")?,
            builder.create_string("strings")?,
            builder.create_string("check")?,
            builder.create_string("the")?,
            builder.create_string("create_vector_of_strings")?,
            builder.create_string("function")?,
        ];
        builder.create_vector(&strings)?;

        let s0 = builder.create_string("test1")?;
        let s1 = builder.create_string("test2")?;
        let fred_name = builder.create_string("Fred")?;

        let args = my_game::example::Monster {
            hp: 80,
            mana: 150,
            name: Some(builder.create_string("MyMonster")?),
            pos: Some(my_game::example::Vec3 {
                x: 1.0,
                y: 2.0,
                z: 3.0,
                test1: 3.0,
                test2: my_game::example::Color::Green,
                test3: my_game::example::Test { a: 5i16, b: 6i8 },
            }),
            test: Some(my_game::example::Any::Monster(builder.create_table(
                &my_game::example::Monster {
                    name: Some(fred_name),
                    ..Default::default()
                },
            )?)),
            inventory: Some(builder.create_vector(&[0u8, 1, 2, 3, 4][..])?),
            test4: Some(builder.create_vector(&[
                my_game::example::Test { a: 10, b: 20 },
                my_game::example::Test { a: 30, b: 40 },
            ])?),
            testarrayofstring: Some(builder.create_vector(&[s0, s1])?),
            ..Default::default()
        };
        builder.create_table(&args)?
    };
    builder.finish(mon)
}

fn main() {
    // test the allocation tracking:
    {
        let before = A.n_allocs();
        let _x: Vec<u8> = vec![0u8; 1];
        let after = A.n_allocs();
        assert_eq!(before + 1, after);
    }

    // When using SliceBuffer and UluruCache, no heap allocations ever occur, even on warmup
    {
        let mut output_slice = [0; 4096];
        let mut flatbuffer =
            Flatbuffer::new(SliceBuffer::new(&mut output_slice), UluruCache::default());
        let before = A.n_allocs();
        create_serialized_example_with_generated_code(flatbuffer.new_message()).unwrap();
        let after = A.n_allocs();
        assert_eq!(before, after, "KO: Heap allocs occurred in Rust write path");
    }

    // With a BumpaloBuffer and UluruCache we might need a bit of warm up
    let mut flatbuffer = Flatbuffer::new(BumpaloBuffer::default(), HashMapCache::default());
    create_serialized_example_with_generated_code(flatbuffer.new_message()).unwrap();
    create_serialized_example_with_generated_code(flatbuffer.new_message()).unwrap();

    let before = A.n_allocs();
    create_serialized_example_with_generated_code(flatbuffer.new_message()).unwrap();
    let after = A.n_allocs();
    assert_eq!(before, after, "KO: Heap allocs occurred in Rust write path");

    // Alternatively we can just pre-allocate sufficient size
    let mut flatbuffer = Flatbuffer::new(
        BumpaloBuffer::with_capacity(4000),
        HashMapCache::with_capacity(64),
    );
    let before = A.n_allocs();
    create_serialized_example_with_generated_code(flatbuffer.new_message()).unwrap();
    let after = A.n_allocs();
    assert_eq!(before, after, "KO: Heap allocs occurred in Rust write path");

    let buf = flatbuffer.collect_last_message().unwrap();

    // use the allocation tracking on the read path:
    {
        let before = A.n_allocs();

        // do many reads, forcing them to execute by using assert_eq:
        {
            let m = my_game::example::get_root_as_monster(&buf).unwrap();
            assert_eq!(80, m.hp());
            assert_eq!(150, m.mana());
            assert_eq!("MyMonster", m.name().as_str());

            let pos = m.pos().unwrap();
            // We know the bits should be exactly equal here but compilers may
            // optimize floats in subtle ways so we're playing it safe and using
            // epsilon comparison
            assert!((pos.x() - 1.0f32).abs() < std::f32::EPSILON);
            assert!((pos.y() - 2.0f32).abs() < std::f32::EPSILON);
            assert!((pos.z() - 3.0f32).abs() < std::f32::EPSILON);
            assert!((pos.test1() - 3.0f64).abs() < std::f64::EPSILON);
            assert_eq!(pos.test2(), my_game::example::Color::Green);
            let pos_test3 = pos.test3();
            assert_eq!(pos_test3.a(), 5i16);
            assert_eq!(pos_test3.b(), 6i8);
            let m2 = m.test().unwrap().as_monster().unwrap();

            assert_eq!(m2.name().as_str(), "Fred");

            let inv = m.inventory().unwrap();
            assert_eq!(inv.len(), 5);
            assert_eq!(inv.iter().sum::<u8>(), 10u8);

            let test4 = m.test4().unwrap();
            assert_eq!(test4.len(), 2);
            assert_eq!(
                i32::from(test4.get(0).a())
                    + i32::from(test4.get(1).a())
                    + i32::from(test4.get(0).b())
                    + i32::from(test4.get(1).b()),
                100
            );

            let testarrayofstring = m.testarrayofstring().unwrap();
            assert_eq!(testarrayofstring.len(), 2);
            assert_eq!(testarrayofstring.get(0).as_str(), "test1");
            assert_eq!(testarrayofstring.get(1).as_str(), "test2");
        }

        // assert that no allocs occurred:
        let after = A.n_allocs();
        assert_eq!(before, after, "KO: Heap allocs occurred in Rust read path");
    }
    println!("Rust: Heap alloc checks completed successfully");
}
