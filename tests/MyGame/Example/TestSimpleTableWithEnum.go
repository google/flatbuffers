// automatically generated, do not modify

package Example

import (
	flatbuffers "github.com/google/flatbuffers/go"
)
type TestSimpleTableWithEnum struct {
	_tab flatbuffers.Table
}

func (rcv *TestSimpleTableWithEnum) Init(buf []byte, i flatbuffers.UOffsetT) {
	rcv._tab.Bytes = buf
	rcv._tab.Pos = i
}

func (rcv *TestSimpleTableWithEnum) Color() int8 {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(4))
	if o != 0 {
		return rcv._tab.GetInt8(o + rcv._tab.Pos)
	}
	return 2
}

func TestSimpleTableWithEnumStart(builder *flatbuffers.Builder) { builder.StartObject(1) }
func TestSimpleTableWithEnumAddColor(builder *flatbuffers.Builder, color int8) { builder.PrependInt8Slot(0, color, 2) }
func TestSimpleTableWithEnumEnd(builder *flatbuffers.Builder) flatbuffers.UOffsetT { return builder.EndObject() }

// constants for IsNullField() calls.
const (
    VtTestSimpleTableWithEnumColor = 4
)
func (rcv *TestSimpleTableWithEnum) IsNullField(slot flatbuffers.VOffsetT) bool{
	return rcv._tab.IsNullField(slot)
}

