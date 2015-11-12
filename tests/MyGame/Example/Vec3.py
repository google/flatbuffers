# automatically generated, do not modify

# namespace: Example

import flatbuffers

class Vec3(object):
    __slots__ = ['_tab']

    # Vec3
    def __init__(self, tab):
        self._tab = tab

    # Vec3
    def X(self): return self._tab.GetFloat32(0)
    # Vec3
    def Y(self): return self._tab.GetFloat32(4)
    # Vec3
    def Z(self): return self._tab.GetFloat32(8)
    # Vec3
    def Test1(self): return self._tab.GetFloat64(16)
    # Vec3
    def Test2(self): return self._tab.GetInt8(24)
    # Vec3
    def Test3(self):
        from .Test import Test
        obj = Test(flatbuffers.Table(self._tab.Bytes, self._tab.Pos + 26))
        return obj


def CreateVec3(builder, x, y, z, test1, test2, test3_a, test3_b):
    builder.Prep(16, 32)
    builder.Pad(2)
    builder.Prep(2, 4)
    builder.Pad(1)
    builder.PrependInt8(test3_b)
    builder.PrependInt16(test3_a)
    builder.Pad(1)
    builder.PrependInt8(test2)
    builder.PrependFloat64(test1)
    builder.Pad(4)
    builder.PrependFloat32(z)
    builder.PrependFloat32(y)
    builder.PrependFloat32(x)
    return builder.Offset()
