#[allow(dead_code, unused_imports)]
#[path = "../../arrays_test_generated.rs"]
mod arrays_test_generated;
use crate::arrays_test_generated::my_game::example::*;

fn create_serialized_example_with_generated_code(builder: &mut flatbuffers::FlatBufferBuilder) {
    let nested_struct1 = NestedStruct::new(
        &[-1, 2],
        TestEnum::A,
        &[TestEnum::C, TestEnum::B],
        &[0x1122334455667788, -0x1122334455667788],
    );
    let nested_struct2 = NestedStruct::new(
        &[3, -4],
        TestEnum::B,
        &[TestEnum::B, TestEnum::A],
        &[-0x1122334455667788, 0x1122334455667788],
    );
    let array_struct = ArrayStruct::new(
        12.34,
        &[1, 2, 3, 4, 5, 6, 7, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF],
        -127,
        &[nested_struct1, nested_struct2],
        1,
        &[-0x8000000000000000, 0x7FFFFFFFFFFFFFFF],
    );
    // Test five makes sense when specified.
    let ss = ArrayTable::create(
        builder,
        &ArrayTableArgs {
            a: Some(&array_struct),
        },
    );
    finish_array_table_buffer(builder, ss);
}

fn serialized_example_is_accessible_and_correct(
    bytes: &[u8],
    identifier_required: bool,
    size_prefixed: bool,
) {
    if identifier_required {
        let correct = if size_prefixed {
            array_table_size_prefixed_buffer_has_identifier(bytes)
        } else {
            array_table_buffer_has_identifier(bytes)
        };
        assert_eq!(correct, true);
    }

    let array_table = if size_prefixed {
        size_prefixed_root_as_array_table(bytes).unwrap()
    } else {
        root_as_array_table(bytes).unwrap()
    };

    let array_struct = array_table.a().unwrap();
    assert_eq!(array_struct.a(), 12.34);
    assert_eq!(array_struct.b().len(), 0xF);
    assert_eq!(array_struct.b().iter().sum::<i32>(), 120);
    assert_eq!(array_struct.c(), -127);

    assert_eq!(array_struct.d().len(), 2);
    let nested_struct1 = array_struct.d().get(0);
    assert_eq!(nested_struct1.a().len(), 2);
    assert_eq!(nested_struct1.a().iter().sum::<i32>(), 1);
    assert_eq!(nested_struct1.b(), TestEnum::A);
    assert_eq!(nested_struct1.c().len(), 2);
    assert_eq!(nested_struct1.c().get(0), TestEnum::C);
    assert_eq!(nested_struct1.c().get(1), TestEnum::B);
    assert_eq!(nested_struct1.d().len(), 2);
    assert_eq!(
        nested_struct1.d().as_array(),
        &[0x1122334455667788, -0x1122334455667788]
    );
    let nested_struct2 = array_struct.d().get(1);
    assert_eq!(nested_struct2.a().len(), 2);
    assert_eq!(nested_struct2.a().iter().sum::<i32>(), -1);
    assert_eq!(nested_struct2.b(), TestEnum::B);
    assert_eq!(nested_struct2.c().len(), 2);
    assert_eq!(nested_struct2.c().get(0), TestEnum::B);
    assert_eq!(nested_struct2.c().get(1), TestEnum::A);
    assert_eq!(nested_struct2.d().len(), 2);
    assert_eq!(
        nested_struct2.d().as_array(),
        &[-0x1122334455667788, 0x1122334455667788]
    );

    assert_eq!(array_struct.e(), 1);
    assert_eq!(array_struct.f().len(), 2);
    assert_eq!(array_struct.f().get(0), -0x8000000000000000);
    assert_eq!(array_struct.f().get(1), 0x7FFFFFFFFFFFFFFF);
}

#[test]
fn generated_code_creates_correct_example() {
    let mut b = flatbuffers::FlatBufferBuilder::new();
    create_serialized_example_with_generated_code(&mut b);
    let buf = b.finished_data();
    serialized_example_is_accessible_and_correct(&buf[..], true, false);
}

#[test]
fn struct_netsted_struct_is_32_bytes() {
    assert_eq!(32, ::std::mem::size_of::<NestedStruct>());
}

#[test]
fn struct_array_struct_is_160_bytes() {
    assert_eq!(160, ::std::mem::size_of::<ArrayStruct>());
}

#[test]
fn test_object_api_reads_correctly() {
    let mut b = flatbuffers::FlatBufferBuilder::new();
    create_serialized_example_with_generated_code(&mut b);

    let array_table = root_as_array_table(b.finished_data()).unwrap().unpack();

    let array_struct = array_table.a.unwrap();
    assert_eq!(array_struct.a, 12.34);
    assert_eq!(array_struct.b.len(), 0xF);
    assert_eq!(array_struct.b.iter().sum::<i32>(), 120);
    assert_eq!(array_struct.c, -127);

    assert_eq!(array_struct.d.len(), 2);
    let nested_struct1 = &array_struct.d[0];
    assert_eq!(nested_struct1.a.len(), 2);
    assert_eq!(nested_struct1.a.iter().sum::<i32>(), 1);
    assert_eq!(nested_struct1.b, TestEnum::A);
    assert_eq!(nested_struct1.c.len(), 2);
    assert_eq!(nested_struct1.c[0], TestEnum::C);
    assert_eq!(nested_struct1.c[1], TestEnum::B);
    assert_eq!(nested_struct1.d.len(), 2);
    assert_eq!(nested_struct1.d, [0x1122334455667788, -0x1122334455667788]);
    let nested_struct2 = &array_struct.d[1];
    assert_eq!(nested_struct2.a.len(), 2);
    assert_eq!(nested_struct2.a.iter().sum::<i32>(), -1);
    assert_eq!(nested_struct2.b, TestEnum::B);
    assert_eq!(nested_struct2.c.len(), 2);
    assert_eq!(nested_struct2.c[0], TestEnum::B);
    assert_eq!(nested_struct2.c[1], TestEnum::A);
    assert_eq!(nested_struct2.d.len(), 2);
    assert_eq!(nested_struct2.d, [-0x1122334455667788, 0x1122334455667788]);

    assert_eq!(array_struct.e, 1);
    assert_eq!(array_struct.f.len(), 2);
    assert_eq!(array_struct.f[0], -0x8000000000000000);
    assert_eq!(array_struct.f[1], 0x7FFFFFFFFFFFFFFF);
}

#[test]
fn object_api_defaults() {
    use arrays_test_generated::my_game::example::*;

    assert_eq!(
        NestedStructT::default(),
        NestedStructT {
            a: [0, 0],
            b: TestEnum::default(),
            c: [TestEnum::default(), TestEnum::default()],
            d: [0, 0]
        }
    );

    assert_eq!(
        ArrayStructT::default(),
        ArrayStructT {
            a: 0.0,
            b: [0; 0xF],
            c: 0,
            d: [NestedStructT::default(), NestedStructT::default()],
            e: 0,
            f: [0, 0]
        }
    );
}

#[test]
fn generated_code_debug_prints_correctly() {
    let b = &mut flatbuffers::FlatBufferBuilder::new();
    create_serialized_example_with_generated_code(b);
    let buf = b.finished_data();
    serialized_example_is_accessible_and_correct(&buf, true, false);
    let array_table = root_as_array_table(buf).unwrap();
    assert_eq!(
        format!("{:.5?}", &array_table),
        "ArrayTable { a: Some(ArrayStruct { a: 12.34000, \
         b: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15], \
         c: -127, d: [NestedStruct { a: [-1, 2], b: A, c: [C, B], \
         d: [1234605616436508552, -1234605616436508552] }, \
         NestedStruct { a: [3, -4], b: B, c: [B, A], d: [-1234605616436508552, 1234605616436508552] }], \
         e: 1, f: [-9223372036854775808, 9223372036854775807] }) }"
    );
}
