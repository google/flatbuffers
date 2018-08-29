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

mod builder;
mod endian_scalar;
mod follow;
mod primitives;
mod push;
mod table;
mod vector;
mod vtable;
mod vtable_writer;

pub use builder::FlatBufferBuilder;
pub use endian_scalar::{EndianScalar, emplace_scalar, read_scalar, read_scalar_at, byte_swap_f32, byte_swap_f64};
pub use follow::{Follow, FollowStart};
pub use primitives::*;
pub use push::Push;
pub use table::{Table, buffer_has_identifier, get_root, get_size_prefixed_root};
pub use vector::{SafeSliceAccess, Vector, follow_cast_ref};
pub use vtable::field_index_to_field_offset;

// TODO(rw): Split fill ops in builder into fill_small, fill_big like in C++.
