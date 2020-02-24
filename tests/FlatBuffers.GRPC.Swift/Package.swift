// swift-tools-version:5.1
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "FlatBuffers.GRPC.Swift",
    platforms: [
           .iOS(.v11),
           .macOS(.v10_14),
    ],
    dependencies: [
        // Main SwiftNIO package
        .package(path: "../../swift"),
        .package(url: "https://github.com/grpc/grpc-swift.git", .branch("nio"))
    ],
    targets: [
        // Targets are the basic building blocks of a package. A target can define a module or a test suite.
        // Targets can depend on other targets in this package, and on products in packages which this package depends on.
        .target(
            name: "Model",
            dependencies: [
                "GRPC",
                "FlatBuffers"
            ],
            path: "Sources/Model"
        ),
        
        // Client for the HelloWorld example
        .target(
            name: "Client",
            dependencies: [
                "GRPC",
                "Model",
            ],
            path: "Sources/client"
        ),
        
        // Server for the HelloWorld example
        .target(
            name: "Server",
            dependencies: [
                "GRPC",
                "Model",
            ],
            path: "Sources/server"
        ),
    ]
)
