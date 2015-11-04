# automatically generated, do not modify

# namespace: Example

import flatbuffers

class Vec3(object):
    __slots__ = ['_tab']

    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    def x(self): return self._tab.get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(0))
    def y(self): return self._tab.get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(4))
    def z(self): return self._tab.get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(8))
    def test1(self): return self._tab.get(flatbuffers.number_types.Float64Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(16))
    def test2(self): return self._tab.get(flatbuffers.number_types.Int8Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(24))
    def test3(self, obj):
        obj.Init(self._tab.Bytes, self._tab.Pos + 26)
        return obj


def CreateVec3(builder, x, y, z, test1, test2, test3_a, test3_b):
    builder.prep(16, 32)
    builder.pad(2)
    builder.prep(2, 4)
    builder.pad(1)
    builder.prepend_Int8(test3_b)
    builder.prepend_Int16(test3_a)
    builder.pad(1)
    builder.prepend_Int8(test2)
    builder.prepend_Float64(test1)
    builder.pad(4)
    builder.prepend_Float32(z)
    builder.prepend_Float32(y)
    builder.prepend_Float32(x)
    return builder.offset()
