/*
 * Copyright 2021 Google Inc. All rights reserved.
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

#if os(Linux)
import CoreFoundation
#else
import Foundation
#endif

/// A boolean to see if the system is littleEndian
let isLitteEndian = CFByteOrderGetCurrent() == Int(CFByteOrderLittleEndian.rawValue)
/// Constant for the file id length
let FileIdLength = 4
/// Type aliases
public typealias Byte = UInt8
public typealias UOffset = UInt32
public typealias SOffset = Int32
public typealias VOffset = UInt16
/// Maximum size for a buffer
public let FlatBufferMaxSize = UInt32.max << ((MemoryLayout<SOffset>.size * 8 - 1) - 1)

/// Protocol that confirms all the numbers
///
/// Scalar is used to confirm all the numbers that can be represented in a FlatBuffer. It's used to write/read from the buffer.
public protocol Scalar: Equatable {
  associatedtype NumericValue
  var convertedEndian: NumericValue { get }
}

extension Scalar where Self: FixedWidthInteger {
  /// Converts the value from BigEndian to LittleEndian
  ///
  /// Converts values to little endian on machines that work with BigEndian, however this is NOT TESTED yet.
  public var convertedEndian: NumericValue {
    self as! Self.NumericValue
  }
}

extension Double: Scalar {
  public typealias NumericValue = UInt64

  public var convertedEndian: UInt64 {
    bitPattern.littleEndian
  }
}

extension Float32: Scalar {
  public typealias NumericValue = UInt32

  public var convertedEndian: UInt32 {
    bitPattern.littleEndian
  }
}

extension Bool: Scalar {
  public var convertedEndian: UInt8 {
    self == true ? 1 : 0
  }

  public typealias NumericValue = UInt8
}

extension Int: Scalar {
  public typealias NumericValue = Int
}

extension Int8: Scalar {
  public typealias NumericValue = Int8
}

extension Int16: Scalar {
  public typealias NumericValue = Int16
}

extension Int32: Scalar {
  public typealias NumericValue = Int32
}

extension Int64: Scalar {
  public typealias NumericValue = Int64
}

extension UInt8: Scalar {
  public typealias NumericValue = UInt8
}

extension UInt16: Scalar {
  public typealias NumericValue = UInt16
}

extension UInt32: Scalar {
  public typealias NumericValue = UInt32
}

extension UInt64: Scalar {
  public typealias NumericValue = UInt64
}

public func FlatBuffersVersion_2_0_0() {}
