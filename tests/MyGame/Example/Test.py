# automatically generated, do not modify

# namespace: Example

import flatbuffers

class Test(object):
    __slots__ = ['_tab']

    # Test
    def __init__(self, tab):
        self._tab = tab

    # Test
    def A(self): return self._tab.GetInt16(0)
    # Test
    def B(self): return self._tab.GetInt8(2)

def CreateTest(builder, a, b):
    builder.Prep(2, 4)
    builder.Pad(1)
    builder.PrependInt8(b)
    builder.PrependInt16(a)
    return builder.Offset()
