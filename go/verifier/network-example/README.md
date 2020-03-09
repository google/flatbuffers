# example for streaming read flatbuffers serialized byte slice

Thanks a lot to [fasthttp](https://github.com/valyala/fasthttp) for [InmemoryListener](https://github.com/valyala/fasthttp/blob/master/fasthttputil/inmemory_listener.go)

it's best tools for TCP testing.

```
// InmemoryListener provides in-memory dialer<->net.Listener implementation.
//
// It may be used either for fast in-process client<->server communications
// without network stack overhead or for client<->server tests.

```



run it.

```
cd .../go/verifier/network-example

go run ./main.go
```

get result in console:

```
******************************************************
************  send a serialized empty table  *********
******************************************************
   <<< get data from tcp client <<<<  
   >>> send data to tcp server >>>>  
... 
 --->  flatbuffers vtable been verified 
 --->    vtable size:    4   head size:      8   data size       4   =========       12
 ---> size in bufio.Buffered  12
 ---> continue read rested flatbuffers data 


---------------------------->  done   &{}
 
 
******************************************************
************  send a serialized Monster  ************
******************************************************
   >>> send data to tcp server >>>>  
   <<< get data from tcp client <<<<  
......... 
 --->  flatbuffers vtable been verified 
 --->    vtable size:    26   head size:      32   data size       184   =========       216
 ---> size in bufio.Buffered  36
 ---> continue read rested flatbuffers data 
.............................................

---------------------------->  un-serialized Monster's name :   Monster Example
```



for streaming read serialized flatbuffers byte slice , check out this

```

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

```



happy hacking.
