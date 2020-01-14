// swift-tools-version:5.1
// The swift-tools-version declares the minimum version of Swift required to build this package.

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
    ],
    targets: [
        .target(
            name: "FlatBuffers",
            dependencies: []),
    ]
)
