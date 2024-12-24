// swift-tools-version:5.9
/*
 * Copyright 2020 Google Inc. All rights reserved.
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

import PackageDescription

let package = Package(
  name: "FlatBuffers.Test.Swift",
  platforms: [
    .iOS(.v11),
    .macOS(.v10_14),
  ],
  dependencies: [
    .package(path: "../../.."),
    .package(url: "https://github.com/grpc/grpc-swift.git", from: "1.4.1"),
    // Prevent the build system from pulling 2.29.1 to prevent Swift 5.8 build breaks.
    // The patch update introduced code that uses "switch expression syntax" that wasn't valid until Swift 5.9 [1].
    // [1] https://github.com/swiftlang/swift-evolution/blob/main/proposals/0380-if-switch-expressions.md
    .package(
      url: "https://github.com/apple/swift-nio-ssl.git",
      exact: "2.29.0"),
  ],
  targets: [
    .executableTarget(
      name: "SwiftFlatBuffers",
      dependencies: [
        .product(name: "FlatBuffers", package: "flatbuffers"),
      ]),
    .testTarget(
      name: "FlatBuffers.Test.SwiftTests",
      dependencies: [
        .product(name: "FlatBuffers", package: "flatbuffers"),
        .product(name: "GRPC", package: "grpc-swift"),
      ]),
    .testTarget(
      name: "FlexBuffers.Test.SwiftTests",
      dependencies: [
        .product(name: "FlexBuffers", package: "flatbuffers"),
      ]),
  ])
