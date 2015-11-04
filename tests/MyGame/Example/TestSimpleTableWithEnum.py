# automatically generated, do not modify

# namespace: Example

import flatbuffers

class TestSimpleTableWithEnum(object):
    __slots__ = ['_tab']

    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    def color(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.offset(4))
        if o != 0:
            return self._tab.get(flatbuffers.number_types.Int8Flags, o + self._tab.Pos)
        return 2

def TestSimpleTableWithEnum_start(builder):
    builder.start_object(1)
def TestSimpleTableWithEnum_add_color(builder, color):
    builder.prepend_Int8_slot(0, color, 2)
def TestSimpleTableWithEnum_end(builder):
    return builder.end_object()
