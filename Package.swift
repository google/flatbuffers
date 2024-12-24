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
  name: "FlatBuffers",
  platforms: [
    .iOS(.v11),
    .macOS(.v10_14),
  ],
  products: [
    .library(
      name: "FlatBuffers",
      targets: ["FlatBuffers"]),
    .library(
      name: "FlexBuffers",
      targets: ["FlexBuffers"]),
  ],
  targets: [
    .target(
      name: "FlatBuffers",
      dependencies: ["Common"],
      path: "swift/Sources/FlatBuffers"),
    .target(
      name: "FlexBuffers",
      dependencies: ["Common"],
      path: "swift/Sources/FlexBuffers"),
    .target(
      name: "Common",
      dependencies: [],
      path: "swift/Sources/Common"),
  ])
