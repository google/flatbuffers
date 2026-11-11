/*
 * Copyright 2026 Google Inc. All rights reserved.
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

#include <stddef.h>
#include <stdint.h>

#include <cstring>
#include <string>
#include <vector>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/minireflect.h"
#include "union_vector/union_vector_generated.h"

namespace {

uint32_t ReadU32(const uint8_t* data, size_t size, size_t offset,
                 uint32_t fallback) {
  if (offset + sizeof(uint32_t) > size) return fallback;
  uint32_t value = 0;
  std::memcpy(&value, data + offset, sizeof(value));
  return value;
}

void WriteU32(std::vector<uint8_t>* buffer, size_t offset, uint32_t value) {
  if (offset + sizeof(uint32_t) > buffer->size()) return;
  std::memcpy(buffer->data() + offset, &value, sizeof(value));
}

struct UnknownValueVisitor : public flatbuffers::IterationVisitor {
  void Unknown(const uint8_t* val) override {
    // MiniReflect should not hand user callbacks a pointer that is unsafe to
    // inspect after generated verification has accepted the buffer.
    if (val) {
      volatile uint8_t byte = *val;
      (void)byte;
    }
  }
};

std::vector<uint8_t> BuildSingleNoneValue(bool size_prefixed) {
  flatbuffers::FlatBufferBuilder builder;
  builder.ForceDefaults(true);
  auto value = builder.CreateString("minireflect_single_none").Union();
  auto movie = CreateMovie(builder, Character_NONE, value, 0, 0);
  if (size_prefixed) {
    FinishSizePrefixedMovieBuffer(builder, movie);
  } else {
    FinishMovieBuffer(builder, movie);
  }
  return std::vector<uint8_t>(builder.GetBufferPointer(),
                              builder.GetBufferPointer() + builder.GetSize());
}

std::vector<uint8_t> BuildVectorNoneValue(bool size_prefixed) {
  flatbuffers::FlatBufferBuilder builder;
  builder.ForceDefaults(true);
  auto value = builder.CreateString("minireflect_vector_none").Union();
  std::vector<uint8_t> types = {static_cast<uint8_t>(Character_NONE)};
  std::vector<flatbuffers::Offset<void>> values = {value};
  auto type_vector = builder.CreateVector(types);
  auto value_vector = builder.CreateVector(values);
  auto movie =
      CreateMovie(builder, Character_NONE, 0, type_vector, value_vector);
  if (size_prefixed) {
    FinishSizePrefixedMovieBuffer(builder, movie);
  } else {
    FinishMovieBuffer(builder, movie);
  }
  return std::vector<uint8_t>(builder.GetBufferPointer(),
                              builder.GetBufferPointer() + builder.GetSize());
}

std::vector<uint8_t> BuildUnknownUnionWithOffset(bool size_prefixed,
                                                 uint32_t offset_value) {
  flatbuffers::FlatBufferBuilder builder;
  builder.ForceDefaults(true);
  auto value = CreateAttacker(builder, 1337).Union();
  std::vector<uint8_t> types = {250};  // Unknown/future union type.
  std::vector<flatbuffers::Offset<void>> values = {value};
  auto type_vector = builder.CreateVector(types);
  auto value_vector = builder.CreateVector(values);
  auto movie =
      CreateMovie(builder, Character_NONE, 0, type_vector, value_vector);
  if (size_prefixed) {
    FinishSizePrefixedMovieBuffer(builder, movie);
  } else {
    FinishMovieBuffer(builder, movie);
  }

  std::vector<uint8_t> buffer(builder.GetBufferPointer(),
                              builder.GetBufferPointer() + builder.GetSize());
  const Movie* root = size_prefixed ? GetSizePrefixedMovie(buffer.data())
                                    : GetMovie(buffer.data());
  auto characters = root->characters();
  if (characters) {
    const auto offset_position = static_cast<size_t>(
        reinterpret_cast<const uint8_t*>(characters->Data()) - buffer.data());
    WriteU32(&buffer, offset_position, offset_value);
  }
  return buffer;
}

void ExerciseMiniReflect(const uint8_t* data, size_t size, bool size_prefixed) {
  if (!data || size < sizeof(flatbuffers::uoffset_t)) return;

  flatbuffers::Verifier verifier(data, size);
  const bool ok = size_prefixed ? VerifySizePrefixedMovieBuffer(verifier)
                                : VerifyMovieBuffer(verifier);
  if (!ok) return;

  const uint8_t* root =
      size_prefixed ? data + sizeof(flatbuffers::uoffset_t) : data;
  (void)flatbuffers::FlatBufferToString(root, Movie::MiniReflectTypeTable(),
                                        true, true, "  ", true);

  UnknownValueVisitor visitor;
  flatbuffers::IterateFlatBuffer(root, Movie::MiniReflectTypeTable(), &visitor);
}

void ExerciseBuffer(const std::vector<uint8_t>& buffer, bool size_prefixed) {
  if (buffer.empty()) return;
  ExerciseMiniReflect(buffer.data(), buffer.size(), size_prefixed);
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (!data || size == 0) return 0;

  const uint8_t mode = data[0] & 0x07;
  const bool size_prefixed = (data[0] & 0x80) != 0;

  switch (mode) {
    case 0:
      ExerciseMiniReflect(data, size, false);
      break;
    case 1:
      ExerciseMiniReflect(data, size, true);
      break;
    case 2:
      ExerciseBuffer(BuildSingleNoneValue(size_prefixed), size_prefixed);
      break;
    case 3:
      ExerciseBuffer(BuildVectorNoneValue(size_prefixed), size_prefixed);
      break;
    case 4: {
      const uint32_t offset = ReadU32(data, size, 1, 0x7fffffffU);
      ExerciseBuffer(BuildUnknownUnionWithOffset(size_prefixed, offset),
                     size_prefixed);
      break;
    }
    default:
      // Keep a stable regression path for the historically problematic
      // out-of-bounds union value offset case.
      ExerciseBuffer(BuildUnknownUnionWithOffset(size_prefixed, 0x7fffffffU),
                     size_prefixed);
      break;
  }

  return 0;
}
