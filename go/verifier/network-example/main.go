package main

import (
	"bufio"
	"errors"
	"fmt"
	"io"
	"sync"
	"time"

	"github.com/valyala/fasthttp/fasthttputil"

	flatbuffers "github.com/google/flatbuffers/go"
	"github.com/google/flatbuffers/go/verifier"
	"github.com/google/flatbuffers/go/verifier/generated"
)

func main() {
	ln := fasthttputil.NewInmemoryListener()
	// buf := SerializedMonster()
	buf := SerializedEmpty()

	fmt.Println("******************************************************")
	fmt.Println("************  send a serialized empty table  *********")
	fmt.Println("******************************************************")
	wg := sync.WaitGroup{}
	wg.Add(2)
	go func() {
		time.Sleep(1 * time.Second)
		_ = ClientSideSendData(ln, buf)
		wg.Done()
	}()

	go func() {
		_ = serverSideEmpty(ln)
		wg.Done()
	}()

	wg.Wait()

	ln.Close()

	ln = fasthttputil.NewInmemoryListener()

	buf1 := SerializedMonster()
	fmt.Println(" ")
	fmt.Println(" ")
	fmt.Println("******************************************************")
	fmt.Println("************  send a serialized Monster  ************")
	fmt.Println("******************************************************")

	wg.Add(2)
	go func() {
		time.Sleep(1 * time.Second)
		_ = ClientSideSendData(ln, buf1)
		wg.Done()
	}()

	go func() {
		_ = serverSideMonster(ln)
		wg.Done()
	}()

	wg.Wait()
	ln.Close()
}

func SerializedEmpty() []byte {
	e := generated.EmptyT{}
	b := flatbuffers.NewBuilder(0)
	b.Finish(e.Pack(b))
	return b.FinishedBytes()
}

func SerializedMonster() []byte {
	e := generated.MonsterT{
		Pos: &generated.Vec3T{
			X: 1.0,
			Y: 2.0,
			Z: 3.0,
		},
		Mana:      123,
		Hp:        123,
		Name:      "Monster Example",
		Inventory: []byte("123"),
		Color:     generated.ColorBlue,
		Weapons: []*generated.WeaponT{
			&generated.WeaponT{
				Name:   "weapon",
				Damage: 1,
			},
		},
		Equipped: &generated.EquipmentT{
			Type: generated.EquipmentGun,
			Value: &generated.GunT{
				Name:   "weapon",
				Damage: 1.0,
				Time:   365,
			},
		},
		Path: []*generated.Vec3T{
			&generated.Vec3T{
				X: 1.0,
				Y: 2.0,
				Z: 3.0,
			},
			&generated.Vec3T{
				X: 1.0,
				Y: 2.0,
				Z: 3.0,
			},
		},
	}
	b := flatbuffers.NewBuilder(0)
	b.Finish(e.Pack(b))
	return b.FinishedBytes()
}

func serverSideMonster(ln *fasthttputil.InmemoryListener) error {
	conn, _ := ln.Accept()
	defer conn.Close()

	rw := bufio.NewReader(conn)

	// initial a new verifier
	v := verifier.NewVerify()

	buf, er1 := StreamingRead(rw, v)
	if er1 == nil {
		mt := generated.GetRootAsMonster(buf, 0).UnPack()
		fmt.Println("\n\n---------------------------->  un-serialized Monster's name :  ", mt.Name)
	} else {
		fmt.Println("\n\n", "-----------------------------> error : ", er1)
	}

	return nil
}

func serverSideEmpty(ln *fasthttputil.InmemoryListener) error {
	conn, _ := ln.Accept()
	defer conn.Close()

	rw := bufio.NewReader(conn)

	// initial a new verifier
	v := verifier.NewVerify()

	buf, err := StreamingRead(rw, v)
	if err == nil {
		mt := generated.GetRootAsEmpty(buf, 0).UnPack()
		fmt.Println("\n\n---------------------------->  done  ", mt)
	} else {
		fmt.Println("\n\n---------------------------->  Error  ", err)
	}

	return nil
}

func ClientSideSendData(ln *fasthttputil.InmemoryListener, buf []byte) error {
	conn, _ := ln.Dial()

	defer conn.Close()

	loop := len(buf) / 4

	fmt.Print("   >>> send data to tcp server >>>>  \n")

	for i := 0; i < loop; i++ {

		time.Sleep(50 * time.Millisecond)

		_, err := conn.Write(buf[i*4 : (i*4 + 4)])
		if err != nil {
			return err
		}
	}

	return nil
}

func StreamingRead(r *bufio.Reader, v *verifier.Verify) ([]byte, error) {
	fmt.Print("   <<< get data from tcp client <<<<  \n")
	pass := false
	n := 4
	for i := 0; i < 1000; i++ {

		b, err := r.Peek(n)
		if len(b) == 0 {
			// Return ErrTimeout on any timeout.
			if x, ok := err.(interface{ Timeout() bool }); ok && x.Timeout() {
				return nil, errors.New("timeout ")
			}
			if err == io.EOF {
				return nil, err
			}
			if err == nil {
				panic("bufio.Reader.Peek() returned nil, nil")
			}
			if err == bufio.ErrBufferFull {
				return nil, err
			}

			return nil, fmt.Errorf("error when reading flatbuffers : %s", err)
		}

		fmt.Print(".")

		// verify flatbuffers message
		if !v.ParseFinish() || !v.Verify() {
			head, errParse := v.Parse(b, 0)
			if errParse != nil {
			} else { // parse and verified
				if !pass {

					pass = true
					fmt.Println(" ")
					fmt.Println(" --->  flatbuffers vtable been verified \n ---> ",
						"  vtable size:   ", v.VTableSize(),
						"  head size:     ", v.HeaderSize(),
						"  data size      ", v.PayloadSize(),
						"  =========      ", v.Size())
					fmt.Println(" ---> size in bufio.Buffered ", head)
					fmt.Println(" ---> continue read rested flatbuffers data ")
					if v.PayloadSize() == 4 {
						break
					}
				}
			}
		}
		// parse finished and verified success, get the rest message payload
		if v.Verify() && r.Buffered() == v.Size() {
			break
		}

		n = r.Buffered() + 4
	}

	// if !v.Verify() {
	//   return nil, errors.New("not found")
	// }
	get := r.Buffered()
	buf, err := r.Peek(get)
	if err != nil {
		return nil, err
	}
	_, _ = r.Discard(get)
	return buf, nil
}
