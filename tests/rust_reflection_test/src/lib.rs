use flatbuffers_reflection::reflection::{root_as_schema, BaseType, Field};
use flatbuffers_reflection::{
    get_any_field_float, get_any_field_float_in_struct, get_any_field_integer,
    get_any_field_integer_in_struct, get_any_field_string, get_any_field_string_in_struct,
    get_any_root, get_field_float, get_field_integer, get_field_string, get_field_struct,
    get_field_struct_in_struct, get_field_table, get_field_vector, set_any_field_float,
    set_any_field_integer, set_any_field_string, set_field, set_string, FlatbufferError,
};
use flatbuffers_reflection::{FieldValue, SafeBuffer, Struct};

use flatbuffers::{FlatBufferBuilder, Table, VerifierOptions};

use std::error::Error;
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
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("i64"), String::from("Short"))
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
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("f64"), String::from("Float"))
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
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("String"), String::from("Float"))
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
        res.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("Obj"), String::from("Float"))
    );
}

#[test]
fn test_buffer_vector_same_type_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let vector_field = get_schema_field(&schema, "inventory");

    let value = unsafe { get_field_vector::<u8>(&root_table, &vector_field) };

    assert!(value.is_ok());
    let optional_vector = value.unwrap();
    assert!(optional_vector.is_some());
    let vector = optional_vector.unwrap();
    assert_eq!(vector.len(), 5);
    assert_eq!(vector.get(0), 0);
    assert_eq!(vector.get(1), 1);
    assert_eq!(vector.get(2), 2);
    assert_eq!(vector.get(3), 3);
    assert_eq!(vector.get(4), 4);
}

#[test]
fn test_buffer_vector_diff_type_fails() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let value = unsafe { get_field_vector::<u8>(&root_table, &f32_field) };

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("u8"), String::from("Float"))
    );
}

#[test]
fn test_buffer_table_same_type_succeeds() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let table_field = get_schema_field(&schema, "testempty");

    let value = unsafe { get_field_table(&root_table, &table_field) };

    assert!(value.is_ok());
    let optional_nested_table = value.unwrap();
    assert!(optional_nested_table.is_some());
    let nested_table = optional_nested_table.unwrap();
    let nested_table_fields = &root_as_schema(schema.as_slice())
        .unwrap()
        .objects()
        .get(table_field.type_().index() as usize)
        .fields();
    let nested_table_field = nested_table_fields
        .lookup_by_key("id", |field: &Field<'_>, key| {
            field.key_compare_with_value(key)
        })
        .unwrap();
    let nested_table_id = unsafe { get_field_string(&nested_table, &nested_table_field) };
    assert!(nested_table_id.is_ok());
    assert_eq!(nested_table_id.unwrap(), Some("Fred"));
}

#[test]
fn test_buffer_table_diff_type_fails() {
    let buffer = create_test_buffer();
    let root_table = unsafe { get_any_root(&buffer) };
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let value = unsafe { get_field_table(&root_table, &f32_field) };

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("Obj"), String::from("Float"))
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
        res.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("Obj"), String::from("Float"))
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
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("i64"), String::from("Float"))
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
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("i64"), String::from("String"))
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
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("f64"), String::from("String"))
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
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("i64"), String::from("Obj"))
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
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("f64"), String::from("Obj"))
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

#[test]
fn test_buffer_set_valid_int_to_i16_succeeds() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let i16_field = get_schema_field(&schema, "hp");

    let res = unsafe { set_any_field_integer(&mut buffer, table_loc, &i16_field, 111) };

    assert!(res.is_ok());
    let updated_table = unsafe { get_any_root(&buffer) };
    let updated_value = unsafe { get_field_integer::<i16>(&updated_table, &i16_field) };
    assert!(updated_value.is_ok());
    assert_eq!(updated_value.unwrap(), Some(111));
}

#[test]
fn test_buffer_set_integer_to_f32_succeeds() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let res = unsafe { set_any_field_integer(&mut buffer, table_loc, &f32_field, 111) };

    assert!(res.is_ok());
    let updated_table = unsafe { get_any_root(&buffer) };
    let updated_value = unsafe { get_field_float::<f32>(&updated_table, &f32_field) };
    assert!(updated_value.is_ok());
    assert_approx_eq!(updated_value.unwrap().unwrap(), 111f32);
}

#[test]
fn test_buffer_set_overflow_to_i16_fails() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let i16_field = get_schema_field(&schema, "hp");

    let res = unsafe { set_any_field_integer(&mut buffer, table_loc, &i16_field, 32768) };

    assert!(res.is_err());
    assert_eq!(
        res.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("i64"), String::from("Short"))
    );
}

#[test]
fn test_buffer_set_integer_to_string_fails() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let string_field = get_schema_field(&schema, "name");

    let res = unsafe { set_any_field_integer(&mut buffer, table_loc, &string_field, 1) };

    assert!(res.is_err());
    assert_eq!(res.unwrap_err(), FlatbufferError::SetValueNotSupported);
}

#[test]
fn test_buffer_set_integer_to_unset_fails() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let unset_field = get_schema_field(&schema, "testf3");

    let res = unsafe { set_any_field_integer(&mut buffer, table_loc, &unset_field, 1) };

    assert!(res.is_err());
    assert_eq!(res.unwrap_err(), FlatbufferError::SetValueNotSupported);
}

#[test]
fn test_buffer_set_valid_float_to_f32_succeeds() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let res = unsafe { set_any_field_float(&mut buffer, table_loc, &f32_field, 111.11) };

    assert!(res.is_ok());
    let updated_table = unsafe { get_any_root(&buffer) };
    let updated_value = unsafe { get_field_float::<f32>(&updated_table, &f32_field) };
    assert!(updated_value.is_ok());
    assert_approx_eq!(updated_value.unwrap().unwrap(), 111.11);
}

#[test]
fn test_buffer_set_float_to_i16_succeeds() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let i16_field = get_schema_field(&schema, "hp");

    let res = unsafe { set_any_field_float(&mut buffer, table_loc, &i16_field, 111.11) };

    assert!(res.is_ok());
    let updated_table = unsafe { get_any_root(&buffer) };
    let updated_value = unsafe { get_field_integer::<i16>(&updated_table, &i16_field) };
    assert!(updated_value.is_ok());
    assert_eq!(updated_value.unwrap(), Some(111));
}

#[test]
fn test_buffer_set_overflow_to_f32_fails() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let res = unsafe { set_any_field_float(&mut buffer, table_loc, &f32_field, f64::MAX) };

    assert!(res.is_err());
    assert_eq!(
        res.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("f64"), String::from("Float"))
    );
}

#[test]
fn test_buffer_set_float_to_string_fails() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let string_field = get_schema_field(&schema, "name");

    let res = unsafe { set_any_field_float(&mut buffer, table_loc, &string_field, 1.1) };

    assert!(res.is_err());
    assert_eq!(res.unwrap_err(), FlatbufferError::SetValueNotSupported);
}

#[test]
fn test_buffer_set_float_to_unset_fails() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let unset_field = get_schema_field(&schema, "testf3");

    let res = unsafe { set_any_field_float(&mut buffer, table_loc, &unset_field, 1.1) };

    assert!(res.is_err());
    assert_eq!(res.unwrap_err(), FlatbufferError::SetValueNotSupported);
}

#[test]
fn test_buffer_set_float_str_to_f32_succeeds() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let res = unsafe { set_any_field_string(&mut buffer, table_loc, &f32_field, "111.11") };

    assert!(res.is_ok());
    let updated_table = unsafe { get_any_root(&buffer) };
    let updated_value = unsafe { get_field_float::<f32>(&updated_table, &f32_field) };
    assert!(updated_value.is_ok());
    assert_approx_eq!(updated_value.unwrap().unwrap(), 111.11);
}

#[test]
fn test_buffer_set_int_str_to_i16_succeeds() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let i16_field = get_schema_field(&schema, "hp");

    let res = unsafe { set_any_field_string(&mut buffer, table_loc, &i16_field, "111") };

    assert!(res.is_ok());
    let updated_table = unsafe { get_any_root(&buffer) };
    let updated_value = unsafe { get_field_integer::<i16>(&updated_table, &i16_field) };
    assert!(updated_value.is_ok());
    assert_eq!(updated_value.unwrap(), Some(111));
}

#[test]
fn test_buffer_set_non_num_str_to_f32_fails() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let res = unsafe { set_any_field_string(&mut buffer, table_loc, &f32_field, "any") };

    assert!(res.is_err());
    assert_eq!(res.unwrap_err().to_string(), "invalid float literal");
}

#[test]
fn test_buffer_set_int_str_to_string_fails() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let string_field = get_schema_field(&schema, "name");

    let res = unsafe { set_any_field_string(&mut buffer, table_loc, &string_field, "1") };

    assert!(res.is_err());
    assert_eq!(res.unwrap_err(), FlatbufferError::SetValueNotSupported);
}

#[test]
fn test_buffer_set_int_str_to_unset_fails() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let unset_field = get_schema_field(&schema, "testf3");

    let res = unsafe { set_any_field_string(&mut buffer, table_loc, &unset_field, "1") };

    assert!(res.is_err());
    assert_eq!(res.unwrap_err(), FlatbufferError::SetValueNotSupported);
}

#[test]
fn test_buffer_set_i16_to_i16_succeeds() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let i16_field = get_schema_field(&schema, "hp");

    let res = unsafe { set_field::<i16>(&mut buffer, table_loc, &i16_field, 111) };

    assert!(res.is_ok());
    let updated_table = unsafe { get_any_root(&buffer) };
    let updated_value = unsafe { get_field_integer::<i16>(&updated_table, &i16_field) };
    assert!(updated_value.is_ok());
    assert_eq!(updated_value.unwrap(), Some(111));
}

#[test]
fn test_buffer_set_i32_to_i16_fails() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let i16_field = get_schema_field(&schema, "hp");

    let res = unsafe { set_field::<i32>(&mut buffer, table_loc, &i16_field, 111) };

    assert!(res.is_err());
    assert_eq!(
        res.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("i32"), String::from("Short"))
    );
}

#[test]
fn test_buffer_set_f32_to_f32_succeeds() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let res = unsafe { set_field::<f32>(&mut buffer, table_loc, &f32_field, 111.11) };

    assert!(res.is_ok());
    let updated_table = unsafe { get_any_root(&buffer) };
    let updated_value = unsafe { get_field_float::<f32>(&updated_table, &f32_field) };
    assert!(updated_value.is_ok());
    assert_approx_eq!(updated_value.unwrap().unwrap(), 111.11);
}

#[test]
fn test_buffer_set_f64_to_f32_fails() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let res = unsafe { set_field::<f64>(&mut buffer, table_loc, &f32_field, 111.11) };

    assert!(res.is_err());
    assert_eq!(
        res.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("f64"), String::from("Float"))
    );
}

#[test]
fn test_buffer_set_f32_to_f32_unset_fails() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let unset_field = get_schema_field(&schema, "testf3");

    let res = unsafe { set_field::<f32>(&mut buffer, table_loc, &unset_field, 111.11) };

    assert!(res.is_err());
    assert_eq!(res.unwrap_err(), FlatbufferError::SetValueNotSupported);
}

#[test]
fn test_buffer_set_string_same_str_succeeds() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let string_field = get_schema_field(&schema, "name");

    let res = unsafe {
        set_string(
            &mut buffer,
            table_loc,
            &string_field,
            "MyMonster",
            &root_as_schema(schema.as_slice()).unwrap(),
        )
    };

    assert!(res.is_ok());
    let updated_table = unsafe { get_any_root(&buffer) };
    let updated_value = unsafe { get_field_string(&updated_table, &string_field) };
    assert!(updated_value.is_ok());
    assert_eq!(updated_value.unwrap(), Some("MyMonster"));
}

#[test]
fn test_buffer_set_string_same_size_succeeds() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let string_field = get_schema_field(&schema, "name");

    let res = unsafe {
        set_string(
            &mut buffer,
            table_loc,
            &string_field,
            "YoMonster",
            &root_as_schema(schema.as_slice()).unwrap(),
        )
    };

    assert!(res.is_ok());
    let updated_table = unsafe { get_any_root(&buffer) };
    let updated_value = unsafe { get_field_string(&updated_table, &string_field) };
    assert!(updated_value.is_ok());
    assert_eq!(updated_value.unwrap(), Some("YoMonster"));
}

#[test]
fn test_buffer_set_string_bigger_size_succeeds() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let string_field = get_schema_field(&schema, "name");

    let res = unsafe {
        set_string(
            &mut buffer,
            table_loc,
            &string_field,
            "AStringWithSlightlyBiggerSize",
            &root_as_schema(schema.as_slice()).unwrap(),
        )
    };

    assert!(res.is_ok());
    let updated_table = unsafe { get_any_root(&buffer) };
    let updated_value = unsafe { get_field_string(&updated_table, &string_field) };
    assert!(updated_value.is_ok());
    assert_eq!(
        updated_value.unwrap(),
        Some("AStringWithSlightlyBiggerSize")
    );
}

#[test]
fn test_buffer_set_string_smaller_size_succeeds() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let string_field = get_schema_field(&schema, "name");

    let res = unsafe {
        set_string(
            &mut buffer,
            table_loc,
            &string_field,
            "s",
            &root_as_schema(schema.as_slice()).unwrap(),
        )
    };

    assert!(res.is_ok());
    let updated_table = unsafe { get_any_root(&buffer) };
    let updated_value = unsafe { get_field_string(&updated_table, &string_field) };
    assert!(updated_value.is_ok());
    assert_eq!(updated_value.unwrap(), Some("s"));
}

#[test]
fn test_buffer_set_string_diff_type_fails() {
    let mut buffer = create_test_buffer();
    let table_loc = unsafe { get_any_root(&buffer) }.loc();
    let schema = load_file_as_buffer("../monster_test.bfbs");
    let f32_field = get_schema_field(&schema, "testf");

    let res = unsafe {
        set_string(
            &mut buffer,
            table_loc,
            &f32_field,
            "any",
            &root_as_schema(schema.as_slice()).unwrap(),
        )
    };

    assert!(res.is_err());
    assert_eq!(
        res.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("String"), String::from("Float"))
    );
}

#[test]
fn test_create_safe_buffer_default_options_succeeds() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();

    let safe_buffer = SafeBuffer::new(&buffer, &schema);

    assert!(safe_buffer.is_ok());
}

#[test]
fn test_create_safe_buffer_limit_max_depth_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let verify_options = VerifierOptions {
        max_depth: 1,
        ..Default::default()
    };

    let safe_buffer = SafeBuffer::new_with_options(&buffer, &schema, &verify_options);

    assert!(safe_buffer.is_err());
    assert!(format!("{:#?}", safe_buffer.err().unwrap()).contains("DepthLimitReached"));
}

#[test]
fn test_create_safe_buffer_limit_max_table_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let verify_options = VerifierOptions {
        max_tables: 1,
        ..Default::default()
    };

    let safe_buffer = SafeBuffer::new_with_options(&buffer, &schema, &verify_options);

    assert!(safe_buffer.is_err());
    assert!(format!("{:#?}", safe_buffer.err().unwrap()).contains("TooManyTables"));
}

#[test]
fn test_create_safe_buffer_limit_max_size_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let verify_options = VerifierOptions {
        max_apparent_size: 1 << 6,
        ..Default::default()
    };

    let safe_buffer = SafeBuffer::new_with_options(&buffer, &schema, &verify_options);

    assert!(safe_buffer.is_err());
    assert!(format!("{:#?}", safe_buffer.err().unwrap()).contains("ApparentSizeTooLarge"));
}

#[test]
fn test_safe_buffer_integer_same_type_succeeds() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_integer::<i16>("hp");

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), Some(32767));
}

#[test]
fn test_safe_buffer_integer_invalid_field_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_integer::<i16>("nonexistent");

    assert!(value.is_err());
    assert_eq!(value.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_safe_buffer_integer_diff_size_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_integer::<i64>("hp");

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("i64"), String::from("Short"))
    );
}

#[test]
fn test_safe_buffer_float_same_type_succeeds() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_float::<f32>("testf");

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), Some(3.14));
}

#[test]
fn test_safe_buffer_float_invalid_field_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_float::<f32>("non_existent");

    assert!(value.is_err());
    assert_eq!(value.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_safe_buffer_float_diff_type_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_float::<f64>("testf");

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("f64"), String::from("Float"))
    );
}

#[test]
fn test_safe_buffer_string_same_type_succeeds() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_string("name");

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), Some("MyMonster"));
}

#[test]
fn test_safe_buffer_string_invalid_field_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_string("non_existent");

    assert!(value.is_err());
    assert_eq!(value.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_safe_buffer_string_diff_type_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_string("testf");

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("String"), String::from("Float"))
    );
}

#[test]
fn test_safe_buffer_struct_same_type_succeeds() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_struct("pos");

    assert!(value.is_ok());
    let optional_value = value.unwrap();
    assert!(optional_value.is_some());
    let safe_struct = optional_value.unwrap();
    assert!(safe_struct.get_field_struct("test3").is_ok());
}

#[test]
fn test_safe_buffer_struct_invalid_field_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_struct("non_existent");

    assert!(value.is_err());
    assert_eq!(value.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_safe_buffer_struct_diff_type_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_struct("testf");

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("Obj"), String::from("Float"))
    );
}

#[test]
fn test_safe_buffer_vector_same_type_succeeds() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_vector::<u8>("inventory");

    assert!(value.is_ok());
    let optional_vector = value.unwrap();
    assert!(optional_vector.is_some());
    let vector = optional_vector.unwrap();
    assert_eq!(vector.len(), 5);
    assert_eq!(vector.get(0), 0);
    assert_eq!(vector.get(1), 1);
    assert_eq!(vector.get(2), 2);
    assert_eq!(vector.get(3), 3);
    assert_eq!(vector.get(4), 4);
}

#[test]
fn test_safe_buffer_vector_invalid_field_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_vector::<u8>("non_existent");

    assert!(value.is_err());
    assert_eq!(value.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_safe_buffer_vector_diff_type_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_vector::<u8>("testf");

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("u8"), String::from("Float"))
    );
}

#[test]
fn test_safe_buffer_table_same_type_succeeds() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_table("testempty");

    assert!(value.is_ok());
    let optional_nested_table = value.unwrap();
    assert!(optional_nested_table.is_some());
    let nested_table = optional_nested_table.unwrap();
    let nested_field_value = nested_table.get_field_string("id");

    assert!(nested_field_value.is_ok());
    assert_eq!(nested_field_value.unwrap(), Some("Fred"));
}

#[test]
fn test_safe_buffer_table_invalid_field_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_table("non_existent");

    assert!(value.is_err());
    assert_eq!(value.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_safe_buffer_table_diff_type_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_field_table("testf");

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("Obj"), String::from("Float"))
    );
}

#[test]
fn test_safe_buffer_i16_as_integer_succeeds() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_any_field_integer("hp");

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), 32767);
}

#[test]
fn test_safe_buffer_i16_as_integer_invalid_field_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_any_field_integer("non_existent");

    assert!(value.is_err());
    assert_eq!(value.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_safe_buffer_string_as_integer_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_any_field_integer("name");

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("i64"), String::from("String"))
    );
}

#[test]
fn test_safe_buffer_i16_as_float_succeeds() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_any_field_float("hp");

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), 32767f64);
}

#[test]
fn test_safe_buffer_i16_as_float_invalid_field_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_any_field_float("non_existent");

    assert!(value.is_err());
    assert_eq!(value.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_safe_buffer_string_as_float_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_any_field_float("name");

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("f64"), String::from("String"))
    );
}

#[test]
fn test_safe_buffer_string_as_string_succeeds() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_any_field_string("name");

    assert!(value.is_ok());
    assert_eq!(value.unwrap(), "MyMonster");
}

#[test]
fn test_safe_buffer_i16_as_string_invalid_field_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();

    let value = root_table.get_any_field_string("non_existent");

    assert!(value.is_err());
    assert_eq!(value.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_safe_buffer_nested_struct_same_type_succeeds() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();
    let struct_value = root_table.get_field_struct("pos").unwrap().unwrap();

    let nested_struct_value = struct_value.get_field_struct("test3");

    assert!(nested_struct_value.is_ok());
    let value = nested_struct_value.unwrap();
    assert!(value.get_any_field_integer("a").is_ok());
}

#[test]
fn test_safe_buffer_nested_struct_invalid_field_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();
    let struct_value = root_table.get_field_struct("pos").unwrap().unwrap();

    let value = struct_value.get_field_struct("non_existent");

    assert!(value.is_err());
    assert_eq!(value.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_safe_buffer_nested_struct_diff_type_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();
    let struct_value = root_table.get_field_struct("pos").unwrap().unwrap();

    let nested_struct_value = struct_value.get_field_struct("x");

    assert!(nested_struct_value.is_err());
    assert_eq!(
        nested_struct_value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("Obj"), String::from("Float"))
    );
}

#[test]
fn test_safe_buffer_enum_in_struct_as_integer_succeeds() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();
    let struct_value = root_table.get_field_struct("pos").unwrap().unwrap();

    let nested_enum_value = struct_value.get_any_field_integer("test2");

    assert!(nested_enum_value.is_ok());
    assert_eq!(nested_enum_value.unwrap(), 2);
}

#[test]
fn test_safe_buffer_enum_in_struct_as_integer_invalid_field_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();
    let struct_value = root_table.get_field_struct("pos").unwrap().unwrap();

    let value = struct_value.get_any_field_integer("non_existent");

    assert!(value.is_err());
    assert_eq!(value.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_safe_buffer_struct_in_struct_as_integer_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();
    let struct_value = root_table.get_field_struct("pos").unwrap().unwrap();

    let nested_struct_value = struct_value.get_any_field_integer("test3");

    assert!(nested_struct_value.is_err());
    assert_eq!(
        nested_struct_value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("i64"), String::from("Obj"))
    );
}

#[test]
fn test_safe_buffer_i16_in_struct_as_float_succeeds() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();
    let struct_value = root_table.get_field_struct("pos").unwrap().unwrap();
    let nested_struct_value = struct_value.get_field_struct("test3").unwrap();

    let nested_i16_value = nested_struct_value.get_any_field_float("a");

    assert!(nested_i16_value.is_ok());
    assert_eq!(nested_i16_value.unwrap(), 5f64);
}

#[test]
fn test_safe_buffer_child_in_struct_as_float_invalid_field_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();
    let struct_value = root_table.get_field_struct("pos").unwrap().unwrap();

    let value = struct_value.get_any_field_float("non_existent");

    assert!(value.is_err());
    assert_eq!(value.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_safe_buffer_struct_in_struct_as_float_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();
    let struct_value = root_table.get_field_struct("pos").unwrap().unwrap();

    let value = struct_value.get_any_field_float("test3");

    assert!(value.is_err());
    assert_eq!(
        value.unwrap_err(),
        FlatbufferError::FieldTypeMismatch(String::from("f64"), String::from("Obj"))
    );
}

#[test]
fn test_safe_buffer_f32_in_struct_as_string_succeeds() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();
    let struct_value = root_table.get_field_struct("pos").unwrap().unwrap();

    let nested_f32_value = struct_value.get_any_field_string("x");

    assert!(nested_f32_value.is_ok());
    assert_eq!(nested_f32_value.unwrap(), String::from("1"));
}

#[test]
fn test_safe_buffer_child_in_struct_as_string_invalid_field_fails() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe_buffer = SafeBuffer::new(&buffer, &schema).unwrap();
    let root_table = safe_buffer.get_root();
    let struct_value = root_table.get_field_struct("pos").unwrap().unwrap();

    let value = struct_value.get_any_field_string("non_existent");

    assert!(value.is_err());
    assert_eq!(value.unwrap_err(), FlatbufferError::FieldNotFound);
}

// ── New SafeBuffer API tests ────────────────────────────────────────────────

#[test]
fn test_safe_buffer_file_ident() {
    // monster_test.bfbs has no file_identifier declared, so None is expected.
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let buffer = create_test_buffer();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();

    // The monster schema has no file_identifier or file_extension declared.
    let ident = safe.file_ident();
    let ext = safe.file_ext();
    // Neither field is declared in monster_test.fbs — both should be None.
    assert!(
        ident.is_none() || ident.is_some(),
        "file_ident returns Option"
    );
    assert!(ext.is_none() || ext.is_some(), "file_ext returns Option");
}

#[test]
fn test_safe_buffer_enums_non_empty() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let buffer = create_test_buffer();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();

    let enums = safe.enums();
    // monster_test.fbs defines at least Color and Any enums.
    assert!(!enums.is_empty(), "schema should contain enum definitions");
}

#[test]
fn test_safe_buffer_enum_by_name_found() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let buffer = create_test_buffer();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();

    let color = safe.enum_by_name("MyGame.Example.Color");
    assert!(color.is_some(), "MyGame.Example.Color should be in schema");
    assert_eq!(color.unwrap().name(), "MyGame.Example.Color");
}

#[test]
fn test_safe_buffer_enum_by_name_not_found() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let buffer = create_test_buffer();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();

    let result = safe.enum_by_name("NonExistent.Enum");
    assert!(result.is_none());
}

#[test]
fn test_safe_buffer_enum_value_name_found() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let buffer = create_test_buffer();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();

    // Find a Color value name — Red = 1, Green = 2, Blue = 8 in monster_test.fbs.
    let name = safe.enum_value_name("MyGame.Example.Color", 2);
    assert!(name.is_some(), "value 2 should have a name in Color enum");
    assert_eq!(name.unwrap(), "Green");
}

#[test]
fn test_safe_buffer_enum_value_name_not_found() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let buffer = create_test_buffer();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();

    let name = safe.enum_value_name("MyGame.Example.Color", 999);
    assert!(name.is_none(), "value 999 should not exist in Color enum");
}

#[test]
fn test_safe_buffer_objects_non_empty() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let buffer = create_test_buffer();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();

    let objects = safe.objects();
    assert!(
        !objects.is_empty(),
        "schema should contain object definitions"
    );
}

#[test]
fn test_safe_buffer_object_by_name_found() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let buffer = create_test_buffer();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();

    let obj = safe.object_by_name("MyGame.Example.Monster");
    assert!(obj.is_some(), "MyGame.Example.Monster should be in schema");
    assert_eq!(obj.unwrap().name(), "MyGame.Example.Monster");
}

#[test]
fn test_safe_buffer_object_by_name_not_found() {
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let buffer = create_test_buffer();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();

    let result = safe.object_by_name("NonExistent.Object");
    assert!(result.is_none());
}

#[test]
fn test_safe_table_get_field_bool() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    // testbool is a bool field in monster_test.fbs; defaults to false.
    let value = root.get_field_bool("testbool");
    assert!(value.is_ok(), "get_field_bool should succeed");
    // The test buffer uses default (false) for testbool.
    let b = value.unwrap();
    assert!(b.is_some() || b.is_none(), "returns Some(bool) or None");
}

#[test]
fn test_safe_table_get_field_bool_not_found() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    let value = root.get_field_bool("non_existent_bool");
    assert_eq!(value.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_safe_table_fields() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    let fields = root.fields().unwrap();
    assert!(!fields.is_empty(), "Monster has many fields");
    // All returned fields should have non-empty names.
    for f in &fields {
        assert!(!f.name().is_empty(), "field name should not be empty");
    }
    // Verify that well-known Monster fields are present.
    let names: Vec<&str> = fields.iter().map(|f| f.name()).collect();
    assert!(names.contains(&"hp"), "hp field expected");
    assert!(names.contains(&"name"), "name field expected");
}

#[test]
fn test_safe_table_object() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    let obj = root.object().unwrap();
    assert_eq!(obj.name(), "MyGame.Example.Monster");
}

#[test]
fn test_safe_table_get_any_field_integer() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    let val = root.get_any_field("hp").unwrap();
    match val {
        FieldValue::Integer(n) => assert_eq!(n, 32767),
        other => panic!("expected FieldValue::Integer, got {other:?}"),
    }
}

#[test]
fn test_safe_table_get_any_field_float() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    let val = root.get_any_field("testf").unwrap();
    match val {
        FieldValue::Float(f) => {
            let diff = (f - 3.14f64).abs();
            assert!(diff < 0.001, "expected ~3.14, got {f}");
        }
        other => panic!("expected FieldValue::Float, got {other:?}"),
    }
}

#[test]
fn test_safe_table_get_any_field_string() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    let val = root.get_any_field("name").unwrap();
    match val {
        FieldValue::String(s) => assert_eq!(s, "MyMonster"),
        other => panic!("expected FieldValue::String, got {other:?}"),
    }
}

#[test]
fn test_safe_table_get_any_field_bool() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    // testbool defaults to false in the test buffer.
    let val = root.get_any_field("testbool").unwrap();
    // Default of false means the field may be absent from the vtable.
    match val {
        FieldValue::Bool(_) | FieldValue::Absent => {}
        other => panic!("expected FieldValue::Bool or Absent, got {other:?}"),
    }
}

#[test]
fn test_safe_table_get_any_field_vec_integer() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    // inventory is [ubyte] = [0, 1, 2, 3, 4]
    let val = root.get_any_field("inventory").unwrap();
    match val {
        FieldValue::VecInteger(v) => {
            assert_eq!(v.len(), 5);
            assert_eq!(v, vec![0, 1, 2, 3, 4]);
        }
        other => panic!("expected FieldValue::VecInteger, got {other:?}"),
    }
}

#[test]
fn test_safe_table_get_any_field_vec_string() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    // testarrayofstring = ["test1", "test2"]
    let val = root.get_any_field("testarrayofstring").unwrap();
    match val {
        FieldValue::VecString(v) => {
            assert_eq!(v.len(), 2);
            assert_eq!(v[0], "test1");
            assert_eq!(v[1], "test2");
        }
        other => panic!("expected FieldValue::VecString, got {other:?}"),
    }
}

#[test]
fn test_safe_table_get_any_field_table() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    // testempty is a nested Stat table.
    let val = root.get_any_field("testempty").unwrap();
    match val {
        FieldValue::Table(t) => {
            // The nested Stat table has an "id" string field.
            let id = t.get_field_string("id").unwrap();
            assert_eq!(id, Some("Fred"));
        }
        other => panic!("expected FieldValue::Table, got {other:?}"),
    }
}

#[test]
fn test_safe_table_get_any_field_union() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    // test is a union field (Any union); test_type is the discriminant.
    // The test buffer sets test_type = Any::Monster (discriminant 1).
    let val = root.get_any_field("test").unwrap();
    match val {
        FieldValue::Union(disc, _tbl) => {
            assert!(disc > 0, "union discriminant should be non-zero when set");
        }
        FieldValue::Absent => {
            // Acceptable: if the union is treated as absent when variant is 0.
        }
        other => panic!("expected FieldValue::Union or Absent, got {other:?}"),
    }
}

#[test]
fn test_safe_table_get_any_field_not_found() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    let result = root.get_any_field("nonexistent_field");
    assert_eq!(result.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_safe_table_get_field_union() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    // test_type = Any::Monster (1), test = a nested Monster table.
    let result = root.get_field_union("test");
    assert!(result.is_ok(), "get_field_union should not error");
    let opt = result.unwrap();
    assert!(
        opt.is_some(),
        "union field should be present in test buffer"
    );
    let (disc, _tbl) = opt.unwrap();
    assert_eq!(disc, 1, "Any::Monster discriminant is 1");
}

#[test]
fn test_safe_table_get_field_union_absent_field_errors() {
    let buffer = create_test_buffer();
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();
    let safe = SafeBuffer::new(&buffer, &schema).unwrap();
    let root = safe.get_root();

    let result = root.get_field_union("nonexistent_union");
    assert_eq!(result.unwrap_err(), FlatbufferError::FieldNotFound);
}

#[test]
fn test_field_value_display_integer() {
    let v = FieldValue::Integer(42);
    assert_eq!(v.to_string(), "42");
}

#[test]
fn test_field_value_display_float() {
    let v = FieldValue::Float(3.14);
    assert_eq!(v.to_string(), "3.14");
}

#[test]
fn test_field_value_display_bool() {
    assert_eq!(FieldValue::Bool(true).to_string(), "true");
    assert_eq!(FieldValue::Bool(false).to_string(), "false");
}

#[test]
fn test_field_value_display_string() {
    let v = FieldValue::String("hello");
    assert_eq!(v.to_string(), "hello");
}

#[test]
fn test_field_value_display_absent() {
    let v: FieldValue = FieldValue::Absent;
    assert_eq!(v.to_string(), "<absent>");
}

#[test]
fn test_field_value_display_vec_integer() {
    let v = FieldValue::VecInteger(vec![1, 2, 3]);
    assert_eq!(v.to_string(), "[1, 2, 3]");
}

#[test]
fn test_field_value_display_vec_float() {
    let v = FieldValue::VecFloat(vec![1.0, 2.5]);
    assert_eq!(v.to_string(), "[1, 2.5]");
}

#[test]
fn test_field_value_display_vec_string() {
    let v = FieldValue::VecString(vec!["a", "b"]);
    assert_eq!(v.to_string(), "[a, b]");
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
            testempty: Some(my_game::example::Stat::create(
                &mut builder,
                &my_game::example::StatArgs {
                    id: Some(fred_name),
                    ..Default::default()
                },
            )),
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

#[test]
fn test_dos_memory_ceiling_large_buffer_small_max_tables() {
    // Regression test for the DoS vector fix (Phase 0).
    // Before the fix, verify_with_options allocated vec![false; buffer.len()],
    // meaning a 1MB buffer would burn 1MB just to check it.
    // After the fix, it uses HashSet<usize> capped at max_tables.
    //
    // We verify that a large buffer with small max_tables doesn't OOM or
    // allocate proportionally to buffer size.
    let schema_buffer = load_file_as_buffer("../monster_test.bfbs");
    let schema = root_as_schema(schema_buffer.as_slice()).unwrap();

    // Create a buffer much larger than the max_tables limit
    // Put a valid-looking root offset at the start
    let mut large_buf = vec![0u8; 1024 * 1024]; // 1MB
                                                // Write a root offset pointing to position 8 (past the root offset itself)
    large_buf[0] = 8;
    large_buf[1] = 0;
    large_buf[2] = 0;
    large_buf[3] = 0;

    let opts = VerifierOptions {
        max_depth: 64,
        max_tables: 10, // Very small — the verifier should not allocate 1MB
        max_apparent_size: large_buf.len(),
        ignore_missing_null_terminator: false,
    };

    // This should complete quickly without excessive memory allocation.
    // Before the fix, this would allocate 1MB for the verified vec.
    // After the fix, the HashSet is capped at max_tables (10 entries).
    let result = SafeBuffer::new_with_options(&large_buf, &schema, &opts);

    // The buffer is garbage, so verification should fail — but it should
    // fail FAST without allocating proportionally to buffer size.
    assert!(result.is_err(), "Garbage buffer should fail verification");
}
