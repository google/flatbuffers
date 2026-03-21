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

//! Schema-driven FlatBuffer verifier used internally by [`SafeBuffer`].
//!
//! This module is `pub(crate)`.  External callers should use [`SafeBuffer::new`]
//! or [`SafeBuffer::new_with_options`], which call [`verify_with_options`] and
//! wrap its output in a safe API.
//!
//! # DoS protections
//!
//! Maliciously crafted buffers can contain deeply nested tables or huge
//! reference graphs.  The verifier defends against these with two limits
//! supplied via [`VerifierOptions`]:
//!
//! - **`max_tables`** — The total number of distinct table positions that may be
//!   visited.  The `HashSet` that tracks visited positions is pre-allocated with
//!   `min(max_tables, 4096)` capacity to avoid large upfront allocations.
//! - **Depth / vtable limits** — Enforced transparently by the underlying
//!   [`flatbuffers::Verifier`]; [`VerifierOptions::max_depth`] and related fields
//!   control these.
//!
//! [`SafeBuffer`]: crate::safe_buffer::SafeBuffer

use crate::reflection_generated::reflection::{BaseType, Field, Object, Schema};
use crate::{FlatbufferError, FlatbufferResult, get_type_size};
use flatbuffers::{
    ForwardsUOffset, InvalidFlatbuffer, SIZE_UOFFSET, SIZE_VOFFSET, TableVerifier, UOffsetT,
    Vector, Verifiable, Verifier, VerifierOptions,
};
use std::collections::{HashMap, HashSet};

/// Verifies `buffer` against `schema` and returns a position → object-index map.
///
/// The map associates every table/struct buffer position with its schema object
/// index (`i32`).  The special sentinel value `-1` is used for the root table
/// so that it can be looked up via [`Schema::root_table`] rather than through
/// the objects vector.
///
/// This map is consumed by [`SafeBuffer`] to resolve field schemas during
/// named field lookups on [`SafeTable`] and [`SafeStruct`].
///
/// # DoS protection
///
/// The `HashSet` used to track visited table positions is pre-allocated with
/// `min(opts.max_tables, 4096)` capacity.  Once `max_tables` distinct positions
/// have been visited, verification fails with
/// [`FlatbufferError::VerificationError`]`(`[`InvalidFlatbuffer::TooManyTables`]`)`.
///
/// # Errors
///
/// - [`FlatbufferError::InvalidSchema`] — the schema has no root table, or the
///   root offset cannot be read from the buffer.
/// - [`FlatbufferError::VerificationError`] — the buffer fails structural
///   validation (out-of-bounds read, too many tables, missing required field,
///   etc.).
///
/// [`SafeBuffer`]: crate::safe_buffer::SafeBuffer
/// [`SafeTable`]: crate::safe_buffer::SafeTable
/// [`SafeStruct`]: crate::safe_buffer::SafeStruct
pub(crate) fn verify_with_options(
    buffer: &[u8],
    schema: &Schema,
    opts: &VerifierOptions,
) -> FlatbufferResult<HashMap<usize, i32>> {
    let mut verifier = Verifier::new(opts, buffer);
    let mut buf_loc_to_obj_idx = HashMap::new();
    if let Some(table_object) = schema.root_table() {
        if let core::result::Result::Ok(table_pos) = verifier.get_uoffset(0) {
            buf_loc_to_obj_idx.insert(table_pos.try_into()?, -1);
            let mut verified = HashSet::with_capacity(opts.max_tables.min(4096));
            verify_table(
                &mut verifier,
                &table_object,
                table_pos.try_into()?,
                schema,
                &mut verified,
                &mut buf_loc_to_obj_idx,
                opts.max_tables,
            )?;
            return Ok(buf_loc_to_obj_idx);
        }
    }
    Err(FlatbufferError::InvalidSchema)
}

/// Recursively verifies a table and all objects it references.
///
/// Short-circuits immediately when `table_pos` is already in `verified` so that
/// shared sub-objects (e.g. two fields pointing to the same nested table) are not
/// visited twice.
///
/// Inserts `table_pos` into `verified` after successfully processing all fields,
/// and inserts every discovered child table/struct position into
/// `buf_loc_to_obj_idx` keyed by its schema object index.
fn verify_table(
    verifier: &mut Verifier,
    table_object: &Object,
    table_pos: usize,
    schema: &Schema,
    verified: &mut HashSet<usize>,
    buf_loc_to_obj_idx: &mut HashMap<usize, i32>,
    max_tables: usize,
) -> FlatbufferResult<()> {
    if verified.contains(&table_pos) {
        return Ok(());
    }

    if verified.len() >= max_tables {
        return Err(FlatbufferError::VerificationError(
            InvalidFlatbuffer::TooManyTables,
        ));
    }

    let mut table_verifier = verifier.visit_table(table_pos)?;

    for field in &table_object.fields() {
        let field_name = field.name().to_owned();
        table_verifier = match field.type_().base_type() {
            BaseType::UType | BaseType::UByte => {
                table_verifier.visit_field::<u8>(field_name, field.offset(), field.required())?
            }
            BaseType::Bool => {
                table_verifier.visit_field::<bool>(field_name, field.offset(), field.required())?
            }
            BaseType::Byte => {
                table_verifier.visit_field::<i8>(field_name, field.offset(), field.required())?
            }
            BaseType::Short => {
                table_verifier.visit_field::<i16>(field_name, field.offset(), field.required())?
            }
            BaseType::UShort => {
                table_verifier.visit_field::<u16>(field_name, field.offset(), field.required())?
            }
            BaseType::Int => {
                table_verifier.visit_field::<i32>(field_name, field.offset(), field.required())?
            }
            BaseType::UInt => {
                table_verifier.visit_field::<u32>(field_name, field.offset(), field.required())?
            }
            BaseType::Long => {
                table_verifier.visit_field::<i64>(field_name, field.offset(), field.required())?
            }
            BaseType::ULong => {
                table_verifier.visit_field::<u64>(field_name, field.offset(), field.required())?
            }
            BaseType::Float => {
                table_verifier.visit_field::<f32>(field_name, field.offset(), field.required())?
            }
            BaseType::Double => {
                table_verifier.visit_field::<f64>(field_name, field.offset(), field.required())?
            }
            BaseType::String => table_verifier.visit_field::<ForwardsUOffset<&str>>(
                field_name,
                field.offset(),
                field.required(),
            )?,
            BaseType::Vector => verify_vector(
                table_verifier,
                &field,
                schema,
                verified,
                buf_loc_to_obj_idx,
                max_tables,
            )?,
            BaseType::Vector64 => verify_vector64(
                table_verifier,
                &field,
                schema,
                verified,
                buf_loc_to_obj_idx,
                max_tables,
            )?,
            BaseType::Array => {
                verify_array(&mut table_verifier, &field, schema, buf_loc_to_obj_idx)?;
                table_verifier
            }
            BaseType::Obj => {
                if let Some(field_pos) = table_verifier.deref(field.offset())? {
                    let object_index = field.type_().index();
                    let child_obj = schema.objects().get(object_index.try_into()?);
                    if child_obj.is_struct() {
                        buf_loc_to_obj_idx.insert(field_pos, object_index);
                        verify_struct(
                            table_verifier.verifier(),
                            &child_obj,
                            field_pos,
                            schema,
                            buf_loc_to_obj_idx,
                        )?;
                    } else {
                        let field_value = table_verifier.verifier().get_uoffset(field_pos)?;
                        let table_pos = field_pos.saturating_add(field_value.try_into()?);
                        buf_loc_to_obj_idx.insert(table_pos, object_index);
                        verify_table(
                            table_verifier.verifier(),
                            &child_obj,
                            table_pos,
                            schema,
                            verified,
                            buf_loc_to_obj_idx,
                            max_tables,
                        )?;
                    }
                } else if field.required() {
                    return InvalidFlatbuffer::new_missing_required(field.name().to_string())?;
                }
                table_verifier
            }
            BaseType::Union => {
                if let Some(field_pos) = table_verifier.deref(field.offset())? {
                    let field_value = table_verifier.verifier().get_uoffset(field_pos)?;
                    verify_union(
                        table_verifier,
                        &field,
                        field_pos.saturating_add(field_value.try_into()?),
                        schema,
                        verified,
                        buf_loc_to_obj_idx,
                        max_tables,
                    )?
                } else if field.required() {
                    return InvalidFlatbuffer::new_missing_required(field.name().to_string())?;
                } else {
                    table_verifier
                }
            }
            _ => {
                return Err(FlatbufferError::TypeNotSupported(
                    field
                        .type_()
                        .base_type()
                        .variant_name()
                        .unwrap_or_default()
                        .to_string(),
                ));
            }
        };
    }

    table_verifier.finish();
    verified.insert(table_pos);
    Ok(())
}

/// Verifies that a struct's byte range is within the buffer and recursively
/// verifies any nested struct fields.
///
/// Unlike tables, structs have a fixed, known byte size (`struct_object.bytesize()`),
/// so verification only needs to confirm that the entire range fits inside the
/// buffer.  Nested struct fields are then visited recursively so their positions
/// are recorded in `buf_loc_to_obj_idx`.
fn verify_struct(
    verifier: &mut Verifier,
    struct_object: &Object,
    struct_pos: usize,
    schema: &Schema,
    buf_loc_to_obj_idx: &mut HashMap<usize, i32>,
) -> FlatbufferResult<()> {
    verifier.range_in_buffer(struct_pos, struct_object.bytesize().try_into()?)?;
    for field in &struct_object.fields() {
        if field.type_().base_type() == BaseType::Obj {
            let obj_idx = field.type_().index();
            let child_obj = schema.objects().get(obj_idx.try_into()?);
            if child_obj.is_struct() {
                let field_pos = struct_pos.saturating_add(field.offset().into());
                buf_loc_to_obj_idx.insert(field_pos, obj_idx);
                verify_struct(verifier, &child_obj, field_pos, schema, buf_loc_to_obj_idx)?;
            }
        }
    }
    Ok(())
}

/// Verifies a fixed-length array field.
///
/// Arrays are inline in the table/struct — their total byte size is
/// `element_size * fixed_length` and they must fit within the buffer.
fn verify_array(
    table_verifier: &mut TableVerifier,
    field: &Field,
    schema: &Schema,
    buf_loc_to_obj_idx: &mut HashMap<usize, i32>,
) -> FlatbufferResult<()> {
    let field_type = field.type_();
    let fixed_length: usize = field_type.fixed_length().into();
    let element_base_type = field_type.element();

    if let Some(field_pos) = table_verifier.deref(field.offset())? {
        let element_size = if element_base_type == BaseType::Obj {
            let child_obj = schema.objects().get(field_type.index().try_into()?);
            usize::try_from(child_obj.bytesize())?
        } else {
            get_type_size(element_base_type)
        };

        let total_size = element_size.saturating_mul(fixed_length);
        table_verifier
            .verifier()
            .range_in_buffer(field_pos, total_size)?;

        if element_base_type == BaseType::Obj {
            let child_obj_idx = field_type.index();
            let child_obj = schema.objects().get(child_obj_idx.try_into()?);
            if child_obj.is_struct() {
                for i in 0..fixed_length {
                    let elem_pos = field_pos.saturating_add(i.saturating_mul(element_size));
                    buf_loc_to_obj_idx.insert(elem_pos, child_obj_idx);
                    verify_struct(
                        table_verifier.verifier(),
                        &child_obj,
                        elem_pos,
                        schema,
                        buf_loc_to_obj_idx,
                    )?;
                }
            }
        }
    } else if field.required() {
        return InvalidFlatbuffer::new_missing_required(field.name().to_string())?;
    }
    Ok(())
}

/// Verifies a 32-bit-length-prefixed vector field.
///
/// Scalar element types are handled by the underlying [`TableVerifier`] directly.
/// Object element vectors are dispatched to [`verify_vector_of_objects`].
fn verify_vector<'a, 'b, 'c>(
    mut table_verifier: TableVerifier<'a, 'b, 'c>,
    field: &Field,
    schema: &Schema,
    verified: &mut HashSet<usize>,
    buf_loc_to_obj_idx: &mut HashMap<usize, i32>,
    max_tables: usize,
) -> FlatbufferResult<TableVerifier<'a, 'b, 'c>> {
    let field_name = field.name().to_owned();
    match field.type_().element() {
        BaseType::UType | BaseType::UByte => table_verifier
            .visit_field::<ForwardsUOffset<Vector<u8>>>(
                field_name,
                field.offset(),
                field.required(),
            )
            .map_err(FlatbufferError::VerificationError),
        BaseType::Bool => table_verifier
            .visit_field::<ForwardsUOffset<Vector<bool>>>(
                field_name,
                field.offset(),
                field.required(),
            )
            .map_err(FlatbufferError::VerificationError),
        BaseType::Byte => table_verifier
            .visit_field::<ForwardsUOffset<Vector<i8>>>(
                field_name,
                field.offset(),
                field.required(),
            )
            .map_err(FlatbufferError::VerificationError),
        BaseType::Short => table_verifier
            .visit_field::<ForwardsUOffset<Vector<i16>>>(
                field_name,
                field.offset(),
                field.required(),
            )
            .map_err(FlatbufferError::VerificationError),
        BaseType::UShort => table_verifier
            .visit_field::<ForwardsUOffset<Vector<u16>>>(
                field_name,
                field.offset(),
                field.required(),
            )
            .map_err(FlatbufferError::VerificationError),
        BaseType::Int => table_verifier
            .visit_field::<ForwardsUOffset<Vector<i32>>>(
                field_name,
                field.offset(),
                field.required(),
            )
            .map_err(FlatbufferError::VerificationError),
        BaseType::UInt => table_verifier
            .visit_field::<ForwardsUOffset<Vector<u32>>>(
                field_name,
                field.offset(),
                field.required(),
            )
            .map_err(FlatbufferError::VerificationError),
        BaseType::Long => table_verifier
            .visit_field::<ForwardsUOffset<Vector<i64>>>(
                field_name,
                field.offset(),
                field.required(),
            )
            .map_err(FlatbufferError::VerificationError),
        BaseType::ULong => table_verifier
            .visit_field::<ForwardsUOffset<Vector<u64>>>(
                field_name,
                field.offset(),
                field.required(),
            )
            .map_err(FlatbufferError::VerificationError),
        BaseType::Float => table_verifier
            .visit_field::<ForwardsUOffset<Vector<f32>>>(
                field_name,
                field.offset(),
                field.required(),
            )
            .map_err(FlatbufferError::VerificationError),
        BaseType::Double => table_verifier
            .visit_field::<ForwardsUOffset<Vector<f64>>>(
                field_name,
                field.offset(),
                field.required(),
            )
            .map_err(FlatbufferError::VerificationError),
        BaseType::String => table_verifier
            .visit_field::<ForwardsUOffset<Vector<ForwardsUOffset<&str>>>>(
                field_name,
                field.offset(),
                field.required(),
            )
            .map_err(FlatbufferError::VerificationError),
        BaseType::Obj => {
            verify_vector_of_objects(
                &mut table_verifier,
                field,
                schema,
                verified,
                buf_loc_to_obj_idx,
                max_tables,
                false,
            )?;
            Ok(table_verifier)
        }
        _ => {
            return Err(FlatbufferError::TypeNotSupported(
                field
                    .type_()
                    .element()
                    .variant_name()
                    .unwrap_or_default()
                    .to_string(),
            ));
        }
    }
}

/// Verifies a Vector64 field (64-bit length prefix instead of 32-bit).
///
/// Vector64 uses a 64-bit length prefix (`u64`) but the same element layout
/// as regular vectors. We read the 64-bit length and verify the element range.
fn verify_vector64<'a, 'b, 'c>(
    mut table_verifier: TableVerifier<'a, 'b, 'c>,
    field: &Field,
    schema: &Schema,
    verified: &mut HashSet<usize>,
    buf_loc_to_obj_idx: &mut HashMap<usize, i32>,
    max_tables: usize,
) -> FlatbufferResult<TableVerifier<'a, 'b, 'c>> {
    if let Some(field_pos) = table_verifier.deref(field.offset())? {
        let verifier = table_verifier.verifier();
        let vector_offset = verifier.get_uoffset(field_pos)?;
        let vector_pos = field_pos.saturating_add(vector_offset.try_into()?);

        // Vector64 uses an 8-byte length prefix
        const SIZE_U64: usize = 8;
        verifier.range_in_buffer(vector_pos, SIZE_U64)?;
        // Read the 64-bit length byte-by-byte since Verifier doesn't expose a get_u64
        let mut len_bytes = [0u8; 8];
        for (i, byte) in len_bytes.iter_mut().enumerate() {
            *byte = verifier.get_u8(vector_pos + i)?;
        }
        let vector_len = u64::from_le_bytes(len_bytes);

        let vector_start = vector_pos.saturating_add(SIZE_U64);
        let element_base_type = field.type_().element();
        let element_size = if element_base_type == BaseType::Obj {
            let child_obj = schema.objects().get(field.type_().index().try_into()?);
            if child_obj.is_struct() {
                usize::try_from(child_obj.bytesize())?
            } else {
                SIZE_UOFFSET
            }
        } else {
            get_type_size(element_base_type)
        };

        let vector_len_usize: usize = vector_len.try_into().map_err(|_| {
            FlatbufferError::VerificationError(InvalidFlatbuffer::ApparentSizeTooLarge)
        })?;
        let total_size = element_size.saturating_mul(vector_len_usize);
        verifier.range_in_buffer(vector_start, total_size)?;

        if element_base_type == BaseType::Obj {
            let child_obj_idx = field.type_().index();
            let child_obj = schema.objects().get(child_obj_idx.try_into()?);
            if child_obj.is_struct() {
                let vector_end = vector_start.saturating_add(total_size);
                let byte_size: usize = child_obj.bytesize().try_into()?;
                let mut pos = vector_start;
                while pos < vector_end {
                    buf_loc_to_obj_idx.insert(pos, child_obj_idx);
                    verify_struct(verifier, &child_obj, pos, schema, buf_loc_to_obj_idx)?;
                    pos = pos.saturating_add(byte_size);
                }
            } else {
                verifier.is_aligned::<UOffsetT>(vector_start)?;
                let vector_end = vector_start.saturating_add(total_size);
                let mut element_pos = vector_start;
                while element_pos < vector_end {
                    let table_pos =
                        element_pos.saturating_add(verifier.get_uoffset(element_pos)?.try_into()?);
                    buf_loc_to_obj_idx.insert(table_pos, child_obj_idx);
                    verify_table(
                        verifier,
                        &child_obj,
                        table_pos,
                        schema,
                        verified,
                        buf_loc_to_obj_idx,
                        max_tables,
                    )?;
                    element_pos = element_pos.saturating_add(SIZE_UOFFSET);
                }
            }
        }
    } else if field.required() {
        return InvalidFlatbuffer::new_missing_required(field.name().to_string())?;
    }
    Ok(table_verifier)
}

/// Shared logic for verifying vectors of Obj elements (used by both Vector and Vector64).
fn verify_vector_of_objects(
    table_verifier: &mut TableVerifier,
    field: &Field,
    schema: &Schema,
    verified: &mut HashSet<usize>,
    buf_loc_to_obj_idx: &mut HashMap<usize, i32>,
    max_tables: usize,
    _is_64bit: bool,
) -> FlatbufferResult<()> {
    if let Some(field_pos) = table_verifier.deref(field.offset())? {
        let verifier = table_verifier.verifier();
        let vector_offset = verifier.get_uoffset(field_pos)?;
        let vector_pos = field_pos.saturating_add(vector_offset.try_into()?);
        let vector_len = verifier.get_uoffset(vector_pos)?;
        let vector_start = vector_pos.saturating_add(SIZE_UOFFSET);
        let child_obj_idx = field.type_().index();
        let child_obj = schema.objects().get(child_obj_idx.try_into()?);
        if child_obj.is_struct() {
            let vector_size = vector_len.saturating_mul(child_obj.bytesize().try_into()?);
            verifier.range_in_buffer(vector_start, vector_size.try_into()?)?;
            let vector_range = core::ops::Range {
                start: vector_start,
                end: vector_start.saturating_add(vector_size.try_into()?),
            };
            for struct_pos in vector_range.step_by(child_obj.bytesize().try_into()?) {
                buf_loc_to_obj_idx.insert(struct_pos, child_obj_idx);
                verify_struct(verifier, &child_obj, struct_pos, schema, buf_loc_to_obj_idx)?;
            }
        } else {
            verifier.is_aligned::<UOffsetT>(vector_start)?;
            let vector_size = vector_len.saturating_mul(SIZE_UOFFSET.try_into()?);
            verifier.range_in_buffer(vector_start, vector_size.try_into()?)?;
            let vector_range = core::ops::Range {
                start: vector_start,
                end: vector_start.saturating_add(vector_size.try_into()?),
            };
            for element_pos in vector_range.step_by(SIZE_UOFFSET) {
                let table_pos =
                    element_pos.saturating_add(verifier.get_uoffset(element_pos)?.try_into()?);
                buf_loc_to_obj_idx.insert(table_pos, child_obj_idx);
                verify_table(
                    verifier,
                    &child_obj,
                    table_pos,
                    schema,
                    verified,
                    buf_loc_to_obj_idx,
                    max_tables,
                )?;
            }
        }
    } else if field.required() {
        return InvalidFlatbuffer::new_missing_required(field.name().to_string())?;
    }
    Ok(())
}

/// Verifies a union field by reading the companion type discriminant and
/// dispatching to the appropriate string or object verifier.
///
/// The discriminant is stored one vtable slot before the union value (at
/// `field.offset() - SIZE_VOFFSET`).  An absent or malformed discriminant
/// returns [`FlatbufferError::VerificationError`] with an inconsistent-union
/// error.
fn verify_union<'a, 'b, 'c>(
    mut table_verifier: TableVerifier<'a, 'b, 'c>,
    field: &Field,
    union_pos: usize,
    schema: &Schema,
    verified: &mut HashSet<usize>,
    buf_loc_to_obj_idx: &mut HashMap<usize, i32>,
    max_tables: usize,
) -> FlatbufferResult<TableVerifier<'a, 'b, 'c>> {
    let union_enum = schema.enums().get(field.type_().index().try_into()?);
    if union_enum.values().is_empty() {
        return Err(FlatbufferError::InvalidUnionEnum);
    }

    let enum_offset = field.offset() - u16::try_from(SIZE_VOFFSET)?;
    if let Some(enum_pos) = table_verifier.deref(enum_offset)? {
        let enum_value = table_verifier.verifier().get_u8(enum_pos)?;
        let enum_type = union_enum
            .values()
            .get(enum_value.into())
            .union_type()
            .ok_or(FlatbufferError::InvalidUnionEnum)?;

        match enum_type.base_type() {
            BaseType::String => <&str>::run_verifier(table_verifier.verifier(), union_pos)?,
            BaseType::Obj => {
                let child_obj = schema.objects().get(enum_type.index().try_into()?);
                buf_loc_to_obj_idx.insert(union_pos, enum_type.index());
                if child_obj.is_struct() {
                    verify_struct(
                        table_verifier.verifier(),
                        &child_obj,
                        union_pos,
                        schema,
                        buf_loc_to_obj_idx,
                    )?;
                } else {
                    verify_table(
                        table_verifier.verifier(),
                        &child_obj,
                        union_pos,
                        schema,
                        verified,
                        buf_loc_to_obj_idx,
                        max_tables,
                    )?;
                }
            }
            _ => {
                return Err(FlatbufferError::TypeNotSupported(
                    enum_type
                        .base_type()
                        .variant_name()
                        .unwrap_or_default()
                        .to_string(),
                ));
            }
        }
    } else {
        return InvalidFlatbuffer::new_inconsistent_union(
            format!("{}_type", field.name()),
            field.name().to_string(),
        )?;
    }

    verified.insert(union_pos);
    Ok(table_verifier)
}
