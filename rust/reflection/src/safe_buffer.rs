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

use crate::reflection_generated::reflection::{BaseType, Field, Schema};
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
use std::fmt;

/// A dynamically-typed field value for generic inspection and logging.
///
/// Returned by [`SafeTable::get_any_field`] to represent any field without
/// committing to a concrete Rust type at compile time.  The [`Display`]
/// implementation produces a human-readable representation suitable for
/// logging.
///
/// [`Display`]: fmt::Display
///
/// # Examples
///
/// ```no_run
/// use flatbuffers_reflection::{SafeBuffer, FieldValue, FlatbufferResult};
/// use flatbuffers_reflection::reflection::Schema;
///
/// fn log_all_fields(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
///     let safe = SafeBuffer::new(message_bytes, &schema)?;
///     let root = safe.get_root();
///
///     for field in root.fields()? {
///         let val = root.get_any_field(field.name())?;
///         println!("{} = {}", field.name(), val);
///     }
///     Ok(())
/// }
/// ```
#[derive(Debug, Clone)]
pub enum FieldValue<'a> {
    /// A boolean field value.
    Bool(bool),
    /// An integer field value, widened to `i64`.
    Integer(i64),
    /// A floating-point field value, widened to `f64`.
    Float(f64),
    /// A string field value borrowed from the buffer.
    String(&'a str),
    /// A nested table field value.
    Table(SafeTable<'a>),
    /// An inline struct field value.
    Struct(SafeStruct<'a>),
    /// A vector of integers (all integer element types are widened to `i64`).
    VecInteger(Vec<i64>),
    /// A vector of floats (all float element types are widened to `f64`).
    VecFloat(Vec<f64>),
    /// A vector of strings.
    VecString(Vec<&'a str>),
    /// A union field with its discriminant byte and the nested table.
    Union(u8, Box<SafeTable<'a>>),
    /// The field is absent from the buffer (not present in the vtable).
    Absent,
}

impl fmt::Display for FieldValue<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::Bool(v) => write!(f, "{v}"),
            Self::Integer(v) => write!(f, "{v}"),
            Self::Float(v) => write!(f, "{v}"),
            Self::String(v) => write!(f, "{v}"),
            Self::Table(_) => write!(f, "<table>"),
            Self::Struct(_) => write!(f, "<struct>"),
            Self::VecInteger(v) => {
                write!(f, "[")?;
                for (i, n) in v.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{n}")?;
                }
                write!(f, "]")
            }
            Self::VecFloat(v) => {
                write!(f, "[")?;
                for (i, n) in v.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{n}")?;
                }
                write!(f, "]")
            }
            Self::VecString(v) => {
                write!(f, "[")?;
                for (i, s) in v.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{s}")?;
                }
                write!(f, "]")
            }
            Self::Union(discriminant, _) => write!(f, "<union type={discriminant}>"),
            Self::Absent => write!(f, "<absent>"),
        }
    }
}

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
    ///
    /// # Examples
    ///
    /// Load a binary schema (`.bfbs`) and a message buffer, then verify and
    /// access fields by name.  A schema is generated by `flatc --binary`:
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferError, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn inspect_elevator_status(
    ///     schema_bytes: &[u8],
    ///     message_bytes: &[u8],
    /// ) -> FlatbufferResult<()> {
    ///     // Parse the binary schema produced by `flatc --binary elevator_status.fbs`.
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes)
    ///         .map_err(FlatbufferError::VerificationError)?;
    ///
    ///     // Verify the message against the schema — returns an error if the
    ///     // buffer is malformed or the wrong schema was supplied.
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///
    ///     let root = safe.get_root();
    ///
    ///     // Read a typed integer field.
    ///     if let Some(floor) = root.get_field_integer::<i32>("current_floor")? {
    ///         println!("elevator is at floor {floor}");
    ///     }
    ///
    ///     // Read a string field.
    ///     if let Some(unit_id) = root.get_field_string("unit_id")? {
    ///         println!("unit: {unit_id}");
    ///     }
    ///
    ///     Ok(())
    /// }
    /// ```
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
    ///
    /// # Examples
    ///
    /// Obtain the root table and read several fields of different types:
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn log_fault_message(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///
    ///     // get_root() never fails — verification already happened in new().
    ///     let root = safe.get_root();
    ///
    ///     let fault_code = root.get_field_integer::<u32>("fault_code")?;
    ///     let description = root.get_field_string("description")?;
    ///     let severity = root.get_field_float::<f32>("severity")?;
    ///
    ///     println!(
    ///         "fault {:?}: {:?} (severity {:?})",
    ///         fault_code, description, severity
    ///     );
    ///     Ok(())
    /// }
    /// ```
    pub fn get_root(&self) -> SafeTable<'_> {
        // SAFETY: the buffer was verified during construction.
        let table = unsafe { get_any_root(self.buf) };

        SafeTable {
            safe_buf: self,
            loc: table.loc(),
        }
    }

    /// Returns the schema's file identifier string, or `None` if absent.
    ///
    /// The file identifier is the four-character string declared with
    /// `file_identifier` in the `.fbs` source (e.g. `"MONS"`).
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn check_ident(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     if let Some(ident) = safe.file_ident() {
    ///         println!("schema file identifier: {ident}");
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn file_ident(&self) -> Option<&str> {
        self.schema.file_ident()
    }

    /// Returns the schema's file extension string, or `None` if absent.
    ///
    /// The file extension is declared with `file_extension` in the `.fbs`
    /// source (e.g. `"mon"`).
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn check_ext(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     if let Some(ext) = safe.file_ext() {
    ///         println!("schema file extension: {ext}");
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn file_ext(&self) -> Option<&str> {
        self.schema.file_ext()
    }

    /// Returns all enum and union definitions in the schema.
    ///
    /// The returned slice is in the order they appear in the compiled schema —
    /// typically alphabetical by fully qualified name.
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn list_enums(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     for e in safe.enums() {
    ///         println!("enum: {}", e.name());
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn enums(&self) -> Vec<crate::reflection_generated::reflection::Enum<'a>> {
        let v = self.schema.enums();
        (0..v.len()).map(|i| v.get(i)).collect()
    }

    /// Finds an enum or union definition by its fully qualified name.
    ///
    /// Returns `None` if no enum with that name exists in the schema.
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn find_color_enum(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     if let Some(color) = safe.enum_by_name("MyGame.Example.Color") {
    ///         println!("found enum with {} values", color.values().len());
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn enum_by_name(
        &self,
        name: &str,
    ) -> Option<crate::reflection_generated::reflection::Enum<'a>> {
        self.schema
            .enums()
            .lookup_by_key(name, |e, key| e.key_compare_with_value(key))
    }

    /// Looks up the string name of an enum value by enum name and integer value.
    ///
    /// Returns `None` if the enum is not found or if no value with the given
    /// integer is defined.
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn color_name(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     if let Some(name) = safe.enum_value_name("MyGame.Example.Color", 2) {
    ///         println!("color with value 2 is named: {name}");
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn enum_value_name(&self, enum_name: &str, value: i64) -> Option<&str> {
        let en = self.enum_by_name(enum_name)?;
        let vals = en.values();
        for i in 0..vals.len() {
            let v = vals.get(i);
            if v.value() == value {
                return Some(v.name());
            }
        }
        None
    }

    /// Returns all object (table and struct) definitions in the schema.
    ///
    /// The returned `Vec` is in the order they appear in the compiled schema —
    /// typically alphabetical by fully qualified name.
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn list_objects(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     for obj in safe.objects() {
    ///         println!("object: {}", obj.name());
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn objects(&self) -> Vec<crate::reflection_generated::reflection::Object<'a>> {
        let v = self.schema.objects();
        (0..v.len()).map(|i| v.get(i)).collect()
    }

    /// Finds an object definition by its fully qualified name.
    ///
    /// Returns `None` if no object with that name exists in the schema.
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn find_monster(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     if let Some(obj) = safe.object_by_name("MyGame.Example.Monster") {
    ///         println!("Monster has {} fields", obj.fields().len());
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn object_by_name(
        &self,
        name: &str,
    ) -> Option<crate::reflection_generated::reflection::Object<'a>> {
        self.schema
            .objects()
            .lookup_by_key(name, |o, key| o.key_compare_with_value(key))
    }

    fn find_field_by_name(
        &self,
        buf_loc: usize,
        field_name: &str,
    ) -> FlatbufferResult<Option<Field<'a>>> {
        Ok(self
            .get_all_fields(buf_loc)?
            .lookup_by_key(field_name, |field: &Field<'_>, key| {
                field.key_compare_with_value(key)
            }))
    }

    fn get_all_fields(
        &self,
        buf_loc: usize,
    ) -> FlatbufferResult<Vector<'a, ForwardsUOffset<Field<'a>>>> {
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

    fn get_object_for_loc(
        &self,
        buf_loc: usize,
    ) -> FlatbufferResult<crate::reflection_generated::reflection::Object<'a>> {
        if let Some(&obj_idx) = self.buf_loc_to_obj_idx.get(&buf_loc) {
            let obj = if obj_idx == -1 {
                self.schema.root_table().unwrap()
            } else {
                self.schema.objects().get(obj_idx.try_into()?)
            };
            Ok(obj)
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
#[derive(Debug, Clone)]
pub struct SafeTable<'a> {
    safe_buf: &'a SafeBuffer<'a>,
    loc: usize,
}

impl<'a> SafeTable<'a> {
    /// Returns the schema [`Object`] definition for this table.
    ///
    /// Useful for iterating over all declared fields or inspecting metadata
    /// (e.g. `is_struct`, `name`, `declaration_file`).
    ///
    /// [`Object`]: crate::reflection_generated::reflection::Object
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::InvalidTableOrStruct`] — the table's buffer position
    ///   was not registered during verification (should not occur in normal use).
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn print_table_name(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     let root = safe.get_root();
    ///     println!("root table type: {}", root.object()?.name());
    ///     Ok(())
    /// }
    /// ```
    pub fn object(&self) -> FlatbufferResult<crate::reflection_generated::reflection::Object<'a>> {
        self.safe_buf.get_object_for_loc(self.loc)
    }

    /// Returns all field definitions for this table from the schema.
    ///
    /// The returned `Vec` is in alphabetical order (as stored in the compiled
    /// schema).  Use this to iterate all fields without knowing their names in
    /// advance.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::InvalidTableOrStruct`] — the table's buffer position
    ///   was not registered during verification.
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn list_field_names(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     let root = safe.get_root();
    ///     for field in root.fields()? {
    ///         println!("  field: {}", field.name());
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn fields(
        &self,
    ) -> FlatbufferResult<Vec<crate::reflection_generated::reflection::Field<'a>>> {
        let all = self.safe_buf.get_all_fields(self.loc)?;
        Ok((0..all.len()).map(|i| all.get(i)).collect())
    }

    /// Reads any field as a [`FieldValue`] for generic inspection.
    ///
    /// Dispatches on the field's `BaseType` from the schema to call the
    /// appropriate typed accessor, then wraps the result in the [`FieldValue`]
    /// enum.  This is the recommended entry point for generic logging,
    /// serialisation, or protocol translation where the field type is not known
    /// at compile time.
    ///
    /// Vector fields with numeric element types are returned as
    /// [`FieldValue::VecInteger`] or [`FieldValue::VecFloat`].  Vector fields
    /// with `String` elements are returned as [`FieldValue::VecString`].  All
    /// other vector element types (nested tables, structs, etc.) return
    /// [`FlatbufferError::TypeNotSupported`].
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::TypeNotSupported`] — the field's `BaseType` has no
    ///   dynamic representation (e.g. `Array`, `Vector64`).
    /// - [`FlatbufferError::InvalidTableOrStruct`] — table position not registered.
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FieldValue, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn inspect_field(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     let root = safe.get_root();
    ///     match root.get_any_field("hp")? {
    ///         FieldValue::Integer(n) => println!("hp = {n}"),
    ///         FieldValue::Absent => println!("hp absent"),
    ///         other => println!("unexpected: {other}"),
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn get_any_field(&self, field_name: &str) -> FlatbufferResult<FieldValue<'a>> {
        let field = self
            .safe_buf
            .find_field_by_name(self.loc, field_name)?
            .ok_or(FlatbufferError::FieldNotFound)?;

        let base_type = field.type_().base_type();
        match base_type {
            BaseType::Bool => {
                // SAFETY: the buffer was verified during construction.
                let raw = unsafe {
                    get_field_integer::<u8>(&Table::new(self.safe_buf.buf, self.loc), &field)
                }?;
                Ok(raw.map_or(FieldValue::Absent, |v| FieldValue::Bool(v != 0)))
            }
            BaseType::Byte
            | BaseType::UByte
            | BaseType::Short
            | BaseType::UShort
            | BaseType::Int
            | BaseType::UInt
            | BaseType::Long
            | BaseType::ULong
            | BaseType::UType => {
                // SAFETY: the buffer was verified during construction.
                let val = unsafe {
                    get_any_field_integer(&Table::new(self.safe_buf.buf, self.loc), &field)
                }?;
                Ok(FieldValue::Integer(val))
            }
            BaseType::Float | BaseType::Double => {
                // SAFETY: the buffer was verified during construction.
                let val = unsafe {
                    get_any_field_float(&Table::new(self.safe_buf.buf, self.loc), &field)
                }?;
                Ok(FieldValue::Float(val))
            }
            BaseType::String => {
                // SAFETY: the buffer was verified during construction.
                let opt =
                    unsafe { get_field_string(&Table::new(self.safe_buf.buf, self.loc), &field) }?;
                Ok(opt.map_or(FieldValue::Absent, FieldValue::String))
            }
            BaseType::Obj => {
                // SAFETY: the buffer was verified during construction.
                let opt =
                    unsafe { get_field_table(&Table::new(self.safe_buf.buf, self.loc), &field) }?;
                Ok(opt.map_or(FieldValue::Absent, |t| {
                    FieldValue::Table(SafeTable {
                        safe_buf: self.safe_buf,
                        loc: t.loc(),
                    })
                }))
            }
            BaseType::Union => {
                let result = self.get_field_union(field_name)?;
                Ok(result.map_or(FieldValue::Absent, |(disc, tbl)| {
                    FieldValue::Union(disc, Box::new(tbl))
                }))
            }
            BaseType::Vector => {
                let elem_type = field.type_().element();
                self.read_vector_field(&field, elem_type)
            }
            _ => Err(FlatbufferError::TypeNotSupported(format!("{base_type:?}"))),
        }
    }

    /// Reads a union field, returning the discriminant type byte and the nested table.
    ///
    /// FlatBuffer union fields are paired with a companion `{name}_type` field
    /// that holds the discriminant (a `u8` enum value).  This method reads both
    /// fields and returns them as a tuple, or `None` if the union is absent.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::InvalidTableOrStruct`] — table position not registered.
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn read_union(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     let root = safe.get_root();
    ///     if let Some((disc, tbl)) = root.get_field_union("test")? {
    ///         println!("union type discriminant: {disc}");
    ///         let _ = tbl; // tbl is a SafeTable for the active variant
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn get_field_union(
        &self,
        field_name: &str,
    ) -> FlatbufferResult<Option<(u8, SafeTable<'a>)>> {
        let type_field_name = format!("{field_name}_type");
        let disc_field = self
            .safe_buf
            .find_field_by_name(self.loc, &type_field_name)?
            .ok_or(FlatbufferError::FieldNotFound)?;

        // SAFETY: the buffer was verified during construction.
        let disc_opt = unsafe {
            get_field_integer::<u8>(&Table::new(self.safe_buf.buf, self.loc), &disc_field)
        }?;
        let Some(disc) = disc_opt else {
            return Ok(None);
        };
        if disc == 0 {
            return Ok(None);
        }

        let value_field = self
            .safe_buf
            .find_field_by_name(self.loc, field_name)?
            .ok_or(FlatbufferError::FieldNotFound)?;

        // Union value fields have BaseType::Union — get_field_table only accepts
        // BaseType::Obj, so we read the table offset directly from the vtable slot.
        // SAFETY: the buffer was verified during construction.
        let opt_tbl: Option<Table<'a>> = unsafe {
            Table::new(self.safe_buf.buf, self.loc)
                .get::<ForwardsUOffset<Table<'a>>>(value_field.offset(), None)
        };
        Ok(opt_tbl.map(|t| {
            (
                disc,
                SafeTable {
                    safe_buf: self.safe_buf,
                    loc: t.loc(),
                },
            )
        }))
    }

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
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferError, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn read_device_id(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     let root = safe.get_root();
    ///
    ///     // The type parameter must match the schema's declared field type.
    ///     // Using i64 for a schema `int` (32-bit) field returns FieldTypeMismatch.
    ///     match root.get_field_integer::<i32>("device_id")? {
    ///         Some(id) => println!("device_id = {id}"),
    ///         None => println!("device_id not present (schema default cannot fit in i32)"),
    ///     }
    ///
    ///     // u8 for a FlatBuffers `ubyte` field:
    ///     if let Some(flags) = root.get_field_integer::<u8>("status_flags")? {
    ///         println!("flags = 0x{flags:02x}");
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn get_field_integer<T: for<'b> Follow<'b, Inner = T> + PrimInt + FromPrimitive>(
        &self,
        field_name: &str,
    ) -> FlatbufferResult<Option<T>> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_field_integer::<T>(&Table::new(self.safe_buf.buf, self.loc), &field) }
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
            unsafe { get_field_float::<T>(&Table::new(self.safe_buf.buf, self.loc), &field) }
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
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn print_unit_label(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     let root = safe.get_root();
    ///
    ///     // Returns None only when the default cannot be represented (never for strings).
    ///     if let Some(label) = root.get_field_string("unit_label")? {
    ///         // The returned &str borrows from the message buffer — no allocation.
    ///         println!("unit label: {label}");
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn get_field_string(&self, field_name: &str) -> FlatbufferResult<Option<&str>> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            unsafe { get_field_string(&Table::new(self.safe_buf.buf, self.loc), &field) }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Returns the value of a boolean field, identified by name.
    ///
    /// FlatBuffers booleans are stored as a single byte; this method reads that
    /// byte and returns `true` for any non-zero value.  Returns `None` when the
    /// field is absent and the schema default cannot be determined.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::FieldTypeMismatch`] — the field is not of type `Bool`.
    /// - [`FlatbufferError::InvalidTableOrStruct`] — table position not registered.
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn check_flag(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     let root = safe.get_root();
    ///     if let Some(enabled) = root.get_field_bool("testbool")? {
    ///         println!("testbool = {enabled}");
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn get_field_bool(&self, field_name: &str) -> FlatbufferResult<Option<bool>> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            let raw = unsafe {
                get_field_integer::<u8>(&Table::new(self.safe_buf.buf, self.loc), &field)
            }?;
            Ok(raw.map(|v| v != 0))
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
                unsafe { get_field_struct(&Table::new(self.safe_buf.buf, self.loc), &field)? };
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
            unsafe { get_field_vector(&Table::new(self.safe_buf.buf, self.loc), &field) }
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
    ///
    /// # Examples
    ///
    /// Traverse a nested table to read fields from a sub-message:
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn read_position(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     let root = safe.get_root();
    ///
    ///     // Drill into a nested table (e.g. an ElevatorStatus containing a Position table).
    ///     if let Some(position) = root.get_field_table("position")? {
    ///         if let Some(floor) = position.get_field_integer::<i32>("floor")? {
    ///             println!("floor = {floor}");
    ///         }
    ///         if let Some(speed) = position.get_field_float::<f32>("speed_mps")? {
    ///             println!("speed = {speed:.2} m/s");
    ///         }
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn get_field_table(&self, field_name: &str) -> FlatbufferResult<Option<SafeTable<'a>>> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            let optional_table =
                unsafe { get_field_table(&Table::new(self.safe_buf.buf, self.loc), &field)? };
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
            unsafe { get_any_field_integer(&Table::new(self.safe_buf.buf, self.loc), &field) }
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
            unsafe { get_any_field_float(&Table::new(self.safe_buf.buf, self.loc), &field) }
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
                    &Table::new(self.safe_buf.buf, self.loc),
                    &field,
                    self.safe_buf.schema,
                ))
            }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Helper: read a `Vector` field and convert all elements to `FieldValue`.
    fn read_vector_field(
        &self,
        field: &Field<'_>,
        elem_type: BaseType,
    ) -> FlatbufferResult<FieldValue<'a>> {
        match elem_type {
            BaseType::Bool
            | BaseType::Byte
            | BaseType::UByte
            | BaseType::Short
            | BaseType::UShort
            | BaseType::Int
            | BaseType::UInt
            | BaseType::Long
            | BaseType::ULong
            | BaseType::UType => {
                // Read as u8 vector and widen each element via get_any_field_integer per-element
                // is not available; instead read the vector as raw bytes and do element reads.
                // The simplest cross-size approach: use the existing get_field_vector for u8-sized
                // variants, u16 for Short/UShort, etc.  Because we need a single code path,
                // we read with get_any_field_integer which gives us the whole-field i64 value —
                // but that only works for scalars, not vectors.
                //
                // For vectors we read the raw Vector and widen each element individually.
                let result = self.read_integer_vec(field, elem_type)?;
                Ok(FieldValue::VecInteger(result))
            }
            BaseType::Float => {
                // SAFETY: buffer verified.
                let opt = unsafe {
                    get_field_vector::<f32>(&Table::new(self.safe_buf.buf, self.loc), field)
                }?;
                Ok(FieldValue::VecFloat(opt.map_or_else(Vec::new, |v| {
                    (0..v.len()).map(|i| f64::from(v.get(i))).collect()
                })))
            }
            BaseType::Double => {
                // SAFETY: buffer verified.
                let opt = unsafe {
                    get_field_vector::<f64>(&Table::new(self.safe_buf.buf, self.loc), field)
                }?;
                Ok(FieldValue::VecFloat(opt.map_or_else(Vec::new, |v| {
                    (0..v.len()).map(|i| v.get(i)).collect()
                })))
            }
            BaseType::String => {
                // SAFETY: buffer verified.  String vectors use ForwardsUOffset<&str>
                // elements, which do not satisfy Follow::Inner = T, so we use
                // table.get directly instead of get_field_vector.
                let tbl = unsafe { Table::new(self.safe_buf.buf, self.loc) };
                let opt: Option<Vector<'a, ForwardsUOffset<&'a str>>> = unsafe {
                    tbl.get::<ForwardsUOffset<Vector<'a, ForwardsUOffset<&'a str>>>>(
                        field.offset(),
                        None,
                    )
                };
                Ok(FieldValue::VecString(opt.map_or_else(Vec::new, |v| {
                    (0..v.len()).map(|i| v.get(i)).collect()
                })))
            }
            _ => Err(FlatbufferError::TypeNotSupported(format!("{elem_type:?}"))),
        }
    }

    fn read_integer_vec(
        &self,
        field: &Field<'_>,
        elem_type: BaseType,
    ) -> FlatbufferResult<Vec<i64>> {
        match elem_type {
            BaseType::Bool | BaseType::Byte | BaseType::UByte | BaseType::UType => {
                // SAFETY: buffer verified.
                let opt = unsafe {
                    get_field_vector::<u8>(&Table::new(self.safe_buf.buf, self.loc), field)
                }?;
                Ok(opt.map_or_else(Vec::new, |v| {
                    (0..v.len()).map(|i| i64::from(v.get(i))).collect()
                }))
            }
            BaseType::Short => {
                // SAFETY: buffer verified.
                let opt = unsafe {
                    get_field_vector::<i16>(&Table::new(self.safe_buf.buf, self.loc), field)
                }?;
                Ok(opt.map_or_else(Vec::new, |v| {
                    (0..v.len()).map(|i| i64::from(v.get(i))).collect()
                }))
            }
            BaseType::UShort => {
                // SAFETY: buffer verified.
                let opt = unsafe {
                    get_field_vector::<u16>(&Table::new(self.safe_buf.buf, self.loc), field)
                }?;
                Ok(opt.map_or_else(Vec::new, |v| {
                    (0..v.len()).map(|i| i64::from(v.get(i))).collect()
                }))
            }
            BaseType::Int => {
                // SAFETY: buffer verified.
                let opt = unsafe {
                    get_field_vector::<i32>(&Table::new(self.safe_buf.buf, self.loc), field)
                }?;
                Ok(opt.map_or_else(Vec::new, |v| {
                    (0..v.len()).map(|i| i64::from(v.get(i))).collect()
                }))
            }
            BaseType::UInt => {
                // SAFETY: buffer verified.
                let opt = unsafe {
                    get_field_vector::<u32>(&Table::new(self.safe_buf.buf, self.loc), field)
                }?;
                Ok(opt.map_or_else(Vec::new, |v| {
                    (0..v.len()).map(|i| i64::from(v.get(i))).collect()
                }))
            }
            BaseType::Long => {
                // SAFETY: buffer verified.
                let opt = unsafe {
                    get_field_vector::<i64>(&Table::new(self.safe_buf.buf, self.loc), field)
                }?;
                Ok(opt.map_or_else(Vec::new, |v| (0..v.len()).map(|i| v.get(i)).collect()))
            }
            BaseType::ULong => {
                // SAFETY: buffer verified.
                let opt = unsafe {
                    get_field_vector::<u64>(&Table::new(self.safe_buf.buf, self.loc), field)
                }?;
                Ok(opt.map_or_else(Vec::new, |v| {
                    (0..v.len()).map(|i| v.get(i) as i64).collect()
                }))
            }
            _ => Err(FlatbufferError::TypeNotSupported(format!("{elem_type:?}"))),
        }
    }
}

/// A handle to an inline FlatBuffer struct whose position was validated by [`SafeBuffer`].
///
/// Obtained from [`SafeTable::get_field_struct`] or [`SafeStruct::get_field_struct`].
/// Unlike tables, FlatBuffer structs store all fields inline without a vtable,
/// so fields are always present — there is no "absent field" concept.
#[derive(Debug, Clone)]
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
                get_field_struct_in_struct(&Struct::new(self.safe_buf.buf, self.loc), &field)?
            };
            Ok(SafeStruct {
                safe_buf: self.safe_buf,
                loc: st.loc(),
            })
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }

    /// Returns the value of a boolean field in this struct, identified by name.
    ///
    /// FlatBuffers booleans are stored as a single byte; this method reads that
    /// byte and returns `true` for any non-zero value.
    ///
    /// # Errors
    ///
    /// - [`FlatbufferError::FieldNotFound`] — `field_name` is not in the schema.
    /// - [`FlatbufferError::FieldTypeMismatch`] — the field is not of type `Bool`.
    /// - [`FlatbufferError::InvalidTableOrStruct`] — struct position not registered.
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use flatbuffers_reflection::{SafeBuffer, FlatbufferResult};
    /// use flatbuffers_reflection::reflection::Schema;
    ///
    /// fn check_struct_bool(schema_bytes: &[u8], message_bytes: &[u8]) -> FlatbufferResult<()> {
    ///     let schema = flatbuffers::root::<Schema>(schema_bytes).unwrap();
    ///     let safe = SafeBuffer::new(message_bytes, &schema)?;
    ///     let root = safe.get_root();
    ///     if let Some(pos) = root.get_field_struct("pos")? {
    ///         println!("{}", pos.get_field_bool("some_bool_field")?);
    ///     }
    ///     Ok(())
    /// }
    /// ```
    pub fn get_field_bool(&self, field_name: &str) -> FlatbufferResult<bool> {
        if let Some(field) = self.safe_buf.find_field_by_name(self.loc, field_name)? {
            // SAFETY: the buffer was verified during construction.
            let raw = unsafe {
                get_any_field_integer_in_struct(&Struct::new(self.safe_buf.buf, self.loc), &field)
            }?;
            Ok(raw != 0)
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
                get_any_field_integer_in_struct(&Struct::new(self.safe_buf.buf, self.loc), &field)
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
                get_any_field_float_in_struct(&Struct::new(self.safe_buf.buf, self.loc), &field)
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
                    &Struct::new(self.safe_buf.buf, self.loc),
                    &field,
                    self.safe_buf.schema,
                ))
            }
        } else {
            Err(FlatbufferError::FieldNotFound)
        }
    }
}
