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

#if canImport(Common)
import Common
#endif

/// Type aliases
public typealias Byte = UInt8
public typealias UOffset = UInt32
public typealias SOffset = Int32
public typealias VOffset = UInt16
/// Maximum size for a buffer
public let FlatBufferMaxSize =
  UInt32
    .max << ((MemoryLayout<SOffset>.size * 8 - 1) - 1)

/// Protocol that All Scalars should conform to
///
/// Scalar is used to conform all the numbers that can be represented in a FlatBuffer. It's used to write/read from the buffer.

extension Scalar where Self: FixedWidthInteger {}

extension Double: Verifiable, FlatbuffersVectorInitializable {}

extension Float32: Verifiable, FlatbuffersVectorInitializable {}

extension Bool: Verifiable, FlatbuffersVectorInitializable {}

extension Int: Verifiable, FlatbuffersVectorInitializable {}

extension Int8: Verifiable, FlatbuffersVectorInitializable {}

extension Int16: Verifiable, FlatbuffersVectorInitializable {}

extension Int32: Verifiable, FlatbuffersVectorInitializable {}

extension Int64: Verifiable, FlatbuffersVectorInitializable {}

extension UInt8: Verifiable, FlatbuffersVectorInitializable {}

extension UInt16: Verifiable, FlatbuffersVectorInitializable {}

extension UInt32: Verifiable, FlatbuffersVectorInitializable {}

extension UInt64: Verifiable, FlatbuffersVectorInitializable {}

public func FlatBuffersVersion_25_9_23() {}
