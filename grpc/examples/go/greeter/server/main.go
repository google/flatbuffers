package main

import (
	"context"
	"fmt"
	"log"
	"net"

	flatbuffers "github.com/google/flatbuffers/go"
	models "github.com/google/flatbuffers/grpc/examples/go/greeter/models"
	"google.golang.org/grpc"
)

var (
	greetings = [...]string{"Hi", "Hallo", "Ciao"}
)

type greeterServer struct {
	models.UnimplementedGreeterServer
}

func (s *greeterServer) SayHello(ctx context.Context, request *models.HelloRequest) (*flatbuffers.Builder, error) {
	v := request.Name()
	var m string
	if v == nil {
		m = "Unknown"
	} else {
		m = string(v)
	}
	b := flatbuffers.NewBuilder(0)
	idx := b.CreateString("welcome " + m)
	models.HelloReplyStart(b)
	models.HelloReplyAddMessage(b, idx)
	b.Finish(models.HelloReplyEnd(b))
	return b, nil
}

func (s *greeterServer) SayManyHellos(request *models.HelloRequest, stream models.Greeter_SayManyHellosServer) error {
	v := request.Name()
	var m string
	if v == nil {
		m = "Unknown"
	} else {
		m = string(v)
	}
	b := flatbuffers.NewBuilder(0)

	for _, greeting := range greetings {
		idx := b.CreateString(greeting + " " + m)
		models.HelloReplyStart(b)
		models.HelloReplyAddMessage(b, idx)
		b.Finish(models.HelloReplyEnd(b))
		if err := stream.Send(b); err != nil {
			return err
		}
	}
	return nil
}

func newServer() *greeterServer {
	s := &greeterServer{}
	return s
}

func main() {
	lis, err := net.Listen("tcp", fmt.Sprintf("localhost:%d", 3000))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	codec := &flatbuffers.FlatbuffersCodec{}
	grpcServer := grpc.NewServer(grpc.ForceServerCodec(codec))
	models.RegisterGreeterServer(grpcServer, newServer())
	if err := grpcServer.Serve(lis); err != nil {
		fmt.Print(err)
		panic(err)
	}
}
