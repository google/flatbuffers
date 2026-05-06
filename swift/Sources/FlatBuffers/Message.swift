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

enum FlatbuffersGRPCError: Error {
  case finishedNotCalledOnBuilder
}

public protocol GRPCVerifiableMessage<Message> {
  associatedtype Message

  init(pointer: UnsafeRawBufferPointer)
  init(byteBuffer: ByteBuffer)

  func decode() throws -> Message
  func withUnsafeReadableBytes<Data>(
    _ body: (UnsafeRawBufferPointer) throws
      -> Data) rethrows -> Data
}

public struct GRPCMessage<
  Table: FlatBufferVerifiableTable
>: Sendable, GRPCVerifiableMessage {
  public typealias Message = Table

  private let buffer: ByteBuffer

  public var size: Int { Int(buffer.size) }

  public init(pointer: UnsafeRawBufferPointer) {
    buffer = ByteBuffer(
      copyingMemoryBound: pointer.baseAddress!,
      capacity: pointer.count)
  }

  public init(builder: inout FlatBufferBuilder) throws {
    guard builder.finished else {
      throw FlatbuffersGRPCError.finishedNotCalledOnBuilder
    }

    buffer = builder.sizedBuffer
  }

  public init(byteBuffer: ByteBuffer) {
    buffer = byteBuffer
  }

  public func decode() throws -> Table {
    var buf = buffer
    return try getCheckedRoot(byteBuffer: &buf)
  }

  @discardableResult
  @inline(__always)
  public func withUnsafeReadableBytes<Data>(
    _ body: (UnsafeRawBufferPointer) throws
      -> Data) rethrows -> Data
  {
    return try buffer.readWithUnsafeRawPointer(position: buffer.reader) {
      try body(UnsafeRawBufferPointer(start: $0, count: size))
    }
  }
}

@available(macOS 15.0, iOS 18.0, watchOS 11.0, tvOS 18.0, visionOS 2.0, *)
public struct FlatBuffersMessageSerializer<
  Message: GRPCVerifiableMessage
>: Sendable {
  public init() {}

  public func serialize<Data>(
    message: Message,
    _ completion: (UnsafeRawBufferPointer) throws -> Data) throws -> Data
  {
    return try message.withUnsafeReadableBytes {
      try completion($0)
    }
  }
}

@available(macOS 15.0, iOS 18.0, watchOS 11.0, tvOS 18.0, visionOS 2.0, *)
public struct FlatBuffersMessageDeserializer<
  Message: GRPCVerifiableMessage
>: Sendable {
  public init() {}

  public func deserialize(pointer: UnsafeRawBufferPointer) throws -> Message {
    Message.init(pointer: pointer)
  }
}
