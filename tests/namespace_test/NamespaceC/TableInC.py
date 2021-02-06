# automatically generated by the FlatBuffers compiler, do not modify

# namespace: NamespaceC

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class TableInC(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        buf = flatbuffers.compat.memoryview_type(buf) if not isinstance(buf, flatbuffers.compat.memoryview_type) else buf
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = TableInC()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsTableInC(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # TableInC
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # TableInC
    def ReferToA1(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Indirect(o + self._tab.Pos)
            obj = TableInFirstNS()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # TableInC
    def ReferToA2(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Indirect(o + self._tab.Pos)
            obj = SecondTableInA()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

def Start(builder): builder.StartObject(2)
def TableInCStart(builder):
    """This method is deprecated. Please switch to Start."""
    return Start(builder)
def AddReferToA1(builder, referToA1): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(referToA1), 0)
def TableInCAddReferToA1(builder, referToA1):
    """This method is deprecated. Please switch to AddReferToA1."""
    return AddReferToA1(builder, referToA1)
def AddReferToA2(builder, referToA2): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(referToA2), 0)
def TableInCAddReferToA2(builder, referToA2):
    """This method is deprecated. Please switch to AddReferToA2."""
    return AddReferToA2(builder, referToA2)
def End(builder): return builder.EndObject()
def TableInCEnd(builder):
    """This method is deprecated. Please switch to End."""
    return End(builder)
try:
    from typing import Optional
except:
    pass

class TableInCT(object):

    # TableInCT
    def __init__(self):
        self.referToA1 = None  # type: Optional[TableInFirstNST]
        self.referToA2 = None  # type: Optional[SecondTableInAT]

    @classmethod
    def InitFromBuf(cls, buf, pos):
        tableInC = TableInC()
        tableInC.Init(buf, pos)
        return cls.InitFromObj(tableInC)

    @classmethod
    def InitFromObj(cls, tableInC):
        x = TableInCT()
        x._UnPack(tableInC)
        return x

    # TableInCT
    def _UnPack(self, tableInC):
        if tableInC is None:
            return
        if tableInC.ReferToA1() is not None:
            self.referToA1 = TableInFirstNST.InitFromObj(tableInC.ReferToA1())
        if tableInC.ReferToA2() is not None:
            self.referToA2 = SecondTableInAT.InitFromObj(tableInC.ReferToA2())

    # TableInCT
    def Pack(self, builder):
        if self.referToA1 is not None:
            referToA1 = self.referToA1.Pack(builder)
        if self.referToA2 is not None:
            referToA2 = self.referToA2.Pack(builder)
        Start(builder)
        if self.referToA1 is not None:
            AddReferToA1(builder, referToA1)
        if self.referToA2 is not None:
            AddReferToA2(builder, referToA2)
        tableInC = End(builder)
        return tableInC
