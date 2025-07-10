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

public struct Value: Equatable {

  @usableFromInline
  enum Union: Equatable {
    case i(Int64)
    case u(UInt64)
    case f(Double)
  }

  var sloc: Union
  let type: FlexBufferType
  let bitWidth: BitWidth

  @inline(__always)
  private init() {
    sloc = .i(0)
    type = .null
    bitWidth = .w8
  }

  @inline(__always)
  init(bool: Bool) {
    sloc = .u(bool ? 1 : 0)
    type = .bool
    bitWidth = .w8
  }

  @inline(__always)
  init(v: UInt64, type: FlexBufferType, bitWidth: BitWidth) {
    sloc = .u(v)
    self.type = type
    self.bitWidth = bitWidth
  }

  @inline(__always)
  init(v: Int64, type: FlexBufferType, bitWidth: BitWidth) {
    sloc = .i(v)
    self.type = type
    self.bitWidth = bitWidth
  }

  @inline(__always)
  init(v: Double, type: FlexBufferType, bitWidth: BitWidth) {
    sloc = .f(v)
    self.type = type
    self.bitWidth = bitWidth
  }

  @inline(__always)
  init(sloc: Union, type: FlexBufferType, bitWidth: BitWidth) {
    self.sloc = sloc
    self.type = type
    self.bitWidth = bitWidth
  }

  @usableFromInline
  var i: Int64 {
    switch sloc {
    case .i(let v): v
    default: 0
    }
  }

  @usableFromInline
  var u: UInt64 {
    switch sloc {
    case .u(let v): v
    default: 0
    }
  }

  @usableFromInline
  var f: Double {
    switch sloc {
    case .f(let v): v
    default: 0
    }
  }

  static let `nil` = Value()
}

extension Value {
  @usableFromInline
  @inline(__always)
  func elementWidth(size: Int, index: UInt64) -> BitWidth {
    if isInline(type) {
      return bitWidth
    } else {
      for byteWidth in stride(from: 1, to: MemoryLayout<UInt64>.size, by: 2) {
        let _offsetLoc: UInt64 = numericCast(numericCast(size) &+ padding(
          bufSize: numericCast(size),
          elementSize: numericCast(byteWidth)))
        let offsetLoc = _offsetLoc &+ (index &* numericCast(byteWidth))
        let offset = offsetLoc &- u

        let bitWidth = widthU(offset)
        if (UInt32.one << bitWidth.rawValue) == byteWidth {
          return bitWidth
        }
      }
      return .w64
    }
  }

  @inline(__always)
  func storedPackedType(width: BitWidth = .w8) -> UInt8 {
    packedType(bitWidth: storedWidth(width: width), type: type)
  }

  @inline(__always)
  private func storedWidth(width: BitWidth) -> BitWidth {
    if isInline(type) {
      return max(bitWidth, width)
    } else {
      return bitWidth
    }
  }
}
