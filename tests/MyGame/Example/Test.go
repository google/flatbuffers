// automatically generated, do not modify

package Example

import (
	flatbuffers "github.com/google/flatbuffers/go"
)
type Test struct {
	_tab flatbuffers.Struct
}

func (rcv *Test) Init(buf []byte, i flatbuffers.UOffsetT) {
	rcv._tab.Bytes = buf
	rcv._tab.Pos = i
}

func (rcv *Test) A() int16 { return rcv._tab.GetInt16(rcv._tab.Pos + flatbuffers.UOffsetT(0)) }
func (rcv *Test) B() int8 { return rcv._tab.GetInt8(rcv._tab.Pos + flatbuffers.UOffsetT(2)) }

func CreateTest(builder *flatbuffers.Builder, a int16, b int8) flatbuffers.UOffsetT {
    builder.Prep(2, 4)
    builder.Pad(1)
    builder.PrependInt8(b)
    builder.PrependInt16(a)
    return builder.Offset()
}
