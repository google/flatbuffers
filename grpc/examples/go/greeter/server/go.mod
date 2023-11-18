module github.com/google/flatbuffers/grpc/examples/go/greeter/server

go 1.15

replace github.com/google/flatbuffers/grpc/examples/go/greeter/models v0.0.0 => ../models

require (
	github.com/google/flatbuffers v2.0.8+incompatible
	github.com/google/flatbuffers/grpc/examples/go/greeter/models v0.0.0
	google.golang.org/grpc v1.56.3
)
