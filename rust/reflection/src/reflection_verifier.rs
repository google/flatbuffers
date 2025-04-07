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

    let fields = table_object.fields();
    for field in &fields {
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
            other => {
                return Err(FlatbufferError::UnsupportedTableFieldType(other));
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
    macro_rules! visit_vec {
        ($t:ty) => {
            table_verifier
                .visit_field::<ForwardsUOffset<Vector<$t>>>(
                    field_name,
                    field.offset(),
                    field.required(),
                )
                .map_err(FlatbufferError::VerificationError)
        };
    }
    match field.type_().element() {
        BaseType::UType | BaseType::UByte => visit_vec!(u8),
        BaseType::Bool => visit_vec!(bool),
        BaseType::Byte => visit_vec!(i8),
        BaseType::Short => visit_vec!(i16),
        BaseType::UShort => visit_vec!(u16),
        BaseType::Int => visit_vec!(i32),
        BaseType::UInt => visit_vec!(u32),
        BaseType::Long => visit_vec!(i64),
        BaseType::ULong => visit_vec!(u64),
        BaseType::Float => visit_vec!(f32),
        BaseType::Double => visit_vec!(f64),
        BaseType::String => visit_vec!(ForwardsUOffset<&str>),
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
        BaseType::Union => {
            verify_vector_of_unions(table_verifier, field, schema, verified, buf_loc_to_obj_idx)
        }
        other => {
            return Err(FlatbufferError::UnsupportedVectorElementType(other));
        }
    }
}

fn verify_vector_of_unions<'a, 'b, 'c>(
    mut table_verifier: TableVerifier<'a, 'b, 'c>,
    field: &Field,
    schema: &Schema,
    verified: &mut [bool],
    buf_loc_to_obj_idx: &mut HashMap<usize, i32>,
) -> FlatbufferResult<TableVerifier<'a, 'b, 'c>> {
    let field_type = field.type_();
    // If the schema is valid, none of these asserts can fail.
    debug_assert_eq!(field_type.base_type(), BaseType::Vector);
    debug_assert_eq!(field_type.element(), BaseType::Union);
    let child_enum_idx = field_type.index();
    let child_enum = schema.enums().get(child_enum_idx.try_into()?);
    debug_assert!(!child_enum.values().is_empty());

    // Assuming the schema is valid, the previous field must be the enum vector, which consists of
    // of 1-byte enums.
    let enum_field_offset = field
        .offset()
        .checked_sub(u16::try_from(SIZE_VOFFSET).unwrap())
        .ok_or(FlatbufferError::InvalidUnionEnum)?;

    // Either both vectors must be present, or both must be absent.
    let (value_field_pos, enum_field_pos) = match (
        table_verifier.deref(field.offset())?,
        table_verifier.deref(enum_field_offset)?,
    ) {
        (Some(value_field_pos), Some(enum_field_pos)) => (value_field_pos, enum_field_pos),
        (None, None) => {
            if field.required() {
                return InvalidFlatbuffer::new_missing_required(field.name().to_owned())?;
            } else {
                return Ok(table_verifier);
            }
        }
        _ => {
            return InvalidFlatbuffer::new_inconsistent_union(
                format!("{}_type", field.name()),
                field.name().to_owned(),
            )?;
        }
    };

    let verifier = table_verifier.verifier();
    let enum_vector_offset = verifier.get_uoffset(enum_field_pos)?;
    let enum_vector_pos = enum_field_pos.saturating_add(enum_vector_offset.try_into()?);
    let enum_vector_len = verifier.get_uoffset(enum_vector_pos)?;
    let enum_vector_start = enum_vector_pos.saturating_add(SIZE_UOFFSET);

    let value_vector_offset = verifier.get_uoffset(value_field_pos)?;
    let value_vector_pos = value_field_pos.saturating_add(value_vector_offset.try_into()?);
    let value_vector_len = verifier.get_uoffset(value_vector_pos)?;
    let value_vector_start = value_vector_pos.saturating_add(SIZE_UOFFSET);

    // Both vectors should have the same length.
    // The C++ verifier instead assumes that the length of the value vector is the length of the enum vector:
    // https://github.com/google/flatbuffers/blob/bd1b2d0bafb8be6059a29487db9e5ace5c32914d/src/reflection.cpp#L147-L162
    // This has been reported at https://github.com/google/flatbuffers/issues/8567
    if enum_vector_len != value_vector_len {
        return InvalidFlatbuffer::new_inconsistent_union(
            format!("{}_type", field.name()),
            field.name().to_owned(),
        )?;
    }

    // Regardless of its contents, the value vector in a vector of unions must be a vector of
    // offsets. Source: https://github.com/dvidelabs/flatcc/blob/master/doc/binary-format.md#unions
    verifier.is_aligned::<UOffsetT>(value_vector_start)?;
    let value_vector_size = value_vector_len.saturating_mul(SIZE_UOFFSET.try_into()?);
    verifier.range_in_buffer(value_vector_start, value_vector_size.try_into()?)?;
    let value_vector_range = core::ops::Range {
        start: value_vector_start,
        end: value_vector_start.saturating_add(value_vector_size.try_into()?),
    };

    // The enums must have a size of 1 byte, so we just use the length of the vector.
    let enum_vector_size = enum_vector_len;
    verifier.range_in_buffer(enum_vector_start, enum_vector_size.try_into()?)?;
    let enum_vector_range = core::ops::Range {
        start: enum_vector_start,
        end: enum_vector_start.saturating_add(enum_vector_size.try_into()?),
    };

    let enum_values = child_enum.values();

    for (enum_pos, union_offset_pos) in
        enum_vector_range.zip(value_vector_range.step_by(SIZE_UOFFSET))
    {
        let enum_value = verifier.get_u8(enum_pos)?;
        if enum_value == 0 {
            // Discriminator is NONE. This should never happen: the C++ implementation forbids it.
            // For example, the C++ JSON parser forbids the type entry to be NONE for a vector of
            // unions: in
            // https://github.com/google/flatbuffers/blob/bd1b2d0bafb8be6059a29487db9e5ace5c32914d/src/idl_parser.cpp#L1383
            // the second argument is 'true', meaning that the NONE entry is skipped for the reverse
            // lookup.
            //
            // This should possibly be an error, but we can be forgiving and just ignore the
            // corresponding union entry entirely.
            continue;
        }

        let enum_value: usize = enum_value.into();
        let enum_type = if enum_value < enum_values.len() {
            enum_values.get(enum_value).union_type().expect(
                "Schema verification should have checked that every union enum value has a type",
            )
        } else {
            return Err(FlatbufferError::InvalidUnionEnum);
        };
        let union_pos =
            union_offset_pos.saturating_add(verifier.get_uoffset(union_offset_pos)?.try_into()?);
        verifier.in_buffer::<u8>(union_pos)?;

        match enum_type.base_type() {
            BaseType::String => <&str>::run_verifier(verifier, union_pos)?,
            BaseType::Obj => {
                let child_obj = schema.objects().get(enum_type.index().try_into()?);
                buf_loc_to_obj_idx.insert(union_pos, enum_type.index());
                if child_obj.is_struct() {
                    verify_struct(verifier, &child_obj, union_pos, schema, buf_loc_to_obj_idx)?
                } else {
                    verify_table(
                        verifier,
                        &child_obj,
                        union_pos,
                        schema,
                        verified,
                        buf_loc_to_obj_idx,
                    )?;
                }
            }
            other => {
                return Err(FlatbufferError::UnsupportedUnionElementType(other));
            }
        }
        verified[union_pos] = true;
    }
    Ok(table_verifier)
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
            other => {
                return Err(FlatbufferError::UnsupportedUnionElementType(other));
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
