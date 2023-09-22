use flatbuffers_reflection::reflection::{root_as_schema, BaseType};

use std::fs::File;
use std::io::Read;

#[test]
fn test_schema_correct_root_table() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();

    let root_table = schema.root_table().unwrap();

    assert_eq!(root_table.name(), "MyGame.Example.Monster");
    assert_eq!(root_table.declaration_file().unwrap(), "//monster_test.fbs");
}

#[test]
fn test_schema_correct_object() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");

    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();

    assert_eq!(
        schema
            .objects()
            .lookup_by_key("TableA", |object, key| object.key_compare_with_value(key))
            .unwrap()
            .declaration_file()
            .unwrap(),
        "//include_test/include_test1.fbs"
    );
    assert_eq!(
        schema
            .objects()
            .lookup_by_key("MyGame.OtherNameSpace.Unused", |object, key| object
                .key_compare_with_value(key))
            .unwrap()
            .declaration_file()
            .unwrap(),
        "//include_test/sub/include_test2.fbs"
    );
}

#[test]
fn test_schema_correct_enum() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");

    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();

    assert_eq!(
        schema
            .enums()
            .lookup_by_key("MyGame.OtherNameSpace.FromInclude", |en, key| en
                .key_compare_with_value(key))
            .unwrap()
            .declaration_file()
            .unwrap(),
        "//include_test/sub/include_test2.fbs"
    );
}

#[test]
fn test_schema_correct_file() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");

    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();

    assert_eq!(schema.fbs_files().unwrap().len(), 3);

    let fbs0 = schema.fbs_files().unwrap().get(0);
    assert_eq!(fbs0.filename(), "//include_test/include_test1.fbs");
    let fbs0_includes = fbs0.included_filenames().unwrap();
    assert_eq!(fbs0_includes.len(), 2);
    assert_eq!(fbs0_includes.get(0), "//include_test/include_test1.fbs");
    assert_eq!(fbs0_includes.get(1), "//include_test/sub/include_test2.fbs");

    let fbs1 = schema.fbs_files().unwrap().get(1);
    assert_eq!(fbs1.filename(), "//include_test/sub/include_test2.fbs");
    let fbs1_includes = fbs1.included_filenames().unwrap();
    assert_eq!(fbs1_includes.len(), 2);
    assert_eq!(fbs1_includes.get(0), "//include_test/include_test1.fbs");
    assert_eq!(fbs1_includes.get(1), "//include_test/sub/include_test2.fbs");

    let fbs2 = schema.fbs_files().unwrap().get(2);
    assert_eq!(fbs2.filename(), "//monster_test.fbs");
    let fbs2_includes = fbs2.included_filenames().unwrap();
    assert_eq!(fbs2_includes.len(), 1);
    assert_eq!(fbs2_includes.get(0), "//include_test/include_test1.fbs");
}

#[test]
fn test_schema_correct_table_field() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let root_table = root_as_schema(schema_buffer.as_slice())
        .unwrap()
        .root_table()
        .unwrap();

    let fields = root_table.fields();

    let hp_field_option =
        fields.lookup_by_key("hp", |field, key| field.key_compare_with_value(key));
    assert!(hp_field_option.is_some());
    let hp_field = hp_field_option.unwrap();
    assert_eq!(hp_field.name(), "hp");
    assert_eq!(hp_field.id(), 2);
    assert_eq!(hp_field.type_().base_type(), BaseType::Short);

    let friendly_field_option =
        fields.lookup_by_key("friendly", |field, key| field.key_compare_with_value(key));
    assert!(friendly_field_option.is_some());
    assert!(friendly_field_option.unwrap().attributes().is_some());
    assert!(friendly_field_option
        .unwrap()
        .attributes()
        .unwrap()
        .lookup_by_key("priority", |key_value, key| key_value
            .key_compare_with_value(key))
        .is_some());
}

#[test]
fn test_schema_correct_table_field_nullability() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let root_table = root_as_schema(schema_buffer.as_slice())
        .unwrap()
        .root_table()
        .unwrap();

    let fields = root_table.fields();

    assert_eq!(
        fields
            .lookup_by_key("hp", |field, key| field.key_compare_with_value(key))
            .unwrap()
            .optional(),
        false
    );
    assert_eq!(
        fields
            .lookup_by_key("pos", |field, key| field.key_compare_with_value(key))
            .unwrap()
            .optional(),
        true
    );
    assert_eq!(
        fields
            .lookup_by_key("name", |field, key| field.key_compare_with_value(key))
            .unwrap()
            .optional(),
        false
    );
}

#[test]
fn test_schema_correct_child_table_index() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let root_table = schema.root_table().unwrap();

    let fields = root_table.fields();

    let pos_field_option =
        fields.lookup_by_key("pos", |field, key| field.key_compare_with_value(key));
    assert!(pos_field_option.is_some());
    let pos_field = pos_field_option.unwrap();
    assert_eq!(pos_field.type_().base_type(), BaseType::Obj);
    let pos_table = schema.objects().get(pos_field.type_().index() as usize);
    assert_eq!(pos_table.name(), "MyGame.Example.Vec3");
}

fn load_file_as_buffer(path: &str) -> Vec<u8> {
    std::fs::read(path).unwrap()
}
