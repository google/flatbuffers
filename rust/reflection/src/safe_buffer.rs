/*
 * Copyright 2025 Google Inc. All rights reserved.
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

use crate::r#struct::Struct;
use crate::reflection_generated::reflection::{Field, Schema};
use crate::reflection_verifier::verify_with_options;
use crate::{
    get_any_field_float, get_any_field_float_in_struct, get_any_field_integer,
    get_any_field_integer_in_struct, get_any_field_string, get_any_field_string_in_struct,
    get_any_root, get_field_float, get_field_integer, get_field_string, get_field_struct,
    get_field_struct_in_struct, get_field_table, get_field_vector, FlatbufferError,
    FlatbufferResult, ForwardsUOffset,
};
use flatbuffers::{Follow, Table, Vector, VerifierOptions};
use num::traits::float::Float;
use num::traits::int::PrimInt;
use num::traits::FromPrimitive;
use std::collections::HashMap;
use std::sync::RwLock;

pub struct SafeBuffer<'a> {
    buf: &'a [u8],
    schema: &'a Schema<'a>,
    buf_loc_to_obj_idx: RwLock<HashMap<usize, i32>>,
}

impl<'a> SafeBuffer<'a> {
    pub fn new(buf: &'a [u8], schema: &'a Schema) -> FlatbufferResult<Self> {
        Self::new_with_custom_options(buf, schema, &VerifierOptions::default())
    }

    pub fn new_with_custom_options(
        buf: &'a [u8],
        schema: &'a Schema,
        opts: &VerifierOptions,
    ) -> FlatbufferResult<Self> {
        let mut buf_loc_to_obj_idx = HashMap::new();
        verify_with_options(&buf, schema, opts, &mut buf_loc_to_obj_idx)?;
        Ok(SafeBuffer {
            buf,
            schema,
            buf_loc_to_obj_idx: RwLock::new(buf_loc_to_obj_idx),
        })
    }

    /// Gets the root table in the buffer.
    pub fn get_root(&self) -> Table {
        // SAFETY: the buffer was verified during construction.
        unsafe { get_any_root(self.buf) }
    }

    /// Gets all the fields the [table] has. Returns error if the [table] doesn't match the buffer.
    pub fn get_all_table_fields(
        &self,
        table: &'a Table,
    ) -> FlatbufferResult<Vector<ForwardsUOffset<Field>>> {
        self.get_all_fields(table.loc())
    }

    /// Gets all the fields the [st] has. Returns error if the [st] doesn't match the buffer.
    pub fn get_all_struct_fields(
        &self,
        st: &'a Struct,
    ) -> FlatbufferResult<Vector<ForwardsUOffset<Field>>> {
        self.get_all_fields(st.loc())
    }

    /// Gets an integer table field given its exact type. Returns default integer value if the field is not set. Returns [None] if no default value is found. Returns error if
    /// the [table] doesn't match the buffer or
    /// the [field_name] doesn't match the [table] or
    /// the field type doesn't match.
    pub fn get_field_integer<T: Follow<'a, Inner = T> + PrimInt + FromPrimitive + 'a>(
        &self,
        table: &'a Table,
        field_name: &str,
    ) -> FlatbufferResult<Option<T>> {
        if !self.contains_table(table)? {
            return Err(FlatbufferError::InvalidTableOrStruct());
        }

        if let Some(field) = self.find_field_by_name(table.loc(), field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_field_integer::<T>(table, &field) }
        } else {
            Err(FlatbufferError::FieldNotFound())
        }
    }

    /// Gets a floating point table field given its exact type. Returns default float value if the field is not set. Returns [None] if no default value is found. Returns error if
    /// the [table] doesn't match the buffer or
    /// the [field_name] doesn't match the [table] or
    /// the field type doesn't match.
    pub fn get_field_float<T: Follow<'a, Inner = T> + Float + 'a>(
        &self,
        table: &'a Table,
        field_name: &str,
    ) -> FlatbufferResult<Option<T>> {
        if !self.contains_table(table)? {
            return Err(FlatbufferError::InvalidTableOrStruct());
        }

        if let Some(field) = self.find_field_by_name(table.loc(), field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_field_float::<T>(table, &field) }
        } else {
            Err(FlatbufferError::FieldNotFound())
        }
    }

    /// Gets a String table field given its exact type. Returns empty string if the field is not set. Returns [None] if no default value is found. Returns error if
    /// the [table] doesn't match the buffer or
    /// the [field_name] doesn't match the [table] or
    /// the field type doesn't match.
    pub fn get_field_string(
        &self,
        table: &'a Table,
        field_name: &str,
    ) -> FlatbufferResult<Option<&'a str>> {
        if !self.contains_table(table)? {
            return Err(FlatbufferError::InvalidTableOrStruct());
        }

        if let Some(field) = self.find_field_by_name(table.loc(), field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_field_string(table, &field) }
        } else {
            Err(FlatbufferError::FieldNotFound())
        }
    }

    /// Gets a [Struct] table field given its exact type. Returns [None] if the field is not set. Returns error if
    /// the [table] doesn't match the buffer or
    /// the [field_name] doesn't match the [table] or
    /// the field type doesn't match.
    pub fn get_field_struct(
        &self,
        table: &'a Table,
        field_name: &str,
    ) -> FlatbufferResult<Option<Struct<'a>>> {
        if !self.contains_table(table)? {
            return Err(FlatbufferError::InvalidTableOrStruct());
        }

        if let Some(field) = self.find_field_by_name(table.loc(), field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_field_struct(table, &field) }
        } else {
            Err(FlatbufferError::FieldNotFound())
        }
    }

    /// Gets a Vector table field given its exact type. Returns empty vector if the field is not set. Returns error if
    /// the [table] doesn't match the buffer or
    /// the [field_name] doesn't match the [table] or
    /// the field type doesn't match.
    pub fn get_field_vector<T: Follow<'a, Inner = T>>(
        &self,
        table: &'a Table,
        field_name: &str,
    ) -> FlatbufferResult<Option<Vector<'a, T>>> {
        if !self.contains_table(table)? {
            return Err(FlatbufferError::InvalidTableOrStruct());
        }

        if let Some(field) = self.find_field_by_name(table.loc(), field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_field_vector(table, &field) }
        } else {
            Err(FlatbufferError::FieldNotFound())
        }
    }

    /// Gets a Table table field given its exact type. Returns [None] if the field is not set. Returns error if
    /// the [table] doesn't match the buffer or
    /// the [field_name] doesn't match the [table] or
    /// the field type doesn't match.
    pub fn get_field_table(
        &self,
        table: &'a Table,
        field_name: &str,
    ) -> FlatbufferResult<Option<Table<'a>>> {
        if !self.contains_table(table)? {
            return Err(FlatbufferError::InvalidTableOrStruct());
        }

        if let Some(field) = self.find_field_by_name(table.loc(), field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_field_table(table, &field) }
        } else {
            Err(FlatbufferError::FieldNotFound())
        }
    }

    /// Returns the value of any table field as a 64-bit int, regardless of what type it is. Returns default integer if the field is not set or error if
    /// the value cannot be parsed as integer or
    /// the [table] doesn't match the buffer or
    /// the [field_name] doesn't match the [table].
    /// [num_traits](https://docs.rs/num-traits/latest/num_traits/cast/trait.NumCast.html) is used for number casting.
    pub fn get_any_field_integer(&self, table: &Table, field_name: &str) -> FlatbufferResult<i64> {
        if !self.contains_table(table)? {
            return Err(FlatbufferError::InvalidTableOrStruct());
        }

        if let Some(field) = self.find_field_by_name(table.loc(), field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_any_field_integer(table, &field) }
        } else {
            Err(FlatbufferError::FieldNotFound())
        }
    }

    /// Returns the value of any table field as a 64-bit floating point, regardless of what type it is. Returns default float if the field is not set or error if
    /// the value cannot be parsed as float or
    /// the [table] doesn't match the buffer or
    /// the [field_name] doesn't match the [table].
    pub fn get_any_field_float(&self, table: &Table, field_name: &str) -> FlatbufferResult<f64> {
        if !self.contains_table(table)? {
            return Err(FlatbufferError::InvalidTableOrStruct());
        }

        if let Some(field) = self.find_field_by_name(table.loc(), field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_any_field_float(table, &field) }
        } else {
            Err(FlatbufferError::FieldNotFound())
        }
    }

    /// Returns the value of any table field as a string, regardless of what type it is. Returns empty string if the field is not set. Returns error if
    /// the [table] doesn't match the buffer or
    /// the [field_name] doesn't match the [table].
    pub fn get_any_field_string(
        &self,
        table: &Table,
        field_name: &str,
    ) -> FlatbufferResult<String> {
        if !self.contains_table(table)? {
            return Err(FlatbufferError::InvalidTableOrStruct());
        }

        if let Some(field) = self.find_field_by_name(table.loc(), field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { Ok(get_any_field_string(table, &field, self.schema)) }
        } else {
            Err(FlatbufferError::FieldNotFound())
        }
    }

    /// Gets a [Struct] struct field given its exact type. Returns error if
    /// the [st] doesn't match the buffer or
    /// the [field_name] doesn't match the [st] or
    /// the field type doesn't match.
    pub fn get_field_struct_in_struct(
        &self,
        st: &'a Struct,
        field_name: &str,
    ) -> FlatbufferResult<Struct<'a>> {
        if !self.contains_struct(st)? {
            return Err(FlatbufferError::InvalidTableOrStruct());
        }

        if let Some(field) = self.find_field_by_name(st.loc(), field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_field_struct_in_struct(st, &field) }
        } else {
            Err(FlatbufferError::FieldNotFound())
        }
    }

    /// Returns the value of any struct field as a 64-bit int, regardless of what type it is. Returns error if
    /// the [st] doesn't match the buffer or
    /// the [field_name] doesn't match the [st] or
    /// the value cannot be parsed as integer.
    pub fn get_any_field_integer_in_struct(
        &self,
        st: &'a Struct,
        field_name: &str,
    ) -> FlatbufferResult<i64> {
        if !self.contains_struct(st)? {
            return Err(FlatbufferError::InvalidTableOrStruct());
        }

        if let Some(field) = self.find_field_by_name(st.loc(), field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_any_field_integer_in_struct(st, &field) }
        } else {
            Err(FlatbufferError::FieldNotFound())
        }
    }

    /// Returns the value of any struct field as a 64-bit floating point, regardless of what type it is. Returns error if
    /// the [st] doesn't match the buffer or
    /// the [field_name] doesn't match the [st] or
    /// the value cannot be parsed as float.
    pub fn get_any_field_float_in_struct(
        &self,
        st: &'a Struct,
        field_name: &str,
    ) -> FlatbufferResult<f64> {
        if !self.contains_struct(st)? {
            return Err(FlatbufferError::InvalidTableOrStruct());
        }

        if let Some(field) = self.find_field_by_name(st.loc(), field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_any_field_float_in_struct(st, &field) }
        } else {
            Err(FlatbufferError::FieldNotFound())
        }
    }

    /// Returns the value of any struct field as a string, regardless of what type it is. Returns error if
    /// the [st] doesn't match the buffer or
    /// the [field_name] doesn't match the [st].
    pub fn get_any_field_string_in_struct(
        &self,
        st: &'a Struct,
        field_name: &str,
    ) -> FlatbufferResult<String> {
        if !self.contains_struct(st)? {
            return Err(FlatbufferError::InvalidTableOrStruct());
        }

        if let Some(field) = self.find_field_by_name(st.loc(), field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { Ok(get_any_field_string_in_struct(st, &field, self.schema)) }
        } else {
            Err(FlatbufferError::FieldNotFound())
        }
    }

    fn contains_table(&self, table: &Table) -> FlatbufferResult<bool> {
        Ok(std::ptr::eq(self.buf, table.buf())
            && self
                .buf_loc_to_obj_idx
                .read()
                .map_err(|err| FlatbufferError::RwLockPoisonError(err.to_string()))?
                .contains_key(&table.loc()))
    }

    fn contains_struct(&self, st: &Struct) -> FlatbufferResult<bool> {
        Ok(std::ptr::eq(self.buf, st.buf())
            && self
                .buf_loc_to_obj_idx
                .read()
                .map_err(|err| FlatbufferError::RwLockPoisonError(err.to_string()))?
                .contains_key(&st.loc()))
    }

    fn find_field_by_name(
        &self,
        buf_loc: usize,
        field_name: &str,
    ) -> FlatbufferResult<Option<Field>> {
        Ok(self
            .get_all_fields(buf_loc)?
            .lookup_by_key(field_name, |field: &Field<'_>, key| {
                field.key_compare_with_value(key)
            }))
    }

    fn get_all_fields(&self, buf_loc: usize) -> FlatbufferResult<Vector<ForwardsUOffset<Field>>> {
        if let Some(&obj_idx) = self
            .buf_loc_to_obj_idx
            .read()
            .map_err(|err| FlatbufferError::RwLockPoisonError(err.to_string()))?
            .get(&buf_loc)
        {
            let obj = if obj_idx == -1 {
                self.schema.root_table().unwrap()
            } else {
                self.schema.objects().get(obj_idx.try_into()?)
            };
            Ok(obj.fields())
        } else {
            Err(FlatbufferError::InvalidTableOrStruct())
        }
    }
}
