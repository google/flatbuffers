GRPC implementation and test

NOTE: files in `src/` are shared with the GRPC project, and maintained there
(any changes should be submitted to GRPC instead). These files are copied
from GRPC, and work with both the Protobuf and FlatBuffers code generator.

`tests/` contains a GRPC specific test, you need to have built and installed
the GRPC libraries for this to compile. This test will build using the
`FLATBUFFERS_BUILD_GRPCTEST` option to the main FlatBuffers CMake project.

## Building Flatbuffers with gRPC

### Linux

1. Download, build and install gRPC. See [instructions](https://github.com/grpc/grpc/tree/master/src/cpp).
    * Lets say your gRPC clone is at `/your/path/to/grpc_repo`.
    * Install gRPC in a custom directory by running `make install prefix=/your/path/to/grpc_repo/install`.
2. `export GRPC_INSTALL_PATH=/your/path/to/grpc_repo/install`
3. `export PROTOBUF_DOWNLOAD_PATH=/your/path/to/grpc_repo/third_party/protobuf`
4. `mkdir build ; cd build`
5. `cmake -DFLATBUFFERS_BUILD_GRPCTEST=ON -DGRPC_INSTALL_PATH=${GRPC_INSTALL_PATH} -DPROTOBUF_DOWNLOAD_PATH=${PROTOBUF_DOWNLOAD_PATH} ..`
6. `make`

For Bazel users:

```shell
$bazel test src/compiler/...
```

## Running FlatBuffer gRPC tests

### Linux

1. `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${GRPC_INSTALL_PATH}/lib`
2. `make test ARGS=-V`

For Bazel users:

```shell
$bazel test tests/...
```

## C++ Callback API Generation

FlatBuffers gRPC C++ code generation now optionally supports the modern gRPC Callback API.

To enable generation of a `CallbackService` skeleton alongside the existing `Service` and async mixins, invoke `flatc` with both `--grpc` and `--grpc-callback-api`:

```shell
flatc --cpp --grpc --grpc-callback-api your_service.fbs
```

This adds (guarded by `#if defined(GRPC_CALLBACK_API_NONEXPERIMENTAL)`) a class:

```cpp
class YourService::CallbackService : public ::grpc::Service { /* reactor virtuals */ };
```

Each RPC shape maps to the appropriate reactor return type:

- Unary -> `::grpc::ServerUnaryReactor*` Method(...)
- Client streaming -> `::grpc::ServerReadReactor<Request>*`
- Server streaming -> `::grpc::ServerWriteReactor<Response>*`
- Bidi streaming -> `::grpc::ServerBidiReactor<Request, Response>*`

Default generated implementations return `nullptr`; override in your derived class and return a reactor instance you manage (see gRPC docs for lifecycle patterns).

If your gRPC library predates the stable callback API macro, the code inside the guard will be skipped (no breaking changes). Ensure you build against a recent gRPC (1.38+; verify current minimum in grpc repo) to use this feature.

### Client Callback Stubs

When `--grpc-callback-api` is supplied, the generated C++ client stub gains native callback / reactor based async methods in addition to the existing synchronous / generic async flavors, guarded by the same macro. For each RPC named `Foo`:

Unary:

```
void async_Foo(::grpc::ClientContext*, const Request&, Response*, std::function<void(::grpc::Status)>);
void async_Foo(::grpc::ClientContext*, const Request&, Response*, ::grpc::ClientUnaryReactor*);
```

Client streaming:

```
::grpc::ClientWriteReactor<Request>* async_Foo(::grpc::ClientContext*, Response*, ::grpc::ClientWriteReactor<Request>*);
```

Server streaming:

```
::grpc::ClientReadReactor<Response>* async_Foo(::grpc::ClientContext*, const Request&, ::grpc::ClientReadReactor<Response>*);
```

Bidirectional streaming:

```
::grpc::ClientBidiReactor<Request, Response>* async_Foo(::grpc::ClientContext*, ::grpc::ClientBidiReactor<Request, Response>*);
```

These map directly onto the native gRPC callback API factories (e.g. `CallbackUnaryCall`, `ClientCallbackWriterFactory::Create`, etc.) and do not spawn threads. Override the appropriate reactor callbacks per gRPC's documentation to drive I/O.

If your build uses an older gRPC lacking the non-experimental macro, these symbols will not be emitted, preserving backwards compatibility.
