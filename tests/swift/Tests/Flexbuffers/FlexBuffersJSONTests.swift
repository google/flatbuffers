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

import Common
import FlexBuffers
import XCTest

final class FlexBuffersJSONTests: XCTestCase {
  func testEncodingJSON() throws {
    let buf: ByteBuffer = createProperBuffer().sizedByteBuffer
    let reference = try getRoot(buffer: buf)!

    let json = reference.jsonString()
    // swiftformat:disable all
    XCTAssertEqual(
      json,
      "{\"bar\": [1, 2, 3], \"bar3\": [1, 2, 3], \"bool\": true, \"bools\": [true, false, true, false], \"foo\": 100.0, \"mymap\": {\"foo\": \"Fred\"}, \"vec\": [-100, \"Fred\", 4.0, \"M\", false, 4.0]}"
    )
    // swiftformat:enable all

    let data = json.data(using: .utf8)!
    let decodedData =
      try JSONSerialization.jsonObject(
        with: data,
        options: []) as! [String: Any]

    XCTAssertEqual(decodedData["bar"] as! [Int], [1, 2, 3])
    XCTAssertEqual(decodedData["bar3"] as! [Int], [1, 2, 3])

    let vec: [Any] = decodedData["vec"] as! [Any]
    XCTAssertEqual(vec[0] as! Int, -100)
    XCTAssertEqual(vec[1] as! String, "Fred")
    XCTAssertEqual(vec[2] as! Double, 4.0)
    XCTAssertEqual(vec[3] as! String, "M")
    XCTAssertEqual(vec[4] as! Bool, false)
    XCTAssertEqual(vec[5] as! Double, 4.0)
  }
}
