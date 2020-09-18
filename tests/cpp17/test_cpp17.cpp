/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// This is a sandbox for modeling C++17 code generator.
// C++17 code generator: "flatc --cpp_std c++17".
// Warning:
// This is an experimental feature and could change at any time.

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/flexbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/minireflect.h"
#include "flatbuffers/registry.h"
#include "flatbuffers/util.h"
#include "test_assert.h"

// Embed generated code into an isolated namespace.
namespace cpp17 {
#include "generated_cpp17/monster_test_generated.h"
#include "generated_cpp17/optional_scalars_generated.h"
}  // namespace cpp17

namespace cpp11 {
#include "../monster_test_generated.h"
#include "../optional_scalars_generated.h"
}  // namespace cpp11

void CreateTableByTypeTest() {
  flatbuffers::FlatBufferBuilder builder;

  // We will create an object of this type using only the type.
  using type_to_create_t = cpp17::MyGame::Example::Stat;

  [&builder] {
    auto id_str = builder.CreateString("my_id");
    auto table = type_to_create_t::Traits::Create(builder, id_str, 42, 7);
    // Be sure that the correct return type was inferred.
    static_assert(
        std::is_same_v<decltype(table), flatbuffers::Offset<type_to_create_t>>);
    builder.Finish(table);
  }();

  // Access it.
  auto stat =
      flatbuffers::GetRoot<type_to_create_t>(builder.GetBufferPointer());
  TEST_EQ_STR(stat->id()->c_str(), "my_id");
  TEST_EQ(stat->val(), 42);
  TEST_EQ(stat->count(), 7);
}

void OptionalScalarsTest() {
  static_assert(std::is_same<flatbuffers::Optional<float>, std::optional<float>>::value);
  static_assert(std::is_same<flatbuffers::nullopt_t, std::nullopt_t>::value);

  // test C++ nullable
  flatbuffers::FlatBufferBuilder fbb;
  FinishScalarStuffBuffer(fbb, cpp17::optional_scalars::CreateScalarStuff(
                                   fbb, 1, static_cast<int8_t>(2)));
  auto opts =
      cpp17::optional_scalars::GetMutableScalarStuff(fbb.GetBufferPointer());
  TEST_ASSERT(!opts->maybe_bool());
  TEST_ASSERT(!opts->maybe_f32().has_value());
  TEST_ASSERT(opts->maybe_i8().has_value());
  TEST_EQ(opts->maybe_i8().value(), 2);
  TEST_ASSERT(opts->mutate_maybe_i8(3));
  TEST_ASSERT(opts->maybe_i8().has_value());
  TEST_EQ(opts->maybe_i8().value(), 3);
  TEST_ASSERT(!opts->mutate_maybe_i16(-10));

  cpp17::optional_scalars::ScalarStuffT obj;
  opts->UnPackTo(&obj);
  TEST_ASSERT(!obj.maybe_bool);
  TEST_ASSERT(!obj.maybe_f32.has_value());
  TEST_ASSERT(obj.maybe_i8.has_value() && obj.maybe_i8.value() == 3);
  TEST_ASSERT(obj.maybe_i8 && *obj.maybe_i8 == 3);
  obj.maybe_i32 = -1;

  fbb.Clear();
  FinishScalarStuffBuffer(
      fbb, cpp17::optional_scalars::ScalarStuff::Pack(fbb, &obj));
  opts = cpp17::optional_scalars::GetMutableScalarStuff(fbb.GetBufferPointer());
  TEST_ASSERT(opts->maybe_i8().has_value());
  TEST_EQ(opts->maybe_i8().value(), 3);
  TEST_ASSERT(opts->maybe_i32().has_value());
  TEST_EQ(opts->maybe_i32().value(), -1);

  TEST_EQ(std::optional<int32_t>(opts->maybe_i32()).value(), -1);
  TEST_EQ(std::optional<int64_t>(opts->maybe_i32()).value(), -1);
  TEST_ASSERT(opts->maybe_i32() == std::optional<int64_t>(-1));
}

int FlatBufferCpp17Tests() {
  CreateTableByTypeTest();
  OptionalScalarsTest();
  return 0;
}

int main(int /*argc*/, const char * /*argv*/[]) {
  InitTestEngine();

  FlatBufferCpp17Tests();

  if (!testing_fails) {
    TEST_OUTPUT_LINE("C++17: ALL TESTS PASSED");
  } else {
    TEST_OUTPUT_LINE("C++17: %d FAILED TESTS", testing_fails);
  }
  return CloseTestEngine();
}
