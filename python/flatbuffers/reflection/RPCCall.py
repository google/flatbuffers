# automatically generated by the FlatBuffers compiler, do not modify

# namespace: reflection

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class RPCCall(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = RPCCall()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsRPCCall(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    @classmethod
    def RPCCallBufferHasIdentifier(cls, buf, offset, size_prefixed=False):
        return flatbuffers.util.BufferHasIdentifier(buf, offset, b"\x42\x46\x42\x53", size_prefixed=size_prefixed)

    # RPCCall
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # RPCCall
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # RPCCall
    def Request(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Indirect(o + self._tab.Pos)
            from reflection.Object import Object
            obj = Object()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # RPCCall
    def Response(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = self._tab.Indirect(o + self._tab.Pos)
            from reflection.Object import Object
            obj = Object()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # RPCCall
    def Attributes(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from reflection.KeyValue import KeyValue
            obj = KeyValue()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # RPCCall
    def AttributesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # RPCCall
    def AttributesIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        return o == 0

    # RPCCall
    def Documentation(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.String(a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return ""

    # RPCCall
    def DocumentationLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # RPCCall
    def DocumentationIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        return o == 0

def RPCCallStart(builder):
    return builder.StartObject(5)

def Start(builder):
    return RPCCallStart(builder)

def RPCCallAddName(builder, name):
    return builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)

def AddName(builder, name):
    return RPCCallAddName(builder, name)

def RPCCallAddRequest(builder, request):
    return builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(request), 0)

def AddRequest(builder, request):
    return RPCCallAddRequest(builder, request)

def RPCCallAddResponse(builder, response):
    return builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(response), 0)

def AddResponse(builder, response):
    return RPCCallAddResponse(builder, response)

def RPCCallAddAttributes(builder, attributes):
    return builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(attributes), 0)

def AddAttributes(builder, attributes):
    return RPCCallAddAttributes(builder, attributes)

def RPCCallStartAttributesVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartAttributesVector(builder, numElems):
    return RPCCallStartAttributesVector(builder, numElems)

def RPCCallAddDocumentation(builder, documentation):
    return builder.PrependUOffsetTRelativeSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(documentation), 0)

def AddDocumentation(builder, documentation):
    return RPCCallAddDocumentation(builder, documentation)

def RPCCallStartDocumentationVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartDocumentationVector(builder, numElems):
    return RPCCallStartDocumentationVector(builder, numElems)

def RPCCallEnd(builder):
    return builder.EndObject()

def End(builder):
    return RPCCallEnd(builder)
