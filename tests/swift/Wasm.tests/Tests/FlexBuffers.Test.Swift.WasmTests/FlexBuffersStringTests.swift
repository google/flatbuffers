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

final class FlexBuffersStringTests: XCTestCase {

  func testEncodingUnicodeString() {
    let text = "プ画をみて✋"

    let bytes = text.data(using: .unicode, allowLossyConversion: true)
    var flx = FlexBuffersWriter()
    flx.map { writer in
      writer.add(blob: bytes!, key: "text", length: bytes!.count)
    }
    flx.finish()
    let byteBuffer = flx.sizedByteBuffer

    let reference = try! getRoot(buffer: byteBuffer)
    let root = reference?.map?["text"]
    let builtString = root?.blob {
      let data = Data(bytes: $0.baseAddress!, count: Int($0.count))
      return String(data: data, encoding: .unicode)
    }

    XCTAssertEqual(builtString, text)
  }
}
