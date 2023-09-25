use flatbuffers_reflection::reflection::{root_as_schema, BaseType, Field};
use flatbuffers_reflection::{
    get_any_field_float, get_any_field_float_in_struct, get_any_field_integer,
    get_any_field_integer_in_struct, get_any_field_string, get_any_field_string_in_struct,
    get_any_root, get_field_float, get_field_integer, get_field_string, get_field_struct,
    get_field_struct_in_struct,
};

use flatbuffers::FlatBufferBuilder;

use std::fs::File;
use std::io::Read;

use assert_approx_eq::assert_approx_eq;

#[allow(dead_code, unused_imports)]
#[path = "../../monster_test/mod.rs"]
mod monster_test_generated;
pub use monster_test_generated::my_game;

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

#[test]
fn test_buffer_integer_same_type_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let i16_field = get_schema_field(&schema, "hp");

    let value = unsafe { get_field_integer::<i16>(&root_table, &i16_field) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), Some(32767));
}

#[test]
fn test_buffer_integer_diff_type_same_size_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let i16_field = get_schema_field(&schema, "hp");

    let value = unsafe { get_field_integer::<u16>(&root_table, &i16_field) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), Some(32767));
}

#[test]
fn test_buffer_integer_diff_size_fails() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let i16_field = get_schema_field(&schema, "hp");

    let value = unsafe { get_field_integer::<i64>(&root_table, &i16_field) };

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err().to_string(),
        "Failed to get data of type i64 from field of type Short"
    );
}

#[test]
fn test_buffer_float_same_type_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let value = unsafe { get_field_float::<f32>(&root_table, &f32_field) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), Some(3.14));
}

#[test]
fn test_buffer_float_diff_type_fails() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let value = unsafe { get_field_float::<f64>(&root_table, &f32_field) };

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err().to_string(),
        "Failed to get data of type f64 from field of type Float"
    );
}

#[test]
fn test_buffer_string_same_type_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let string_field = get_schema_field(&schema, "name");

    let value = unsafe { get_field_string(&root_table, &string_field) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), Some("MyMonster"));
}

#[test]
fn test_buffer_string_diff_type_fails() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let value = unsafe { get_field_string(&root_table, &f32_field) };

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err().to_string(),
        "Failed to get data of type String from field of type Float"
    );
}

#[test]
fn test_buffer_struct_same_type_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");

    let res = unsafe { get_field_struct(&root_table, &struct_field) };

    assert!(res.is_ok());
    let optional_value = res.unwrap();
    assert!(optional_value.is_some());
    assert_eq!(optional_value.unwrap().buf(), &buffer);
    assert!(optional_value.unwrap().loc() > root_table.loc());
}

#[test]
fn test_buffer_struct_diff_type_fails() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let res = unsafe { get_field_struct(&root_table, &f32_field) };

    assert!(res.is_err());
    assert_eq!(
        res.unwrap_err().to_string(),
        "Failed to get data of type Obj from field of type Float"
    );
}

#[test]
fn test_buffer_nested_struct_same_type_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");
    let struct_value = unsafe {
        get_field_struct(&root_table, &struct_field)
            .unwrap()
            .unwrap()
    };
    let struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(struct_field.type_().index() as usize);
    let nested_struct_field = struct_schema
        .fields()
        .lookup_by_key("test3", |field, key| field.key_compare_with_value(key))
        .unwrap();

    let res = unsafe { get_field_struct_in_struct(&struct_value, &nested_struct_field) };

    assert!(res.is_ok());
    let value = res.unwrap();
    assert_eq!(value.buf(), &buffer);
    assert!(value.loc() > struct_value.loc());
}

#[test]
fn test_buffer_nested_struct_diff_type_fails() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");
    let struct_value = unsafe {
        get_field_struct(&root_table, &struct_field)
            .unwrap()
            .unwrap()
    };
    let struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(struct_field.type_().index() as usize);
    let nested_float_field = struct_schema
        .fields()
        .lookup_by_key("x", |field, key| field.key_compare_with_value(key))
        .unwrap();

    let res = unsafe { get_field_struct_in_struct(&struct_value, &nested_float_field) };

    assert!(res.is_err());
    assert_eq!(
        res.unwrap_err().to_string(),
        "Failed to get data of type Obj from field of type Float"
    );
}

#[test]
fn test_buffer_i16_as_integer_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let i16_field = get_schema_field(&schema, "hp");

    let value = unsafe { get_any_field_integer(&root_table, &i16_field) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), 32767);
}

#[test]
fn test_buffer_f32_as_integer_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let value = unsafe { get_any_field_integer(&root_table, &f32_field) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), 3);
}

#[test]
fn test_buffer_inf_as_integer_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let inf_field = get_schema_field(&schema, "inf_default");

    let value = unsafe { get_any_field_integer(&root_table, &inf_field) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), 0);
}

#[test]
fn test_buffer_bool_as_integer_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let bool_field = get_schema_field(&schema, "testbool");

    let value = unsafe { get_any_field_integer(&root_table, &bool_field) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), 0);
}

#[test]
fn test_buffer_nan_as_integer_fails() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let nan_field = get_schema_field(&schema, "nan_default");

    let value = unsafe { get_any_field_integer(&root_table, &nan_field) };

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err().to_string(),
        "Failed to get data of type i64 from field of type Float"
    );
}

#[test]
fn test_buffer_string_as_integer_fails() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let string_field = get_schema_field(&schema, "name");

    let value = unsafe { get_any_field_integer(&root_table, &string_field) };

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err().to_string(),
        "Failed to get data of type i64 from field of type String"
    );
}

#[test]
fn test_buffer_i16_as_float_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let i16_field = get_schema_field(&schema, "hp");

    let value = unsafe { get_any_field_float(&root_table, &i16_field) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), 32767f64);
}

#[test]
fn test_buffer_f32_as_float_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let value = unsafe { get_any_field_float(&root_table, &f32_field) };

    assert!(value.is_ok());
    assert_approx_eq!(value.unwrap(), 3.14);
}

#[test]
fn test_buffer_string_as_float_fails() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let string_field = get_schema_field(&schema, "name");

    let value = unsafe { get_any_field_float(&root_table, &string_field) };

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err().to_string(),
        "Failed to get data of type f64 from field of type String"
    );
}

#[test]
fn test_buffer_i16_as_string_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let i16_field = get_schema_field(&schema, "hp");

    let value = unsafe {
        get_any_field_string(
            &root_table,
            &i16_field,
            &root_as_schema(schema.as_slice()).unwrap(),
        )
    };

    assert_eq!(value, String::from("32767"));
}

#[test]
fn test_buffer_f32_as_string_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let mut value = unsafe {
        get_any_field_string(
            &root_table,
            &f32_field,
            &root_as_schema(schema.as_slice()).unwrap(),
        )
    };

    value.truncate(4);
    assert_eq!(value, String::from("3.14"));
}

#[test]
fn test_buffer_string_as_string_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let string_field = get_schema_field(&schema, "name");

    let value = unsafe {
        get_any_field_string(
            &root_table,
            &string_field,
            &root_as_schema(schema.as_slice()).unwrap(),
        )
    };

    assert_eq!(value, "MyMonster");
}

#[test]
fn test_buffer_i16_in_struct_as_integer_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");
    let struct_value = unsafe {
        get_field_struct(&root_table, &struct_field)
            .unwrap()
            .unwrap()
    };
    let struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(struct_field.type_().index() as usize);
    let nested_struct_field = struct_schema
        .fields()
        .lookup_by_key("test3", |field, key| field.key_compare_with_value(key))
        .unwrap();
    let nested_struct_value =
        unsafe { get_field_struct_in_struct(&struct_value, &nested_struct_field).unwrap() };
    let nested_struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(nested_struct_field.type_().index() as usize);
    let i16_in_struct = nested_struct_schema
        .fields()
        .lookup_by_key("a", |field, key| field.key_compare_with_value(key))
        .unwrap();

    let value = unsafe { get_any_field_integer_in_struct(&nested_struct_value, &i16_in_struct) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), 5);
}

#[test]
fn test_buffer_f32_in_struct_as_integer_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");
    let struct_value = unsafe {
        get_field_struct(&root_table, &struct_field)
            .unwrap()
            .unwrap()
    };
    let struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(struct_field.type_().index() as usize);
    let f32_in_struct = struct_schema
        .fields()
        .lookup_by_key("x", |field, key| field.key_compare_with_value(key))
        .unwrap();

    let value = unsafe { get_any_field_integer_in_struct(&struct_value, &f32_in_struct) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), 1);
}

#[test]
fn test_buffer_enum_in_struct_as_integer_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");
    let struct_value = unsafe {
        get_field_struct(&root_table, &struct_field)
            .unwrap()
            .unwrap()
    };
    let struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(struct_field.type_().index() as usize);
    let enum_in_struct = struct_schema
        .fields()
        .lookup_by_key("test2", |field, key| field.key_compare_with_value(key))
        .unwrap();

    let value = unsafe { get_any_field_integer_in_struct(&struct_value, &enum_in_struct) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), 2);
}

#[test]
fn test_buffer_struct_in_struct_as_integer_fails() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");
    let struct_value = unsafe {
        get_field_struct(&root_table, &struct_field)
            .unwrap()
            .unwrap()
    };
    let struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(struct_field.type_().index() as usize);
    let struct_in_struct = struct_schema
        .fields()
        .lookup_by_key("test3", |field, key| field.key_compare_with_value(key))
        .unwrap();

    let value = unsafe { get_any_field_integer_in_struct(&struct_value, &struct_in_struct) };

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err().to_string(),
        "Failed to get data of type i64 from field of type Obj"
    );
}

#[test]
fn test_buffer_i16_in_struct_as_float_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");
    let struct_value = unsafe {
        get_field_struct(&root_table, &struct_field)
            .unwrap()
            .unwrap()
    };
    let struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(struct_field.type_().index() as usize);
    let nested_struct_field = struct_schema
        .fields()
        .lookup_by_key("test3", |field, key| field.key_compare_with_value(key))
        .unwrap();
    let nested_struct_value =
        unsafe { get_field_struct_in_struct(&struct_value, &nested_struct_field).unwrap() };
    let nested_struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(nested_struct_field.type_().index() as usize);
    let i16_in_struct = nested_struct_schema
        .fields()
        .lookup_by_key("a", |field, key| field.key_compare_with_value(key))
        .unwrap();

    let value = unsafe { get_any_field_float_in_struct(&nested_struct_value, &i16_in_struct) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), 5f64);
}

#[test]
fn test_buffer_f32_in_struct_as_float_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");
    let struct_value = unsafe {
        get_field_struct(&root_table, &struct_field)
            .unwrap()
            .unwrap()
    };
    let struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(struct_field.type_().index() as usize);
    let f32_in_struct = struct_schema
        .fields()
        .lookup_by_key("x", |field, key| field.key_compare_with_value(key))
        .unwrap();

    let value = unsafe { get_any_field_float_in_struct(&struct_value, &f32_in_struct) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), 1f64);
}

#[test]
fn test_buffer_enum_in_struct_as_float_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");
    let struct_value = unsafe {
        get_field_struct(&root_table, &struct_field)
            .unwrap()
            .unwrap()
    };
    let struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(struct_field.type_().index() as usize);
    let enum_in_struct = struct_schema
        .fields()
        .lookup_by_key("test2", |field, key| field.key_compare_with_value(key))
        .unwrap();

    let value = unsafe { get_any_field_float_in_struct(&struct_value, &enum_in_struct) };

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), 2f64);
}

#[test]
fn test_buffer_struct_in_struct_as_float_fails() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");
    let struct_value = unsafe {
        get_field_struct(&root_table, &struct_field)
            .unwrap()
            .unwrap()
    };
    let struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(struct_field.type_().index() as usize);
    let struct_in_struct = struct_schema
        .fields()
        .lookup_by_key("test3", |field, key| field.key_compare_with_value(key))
        .unwrap();

    let value = unsafe { get_any_field_float_in_struct(&struct_value, &struct_in_struct) };

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err().to_string(),
        "Failed to get data of type f64 from field of type Obj"
    );
}

#[test]
fn test_buffer_i16_in_struct_as_string_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");
    let struct_value = unsafe {
        get_field_struct(&root_table, &struct_field)
            .unwrap()
            .unwrap()
    };
    let struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(struct_field.type_().index() as usize);
    let nested_struct_field = struct_schema
        .fields()
        .lookup_by_key("test3", |field, key| field.key_compare_with_value(key))
        .unwrap();
    let nested_struct_value =
        unsafe { get_field_struct_in_struct(&struct_value, &nested_struct_field).unwrap() };
    let nested_struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(nested_struct_field.type_().index() as usize);
    let i16_in_struct = nested_struct_schema
        .fields()
        .lookup_by_key("a", |field, key| field.key_compare_with_value(key))
        .unwrap();

    let value = unsafe {
        get_any_field_string_in_struct(
            &nested_struct_value,
            &i16_in_struct,
            &root_as_schema(schema.as_slice()).unwrap(),
        )
    };

    assert_eq!(value, String::from("5"));
}

#[test]
fn test_buffer_f32_in_struct_as_string_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");
    let struct_value = unsafe {
        get_field_struct(&root_table, &struct_field)
            .unwrap()
            .unwrap()
    };
    let struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(struct_field.type_().index() as usize);
    let f32_in_struct = struct_schema
        .fields()
        .lookup_by_key("x", |field, key| field.key_compare_with_value(key))
        .unwrap();

    let value = unsafe {
        get_any_field_string_in_struct(
            &struct_value,
            &f32_in_struct,
            &root_as_schema(schema.as_slice()).unwrap(),
        )
    };

    assert_eq!(value, String::from("1"));
}

#[test]
fn test_buffer_enum_in_struct_as_string_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");
    let struct_value = unsafe {
        get_field_struct(&root_table, &struct_field)
            .unwrap()
            .unwrap()
    };
    let struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(struct_field.type_().index() as usize);
    let enum_in_struct = struct_schema
        .fields()
        .lookup_by_key("test2", |field, key| field.key_compare_with_value(key))
        .unwrap();

    let value = unsafe {
        get_any_field_string_in_struct(
            &struct_value,
            &enum_in_struct,
            &root_as_schema(schema.as_slice()).unwrap(),
        )
    };

    assert_eq!(value, String::from("2"));
}

#[test]
fn test_buffer_struct_in_struct_as_string_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let struct_field = get_schema_field(&schema, "pos");
    let struct_value = unsafe {
        get_field_struct(&root_table, &struct_field)
            .unwrap()
            .unwrap()
    };
    let struct_schema = root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(struct_field.type_().index() as usize);
    let struct_in_struct = struct_schema
        .fields()
        .lookup_by_key("test3", |field, key| field.key_compare_with_value(key))
        .unwrap();

    let value = unsafe {
        get_any_field_string_in_struct(
            &struct_value,
            &struct_in_struct,
            &root_as_schema(schema.as_slice()).unwrap(),
        )
    };

    assert_eq!(value, String::from("MyGame.Example.Test { a: 5, b: 6, }"));
}

fn load_file_as_buffer(path: &str) -> Vec<u8> {
    std::fs::read(path).unwrap()
}

fn get_schema_field<'a>(schema_buffer: &'a Vec<u8>, field_name: &'a str) -> Field<'a> {
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let root_table = schema.root_table().unwrap();
    let fields = root_table.fields();
    fields
        .lookup_by_key(field_name, |field, key| field.key_compare_with_value(key))
        .unwrap()
}

fn create_test_buffer() -> Vec<u8> {
    let mut builder = FlatBufferBuilder::new();
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
            hp: 32767,
            testf: 3.14,
            mana: 150,
            name: Some(builder.create_string("MyMonster")),
            pos: Some(&pos),
            test_type: my_game::example::Any::Monster,
            test: Some(
                my_game::example::Monster::create(
                    &mut builder,
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
        my_game::example::Monster::create(&mut builder, &args)
    };
    my_game::example::finish_monster_buffer(&mut builder, mon);
    let (flatbuf, start_loc) = builder.mut_finished_buffer();
    flatbuf[start_loc..].to_vec()
}
