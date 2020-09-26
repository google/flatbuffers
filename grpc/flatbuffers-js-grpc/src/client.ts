import grpc from 'grpc';
import { HelloRequest } from './greeter_generated';
import services from './greeter_grpc';

const flatbuffers = require('flatbuffers').flatbuffers;

function main() {
    console.log(services);
    let _server = new services.GreeterClient(`localhost:${3000}`, grpc.credentials.createInsecure());
    let builder = new flatbuffers.Builder();
    let offset = builder.createString(`mustii`);
    let root = HelloRequest.createHelloRequest(builder, offset);
    builder.finish(root);
    _server.SayHello(HelloRequest.getRootAsHelloRequest(new flatbuffers.ByteBuffer(builder.asUint8Array())), (err, response) => {
        console.log(err);
        console.log(response);
    });
}

main();