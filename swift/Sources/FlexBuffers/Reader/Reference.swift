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

enum FlexBuffersErrors: Error {
  case sizeOfBufferIsTooSmall
  case typeCouldNotBeDetermined
}

@inline(__always)
public func getRoot(buffer: ByteBuffer) throws -> Reference? {
  let end = buffer.count
  if buffer.count < 3 {
    throw FlexBuffersErrors.sizeOfBufferIsTooSmall
  }

  let byteWidth = buffer.read(def: UInt8.self, position: end &- 1)
  let packedType = buffer.read(def: UInt8.self, position: end &- 2)
  let offset = end &- 2 &- numericCast(byteWidth)

  return Reference(
    byteBuffer: buffer,
    offset: offset,
    parentWidth: byteWidth,
    packedType: packedType)
}

@inline(__always)
public func getRootChecked(buffer: ByteBuffer) throws -> Reference? {
  // TODO(mustiikhalil): implement verifier
  return try getRoot(buffer: buffer)
}

public struct Reference {
  private let byteBuffer: ByteBuffer
  private let offset: Int
  private let parentWidth: UInt8
  private let byteWidth: UInt8

  public let type: FlexBufferType

  @inline(__always)
  init?(
    byteBuffer: ByteBuffer,
    offset: Int,
    parentWidth: UInt8,
    packedType: UInt8)
  {
    guard let type = FlexBufferType(rawValue: UInt64(packedType >> 2)) else {
      return nil
    }
    self.byteBuffer = byteBuffer
    self.offset = offset
    self.parentWidth = parentWidth
    byteWidth = 1 << (packedType & 3)
    self.type = type
  }

  @inline(__always)
  init(
    byteBuffer: ByteBuffer,
    offset: Int,
    parentWidth: UInt8,
    byteWidth: UInt8,
    type: FlexBufferType)
  {
    self.byteBuffer = byteBuffer
    self.offset = offset
    self.parentWidth = parentWidth
    self.byteWidth = byteWidth
    self.type = type
  }

  @inline(__always)
  public var bool: Bool? {
    return switch type {
    case .bool: byteBuffer.readUInt64(offset: offset, byteWidth: byteWidth) != 0
    default: nil
    }
  }

  @inline(__always)
  public var uint: UInt64? {
    return switch type {
    case .uint: byteBuffer.readUInt64(offset: offset, byteWidth: byteWidth)
    case .indirectUInt: byteBuffer.readUInt64(
        offset: indirect(),
        byteWidth: byteWidth)
    default: nil
    }
  }

  @inline(__always)
  public var int: Int64? {
    return switch type {
    case .int: byteBuffer.readInt64(offset: offset, byteWidth: byteWidth)
    case .indirectInt: byteBuffer.readInt64(
        offset: indirect(),
        byteWidth: byteWidth)
    default: nil
    }
  }

  @inline(__always)
  public var double: Double? {
    return switch type {
    case .float: byteBuffer.readDouble(offset: offset, byteWidth: byteWidth)
    case .indirectFloat: byteBuffer.readDouble(
        offset: indirect(),
        byteWidth: byteWidth)
    default: nil
    }
  }

  @inline(__always)
  public var map: Map? {
    guard type == .map else { return nil }
    return Map(
      byteBuffer: byteBuffer,
      offset: indirect(),
      byteWidth: byteWidth)
  }

  @inline(__always)
  public var vector: Vector? {
    guard type == .vector || type == .map else { return nil }
    return Vector(
      byteBuffer: byteBuffer,
      offset: indirect(),
      byteWidth: byteWidth)
  }

  @inline(__always)
  public var cString: String? {
    guard type == .string || type == .key else { return nil }
    let offset = indirect()

    let count = getCount(
      buffer: byteBuffer,
      offset: offset,
      byteWidth: byteWidth)

    return byteBuffer.readString(
      at: offset,
      count: count)
  }

  @inline(__always)
  public func blob<Result>(_ completion: (UnsafeRawBufferPointer) -> Result)
    -> Result?
  {
    guard type == .blob || type == .string else { return nil }

    let offset = indirect()
    let count = getCount(
      buffer: byteBuffer,
      offset: offset,
      byteWidth: byteWidth)
    return byteBuffer.withUnsafePointerToSlice(
      index: offset,
      count: count,
      body: completion)
  }

  @inline(__always)
  public var typedVector: TypedVector? {
    guard isTypedVectorType(type: type) else { return nil }
    guard var type = toTypedVectorElementType(type: type) else { return nil }
    if type == .string {
      type = .key
    }
    return TypedVector(
      byteBuffer: byteBuffer,
      offset: indirect(),
      byteWidth: byteWidth,
      type: type)
  }

  @inline(__always)
  public var fixedTypedVector: FixedTypedVector? {
    guard isFixedTypedVectorType(type: type) else { return nil }
    let t = toFixedTypedVectorElementType(type: type)
    guard let type = t.type else { return nil }
    return FixedTypedVector(
      byteBuffer: byteBuffer,
      offset: indirect(),
      byteWidth: byteWidth,
      type: type,
      count: t.count)
  }

  @inline(__always)
  public func string(encoding: String.Encoding = .utf8) -> String? {
    guard type == .string else { return nil }
    let offset = indirect()

    let count = getCount(
      buffer: byteBuffer,
      offset: offset,
      byteWidth: byteWidth)

    return byteBuffer.readString(
      at: offset,
      count: count,
      type: encoding)
  }

  @inline(__always)
  public func asInt<T: FixedWidthInteger>() -> T? {
    guard let v = int else {
      return nil
    }
    return numericCast(v)
  }

  @inline(__always)
  public func asUInt<T: FixedWidthInteger>() -> T? {
    guard let v = uint else {
      return nil
    }
    return numericCast(v)
  }

  @inline(__always)
  public func withUnsafeRawPointer<Result>(
    _ completion: (UnsafeRawPointer) throws
      -> Result)
    rethrows -> Result?
  {
    return try byteBuffer.readWithUnsafeRawPointer(
      position: indirect(),
      completion)
  }

  private func indirect() -> Int {
    readIndirect(buffer: byteBuffer, offset: offset, parentWidth)
  }
}

extension Reference {

  public func jsonString() -> String {
    var str = ""
    jsonBuilder(json: &str)
    return str
  }

  func jsonBuilder(json: inout String) {
    switch type {
    case .null:
      json += StaticJSON.null
    case .uint, .indirectUInt:
      json += uint.valueOrNull
    case .int, .indirectInt:
      json += int.valueOrNull
    case .float, .indirectFloat:
      json += double.valueOrNull
    case .string, .key:
      json += "\"\(cString ?? StaticJSON.null)\""
    case .map:
      map?.jsonBuilder(json: &json)
    case .bool:
      json += bool.valueOrNull
    case .blob:
      if let p = blob({ String(data: Data($0), encoding: .utf8) })?
        .valueOrNull
      {
        json += "\"\(p)\""
      } else {
        json += StaticJSON.null
      }
    default:
      if type == .vector {
        vector?.jsonBuilder(json: &json)
      } else if isTypedVectorType(type: type) {
        typedVector?.jsonBuilder(json: &json)
      } else if isFixedTypedVectorType(type: type) {
        fixedTypedVector?.jsonBuilder(json: &json)
      } else {
        json += StaticJSON.null
      }
    }
  }
}
