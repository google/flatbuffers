// automatically generated, do not modify

package NamespaceB

import (
	flatbuffers "github.com/google/flatbuffers/go"
)
type TableInNestedNS struct {
	_tab flatbuffers.Table
}

func (rcv *TableInNestedNS) Init(buf []byte, i flatbuffers.UOffsetT) {
	rcv._tab.Bytes = buf
	rcv._tab.Pos = i
}

func (rcv *TableInNestedNS) Foo() int32 {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(4))
	if o != 0 {
		return rcv._tab.GetInt32(o + rcv._tab.Pos)
	}
	return 0
}

func TableInNestedNSStart(builder *flatbuffers.Builder) { builder.StartObject(1) }
func TableInNestedNSAddFoo(builder *flatbuffers.Builder, foo int32) { builder.PrependInt32Slot(0, foo, 0) }
func TableInNestedNSEnd(builder *flatbuffers.Builder) flatbuffers.UOffsetT { return builder.EndObject() }
