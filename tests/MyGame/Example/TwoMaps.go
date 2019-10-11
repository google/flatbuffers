// Code generated by the FlatBuffers compiler. DO NOT EDIT.

package Example

import (
	flatbuffers "github.com/google/flatbuffers/go"
)

type TwoMaps struct {
	_tab flatbuffers.Table
}

func GetRootAsTwoMaps(buf []byte, offset flatbuffers.UOffsetT) *TwoMaps {
	n := flatbuffers.GetUOffsetT(buf[offset:])
	x := &TwoMaps{}
	x.Init(buf, n+offset)
	return x
}

func (rcv *TwoMaps) Init(buf []byte, i flatbuffers.UOffsetT) {
	rcv._tab.Bytes = buf
	rcv._tab.Pos = i
}

func (rcv *TwoMaps) Table() flatbuffers.Table {
	return rcv._tab
}

func (rcv *TwoMaps) MapFromStringToInt(obj *KeyValStringInt, j int) bool {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(4))
	if o != 0 {
		x := rcv._tab.Vector(o)
		x += flatbuffers.UOffsetT(j) * 4
		x = rcv._tab.Indirect(x)
		obj.Init(rcv._tab.Bytes, x)
		return true
	}
	return false
}

func (rcv *TwoMaps) MapFromStringToIntLength() int {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(4))
	if o != 0 {
		return rcv._tab.VectorLen(o)
	}
	return 0
}

func (rcv *TwoMaps) MapFromStringToBool(obj *KeyValStringBool, j int) bool {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(6))
	if o != 0 {
		x := rcv._tab.Vector(o)
		x += flatbuffers.UOffsetT(j) * 4
		x = rcv._tab.Indirect(x)
		obj.Init(rcv._tab.Bytes, x)
		return true
	}
	return false
}

func (rcv *TwoMaps) MapFromStringToBoolLength() int {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(6))
	if o != 0 {
		return rcv._tab.VectorLen(o)
	}
	return 0
}

func TwoMapsStart(builder *flatbuffers.Builder) {
	builder.StartObject(2)
}
func TwoMapsAddMapFromStringToInt(builder *flatbuffers.Builder, mapFromStringToInt flatbuffers.UOffsetT) {
	builder.PrependUOffsetTSlot(0, flatbuffers.UOffsetT(mapFromStringToInt), 0)
}
func TwoMapsStartMapFromStringToIntVector(builder *flatbuffers.Builder, numElems int) flatbuffers.UOffsetT {
	return builder.StartVector(4, numElems, 4)
}
func TwoMapsAddMapFromStringToBool(builder *flatbuffers.Builder, mapFromStringToBool flatbuffers.UOffsetT) {
	builder.PrependUOffsetTSlot(1, flatbuffers.UOffsetT(mapFromStringToBool), 0)
}
func TwoMapsStartMapFromStringToBoolVector(builder *flatbuffers.Builder, numElems int) flatbuffers.UOffsetT {
	return builder.StartVector(4, numElems, 4)
}
func TwoMapsEnd(builder *flatbuffers.Builder) flatbuffers.UOffsetT {
	return builder.EndObject()
}
