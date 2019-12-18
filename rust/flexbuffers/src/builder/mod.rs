// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

use crate::bitwidth::{align, BitWidth};
mod value;
use crate::FlexBufferType;
use std::cmp::max;
use value::{find_vector_type, store_value, Value};
mod map;
mod push;
mod ser;
mod vector;
pub use map::MapBuilder;
use map::{read_trusted_str, sort_map_by_keys};
pub use push::Pushable;
pub use ser::FlexbufferSerializer;
pub use vector::VectorBuilder;

macro_rules! push_slice {
    ($push_name: ident, $scalar: ty, $Val: ident, $new_vec: ident) => {
        fn $push_name<T, S>(&mut self, xs: S)
        where
            T: Into<$scalar> + Copy,
            S: AsRef<[T]>
        {
            let mut value = Value::$new_vec(xs.as_ref().len());
            let mut width = xs.as_ref()
                .iter()
                .map(|x| {
                    BitWidth::from((*x).into())
                })
                .max()
                .unwrap_or_default();
            if !value.is_fixed_length_vector() {
                let length = Value::UInt(xs.as_ref().len() as u64);
                width = std::cmp::max(width, length.width_or_child_width());
                align(&mut self.buffer, width);
                store_value(&mut self.buffer, length, width);
            } else {
                align(&mut self.buffer, width);
            }
            let address = self.buffer.len();
            for &x in xs.as_ref().iter() {
                store_value(&mut self.buffer, Value::$Val(x.into()), width);
            }
            value.set_address_or_panic(address);
            value.set_child_width_or_panic(width);
            self.values.push(value);
        }
    }
}
macro_rules! push_indirect {
    ($push_name: ident, $scalar: ty, $Direct: ident, $Indirect: ident) => {
        fn $push_name<T: Into<$scalar>>(&mut self, x: T) {
            let x = Value::$Direct(x.into());
            let child_width = x.width_or_child_width();
            let address = self.buffer.len();
            store_value(&mut self.buffer, x, child_width);
            self.values.push(
                Value::Reference {
                    address,
                    child_width,
                    fxb_type: FlexBufferType::$Indirect,
                }
            );
        }
    }
}
#[derive(Debug, Clone)]
struct CachedKey {
    address: usize,
    length: usize,
}
/// **Use this struct to build a Flexbuffer.**
///
/// Flexbuffers may only have a single root value, which may be constructed
/// with  one of the following functions.
/// * `build_singleton` will push 1 value to the buffer and serialize it as the root.
/// * `build_vector` returns a `VectorBuilder`, into which many (potentially
/// heterogenous) values can be pushed. The vector itself is the root and is serialized
/// when the `VectorBuilder` is dropped (or `end` is called).
/// * `build_map` returns a `MapBuilder`, which is similar to a `VectorBuilder` except
/// every value must be pushed with an associated key. The map is serialized when the
/// `MapBuilder` is dropped (or `end` is called).
///
/// These functions reset and overwrite the Builder which means, while there are no
/// active `MapBuilder` or `VectorBuilder`, the internal buffer is empty or contains a
/// finished Flexbuffer. The internal buffer is accessed with `view`.
///
/// The default builder interns keys. In the future, there should be string interning,
/// and potentially key-vector interning.
// TODO(cneo): Stablize options before 1.0.
#[derive(Debug, Clone)]
pub struct Builder {
    buffer: Vec<u8>,
    values: Vec<Value>,
    key_pool: Option<Vec<CachedKey>>,
}
impl Default for Builder {
    fn default() -> Self {
        Builder {
            key_pool: Some(Vec::new()),
            values: Vec::new(),
            buffer: Vec::new(),
        }
    }
}


impl<'a> Builder {
    pub fn new() -> Self {
        Self::default()
    }
    /// Shows the internal flexbuffer. It will be empty on initialization or if `reset` was called.
    /// Otherwise after `build_map`, `build_vector`, or `build_singleton` the internal flexbuffer
    /// will be completed.
    pub fn view(&self) -> &[u8] {
        &self.buffer
    }
    /// Resets the internal state. Automatically called before building a new flexbuffer.
    pub fn reset(&mut self) {
        self.buffer.truncate(0);
        self.values.truncate(0);
        if let Some(pool) = self.key_pool.as_mut() {
            pool.truncate(0);
        }
    }
    fn push_key(&mut self, key: &str) {
        assert!(
            key.bytes().all(|b| b != b'\0'),
            "Keys must not have internal nulls."
        );
        // Search key pool if there is one.
        let found = self.key_pool.as_ref().map(|pool| {
            pool.binary_search_by(|ck| {
                let old_key = unsafe { read_trusted_str(&self.buffer, ck.address, ck.length) };
                old_key.cmp(key)
            })
        });
        let address = if let Some(Ok(idx)) = found {
            // Found key in key pool.
            self.key_pool.as_ref().unwrap()[idx].address
        } else {
            // Key not in pool (or no pool).
            let address = self.buffer.len();
            self.buffer.extend(key.bytes());
            self.buffer.push(b'\0');
            address
        };
        let length = key.len();
        if let Some(Err(idx)) = found {
            // Insert into key pool.
            let pool = self.key_pool.as_mut().unwrap();
            pool.insert(idx, CachedKey { address, length });
        }
        self.values.push(Value::Key { address, length });
    }
    fn push_uint<T: Into<u64>>(&mut self, x: T) {
        self.values.push(Value::UInt(x.into()));
    }
    fn push_int<T: Into<i64>>(&mut self, x: T) {
        self.values.push(Value::Int(x.into()));
    }
    fn push_float<T: Into<f64>>(&mut self, x: T) {
        self.values.push(Value::Float(x.into()));
    }
    fn push_null(&mut self) {
        self.values.push(Value::Null);
    }
    fn push_bool(&mut self, x: bool) {
        self.values.push(Value::Bool(x));
    }
    fn store_blob(&mut self, xs: &[u8]) -> Value {
        let length = Value::UInt(xs.len() as u64);
        let width = length.width_or_child_width();
        align(&mut self.buffer, width);
        store_value(&mut self.buffer, length, width);
        let address = self.buffer.len();
        self.buffer.extend(xs);
        Value::Reference {
            fxb_type: FlexBufferType::Blob,
            address,
            child_width: width,
        }
    }
    fn push_str(&mut self, x: &str) {
        let mut string = self.store_blob(x.as_bytes());
        self.buffer.push(b'\0');
        string.set_fxb_type_or_panic(FlexBufferType::String);
        self.values.push(string);
    }
    fn push_blob(&mut self, x: &[u8]) {
        let blob = self.store_blob(x);
        self.values.push(blob);
    }
    fn push_bools(&mut self, xs: &[bool]) {
        let length = Value::UInt(xs.len() as u64);
        let width = length.width_or_child_width();
        align(&mut self.buffer, width);
        store_value(&mut self.buffer, length, width);
        let address = self.buffer.len();
        for &b in xs.iter() {
            self.buffer.push(b as u8);
            for _ in 0..width as u8 {
                self.buffer.push(0); // Well this seems wasteful.
            }
        }
        self.values.push(Value::Reference {
            fxb_type: FlexBufferType::VectorBool,
            address,
            child_width: width,
        });
    }

    push_slice!(push_uints, u64, UInt, new_uint_vector);
    push_slice!(push_ints, i64, Int, new_int_vector);
    push_slice!(push_floats, f64, Float, new_float_vector);
    push_indirect!(push_indirect_int, i64, Int, IndirectInt);
    push_indirect!(push_indirect_uint, u64, UInt, IndirectUInt);
    push_indirect!(push_indirect_float, f64, Float, IndirectFloat);

    /// Resets the builder and starts a new flexbuffer with a vector at the root.
    /// The exact Flexbuffer vector type is dynamically inferred.
    pub fn build_vector(&'a mut self) -> VectorBuilder<'a> {
        self.reset();
        VectorBuilder {
            builder: self,
            start: None,
        }
    }
    /// Resets the builder and builds a new flexbuffer with a map at the root.
    pub fn build_map(&'a mut self) -> MapBuilder<'a> {
        self.reset();
        MapBuilder {
            builder: self,
            start: None,
        }
    }
    /// Resets the builder and builds a new flexbuffer with the pushed value at the root.
    pub fn build_singleton<P: Pushable>(&mut self, p: P) {
        self.reset();
        p.push_to_builder(self);
        let root = self.values.pop().unwrap();
        store_root(&mut self.buffer, root);
    }
    fn push<P: Pushable>(&mut self, p: P) {
        p.push_to_builder(self);
    }
    /// Stores the values past `previous_end` as a map or vector depending on `is_map`.
    /// If `previous_end` is None then this was a root map / vector and the last value
    /// is stored as the root.
    fn end_map_or_vector(&mut self, is_map: bool, previous_end: Option<usize>) {
        let split = previous_end.unwrap_or(0);
        let value = if is_map {
            let key_vals = &mut self.values[split..];
            sort_map_by_keys(key_vals, &self.buffer);
            let key_vector = store_vector(&mut self.buffer, key_vals, StoreOption::MapKeys);
            store_vector(&mut self.buffer, key_vals, StoreOption::Map(key_vector))
        } else {
            store_vector(&mut self.buffer, &self.values[split..], StoreOption::Vector)
        };
        self.values.truncate(split);
        if previous_end.is_some() {
            self.values.push(value);
        } else {
            store_root(&mut self.buffer, value);
        }
    }
}

/// Builds a Flexbuffer with the single pushed value as the root.
pub fn singleton<P: Pushable>(p: P) -> Vec<u8> {
    let mut b = Builder::new();
    b.build_singleton(p);
    let Builder { buffer, .. } = b;
    buffer
}

/// Stores the root value, root type and root width.
/// This should be called to finish the Flexbuffer.
fn store_root(buffer: &mut Vec<u8>, root: Value) {
    let root_width = root.width_in_vector(buffer.len(), 0);
    align(buffer, root_width);
    store_value(buffer, root, root_width);
    buffer.push(root.packed_type(root_width));
    buffer.push(root_width as u8);
}

pub enum StoreOption {
    Vector,
    Map(Value),
    MapKeys,
}
/// Writes a Flexbuffer Vector or Map.
/// StoreOption::Map(Keys) must be a Value::Key or this will panic.
pub fn store_vector(buffer: &mut Vec<u8>, values: &[Value], opt: StoreOption) -> Value {
    let (skip, stride) = match opt {
        StoreOption::Vector => (0, 1),
        StoreOption::MapKeys => (0, 2),
        StoreOption::Map(_) => (1, 2),
    };
    let iter_values = || values.iter().skip(skip).step_by(stride);

    // Figure out vector type and how long is the prefix.
    let mut result = if let StoreOption::Map(_) = opt {
        Value::new_map()
    } else {
        find_vector_type(iter_values())
    };
    let length_slot = if !result.is_fixed_length_vector() {
        let length = iter_values().count();
        Some(Value::UInt(length as u64))
    } else {
        None
    };
    // Measure required width and align to it.
    let mut width = BitWidth::W8;
    if let StoreOption::Map(keys) = opt {
        width = max(width, keys.width_in_vector(buffer.len(), 0))
    }
    if let Some(l) = length_slot {
        width = max(width, l.width_or_child_width());
    }
    let prefix_length = result.prefix_length();
    for (i, &val) in iter_values().enumerate() {
        width = max(width, val.width_in_vector(buffer.len(), i + prefix_length));
    }
    align(buffer, width);
    debug_assert_ne!(
        result.fxb_type(),
        FlexBufferType::VectorString,
        "VectorString is deprecated and cannot be written.\
         (https://github.com/google/flatbuffers/issues/5627)"
    );
    // Write Prefix.
    if let StoreOption::Map(keys) = opt {
        let key_width = Value::UInt(keys.width_or_child_width() as u64);
        store_value(buffer, keys, width);
        store_value(buffer, key_width, width);
    }
    if let Some(len) = length_slot {
        store_value(buffer, len, width);
    }
    // Write data.
    let address = buffer.len();
    for &v in iter_values() {
        store_value(buffer, v, width);
    }
    // Write types
    if result.is_typed_vector_or_map() {
        for v in iter_values() {
            buffer.push(v.packed_type(width));
        }
    }
    // Return Value representing this Vector.
    result.set_address_or_panic(address);
    result.set_child_width_or_panic(width);
    result
}
