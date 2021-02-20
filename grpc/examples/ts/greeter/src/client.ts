import grpc from 'grpc';
import { HelloReply } from './models/hello-reply';
import { HelloRequest } from './models/hello-request';
import { GreeterClient } from './greeter_grpc';
import { flatbuffers } from 'flatbuffers';

async function main(PORT: Number, name: String) {
    const _server = new GreeterClient(`localhost:${PORT}`, grpc.credentials.createInsecure());
    const builder = new flatbuffers.Builder();
    const offset = builder.createString(name);
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
}

const args = process.argv.slice(2)
const PORT = Number(args[0]);
const name = String(args[1] ?? "flatbuffers");

if (PORT) {
    main(PORT, name);
} else {
    throw new Error("Requires a valid port number.")
}