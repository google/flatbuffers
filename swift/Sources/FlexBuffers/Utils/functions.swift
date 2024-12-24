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

@inline(__always)
internal func isInline(_ t: FlexBufferType) -> Bool {
  return t <= .float || t == .bool
}

@inline(__always)
private func check(_ v: UInt64, width: UInt64) -> Bool {
  (v & ~((.one << width) &- 1)) == 0
}

@inline(__always)
internal func widthI(_ v: Int64) -> BitWidth {
  let u = unsafeBitCast(v, to: UInt64.self) << 1
  return widthU(v >= 0 ? u : ~u)
}

@inline(__always)
internal func widthF(_ v: Double) -> BitWidth {
  Double(Float(v)) == v ? .w32 : .w64
}

@inline(__always)
internal func widthU(_ v: UInt64) -> BitWidth {
  if check(v, width: 8) { return .w8 }
  if check(v, width: 16) { return .w16 }
  if check(v, width: 32) { return .w32 }
  return .w64
}

@inline(__always)
internal func packedType(bitWidth: BitWidth, type: FlexBufferType) -> UInt8 {
  numericCast(bitWidth.rawValue | (type.rawValue << 2))
}

@inline(__always)
func getScalarType<T>(type: T.Type) -> FlexBufferType where T: Scalar {
  if T.self is (any BinaryFloatingPoint.Type) {
    return .float
  }

  if T.self is Bool.Type {
    return .bool
  }

  if T.self is (any UnsignedInteger.Type) {
    return .uint
  }

  return .int
}


@inline(__always)
func toTypedVector(type: FlexBufferType, length: UInt64) -> FlexBufferType {
  let type: UInt64 = switch length {
  case 0: type.rawValue &- FlexBufferType.int.rawValue &+ FlexBufferType
    .vectorInt.rawValue
  case 2: type.rawValue &- FlexBufferType.int.rawValue &+ FlexBufferType
    .vectorInt2.rawValue
  case 3: type.rawValue &- FlexBufferType.int.rawValue &+ FlexBufferType
    .vectorInt3.rawValue
  case 4: type.rawValue &- FlexBufferType.int.rawValue &+ FlexBufferType
    .vectorInt4.rawValue
  default: 0
  }
  return FlexBufferType(rawValue: type) ?? .null
}

@inline(__always)
func isTypedVectorType(type: FlexBufferType) -> Bool {
  return type >= .int && type <= .string || type == .bool
}
