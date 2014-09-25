// automatically generated, do not modify

package Example

import (
	flatbuffers "github.com/google/flatbuffers/go"
)
type Monster struct {
	_tab flatbuffers.Table
}

func GetRootAsMonster(buf []byte, offset flatbuffers.UOffsetT) *Monster {
	n := flatbuffers.GetUOffsetT(buf[offset:])
	x := &Monster{}
	x.Init(buf, n + offset)
	return x
}

func (rcv *Monster) Init(buf []byte, i flatbuffers.UOffsetT) {
	rcv._tab.Bytes = buf
	rcv._tab.Pos = i
}

func (rcv *Monster) Pos(obj *Vec3) *Vec3 {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(4))
	if o != 0 {
		x := o + rcv._tab.Pos
		if obj == nil {
			obj = new(Vec3)
		}
		obj.Init(rcv._tab.Bytes, x)
		return obj
	}
	return nil
}

func (rcv *Monster) Mana() int16 {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(6))
	if o != 0 {
		return rcv._tab.GetInt16(o + rcv._tab.Pos)
	}
	return 150
}

func (rcv *Monster) Hp() int16 {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(8))
	if o != 0 {
		return rcv._tab.GetInt16(o + rcv._tab.Pos)
	}
	return 100
}

func (rcv *Monster) Name() string {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(10))
	if o != 0 {
		return rcv._tab.String(o + rcv._tab.Pos)
	}
	return ""
}

func (rcv *Monster) Inventory(j int) byte {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(14))
	if o != 0 {
		a := rcv._tab.Vector(o)
		return rcv._tab.GetByte(a + flatbuffers.UOffsetT(j * 1))
	}
	return 0
}

func (rcv *Monster) InventoryLength() int {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(14))
	if o != 0 {
		return rcv._tab.VectorLen(o)
	}
	return 0
}

func (rcv *Monster) Color() int8 {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(16))
	if o != 0 {
		return rcv._tab.GetInt8(o + rcv._tab.Pos)
	}
	return 8
}

func (rcv *Monster) TestType() byte {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(18))
	if o != 0 {
		return rcv._tab.GetByte(o + rcv._tab.Pos)
	}
	return 0
}

func (rcv *Monster) Test(obj *flatbuffers.Table) bool {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(20))
	if o != 0 {
		rcv._tab.Union(obj, o)
		return true
	}
	return false
}

func (rcv *Monster) Test4(obj *Test, j int) bool {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(22))
	if o != 0 {
		x := rcv._tab.Vector(o)
		x += flatbuffers.UOffsetT(j) * 4
	if obj == nil {
		obj = new(Test)
	}
		obj.Init(rcv._tab.Bytes, x)
		return true
	}
	return false
}

func (rcv *Monster) Test4Length() int {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(22))
	if o != 0 {
		return rcv._tab.VectorLen(o)
	}
	return 0
}

func (rcv *Monster) Testarrayofstring(j int) string {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(24))
	if o != 0 {
		a := rcv._tab.Vector(o)
		return rcv._tab.String(a + flatbuffers.UOffsetT(j * 4))
	}
	return ""
}

func (rcv *Monster) TestarrayofstringLength() int {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(24))
	if o != 0 {
		return rcv._tab.VectorLen(o)
	}
	return 0
}

/// an example documentation comment: this will end up in the generated code multiline too
func (rcv *Monster) Testarrayoftables(obj *Monster, j int) bool {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(26))
	if o != 0 {
		x := rcv._tab.Vector(o)
		x += flatbuffers.UOffsetT(j) * 4
		x = rcv._tab.Indirect(x)
	if obj == nil {
		obj = new(Monster)
	}
		obj.Init(rcv._tab.Bytes, x)
		return true
	}
	return false
}

func (rcv *Monster) TestarrayoftablesLength() int {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(26))
	if o != 0 {
		return rcv._tab.VectorLen(o)
	}
	return 0
}

func (rcv *Monster) Enemy(obj *Monster) *Monster {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(28))
	if o != 0 {
		x := rcv._tab.Indirect(o + rcv._tab.Pos)
		if obj == nil {
			obj = new(Monster)
		}
		obj.Init(rcv._tab.Bytes, x)
		return obj
	}
	return nil
}

func (rcv *Monster) Testnestedflatbuffer(j int) byte {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(30))
	if o != 0 {
		a := rcv._tab.Vector(o)
		return rcv._tab.GetByte(a + flatbuffers.UOffsetT(j * 1))
	}
	return 0
}

func (rcv *Monster) TestnestedflatbufferLength() int {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(30))
	if o != 0 {
		return rcv._tab.VectorLen(o)
	}
	return 0
}

func (rcv *Monster) Testempty(obj *Stat) *Stat {
	o := flatbuffers.UOffsetT(rcv._tab.Offset(32))
	if o != 0 {
		x := rcv._tab.Indirect(o + rcv._tab.Pos)
		if obj == nil {
			obj = new(Stat)
		}
		obj.Init(rcv._tab.Bytes, x)
		return obj
	}
	return nil
}

func MonsterStart(builder *flatbuffers.Builder) { builder.StartObject(15) }
func MonsterAddPos(builder *flatbuffers.Builder, pos flatbuffers.UOffsetT) { builder.PrependStructSlot(0, flatbuffers.UOffsetT(pos), 0) }
func MonsterAddMana(builder *flatbuffers.Builder, mana int16) { builder.PrependInt16Slot(1, mana, 150) }
func MonsterAddHp(builder *flatbuffers.Builder, hp int16) { builder.PrependInt16Slot(2, hp, 100) }
func MonsterAddName(builder *flatbuffers.Builder, name flatbuffers.UOffsetT) { builder.PrependUOffsetTSlot(3, flatbuffers.UOffsetT(name), 0) }
func MonsterAddInventory(builder *flatbuffers.Builder, inventory flatbuffers.UOffsetT) { builder.PrependUOffsetTSlot(5, flatbuffers.UOffsetT(inventory), 0) }
func MonsterStartInventoryVector(builder *flatbuffers.Builder, numElems int) flatbuffers.UOffsetT { return builder.StartVector(1, numElems, 1)
}
func MonsterAddColor(builder *flatbuffers.Builder, color int8) { builder.PrependInt8Slot(6, color, 8) }
func MonsterAddTestType(builder *flatbuffers.Builder, testType byte) { builder.PrependByteSlot(7, testType, 0) }
func MonsterAddTest(builder *flatbuffers.Builder, test flatbuffers.UOffsetT) { builder.PrependUOffsetTSlot(8, flatbuffers.UOffsetT(test), 0) }
func MonsterAddTest4(builder *flatbuffers.Builder, test4 flatbuffers.UOffsetT) { builder.PrependUOffsetTSlot(9, flatbuffers.UOffsetT(test4), 0) }
func MonsterStartTest4Vector(builder *flatbuffers.Builder, numElems int) flatbuffers.UOffsetT { return builder.StartVector(4, numElems, 2)
}
func MonsterAddTestarrayofstring(builder *flatbuffers.Builder, testarrayofstring flatbuffers.UOffsetT) { builder.PrependUOffsetTSlot(10, flatbuffers.UOffsetT(testarrayofstring), 0) }
func MonsterStartTestarrayofstringVector(builder *flatbuffers.Builder, numElems int) flatbuffers.UOffsetT { return builder.StartVector(4, numElems, 4)
}
func MonsterAddTestarrayoftables(builder *flatbuffers.Builder, testarrayoftables flatbuffers.UOffsetT) { builder.PrependUOffsetTSlot(11, flatbuffers.UOffsetT(testarrayoftables), 0) }
func MonsterStartTestarrayoftablesVector(builder *flatbuffers.Builder, numElems int) flatbuffers.UOffsetT { return builder.StartVector(4, numElems, 4)
}
func MonsterAddEnemy(builder *flatbuffers.Builder, enemy flatbuffers.UOffsetT) { builder.PrependUOffsetTSlot(12, flatbuffers.UOffsetT(enemy), 0) }
func MonsterAddTestnestedflatbuffer(builder *flatbuffers.Builder, testnestedflatbuffer flatbuffers.UOffsetT) { builder.PrependUOffsetTSlot(13, flatbuffers.UOffsetT(testnestedflatbuffer), 0) }
func MonsterStartTestnestedflatbufferVector(builder *flatbuffers.Builder, numElems int) flatbuffers.UOffsetT { return builder.StartVector(1, numElems, 1)
}
func MonsterAddTestempty(builder *flatbuffers.Builder, testempty flatbuffers.UOffsetT) { builder.PrependUOffsetTSlot(14, flatbuffers.UOffsetT(testempty), 0) }
func MonsterEnd(builder *flatbuffers.Builder) flatbuffers.UOffsetT { return builder.EndObject() }
