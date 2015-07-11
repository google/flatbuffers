# automatically generated, do not modify

# namespace: Example

import flatbuffers


class Stat(object):
    __slots__ = ['_tab']

    # Stat
    def Init(self, buf, pos):
        self._tab = flatbuffers.Table(buf, pos)

    id = flatbuffers.StringProperty(4)
    val = flatbuffers.Int64Property(6, default=0)
    count = flatbuffers.Uint16Property(8, default=0)

    @classmethod
    def CreateObject(cls, builder, d, finished=True):
        d_get = d.get
        builder.StartObject(3)
        v = d_get('id', None)
        if v is not None:
            v = builder.CreateString(v)
            builder.PrependUOffsetTRelativeSlot(0, v, 0)
        v = d_get('val', None)
        if v is not None:
            builder.PrependInt64Slot(1, v, 0)
        v = d_get('count', None)
        if v is not None:
            builder.PrependUint16Slot(2, v, 0)
        o = builder.EndObject()
        if finished:
            builder.Finish(o)
        return o


def StatStart(builder): builder.StartObject(3)
def StatAddId(builder, id): builder.PrependUOffsetTRelativeSlot(0, id, 0)
def StatAddVal(builder, val): builder.PrependInt64Slot(1, val, 0)
def StatAddCount(builder, count): builder.PrependUint16Slot(2, count, 0)
def StatEnd(builder): return builder.EndObject()
