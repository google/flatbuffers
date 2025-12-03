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

import FlexBuffers

// swiftformat:disable all
let flexbufferGolden: [UInt8] = [
  118, 101, 99, 0, 4, 70, 114, 101, 100, 0, 0, 0, 0, 0, 128, 64, 1, 77, 6, 156, 15, 9, 5, 0, 12, 4,
  20, 34, 100, 104, 34, 98, 97, 114, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 98, 97,
  114, 51, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 98, 111, 111, 108, 115, 0, 4, 1, 0, 1, 0,
  98, 111, 111, 108, 0, 102, 111, 111, 0, 109, 121, 109, 97, 112, 0, 1, 11, 1, 1, 1, 98, 20, 7, 75,
  55, 25, 37, 22, 19, 112, 0, 0, 0, 10, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 88, 0, 0, 0, 72, 0, 0, 0,
  1, 0, 0, 0, 61, 0, 0, 0, 0, 0, 200, 66, 45, 0, 0, 0, 133, 0, 0, 0, 46, 78, 106, 144, 14, 36, 40,
  35, 38, 1,
]

let allTypesGolden: [UInt8] = [
  9, 9, 0, 0, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0,
  0, 31, 0, 0, 0, 0, 0, 0, 0, 36, 0, 0, 0, 0, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 247, 255, 255, 255, 255, 255, 255, 255, 9, 0,
  0, 0, 0, 0, 0, 0, 51, 51, 51, 51, 51, 51, 3, 64, 0, 0, 0, 64, 51, 51, 3, 64, 51, 51, 51, 51, 51,
  51, 3, 192, 0, 0, 0, 64, 51, 51, 3, 192, 24, 28, 34, 34, 3, 107, 7, 7, 11, 15, 15, 15, 15, 117,
  43, 1,
]
// swiftformat:enable all

@inline(__always)
func createSizedBuffer() -> ByteBuffer {
  createProperBuffer().sizedByteBuffer
}

@inline(__always)
func createProperBuffer() -> FlexBuffersWriter {
  var fbx = FlexBuffersWriter(
    initialSize: 8,
    flags: .shareKeysAndStrings)
  write(fbx: &fbx)
  return fbx
}

func write(fbx: inout FlexBuffersWriter) {
  fbx.map { map in
    map.vector(key: "vec") { v in
      v.add(int64: -100)
      v.add(string: "Fred")
      v.indirect(float32: 4.0)
      let lv = v.lastValue()
      let blob: [UInt8] = [77]
      v.add(blob: blob, length: blob.count)
      v.add(bool: false)
      v.reuse(value: lv!)
    }
    let ints: [Int32] = [1, 2, 3]
    map.create(vector: ints, key: "bar")
    map.createFixed(vector: ints, key: "bar3")
    let bools = [true, false, true, false]
    map.create(vector: bools, key: "bools")
    map.add(bool: true, key: "bool")
    map.add(double: 100, key: "foo")
    map.map(key: "mymap") { m in
      m.add(string: "Fred", key: "foo")
    }
  }

  fbx.finish()
}
