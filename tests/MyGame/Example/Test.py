# automatically generated, do not modify

# namespace: Example

import flatbuffers

class Test(object):
    __slots__ = ['_tab']

    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    def a(self): return self._tab.get(flatbuffers.number_types.Int16Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(0))
    def b(self): return self._tab.get(flatbuffers.number_types.Int8Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(2))

def CreateTest(builder, a, b):
    builder.prep(2, 4)
    builder.pad(1)
    builder.prepend_Int8(b)
    builder.prepend_Int16(a)
    return builder.offset()
