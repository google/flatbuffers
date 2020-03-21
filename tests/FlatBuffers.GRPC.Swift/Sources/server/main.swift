/*
 * Copyright 2020, gRPC Authors All rights reserved.
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

import GRPC
import NIO
import FlatBuffers
import Logging
import Model

class Greeter: GreeterProvider {
    
    var hellos: [Message<HelloReply>] = []
    
    init() {
        let names = ["Stranger1", "Stranger2", "Stranger4", "Stranger3", "Stranger5", "Stranger6"]
        for name in names {
            var builder = FlatBufferBuilder()
            let off = builder.create(string: name)
            let root = HelloReply.createHelloReply(builder, offsetOfMessage: off)
            builder.finish(offset: root)
            hellos.append(Message(builder: &builder))
        }
    }
    
    func SayHello(
        _ request: Message<HelloRequest>,
        context: StatusOnlyCallContext
    ) -> EventLoopFuture<Message<HelloReply>> {
        let recipient = request.object.name ?? "Stranger"
        
        var builder = FlatBufferBuilder()
        let off = builder.create(string: recipient)
        let root = HelloReply.createHelloReply(builder, offsetOfMessage: off)
        builder.finish(offset: root)
        return context.eventLoop.makeSucceededFuture(Message<HelloReply>(builder: &builder))
    }
    
    func SayManyHellos(
        request: Message<ManyHellosRequest>,
        context: StreamingResponseCallContext<Message<HelloReply>>
    ) -> EventLoopFuture<GRPCStatus> {
        for _ in 0..<Int(request.object.numGreetings) {
            let index = Int.random(in: 0..<hellos.count)
            _ = context.sendResponse(hellos[index])
        }
        return context.eventLoop.makeSucceededFuture(.ok)
    }
}

// Quieten the logs.
LoggingSystem.bootstrap {
  var handler = StreamLogHandler.standardOutput(label: $0)
  handler.logLevel = .critical
  return handler
}

let group = MultiThreadedEventLoopGroup(numberOfThreads: 1)
defer {
  try! group.syncShutdownGracefully()
}

// Create some configuration for the server:
let configuration = Server.Configuration(
  target: .hostAndPort("localhost", 0),
  eventLoopGroup: group,
  serviceProviders: [Greeter()]
)

// Start the server and print its address once it has started.
let server = Server.start(configuration: configuration)
server.map {
  $0.channel.localAddress
}.whenSuccess { address in
  print("server started on port \(address!.port!)")
}

// Wait on the server's `onClose` future to stop the program from exiting.
_ = try server.flatMap {
  $0.onClose
}.wait()
