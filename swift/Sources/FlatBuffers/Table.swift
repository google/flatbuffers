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

import Foundation

@frozen
public struct Table {
  public private(set) var bb: ByteBuffer
  public private(set) var postion: Int32

  public init(bb: ByteBuffer, position: Int32 = 0) {
    guard isLitteEndian else {
      fatalError("Reading/Writing a buffer in big endian machine is not supported on swift")
    }
    self.bb = bb
    postion = position
  }

  public func offset(_ o: Int32) -> Int32 {
    let vtable = postion - bb.read(def: Int32.self, position: Int(postion))
    return o < bb.read(def: VOffset.self, position: Int(vtable)) ? Int32(bb.read(
      def: Int16.self,
      position: Int(vtable + o))) : 0
  }

  public func indirect(_ o: Int32) -> Int32 { o + bb.read(def: Int32.self, position: Int(o)) }

  /// String reads from the buffer with respect to position of the current table.
  /// - Parameter offset: Offset of the string
  public func string(at offset: Int32) -> String? {
    directString(at: offset + postion)
  }

  /// Direct string reads from the buffer disregarding the position of the table.
  /// It would be preferable to use string unless the current position of the table is not needed
  /// - Parameter offset: Offset of the string
  public func directString(at offset: Int32) -> String? {
    var offset = offset
    offset += bb.read(def: Int32.self, position: Int(offset))
    let count = bb.read(def: Int32.self, position: Int(offset))
    let position = offset + Int32(MemoryLayout<Int32>.size)
    return bb.readString(at: position, count: count)
  }

  /// Reads from the buffer with respect to the position in the table.
  /// - Parameters:
  ///   - type: Type of Element that needs to be read from the buffer
  ///   - o: Offset of the Element
  public func readBuffer<T>(of type: T.Type, at o: Int32) -> T {
    directRead(of: T.self, offset: o + postion)
  }

  /// Reads from the buffer disregarding the position of the table.
  /// It would be used when reading from an
  ///   ```
  ///   let offset = __t.offset(10)
  ///   //Only used when the we already know what is the
  ///   // position in the table since __t.vector(at:)
  ///   // returns the index with respect to the position
  ///   __t.directRead(of: Byte.self,
  ///                  offset: __t.vector(at: offset) + index * 1)
  ///   ```
  /// - Parameters:
  ///   - type: Type of Element that needs to be read from the buffer
  ///   - o: Offset of the Element
  public func directRead<T>(of type: T.Type, offset o: Int32) -> T {
    let r = bb.read(def: T.self, position: Int(o))
    return r
  }

  public func union<T: FlatbuffersInitializable>(_ o: Int32) -> T {
    let o = o + postion
    return directUnion(o)
  }

  public func directUnion<T: FlatbuffersInitializable>(_ o: Int32) -> T {
    T.init(bb, o: o + bb.read(def: Int32.self, position: Int(o)))
  }

  public func getVector<T>(at off: Int32) -> [T]? {
    let o = offset(off)
    guard o != 0 else { return nil }
    return bb.readSlice(index: vector(at: o), count: vector(count: o))
  }

  /// Vector count gets the count of Elements within the array
  /// - Parameter o: start offset of the vector
  /// - returns: Count of elements
  public func vector(count o: Int32) -> Int32 {
    var o = o
    o += postion
    o += bb.read(def: Int32.self, position: Int(o))
    return bb.read(def: Int32.self, position: Int(o))
  }

  /// Vector start index in the buffer
  /// - Parameter o:start offset of the vector
  /// - returns: the start index of the vector
  public func vector(at o: Int32) -> Int32 {
    var o = o
    o += postion
    return o + bb.read(def: Int32.self, position: Int(o)) + 4
  }

  static public func indirect(_ o: Int32, _ fbb: ByteBuffer) -> Int32 { o + fbb.read(
    def: Int32.self,
    position: Int(o)) }

  static public func offset(_ o: Int32, vOffset: Int32, fbb: ByteBuffer) -> Int32 {
    let vTable = Int32(fbb.capacity) - o
    return vTable + Int32(fbb.read(
      def: Int16.self,
      position: Int(vTable + vOffset - fbb.read(def: Int32.self, position: Int(vTable)))))
  }

  static public func compare(_ off1: Int32, _ off2: Int32, fbb: ByteBuffer) -> Int32 {
    let memorySize = Int32(MemoryLayout<Int32>.size)
    let _off1 = off1 + fbb.read(def: Int32.self, position: Int(off1))
    let _off2 = off2 + fbb.read(def: Int32.self, position: Int(off2))
    let len1 = fbb.read(def: Int32.self, position: Int(_off1))
    let len2 = fbb.read(def: Int32.self, position: Int(_off2))
    let startPos1 = _off1 + memorySize
    let startPos2 = _off2 + memorySize
    let minValue = min(len1, len2)
    for i in 0...minValue {
      let b1 = fbb.read(def: Int8.self, position: Int(i + startPos1))
      let b2 = fbb.read(def: Int8.self, position: Int(i + startPos2))
      if b1 != b2 {
        return Int32(b2 - b1)
      }
    }
    return len1 - len2
  }

  static public func compare(_ off1: Int32, _ key: [Byte], fbb: ByteBuffer) -> Int32 {
    let memorySize = Int32(MemoryLayout<Int32>.size)
    let _off1 = off1 + fbb.read(def: Int32.self, position: Int(off1))
    let len1 = fbb.read(def: Int32.self, position: Int(_off1))
    let len2 = Int32(key.count)
    let startPos1 = _off1 + memorySize
    let minValue = min(len1, len2)
    for i in 0..<minValue {
      let b = fbb.read(def: Int8.self, position: Int(i + startPos1))
      let byte = key[Int(i)]
      if b != byte {
        return Int32(b - Int8(byte))
      }
    }
    return len1 - len2
  }
}
