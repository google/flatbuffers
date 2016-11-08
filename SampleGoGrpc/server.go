package main

import (
	"io"
	"log"
	"net"

	"./SimpleMath"

	"github.com/google/flatbuffers/go"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
)

const (
	port = ":50052"
)

type server struct{}

// No Streaming GRPC method
func (s *server) Mod2(c context.Context, req *SimpleMath.NumReq) (*flatbuffers.Builder, error) {
	b := flatbuffers.NewBuilder(0)
	SimpleMath.NumResStart(b)
	SimpleMath.NumResAddNum(b, req.Num()%2)
	b.Finish(SimpleMath.NumResEnd(b))
	return b, nil
}

// Client Streaming GRPC method
func (s *server) AddNumbers(stream SimpleMath.Math_AddNumbersServer) error {
	var counter int32 = 0
	for {
		req, err := stream.Recv()
		if err == io.EOF {
			b := flatbuffers.NewBuilder(0)
			SimpleMath.NumResStart(b)
			SimpleMath.NumResAddNum(b, counter)
			b.Finish(SimpleMath.NumResEnd(b))
			return stream.SendAndClose(b)
		} else if err != nil {
			return err
		}
		counter += req.Num()
	}
	return nil
}

// Server Streaming GRPC method
func (s *server) Squares(req *SimpleMath.NumReq, c SimpleMath.Math_SquaresServer) error {
	n := int(req.Num())
	for i := 1; i <= n; i++ {
		b := flatbuffers.NewBuilder(0)
		SimpleMath.NumResStart(b)
		SimpleMath.NumResAddNum(b, int32(i*i))
		b.Finish(SimpleMath.NumResEnd(b))
		err := c.Send(b)
		if err != nil {
			return err
		}
	}
	return nil
}

// Bidi streaming GRPC method
func (s *server) IsOdd(c SimpleMath.Math_IsOddServer) error {
	for {
		res, err := c.Recv()
		if err == io.EOF {
			break
		} else if err != nil {
			return err
		}
		out := 0
		if res.Num()%2 == 0 {
			out = 1
		}
		b := flatbuffers.NewBuilder(0)
		SimpleMath.NumResStart(b)
		SimpleMath.NumResAddNum(b, int32(out))
		b.Finish(SimpleMath.NumResEnd(b))
		err = c.Send(b)
		if err != nil {
			return err
		}
	}
	return nil
}

func main() {
	lis, err := net.Listen("tcp", port)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	s := grpc.NewServer(grpc.CustomCodec(SimpleMath.FlatCodec{}))
	SimpleMath.RegisterMathServer(s, &server{})
	if err := s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}
