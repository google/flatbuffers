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

import ArgumentParser
import FlatBuffers
import GRPCCore
import GRPCNIOTransportHTTP2
import Models

enum ClientRequest: String, ExpressibleByArgument {
  case get, expand, collect, update
}

struct ClientCommand: AsyncParsableCommand {
  static let configuration = CommandConfiguration(
    commandName: "client")

  @Option(help: "Name to send to the server")
  var name: String

  @Option(help: "request type")
  var request: ClientRequest

  func run() async throws {
    try await withGRPCClient(
      transport: .http2NIOPosix(
        target: .dns(host: "localhost", port: port),
        transportSecurity: .plaintext))
    {
      let client = models_Greeter.Client(wrapping: $0)

      switch request {
      case .get: try await get(client: client)
      case .expand: try await expand(client: client)
      case .collect: try await collect(client: client)
      case .update: try await update(client: client)
      }
    }
  }

  func get(
    client: models_Greeter
      .Client<HTTP2ClientTransport.Posix>) async throws
  {
    for _ in 0..<3 {
      var builder = FlatBufferBuilder(initialSize: 64)
      let off = builder.create(string: name)
      let root = models_HelloRequest.createHelloRequest(
        &builder,
        nameOffset: off)
      builder.finish(offset: root)

      let response = try await client
        .Get(GRPCMessage(builder: &builder))
      let message = try? response.decode().message
      print("get message: \(message ?? "nil")")
    }
  }

  func expand(
    client: models_Greeter
      .Client<HTTP2ClientTransport.Posix>) async throws
  {
    for _ in 0..<3 {
      var builder = FlatBufferBuilder(initialSize: 64)
      let off = builder.create(string: name)
      let root = models_HelloRequest.createHelloRequest(
        &builder,
        nameOffset: off)
      builder.finish(offset: root)
      try await client.Expand(GRPCMessage(builder: &builder)) { response in
        for try await message in response.messages {
          let message = try? message.decode().message
          print("expand message: \(message ?? "nil")")
        }
      }
    }
  }

  func collect(
    client: models_Greeter
      .Client<HTTP2ClientTransport.Posix>) async throws
  {
    for _ in 0..<3 {
      let response = try await client.Collect { writer in
        for part in name {
          print("collect sending: \(part)")
          var builder = FlatBufferBuilder(initialSize: 64)
          let off = builder.create(string: String(part))
          let root = models_HelloRequest.createHelloRequest(
            &builder,
            nameOffset: off)
          builder.finish(offset: root)
          try await writer.write(GRPCMessage(builder: &builder))
        }
      }

      let message = try response.decode().message
      print("collect message: \(message ?? "nil")")
    }
  }

  func update(
    client: models_Greeter
      .Client<HTTP2ClientTransport.Posix>) async throws
  {
    for _ in 0..<3 {
      try await client.Update { writer in
        for part in name {
          print("update sending: \(part)")
          var builder = FlatBufferBuilder(initialSize: 64)
          let off = builder.create(string: String(part))
          let root = models_HelloRequest.createHelloRequest(
            &builder,
            nameOffset: off)
          builder.finish(offset: root)
          try await writer.write(GRPCMessage(builder: &builder))
        }
      } onResponse: { response in
        for try await message in response.messages {
          let message = try message.decode().message
          print("collect message: \(message ?? "nil")")
        }
      }
    }
  }
}
