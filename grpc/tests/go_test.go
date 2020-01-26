// Execute this test like:
// `CGO_ENABLED=0 go test go_test.go`

package testing

import (
	"fake.flatbuffers.moduleroot/tests/MyGame/Example"

	"context"
	"fmt"
	"net"
	"testing"

	flatbuffers "github.com/google/flatbuffers/go"
	"google.golang.org/grpc"
)

type server struct{}

// test used to send and receive in grpc methods
var test = "Flatbuffers"
var addr = "0.0.0.0:50051"

// gRPC server store method
func (s *server) Store(context context.Context, in *Example.Monster) (*flatbuffers.Builder, error) {
	b := flatbuffers.NewBuilder(0)
	i := b.CreateString(test)
	Example.StatStart(b)
	Example.StatAddId(b, i)
	b.Finish(Example.StatEnd(b))
	return b, nil

}

// gRPC server retrieve method
func (s *server) Retrieve(in *Example.Stat, m Example.MonsterStorage_RetrieveServer) error {
	b := flatbuffers.NewBuilder(0)
	i := b.CreateString(test)
	Example.MonsterStart(b)
	Example.MonsterAddName(b, i)
	b.Finish(Example.MonsterEnd(b))
	return m.Send(b)
}

func (s *server) GetMaxHitPoint(_ Example.MonsterStorage_GetMaxHitPointServer) error {
	// TODO: impement this properly for testing. Current barebones implimentation
	// is just to minimally satisfy Example.MonsterStorageServer interface
	panic(fmt.Errorf("GetMaxHitPoint Procedure Unimplimented"))
}

func (s *server) GetMinMaxHitPoints(_ Example.MonsterStorage_GetMinMaxHitPointsServer) error {
	// TODO: impement this properly for testing. Current barebones implimentation
	// is just to minimally satisfy Example.MonsterStorageServer interface
	panic(fmt.Errorf("GetMinMaxHitPoints Procedure Unimplimented"))
}

func StoreClient(c Example.MonsterStorageClient, t *testing.T) {
	b := flatbuffers.NewBuilder(0)
	i := b.CreateString(test)
	Example.MonsterStart(b)
	Example.MonsterAddName(b, i)
	b.Finish(Example.MonsterEnd(b))
	out, err := c.Store(context.Background(), b)
	if err != nil {
		t.Fatalf("Store client failed: %v", err)
	}
	if string(out.Id()) != test {
		t.Errorf("StoreClient failed: expected=%s, got=%s\n", test, out.Id())
		t.Fail()
	}
}

func RetrieveClient(c Example.MonsterStorageClient, t *testing.T) {
	b := flatbuffers.NewBuilder(0)
	i := b.CreateString(test)
	Example.StatStart(b)
	Example.StatAddId(b, i)
	b.Finish(Example.StatEnd(b))
	x, err := c.Retrieve(context.Background(), b)
	if err != nil {
		t.Fatalf("Retrieve client failed: %v", err)
	}
	out, err := x.Recv()
	if err != nil {
		t.Fatalf("MonsterStorage_RetrieveClient recv failed: %v", err)
	}
	if string(out.Name()) != test {
		t.Errorf("RetrieveClient failed: expected=%s, got=%s\n", test, out.Name())
		t.Fail()
	}
}

func TestGRPC(t *testing.T) {
	lis, err := net.Listen("tcp", addr)
	if err != nil {
		t.Fatalf("Failed to listen: %v", err)
	}
	ser := grpc.NewServer(grpc.CustomCodec(flatbuffers.FlatbuffersCodec{}))
	Example.RegisterMonsterStorageServer(ser, &server{})
	go func() {
		if err := ser.Serve(lis); err != nil {
			t.Fatalf("Failed to serve: %v", err)
			t.FailNow()
		}
	}()
	conn, err := grpc.Dial(addr, grpc.WithInsecure(), grpc.WithCodec(flatbuffers.FlatbuffersCodec{}))
	if err != nil {
		t.Fatalf("Failed to connect: %v", err)
	}
	defer conn.Close()
	client := Example.NewMonsterStorageClient(conn)
	StoreClient(client, t)
	RetrieveClient(client, t)
}
