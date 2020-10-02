// Generated GRPC code for FlatBuffers TS *** DO NOT EDIT ***
import { flatbuffers } from 'flatbuffers';
import *  as Greeter_fbs from './greeter_generated';

import * as grpc from 'grpc';

interface IGreeterService extends grpc.ServiceDefinition<grpc.UntypedServiceImplementation> {
  SayHello: IGreeterService_ISayHello;
  SayManyHellos: IGreeterService_ISayManyHellos;
}
interface IGreeterService_ISayHello extends grpc.MethodDefinition<Greeter_fbs.HelloRequest, Greeter_fbs.HelloReply> {
  path: string; // /Greeter/SayHello
  requestStream: boolean; // false
  responseStream: boolean; // false
  requestSerialize: grpc.serialize<Greeter_fbs.HelloRequest>;
  requestDeserialize: grpc.deserialize<Greeter_fbs.HelloRequest>;
  responseSerialize: grpc.serialize<Greeter_fbs.HelloReply>;
  responseDeserialize: grpc.deserialize<Greeter_fbs.HelloReply>;
}

interface IGreeterService_ISayManyHellos extends grpc.MethodDefinition<Greeter_fbs.HelloRequest, Greeter_fbs.HelloReply> {
  path: string; // /Greeter/SayManyHellos
  requestStream: boolean; // false
  responseStream: boolean; // true
  requestSerialize: grpc.serialize<Greeter_fbs.HelloRequest>;
  requestDeserialize: grpc.deserialize<Greeter_fbs.HelloRequest>;
  responseSerialize: grpc.serialize<Greeter_fbs.HelloReply>;
  responseDeserialize: grpc.deserialize<Greeter_fbs.HelloReply>;
}


export const GreeterService: IGreeterService;

export interface IGreeterServer {
  SayHello: grpc.handleUnaryCall<Greeter_fbs.HelloRequest, Greeter_fbs.HelloReply>;
  SayManyHellos: grpc.handleServerStreamingCall<Greeter_fbs.HelloRequest, Greeter_fbs.HelloReply>;
}

export interface IGreeterClient {
  SayHello(request: Greeter_fbs.HelloRequest, callback: (error: grpc.ServiceError | null, response: Greeter_fbs.HelloReply) => void): grpc.ClientUnaryCall;
  SayHello(request: Greeter_fbs.HelloRequest, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: Greeter_fbs.HelloReply) => void): grpc.ClientUnaryCall;
  SayHello(request: Greeter_fbs.HelloRequest, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: Greeter_fbs.HelloReply) => void): grpc.ClientUnaryCall;
  SayManyHellos(request: Greeter_fbs.HelloRequest, metadata: grpc.Metadata): grpc.ClientReadableStream<Greeter_fbs.HelloReply>;
  SayManyHellos(request: Greeter_fbs.HelloRequest, options: Partial<grpc.CallOptions>): grpc.ClientReadableStream<Greeter_fbs.HelloReply>;
}

export class GreeterClient extends grpc.Client implements IGreeterClient {
  constructor(address: string, credentials: grpc.ChannelCredentials, options?: object);  public SayHello(request: Greeter_fbs.HelloRequest, callback: (error: grpc.ServiceError | null, response: Greeter_fbs.HelloReply) => void): grpc.ClientUnaryCall;
  public SayHello(request: Greeter_fbs.HelloRequest, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: Greeter_fbs.HelloReply) => void): grpc.ClientUnaryCall;
  public SayHello(request: Greeter_fbs.HelloRequest, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: Greeter_fbs.HelloReply) => void): grpc.ClientUnaryCall;
  public SayManyHellos(request: Greeter_fbs.HelloRequest, metadata: grpc.Metadata): grpc.ClientReadableStream<Greeter_fbs.HelloReply>;
  public SayManyHellos(request: Greeter_fbs.HelloRequest, options: Partial<grpc.CallOptions>): grpc.ClientReadableStream<Greeter_fbs.HelloReply>;
}

