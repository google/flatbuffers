# automatically generated, do not modify

# namespace: Example

import flatbuffers


class Monster(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRoot(cls, buf, offset):
        n = flatbuffers.GetUOffset(buf, offset)
        x = cls()
        x.Init(buf, n + offset)
        return x

    # Monster
    def Init(self, buf, pos):
        self._tab = flatbuffers.Table(buf, pos)

    pos = flatbuffers.StructProperty('Vec3', __package__, 4, True)
    mana = flatbuffers.Int16Property(6, default=150)
    hp = flatbuffers.Int16Property(8, default=100)
    name = flatbuffers.StringProperty(10)
    inventory = flatbuffers.VectorUint8Property(14)
    inventory_length = flatbuffers.VectorLengthProperty(14)
    color = flatbuffers.Int8Property(16, default=8)
    test_type = flatbuffers.Uint8Property(18, default=0)
    test = flatbuffers.UnionProperty(20)
    test4 = flatbuffers.VectorStructProperty('Test', __package__, 22, True)
    test4_length = flatbuffers.VectorLengthProperty(22)
    testarrayofstring = flatbuffers.VectorStringProperty(24)
    testarrayofstring_length = flatbuffers.VectorLengthProperty(24)
# /// an example documentation comment: this will end up in the generated code
# /// multiline too
    testarrayoftables = flatbuffers.VectorStructProperty('Monster', __package__, 26, False)
    testarrayoftables_length = flatbuffers.VectorLengthProperty(26)
    enemy = flatbuffers.StructProperty('Monster', __package__, 28, False)
    testnestedflatbuffer = flatbuffers.VectorUint8Property(30)
    testnestedflatbuffer_length = flatbuffers.VectorLengthProperty(30)
    testempty = flatbuffers.StructProperty('Stat', __package__, 32, False)
    testbool = flatbuffers.BoolProperty(34, default=0)
    testhashs32_fnv1 = flatbuffers.Int32Property(36, default=0)
    testhashu32_fnv1 = flatbuffers.Uint32Property(38, default=0)
    testhashs64_fnv1 = flatbuffers.Int64Property(40, default=0)
    testhashu64_fnv1 = flatbuffers.Uint64Property(42, default=0)
    testhashs32_fnv1a = flatbuffers.Int32Property(44, default=0)
    testhashu32_fnv1a = flatbuffers.Uint32Property(46, default=0)
    testhashs64_fnv1a = flatbuffers.Int64Property(48, default=0)
    testhashu64_fnv1a = flatbuffers.Uint64Property(50, default=0)

    @classmethod
    def CreateObject(cls, builder, d, finished=True):
        d_get = d.get
        v = d_get('inventory', None)
        o_inventory = None if v is None else cls.inventory.build(builder, v, 1)
        v = d_get('test4', None)
        o_test4 = None if v is None else cls.test4.build(builder, v, 2)
        v = d_get('testarrayofstring', None)
        o_testarrayofstring = None if v is None else cls.testarrayofstring.build(builder, v, 4)
        v = d_get('testarrayoftables', None)
        o_testarrayoftables = None if v is None else cls.testarrayoftables.build(builder, v, 4)
        v = d_get('enemy', None)
        o_enemy = None if v is None else cls.enemy.build(builder, v, 0)
        v = d_get('testnestedflatbuffer', None)
        o_testnestedflatbuffer = None if v is None else cls.testnestedflatbuffer.build(builder, v, 1)
        v = d_get('testempty', None)
        o_testempty = None if v is None else cls.testempty.build(builder, v, 0)
        builder.StartObject(24)
        v = d_get('pos', None)
        if v is not None:
            v = cls.pos.build(builder, v, 0)
            builder.PrependStructSlot(0, v, 0)
        v = d_get('mana', None)
        if v is not None:
            builder.PrependInt16Slot(1, v, 150)
        v = d_get('hp', None)
        if v is not None:
            builder.PrependInt16Slot(2, v, 100)
        v = d_get('name', None)
        if v is not None:
            v = builder.CreateString(v)
            builder.PrependUOffsetTRelativeSlot(3, v, 0)
        if o_inventory is not None:
            builder.PrependUOffsetTRelativeSlot(5, o_inventory, 0)
        v = d_get('color', None)
        if v is not None:
            builder.PrependInt8Slot(6, v, 8)
        v = d_get('test_type', None)
        if v is not None:
            builder.PrependUint8Slot(7, v, 0)
        v = d_get('test', None)
        if v is not None:
            builder.PrependUOffsetTRelativeSlot(8, v, 0)
        if o_test4 is not None:
            builder.PrependUOffsetTRelativeSlot(9, o_test4, 0)
        if o_testarrayofstring is not None:
            builder.PrependUOffsetTRelativeSlot(10, o_testarrayofstring, 0)
        if o_testarrayoftables is not None:
            builder.PrependUOffsetTRelativeSlot(11, o_testarrayoftables, 0)
        if o_enemy is not None:
            builder.PrependUOffsetTRelativeSlot(12, o_enemy, 0)
        if o_testnestedflatbuffer is not None:
            builder.PrependUOffsetTRelativeSlot(13, o_testnestedflatbuffer, 0)
        if o_testempty is not None:
            builder.PrependUOffsetTRelativeSlot(14, o_testempty, 0)
        v = d_get('testbool', None)
        if v is not None:
            builder.PrependBoolSlot(15, v, 0)
        v = d_get('testhashs32_fnv1', None)
        if v is not None:
            builder.PrependInt32Slot(16, v, 0)
        v = d_get('testhashu32_fnv1', None)
        if v is not None:
            builder.PrependUint32Slot(17, v, 0)
        v = d_get('testhashs64_fnv1', None)
        if v is not None:
            builder.PrependInt64Slot(18, v, 0)
        v = d_get('testhashu64_fnv1', None)
        if v is not None:
            builder.PrependUint64Slot(19, v, 0)
        v = d_get('testhashs32_fnv1a', None)
        if v is not None:
            builder.PrependInt32Slot(20, v, 0)
        v = d_get('testhashu32_fnv1a', None)
        if v is not None:
            builder.PrependUint32Slot(21, v, 0)
        v = d_get('testhashs64_fnv1a', None)
        if v is not None:
            builder.PrependInt64Slot(22, v, 0)
        v = d_get('testhashu64_fnv1a', None)
        if v is not None:
            builder.PrependUint64Slot(23, v, 0)
        o = builder.EndObject()
        if finished:
            builder.Finish(o)
        return o


def MonsterStart(builder): builder.StartObject(24)
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
def MonsterEnd(builder): return builder.EndObject()
