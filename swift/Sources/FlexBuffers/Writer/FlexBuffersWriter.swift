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

private let twentyFourBytes: Int = 24
public typealias FlexBuffersWriterBuilder = (inout FlexBuffersWriter) -> Void

public struct FlexBuffersWriter {

  var capacity: Int {
    _bb.capacity
  }

  var writerIndex: Int {
    _bb.writerIndex
  }

  private var finished = false
  private var hasDuplicatedKeys = false
  private var minBitWidth: BitWidth = .w8
  private var _bb: _InternalByteBuffer
  private var stack: [Value] = []
  private var keyPool: [Int: UInt] = [:]
  private var stringPool: [Int: UInt] = [:]
  private var flags: BuilderFlag

  public init(initialSize: Int = 1024, flags: BuilderFlag = .shareKeys) {
    _bb = _InternalByteBuffer(initialSize: initialSize)
    self.flags = flags
  }

  /// Returns the written bytes into the ``ByteBuffer``
  ///
  /// Should only be used after ``finish(offset:addPrefix:)`` is called
  public var sizedByteArray: [UInt8] {
    assert(
      finished == true,
      "function finish() should be called before accessing data")
    return _bb.underlyingBytes
  }

  public var sizedByteBuffer: ByteBuffer {
    assert(
      finished == true,
      "function finish() should be called before accessing data")
    return _bb.withUnsafeSlicedBytes {
      ByteBuffer(copyingMemoryBound: $0.baseAddress!, capacity: $0.count)
    }
  }

  public var byteBuffer: ByteBuffer {
    assert(
      finished == true,
      "function finish() should be called before accessing data")
    return ByteBuffer(byteBuffer: _bb)
  }

  /// Resets the internal state. Automatically called before building a new flexbuffer.
  public mutating func reset() {
    _bb.clear()
    stack.removeAll(keepingCapacity: true)
    finished = false
    minBitWidth = .w8
    keyPool.removeAll()
    stringPool.removeAll()
  }

  // MARK: - Storing root
  public mutating func finish() {
    assert(stack.count == 1)

    // Write root value.
    var byteWidth = align(
      width: stack[0].elementWidth(
        size: writerIndex,
        index: 0))

    write(value: stack[0], byteWidth: byteWidth)
    var storedType = stack[0].storedPackedType()
    // Write root type.
    _bb.writeBytes(&storedType, len: 1)
    // Write root size. Normally determined by parent, but root has no parent :)
    _bb.writeBytes(&byteWidth, len: 1)

    finished = true
  }

  // MARK: - Vector
  @discardableResult
  @inline(__always)
  public func startVector() -> Int {
    stack.count
  }

  @discardableResult
  @inline(__always)
  public mutating func startVector(key k: String) -> Int {
    add(key: k)
    return stack.count
  }

  @discardableResult
  public mutating func endVector(
    start: Int,
    typed: Bool = false,
    fixed: Bool = false) -> UInt64
  {
    let vec = createVector(
      start: start,
      count: stack.count - start,
      step: 1,
      typed: typed,
      fixed: fixed,
      keys: nil)
    stack = Array(stack[..<start])
    stack.append(vec)
    return vec.u
  }

  @discardableResult
  @inline(__always)
  public mutating func create<T>(vector: [T]) -> Int where T: Scalar {
    create(vector: vector, fixed: false)
  }

  @discardableResult
  @inline(__always)
  public mutating func create<T>(vector: [T], key: borrowing String) -> Int
    where T: Scalar
  {
    add(key: key)
    return create(vector: vector, fixed: false)
  }

  @discardableResult
  @inline(__always)
  public mutating func createFixed<T>(vector: [T]) -> Int where T: Scalar {
    assert(vector.count >= 2 && vector.count <= 4)
    return create(vector: vector, fixed: true)
  }

  @discardableResult
  @inline(__always)
  public mutating func createFixed<T>(
    vector: [T],
    key: borrowing String) -> Int where T: Scalar
  {
    assert(vector.count >= 2 && vector.count <= 4)
    add(key: key)
    return create(vector: vector, fixed: true)
  }

  // MARK: - Map
  @discardableResult
  @inline(__always)
  public func startMap() -> Int {
    stack.count
  }

  @discardableResult
  @inline(__always)
  public mutating func startMap(key k: String) -> Int {
    add(key: k)
    return stack.count
  }

  @discardableResult
  public mutating func endMap(start: Int) -> UInt64 {
    let len = sortMapByKeys(start: start)

    let keys = createVector(
      start: start,
      count: len,
      step: 2,
      typed: true,
      fixed: false)
    let vec = createVector(
      start: start + 1,
      count: len,
      step: 2,
      typed: false,
      fixed: false,
      keys: keys)
    stack = Array(stack[..<start])
    stack.append(vec)
    return numericCast(vec.u)
  }

  // MARK: - Write Null

  @inline(__always)
  public mutating func addNil() {
    stack.append(Value.nil)
  }

  @inline(__always)
  public mutating func addNil(key: borrowing String) {
    add(key: key)
    addNil()
  }

  // MARK: - Write Bool

  @inline(__always)
  public mutating func add(bool: borrowing Bool) {
    stack.append(Value(bool: bool))
  }

  @inline(__always)
  public mutating func add(bool: borrowing Bool, key: borrowing String) {
    add(key: key)
    add(bool: bool)
  }

  // MARK: - Write UInt

  @inline(__always)
  public mutating func add(uint8 value: UInt8) {
    add(uint64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(uint8 value: UInt8, key: borrowing String) {
    add(key: key)
    add(uint64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(uint16 value: UInt16) {
    add(uint64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(uint16 value: UInt16, key: borrowing String) {
    add(key: key)
    add(uint64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(uint32 value: UInt32) {
    add(uint64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(uint32 value: UInt32, key: borrowing String) {
    add(key: key)
    add(uint64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(uint value: UInt) {
    add(uint64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(uint value: UInt, key: borrowing String) {
    add(key: key)
    add(uint64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(uint64 value: borrowing UInt64) {
    stack.append(Value(v: value, type: .uint, bitWidth: widthU(value)))
  }

  @inline(__always)
  public mutating func add(
    uint64 value: borrowing UInt64,
    key: borrowing String)
  {
    add(key: key)
    add(uint64: value)
  }

  @inline(__always)
  public mutating func indirect(uint64 val: borrowing UInt64) {
    pushIndirect(value: val, type: .indirectUInt, bitWidth: widthU(val))
  }

  @inline(__always)
  public mutating func indirect(
    uint64 val: borrowing UInt64,
    key: borrowing String)
  {
    add(key: key)
    indirect(uint64: val)
  }

  @inline(__always)
  public mutating func indirect(
    uint val: borrowing UInt,
    key: borrowing String)
  {
    add(key: key)
    indirect(uint64: numericCast(val))
  }

  // MARK: - Write Int

  @inline(__always)
  public mutating func add(int8 value: Int8) {
    add(int64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(int8 value: Int8, key: borrowing String) {
    add(key: key)
    add(int64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(int16 value: Int16) {
    add(int64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(int16 value: Int16, key: borrowing String) {
    add(key: key)
    add(int64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(int32 value: Int32) {
    add(int64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(int32 value: Int32, key: borrowing String) {
    add(key: key)
    add(int64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(int value: Int) {
    add(int64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(int value: Int, key: borrowing String) {
    add(key: key)
    add(int64: numericCast(value))
  }

  @inline(__always)
  public mutating func add(int64 value: borrowing Int64) {
    stack.append(Value(v: value, type: .int, bitWidth: widthI(value)))
  }

  @inline(__always)
  public mutating func add(
    int64 value: borrowing Int64,
    key: borrowing String)
  {
    add(key: key)
    add(int64: value)
  }

  @inline(__always)
  public mutating func indirect(int64 val: borrowing Int64) {
    pushIndirect(value: val, type: .indirectInt, bitWidth: widthI(val))
  }

  @inline(__always)
  public mutating func indirect(
    int64 val: borrowing Int64,
    key: borrowing String)
  {
    add(key: key)
    indirect(int64: val)
  }

  @inline(__always)
  public mutating func indirect(
    int val: borrowing Int,
    key: borrowing String)
  {
    add(key: key)
    indirect(int64: numericCast(val))
  }

  // MARK: - Write Floats

  @inline(__always)
  public mutating func add(float32 value: borrowing Float32) {
    stack.append(Value(v: Double(value), type: .float, bitWidth: .w32))
  }

  @inline(__always)
  public mutating func add(
    float32 value: borrowing Float32,
    key: borrowing String)
  {
    add(key: key)
    add(float32: value)
  }

  @inline(__always)
  public mutating func indirect(float32 val: borrowing Float32) {
    pushIndirect(value: val, type: .indirectFloat, bitWidth: .w32)
  }

  @inline(__always)
  public mutating func indirect(
    float32 val: borrowing Float32,
    key: borrowing String)
  {
    add(key: key)
    indirect(float32: val)
  }

  @inline(__always)
  public mutating func add(double value: borrowing Double) {
    stack.append(Value(v: value, type: .float, bitWidth: widthF(value)))
  }

  @inline(__always)
  public mutating func add(
    double value: borrowing Double,
    key: borrowing String)
  {
    add(key: key)
    add(double: value)
  }

  @inline(__always)
  public mutating func indirect(double val: borrowing Double) {
    pushIndirect(value: val, type: .indirectFloat, bitWidth: widthF(val))
  }

  @inline(__always)
  public mutating func indirect(
    double val: borrowing Double,
    key: borrowing String)
  {
    add(key: key)
    indirect(double: val)
  }

  // MARK: - Writing strings
  @inline(__always)
  public mutating func add(string: borrowing String, key: borrowing String) {
    add(key: key)
    write(str: string, len: string.count)
  }

  @inline(__always)
  public mutating func add(string: borrowing String) {
    write(str: string, len: string.count)
  }

  // MARK: - Writing Blobs
  @discardableResult
  @inline(__always)
  public mutating func add<T>(
    blob: borrowing T,
    length l: Int) -> UInt where T: ContiguousBytes
  {
    storeBlob(blob, len: l, type: .blob)
  }

  @discardableResult
  @inline(__always)
  public mutating func add<T>(
    blob: borrowing T,
    key: borrowing String,
    length l: Int) -> UInt where T: ContiguousBytes
  {
    add(key: key)
    return storeBlob(blob, len: l, type: .blob)
  }

  // MARK: - Reuse Values
  @inline(__always)
  public func lastValue() -> Value? {
    return stack.last
  }

  @inline(__always)
  public mutating func reuse(value: Value) {
    stack.append(value)
  }

  @inline(__always)
  public mutating func reuse(value: Value, key: borrowing String) {
    add(key: key)
    reuse(value: value)
  }

  // MARK: - Private -

  // MARK: Writing to buffer

  @usableFromInline
  mutating func write(value: Value, byteWidth: Int) {
    switch value.type {
    case .null, .int: write(value: value.i, byteWidth: byteWidth)
    case .bool, .uint: write(value: value.u, byteWidth: byteWidth)
    case .float: write(double: value.f, byteWidth: byteWidth)
    default:
      write(offset: value.u, byteWidth: byteWidth)
    }
  }

  @usableFromInline
  mutating func pushIndirect<T>(
    value: T,
    type: FlexBufferType,
    bitWidth: BitWidth)
  {
    let byteWidth = align(width: bitWidth)
    let iloc = writerIndex
    _bb.ensureSpace(size: byteWidth)

    _bb.write(value, len: byteWidth)
    stack.append(
      Value(
        sloc: .u(numericCast(iloc)),
        type: type,
        bitWidth: bitWidth))
  }

  // MARK: Internal Writing Strings

  /// Adds a string to the buffer using swift.utf8 object
  /// - Parameter str: String that will be added to the buffer
  /// - Parameter len: length of the string
  @discardableResult
  @usableFromInline
  mutating func write(str: borrowing String, len: Int) -> UInt {
    let resetTo = writerIndex
    var sloc = str.withCString {
      storeBlob(pointer: $0, len: len, trailing: 1, type: .string)
    }

    if flags >= .shareKeysAndStrings {
      let loc = stringPool[str.hashValue]
      if let loc {
        _bb.resetWriter(to: resetTo)
        sloc = loc
        assert(
          stack.count > 0,
          "Attempting to override the location, but stack is empty")
        stack[stack.count - 1].sloc = .u(numericCast(sloc))
      } else {
        stringPool[str.hashValue] = sloc
      }
    }
    return sloc
  }

  // MARK: Write Keys
  @discardableResult
  @inline(__always)
  private mutating func add(key: borrowing String) -> UInt {
    add(key: key, len: key.count)
  }

  @discardableResult
  @usableFromInline
  mutating func add(key: borrowing String, len: Int) -> UInt {
    _bb.ensureSpace(size: len)

    var sloc: UInt = numericCast(writerIndex)
    key.withCString {
      _bb.writeBytes($0, len: len + 1)
    }

    if flags > .shareKeys {
      let loc = keyPool[key.hashValue]
      if let loc {
        _bb.resetWriter(to: Int(sloc))
        sloc = loc
      } else {
        keyPool[key.hashValue] = sloc
      }
    }
    stack.append(Value(sloc: .u(numericCast(sloc)), type: .key, bitWidth: .w8))
    return sloc
  }

  // MARK: - Storing Blobs
  @usableFromInline
  mutating func storeBlob<T>(
    _ bytes: T,
    len: Int,
    type: FlexBufferType) -> UInt where T: ContiguousBytes
  {
    return bytes.withUnsafeBytes {
      storeBlob(pointer: $0.baseAddress!, len: len, type: type)
    }
  }

  @discardableResult
  @usableFromInline
  mutating func storeBlob(
    pointer: borrowing UnsafeRawPointer,
    len: Int,
    trailing: Int = 0,
    type: FlexBufferType) -> UInt
  {
    _bb.ensureSpace(size: len &+ trailing)
    let bitWidth = widthU(numericCast(len))

    let bytes = align(width: bitWidth)

    var len = len
    _bb.writeBytes(&len, len: bytes)
    let sloc = writerIndex

    _bb.writeBytes(pointer, len: len &+ trailing)
    stack.append(
      Value(
        sloc: .u(numericCast(sloc)),
        type: type,
        bitWidth: bitWidth))
    return numericCast(sloc)
  }

  // MARK: Write Vectors
  @discardableResult
  @usableFromInline
  mutating func create<T>(vector: [T], fixed: Bool) -> Int
    where T: Scalar
  {
    let length: UInt64 = numericCast(vector.count)
    let vectorType = getScalarType(type: T.self)
    let byteWidth = MemoryLayout<T>.size
    let bitWidth = BitWidth.widthB(byteWidth)

    _bb.ensureSpace(size: vector.count &* Int(bitWidth.rawValue))

    assert(widthU(length) <= bitWidth)

    align(width: bitWidth)

    if !fixed {
      write(value: length, byteWidth: byteWidth)
    }
    let vloc = _bb.writerIndex

    for i in stride(from: 0, to: vector.count, by: 1) {
      write(value: vector[i], byteWidth: byteWidth)
    }

    stack.append(
      Value(
        sloc: .u(numericCast(vloc)),
        type: toTypedVector(type: vectorType, length: fixed ? length : 0),
        bitWidth: bitWidth))
    return vloc
  }

  @usableFromInline
  mutating func createVector(
    start: Int,
    count: Int,
    step: Int,
    typed: Bool,
    fixed: Bool,
    keys: Value? = nil) -> Value
  {
    assert(
      !fixed || typed,
      "Typed false and fixed true is a combination not supported currently")

    var bitWidth = BitWidth.max(minBitWidth, rhs: widthU(numericCast(count)))
    var prefixElements = 1
    if keys != nil {
      /// If this vector is part of a map, we will pre-fix an offset to the keys
      /// to this vector.
      bitWidth = max(bitWidth, keys!.elementWidth(size: writerIndex, index: 0))
      prefixElements += 2
    }
    var vectorType: FlexBufferType = .key

    for i in stride(from: start, to: stack.count, by: step) {
      let elemWidth = stack[i].elementWidth(
        size: _bb.writerIndex,
        index: numericCast(i &- start &+ prefixElements))
      bitWidth = BitWidth.max(bitWidth, rhs: elemWidth)
      guard typed else { continue }
      if i == start {
        vectorType = stack[i].type
      } else {
        assert(
          vectorType == stack[i].type,
          """
          If you get this assert you are writing a typed vector 
          with elements that are not all the same type
          """)
      }
    }
    assert(
      !typed || isTypedVectorElementType(type: vectorType),
      """
      If you get this assert, your typed types are not one of:
      Int / UInt / Float / Key.
      """)

    let byteWidth = align(width: bitWidth)

    let currentSize: Int = count &* step &* byteWidth
    let requiredSize: Int = if !typed {
      // We ensure that we have enough space
      // for loop two write operations &
      // 24 bytes for when its not fixed,
      // and keys isn't null. As an extra safe
      // guard
      (currentSize &* 2) &+ twentyFourBytes
    } else {
      currentSize
    }

    _bb.ensureSpace(
      size: requiredSize)

    if keys != nil {
      write(offset: keys!.u, byteWidth: byteWidth)
      write(value: UInt64.one << keys!.bitWidth.rawValue, byteWidth: byteWidth)
    }

    if !fixed {
      write(value: count, byteWidth: byteWidth)
    }

    let vloc = _bb.writerIndex

    for i in stride(from: start, to: stack.count, by: step) {
      write(value: stack[i], byteWidth: byteWidth)
    }

    if !typed {
      for i in stride(from: start, to: stack.count, by: step) {
        _bb.write(stack[i].storedPackedType(width: bitWidth), len: 1)
      }
    }

    let type: FlexBufferType =
      if keys != nil {
        .map
      } else if typed {
        toTypedVector(type: vectorType, length: numericCast(fixed ? count : 0))
      } else {
        .vector
      }

    return Value(sloc: .u(numericCast(vloc)), type: type, bitWidth: bitWidth)
  }

  // MARK: Write Scalar functions
  @inline(__always)
  private mutating func write(offset: UInt64, byteWidth: Int) {
    let offset: UInt64 = numericCast(writerIndex) &- offset
    assert(byteWidth == 8 || offset < UInt64.one << (byteWidth * 8))
    withUnsafePointer(to: offset) {
      _bb.writeBytes($0, len: byteWidth)
    }
  }

  @inline(__always)
  private mutating func write<T>(value: T, byteWidth: Int) where T: Scalar {
    withUnsafePointer(to: value) {
      _bb.writeBytes($0, len: byteWidth)
    }
  }

  @inline(__always)
  private mutating func write(double value: Double, byteWidth: Int) {
    switch byteWidth {
    case 8: write(value: value, byteWidth: byteWidth)
    case 4: write(value: Float(value), byteWidth: byteWidth)
    default: assert(false, "Should never reach here")
    }
  }

  // MARK: Misc functions
  @discardableResult
  @inline(__always)
  private mutating func align(width: BitWidth) -> Int {
    let bytes: Int = numericCast(UInt32.one << width.rawValue)
    _bb.addPadding(bytes: bytes)
    return bytes
  }

  @usableFromInline
  mutating func sortMapByKeys(start: Int) -> Int {
    let len = mapElementCount(start: start)
    for index in stride(from: start, to: stack.count, by: 2) {
      assert(stack[index].type == .key)
    }

    struct TwoValue: Equatable {
      let key, value: Value
    }

    stack[start...].withUnsafeMutableBytes { buffer in
      var ptr = buffer.assumingMemoryBound(to: TwoValue.self)
      ptr.sort { a, b in
        let aMem = _bb.memory.advanced(by: numericCast(a.key.u))
          .assumingMemoryBound(to: CChar.self)
        let bMem = _bb.memory.advanced(by: numericCast(b.key.u))
          .assumingMemoryBound(to: CChar.self)
        let comp = strcmp(aMem, bMem)
        if (comp == 0) && a != b { hasDuplicatedKeys = true }
        return comp < 0
      }
    }
    return len
  }

  @inline(__always)
  private func mapElementCount(start: Int) -> Int {
    var len = stack.count - start
    assert((len & 1) == 0)
    len /= 2
    return len
  }
}

// MARK: - Vectors helper functions
extension FlexBuffersWriter {
  @discardableResult
  public mutating func vector(
    key: String,
    _ closure: FlexBuffersWriterBuilder) -> UInt64
  {
    let start = startVector(key: key)
    closure(&self)
    return endVector(start: start)
  }

  @discardableResult
  public mutating func vector(_ closure: FlexBuffersWriterBuilder)
    -> UInt64
  {
    let start = startVector()
    closure(&self)
    return endVector(start: start)
  }
}

// MARK: - Maps helper functions
extension FlexBuffersWriter {
  @discardableResult
  public mutating func map(
    key: String,
    _ closure: FlexBuffersWriterBuilder) -> UInt64
  {
    let start = startMap(key: key)
    closure(&self)
    return endMap(start: start)
  }

  @discardableResult
  public mutating func map(_ closure: FlexBuffersWriterBuilder)
    -> UInt64
  {
    let start = startMap()
    closure(&self)
    return endMap(start: start)
  }
}
