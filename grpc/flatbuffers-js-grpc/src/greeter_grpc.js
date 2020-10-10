// Generated GRPC code for FlatBuffers TS *** DO NOT EDIT ***
import { flatbuffers } from 'flatbuffers';
import *  as Greeter_fbs from './greeter_generated';

var grpc = require('grpc');

function serialize_HelloReply(buffer_args) {
  if (!(buffer_args instanceof Greeter_fbs.HelloReply)) {
    throw new Error('Expected argument of type Greeter_fbs.HelloReply');
  }
  return buffer_args.serialize();
}

function deserialize_HelloReply(buffer) {
  return Greeter_fbs.HelloReply.getRootAsHelloReply(new flatbuffers.ByteBuffer(buffer))
}


function serialize_HelloRequest(buffer_args) {
  if (!(buffer_args instanceof Greeter_fbs.HelloRequest)) {
    throw new Error('Expected argument of type Greeter_fbs.HelloRequest');
  }
  return buffer_args.serialize();
}

function deserialize_HelloRequest(buffer) {
  return Greeter_fbs.HelloRequest.getRootAsHelloRequest(new flatbuffers.ByteBuffer(buffer))
}


var GreeterService = exports.GreeterService = {
  SayHello: {
    path: '/Greeter/SayHello',
    requestStream: false,
    responseStream: false,
    requestType: flatbuffers.ByteBuffer,
    responseType: Greeter_fbs.HelloReply,
    requestSerialize: serialize_HelloRequest,
    requestDeserialize: deserialize_HelloRequest,
    responseSerialize: serialize_HelloReply,
    responseDeserialize: deserialize_HelloReply,
  },
  SayManyHellos: {
    path: '/Greeter/SayManyHellos',
    requestStream: false,
    responseStream: true,
    requestType: flatbuffers.ByteBuffer,
    responseType: Greeter_fbs.HelloReply,
    requestSerialize: serialize_HelloRequest,
    requestDeserialize: deserialize_HelloRequest,
    responseSerialize: serialize_HelloReply,
    responseDeserialize: deserialize_HelloReply,
  },
};
exports.GreeterClient = grpc.makeGenericClientConstructor(GreeterService);
