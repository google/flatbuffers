## Languages known issues

### Go

- Always requires the `content-type` of the payload to be set to `application/grpc+flatbuffers`

example: `.SayHello(ctx, b, grpc.CallContentSubtype("flatbuffers"))`