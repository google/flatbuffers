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

mod reflection_generated;
mod r#struct;
pub use crate::r#struct::Struct;
pub use crate::reflection_generated::reflection;

use flatbuffers::{Follow, ForwardsUOffset, Table};
use reflection_generated::reflection::{BaseType, Field};

use core::mem::size_of;
use num::traits::float::Float;
use num::traits::int::PrimInt;
use num::traits::FromPrimitive;
use thiserror::Error;

#[derive(Error, Debug)]
pub enum FlatbufferError {
    #[error("Failed to get data of type {0} from field of type {1}")]
    FieldTypeMismatch(String, String),
}

pub type FlatbufferResult<T, E = FlatbufferError> = core::result::Result<T, E>;

/// Gets the root table from a trusted Flatbuffer.
///
/// # Safety
///
/// Flatbuffers accessors do not perform validation checks before accessing. Users
/// must trust [data] contains a valid flatbuffer. Reading unchecked buffers may cause panics or even UB.
pub unsafe fn get_any_root(data: &[u8]) -> Table {
    <ForwardsUOffset<Table>>::follow(data, 0)
}

/// Gets an integer table field given its exact type. Returns default integer value if the field is not set. Returns [None] if no default value is found. Returns error if the type size doesn't match.
///
/// # Safety
///
/// The value of the corresponding slot must have type T
pub unsafe fn get_field_integer<'a, T: Follow<'a, Inner = T> + PrimInt + FromPrimitive + 'a>(
    table: &'a Table,
    field: &Field,
) -> FlatbufferResult<Option<T>> {
    if size_of::<T>() != get_type_size(field.type_().base_type()) {
        return Err(FlatbufferError::FieldTypeMismatch(
            std::any::type_name::<T>().to_string(),
            field
                .type_()
                .base_type()
                .variant_name()
                .unwrap_or_default()
                .to_string(),
        ));
    }

    let default = T::from_i64(field.default_integer());
    Ok(table.get::<T>(field.offset(), default))
}

/// Gets a floating point table field given its exact type. Returns default float value if the field is not set. Returns [None] if no default value is found. Returns error if the type doesn't match.
///
/// # Safety
///
/// The value of the corresponding slot must have type T
pub unsafe fn get_field_float<'a, T: Follow<'a, Inner = T> + Float + 'a>(
    table: &'a Table,
    field: &Field,
) -> FlatbufferResult<Option<T>> {
    if size_of::<T>() != get_type_size(field.type_().base_type()) {
        return Err(FlatbufferError::FieldTypeMismatch(
            std::any::type_name::<T>().to_string(),
            field
                .type_()
                .base_type()
                .variant_name()
                .unwrap_or_default()
                .to_string(),
        ));
    }

    let default = T::from(field.default_real());
    Ok(table.get::<T>(field.offset(), default))
}

/// Gets a String table field given its exact type. Returns empty string if the field is not set. Returns [None] if no default value is found. Returns error if the type size doesn't match.
///
/// # Safety
///
/// The value of the corresponding slot must have type String
pub unsafe fn get_field_string<'a>(
    table: &'a Table,
    field: &Field,
) -> FlatbufferResult<Option<&'a str>> {
    if field.type_().base_type() != BaseType::String {
        return Err(FlatbufferError::FieldTypeMismatch(
            String::from("String"),
            field
                .type_()
                .base_type()
                .variant_name()
                .unwrap_or_default()
                .to_string(),
        ));
    }

    Ok(table.get::<ForwardsUOffset<&'a str>>(field.offset(), Some("")))
}

/// Gets a [Struct] table field given its exact type. Returns [None] if the field is not set. Returns error if the type doesn't match.
///
/// # Safety
///
/// The value of the corresponding slot must have type Struct
pub unsafe fn get_field_struct<'a>(
    table: &'a Table,
    field: &Field,
) -> FlatbufferResult<Option<Struct<'a>>> {
    // TODO inherited from C++: This does NOT check if the field is a table or struct, but we'd need
    // access to the schema to check the is_struct flag.
    if field.type_().base_type() != BaseType::Obj {
        return Err(FlatbufferError::FieldTypeMismatch(
            String::from("Obj"),
            field
                .type_()
                .base_type()
                .variant_name()
                .unwrap_or_default()
                .to_string(),
        ));
    }

    Ok(table.get::<Struct>(field.offset(), None))
}

/// Gets a [Struct] struct field given its exact type. Returns error if the type doesn't match.
///
/// # Safety
///
/// The value of the corresponding slot must have type Struct.
pub unsafe fn get_field_struct_in_struct<'a>(
    st: &'a Struct,
    field: &Field,
) -> FlatbufferResult<Struct<'a>> {
    // TODO inherited from C++: This does NOT check if the field is a table or struct, but we'd need
    // access to the schema to check the is_struct flag.
    if field.type_().base_type() != BaseType::Obj {
        return Err(FlatbufferError::FieldTypeMismatch(
            String::from("Obj"),
            field
                .type_()
                .base_type()
                .variant_name()
                .unwrap_or_default()
                .to_string(),
        ));
    }

    Ok(st.get::<Struct>(field.offset() as usize))
}

/// Returns the size of a scalar type in the `BaseType` enum. In the case of structs, returns the size of their offset (`UOffsetT`) in the buffer.
fn get_type_size(base_type: BaseType) -> usize {
    match base_type {
        BaseType::UType | BaseType::Bool | BaseType::Byte | BaseType::UByte => 1,
        BaseType::Short | BaseType::UShort => 2,
        BaseType::Int
        | BaseType::UInt
        | BaseType::Float
        | BaseType::String
        | BaseType::Vector
        | BaseType::Obj
        | BaseType::Union => 4,
        BaseType::Long | BaseType::ULong | BaseType::Double | BaseType::Vector64 => 8,
        _ => 0,
    }
}
