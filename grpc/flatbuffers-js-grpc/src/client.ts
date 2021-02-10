import grpc from 'grpc';
import { HelloReply } from './models/hello-reply';
import { HelloRequest } from './models/hello-request';
import { GreeterClient } from './greeter_grpc';
import { flatbuffers } from 'flatbuffers';

async function main(PORT: Number) {
    const _server = new GreeterClient(`localhost:${PORT}`, grpc.credentials.createInsecure());
    const builder = new flatbuffers.Builder();
    const offset = builder.createString('mustii');
    const root = HelloRequest.createHelloRequest(builder, offset);
    builder.finish(root);
    const buffer = HelloRequest.getRootAsHelloRequest(new flatbuffers.ByteBuffer(builder.asUint8Array()));

    _server.SayHello(buffer, (err, response) => {
        console.log(response.message());
    });

    const data = _server.SayManyHellos(buffer, null);

    data.on('data', (data) => {
        console.log(data.message());
    });
    data.on('end', (data) => {
        console.log('end');
    });
}

var PORT = Number(process.argv.slice(2));
if (PORT) {
    main(PORT);
} else {
    throw new Error("Requires a valid port number.")
}