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

extern crate serde;
use crate::Reader;
use serde::ser::{Serialize, SerializeMap, SerializeSeq, Serializer};

impl<'de> Serialize for Reader<'de> {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        use crate::BitWidth::*;
        use crate::FlexBufferType::*;
        match self.flexbuffer_type() {
            Null => serializer.serialize_unit(),
            Bool => serializer.serialize_bool(self.as_bool()),
            Int | IndirectInt => match self.bitwidth() {
                W8 => serializer.serialize_i8(self.as_i8()),
                W16 => serializer.serialize_i16(self.as_i16()),
                W32 => serializer.serialize_i32(self.as_i32()),
                W64 => serializer.serialize_i64(self.as_i64()),
            },
            UInt | IndirectUInt => match self.bitwidth() {
                W8 => serializer.serialize_u8(self.as_u8()),
                W16 => serializer.serialize_u16(self.as_u16()),
                W32 => serializer.serialize_u32(self.as_u32()),
                W64 => serializer.serialize_u64(self.as_u64()),
            },
            Float | IndirectFloat => match self.bitwidth() {
                W32 => serializer.serialize_f32(self.as_f32()),
                W64 => serializer.serialize_f64(self.as_f64()),
                bw => unreachable!("TODO serialize {:?}.", bw),
            },
            String | Key => serializer.serialize_str(self.as_str()),
            Blob => serializer.serialize_bytes(self.as_blob().0),
            Map => {
                let mapreader = self.as_map();
                let mut map = serializer.serialize_map(Some(mapreader.len()))?;
                for (k, v) in mapreader.iter_keys().zip(mapreader.iter_values()) {
                    map.serialize_entry(k, &v)?;
                }
                map.end()
            }
            ty if ty.is_vector() => {
                let vecreader = self.as_vector();
                let mut seq = serializer.serialize_seq(Some(vecreader.len()))?;
                for e in vecreader.iter() {
                    seq.serialize_element(&e)?;
                }
                seq.end()
            }
            ty => unreachable!("TODO serialize {:?}.", ty),
        }
    }
}
