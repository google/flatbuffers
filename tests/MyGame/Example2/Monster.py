# automatically generated by the FlatBuffers compiler, do not modify

# namespace: Example2

import flatbuffers

class Monster(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsMonster(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Monster()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def MonsterBufferHasIdentifier(cls, buf, offset, size_prefixed = False):
        return flatbuffers.util.BufferHasIdentifier(buf, offset, b"\x4D\x4F\x4E\x53", size_prefixed)

    # Monster
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

def MonsterStart(builder): builder.StartObject(0)
def MonsterEnd(builder): return builder.EndObject()
