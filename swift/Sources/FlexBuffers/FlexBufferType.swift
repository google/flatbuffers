/*
 * Copyright 2024 Google Inc. All rights reserved.
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

import Foundation

public enum FlexBufferType: UInt64 {
  case null = 0
  /// Variable width signed integer: `Int8, Int16, Int32, Int64`
  case int = 1
  /// Variable width signed integer: `UInt8, UInt16, UInt32, UInt64`
  case uint = 2
  /// Variable width floating point: `Float32, Double64`
  case float = 3
  case key = 4
  case string = 5
  /// An Int, stored by offset rather than inline. Indirect types can keep the bitwidth of a
  /// vector or map small when the inline value would have increased the bitwidth.
  case indirectInt = 6
  /// A UInt, stored by offset rather than inline. Indirect types can keep the bitwidth of a
  /// vector or map small when the inline value would have increased the bitwidth.
  case indirectUInt = 7
  /// A Float, stored by offset rather than inline. Indirect types can keep the bitwidth of a
  /// vector or map small when the inline value would have increased the bitwidth.
  case indirectFloat = 8
  case map = 9
  /// Untyped
  case vector = 10
  /// Typed any sizes (stores no type table).
  case vectorInt = 11
  case vectorUInt = 12
  case vectorFloat = 13
  case vectorKey = 14
  @available(
    *,
    deprecated,
    message: "use FBT_VECTOR or FBT_VECTOR_KEY instead.")
  case vectorString = 15

  /// Typed tuples (no type table, no size field).
  case vectorInt2 = 16
  case vectorUInt2 = 17
  case vectorFloat2 = 18
  /// Typed triples (no type table, no size field).
  case vectorInt3 = 19
  case vectorUInt3 = 20
  case vectorFloat3 = 21
  /// Typed quad (no type table, no size field).
  case vectorInt4 = 22
  case vectorUInt4 = 23
  case vectorFloat4 = 24
  case blob = 25
  case bool = 26
  /// To Allow the same type of conversion of type to vector type
  case vectorBool = 36
  case max = 37
}

extension FlexBufferType: Comparable {
  public static func < (lhs: FlexBufferType, rhs: FlexBufferType) -> Bool {
    lhs.rawValue < rhs.rawValue
  }
}
