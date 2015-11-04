# automatically generated, do not modify

# namespace: Example

import flatbuffers

# /// an example documentation comment: monster object
class Monster(object):
    __slots__ = ['_tab']

    @classmethod
    def get_root_as_Monster(cls, buf, offset):
        n = flatbuffers.encode.get(flatbuffers.packer.uoffset, buf, offset)
        x = Monster()
        x.Init(buf, n + offset)
        return x


    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    def pos(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(4))
        if o != 0:
            x = o + self._tab.Pos
            from .Vec3 import Vec3
            obj = Vec3()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    def mana(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(6))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Int16Flags, o + self._tab.Pos)
        return 150

    def hp(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(8))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Int16Flags, o + self._tab.Pos)
        return 100

    def name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(10))
        if o != 0:
            return self._tab.string(o + self._tab.Pos)
        return ""

    def inventory(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(14))
        if o != 0:
            a = self._tab.vector(o)
            return self._tab.get(flatbuffers.number_types.Uint8Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 1))
        return 0

    def inventory_length(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(14))
        if o != 0:
            return self._tab.vector_len(o)
        return 0

    def color(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(16))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Int8Flags, o + self._tab.Pos)
        return 8

    def test_type(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(18))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    def test(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(20))
        if o != 0:
            from flatbuffers.table import Table
            obj = Table(bytearray(), 0)
            self._tab.union(obj, o)
            return obj
        return None

    def test4(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(22))
        if o != 0:
            x = self._tab.vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            from .Test import Test
            obj = Test()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    def test4_length(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(22))
        if o != 0:
            return self._tab.vector_len(o)
        return 0

    def testarrayofstring(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(24))
        if o != 0:
            a = self._tab.vector(o)
            return self._tab.string(a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return ""

    def testarrayofstring_length(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(24))
        if o != 0:
            return self._tab.vector_len(o)
        return 0

# /// an example documentation comment: this will end up in the generated code
# /// multiline too
    def testarrayoftables(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(26))
        if o != 0:
            x = self._tab.vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.indirect(x)
            from .Monster import Monster
            obj = Monster()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    def testarrayoftables_length(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(26))
        if o != 0:
            return self._tab.vector_len(o)
        return 0

    def enemy(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(28))
        if o != 0:
            x = self._tab.indirect(o + self._tab.Pos)
            from .Monster import Monster
            obj = Monster()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    def testnestedflatbuffer(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(30))
        if o != 0:
            a = self._tab.vector(o)
            return self._tab.get(flatbuffers.number_types.Uint8Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 1))
        return 0

    def testnestedflatbuffer_length(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(30))
        if o != 0:
            return self._tab.vector_len(o)
        return 0

    def testempty(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(32))
        if o != 0:
            x = self._tab.indirect(o + self._tab.Pos)
            from .Stat import Stat
            obj = Stat()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    def testbool(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(34))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos)
        return 0

    def testhashs32_fnv1(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(36))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Int32Flags, o + self._tab.Pos)
        return 0

    def testhashu32_fnv1(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(38))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    def testhashs64_fnv1(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(40))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Int64Flags, o + self._tab.Pos)
        return 0

    def testhashu64_fnv1(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(42))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Uint64Flags, o + self._tab.Pos)
        return 0

    def testhashs32_fnv1a(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(44))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Int32Flags, o + self._tab.Pos)
        return 0

    def testhashu32_fnv1a(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(46))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    def testhashs64_fnv1a(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(48))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Int64Flags, o + self._tab.Pos)
        return 0

    def testhashu64_fnv1a(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(50))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Uint64Flags, o + self._tab.Pos)
        return 0

    def testarrayofbools(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(52))
        if o != 0:
            a = self._tab.vector(o)
            return self._tab.get(flatbuffers.number_types.BoolFlags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 1))
        return 0

    def testarrayofbools_length(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(52))
        if o != 0:
            return self._tab.vector_len(o)
        return 0

def Monster_start(builder):
    builder.start_object(25)
def Monster_add_pos(builder, pos):
    builder.prepend_Struct_slot(0, flatbuffers.number_types.UOffsetTFlags.py_type(pos), 0)
def Monster_add_mana(builder, mana):
    builder.prepend_Int16_slot(1, mana, 150)
def Monster_add_hp(builder, hp):
    builder.prepend_Int16_slot(2, hp, 100)
def Monster_add_name(builder, name):
    builder.prepend_UOffsetT_relative_slot(3, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def Monster_add_inventory(builder, inventory):
    builder.prepend_UOffsetT_relative_slot(5, flatbuffers.number_types.UOffsetTFlags.py_type(inventory), 0)
def Monster_start_inventory_vector(builder, numElems):
    return builder.start_vector(1, numElems, 1)
def Monster_add_color(builder, color):
    builder.prepend_Int8_slot(6, color, 8)
def Monster_add_test_type(builder, test_type):
    builder.prepend_Uint8_slot(7, test_type, 0)
def Monster_add_test(builder, test):
    builder.prepend_UOffsetT_relative_slot(8, flatbuffers.number_types.UOffsetTFlags.py_type(test), 0)
def Monster_add_test4(builder, test4):
    builder.prepend_UOffsetT_relative_slot(9, flatbuffers.number_types.UOffsetTFlags.py_type(test4), 0)
def Monster_start_test4_vector(builder, numElems):
    return builder.start_vector(4, numElems, 2)
def Monster_add_testarrayofstring(builder, testarrayofstring):
    builder.prepend_UOffsetT_relative_slot(10, flatbuffers.number_types.UOffsetTFlags.py_type(testarrayofstring), 0)
def Monster_start_testarrayofstring_vector(builder, numElems):
    return builder.start_vector(4, numElems, 4)
def Monster_add_testarrayoftables(builder, testarrayoftables):
    builder.prepend_UOffsetT_relative_slot(11, flatbuffers.number_types.UOffsetTFlags.py_type(testarrayoftables), 0)
def Monster_start_testarrayoftables_vector(builder, numElems):
    return builder.start_vector(4, numElems, 4)
def Monster_add_enemy(builder, enemy):
    builder.prepend_UOffsetT_relative_slot(12, flatbuffers.number_types.UOffsetTFlags.py_type(enemy), 0)
def Monster_add_testnestedflatbuffer(builder, testnestedflatbuffer):
    builder.prepend_UOffsetT_relative_slot(13, flatbuffers.number_types.UOffsetTFlags.py_type(testnestedflatbuffer), 0)
def Monster_start_testnestedflatbuffer_vector(builder, numElems):
    return builder.start_vector(1, numElems, 1)
def Monster_add_testempty(builder, testempty):
    builder.prepend_UOffsetT_relative_slot(14, flatbuffers.number_types.UOffsetTFlags.py_type(testempty), 0)
def Monster_add_testbool(builder, testbool):
    builder.prepend_Bool_slot(15, testbool, 0)
def Monster_add_testhashs32_fnv1(builder, testhashs32_fnv1):
    builder.prepend_Int32_slot(16, testhashs32_fnv1, 0)
def Monster_add_testhashu32_fnv1(builder, testhashu32_fnv1):
    builder.prepend_Uint32_slot(17, testhashu32_fnv1, 0)
def Monster_add_testhashs64_fnv1(builder, testhashs64_fnv1):
    builder.prepend_Int64_slot(18, testhashs64_fnv1, 0)
def Monster_add_testhashu64_fnv1(builder, testhashu64_fnv1):
    builder.prepend_Uint64_slot(19, testhashu64_fnv1, 0)
def Monster_add_testhashs32_fnv1a(builder, testhashs32_fnv1a):
    builder.prepend_Int32_slot(20, testhashs32_fnv1a, 0)
def Monster_add_testhashu32_fnv1a(builder, testhashu32_fnv1a):
    builder.prepend_Uint32_slot(21, testhashu32_fnv1a, 0)
def Monster_add_testhashs64_fnv1a(builder, testhashs64_fnv1a):
    builder.prepend_Int64_slot(22, testhashs64_fnv1a, 0)
def Monster_add_testhashu64_fnv1a(builder, testhashu64_fnv1a):
    builder.prepend_Uint64_slot(23, testhashu64_fnv1a, 0)
def Monster_add_testarrayofbools(builder, testarrayofbools):
    builder.prepend_UOffsetT_relative_slot(24, flatbuffers.number_types.UOffsetTFlags.py_type(testarrayofbools), 0)
def Monster_start_testarrayofbools_vector(builder, numElems):
    return builder.start_vector(1, numElems, 1)
def Monster_end(builder):
    return builder.end_object()
