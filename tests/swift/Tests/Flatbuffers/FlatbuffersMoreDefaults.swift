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
import Testing

@testable import FlatBuffers

struct FlatBuffersMoreDefaults {

  func testFlatbuffersObject() {
    var fbb = FlatBufferBuilder()
    let root = MoreDefaults.createMoreDefaults(&fbb)
    fbb.finish(offset: root)
    var byteBuffer = fbb.sizedBuffer
    let defaults: MoreDefaults = getRoot(byteBuffer: &byteBuffer)
    #expect(defaults.emptyString == "")
    #expect(defaults.someString == "some")
    #expect(defaults.ints.isEmpty == true)
    #expect(defaults.floats.isEmpty == true)
    #expect(defaults.bools.isEmpty == true)
    #expect(defaults.ints.count == 0)
    #expect(defaults.floats.count == 0)
    #expect(defaults.abcs.count == 0)
    #expect(defaults.bools.count == 0)
  }

  func testFlatbuffersObjectAPI() {
    var fbb = FlatBufferBuilder()
    let defaults = MoreDefaultsT()
    #expect(defaults.emptyString == "")
    #expect(defaults.someString == "some")
    #expect(defaults.ints == [])
    #expect(defaults.floats == [])
    #expect(defaults.abcs == [])
    #expect(defaults.bools == [])

    var buffer = defaults.serialize(builder: &fbb, type: MoreDefaults.self)
    let fDefaults: MoreDefaults = getRoot(byteBuffer: &buffer)
    #expect(fDefaults.emptyString == "")
    #expect(fDefaults.someString == "some")
    #expect(fDefaults.ints.isEmpty == true)
    #expect(fDefaults.floats.isEmpty == true)
    #expect(fDefaults.ints.count == 0)
    #expect(fDefaults.floats.count == 0)
    #expect(fDefaults.abcs.count == 0)
    #expect(fDefaults.bools.count == 0)
  }

  @Test
  func testEncoding() throws {
    var fbb = FlatBufferBuilder()
    let root = MoreDefaults.createMoreDefaults(&fbb)
    fbb.finish(offset: root)
    var sizedBuffer = fbb.sizedBuffer
    struct Test: Decodable {
      var emptyString: String
      var someString: String
    }
    let reader: MoreDefaults = try getCheckedRoot(byteBuffer: &sizedBuffer)
    let encoder = JSONEncoder()
    encoder.keyEncodingStrategy = .convertToSnakeCase
    let data = try encoder.encode(reader)
    let decoder = JSONDecoder()
    decoder.keyDecodingStrategy = .convertFromSnakeCase
    let value = try decoder.decode(Test.self, from: data)
    #expect(value.someString == "some")
    #expect(value.emptyString == "")
  }

}
