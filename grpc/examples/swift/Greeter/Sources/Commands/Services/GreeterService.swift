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

import FlatBuffers
import GRPCCore
import Models

struct GreeterService: models_Greeter.SimpleServiceProtocol {
  func Get(
    request: GRPCMessage<models_HelloResponse>,
    context: GRPCCore
      .ServerContext) async throws -> GRPCMessage<models_HelloResponse>
  {
    let model = try request.decode()
    print("## GreeterService.Get: \(model.message)")

    var builder = FlatBufferBuilder(initialSize: 128)
    let off = builder.create(string: "Hello \(model.message ?? "Unknown")")
    let root = models_HelloResponse.createHelloResponse(
      &builder,
      messageOffset: off)
    builder.finish(offset: root)
    return try GRPCMessage<models_HelloResponse>(builder: &builder)
  }

  func Collect(
    request: GRPCCore.RPCAsyncSequence<
      GRPCMessage<models_HelloResponse>,
      any Swift.Error
    >,
    context: GRPCCore
      .ServerContext) async throws -> GRPCMessage<models_HelloResponse>
  {
    let messages: [String] = try await request
      .reduce(into: []) { array, message in
        let model = try message.decode()
        return array.append(model.message ?? "Unknown")
      }

    let joined = messages.joined(separator: ", ")
    print("## GreeterService.Collect: \(joined)")

    var builder = FlatBufferBuilder(initialSize: 128)
    let off = builder.create(string: "Hello \(joined)")
    let root = models_HelloResponse.createHelloResponse(
      &builder,
      messageOffset: off)
    builder.finish(offset: root)
    return try GRPCMessage<models_HelloResponse>(builder: &builder)
  }

  func Expand(
    request: GRPCMessage<models_HelloResponse>,
    response: GRPCCore.RPCWriter<GRPCMessage<models_HelloResponse>>,
    context: GRPCCore.ServerContext) async throws
  {

    let model = try request.decode()
    print("## GreeterService.Expand: \(model.message)")

    let message = model.message ?? "Unknown"
    let stream = AsyncThrowingStream<
      GRPCMessage<models_HelloResponse>,
      Error
    > { continuation in
      var builder = FlatBufferBuilder(initialSize: 128)
      for char in message {
        let off = builder.create(string: "\(char)")
        let root = models_HelloResponse.createHelloResponse(
          &builder,
          messageOffset: off)
        builder.finish(offset: root)

        do {
          continuation
            .yield(try GRPCMessage<models_HelloResponse>(builder: &builder))
        } catch {
          continuation.finish(throwing: error)
        }
      }

      continuation.finish()
    }
    try await response.write(contentsOf: stream)
  }

  func Update(
    request: GRPCCore.RPCAsyncSequence<
      GRPCMessage<models_HelloResponse>,
      any Swift.Error
    >,
    response: GRPCCore.RPCWriter<GRPCMessage<models_HelloResponse>>,
    context: GRPCCore.ServerContext) async throws
  {
    for try await message in request {
      let model = try message.decode()
      print("## GreeterService.Update: \(model.message)")
      var builder = FlatBufferBuilder(initialSize: 128)
      let off = builder.create(string: "Hello \(model.message ?? "Unknown")")
      let root = models_HelloResponse.createHelloResponse(
        &builder,
        messageOffset: off)
      builder.finish(offset: root)
      try await response
        .write(try GRPCMessage<models_HelloResponse>(builder: &builder))
    }
  }
}
