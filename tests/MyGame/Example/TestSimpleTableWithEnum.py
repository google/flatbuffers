# automatically generated, do not modify

# namespace: Example

import flatbuffers

class TestSimpleTableWithEnum(object):
    __slots__ = ['_tab']

    # TestSimpleTableWithEnum
    def __init__(self, tab):
        self._tab = tab

    # TestSimpleTableWithEnum
    def Color(self):
        o = self._tab.Offset(4)
        if o != 0:
            return self._tab.GetInt8(o)
        return 2

def TestSimpleTableWithEnumStart(builder): builder.StartObject(1)
def TestSimpleTableWithEnumAddColor(builder, color): builder.PrependInt8Slot(0, color, 2)
def TestSimpleTableWithEnumEnd(builder): return builder.EndObject()
