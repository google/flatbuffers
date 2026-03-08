// swift-tools-version:6.2
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
  name: "Greeter",
  platforms: [
    .iOS(.v18),
    .macOS(.v15),
  ],
  dependencies: [
    .package(path: "../../../.."),
    .package(url: "https://github.com/grpc/grpc-swift-2.git", from: "2.0.0"),
    .package(
      url: "https://github.com/grpc/grpc-swift-nio-transport.git",
      from: "2.0.0"),
    .package(
      url: "https://github.com/apple/swift-argument-parser.git",
      from: "1.5.0"),
  ],
  targets: [
    // Targets are the basic building blocks of a package. A target can define a module or a test suite.
    // Targets can depend on other targets in this package, and on products in packages which this package depends on.
    .target(
      name: "Models",
      dependencies: [
        .product(name: "FlatBuffers", package: "flatbuffers"),
        .product(name: "GRPCCore", package: "grpc-swift-2"),
        .product(
          name: "GRPCNIOTransportHTTP2",
          package: "grpc-swift-nio-transport"),
      ]),

    // Client for the Greeter example
    .executableTarget(
      name: "Commands",
      dependencies: [
        .product(name: "GRPCCore", package: "grpc-swift-2"),
        .product(
          name: "GRPCNIOTransportHTTP2",
          package: "grpc-swift-nio-transport"),
        .product(
          name: "ArgumentParser",
          package: "swift-argument-parser"),
        "Models",
      ]),
  ])
