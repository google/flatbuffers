# automatically generated by the FlatBuffers compiler, do not modify

# namespace: NestedUnion

import flatbuffers
from flatbuffers.compat import import_numpy
from typing import Any
np = import_numpy()

class Test(object):
    __slots__ = ['_tab']

    @classmethod
    def SizeOf(cls) -> int:
        return 4

    # Test
    def Init(self, buf: bytes, pos: int):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Test
    def A(self): return self._tab.Get(flatbuffers.number_types.Int16Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(0))
    # Test
    def B(self): return self._tab.Get(flatbuffers.number_types.Int8Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(2))

def CreateTest(builder, a, b):
    builder.Prep(2, 4)
    builder.Pad(1)
    builder.PrependInt8(b)
    builder.PrependInt16(a)
    return builder.Offset()


class TestT(object):

    # TestT
    def __init__(self):
        self.a = 0  # type: int
        self.b = 0  # type: int

    @classmethod
    def InitFromBuf(cls, buf, pos):
        test = Test()
        test.Init(buf, pos)
        return cls.InitFromObj(test)

    @classmethod
    def InitFromPackedBuf(cls, buf, pos=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, pos)
        return cls.InitFromBuf(buf, pos+n)

    @classmethod
    def InitFromObj(cls, test):
        x = TestT()
        x._UnPack(test)
        return x

    # TestT
    def _UnPack(self, test):
        if test is None:
            return
        self.a = test.A()
        self.b = test.B()

    # TestT
    def Pack(self, builder):
        return CreateTest(builder, self.a, self.b)
