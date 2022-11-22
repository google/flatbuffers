package main

import (
	"bytes"
	"fmt"
	"io/ioutil"
	"net/http"

	"echo/hero"
	"echo/net"

	flatbuffers "github.com/google/flatbuffers/go"
)

func RequestBody() *bytes.Reader {
	b := flatbuffers.NewBuilder(0)
	r := net.RequestT{Player: &hero.WarriorT{Name: "Krull", Hp: 100}}
	b.Finish(r.Pack(b))

	// Encode builder head in last 4 bytes of request body
	buf := make([]byte, 4)
	flatbuffers.WriteUOffsetT(buf, b.Head())
	buf = append(b.Bytes, buf...)

	return bytes.NewReader(buf)
}

func ReadResponse(r *http.Response) {
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		fmt.Printf("Unable to read request body: %v\n", err)
		return
	}

	// Last 4 bytes is offset.
	off := flatbuffers.GetUOffsetT(body[len(body)-4:])
	buf := body[:len(body) - 4] 

	res := net.GetRootAsResponse(buf, off)
	player := res.Player(nil)
	
	fmt.Printf("Got response (name: %v, hp: %v)\n", string(player.Name()), player.Hp())
}

func main() {

	body := RequestBody()	
	req, err := http.NewRequest("POST", "http://localhost:8080/echo", body)
	if err != nil {
		fmt.Println(err)
		return
	}

	client := http.DefaultClient
	resp, err := client.Do(req)
	if err != nil {
		fmt.Println(err)
		return
	}

	ReadResponse(resp)
}
