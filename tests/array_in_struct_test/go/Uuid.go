// Code generated by the FlatBuffers compiler. DO NOT EDIT.

package Uuid

import (
	flatbuffers "github.com/google/flatbuffers/go"
)

type Uuid struct {
	_tab flatbuffers.Struct
}

func (rcv *Uuid) Init(buf []byte, i flatbuffers.UOffsetT) {
	rcv._tab.Bytes = buf
	rcv._tab.Pos = i
}

func (rcv *Uuid) Table() flatbuffers.Table {
	return rcv._tab.Table
}

func (rcv *Uuid) Data(j int) byte {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(0))
	if o != 0 {
		a := rcv._tab.Vector(o)
		return rcv._tab.GetByte(a + flatbuffers.UOffsetT(j*1))
	}
	return 0
}

func CreateUuid(builder *flatbuffers.Builder, data []byte) flatbuffers.UOffsetT {
	builder.Prep(1, 16)
	for i := 16; i >= 0; i-- {
		if len(data) < i+1 {
			builder.PlaceByte(0)
		} else {
			builder.PlaceByte(data[i])
		}
	}
	return builder.Offset()
}
