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

@usableFromInline
enum BitWidth: UInt64, CaseIterable {
  case w8 = 0, w16 = 1, w32 = 2, w64 = 3
}

extension BitWidth: Comparable {
  @usableFromInline
  static func < (lhs: BitWidth, rhs: BitWidth) -> Bool {
    lhs.rawValue < rhs.rawValue
  }

  @inline(__always)
  static func widthB(_ v: Int) -> BitWidth {
    switch v {
    case 1: return .w8
    case 2: return .w16
    case 4: return .w32
    case 8: return .w64
    default:
      assert(false, "We shouldn't reach here")
      return .w64
    }
  }

  @inline(__always)
  static func max(_ lhs: BitWidth, rhs: BitWidth) -> BitWidth {
    if lhs.rawValue > rhs.rawValue { return lhs }
    return rhs
  }
}
