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

use crate::reflection_generated::reflection::{BaseType, Field, Object, Schema};
use crate::{FlatbufferError, FlatbufferResult};
use flatbuffers::{
    ForwardsUOffset, InvalidFlatbuffer, TableVerifier, UOffsetT, Vector, Verifiable, Verifier,
    VerifierOptions, SIZE_UOFFSET, SIZE_VOFFSET,
};
use std::collections::HashMap;

/// Verifies a buffer against its schema with custom verification options.
pub fn verify_with_options(
    buffer: &[u8],
    schema: &Schema,
    opts: &VerifierOptions,
    buf_loc_to_obj_idx: &mut HashMap<usize, i32>,
) -> FlatbufferResult<()> {
    let mut verifier = Verifier::new(opts, buffer);
    if let Some(table_object) = schema.root_table() {
        if let core::result::Result::Ok(table_pos) = verifier.get_uoffset(0) {
            // Inserts -1 as object index for root table
            buf_loc_to_obj_idx.insert(table_pos.try_into()?, -1);
            let mut verified = vec![false; buffer.len()];
            return verify_table(
                &mut verifier,
                &table_object,
                table_pos.try_into()?,
                schema,
                &mut verified,
                buf_loc_to_obj_idx,
            );
        }
    }
    Err(FlatbufferError::InvalidSchema)
}

fn verify_table(
    verifier: &mut Verifier,
    table_object: &Object,
    table_pos: usize,
    schema: &Schema,
    verified: &mut [bool],
    buf_loc_to_obj_idx: &mut HashMap<usize, i32>,
) -> FlatbufferResult<()> {
    if table_pos < verified.len() && verified[table_pos] {
        return Ok(());
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
            BaseType::Vector => {
                verify_vector(table_verifier, &field, schema, verified, buf_loc_to_obj_idx)?
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
                        )?
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
    verified[table_pos] = true;
    Ok(())
}

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

fn verify_vector<'a, 'b, 'c>(
    mut table_verifier: TableVerifier<'a, 'b, 'c>,
    field: &Field,
    schema: &Schema,
    verified: &mut [bool],
    buf_loc_to_obj_idx: &mut HashMap<usize, i32>,
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
                        verify_struct(
                            verifier,
                            &child_obj,
                            struct_pos,
                            schema,
                            buf_loc_to_obj_idx,
                        )?;
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
                        let table_pos = element_pos
                            .saturating_add(verifier.get_uoffset(element_pos)?.try_into()?);
                        buf_loc_to_obj_idx.insert(table_pos, child_obj_idx);
                        verify_table(
                            verifier,
                            &child_obj,
                            table_pos,
                            schema,
                            verified,
                            buf_loc_to_obj_idx,
                        )?;
                    }
                }
            } else if field.required() {
                return InvalidFlatbuffer::new_missing_required(field.name().to_string())?;
            }
            Ok(table_verifier)
        }
        _ => {
            return Err(FlatbufferError::TypeNotSupported(
                field
                    .type_()
                    .base_type()
                    .variant_name()
                    .unwrap_or_default()
                    .to_string(),
            ))
        }
    }
}

fn verify_union<'a, 'b, 'c>(
    mut table_verifier: TableVerifier<'a, 'b, 'c>,
    field: &Field,
    union_pos: usize,
    schema: &Schema,
    verified: &mut [bool],
    buf_loc_to_obj_idx: &mut HashMap<usize, i32>,
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
                    )?
                } else {
                    verify_table(
                        table_verifier.verifier(),
                        &child_obj,
                        union_pos,
                        schema,
                        verified,
                        buf_loc_to_obj_idx,
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
                ))
            }
        }
    } else {
        return InvalidFlatbuffer::new_inconsistent_union(
            format!("{}_type", field.name()),
            field.name().to_string(),
        )?;
    }

    verified[union_pos] = true;
    Ok(table_verifier)
}
