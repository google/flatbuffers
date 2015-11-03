# automatically generated, do not modify

# namespace: Example

import flatbuffers

class Stat(object):
    __slots__ = ['_tab']

    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    def id(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return ""

    def val(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int64Flags, o + self._tab.Pos)
        return 0

    def count(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint16Flags, o + self._tab.Pos)
        return 0

def Stat_start(builder):
    builder.StartObject(3)
def Stat_add_id(builder, id):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(id), 0)
def Stat_add_val(builder, val):
    builder.PrependInt64Slot(1, val, 0)
def Stat_add_count(builder, count):
    builder.PrependUint16Slot(2, count, 0)
def Stat_end(builder):
    return builder.EndObject()
