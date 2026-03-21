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

//! Safe, schema-aware wrappers around a verified FlatBuffer binary.
//!
//! The types in this module (`[`SafeBuffer`]`, [`SafeTable`], [`SafeStruct`])
//! provide the recommended entry point for dynamic FlatBuffer access in the VTS
//! (Vendor Translation Server) and related consumers.  Because the buffer is
//! fully verified during [`SafeBuffer::new`] construction, all subsequent field
//! access methods are free of `unsafe` at the call site.

use crate::reflection_generated::reflection::{Field, Schema};
use crate::reflection_verifier::verify_with_options;
use crate::r#struct::Struct;
use crate::{
    FlatbufferError, FlatbufferResult, ForwardsUOffset, get_any_field_float,
    get_any_field_float_in_struct, get_any_field_integer, get_any_field_integer_in_struct,
    get_any_field_string, get_any_field_string_in_struct, get_any_root, get_field_float,
    get_field_integer, get_field_string, get_field_struct, get_field_struct_in_struct,
    get_field_table, get_field_vector,
};
use flatbuffers::{Follow, Table, Vector, VerifierOptions};
use num_traits::FromPrimitive;
use num_traits::float::Float;
use num_traits::int::PrimInt;
use std::collections::HashMap;

/// A verified FlatBuffer paired with its schema, enabling safe dynamic field access.
///
/// `SafeBuffer` wraps a raw byte slice and a [`Schema`] and validates the entire
/// buffer structure on construction.  Successful construction guarantees that:
///
/// - The buffer root offset is reachable.
/// - Every referenced table, struct, vector, array, and union value is within
///   bounds.
/// - Each table's object type is recorded in an internal position map so that
///   field lookups via [`SafeTable`] and [`SafeStruct`] can resolve field
///   schemas without additional unsafe operations.
///
/// Because verification happens once at construction time, all subsequent field
/// accessors on [`SafeTable`] and [`SafeStruct`] are entirely safe — no `unsafe`
/// blocks appear in their implementations.
///
/// # Lifetimes
///
/// Both `buf` and `schema` are borrowed for lifetime `'a`.  `SafeBuffer` does
/// not copy the buffer contents.
///
/// # Example
///
/// ```no_run
/// # use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
/// # use flatbuffers_reflection::reflection::Schema;
/// # fn example(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
/// let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
/// let safe = SafeBuffer::new(message_bytes, &schema)?;
/// let root = safe.get_root();
/// // Access fields by name without any unsafe code:
/// if let Some(id) = root.get_field_integer::<i32>("device_id")? {
///     println!("device_id = {id}");
/// }
/// # Ok(())
/// # }
/// ```
#[derive(Debug)]
pub struct SafeBuffer<'a> {
    buf: &'a [u8],
    schema: &'a Schema<'a>,
    buf_loc_to_obj_idx: HashMap<usize, i32>,
}

impl<'a> SafeBuffer<'a> {
    /// Constructs a `SafeBuffer` by verifying `buf` against `schema`.
    ///
    /// Uses default [`VerifierOptions`] (max depth 64, max tables 1 000 000).
    /// Returns [`FlatbufferError::VerificationError`] if the buffer fails
    /// structural validation, or [`FlatbufferError::InvalidSchema`] if the
    /// schema has no root table.
    ///
    /// Prefer this constructor unless you need to tune the DoS-protection limits
    /// described on [`SafeBuffer::new_with_options`].
    pub fn new(buf: &'a [u8], schema: &'a Schema) -> FlatbufferResult<Self> {
        Self::new_with_options(buf, schema, &VerifierOptions::default())
    }

    /// Constructs a `SafeBuffer` using custom verification options.
    ///
    /// `opts` controls the depth and table-count limits used during traversal.
    /// Tightening these limits is useful when processing untrusted input where
    /// the expected schema complexity is well-understood.
    ///
    /// Returns an error if verification fails or if the schema has no root
    /// table.  On success, the internal position map is populated and subsequent
    /// field lookups on [`SafeTable`] / [`SafeStruct`] do not require unsafe
    /// code.
    pub fn new_with_options(
        buf: &'a [u8],
        schema: &'a Schema,
        opts: &VerifierOptions,
    ) -> FlatbufferResult<Self> {
        let buf_loc_to_obj_idx = verify_with_options(buf, schema, opts)?;
        Ok(SafeBuffer {
            buf,
            schema,
            buf_loc_to_obj_idx,
        })
    }

    /// Returns the root [`SafeTable`] of this buffer.
    ///
    /// The root is the entry point for all field traversal.  Because the buffer
    /// was fully verified during construction, the returned [`SafeTable`] is
    /// guaranteed to be within bounds.
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

/// A handle to a FlatBuffer table whose position was validated by [`SafeBuffer`].
///
/// Obtained from [`SafeBuffer::get_root`] or [`SafeTable::get_field_table`].
/// All field accessors look up schema information by name — there is no need to
/// know vtable offsets or scalar sizes ahead of time.
///
/// Field lookup is O(log n) in the number of fields because the schema stores
/// fields in a sorted vector that is searched with a binary comparison.
#[derive(Debug)]
pub struct SafeTable<'a> {
    safe_buf: &'a SafeBuffer<'a>,
    loc: usize,
}

impl<'a> SafeTable<'a> {
    /// Returns the value of a typed integer field, identified by name.
    ///
    /// `T` must match the field's declared integer type exactly (e.g. `i32` for
    /// a FlatBuffers `int` field).  Returns the schema default when the field is
    /// absent from the vtable, or `None` if the default cannot be represented in
    /// `T`.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::FieldTypeMismatch`] — `size_of::<T>()` does not
    ///   match the declared field size.
    /// - [`FlatbufferError::InvalidTableOrStruct`] — this table's buffer position
    ///   was not registered during verification (should not occur in normal use).
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

    /// Returns the value of a typed floating-point field, identified by name.
    ///
    /// `T` must match the field's declared float type exactly (`f32` or `f64`).
    /// Returns the schema default when the field is absent, or `None` if the
    /// default cannot be represented in `T`.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::FieldTypeMismatch`] — `size_of::<T>()` does not match
    ///   the declared field size (e.g. `f64` for an `f32` schema field).
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

    /// Returns the value of a string field, identified by name.
    ///
    /// Returns `Some("")` (the FlatBuffers default for string fields) when the
    /// field is absent from the vtable.
    ///
    /// The returned `&str` borrows from the underlying buffer for lifetime `'a`.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::FieldTypeMismatch`] — the field is not of type `String`.
    pub fn get_field_string(&self, field_name: &str) -> FlatbufferResult<Option<&str>> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_field_string(&Table::new(&self.safe_buf.buf, self.loc), &field) }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Returns a [`SafeStruct`] handle to an inline struct field, identified by name.
    ///
    /// Returns `None` when the field is absent from the vtable.  The returned
    /// [`SafeStruct`] shares the lifetime of the parent buffer.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::FieldTypeMismatch`] — the field's `BaseType` is not `Obj`.
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

    /// Returns a typed [`flatbuffers::Vector`] field, identified by name.
    ///
    /// Returns an empty vector default when the field is absent.  The element
    /// type `T` must match the schema's declared element type exactly.
    ///
    /// The returned `Vector` borrows from the underlying buffer for lifetime `'a`.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::FieldTypeMismatch`] — field is not a `Vector`, or
    ///   `size_of::<T>()` does not match the element type size.
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

    /// Returns a [`SafeTable`] handle to a nested table field, identified by name.
    ///
    /// Returns `None` when the field is absent.  The returned [`SafeTable`]
    /// shares the lifetime of the parent buffer and has its own position entry
    /// in the verification map, so its field accessors are also safe.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::FieldTypeMismatch`] — the field's `BaseType` is not `Obj`.
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

    /// Returns any field's value coerced to `i64`, regardless of its declared type.
    ///
    /// Scalar types are cast using
    /// [`num_traits::NumCast`](https://docs.rs/num-traits/latest/num_traits/cast/trait.NumCast.html).
    /// String fields are parsed with [`str::parse`].  Returns the schema
    /// default integer when the field is absent.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::FieldTypeMismatch`] — the value cannot be represented
    ///   as `i64` (e.g. a table or vector field).
    pub fn get_any_field_integer(&self, field_name: &str) -> FlatbufferResult<i64> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_any_field_integer(&Table::new(&self.safe_buf.buf, self.loc), &field) }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Returns any field's value coerced to `f64`, regardless of its declared type.
    ///
    /// Same promotion logic as [`get_any_field_integer`] but targets `f64`.
    /// Returns the schema default real when the field is absent.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::FieldTypeMismatch`] — the value cannot be represented
    ///   as `f64`.
    pub fn get_any_field_float(&self, field_name: &str) -> FlatbufferResult<f64> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_any_field_float(&Table::new(&self.safe_buf.buf, self.loc), &field) }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Returns any field's value as a human-readable string, regardless of its declared type.
    ///
    /// Numeric types are formatted as decimal.  String fields are returned
    /// directly.  Object fields are formatted as `TypeName { field: value, ... }`.
    /// Returns an empty string when the field is absent.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
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

/// A handle to an inline FlatBuffer struct whose position was validated by [`SafeBuffer`].
///
/// Obtained from [`SafeTable::get_field_struct`] or [`SafeStruct::get_field_struct`].
/// Unlike tables, FlatBuffer structs store all fields inline without a vtable,
/// so fields are always present — there is no "absent field" concept.
#[derive(Debug)]
pub struct SafeStruct<'a> {
    safe_buf: &'a SafeBuffer<'a>,
    loc: usize,
}

impl<'a> SafeStruct<'a> {
    /// Returns a [`SafeStruct`] handle to a nested inline struct field, identified by name.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::FieldTypeMismatch`] — the field's `BaseType` is not `Obj`.
    /// - [`FlatbufferError::InvalidTableOrStruct`] — the struct's buffer position
    ///   was not registered during verification.
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

    /// Returns any struct field's value coerced to `i64`, regardless of its declared type.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::FieldTypeMismatch`] — the value cannot be represented
    ///   as `i64` (e.g. a nested object field).
    /// - [`FlatbufferError::InvalidTableOrStruct`] — the struct's position was not
    ///   registered during verification.
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

    /// Returns any struct field's value coerced to `f64`, regardless of its declared type.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::FieldTypeMismatch`] — the value cannot be represented
    ///   as `f64`.
    /// - [`FlatbufferError::InvalidTableOrStruct`] — the struct's position was not
    ///   registered during verification.
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

    /// Returns any struct field's value as a human-readable string, regardless of its declared type.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::InvalidTableOrStruct`] — the struct's position was not
    ///   registered during verification.
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
