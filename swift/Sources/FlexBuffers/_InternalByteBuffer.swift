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

/// `ByteBuffer` is the interface that stores the data for a `Flatbuffers` object
/// it allows users to write and read data directly from memory thus the use of its
/// functions should be used
@usableFromInline
struct _InternalByteBuffer {

  /// Storage is a container that would hold the memory pointer to solve the issue of
  /// deallocating the memory that was held by (memory: UnsafeMutableRawPointer)
  @usableFromInline
  final class Storage {
    /// pointer to the start of the buffer object in memory
    var memory: UnsafeMutableRawPointer

    @usableFromInline
    init(count: Int, alignment: Int) {
      memory = UnsafeMutableRawPointer.allocate(
        byteCount: count,
        alignment: alignment)
    }

    deinit {
      memory.deallocate()
    }

    @usableFromInline
    func initialize(for size: Int) {
      memset(memory, 0, size)
    }

    /// Reallocates the buffer incase the object to be written doesnt fit in the current buffer
    /// - Parameter size: Size of the current object
    @usableFromInline
    func reallocate(
      capacity: Int,
      writerSize: Int,
      alignment: Int
    ) {
      let newData = UnsafeMutableRawPointer.allocate(
        byteCount: capacity,
        alignment: alignment)
      memset(newData, 0, capacity &- writerSize)
      memcpy(
        newData,
        memory,
        writerSize)
      memory.deallocate()
      memory = newData
    }
  }

  @usableFromInline var _storage: Storage
  // Initial size of the internal storage
  private let initialSize: Int
  /// The size of the elements written to the buffer + their paddings
  var writerIndex: Int = 0
  /// Alignment of the current  memory being written to the buffer
  private var alignment = 1
  /// Public Pointer to the buffer object in memory. This should NOT be modified for any reason
  public var memory: UnsafeMutableRawPointer { _storage.memory }
  /// Current capacity for the buffer
  public private(set) var capacity: Int

  /// Returns the written bytes into the ``ByteBuffer``
  public var underlyingBytes: [UInt8] {
    let start = memory.bindMemory(to: UInt8.self, capacity: writerIndex)

    let ptr = UnsafeBufferPointer<UInt8>(start: start, count: writerIndex)
    return Array(ptr)
  }

  /// Constructor that creates a Flatbuffer instance with a size
  /// - Parameter:
  ///   - size: Length of the buffer
  ///   - allowReadingUnalignedBuffers: allow reading from unaligned buffer
  init(initialSize size: Int) {
    initialSize = size.convertToPowerofTwo
    capacity = initialSize
    _storage = Storage(count: initialSize, alignment: alignment)
    _storage.initialize(for: initialSize)
  }

  /// Clears the current instance of the buffer, replacing it with new memory
  @inline(__always)
  mutating public func clear(keepingCapacity: Bool = false) {
    writerIndex = 0
    alignment = 1
    if keepingCapacity {
      _storage.initialize(for: capacity)
    } else {
      capacity = initialSize
      _storage = Storage(count: initialSize, alignment: alignment)
    }
  }

  @inline(__always)
  mutating public func resetWriter(to writer: Int) {
    writerIndex = writer
  }

  /// Makes sure that buffer has enouch space for each of the objects that will be written into it
  /// - Parameter size: size of object
  @usableFromInline
  mutating func ensureSpace(size: Int) {
    guard size &+ writerIndex > capacity else { return }

    while capacity <= writerIndex &+ size {
      capacity = capacity << 1
    }

    /// solution take from Apple-NIO
    capacity = capacity.convertToPowerofTwo

    _storage.reallocate(
      capacity: capacity,
      writerSize: writerIndex,
      alignment: alignment
    )
  }

  @inline(__always)
  mutating func addPadding(bytes: Int) {
    writerIndex =
      writerIndex
      &+ numericCast(
        padding(
          bufSize: numericCast(writerIndex),
          elementSize: numericCast(bytes)))
    ensureSpace(size: writerIndex)
  }

  @inline(__always)
  mutating func writeBytes(_ ptr: UnsafeRawPointer, len: Int) {
    memcpy(
      _storage.memory.advanced(by: writerIndex),
      ptr,
      len)
    writerIndex = writerIndex &+ len
  }

  @inline(__always)
  mutating func write<T>(_ v: T, len: Int) {
    withUnsafePointer(to: v) {
      memcpy(
        _storage.memory.advanced(by: writerIndex),
        $0,
        len)
      writerIndex = writerIndex &+ len
    }
  }

  @discardableResult
  @inline(__always)
  func withUnsafeBytes<T>(
    _ body: (UnsafeRawBufferPointer) throws
      -> T
  ) rethrows -> T {
    try body(
      UnsafeRawBufferPointer(
        start: _storage.memory,
        count: capacity))
  }

  @discardableResult
  @inline(__always)
  func withUnsafeSlicedBytes<T>(
    _ body: (UnsafeRawBufferPointer) throws
      -> T
  ) rethrows -> T {
    try body(
      UnsafeRawBufferPointer(
        start: _storage.memory,
        count: writerIndex))
  }

  @discardableResult
  @inline(__always)
  func withUnsafeRawPointer<T>(
    _ body: (UnsafeMutableRawPointer) throws
      -> T
  ) rethrows -> T {
    try body(_storage.memory)
  }

  @discardableResult
  @inline(__always)
  func readWithUnsafeRawPointer<T>(
    position: Int,
    _ body: (UnsafeRawPointer) throws -> T
  ) rethrows -> T {
    try body(_storage.memory.advanced(by: position))
  }
}

extension _InternalByteBuffer: CustomDebugStringConvertible {

  public var debugDescription: String {
    """
    buffer located at: \(_storage.memory), with capacity of \(capacity)
    { writerIndex: \(writerIndex) }
    """
  }
}
