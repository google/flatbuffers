# automatically generated, do not modify

# namespace: Example

import flatbuffers

class Stat(object):
    __slots__ = ['_tab']

    def init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    def id(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(4))
        if o != 0:
            return self._tab.string(o + self._tab.Pos)
        return ""

    def val(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(6))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Int64Flags, o + self._tab.Pos)
        return 0

    def count(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(8))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Uint16Flags, o + self._tab.Pos)
        return 0

def start(builder):
    builder.start_object(3)
def add_id(builder, id):
    builder.prepend_UOffsetT_relative_slot(0, flatbuffers.number_types.UOffsetTFlags.py_type(id), 0)
def add_val(builder, val):
    builder.prepend_Int64_slot(1, val, 0)
def add_count(builder, count):
    builder.prepend_Uint16_slot(2, count, 0)
def end(builder):
    return builder.end_object()
