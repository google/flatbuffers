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

/// `ByteBuffer` is the interface that stores the data for a `Flatbuffers` object
/// it allows users to Construct their buffers internally without much cost to performance
@usableFromInline
struct _InternalByteBuffer {

  /// Storage is a container that would hold the memory pointer to solve the issue of
  /// deallocating the memory that was held by (memory: UnsafeMutableRawPointer)
  @usableFromInline
  final class Storage {
    // This storage doesn't own the memory, therefore, we won't deallocate on deinit.
    private let unowned: Bool
    /// pointer to the start of the buffer object in memory
    var memory: UnsafeMutableRawPointer
    /// Capacity of UInt8 the buffer can hold
    var capacity: Int

    @usableFromInline
    init(count: Int, alignment: Int) {
      memory = UnsafeMutableRawPointer.allocate(
        byteCount: count,
        alignment: alignment)
      capacity = count
      unowned = false
    }

    deinit {
      if !unowned {
        memory.deallocate()
      }
    }

    @usableFromInline
    func initialize(for size: Int) {
      assert(
        !unowned,
        "initalize should NOT be called on a buffer that is built by assumingMemoryBound")
      memset(memory, 0, size)
    }

    /// Reallocates the buffer incase the object to be written doesnt fit in the current buffer
    /// - Parameter size: Size of the current object
    @usableFromInline
    func reallocate(_ size: Int, writerSize: Int, alignment: Int) {
      let currentWritingIndex = capacity &- writerSize
      while capacity <= writerSize &+ size {
        capacity = capacity << 1
      }

      /// solution take from Apple-NIO
      capacity = capacity.convertToPowerofTwo

      let newData = UnsafeMutableRawPointer.allocate(
        byteCount: capacity,
        alignment: alignment)
      memset(newData, 0, capacity &- writerSize)
      memcpy(
        newData.advanced(by: capacity &- writerSize),
        memory.advanced(by: currentWritingIndex),
        writerSize)
      memory.deallocate()
      memory = newData
    }
  }

  @usableFromInline var _storage: Storage

  /// The size of the elements written to the buffer + their paddings
  private var _writerSize: Int = 0
  /// Alignment of the current  memory being written to the buffer
  var alignment = 1
  /// Current Index which is being used to write to the buffer, it is written from the end to the start of the buffer
  var writerIndex: Int { _storage.capacity &- _writerSize }

  /// Reader is the position of the current Writer Index (capacity - size)
  public var reader: Int { writerIndex }
  /// Current size of the buffer
  public var size: UOffset { UOffset(_writerSize) }
  /// Public Pointer to the buffer object in memory. This should NOT be modified for any reason
  @usableFromInline
  var memory: UnsafeMutableRawPointer { _storage.memory }
  /// Current capacity for the buffer
  public var capacity: Int { _storage.capacity }

  /// Constructor that creates a Flatbuffer instance with a size
  /// - Parameter:
  ///   - size: Length of the buffer
  ///   - allowReadingUnalignedBuffers: allow reading from unaligned buffer
  init(initialSize size: Int) {
    let size = size.convertToPowerofTwo
    _storage = Storage(count: size, alignment: alignment)
    _storage.initialize(for: size)
  }

  /// Fills the buffer with padding by adding to the writersize
  /// - Parameter padding: Amount of padding between two to be serialized objects
  @inline(__always)
  @usableFromInline
  mutating func fill(padding: Int) {
    assert(padding >= 0, "Fill should be larger than or equal to zero")
    ensureSpace(size: padding)
    _writerSize = _writerSize &+ (MemoryLayout<UInt8>.size &* padding)
  }

  /// Adds an array of type Scalar to the buffer memory
  /// - Parameter elements: An array of Scalars
  @inline(__always)
  @usableFromInline
  mutating func push<T: Scalar>(elements: [T]) {
    elements.withUnsafeBytes { ptr in
      ensureSpace(size: ptr.count)
      memcpy(
        _storage.memory.advanced(by: writerIndex &- ptr.count),
        ptr.baseAddress!,
        ptr.count)
      _writerSize = _writerSize &+ ptr.count
    }
  }

  /// Adds an array of type Scalar to the buffer memory
  /// - Parameter elements: An array of Scalars
  @inline(__always)
  @usableFromInline
  mutating func push<T: NativeStruct>(elements: [T]) {
    elements.withUnsafeBytes { ptr in
      ensureSpace(size: ptr.count)
      memcpy(
        _storage.memory.advanced(by: writerIndex &- ptr.count),
        ptr.baseAddress!,
        ptr.count)
      _writerSize = _writerSize &+ ptr.count
    }
  }

  /// Adds a `ContiguousBytes` to buffer memory
  /// - Parameter value: bytes to copy
  #if swift(>=5.0) && !os(WASI)
  @inline(__always)
  @usableFromInline
  mutating func push(bytes: ContiguousBytes) {
    bytes.withUnsafeBytes { ptr in
      ensureSpace(size: ptr.count)
      memcpy(
        _storage.memory.advanced(by: writerIndex &- ptr.count),
        ptr.baseAddress!,
        ptr.count)
      _writerSize = _writerSize &+ ptr.count
    }
  }
  #endif

  /// Adds an object of type NativeStruct into the buffer
  /// - Parameters:
  ///   - value: Object  that will be written to the buffer
  ///   - size: size to subtract from the WriterIndex
  @usableFromInline
  @inline(__always)
  mutating func push<T: NativeStruct>(struct value: T, size: Int) {
    ensureSpace(size: size)
    withUnsafePointer(to: value) {
      memcpy(
        _storage.memory.advanced(by: writerIndex &- size),
        $0,
        size)
      _writerSize = _writerSize &+ size
    }
  }

  /// Adds an object of type Scalar into the buffer
  /// - Parameters:
  ///   - value: Object  that will be written to the buffer
  ///   - len: Offset to subtract from the WriterIndex
  @inline(__always)
  @usableFromInline
  mutating func push<T: Scalar>(value: T, len: Int) {
    ensureSpace(size: len)
    withUnsafePointer(to: value) {
      memcpy(
        _storage.memory.advanced(by: writerIndex &- len),
        $0,
        len)
      _writerSize = _writerSize &+ len
    }
  }

  /// Adds a string to the buffer using swift.utf8 object
  /// - Parameter str: String that will be added to the buffer
  /// - Parameter len: length of the string
  @inline(__always)
  @usableFromInline
  mutating func push(string str: String, len: Int) {
    ensureSpace(size: len)
    if str.utf8
      .withContiguousStorageIfAvailable({ self.push(bytes: $0, len: len) }) !=
      nil
    {
    } else {
      let utf8View = str.utf8
      for c in utf8View.reversed() {
        push(value: c, len: 1)
      }
    }
  }

  /// Writes a string to Bytebuffer using UTF8View
  /// - Parameters:
  ///   - bytes: Pointer to the view
  ///   - len: Size of string
  @usableFromInline
  @inline(__always)
  mutating func push(
    bytes: UnsafeBufferPointer<String.UTF8View.Element>,
    len: Int) -> Bool
  {
    memcpy(
      _storage.memory.advanced(by: writerIndex &- len),
      bytes.baseAddress!,
      len)
    _writerSize = _writerSize &+ len
    return true
  }

  /// Write stores an object into the buffer directly or indirectly.
  ///
  /// Direct: ignores the capacity of buffer which would mean we are referring to the direct point in memory
  /// indirect: takes into respect the current capacity of the buffer (capacity - index), writing to the buffer from the end
  /// - Parameters:
  ///   - value: Value that needs to be written to the buffer
  ///   - index: index to write to
  ///   - direct: Should take into consideration the capacity of the buffer
  @inline(__always)
  func write<T>(value: T, index: Int, direct: Bool = false) {
    var index = index
    if !direct {
      index = _storage.capacity &- index
    }
    assert(index < _storage.capacity, "Write index is out of writing bound")
    assert(index >= 0, "Writer index should be above zero")
    withUnsafePointer(to: value) {
      memcpy(
        _storage.memory.advanced(by: index),
        $0,
        MemoryLayout<T>.size)
    }
  }

  /// Makes sure that buffer has enouch space for each of the objects that will be written into it
  /// - Parameter size: size of object
  @discardableResult
  @usableFromInline
  @inline(__always)
  mutating func ensureSpace(size: Int) -> Int {
    if size &+ _writerSize > _storage.capacity {
      _storage.reallocate(size, writerSize: _writerSize, alignment: alignment)
    }
    assert(size < FlatBufferMaxSize, "Buffer can't grow beyond 2 Gigabytes")
    return size
  }

  /// pops the written VTable if it's already written into the buffer
  /// - Parameter size: size of the `VTable`
  @usableFromInline
  @inline(__always)
  mutating func pop(_ size: Int) {
    assert(
      (_writerSize &- size) > 0,
      "New size should NOT be a negative number")
    memset(_storage.memory.advanced(by: writerIndex), 0, _writerSize &- size)
    _writerSize = size
  }

  /// Clears the current size of the buffer
  @inline(__always)
  mutating public func clearSize() {
    _writerSize = 0
  }

  /// Clears the current instance of the buffer, replacing it with new memory
  @inline(__always)
  mutating public func clear() {
    _writerSize = 0
    alignment = 1
    _storage.initialize(for: _storage.capacity)
  }

  /// Reads an object from the buffer
  /// - Parameters:
  ///   - def: Type of the object
  ///   - position: the index of the object in the buffer
  @inline(__always)
  public func read<T>(def: T.Type, position: Int) -> T {
    _storage.memory
      .advanced(by: position)
      .bindMemory(to: T.self, capacity: 1)
      .pointee
  }

  @discardableResult
  @inline(__always)
  func withUnsafeBytes<T>(
    _ body: (UnsafeRawBufferPointer) throws
      -> T) rethrows -> T
  {
    try body(UnsafeRawBufferPointer(
      start: _storage.memory,
      count: capacity))
  }

  @discardableResult
  @inline(__always)
  func withUnsafeSlicedBytes<T>(
    _ body: (UnsafeRawBufferPointer) throws
      -> T) rethrows -> T
  {
    try body(UnsafeRawBufferPointer(
      start: _storage.memory.advanced(by: writerIndex),
      count: capacity &- writerIndex))
  }

  @discardableResult
  @inline(__always)
  func withUnsafeRawPointer<T>(
    _ body: (UnsafeMutableRawPointer) throws
      -> T) rethrows -> T
  {
    try body(_storage.memory)
  }

  @discardableResult
  @inline(__always)
  func readWithUnsafeRawPointer<T>(
    position: Int,
    _ body: (UnsafeRawPointer) throws -> T) rethrows -> T
  {
    try body(_storage.memory.advanced(by: position))
  }
}

extension _InternalByteBuffer: CustomDebugStringConvertible {

  public var debugDescription: String {
    """
    buffer located at: \(_storage.memory), with capacity of \(_storage.capacity)
    { writerSize: \(_writerSize), readerSize: \(reader), writerIndex: \(
      writerIndex) }
    """
  }
}
