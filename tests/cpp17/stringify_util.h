/*
 * Copyright 2020 Google Inc. All rights reserved.
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

// This contains some utilities/examples for how to leverage the static reflec-
// tion features of tables and structs in the C++17 code generation to recur-
// sively produce a string representation of any Flatbuffer table or struct use
// compile-time iteration over the fields. Note that this code is completely
// generic in that it makes no reference to any particular Flatbuffer type.

#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "flatbuffers/flatbuffers.h"

namespace cpp17 {

// User calls this; need to forward declare it since it is called recursively.
template<typename T>
std::optional<std::string>
StringifyFlatbufferValue(T&& val, const std::string &indent = "");

namespace detail {

/*******************************************************************************
** Metaprogramming helpers for detecting Flatbuffers Tables, Structs, & Vectors.
*******************************************************************************/
template<typename FB, typename = void>
struct is_fb_table_or_struct : std::false_type {};

// We know it's a table or struct when it has a Traits subclass.
template<typename FB>
struct is_fb_table_or_struct<FB, std::void_t<typename FB::Traits>>
  : std::true_type {};

template<typename FB>
constexpr bool is_fb_table_or_struct_v = is_fb_table_or_struct<FB>::value;

template<typename T>
struct is_fb_vector : std::false_type {};

// We know it's a table or struct when it has a Traits subclass.
template<typename T>
struct is_fb_vector<flatbuffers::Vector<T>> : std::true_type {};

template<typename T>
constexpr bool is_fb_vector_v = is_fb_vector<T>::value;

/*******************************************************************************
** Compile-time Iteration & Recursive Stringification over Flatbuffers types.
*******************************************************************************/
template<typename FB, size_t... Indexes>
std::string StringifyTableOrStructImpl(const FB &flatbuff,
                                       const std::string &indent,
                                       std::index_sequence<Indexes...>) {
  // Getting the fields_pack should be a relatively light-weight operation. It
  // will copy a std::tuple of primitive types, pointers, and references; no
  // deep copies of anything will be made.
  auto fields_pack = flatbuff.fields_pack();
  std::ostringstream oss;
  auto add_field = [&](auto&& field_value, size_t index) {
    auto value_string = StringifyFlatbufferValue(field_value, indent);
    if (!value_string) {
      return;
    }
    oss << indent
        << FB::Traits::field_names[index]
        << " = "
        << *value_string
        << "\n";
  };
  // Prevents unused-var warning when object has no fields.
  (void)fields_pack; (void)add_field;
  // This line is where the compile-time iteration happens!
  (add_field(std::get<Indexes>(fields_pack), Indexes), ...);
  return oss.str();
}

template<typename FB>
std::string StringifyTableOrStruct(const FB &flatbuff,
                                   const std::string &indent) {
  constexpr size_t field_count =
      std::tuple_size_v<typename FB::FieldTypes>;
  std::ostringstream oss;
  oss << FB::Traits::name << "{\n"
      << StringifyTableOrStructImpl(
             flatbuff, indent+"  ", std::make_index_sequence<field_count>{})
      << indent
      << "}";
  return oss.str();
}

template<typename T>
std::string StringifyVector(const flatbuffers::Vector<T> &vec,
                            const std::string &indent) {
  std::ostringstream oss;
  oss << "[\n";
  for (size_t i = 0; i < vec.size(); ++i) {
    if (auto value_string = StringifyFlatbufferValue(vec.Get(i))) {
      oss << indent+"  " << *value_string;
      if (i != vec.size()-1) {
        oss << ",";
      }
      oss << "\n";
    }
  }
  oss << indent << "]";
  return oss.str();
}

template<typename T>
std::string StringifyArithmeticType(T val) {
  std::ostringstream oss;
  oss << val;
  return oss.str();
}

}  // namespace detail

/*******************************************************************************
** Take any flatbuffer type (table, struct, Vector, int...) and stringify it.
*******************************************************************************/
template<typename T>
std::optional<std::string> StringifyFlatbufferValue(T&& val,
                                                   const std::string &indent) {
  constexpr bool is_pointer = std::is_pointer_v<std::remove_reference_t<T>>;
  if constexpr (is_pointer) {
    if (val == nullptr) {
      return std::nullopt; // Field is absent.
    }
  }
  using decayed =
    std::decay_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

  // Is it a Flatbuffers Table or Struct?
  if constexpr (detail::is_fb_table_or_struct_v<decayed>) {
    // We have a nested table or struct; use recursion!
    if constexpr(is_pointer) {
      return detail::StringifyTableOrStruct(*val, indent);
    } else {
      return detail::StringifyTableOrStruct(val, indent);
    }
  }

  // Is it an 8-bit number?  If so, print it like an int (not char).
  if constexpr (std::is_same_v<decayed, int8_t> ||
                std::is_same_v<decayed, uint8_t>)  {
    return detail::StringifyArithmeticType(static_cast<int>(val));
  }

  // Is it an enum? If so, print it like an int, since Flatbuffers doesn't yet
  // have type-based reflection for enums, so we can't print the enum's name :(
  if constexpr (std::is_enum_v<decayed>) {
    return StringifyFlatbufferValue(
             static_cast<std::underlying_type_t<decayed>>(val),
             indent);
  }

  // Is it an int, double, float, uint32_t, etc.?
  if constexpr (std::is_arithmetic_v<decayed>) {
    return detail::StringifyArithmeticType(val);
  }

  // Is it a Flatbuffers string?
  if constexpr (std::is_same_v<decayed, flatbuffers::String>) {
    return "\"" + val->str() + "\"";
  }

  // Is it a Flatbuffers Vector?
  if constexpr (detail::is_fb_vector_v<decayed>) {
    return detail::StringifyVector(*val, indent);
  }

  // Not sure how to format this type, whatever it is.
  return std::nullopt;
}

}  // namespace cpp17