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

/// `ByteBuffer` is the interface that stores the data for a `Flatbuffers` object
/// it allows users to write and read data directly from memory thus the use of its
/// functions should be used
@frozen
public struct ByteBuffer {

  /// Storage is a container that would hold the memory pointer to solve the issue of
  /// deallocating the memory that was held by (memory: UnsafeMutableRawPointer)
  @usableFromInline
  final class Storage {
    @usableFromInline
    enum Blob {
      #if !os(WASI)
      case data(Data)
      case bytes(ContiguousBytes)
      #endif

      case byteBuffer(_InternalByteBuffer)
      case array([UInt8])
      case pointer(UnsafeMutableRawPointer)
      case writePointer(UnsafeMutableRawPointer)

      var isOwned: Bool {
        switch self {
        case .writePointer: true
        default: false
        }
      }
    }

    /// Retained blob of data that requires the storage to retain a pointer to.
    @usableFromInline
    var retainedBlob: Blob
    /// Capacity of UInt8 the buffer can hold
    var capacity: Int

    @usableFromInline
    init(count: Int) {
      let memory = UnsafeMutableRawPointer.allocate(
        byteCount: count,
        alignment: MemoryLayout<UInt8>.alignment)
      capacity = count
      retainedBlob = .writePointer(memory)
    }

    @usableFromInline
    init(blob: Blob, capacity count: Int) {
      capacity = count
      retainedBlob = blob
    }

    deinit {
      switch retainedBlob {
      case .writePointer(let unsafeMutableRawPointer):
        unsafeMutableRawPointer.deallocate()
      default: break
      }
    }

    @usableFromInline
    func copy(from ptr: UnsafeRawPointer, count: Int) {
      assert(
        retainedBlob.isOwned,
        "copy should NOT be called on a buffer that is built by assumingMemoryBound")
      withUnsafeRawPointer {
        $0.copyMemory(from: ptr, byteCount: count)
      }
    }

    @usableFromInline
    func initialize(for size: Int) {
      assert(
        retainedBlob.isOwned,
        "initalize should NOT be called on a buffer that is built by assumingMemoryBound")
      withUnsafeRawPointer {
        memset($0, 0, size)
      }
    }

    @inline(__always)
    func withUnsafeBytes<T>(
      _ body: (UnsafeRawBufferPointer) throws
        -> T) rethrows -> T
    {
      switch retainedBlob {
      case .byteBuffer(let byteBuffer):
        try byteBuffer.withUnsafeBytes(body)
      case .data(let data):
        try data.withUnsafeBytes(body)
      case .bytes(let contiguousBytes):
        try contiguousBytes.withUnsafeBytes(body)
      case .array(let array):
        try array.withUnsafeBytes(body)
      case .pointer(let ptr):
        try body(UnsafeRawBufferPointer(start: ptr, count: capacity))
      case .writePointer(let ptr):
        try body(UnsafeRawBufferPointer(start: ptr, count: capacity))
      }
    }

    @inline(__always)
    func withUnsafeRawPointer<T>(
      _ body: (UnsafeMutableRawPointer) throws
        -> T) rethrows -> T
    {
      switch retainedBlob {
      case .byteBuffer(let byteBuffer):
        try byteBuffer.withUnsafeRawPointer(body)
      case .data(let data):
        try data
          .withUnsafeBytes {
            try body(UnsafeMutableRawPointer(mutating: $0.baseAddress!))
          }
      case .bytes(let contiguousBytes):
        try contiguousBytes
          .withUnsafeBytes {
            try body(UnsafeMutableRawPointer(mutating: $0.baseAddress!))
          }
      case .array(let array):
        try array
          .withUnsafeBytes {
            try body(UnsafeMutableRawPointer(mutating: $0.baseAddress!))
          }
      case .pointer(let ptr):
        try body(ptr)
      case .writePointer(let ptr):
        try body(ptr)
      }
    }

    @inline(__always)
    func readWithUnsafeRawPointer<T>(
      position: Int,
      _ body: (UnsafeRawPointer) throws -> T) rethrows -> T
    {
      switch retainedBlob {
      case .byteBuffer(let byteBuffer):
        try byteBuffer.readWithUnsafeRawPointer(position: position, body)
      case .data(let data):
        try data.withUnsafeBytes {
          try body($0.baseAddress!.advanced(by: position))
        }
      case .bytes(let contiguousBytes):
        try contiguousBytes.withUnsafeBytes {
          try body($0.baseAddress!.advanced(by: position))
        }
      case .array(let array):
        try array.withUnsafeBytes {
          try body($0.baseAddress!.advanced(by: position))
        }
      case .pointer(let ptr):
        try body(ptr.advanced(by: position))
      case .writePointer(let ptr):
        try body(ptr.advanced(by: position))
      }
    }
  }

  @usableFromInline var _storage: Storage

  /// The size of the elements written to the buffer + their paddings
  private var _readerIndex: Int = 0
  /// Current Index which is being used to write to the buffer, it is written from the end to the start of the buffer
  var writerIndex: Int { _storage.capacity &- _readerIndex }
  /// Reader is the position of the current Writer Index (capacity - size)
  public var reader: Int { writerIndex }
  /// Current size of the buffer
  public var size: UOffset { UOffset(_readerIndex) }
  /// Current capacity for the buffer
  public var capacity: Int { _storage.capacity }

  /// Constructor that creates a Flatbuffer object from an InternalByteBuffer
  /// - Parameter
  ///   - bytes: Array of UInt8
  @inline(__always)
  init(byteBuffer: _InternalByteBuffer) {
    _storage = Storage(blob: .byteBuffer(byteBuffer), capacity: byteBuffer.capacity)
    _readerIndex = Int(byteBuffer.size)
  }

  /// Constructor that creates a Flatbuffer from unsafe memory region by copying
  /// the underlying data to a new pointer
  ///
  /// - Parameters:
  ///   - copyingMemoryBound: The unsafe memory region
  ///   - capacity: The size of the given memory region
  @inline(__always)
  public init(
    copyingMemoryBound memory: UnsafeRawPointer,
    capacity: Int)
  {
    _storage = Storage(count: capacity)
    _storage.copy(from: memory, count: capacity)
    _readerIndex = _storage.capacity
  }

  /// Constructor that creates a Flatbuffer object from a UInt8
  /// - Parameter
  ///   - bytes: Array of UInt8
  @inline(__always)
  public init(bytes: [UInt8]) {
    _storage = Storage(blob: .array(bytes), capacity: bytes.count)
    _readerIndex = _storage.capacity
  }

  #if !os(WASI)
  /// Constructor that creates a Flatbuffer from the Swift Data type object
  /// - Parameter
  ///   - data: Swift data Object
  @inline(__always)
  public init(data: Data) {
    _storage = Storage(blob: .data(data), capacity: data.count)
    _readerIndex = _storage.capacity
  }

  /// Constructor that creates a Flatbuffer object from a ContiguousBytes
  /// - Parameters:
  ///   - contiguousBytes: Binary stripe to use as the buffer
  ///   - count: amount of readable bytes
  @inline(__always)
  public init<Bytes: ContiguousBytes>(
    contiguousBytes: Bytes,
    count: Int)
  {
    _storage = Storage(blob: .bytes(contiguousBytes), capacity: count)
    _readerIndex = _storage.capacity
  }
  #endif

  /// Constructor that creates a Flatbuffer from unsafe memory region without copying
  /// **NOTE** Needs a call to `memory.deallocate()` later on to free the memory
  ///
  /// - Parameters:
  ///   - assumingMemoryBound: The unsafe memory region
  ///   - capacity: The size of the given memory region
  @inline(__always)
  public init(
    assumingMemoryBound memory: UnsafeMutableRawPointer,
    capacity: Int)
  {
    _storage = Storage(
      blob: .pointer(memory),
      capacity: capacity)
    _readerIndex = _storage.capacity
  }

  /// Creates a copy of the existing flatbuffer, by copying it to a different memory.
  /// - Parameters:
  ///   - memory: Current memory of the buffer
  ///   - count: count of bytes
  ///   - removeBytes: Removes a number of bytes from the current size
  @inline(__always)
  init(
    blob: Storage.Blob,
    count: Int,
    removing removeBytes: Int)
  {
    _storage = Storage(blob: blob, capacity: count)
    _readerIndex = removeBytes
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
    withUnsafePointer(to: value) { ptr in
      _storage.withUnsafeRawPointer {
        memcpy(
          $0.advanced(by: index),
          ptr,
          MemoryLayout<T>.size)
      }
    }
  }

  /// Reads an object from the buffer
  /// - Parameters:
  ///   - def: Type of the object
  ///   - position: the index of the object in the buffer
  @inline(__always)
  public func read<T>(def: T.Type, position: Int) -> T {
    _storage.readWithUnsafeRawPointer(position: position) {
      $0.bindMemory(to: T.self, capacity: 1)
        .pointee
    }
  }

  /// Reads a slice from the memory assuming a type of T
  /// - Parameters:
  ///   - index: index of the object to be read from the buffer
  ///   - count: count of bytes in memory
  @inline(__always)
  public func readSlice<T>(
    index: Int,
    count: Int) -> [T]
  {
    assert(
      index + count <= _storage.capacity,
      "Reading out of bounds is illegal")

    return _storage.readWithUnsafeRawPointer(position: index) {
      let buf = UnsafeBufferPointer(
        start: $0.bindMemory(to: T.self, capacity: count),
        count: count)
      return Array(buf)
    }
  }

  /// Provides a pointer towards the underlying primitive types
  /// - Parameters:
  ///   - index: index of the object to be read from the buffer
  ///   - count: count of bytes in memory
  @discardableResult
  @inline(__always)
  public func withUnsafePointerToSlice<T>(
    index: Int,
    count: Int,
    body: (UnsafeRawBufferPointer) throws -> T) rethrows -> T
  {
    assert(
      index + count <= _storage.capacity,
      "Reading out of bounds is illegal")
    return try _storage.readWithUnsafeRawPointer(position: index) {
      try body(UnsafeRawBufferPointer(start: $0, count: count))
    }
  }

  #if !os(WASI)
  /// Reads a string from the buffer and encodes it to a swift string
  /// - Parameters:
  ///   - index: index of the string in the buffer
  ///   - count: length of the string
  ///   - type: Encoding of the string
  @inline(__always)
  public func readString(
    at index: Int,
    count: Int,
    type: String.Encoding = .utf8) -> String?
  {
    assert(
      index + count <= _storage.capacity,
      "Reading out of bounds is illegal")
    return _storage.readWithUnsafeRawPointer(position: index) {
      let buf = UnsafeBufferPointer(
        start: $0.bindMemory(to: UInt8.self, capacity: count),
        count: count)
      return String(
        bytes: buf,
        encoding: type)
    }
  }
  #else
  /// Reads a string from the buffer and encodes it to a swift string
  /// - Parameters:
  ///   - index: index of the string in the buffer
  ///   - count: length of the string
  @inline(__always)
  public func readString(
    at index: Int,
    count: Int) -> String?
  {
    assert(
      index + count <= _storage.capacity,
      "Reading out of bounds is illegal")
    return _storage.retainedBlob.readWithUnsafeRawPointer(position: index) {
      String(cString: $0.bindMemory(to: UInt8.self, capacity: count))
    }
  }
  #endif

  /// Creates a new Flatbuffer object that's duplicated from the current one
  /// - Parameter removeBytes: the amount of bytes to remove from the current Size
  @inline(__always)
  public func duplicate(removing removeBytes: Int = 0) -> ByteBuffer {
    assert(removeBytes > 0, "Can NOT remove negative bytes")
    assert(
      removeBytes < _storage.capacity,
      "Can NOT remove more bytes than the ones allocated")
    return ByteBuffer(
      blob: _storage.retainedBlob,
      count: _storage.capacity,
      removing: _readerIndex &- removeBytes)
  }

  /// SkipPrefix Skips the first 4 bytes in case one of the following
  /// functions are called `getPrefixedSizeCheckedRoot` & `getPrefixedSizeRoot`
  /// which allows us to skip the first 4 bytes instead of recreating the buffer
  @discardableResult
  @usableFromInline
  @inline(__always)
  mutating func skipPrefix() -> Int32 {
    _readerIndex = _readerIndex &- MemoryLayout<Int32>.size
    return read(def: Int32.self, position: 0)
  }

  @discardableResult
  @inline(__always)
  public func withUnsafeBytes<T>(
    body: (UnsafeRawBufferPointer) throws
      -> T) rethrows -> T
  {
    try _storage.withUnsafeBytes(body)
  }

  @discardableResult
  @usableFromInline
  @inline(__always)
  func withUnsafeMutableRawPointer<T>(
    body: (UnsafeMutableRawPointer) throws
      -> T) rethrows -> T
  {
    try _storage.withUnsafeRawPointer(body)
  }
}

extension ByteBuffer: CustomDebugStringConvertible {

  public var debugDescription: String {
    """
    buffer located at: \(_storage.retainedBlob), 
    with capacity of \(_storage.capacity),
    { writerSize: \(_readerIndex), readerSize: \(reader), writerIndex: \(writerIndex) }
    """
  }
}
