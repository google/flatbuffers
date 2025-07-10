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
#if canImport(Common)
import Common
#endif

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
  let u = UInt64(bitPattern: v) << 1
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
func isTypedVectorElementType(type: FlexBufferType) -> Bool {
  return type >= .int && type <= .string || type == .bool
}

@inline(__always)
func isTypedVectorType(type: FlexBufferType) -> Bool {
  return type >= .vectorInt && type <= .vectorString || type == .vectorBool
}

@inline(__always)
func toTypedVectorElementType(type: FlexBufferType) -> FlexBufferType? {
  return FlexBufferType(
    rawValue: type.rawValue &- FlexBufferType.vectorInt
      .rawValue &+ FlexBufferType.int.rawValue)
}

@inline(__always)
func isFixedTypedVectorType(type: FlexBufferType) -> Bool {
  return type >= .vectorInt2 && type <= .vectorFloat4
}

@inline(__always)
func toFixedTypedVectorElementType(type: FlexBufferType)
  -> (type: FlexBufferType?, count: Int)
{
  assert(isFixedTypedVectorType(type: type))
  let fixedType: UInt64 = numericCast(
    type.rawValue &- FlexBufferType.vectorInt2
      .rawValue)
  let len: Int = numericCast((fixedType / 3) + 2)
  return (
    FlexBufferType(rawValue: (fixedType % 3) + FlexBufferType.int.rawValue),
    len)
}

// MARK: - Reader functions

@inline(__always)
func binarySearch(
  vector: TypedVector,
  target: String) -> Int?
{
  var left = 0
  var right = vector.count

  while left <= right {
    let mid = left &+ (right &- left) / 2
    let comp = vector.compare(offset: mid, target: target)
    if comp == 0 {
      return mid
    } else if comp < 0 {
      left = mid &+ 1
    } else {
      right = mid &- 1
    }
  }
  return nil
}

@inline(__always)
func readIndirect(buffer: ByteBuffer, offset: Int, _ byteWidth: UInt8) -> Int {
  return offset &- numericCast(buffer.readUInt64(
    offset: offset,
    byteWidth: byteWidth))
}

@inline(__always)
func getCount(buffer: ByteBuffer, offset: Int, byteWidth: UInt8) -> Int {
  Int(buffer.readUInt64(
    offset: offset &- numericCast(byteWidth),
    byteWidth: byteWidth))
}
