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
public struct Struct {
  public private(set) var bb: ByteBuffer
  public private(set) var postion: Int32

  public init(bb: ByteBuffer, position: Int32 = 0) {
    self.bb = bb
    postion = position
  }

  public func readBuffer<T: Scalar>(of type: T.Type, at o: Int32) -> T {
    let r = bb.read(def: T.self, position: Int(o + postion))
    return r
  }
}
