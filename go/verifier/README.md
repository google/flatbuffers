# about flatbuffers Verify in Golang

read [movation about why a Go verifier in flatbuffers](./movation.md) first

## 0. movation

i trying implementing a instance message service via  tcp client / server.    The Message serialized by flatbuffers, so, read streaming flatbuffers Message is must.

i will implementing verify in Go,  here is some knowdge:

## 1. format of serialized flatbuffers

  assume serialized flatbufers ( Message ) as two part:  Header ( metadata ) / Payload ( data ....)

### 1.1 Message Header

1. 4 byte of root header ( , get offset of payload start

2. zero or more then one byte of prefilled ZERO

3. vtable struct , as below reference from builder.go,  the Payload size ( data size ) include in vtable

```

// A vtable has the following format:

//   <VOffsetT: size of the vtable in bytes, including this value>

//   <VOffsetT: size of the object in bytes, including the vtable offset>

//   <VOffsetT: offset for a field> * N, where N is the number of fields in

//	        the schema for this type. Includes deprecated fields.

// Thus, a vtable is made of 2 + N elements, each SizeVOffsetT bytes wide.

//

// An object has the following format:

//   <SOffsetT: offset to this object's vtable (may be negative)>

//   <byte: data>+

```

### 1.2 Message payload part struct

TBD....


## 2. read streaming serialized flatbuffers

1. read  Message (flatbuf serialized data)  from network, 4 byte by 4 byte

2. read and parse header.  check is a valid flatbufers or not

3. get the data size of message from header,   and get the rest data

4. un-serialize the Message when know the IDL

5. ( option ) Experimental reflaction to guess ths struct of IDL

## 3. usage

golang version 1.13+


define verifier struct in go
```
// Verify parse a valid serialized flatbuffers byte slice and return parse result 
type Verify struct {
	payloadSize    flatbuffers.VOffsetT // payload size, include 4 byte SOffset to vTable
	vTableSize     flatbuffers.VOffsetT // vTable size
	vTableStart    flatbuffers.SOffsetT // vTable offset from start of serialized flatbuffers byte slice
	vTableOffset   flatbuffers.SOffsetT // value in first 4 byte of payload
	PositionToData flatbuffers.UOffsetT // Always < 1<<31.
	slotNumbers    int                  // how many slot ( object field ) in vTable
	slotExited     bool                 // set true when slot numbers is zero
	headVerified   bool                 // verified as avid header of serialized flatbuffers Header
	parseFinished  bool                 // flag for streaming read
}
```

go get 

```
go get github.com/google/flatbuffers/go/verifier
```

import as 

```
import (

	flatbuffers "github.com/google/flatbuffers/go"
	"github.com/google/flatbuffers/go/verifier"
  
  ...

)
	
```



code, use the IDL [ monster.fbs ](./monster.fbs)

```
  // generate by flatc --go --gen-object-api ./monster.fbs
  // MonsterT object initial 
	e := example.MonsterT{
  ....
  }
	
	// serialized MonsterT object to byte slice 
	b := flatbuffers.NewBuilder(0)
	b.Finish(e.Pack(b))
	buf := b.FinishedBytes()

  // initial a flatbuffers  verifier
	v := verifier.NewVerify()

  // parse the serialized flatbuffers header
  //  when parse success , return n as the headerSize +  4 byte Soffset
  
	n, err := v.Parse(buf, 0)
	
	
	// get parsed result
	
	v.PayloadSize() // PayloadSize  size of flatbuffers Message payload ( data ) byte slice, include first 4  byte as SOffset
	
	
```



## 4. example code

streaming read serialized flatbuffers example in [ tcp client / server ](./network-example/main.go)

check the test code in [verify_test.go](./verify_test.go)





tsingson 2020/03/09 