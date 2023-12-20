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

use flatbuffers::{
    emplace_scalar, read_scalar, EndianScalar, Follow, ForwardsUOffset, SOffsetT, Table, UOffsetT,
    VOffsetT, Vector, SIZE_SOFFSET, SIZE_UOFFSET,
};
use reflection_generated::reflection::{BaseType, Field, Object, Schema};

use anyhow::{Ok, Result};
use core::mem::size_of;
use escape_string::escape;
use num::traits::float::Float;
use num::traits::int::PrimInt;
use num::traits::FromPrimitive;
use stdint::uintmax_t;

/// Gets the root table from a trusted Flatbuffer.
///
/// # Safety
///
/// Flatbuffers accessors do not perform validation checks before accessing. Users
/// must trust `data` contains a valid flatbuffer. Reading unchecked buffers may cause panics or even UB.
pub unsafe fn get_any_root<'buf>(data: &[u8]) -> Table {
    <ForwardsUOffset<Table>>::follow(data, 0)
}

/// Gets an integer table field given its exact type. Returns default integer value if the field is not set. Returns None if the type size doesn't match.
///
/// # Safety
///
/// The value of the corresponding slot must have type T
pub unsafe fn get_field_integer<'a, T: Follow<'a, Inner = T> + PrimInt + FromPrimitive + 'a>(
    table: &'a Table,
    field: &Field,
) -> Option<T> {
    if size_of::<T>() != get_type_size(field.type_().base_type()) {
        return None;
    }

    let default = T::from_i64(field.default_integer());
    table.get::<T>(field.offset(), default)
}

/// Gets a floating point table field given its exact type. Returns default float value if the field is not set. Returns None if the type doesn't match.
///
/// # Safety
///
/// The value of the corresponding slot must have type T
pub unsafe fn get_field_float<'a, T: Follow<'a, Inner = T> + Float + 'a>(
    table: &'a Table,
    field: &Field,
) -> Option<T> {
    if core::mem::size_of::<T>() != get_type_size(field.type_().base_type()) {
        return None;
    }

    let default = T::from(field.default_real());
    table.get::<T>(field.offset(), default)
}

/// Gets a String table field given its exact type. Returns empty string if the field is not set. Returns None if the type doesn't match.
///
/// # Safety
///
/// The value of the corresponding slot must have type String
pub unsafe fn get_field_string<'a>(table: &'a Table, field: &Field) -> Option<&'a str> {
    if field.type_().base_type() != BaseType::String {
        return None;
    }

    table.get::<ForwardsUOffset<&'a str>>(field.offset(), Some(""))
}

/// Gets a `Struct` table field given its exact type. Returns [None] if the field is not set or the type doesn't match.
///
/// # Safety
///
/// The value of the corresponding slot must have type Struct
pub unsafe fn get_field_struct<'a>(table: &'a Table, field: &Field) -> Option<Struct<'a>> {
    // TODO inherited from C++: This does NOT check if the field is a table or struct, but we'd need
    // access to the schema to check the is_struct flag.
    if field.type_().base_type() != BaseType::Obj {
        return None;
    }

    table.get::<Struct>(field.offset(), None)
}

/// Gets a Vector table field given its exact type. Returns empty vector if the field is not set. Returns [None] if the type doesn't match.
///
/// # Safety
///
/// The value of the corresponding slot must have type Vector
pub unsafe fn get_field_vector<'a, T: Follow<'a, Inner = T>>(
    table: &'a Table,
    field: &Field,
) -> Option<Vector<'a, T>> {
    if field.type_().base_type() != BaseType::Vector
        || core::mem::size_of::<T>() != get_type_size(field.type_().element())
    {
        return None;
    }

    table.get::<ForwardsUOffset<Vector<'a, T>>>(field.offset(), Some(Vector::<T>::default()))
}

/// Gets a Table table field given its exact type. Returns `None` if the field is not set or the type doesn't match.
///
/// # Safety
///
/// The value of the corresponding slot must have type Table
pub unsafe fn get_field_table<'a>(table: &'a Table, field: &Field) -> Option<Table<'a>> {
    if field.type_().base_type() != BaseType::Obj {
        return None;
    }

    table.get::<ForwardsUOffset<Table<'a>>>(field.offset(), None)
}

/// Gets a `Struct` struct field given its exact type.
///
/// # Safety
///
/// The value of the corresponding slot must have type Struct.
pub unsafe fn get_field_struct_in_struct<'a>(st: &'a Struct, field: &Field) -> Option<Struct<'a>> {
    // TODO inherited from C++: This does NOT check if the field is a table or struct, but we'd need
    // access to the schema to check the is_struct flag.
    if field.type_().base_type() != BaseType::Obj {
        return None;
    }

    st.get::<Struct>(field.offset() as usize)
}

/// Returns the value of any table field as a 64-bit int, regardless of what type it is. Returns default integer if the field is not set or None if the value cannot be parsed as integer.
///
/// # Safety
///
/// The field must point to valid value in the buffer if set.
pub unsafe fn get_any_field_integer(table: &Table, field: &Field) -> Option<i64> {
    if let Some(field_loc) = get_field_loc(table, field) {
        get_any_value_integer(field.type_().base_type(), table.buf(), field_loc)
    } else {
        Some(field.default_integer())
    }
}

/// Returns the value of any table field as a 64-bit floating point, regardless of what type it is. Returns default float if the field is not set or [None] if the value cannot be parsed as float.
///
/// # Safety
///
/// The field must point to valid value in the buffer if set.
pub unsafe fn get_any_field_float(table: &Table, field: &Field) -> Option<f64> {
    if let Some(field_loc) = get_field_loc(table, field) {
        get_any_value_float(field.type_().base_type(), table.buf(), field_loc)
    } else {
        Some(field.default_real())
    }
}

/// Returns the value of any table field as a string, regardless of what type it is. Returns empty string if the field is not set.
///
/// # Safety
///
/// The field must point to valid value in the buffer if set.
pub unsafe fn get_any_field_string(table: &Table, field: &Field, schema: &Schema) -> String {
    if let Some(field_loc) = get_field_loc(table, field) {
        get_any_value_string(
            field.type_().base_type(),
            table.buf(),
            field_loc,
            schema,
            field.type_().index() as usize,
        )
    } else {
        String::from("")
    }
}

/// Returns the value of any struct field as a 64-bit int, regardless of what type it is. Returns [None] if the value cannot be parsed as integer.
///
/// # Safety
///
/// The field must point to valid value in the buffer.
pub unsafe fn get_any_field_integer_in_struct(st: &Struct, field: &Field) -> Option<i64> {
    let field_loc = st.loc() + field.offset() as usize;

    get_any_value_integer(field.type_().base_type(), st.buf(), field_loc)
}

/// Returns the value of any struct field as a 64-bit floating point, regardless of what type it is. Returns [None] if the value cannot be parsed as float.
///
/// # Safety
///
/// The field must point to valid value in the buffer.
pub unsafe fn get_any_field_float_in_struct(st: &Struct, field: &Field) -> Option<f64> {
    let field_loc = st.loc() + field.offset() as usize;

    get_any_value_float(field.type_().base_type(), st.buf(), field_loc)
}

/// Returns the value of any struct field as a string, regardless of what type it is.
///
/// # Safety
///
/// The field must point to valid value in the buffer.
pub unsafe fn get_any_field_string_in_struct(
    st: &Struct,
    field: &Field,
    schema: &Schema,
) -> String {
    let field_loc = st.loc() + field.offset() as usize;

    get_any_value_string(
        field.type_().base_type(),
        st.buf(),
        field_loc,
        schema,
        field.type_().index() as usize,
    )
}

/// Sets any table field with the value of a 64-bit integer. Returns false if the field doesn't point to a valid buffer location or is with non-scalar value or the provided value cannot be cast into the field type.
///
/// # Safety
///
/// The [buf] must be valid.
pub unsafe fn set_any_field_integer<'a>(buf: &'a mut [u8], field: &Field, v: i64) -> bool {
    let field_type = field.type_().base_type();
    let table = get_any_root(buf);

    if let Some(field_loc) = get_field_loc(&table, field) {
        if !is_scalar(field_type) || buf.len() < field_loc + get_type_size(field_type) {
            return false;
        }

        // SAFETY: the index was verified above.
        unsafe { set_any_value_integer(field_type, &mut buf[field_loc..], v) }
    } else {
        false
    }
}

/// Sets any table field with the value of a 64-bit floating point. Returns false if the field doesn't point to a valid buffer location or is with non-scalar value or the provided value cannot be cast into the field type.
///
/// # Safety
///
/// The [buf] must be valid.
pub unsafe fn set_any_field_float<'a>(buf: &'a mut [u8], field: &Field, v: f64) -> bool {
    let field_type = field.type_().base_type();
    let table = get_any_root(buf);

    if let Some(field_loc) = get_field_loc(&table, field) {
        if !is_scalar(field_type) || buf.len() < field_loc + get_type_size(field_type) {
            return false;
        }

        // SAFETY: the index was verified above.
        unsafe { set_any_value_float(field_type, &mut buf[field_loc..], v) }
    } else {
        false
    }
}

/// Sets any table field with the value of a string. Returns false if the field doesn't point to a valid buffer location or is with non-scalar value or the provided value cannot be parsed as the field type.
///
/// # Safety
///
/// The [buf] must be valid.
pub unsafe fn set_any_field_string<'a>(buf: &'a mut [u8], field: &Field, v: &str) -> bool {
    let field_type = field.type_().base_type();
    let table = get_any_root(buf);

    if let Some(field_loc) = get_field_loc(&table, field) {
        if !is_scalar(field_type) || buf.len() < field_loc + get_type_size(field_type) {
            return false;
        }

        if let core::result::Result::Ok(value) = v.parse::<f64>() {
            // SAFETY: the index was verified above.
            return unsafe { set_any_value_float(field_type, &mut buf[field_loc..], value) };
        }
    }
    false
}

/// Sets any scalar field given its exact type. Returns false if the field doesn't point to a valid buffer location or is with non-scalar value.
///
/// # Safety
///
/// The [buf] must be valid.
pub unsafe fn set_field<'a, T: EndianScalar>(buf: &'a mut [u8], field: &Field, v: T) -> bool {
    let field_type = field.type_().base_type();
    let table = get_any_root(buf);

    if !is_scalar(field_type) || core::mem::size_of::<T>() != get_type_size(field_type) {
        return false;
    }

    if let Some(field_loc) = get_field_loc(&table, field) {
        if buf.len() < field_loc + get_type_size(field_type) {
            return false;
        }

        // SAFETY: the index was verified above.
        unsafe { emplace_scalar::<T>(&mut buf[field_loc..], v) };
        true
    } else {
        false
    }
}

/// Sets a string field to a new value. Returns false if the field doesn't point to a valid buffer location or is not of string type in which cases the [buf] stays intact. Returns error if the [buf] fails to be updated.
///
/// # Safety
///
/// The buffer should be valid and conform to the schema.
pub unsafe fn set_string<'a>(
    buf: &mut Vec<u8>,
    field: &Field,
    v: &str,
    schema: &Schema,
) -> Result<bool> {
    let field_type = field.type_().base_type();
    if field_type != BaseType::String {
        return Ok(false);
    }

    let table = get_any_root(buf);
    let table_loc = table.loc();

    if let Some(field_loc) = get_field_loc(&table, field) {
        if buf.len() < field_loc + get_type_size(BaseType::String) {
            return Ok(false);
        }

        // SAFETY: the index was verified above.
        let string_loc = unsafe { deref_uoffset(buf, field_loc)? };
        if buf.len() < string_loc + SIZE_UOFFSET {
            return Ok(false);
        }

        // SAFETY: the index was verified above.
        let len_old = unsafe { read_uoffset(buf, string_loc) };
        if buf.len()
            < string_loc
                .saturating_add(SIZE_UOFFSET)
                .saturating_add(len_old.try_into()?)
        {
            return Ok(false);
        }

        let len_new = v.len();
        let delta = len_new as isize - len_old as isize;
        let mut bytes_to_insert = v.as_bytes().to_vec();

        if delta != 0 {
            // Rounds the delta up to the nearest multiple of the maximum int size to keep the types after the insersion point aligned.
            let mask = (size_of::<uintmax_t>() - 1) as isize;
            let offset = (delta + mask) & !mask;
            let mut visited_vec = vec![false; buf.len()];

            if offset != 0 {
                update_offset(
                    buf,
                    table_loc,
                    &mut visited_vec,
                    &schema.root_table().unwrap(),
                    schema,
                    string_loc,
                    offset,
                )?;

                // Sets the new length.
                emplace_scalar::<SOffsetT>(
                    &mut buf[string_loc..string_loc + SIZE_UOFFSET],
                    len_new.try_into()?,
                );
            }

            // Pads the bytes vector with 0 if `offset` doesn't equal `delta`.
            bytes_to_insert.resize(bytes_to_insert.len() + (offset - delta) as usize, 0);
        }

        // Replaces the data.
        buf.splice(
            string_loc + SIZE_SOFFSET..string_loc + SIZE_UOFFSET + usize::try_from(len_old)?,
            bytes_to_insert,
        );
        Ok(true)
    } else {
        Ok(v.is_empty())
    }
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

/// Returns the absolute field location in the buffer and [None] if the field is not populated.
fn get_field_loc(table: &Table, field: &Field) -> Option<usize> {
    let field_offset = table.vtable().get(field.offset()) as usize;
    if field_offset == 0 {
        return None;
    }

    Some(table.loc() + field_offset)
}

/// Reads value as a 64-bit int from the provided byte slice at the specified location. Returns [None] if the value cannot be parsed as integer.
///
/// # Safety
///
/// Caller must ensure `buf.len() >= loc + size_of::<T>()` at all the access layers.
unsafe fn get_any_value_integer<'a>(base_type: BaseType, buf: &'a [u8], loc: usize) -> Option<i64> {
    match base_type {
        BaseType::UType | BaseType::UByte => i64::from_u8(u8::follow(buf, loc)),
        BaseType::Bool => bool::follow(buf, loc).try_into().ok(),
        BaseType::Byte => i64::from_i8(i8::follow(buf, loc)),
        BaseType::Short => i64::from_i16(i16::follow(buf, loc)),
        BaseType::UShort => i64::from_u16(u16::follow(buf, loc)),
        BaseType::Int => i64::from_i32(i32::follow(buf, loc)),
        BaseType::UInt => i64::from_u32(u32::follow(buf, loc)),
        BaseType::Long => Some(i64::follow(buf, loc)),
        BaseType::ULong => i64::from_u64(u64::follow(buf, loc)),
        BaseType::Float => i64::from_f32(f32::follow(buf, loc)),
        BaseType::Double => i64::from_f64(f64::follow(buf, loc)),
        BaseType::String => ForwardsUOffset::<&str>::follow(buf, loc)
            .parse::<i64>()
            .ok(),
        _ => None, // Tables & vectors do not make sense.
    }
}

/// Reads value as a 64-bit floating point from the provided byte slice at the specified location. Returns None if the value cannot be parsed as float.
///
/// # Safety
///
/// Caller must ensure `buf.len() >= loc + size_of::<T>()` at all the access layers.
unsafe fn get_any_value_float<'a>(base_type: BaseType, buf: &'a [u8], loc: usize) -> Option<f64> {
    match base_type {
        BaseType::UType | BaseType::UByte => f64::from_u8(u8::follow(buf, loc)),
        BaseType::Bool => bool::follow(buf, loc).try_into().ok(),
        BaseType::Byte => f64::from_i8(i8::follow(buf, loc)),
        BaseType::Short => f64::from_i16(i16::follow(buf, loc)),
        BaseType::UShort => f64::from_u16(u16::follow(buf, loc)),
        BaseType::Int => f64::from_i32(i32::follow(buf, loc)),
        BaseType::UInt => f64::from_u32(u32::follow(buf, loc)),
        BaseType::Long => f64::from_i64(i64::follow(buf, loc)),
        BaseType::ULong => f64::from_u64(u64::follow(buf, loc)),
        BaseType::Float => f64::from_f32(f32::follow(buf, loc)),
        BaseType::Double => Some(f64::follow(buf, loc)),
        BaseType::String => ForwardsUOffset::<&str>::follow(buf, loc)
            .parse::<f64>()
            .ok(),
        _ => None,
    }
}

/// Reads value as a string from the provided byte slice at the specified location.
///
/// # Safety
///
/// Caller must ensure `buf.len() >= loc + size_of::<T>()` at all the access layers.
unsafe fn get_any_value_string(
    base_type: BaseType,
    buf: &[u8],
    loc: usize,
    schema: &Schema,
    type_index: usize,
) -> String {
    match base_type {
        BaseType::Float | BaseType::Double => get_any_value_float(base_type, buf, loc)
            .unwrap_or_default()
            .to_string(),
        BaseType::String => ForwardsUOffset::<&str>::follow(buf, loc).to_string(),
        BaseType::Obj => {
            // Converts the table to a string. This is mostly for debugging purposes,
            // and does NOT promise to be JSON compliant.
            // Also prefixes the type.
            let object: Object = schema.objects().get(type_index);
            let mut s = object.name().to_string();
            s += " { ";
            if object.is_struct() {
                let st: Struct<'_> = Struct::follow(buf, loc);
                for field in object.fields() {
                    let field_value = get_any_field_string_in_struct(&st, &field, schema);
                    s += field.name();
                    s += ": ";
                    s += field_value.as_str();
                    s += ", ";
                }
            } else {
                let table = ForwardsUOffset::<Table>::follow(buf, loc);
                for field in object.fields() {
                    if table.vtable().get(field.offset()) == 0 {
                        continue;
                    }
                    let mut field_value = get_any_field_string(&table, &field, schema);
                    if field.type_().base_type() == BaseType::String {
                        field_value = escape(field_value.as_str()).to_string();
                    }
                    s += field.name();
                    s += ": ";
                    s += field_value.as_str();
                    s += ", ";
                }
            }
            s + "}"
        }
        BaseType::Vector => String::from("[(elements)]"), // TODO inherited from C++: implement this as well.
        BaseType::Union => String::from("(union)"), // TODO inherited from C++: implement this as well.
        _ => get_any_value_integer(base_type, buf, loc)
            .unwrap_or_default()
            .to_string(),
    }
}

/// Sets any scalar value with a 64-bit integer. Returns true if the value is successfully replaced otherwise returns false.
///
/// # Safety
///
/// Caller must ensure `buf.len() >= loc + size_of::<T>()`.
unsafe fn set_any_value_integer(base_type: BaseType, buf: &mut [u8], v: i64) -> bool {
    match base_type {
        BaseType::UType | BaseType::UByte => {
            if let Some(value) = u8::from_i64(v) {
                emplace_scalar::<u8>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::Bool => emplace_scalar::<bool>(buf, v != 0),
        BaseType::Byte => {
            if let Some(value) = i8::from_i64(v) {
                emplace_scalar::<i8>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::Short => {
            if let Some(value) = i16::from_i64(v) {
                emplace_scalar::<i16>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::UShort => {
            if let Some(value) = u16::from_i64(v) {
                emplace_scalar::<u16>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::Int => {
            if let Some(value) = i32::from_i64(v) {
                emplace_scalar::<i32>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::UInt => {
            if let Some(value) = u32::from_i64(v) {
                emplace_scalar::<u32>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::Long => emplace_scalar::<i64>(buf, v),
        BaseType::ULong => {
            if let Some(value) = u64::from_i64(v) {
                emplace_scalar::<u64>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::Float => {
            if let Some(value) = f32::from_i64(v) {
                emplace_scalar::<f32>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::Double => {
            if let Some(value) = f64::from_i64(v) {
                emplace_scalar::<f64>(buf, value)
            } else {
                return false;
            }
        }
        _ => return false,
    }
    true
}

/// Sets any scalar value with a 64-bit floating point. Returns true if the value is successfully replaced otherwise returns false.
///
/// # Safety
///
/// Caller must ensure `buf.len() >= loc + size_of::<T>()`.
unsafe fn set_any_value_float(base_type: BaseType, buf: &mut [u8], v: f64) -> bool {
    match base_type {
        BaseType::UType | BaseType::UByte => {
            if let Some(value) = u8::from_f64(v) {
                emplace_scalar::<u8>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::Bool => emplace_scalar::<bool>(buf, v != 0f64),
        BaseType::Byte => {
            if let Some(value) = i8::from_f64(v) {
                emplace_scalar::<i8>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::Short => {
            if let Some(value) = i16::from_f64(v) {
                emplace_scalar::<i16>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::UShort => {
            if let Some(value) = u16::from_f64(v) {
                emplace_scalar::<u16>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::Int => {
            if let Some(value) = i32::from_f64(v) {
                emplace_scalar::<i32>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::UInt => {
            if let Some(value) = u32::from_f64(v) {
                emplace_scalar::<u32>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::Long => {
            if let Some(value) = i64::from_f64(v) {
                emplace_scalar::<i64>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::ULong => {
            if let Some(value) = u64::from_f64(v) {
                emplace_scalar::<u64>(buf, value)
            } else {
                return false;
            }
        }
        BaseType::Float => {
            if let Some(value) = f32::from_f64(v) {
                // Value converted to inf if overflow occurs
                if value != f32::INFINITY {
                    emplace_scalar::<f32>(buf, value)
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
        BaseType::Double => emplace_scalar::<f64>(buf, v),
        _ => return false,
    }
    true
}

fn is_scalar(base_type: BaseType) -> bool {
    return base_type <= BaseType::Double;
}

/// Iterates through the buffer and updates all the relative offsets affected by the insertion.
///
/// # Safety
///
/// Caller must ensure [updated] has the same length as [buf] and the [buf] contains valid data that conforms to [schema].
unsafe fn update_offset(
    buf: &mut [u8],
    table_loc: usize,
    updated: &mut [bool],
    object: &Object,
    schema: &Schema,
    insertion_loc: usize,
    offset: isize,
) -> Result<()> {
    if updated[table_loc] {
        return Ok(());
    }

    let slice = &mut buf[table_loc..table_loc + SIZE_SOFFSET];
    let vtable_offset = isize::try_from(read_scalar::<SOffsetT>(slice))?;
    let vtable_loc = (isize::try_from(table_loc)? - vtable_offset).try_into()?;

    if insertion_loc <= table_loc {
        // Checks if insertion point is between the table and a vtable that
        // precedes it.
        if is_update(insertion_loc, vtable_loc, table_loc) {
            emplace_scalar::<SOffsetT>(slice, (vtable_offset + offset).try_into()?);
            updated[table_loc] = true;
        }

        // Early out: since all fields inside the table must point forwards in
        // memory, if the insertion point is before the table we can stop here.
        return Ok(());
    }

    for field in object.fields() {
        let field_type = field.type_().base_type();
        if is_scalar(field_type) {
            continue;
        }

        let field_offset = VOffsetT::follow(buf, vtable_loc.saturating_add(field.offset().into()));
        if field_offset == 0 {
            continue;
        }

        let field_loc = table_loc + usize::from(field_offset);
        if updated[field_loc] {
            continue;
        }

        if field_type == BaseType::Obj
            && schema
                .objects()
                .get(field.type_().index().try_into()?)
                .is_struct()
        {
            continue;
        }

        // Updates the relative offset from table to actual data if needed
        let slice = &mut buf[field_loc..field_loc + SIZE_UOFFSET];
        let field_value_offset = read_scalar::<UOffsetT>(slice);
        let field_value_loc = field_loc.saturating_add(field_value_offset.try_into()?);
        if is_update(insertion_loc, field_loc, field_value_loc) {
            emplace_scalar::<UOffsetT>(
                slice,
                (isize::try_from(field_value_offset)? + offset).try_into()?,
            );
            updated[field_loc] = true;
        }

        match field_type {
            BaseType::Obj => {
                let field_obj = schema.objects().get(field.type_().index().try_into()?);
                update_offset(
                    buf,
                    field_value_loc,
                    updated,
                    &field_obj,
                    schema,
                    insertion_loc,
                    offset,
                )?;
            }
            BaseType::Vector => {
                let elem_type = field.type_().element();
                if elem_type != BaseType::Obj || elem_type != BaseType::String {
                    continue;
                }
                if elem_type == BaseType::Obj
                    && schema
                        .objects()
                        .get(field.type_().index().try_into()?)
                        .is_struct()
                {
                    continue;
                }
                let vec_size = usize::try_from(read_uoffset(buf, field_value_loc))?;
                for index in 0..vec_size {
                    let elem_loc = field_value_loc + SIZE_UOFFSET + index * SIZE_UOFFSET;
                    if updated[elem_loc] {
                        continue;
                    }
                    let slice = &mut buf[elem_loc..elem_loc + SIZE_UOFFSET];
                    let elem_value_offset = read_scalar::<UOffsetT>(slice);
                    let elem_value_loc = elem_loc.saturating_add(elem_value_offset.try_into()?);
                    if is_update(insertion_loc, elem_loc, elem_value_loc) {
                        emplace_scalar::<UOffsetT>(
                            slice,
                            (isize::try_from(elem_value_offset)? + offset).try_into()?,
                        );
                        updated[elem_loc] = true;
                    }

                    if elem_type == BaseType::Obj {
                        let elem_obj = schema.objects().get(field.type_().index().try_into()?);
                        update_offset(
                            buf,
                            elem_value_loc,
                            updated,
                            &elem_obj,
                            schema,
                            insertion_loc,
                            offset,
                        )?;
                    }
                }
            }
            BaseType::Union => {
                let union_enum = schema.enums().get(field.type_().index().try_into()?);
                let union_type = object
                    .fields()
                    .lookup_by_key(field.name().to_string() + "_type", |field, key| {
                        field.key_compare_with_value(key)
                    })
                    .unwrap();
                let union_type_loc = vtable_loc.saturating_add(union_type.offset().into());
                let union_type_offset = VOffsetT::follow(buf, union_type_loc);
                let union_type_value =
                    u8::follow(buf, table_loc.saturating_add(union_type_offset.into()));
                let union_enum_value = union_enum
                    .values()
                    .lookup_by_key(union_type_value.into(), |value, key| {
                        value.key_compare_with_value(*key)
                    })
                    .unwrap();
                let union_object = schema
                    .objects()
                    .get(union_enum_value.union_type().unwrap().index().try_into()?);
                update_offset(
                    buf,
                    field_value_loc,
                    updated,
                    &union_object,
                    schema,
                    insertion_loc,
                    offset,
                )?;
            }
            _ => (),
        }
    }

    // Checks if the vtable offset points beyond the insertion point.
    if is_update(insertion_loc, table_loc, vtable_loc) {
        let slice = &mut buf[table_loc..table_loc + SIZE_SOFFSET];
        emplace_scalar::<SOffsetT>(slice, (vtable_offset - offset).try_into()?);
        updated[table_loc] = true;
    }
    Ok(())
}

/// Returns true if the insertion point is in between `left` (which contains the relative offset) and `right` (where the relative offset points to).
fn is_update(insertion_loc: usize, left: usize, right: usize) -> bool {
    (left..right).contains(&insertion_loc)
}

/// Returns the absolute location of the data (e.g. string) in the buffer when the field contains relative offset (`UOffsetT`) to the data.
///
/// # Safety
///
/// The value of the corresponding slot must have type `UOffsetT`.
unsafe fn deref_uoffset<'a>(buf: &'a [u8], field_loc: usize) -> Result<usize> {
    Ok(field_loc.saturating_add(read_uoffset(buf, field_loc).try_into()?))
}

/// Reads the value of `UOffsetT` at the give location.
///
/// # Safety
///
/// The value of the corresponding slot must have type `UOffsetT`.
unsafe fn read_uoffset<'a>(buf: &'a [u8], loc: usize) -> UOffsetT {
    let slice = &buf[loc..loc + SIZE_UOFFSET];
    read_scalar::<UOffsetT>(slice)
}
