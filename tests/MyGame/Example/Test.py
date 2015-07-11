# automatically generated, do not modify

# namespace: Example

import flatbuffers


class Test(object):
    __slots__ = ['_tab']

    # Test
    def Init(self, buf, pos):
        self._tab = flatbuffers.Table(buf, pos)

    a = flatbuffers.StructInt16Property(0)
    b = flatbuffers.StructInt8Property(2)

    @classmethod
    def CreateObject(cls, builder, d):
        builder.Prep(2, 4)
        builder.Pad(1)
        builder.PrependInt8(d['b'])
        builder.PrependInt16(d['a'])
        return builder.Offset()


def CreateTest(builder, a, b):
    builder.Prep(2, 4)
    builder.Pad(1)
    builder.PrependInt8(b)
    builder.PrependInt16(a)
    return builder.Offset()
