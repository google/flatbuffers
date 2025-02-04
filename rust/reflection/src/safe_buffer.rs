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
use num_traits::float::Float;
use num_traits::int::PrimInt;
use num_traits::FromPrimitive;
use std::collections::HashMap;

#[derive(Debug)]
pub struct SafeBuffer<'a> {
    buf: &'a [u8],
    schema: &'a Schema<'a>,
    buf_loc_to_obj_idx: HashMap<usize, i32>,
}

impl<'a> SafeBuffer<'a> {
    pub fn new(buf: &'a [u8], schema: &'a Schema) -> FlatbufferResult<Self> {
        Self::new_with_options(buf, schema, &VerifierOptions::default())
    }

    pub fn new_with_options(
        buf: &'a [u8],
        schema: &'a Schema,
        opts: &VerifierOptions,
    ) -> FlatbufferResult<Self> {
        let mut buf_loc_to_obj_idx = HashMap::new();
        verify_with_options(&buf, schema, opts, &mut buf_loc_to_obj_idx)?;
        Ok(SafeBuffer {
            buf,
            schema,
            buf_loc_to_obj_idx,
        })
    }

    /// Gets the root table in the buffer.
    pub fn get_root(&self) -> SafeTable {
        // SAFETY: the buffer was verified during construction.
        let table = unsafe { get_any_root(self.buf) };

        SafeTable {
            safe_buf: self,
            loc: table.loc(),
        }
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
        if let Some(&obj_idx) = self.buf_loc_to_obj_idx.get(&buf_loc) {
            let obj = if obj_idx == -1 {
                self.schema.root_table().unwrap()
            } else {
                self.schema.objects().get(obj_idx.try_into()?)
            };
            Ok(obj.fields())
        } else {
            Err(FlatbufferError::InvalidTableOrStruct)
        }
    }
}

#[derive(Debug)]
pub struct SafeTable<'a> {
    safe_buf: &'a SafeBuffer<'a>,
    loc: usize,
}

impl<'a> SafeTable<'a> {
    /// Gets an integer table field given its exact type. Returns default integer value if the field is not set. Returns [None] if no default value is found. Returns error if
    /// the table doesn't match the buffer or
    /// the [field_name] doesn't match the table or
    /// the field type doesn't match.
    pub fn get_field_integer<T: for<'b> Follow<'b, Inner = T> + PrimInt + FromPrimitive>(
        &self,
        field_name: &str,
    ) -> FlatbufferResult<Option<T>> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_field_integer::<T>(&Table::new(&self.safe_buf.buf, self.loc), &field) }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Gets a floating point table field given its exact type. Returns default float value if the field is not set. Returns [None] if no default value is found. Returns error if
    /// the table doesn't match the buffer or
    /// the [field_name] doesn't match the table or
    /// the field type doesn't match.
    pub fn get_field_float<T: for<'b> Follow<'b, Inner = T> + Float>(
        &self,
        field_name: &str,
    ) -> FlatbufferResult<Option<T>> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_field_float::<T>(&Table::new(&self.safe_buf.buf, self.loc), &field) }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Gets a String table field given its exact type. Returns empty string if the field is not set. Returns [None] if no default value is found. Returns error if
    /// the table doesn't match the buffer or
    /// the [field_name] doesn't match the table or
    /// the field type doesn't match.
    pub fn get_field_string(&self, field_name: &str) -> FlatbufferResult<Option<&str>> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_field_string(&Table::new(&self.safe_buf.buf, self.loc), &field) }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Gets a [SafeStruct] table field given its exact type. Returns [None] if the field is not set. Returns error if
    /// the table doesn't match the buffer or
    /// the [field_name] doesn't match the table or
    /// the field type doesn't match.
    pub fn get_field_struct(&self, field_name: &str) -> FlatbufferResult<Option<SafeStruct<'a>>> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            let optional_st =
                unsafe { get_field_struct(&Table::new(&self.safe_buf.buf, self.loc), &field)? };
            Ok(optional_st.map(|st| SafeStruct {
                safe_buf: self.safe_buf,
                loc: st.loc(),
            }))
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Gets a Vector table field given its exact type. Returns empty vector if the field is not set. Returns error if
    /// the table doesn't match the buffer or
    /// the [field_name] doesn't match the table or
    /// the field type doesn't match.
    pub fn get_field_vector<T: Follow<'a, Inner = T>>(
        &self,
        field_name: &str,
    ) -> FlatbufferResult<Option<Vector<'a, T>>> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_field_vector(&Table::new(&self.safe_buf.buf, self.loc), &field) }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Gets a [SafeTable] table field given its exact type. Returns [None] if the field is not set. Returns error if
    /// the table doesn't match the buffer or
    /// the [field_name] doesn't match the table or
    /// the field type doesn't match.
    pub fn get_field_table(&self, field_name: &str) -> FlatbufferResult<Option<SafeTable<'a>>> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            let optional_table =
                unsafe { get_field_table(&Table::new(&self.safe_buf.buf, self.loc), &field)? };
            Ok(optional_table.map(|t| SafeTable {
                safe_buf: self.safe_buf,
                loc: t.loc(),
            }))
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Returns the value of any table field as a 64-bit int, regardless of what type it is. Returns default integer if the field is not set or error if
    /// the value cannot be parsed as integer or
    /// the table doesn't match the buffer or
    /// the [field_name] doesn't match the table.
    /// [num_traits](https://docs.rs/num-traits/latest/num_traits/cast/trait.NumCast.html) is used for number casting.
    pub fn get_any_field_integer(&self, field_name: &str) -> FlatbufferResult<i64> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_any_field_integer(&Table::new(&self.safe_buf.buf, self.loc), &field) }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Returns the value of any table field as a 64-bit floating point, regardless of what type it is. Returns default float if the field is not set or error if
    /// the value cannot be parsed as float or
    /// the table doesn't match the buffer or
    /// the [field_name] doesn't match the table.
    pub fn get_any_field_float(&self, field_name: &str) -> FlatbufferResult<f64> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_any_field_float(&Table::new(&self.safe_buf.buf, self.loc), &field) }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Returns the string representation of any table field value (e.g. integer 123 is returned as "123"), regardless of what type it is. Returns empty string if the field is not set. Returns error if
    /// the table doesn't match the buffer or
    /// the [field_name] doesn't match the table.
    pub fn get_any_field_string(&self, field_name: &str) -> FlatbufferResult<String> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe {
                Ok(get_any_field_string(
                    &Table::new(&self.safe_buf.buf, self.loc),
                    &field,
                    self.safe_buf.schema,
                ))
            }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }
}

#[derive(Debug)]
pub struct SafeStruct<'a> {
    safe_buf: &'a SafeBuffer<'a>,
    loc: usize,
}

impl<'a> SafeStruct<'a> {
    /// Gets a [SafeStruct] struct field given its exact type. Returns error if
    /// the struct doesn't match the buffer or
    /// the [field_name] doesn't match the struct or
    /// the field type doesn't match.
    pub fn get_field_struct(&self, field_name: &str) -> FlatbufferResult<SafeStruct<'a>> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            let st = unsafe {
                get_field_struct_in_struct(&Struct::new(&self.safe_buf.buf, self.loc), &field)?
            };
            Ok(SafeStruct {
                safe_buf: self.safe_buf,
                loc: st.loc(),
            })
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Returns the value of any struct field as a 64-bit int, regardless of what type it is. Returns error if
    /// the struct doesn't match the buffer or
    /// the [field_name] doesn't match the struct or
    /// the value cannot be parsed as integer.
    pub fn get_any_field_integer(&self, field_name: &str) -> FlatbufferResult<i64> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe {
                get_any_field_integer_in_struct(&Struct::new(&self.safe_buf.buf, self.loc), &field)
            }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Returns the value of any struct field as a 64-bit floating point, regardless of what type it is. Returns error if
    /// the struct doesn't match the buffer or
    /// the [field_name] doesn't match the struct or
    /// the value cannot be parsed as float.
    pub fn get_any_field_float(&self, field_name: &str) -> FlatbufferResult<f64> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe {
                get_any_field_float_in_struct(&Struct::new(&self.safe_buf.buf, self.loc), &field)
            }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Returns the string representation of any struct field value (e.g. integer 123 is returned as "123"), regardless of what type it is. Returns error if
    /// the struct doesn't match the buffer or
    /// the [field_name] doesn't match the struct.
    pub fn get_any_field_string(&self, field_name: &str) -> FlatbufferResult<String> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe {
                Ok(get_any_field_string_in_struct(
                    &Struct::new(&self.safe_buf.buf, self.loc),
                    &field,
                    self.safe_buf.schema,
                ))
            }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }
}
