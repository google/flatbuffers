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

extension UInt64 {
  static let one: UInt64 = 1
}

extension UInt32 {
  static let one: UInt32 = 1
}

public enum BuilderFlag: UInt8 {
  case none = 0
  case shareKeys = 1
  case shareStrings = 2
  case shareKeysAndStrings = 3
  case shareKeyVectors = 4
  case shareAll = 7
}

extension BuilderFlag: Comparable {
  public static func < (lhs: BuilderFlag, rhs: BuilderFlag) -> Bool {
    lhs.rawValue < rhs.rawValue
  }
}

enum StaticJSON {
  static let null = "null"
}

extension Optional {
  var valueOrNull: String {
    if let value = self {
      return "\(value)"
    } else {
      return StaticJSON.null
    }
  }
}

