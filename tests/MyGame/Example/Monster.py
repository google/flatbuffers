# automatically generated, do not modify

# namespace: Example

import flatbuffers

# /// an example documentation comment: monster object
class Monster(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsMonster(cls, buf, offset):
        x = cls(flatbuffers.Table.GetRoot(buf, offset))
        return x


    # Monster
    def __init__(self, tab):
        self._tab = tab

    # Monster
    def Pos(self):
        o = self._tab.Offset(4)
        if o != 0:
            from .Vec3 import Vec3
            obj = Vec3(flatbuffers.Table(self._tab.Bytes, self._tab.Pos + o))
            return obj
        return None

    # Monster
    def Mana(self):
        o = self._tab.Offset(6)
        if o != 0:
            return self._tab.GetInt16(o)
        return 150

    # Monster
    def Hp(self):
        o = self._tab.Offset(8)
        if o != 0:
            return self._tab.GetInt16(o)
        return 100

    # Monster
    def Name(self):
        o = self._tab.Offset(10)
        if o != 0:
            return self._tab.String(o)
        return b""

    # Monster
    def Inventory(self, j):
        o = self._tab.Offset(14)
        if o != 0:
            x = self._tab.Vector(o) + int(j) * 1
            return self._tab.GetUint8(x)
        return 0

    # Monster
    def InventoryLength(self):
        o = self._tab.Offset(14)
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # Monster
    def Color(self):
        o = self._tab.Offset(16)
        if o != 0:
            return self._tab.GetInt8(o)
        return 8

    # Monster
    def TestType(self):
        o = self._tab.Offset(18)
        if o != 0:
            return self._tab.GetUint8(o)
        return 0

    # Monster
    def Test(self):
        o = self._tab.Offset(20)
        if o != 0:
            return self._tab.Union(o)
        return None

    # Monster
    def Test4(self, j):
        o = self._tab.Offset(22)
        if o != 0:
            x = self._tab.Vector(o) + int(j) * 4
            from .Test import Test
            obj = Test(flatbuffers.Table(self._tab.Bytes, self._tab.Pos + x))
            return obj
        return None

    # Monster
    def Test4Length(self):
        o = self._tab.Offset(22)
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # Monster
    def Testarrayofstring(self, j):
        o = self._tab.Offset(24)
        if o != 0:
            x = self._tab.Vector(o) + int(j) * 4
            return self._tab.String(x)
        return b""

    # Monster
    def TestarrayofstringLength(self):
        o = self._tab.Offset(24)
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

# /// an example documentation comment: this will end up in the generated code
# /// multiline too
    # Monster
    def Testarrayoftables(self, j):
        o = self._tab.Offset(26)
        if o != 0:
            x = self._tab.Vector(o) + int(j) * 4
            x = self._tab.Indirect(x)
            from .Monster import Monster
            obj = Monster(flatbuffers.Table(self._tab.Bytes, self._tab.Pos + x))
            return obj
        return None

    # Monster
    def TestarrayoftablesLength(self):
        o = self._tab.Offset(26)
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # Monster
    def Enemy(self):
        o = self._tab.Offset(28)
        if o != 0:
            o = self._tab.Indirect(o)
            from .Monster import Monster
            obj = Monster(flatbuffers.Table(self._tab.Bytes, self._tab.Pos + o))
            return obj
        return None

    # Monster
    def Testnestedflatbuffer(self, j):
        o = self._tab.Offset(30)
        if o != 0:
            x = self._tab.Vector(o) + int(j) * 1
            return self._tab.GetUint8(x)
        return 0

    # Monster
    def TestnestedflatbufferLength(self):
        o = self._tab.Offset(30)
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # Monster
    def Testempty(self):
        o = self._tab.Offset(32)
        if o != 0:
            o = self._tab.Indirect(o)
            from .Stat import Stat
            obj = Stat(flatbuffers.Table(self._tab.Bytes, self._tab.Pos + o))
            return obj
        return None

    # Monster
    def Testbool(self):
        o = self._tab.Offset(34)
        if o != 0:
            return self._tab.GetBool(o)
        return 0

    # Monster
    def Testhashs32Fnv1(self):
        o = self._tab.Offset(36)
        if o != 0:
            return self._tab.GetInt32(o)
        return 0

    # Monster
    def Testhashu32Fnv1(self):
        o = self._tab.Offset(38)
        if o != 0:
            return self._tab.GetUint32(o)
        return 0

    # Monster
    def Testhashs64Fnv1(self):
        o = self._tab.Offset(40)
        if o != 0:
            return self._tab.GetInt64(o)
        return 0

    # Monster
    def Testhashu64Fnv1(self):
        o = self._tab.Offset(42)
        if o != 0:
            return self._tab.GetUint64(o)
        return 0

    # Monster
    def Testhashs32Fnv1a(self):
        o = self._tab.Offset(44)
        if o != 0:
            return self._tab.GetInt32(o)
        return 0

    # Monster
    def Testhashu32Fnv1a(self):
        o = self._tab.Offset(46)
        if o != 0:
            return self._tab.GetUint32(o)
        return 0

    # Monster
    def Testhashs64Fnv1a(self):
        o = self._tab.Offset(48)
        if o != 0:
            return self._tab.GetInt64(o)
        return 0

    # Monster
    def Testhashu64Fnv1a(self):
        o = self._tab.Offset(50)
        if o != 0:
            return self._tab.GetUint64(o)
        return 0

    # Monster
    def Testarrayofbools(self, j):
        o = self._tab.Offset(52)
        if o != 0:
            x = self._tab.Vector(o) + int(j) * 1
            return self._tab.GetBool(x)
        return 0

    # Monster
    def TestarrayofboolsLength(self):
        o = self._tab.Offset(52)
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

def MonsterStart(builder): builder.StartObject(25)
def MonsterAddPos(builder, pos): builder.PrependStructSlot(0, pos, 0)
def MonsterAddMana(builder, mana): builder.PrependInt16Slot(1, mana, 150)
def MonsterAddHp(builder, hp): builder.PrependInt16Slot(2, hp, 100)
def MonsterAddName(builder, name): builder.PrependUOffsetTRelativeSlot(3, name, 0)
def MonsterAddInventory(builder, inventory): builder.PrependUOffsetTRelativeSlot(5, inventory, 0)
def MonsterStartInventoryVector(builder, numElems): return builder.StartVector(1, numElems, 1)
def MonsterAddColor(builder, color): builder.PrependInt8Slot(6, color, 8)
def MonsterAddTestType(builder, testType): builder.PrependUint8Slot(7, testType, 0)
def MonsterAddTest(builder, test): builder.PrependUOffsetTRelativeSlot(8, test, 0)
def MonsterAddTest4(builder, test4): builder.PrependUOffsetTRelativeSlot(9, test4, 0)
def MonsterStartTest4Vector(builder, numElems): return builder.StartVector(4, numElems, 2)
def MonsterAddTestarrayofstring(builder, testarrayofstring): builder.PrependUOffsetTRelativeSlot(10, testarrayofstring, 0)
def MonsterStartTestarrayofstringVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def MonsterAddTestarrayoftables(builder, testarrayoftables): builder.PrependUOffsetTRelativeSlot(11, testarrayoftables, 0)
def MonsterStartTestarrayoftablesVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def MonsterAddEnemy(builder, enemy): builder.PrependUOffsetTRelativeSlot(12, enemy, 0)
def MonsterAddTestnestedflatbuffer(builder, testnestedflatbuffer): builder.PrependUOffsetTRelativeSlot(13, testnestedflatbuffer, 0)
def MonsterStartTestnestedflatbufferVector(builder, numElems): return builder.StartVector(1, numElems, 1)
def MonsterAddTestempty(builder, testempty): builder.PrependUOffsetTRelativeSlot(14, testempty, 0)
def MonsterAddTestbool(builder, testbool): builder.PrependBoolSlot(15, testbool, 0)
def MonsterAddTesthashs32Fnv1(builder, testhashs32Fnv1): builder.PrependInt32Slot(16, testhashs32Fnv1, 0)
def MonsterAddTesthashu32Fnv1(builder, testhashu32Fnv1): builder.PrependUint32Slot(17, testhashu32Fnv1, 0)
def MonsterAddTesthashs64Fnv1(builder, testhashs64Fnv1): builder.PrependInt64Slot(18, testhashs64Fnv1, 0)
def MonsterAddTesthashu64Fnv1(builder, testhashu64Fnv1): builder.PrependUint64Slot(19, testhashu64Fnv1, 0)
def MonsterAddTesthashs32Fnv1a(builder, testhashs32Fnv1a): builder.PrependInt32Slot(20, testhashs32Fnv1a, 0)
def MonsterAddTesthashu32Fnv1a(builder, testhashu32Fnv1a): builder.PrependUint32Slot(21, testhashu32Fnv1a, 0)
def MonsterAddTesthashs64Fnv1a(builder, testhashs64Fnv1a): builder.PrependInt64Slot(22, testhashs64Fnv1a, 0)
def MonsterAddTesthashu64Fnv1a(builder, testhashu64Fnv1a): builder.PrependUint64Slot(23, testhashu64Fnv1a, 0)
def MonsterAddTestarrayofbools(builder, testarrayofbools): builder.PrependUOffsetTRelativeSlot(24, testarrayofbools, 0)
def MonsterStartTestarrayofboolsVector(builder, numElems): return builder.StartVector(1, numElems, 1)
def MonsterEnd(builder): return builder.EndObject()
