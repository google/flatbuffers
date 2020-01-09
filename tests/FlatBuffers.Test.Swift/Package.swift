// swift-tools-version:5.1
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "FlatBuffers.Test.Swift",
    platforms: [
        .iOS(.v11),
        .macOS(.v10_14),
    ],
    dependencies: [
        .package(path: "../../swift/")
    ],
    targets: [
        .testTarget(
            name: "FlatBuffers.Test.SwiftTests",
            dependencies: ["FlatBuffers"]),
    ]
)
