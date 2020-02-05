// Code generated by the FlatBuffers compiler. DO NOT EDIT.

package NamespaceC

import (
	flatbuffers "github.com/google/flatbuffers/go"

	NamespaceA "fake.flatbuffers.moduleroot/tests/namespace_test/NamespaceA"
)

type TableInCT struct {
	ReferToA1 *NamespaceA.TableInFirstNST
	ReferToA2 *NamespaceA.SecondTableInAT
}

func (t *TableInCT) Pack(builder *flatbuffers.Builder) flatbuffers.UOffsetT {
	if t == nil { return 0 }
	referToA1Offset := t.ReferToA1.Pack(builder)
	referToA2Offset := t.ReferToA2.Pack(builder)
	TableInCStart(builder)
	TableInCAddReferToA1(builder, referToA1Offset)
	TableInCAddReferToA2(builder, referToA2Offset)
	return TableInCEnd(builder)
}

func (rcv *TableInC) UnPackTo(t *TableInCT) {
	t.ReferToA1 = rcv.ReferToA1(nil).UnPack()
	t.ReferToA2 = rcv.ReferToA2(nil).UnPack()
}

func (rcv *TableInC) UnPack() *TableInCT {
	if rcv == nil { return nil }
	t := &TableInCT{}
	rcv.UnPackTo(t)
	return t
}

type TableInC struct {
	_tab flatbuffers.Table
}

func GetRootAsTableInC(buf []byte, offset flatbuffers.UOffsetT) *TableInC {
	n := flatbuffers.GetUOffsetT(buf[offset:])
	x := &TableInC{}
	x.Init(buf, n+offset)
	return x
}

func (rcv *TableInC) Init(buf []byte, i flatbuffers.UOffsetT) {
	rcv._tab.Bytes = buf
	rcv._tab.Pos = i
}

func (rcv *TableInC) Table() flatbuffers.Table {
	return rcv._tab
}

func (rcv *TableInC) ReferToA1(obj *NamespaceA.TableInFirstNS) *NamespaceA.TableInFirstNS {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(4))
	if o != 0 {
		x := rcv._tab.Indirect(o + rcv._tab.Pos)
		if obj == nil {
			obj = new(NamespaceA.TableInFirstNS)
		}
		obj.Init(rcv._tab.Bytes, x)
		return obj
	}
	return nil
}

func (rcv *TableInC) ReferToA2(obj *NamespaceA.SecondTableInA) *NamespaceA.SecondTableInA {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(6))
	if o != 0 {
		x := rcv._tab.Indirect(o + rcv._tab.Pos)
		if obj == nil {
			obj = new(NamespaceA.SecondTableInA)
		}
		obj.Init(rcv._tab.Bytes, x)
		return obj
	}
	return nil
}

func TableInCStart(builder *flatbuffers.Builder) {
	builder.StartObject(2)
}
func TableInCAddReferToA1(builder *flatbuffers.Builder, referToA1 flatbuffers.UOffsetT) {
	builder.PrependUOffsetTSlot(0, flatbuffers.UOffsetT(referToA1), 0)
}
func TableInCAddReferToA2(builder *flatbuffers.Builder, referToA2 flatbuffers.UOffsetT) {
	builder.PrependUOffsetTSlot(1, flatbuffers.UOffsetT(referToA2), 0)
}
func TableInCEnd(builder *flatbuffers.Builder) flatbuffers.UOffsetT {
	return builder.EndObject()
}
