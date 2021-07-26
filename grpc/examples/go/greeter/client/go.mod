module github.com/google/flatbuffers/grpc/examples/go/greeter/client

go 1.15

replace github.com/google/flatbuffers/grpc/examples/go/greeter/models v0.0.0 => ../models

require (
	github.com/google/flatbuffers v1.12.0
	github.com/google/flatbuffers/grpc/examples/go/greeter/models v0.0.0
	google.golang.org/grpc v1.35.0
)
