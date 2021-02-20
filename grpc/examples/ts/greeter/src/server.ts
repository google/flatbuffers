import grpc from 'grpc';
import { HelloReply } from './models/hello-reply';
import { HelloRequest } from './models/hello-request';
import { IGreeterServer, GreeterService } from './greeter_grpc';
import { flatbuffers } from 'flatbuffers';

class GreeterServer implements IGreeterServer {

    SayHello(call: grpc.ServerUnaryCall<HelloRequest>, callback: grpc.sendUnaryData<HelloReply>): void {
        console.log(`SayHello ${call.request.name()}`);
        const builder = new flatbuffers.Builder();
        const offset = builder.createString(`welcome ${call.request.name()}`);
        const root = HelloReply.createHelloReply(builder, offset);
        builder.finish(root);
        callback(null, HelloReply.getRootAsHelloReply(new flatbuffers.ByteBuffer(builder.asUint8Array())));
    }

    async SayManyHellos(call: grpc.ServerWritableStream<HelloRequest>): Promise<void> {
        const name = call.request.name();
        console.log(`${call.request.name()} saying hi in different langagues`);
        ['Hi', 'Hallo', 'Ciao'].forEach(element => {
            const builder = new flatbuffers.Builder();
            const offset = builder.createString(`${element} ${name}`);
            const root = HelloReply.createHelloReply(builder, offset);
            builder.finish(root);
            call.write(HelloReply.getRootAsHelloReply(new flatbuffers.ByteBuffer(builder.asUint8Array())))
        });
        call.end();
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