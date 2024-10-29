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
import XCTest
@testable import FlatBuffers

class FlatBuffersMoreDefaults: XCTestCase {

  func testFlatbuffersObject() {
    var fbb = FlatBufferBuilder()
    let root = MoreDefaults.createMoreDefaults(&fbb)
    fbb.finish(offset: root)
    let defaults = MoreDefaults.getRootAsMoreDefaults(bb: fbb.sizedBuffer)
    XCTAssertEqual(defaults.emptyString, "")
    XCTAssertEqual(defaults.someString, "some")
    XCTAssertEqual(defaults.ints, [])
    XCTAssertEqual(defaults.floats, [])
    XCTAssertEqual(defaults.bools, [])
    XCTAssertEqual(defaults.intsCount, 0)
    XCTAssertEqual(defaults.floatsCount, 0)
    XCTAssertEqual(defaults.abcsCount, 0)
    XCTAssertEqual(defaults.boolsCount, 0)
  }

  func testFlatbuffersObjectAPI() {
    var fbb = FlatBufferBuilder()
    let defaults = MoreDefaultsT()
    XCTAssertEqual(defaults.emptyString, "")
    XCTAssertEqual(defaults.someString, "some")
    XCTAssertEqual(defaults.ints, [])
    XCTAssertEqual(defaults.floats, [])
    XCTAssertEqual(defaults.abcs, [])
    XCTAssertEqual(defaults.bools, [])

    let buffer = defaults.serialize(builder: &fbb, type: MoreDefaults.self)
    let fDefaults = MoreDefaults.getRootAsMoreDefaults(bb: buffer)
    XCTAssertEqual(fDefaults.emptyString, "")
    XCTAssertEqual(fDefaults.someString, "some")
    XCTAssertEqual(fDefaults.ints, [])
    XCTAssertEqual(fDefaults.floats, [])
    XCTAssertEqual(fDefaults.intsCount, 0)
    XCTAssertEqual(fDefaults.floatsCount, 0)
    XCTAssertEqual(fDefaults.abcsCount, 0)
    XCTAssertEqual(fDefaults.boolsCount, 0)
  }

  func testEncoding() {
    var fbb = FlatBufferBuilder()
    let root = MoreDefaults.createMoreDefaults(&fbb)
    fbb.finish(offset: root)
    var sizedBuffer = fbb.sizedBuffer
    do {
      let reader: MoreDefaults = try getCheckedRoot(byteBuffer: &sizedBuffer)
      let encoder = JSONEncoder()
      encoder.keyEncodingStrategy = .convertToSnakeCase
      let data = try encoder.encode(reader)
      XCTAssertEqual(data, jsonData.data(using: .utf8))
    } catch {
      XCTFail(error.localizedDescription)
    }
  }

  var jsonData: String {
    "{\"empty_string\":\"\",\"some_string\":\"some\"}"
  }
}
