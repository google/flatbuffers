GRPC implementation and test
============================

NOTE: files in `src/` are shared with the GRPC project, and maintained there
(any changes should be submitted to GRPC instead). These files are copied
from GRPC, and work with both the Protobuf and FlatBuffers code generator.

`tests/` contains a GRPC specific test, you need to have built and installed
the GRPC libraries for this to compile. This test will build using the
`FLATBUFFERS_BUILD_GRPCTEST` option to the main FlatBuffers CMake project.

