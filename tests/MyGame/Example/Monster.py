# automatically generated, do not modify

# namespace: Example

import flatbuffers

# /// an example documentation comment: monster object
class Monster(object):
    __slots__ = ['_tab']

    @classmethod
    def get_root_as_Monster(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Monster()
        x.Init(buf, n + offset)
        return x


    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    def pos(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = o + self._tab.Pos
            from .Vec3 import Vec3
            obj = Vec3()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    def mana(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int16Flags, o + self._tab.Pos)
        return 150

    def hp(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int16Flags, o + self._tab.Pos)
        return 100

    def name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return ""

    def inventory(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 1))
        return 0

    def inventory_length(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    def color(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int8Flags, o + self._tab.Pos)
        return 8

    def test_type(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    def test(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(20))
        if o != 0:
            from flatbuffers.table import Table
            obj = Table(bytearray(), 0)
            self._tab.Union(obj, o)
            return obj
        return None

    def test4(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(22))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            from .Test import Test
            obj = Test()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    def test4_length(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(22))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    def testarrayofstring(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(24))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.String(a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return ""

    def testarrayofstring_length(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(24))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

# /// an example documentation comment: this will end up in the generated code
# /// multiline too
    def testarrayoftables(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .Monster import Monster
            obj = Monster()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    def testarrayoftables_length(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    def enemy(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(28))
        if o != 0:
            x = self._tab.Indirect(o + self._tab.Pos)
            from .Monster import Monster
            obj = Monster()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    def testnestedflatbuffer(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(30))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 1))
        return 0

    def testnestedflatbuffer_length(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(30))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    def testempty(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(32))
        if o != 0:
            x = self._tab.Indirect(o + self._tab.Pos)
            from .Stat import Stat
            obj = Stat()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    def testbool(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(34))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos)
        return 0

    def testhashs32_fnv1(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(36))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int32Flags, o + self._tab.Pos)
        return 0

    def testhashu32_fnv1(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(38))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    def testhashs64_fnv1(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(40))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int64Flags, o + self._tab.Pos)
        return 0

    def testhashu64_fnv1(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(42))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint64Flags, o + self._tab.Pos)
        return 0

    def testhashs32_fnv1a(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(44))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int32Flags, o + self._tab.Pos)
        return 0

    def testhashu32_fnv1a(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(46))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    def testhashs64_fnv1a(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(48))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int64Flags, o + self._tab.Pos)
        return 0

    def testhashu64_fnv1a(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(50))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint64Flags, o + self._tab.Pos)
        return 0

    def testarrayofbools(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(52))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.BoolFlags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 1))
        return 0

    def testarrayofbools_length(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(52))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

def Monster_start(builder):
    builder.StartObject(25)
def Monster_add_pos(builder, pos):
    builder.PrependStructSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(pos), 0)
def Monster_add_mana(builder, mana):
    builder.PrependInt16Slot(1, mana, 150)
def Monster_add_hp(builder, hp):
    builder.PrependInt16Slot(2, hp, 100)
def Monster_add_name(builder, name):
    builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def Monster_add_inventory(builder, inventory):
    builder.PrependUOffsetTRelativeSlot(5, flatbuffers.number_types.UOffsetTFlags.py_type(inventory), 0)
def Monster_start_inventory_vector(builder, numElems):
    return builder.StartVector(1, numElems, 1)
def Monster_add_color(builder, color):
    builder.PrependInt8Slot(6, color, 8)
def Monster_add_test_type(builder, test_type):
    builder.PrependUint8Slot(7, test_type, 0)
def Monster_add_test(builder, test):
    builder.PrependUOffsetTRelativeSlot(8, flatbuffers.number_types.UOffsetTFlags.py_type(test), 0)
def Monster_add_test4(builder, test4):
    builder.PrependUOffsetTRelativeSlot(9, flatbuffers.number_types.UOffsetTFlags.py_type(test4), 0)
def Monster_start_test4_vector(builder, numElems):
    return builder.StartVector(4, numElems, 2)
def Monster_add_testarrayofstring(builder, testarrayofstring):
    builder.PrependUOffsetTRelativeSlot(10, flatbuffers.number_types.UOffsetTFlags.py_type(testarrayofstring), 0)
def Monster_start_testarrayofstring_vector(builder, numElems):
    return builder.StartVector(4, numElems, 4)
def Monster_add_testarrayoftables(builder, testarrayoftables):
    builder.PrependUOffsetTRelativeSlot(11, flatbuffers.number_types.UOffsetTFlags.py_type(testarrayoftables), 0)
def Monster_start_testarrayoftables_vector(builder, numElems):
    return builder.StartVector(4, numElems, 4)
def Monster_add_enemy(builder, enemy):
    builder.PrependUOffsetTRelativeSlot(12, flatbuffers.number_types.UOffsetTFlags.py_type(enemy), 0)
def Monster_add_testnestedflatbuffer(builder, testnestedflatbuffer):
    builder.PrependUOffsetTRelativeSlot(13, flatbuffers.number_types.UOffsetTFlags.py_type(testnestedflatbuffer), 0)
def Monster_start_testnestedflatbuffer_vector(builder, numElems):
    return builder.StartVector(1, numElems, 1)
def Monster_add_testempty(builder, testempty):
    builder.PrependUOffsetTRelativeSlot(14, flatbuffers.number_types.UOffsetTFlags.py_type(testempty), 0)
def Monster_add_testbool(builder, testbool):
    builder.PrependBoolSlot(15, testbool, 0)
def Monster_add_testhashs32_fnv1(builder, testhashs32_fnv1):
    builder.PrependInt32Slot(16, testhashs32_fnv1, 0)
def Monster_add_testhashu32_fnv1(builder, testhashu32_fnv1):
    builder.PrependUint32Slot(17, testhashu32_fnv1, 0)
def Monster_add_testhashs64_fnv1(builder, testhashs64_fnv1):
    builder.PrependInt64Slot(18, testhashs64_fnv1, 0)
def Monster_add_testhashu64_fnv1(builder, testhashu64_fnv1):
    builder.PrependUint64Slot(19, testhashu64_fnv1, 0)
def Monster_add_testhashs32_fnv1a(builder, testhashs32_fnv1a):
    builder.PrependInt32Slot(20, testhashs32_fnv1a, 0)
def Monster_add_testhashu32_fnv1a(builder, testhashu32_fnv1a):
    builder.PrependUint32Slot(21, testhashu32_fnv1a, 0)
def Monster_add_testhashs64_fnv1a(builder, testhashs64_fnv1a):
    builder.PrependInt64Slot(22, testhashs64_fnv1a, 0)
def Monster_add_testhashu64_fnv1a(builder, testhashu64_fnv1a):
    builder.PrependUint64Slot(23, testhashu64_fnv1a, 0)
def Monster_add_testarrayofbools(builder, testarrayofbools):
    builder.PrependUOffsetTRelativeSlot(24, flatbuffers.number_types.UOffsetTFlags.py_type(testarrayofbools), 0)
def Monster_start_testarrayofbools_vector(builder, numElems):
    return builder.StartVector(1, numElems, 1)
def Monster_end(builder):
    return builder.EndObject()
