package main

import (
	"io"
	"log"

	"./SimpleMath"

	"github.com/google/flatbuffers/go"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
)

const (
	address     = "localhost:50052"
	defaultName = "world"
)

// No Streaming GRPC method
func Mod2(c SimpleMath.MathClient) {
	input := 10
	b := flatbuffers.NewBuilder(0)
	SimpleMath.NumReqStart(b)
	SimpleMath.NumReqAddNum(b, int32(input))
	b.Finish(SimpleMath.NumReqEnd(b))
	res, err := c.Mod2(context.Background(), b)
	if err != nil {
		log.Fatal(err)
	}
	log.Printf("Mod2(%d):%d (No streaming)\n", input, res.Num())
}

// Client Streaming GRPC method
func AddNumbers(c SimpleMath.MathClient) {
	input := 10
	stream, err := c.AddNumbers(context.Background())
	if err != nil {
		log.Fatal(err)
	}
	for i := 1; i < input; i++ {
		b := flatbuffers.NewBuilder(0)
		SimpleMath.NumReqStart(b)
		SimpleMath.NumReqAddNum(b, int32(i))
		b.Finish(SimpleMath.NumReqEnd(b))
		err := stream.Send(b)
		if err != nil {
			log.Fatal(err)
		}
	}
	res, err := stream.CloseAndRecv()
	if err != nil {
		log.Fatal(err)
	}
	log.Printf("AddNumbers(%d):%d (Client streaming)\n", input, res.Num())
}

// Server Streaming GPRC method
func Squares(c SimpleMath.MathClient) {
	input := 10
	b := flatbuffers.NewBuilder(0)
	SimpleMath.NumReqStart(b)
	SimpleMath.NumReqAddNum(b, int32(input))
	b.Finish(SimpleMath.NumReqEnd(b))
	stream, err := c.Squares(context.Background(), b)
	if err != nil {
		log.Fatal(err)
	}
	var squares []int32
	for {
		res, err := stream.Recv()
		if err == io.EOF {
			break
		} else if err != nil {
			log.Fatal(err)
		}
		squares = append(squares, res.Num())
	}
	log.Printf("Squares(%d): %v (Server streaming)\n", input, squares)
}

// Bidi streaming GRPC method
func IsOdd(c SimpleMath.MathClient) {
	stream, err := c.IsOdd(context.Background())
	if err != nil {
		log.Fatal(err)
	}
	num := 10
	for i := 1; i < num; i++ {
		b := flatbuffers.NewBuilder(0)
		SimpleMath.NumReqStart(b)
		SimpleMath.NumReqAddNum(b, int32(i))
		b.Finish(SimpleMath.NumReqEnd(b))
		err := stream.Send(b)
		if err != nil {
			log.Fatal(err)
		}
		res, err := stream.Recv()
		if err != nil {
			log.Fatal(err)
		}
		log.Printf("IsOdd(%d):%d (Bidi streaming)\n", i, res.Num())
	}
}

func main() {
	conn, err := grpc.Dial(address, grpc.WithInsecure(), grpc.WithCodec(SimpleMath.FlatCodec{}))
	if err != nil {
		log.Fatalf("did not connect: %v", err)
	}
	defer conn.Close()
	c := SimpleMath.NewMathClient(conn)
	AddNumbers(c)
	Mod2(c)
	Squares(c)
	IsOdd(c)
}
