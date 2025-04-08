// Generated GRPC code for FlatBuffers swift!
/// The following code is generated by the Flatbuffers library which might not be in sync with grpc-swift
/// in case of an issue please open github issue, though it would be maintained

// swiftlint:disable all
// swiftformat:disable all

import Foundation
import GRPC
import NIO
import NIOHTTP1
import FlatBuffers

public protocol GRPCFlatBufPayload: GRPCPayload, FlatBufferGRPCMessage {}
public extension GRPCFlatBufPayload {
  init(serializedByteBuffer: inout NIO.ByteBuffer) throws {
    self.init(byteBuffer: FlatBuffers.ByteBuffer(contiguousBytes: serializedByteBuffer.readableBytesView, count: serializedByteBuffer.readableBytes))
  }
  func serialize(into buffer: inout NIO.ByteBuffer) throws {
    withUnsafeReadableBytes { buffer.writeBytes($0) }
  }
}
extension Message: GRPCFlatBufPayload {}

/// Usage: instantiate models_GreeterServiceClient, then call methods of this protocol to make API calls.
public protocol models_GreeterClientProtocol: GRPCClient {

  var serviceName: String { get }

  var interceptors: models_GreeterClientInterceptorFactoryProtocol? { get }

  func SayHello(
    _ request: Message<models_HelloRequest>
    , callOptions: CallOptions?
  ) -> UnaryCall<Message<models_HelloRequest>, Message<models_HelloReply>>

  func SayManyHellos(
    _ request: Message<models_HelloRequest>
    , callOptions: CallOptions?,
    handler: @escaping (Message<models_HelloReply>) -> Void
  ) -> ServerStreamingCall<Message<models_HelloRequest>, Message<models_HelloReply>>

}

extension models_GreeterClientProtocol {

  public var serviceName: String { "models.Greeter" }

  public func SayHello(
    _ request: Message<models_HelloRequest>
    , callOptions: CallOptions? = nil
  ) -> UnaryCall<Message<models_HelloRequest>, Message<models_HelloReply>> {
    return self.makeUnaryCall(
      path: "/models.Greeter/SayHello",
      request: request,
      callOptions: callOptions ?? self.defaultCallOptions,
      interceptors: self.interceptors?.makeSayHelloInterceptors() ?? []
    )
  }

  public func SayManyHellos(
    _ request: Message<models_HelloRequest>
    , callOptions: CallOptions? = nil,
    handler: @escaping (Message<models_HelloReply>) -> Void
  ) -> ServerStreamingCall<Message<models_HelloRequest>, Message<models_HelloReply>> {
    return self.makeServerStreamingCall(
      path: "/models.Greeter/SayManyHellos",
      request: request,
      callOptions: callOptions ?? self.defaultCallOptions,
      interceptors: self.interceptors?.makeSayManyHellosInterceptors() ?? [],
      handler: handler
    )
  }
}

public protocol models_GreeterClientInterceptorFactoryProtocol {
  /// - Returns: Interceptors to use when invoking 'SayHello'.
  func makeSayHelloInterceptors() -> [ClientInterceptor<Message<models_HelloRequest>, Message<models_HelloReply>>]

  /// - Returns: Interceptors to use when invoking 'SayManyHellos'.
  func makeSayManyHellosInterceptors() -> [ClientInterceptor<Message<models_HelloRequest>, Message<models_HelloReply>>]

}

public final class models_GreeterServiceClient: models_GreeterClientProtocol {
  public let channel: GRPCChannel
  public var defaultCallOptions: CallOptions
  public var interceptors: models_GreeterClientInterceptorFactoryProtocol?

  public init(
    channel: GRPCChannel,
    defaultCallOptions: CallOptions = CallOptions(),
    interceptors: models_GreeterClientInterceptorFactoryProtocol? = nil
  ) {
    self.channel = channel
    self.defaultCallOptions = defaultCallOptions
    self.interceptors = interceptors
  }
}

public protocol models_GreeterProvider: CallHandlerProvider {
  var interceptors: models_GreeterServerInterceptorFactoryProtocol? { get }
  func SayHello(request: Message<models_HelloRequest>, context: StatusOnlyCallContext) -> EventLoopFuture<Message<models_HelloReply>>
  func SayManyHellos(request: Message<models_HelloRequest>, context: StreamingResponseCallContext<Message<models_HelloReply>>) -> EventLoopFuture<GRPCStatus>
}

public extension models_GreeterProvider {

  var serviceName: Substring { return "models.Greeter" }

  func handle(method name: Substring, context: CallHandlerContext) -> GRPCServerHandlerProtocol? {
    switch name {
    case "SayHello":
    return UnaryServerHandler(
      context: context,
      requestDeserializer: GRPCPayloadDeserializer<Message<models_HelloRequest>>(),
      responseSerializer: GRPCPayloadSerializer<Message<models_HelloReply>>(),
      interceptors: self.interceptors?.makeSayHelloInterceptors() ?? [],
      userFunction: self.SayHello(request:context:))

    case "SayManyHellos":
    return ServerStreamingServerHandler(
      context: context,
      requestDeserializer: GRPCPayloadDeserializer<Message<models_HelloRequest>>(),
      responseSerializer: GRPCPayloadSerializer<Message<models_HelloReply>>(),
      interceptors: self.interceptors?.makeSayManyHellosInterceptors() ?? [],
      userFunction: self.SayManyHellos(request:context:))

    default: return nil;
    }
  }

}

public protocol models_GreeterServerInterceptorFactoryProtocol {
  /// - Returns: Interceptors to use when handling 'SayHello'.
  ///   Defaults to calling `self.makeInterceptors()`.
  func makeSayHelloInterceptors() -> [ServerInterceptor<Message<models_HelloRequest>, Message<models_HelloReply>>]

  /// - Returns: Interceptors to use when handling 'SayManyHellos'.
  ///   Defaults to calling `self.makeInterceptors()`.
  func makeSayManyHellosInterceptors() -> [ServerInterceptor<Message<models_HelloRequest>, Message<models_HelloReply>>]

}
