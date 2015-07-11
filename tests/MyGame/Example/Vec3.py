# automatically generated, do not modify

# namespace: Example

import flatbuffers


class Vec3(object):
    __slots__ = ['_tab']

    # Vec3
    def Init(self, buf, pos):
        self._tab = flatbuffers.Table(buf, pos)

    x = flatbuffers.StructFloat32Property(0)
    y = flatbuffers.StructFloat32Property(4)
    z = flatbuffers.StructFloat32Property(8)
    test1 = flatbuffers.StructFloat64Property(16)
    test2 = flatbuffers.StructInt8Property(24)
    test3 = flatbuffers.StructStructProperty('Test', __package__, 26)

    @classmethod
    def CreateObject(cls, builder, d):
        builder.Prep(16, 32)
        builder.Pad(2)
        builder.Prep(2, 4)
        builder.Pad(1)
        builder.PrependInt8(d['test3']['b'])
        builder.PrependInt16(d['test3']['a'])
        builder.Pad(1)
        builder.PrependInt8(d['test2'])
        builder.PrependFloat64(d['test1'])
        builder.Pad(4)
        builder.PrependFloat32(d['z'])
        builder.PrependFloat32(d['y'])
        builder.PrependFloat32(d['x'])
        return builder.Offset()


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
