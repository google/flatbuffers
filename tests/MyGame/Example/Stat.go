// automatically generated, do not modify

package Example

import (
	flatbuffers "github.com/google/flatbuffers/go"
)
type Stat struct {
	_tab flatbuffers.Table
}

func (rcv *Stat) Init(buf []byte, i flatbuffers.UOffsetT) {
	rcv._tab.Bytes = buf
	rcv._tab.Pos = i
}

func (rcv *Stat) Id() string {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(4))
	if o != 0 {
		return rcv._tab.String(o + rcv._tab.Pos)
	}
	return ""
}

func (rcv *Stat) Val() int64 {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(6))
	if o != 0 {
		return rcv._tab.GetInt64(o + rcv._tab.Pos)
	}
	return 0
}

func (rcv *Stat) Count() uint16 {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(8))
	if o != 0 {
		return rcv._tab.GetUint16(o + rcv._tab.Pos)
	}
	return 0
}

func StatStart(builder *flatbuffers.Builder) { builder.StartObject(3) }
func StatAddId(builder *flatbuffers.Builder, id flatbuffers.UOffsetT) { builder.PrependUOffsetTSlot(0, flatbuffers.UOffsetT(id), 0) }
func StatAddVal(builder *flatbuffers.Builder, val int64) { builder.PrependInt64Slot(1, val, 0) }
func StatAddCount(builder *flatbuffers.Builder, count uint16) { builder.PrependUint16Slot(2, count, 0) }
func StatEnd(builder *flatbuffers.Builder) flatbuffers.UOffsetT { return builder.EndObject() }
