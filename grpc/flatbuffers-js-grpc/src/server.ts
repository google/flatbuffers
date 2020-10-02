import grpc from 'grpc';
import { HelloReply, HelloRequest } from './greeter_generated';
import { IGreeterServer, GreeterService } from './greeter_grpc';

const flatbuffers = require('flatbuffers').flatbuffers;

class GreeterServer implements IGreeterServer {

    SayHello(call: grpc.ServerUnaryCall<HelloRequest>, callback: grpc.sendUnaryData<HelloReply>): void {
        console.log(`${call.request.name()}`);
        let builder = new flatbuffers.Builder();
        let offset = builder.createString(`welcome ${call.request.name()}`);
        let root = HelloReply.createHelloReply(builder, offset);
        builder.finish(root);
        callback(null, HelloReply.getRootAsHelloReply(new flatbuffers.ByteBuffer(builder.asUint8Array())));
    }

    SayManyHellos() {
        
    }
}

function serve(): void {
    const PORT = 3000;
    const server = new grpc.Server();
    server.addService<IGreeterServer>(GreeterService, new GreeterServer());
    console.log(`Listening on ${PORT}`);
    server.bind(`localhost:${PORT}`, grpc.ServerCredentials.createInsecure());
    server.start();
}

serve();
