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
use crate::{Blob, Builder, MapBuilder, Pushable, VectorBuilder};
use serde::de::{Deserialize, DeserializeSeed, Deserializer, MapAccess, SeqAccess, Visitor};
use std::fmt;

impl<'de> Deserialize<'de> for Builder {
    fn deserialize<D>(deserializer: D) -> Result<Builder, D::Error>
    where
        D: Deserializer<'de>,
    {
        let mut builder = Builder::default();
        {
            let visitor = BuilderDeserializer {
                builder: &mut builder,
            };
            deserializer.deserialize_any(visitor)?;
        }
        Ok(builder)
    }
}

struct BuilderDeserializer<'a> {
    builder: &'a mut Builder,
}

struct VectorBuilderDeserializer<'a, 'b: 'a> {
    vectorbuilder: &'a mut VectorBuilder<'b>,
}

struct MapBuilderDeserializer<'de, 'a, 'b: 'a> {
    key: &'de str,
    mapbuilder: &'a mut MapBuilder<'b>,
}

impl<'a> BuilderDeserializer<'a> {
    fn push_value<E, P: Pushable>(self, value: P) -> Result<(), E> {
        self.builder.build_singleton(value);
        Ok(())
    }

    fn start_vector(self) -> VectorBuilder<'a> {
        self.builder.start_vector()
    }

    fn start_map(self) -> MapBuilder<'a> {
        self.builder.start_map()
    }
}

impl<'a, 'b: 'a> VectorBuilderDeserializer<'a, 'b> {
    fn push_value<E, P: Pushable>(self, value: P) -> Result<(), E> {
        self.vectorbuilder.push(value);
        Ok(())
    }

    fn start_vector(self) -> VectorBuilder<'a> {
        self.vectorbuilder.start_vector()
    }

    fn start_map(self) -> MapBuilder<'a> {
        self.vectorbuilder.start_map()
    }
}

impl<'de, 'a, 'b: 'a> MapBuilderDeserializer<'de, 'a, 'b> {
    fn push_value<E, P: Pushable>(self, value: P) -> Result<(), E> {
        self.mapbuilder.push(self.key, value);
        Ok(())
    }

    fn start_vector(self) -> VectorBuilder<'a> {
        self.mapbuilder.start_vector(self.key)
    }

    fn start_map(self) -> MapBuilder<'a> {
        self.mapbuilder.start_map(self.key)
    }
}

macro_rules! visit_builder {
    ( $( $visit:ident($ty:ty) )* ) => {
        $(fn $visit<E>(self, value: $ty) -> Result<(), E> {
            self.push_value(value)
        })*
    }
}

// Unable to implement external trait for generic bound by local trait.
macro_rules! visitor_impl {
    () => {
        type Value = ();

        fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
            formatter.write_str("any valid Flexbuffer value")
        }

        visit_builder!(
            visit_bool(bool)
            visit_str(&str)
            visit_i8(i8) visit_i16(i16) visit_i32(i32) visit_i64(i64)
            visit_u8(u8) visit_u16(u16) visit_u32(u32) visit_u64(u64)
            visit_f32(f32) visit_f64(f64)
        );

        fn visit_bytes<E>(self, value: &[u8]) -> Result<(), E> {
            self.push_value(Blob(value))
        }

        fn visit_none<E>(self) -> Result<(), E> {
            self.push_value(())
        }

        fn visit_some<D>(self, deserializer: D) -> Result<(), D::Error>
        where
            D: Deserializer<'de>,
        {
            Deserialize::deserialize(deserializer)
        }

        fn visit_unit<E>(self) -> Result<(), E> {
            self.push_value(())
        }

        fn visit_seq<V>(self, mut visitor: V) -> Result<(), V::Error>
        where
            V: SeqAccess<'de>,
        {
            let mut vectorbuilder = self.start_vector();
            while let Some(()) = visitor.next_element_seed(VectorBuilderDeserializer {
                vectorbuilder: &mut vectorbuilder,
            })? {}
            vectorbuilder.end_vector();
            Ok(())
        }

        fn visit_map<V>(self, mut visitor: V) -> Result<(), V::Error>
        where
            V: MapAccess<'de>,
        {
            let mut mapbuilder = self.start_map();
            while let Some(key) = visitor.next_key::<&str>()? {
                visitor.next_value_seed(MapBuilderDeserializer {
                    key,
                    mapbuilder: &mut mapbuilder,
                })?
            }
            mapbuilder.end_map();
            Ok(())
        }
    };
}

impl<'de, 'a> DeserializeSeed<'de> for BuilderDeserializer<'a> {
    type Value = ();

    fn deserialize<D>(self, deserializer: D) -> Result<(), D::Error>
    where
        D: Deserializer<'de>,
    {
        impl<'de, 'a> Visitor<'de> for BuilderDeserializer<'a> {
            visitor_impl!();
        }

        deserializer.deserialize_any(self)
    }
}

impl<'de, 'a, 'b> DeserializeSeed<'de> for VectorBuilderDeserializer<'a, 'b> {
    type Value = ();

    fn deserialize<D>(self, deserializer: D) -> Result<(), D::Error>
    where
        D: Deserializer<'de>,
    {
        impl<'de, 'a, 'b> Visitor<'de> for VectorBuilderDeserializer<'a, 'b> {
            visitor_impl!();
        }

        deserializer.deserialize_any(self)
    }
}

impl<'de, 'a, 'b: 'a> DeserializeSeed<'de> for MapBuilderDeserializer<'de, 'a, 'b> {
    type Value = ();

    fn deserialize<D>(self, deserializer: D) -> Result<(), D::Error>
    where
        D: Deserializer<'de>,
    {
        impl<'de, 'a, 'b> Visitor<'de> for MapBuilderDeserializer<'de, 'a, 'b> {
            visitor_impl!();
        }

        deserializer.deserialize_any(self)
    }
}
