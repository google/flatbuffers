# -*- coding:utf8 -*-
# distutils: language = c++
# cython: always_allow_keywords = False
#
# Copyright 2015 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from libc.stdint cimport (
    int8_t,
    int16_t,
    int32_t,
    int64_t,
    uint8_t,
    uint16_t,
    uint32_t,
    uint64_t,
    uintmax_t)
from libc.stddef cimport size_t
from libcpp cimport bool as bool_t
from cpython.bytes cimport PyBytes_AS_STRING

from .exceptions import (
    OffsetArithmeticError,
    NotInObjectError,
    ObjectIsNestedError,
    StructIsNotInlineError,
    BuilderSizeError)


cdef extern from "flatbuffers/flatwrapper.h" namespace "flatbuffers":
    ctypedef uint32_t uoffset_t
    ctypedef int32_t soffset_t
    ctypedef uint16_t voffset_t
    ctypedef uintmax_t largest_scalar_t

    cdef cppclass Offset[T]:
        uoffset_t o
        Offset()
        Offset(uoffset_t _o)

    void EndianCheck()
    T EndianScalar[T](T t)
    T ReadScalar[T](const void *p)
    void WriteScalar[T](void *p, T t)
    size_t AlignOf[T]()

    cdef cppclass Vector[T]:
        uoffset_t size()
        const uint8_t *Data()

    size_t VectorLength[T](const Vector[T] *v)

    cdef cppclass String(Vector[char]):
        pass

    cdef cppclass simple_allocator:
        pass

    voffset_t FieldIndexToOffset(voffset_t field_id)
    size_t PaddingBytes(size_t buf_size, size_t scalar_size)

    cdef cppclass FlatBufferBuilder:
        FlatBufferBuilder()
        FlatBufferBuilder(uoffset_t initial_size)
        FlatBufferBuilder(uoffset_t initial_size, const simple_allocator *allocator)
        void Clear()
        uoffset_t GetSize()
        uint8_t *GetBufferPointer()
        void ForceDefaults(bool_t fd)
        void Pad(size_t num_bytes)
        void Align(size_t elem_size)
        void PushBytes(const uint8_t *bytes, size_t size)
        void PopBytes(size_t amount)
        void AssertScalarT[T]()
        uoffset_t PushElement[T](T element)
        void TrackField(voffset_t field, uoffset_t off)
        void AddElement[T](voffset_t field, T e, T def_)
        void AddOffset[T](voffset_t field, Offset[T] off)
        void AddStruct[T](voffset_t field, const T *structptr)
        void AddStructOffset(voffset_t field, uoffset_t off)
        uoffset_t ReferTo(uoffset_t off)
        void NotNested()
        uoffset_t StartTable()
        uoffset_t EndTable(uoffset_t start, voffset_t numfields)
        void Required[T](Offset[T] table, voffset_t field)
        uoffset_t StartStruct(size_t alignment)
        uoffset_t EndStruct()
        void ClearOffsets()
        void PreAlign(size_t len, size_t alignment)
        void PreAlign[T](size_t len)
        Offset[String] CreateString(const char *str)
        Offset[String] CreateString(const char *str, size_t len)
        uoffset_t EndVector(size_t len)
        void StartVector(size_t len, size_t elemsize)
        uint8_t *ReserveElements(size_t len, size_t elemsize)
        Offset[Vector[T]] CreateVector[T](const T *v, size_t len)
        Offset[Vector[T]] CreateVectorOfStructs[T](T v, size_t len)
        Offset[Vector[Offset[T]]] CreateVectorOfSortedTables[T](Offset[T] *v, size_t len)
        uoffset_t CreateUninitializedVector(size_t len, size_t elemsize, uint8_t **buf)
        Offset[Vector[T]] CreateUninitializedVector[T](size_t len, T **buf)
        void Finish[T](Offset[T] root, const char *file_identifier)
        void Finish[T](Offset[T] root)

    T GetMutableRoot[T](void *buf)
    const T GetRoot[T](const void *buf)
    bool_t BufferHasIdentifier(const void *buf, const char *identifier)

    cdef cppclass Verifier:
        Verifier(const uint8_t *buf, size_t buf_len)
        Verifier(const uint8_t *buf, size_t buf_len, size_t _max_depth)
        Verifier(const uint8_t *buf, size_t buf_len, size_t _max_depth, size_t _max_tables)
        bool_t Check(bool_t ok)
        bool_t Verify(const void *elem, size_t elem_len)
        bool_t Verify[T](const void *elem)
        bool_t VerifyTable[T](const T *table)
        bool_t Verify[T](const Vector[T] *vec)
        bool_t Verify(const String *str)
        bool_t VerifyVector(const uint8_t *vec, size_t elem_size, const uint8_t **end)
        bool_t VerifyVectorOfStrings[T](const Vector[Offset[String]] *vec)
        bool_t VerifyVectorOfTables[T](const Vector[Offset[T]] *vec)
        bool_t VerifyBuffer[T]()
        bool_t VerifyComplexity()
        bool_t EndTable()

    cdef cppclass Struct:
        T GetField[T](uoffset_t o)
        T GetPointer[T](uoffset_t o)
        T GetStruct[T](uoffset_t o)

    cdef cppclass Table:
        voffset_t GetOptionalFieldOffset(voffset_t field)
        T GetField[T](voffset_t field, T defaultval)
        P GetPointer[P](voffset_t field)
        P GetStruct[P](voffset_t field)
        bool_t SetField[T](voffset_t field, T val)
        uint8_t *GetAddressOf(voffset_t field)
        uint8_t *GetVTable()
        bool_t CheckField(voffset_t field)
        bool_t VerifyTableStart(Verifier &verifier)
        bool_t VerifyField[T](const Verifier &verifier, voffset_t field)
        bool_t VerifyFieldRequired[T](const Verifier &verifier, voffset_t field)


cdef extern from "flatwrapper.h":
    cdef cppclass PythonAllocator(simple_allocator):
        pass

    uoffset_t PushUOffsetElement(FlatBufferBuilder *builder, Offset[void] off)
    const Table *GetTable(const void *buf)


cdef PythonAllocator pyallocator


cdef class FastBuilder(object):
    cdef FlatBufferBuilder* thisptr
    cdef uoffset_t table_offset
    cdef voffset_t table_numfields
    cdef bool_t    table_started

    def __cinit__(self, uoffset_t initial_size):
        self.thisptr = new FlatBufferBuilder(initial_size, &pyallocator)
        self.table_started = False

    def __dealloc__(self):
        if self.thisptr:
            del self.thisptr

    property Bytes:
        def __get__(self): return self.Output()

    def Head(self):
        return 0

    def Offset(self):
        return self.thisptr.GetSize()

    cpdef bytes Output(self):
        cdef uint8_t *buf
        buf = self.thisptr.GetBufferPointer()
        return buf[:self.thisptr.GetSize()]

    def Reset(self):
        self.thisptr.Clear()
        self.table_started = False

    def StartObject(self, voffset_t numfields):
        if self.table_started:
            raise ObjectIsNestedError("flatbuffers: Tried to write a new "
                "Object when the Builder was already writing an Object.")
        self.table_offset = self.thisptr.StartTable()
        self.table_numfields = numfields
        self.table_started = True

    def EndObject(self):
        cdef uoffset_t offset
        if not self.table_started:
            raise NotInObjectError("flatbuffers: Tried to write the end of an "
                "Object when the Builder was not currently writing an Object.")
        offset = self.thisptr.EndTable(self.table_offset, self.table_numfields)
        self.table_started = False
        return offset

    def Pad(self, size_t num_bytes):
        self.thisptr.Pad(num_bytes)

    def Prep(self, size_t alignment, size_t len):
        self.thisptr.PreAlign(len, alignment)

    def PrependUOffsetTRelative(self, uoffset_t off):
        self.thisptr.PreAlign(0, 4)
        if off > self.thisptr.GetSize():
            raise OffsetArithmeticError("flatbuffers: Offset arithmetic error.")
        # self.thisptr.PushElement[void](Offset[void](off))
        PushUOffsetElement(self.thisptr, Offset[void](off))

    def StartVector(self, size_t elemsize, size_t len, size_t alignment):
        if self.table_started:
            raise ObjectIsNestedError("flatbuffers: Tried to write a new "
                "Object when the Builder was already writing an Object.")
        self.thisptr.StartVector(len, elemsize)
        return self.thisptr.GetSize()

    def EndVector(self, size_t len):
        return self.thisptr.EndVector(len)

    def CreateString(self, bytes s not None):
        cdef Offset[String] off
        if self.table_started:
            raise ObjectIsNestedError("flatbuffers: Tried to write a new "
                "Object when the Builder was already writing an Object.")
        off = self.thisptr.CreateString(PyBytes_AS_STRING(s), <size_t>len(s))
        return off.o

    def Finish(self, uoffset_t root):
        self.thisptr.Finish(Offset[Table](root))
        return 0

    def PrependBoolSlot(self, voffset_t field_id, bool_t e, bool_t d):
        cdef voffset_t field
        field = FieldIndexToOffset(field_id)
        self.thisptr.AddElement[uint8_t](field, <uint8_t>e, <uint8_t>d)

    def PrependByteSlot(self, voffset_t field_id, uint8_t e, uint8_t d):
        cdef voffset_t field
        field = FieldIndexToOffset(field_id)
        self.thisptr.AddElement[uint8_t](field, e, d)

    def PrependUint8Slot(self, voffset_t field_id, uint8_t e, uint8_t d):
        cdef voffset_t field
        field = FieldIndexToOffset(field_id)
        self.thisptr.AddElement[uint8_t](field, e, d)

    def PrependUint16Slot(self, voffset_t field_id, uint16_t e, uint16_t d):
        cdef voffset_t field
        field = FieldIndexToOffset(field_id)
        self.thisptr.AddElement[uint16_t](field, e, d)

    def PrependUint32Slot(self, voffset_t field_id, uint32_t e, uint32_t d):
        cdef voffset_t field
        field = FieldIndexToOffset(field_id)
        self.thisptr.AddElement[uint32_t](field, e, d)

    def PrependUint64Slot(self, voffset_t field_id, uint64_t e, uint64_t d):
        cdef voffset_t field
        field = FieldIndexToOffset(field_id)
        self.thisptr.AddElement[uint64_t](field, e, d)

    def PrependInt8Slot(self, voffset_t field_id, int8_t e, int8_t d):
        cdef voffset_t field
        field = FieldIndexToOffset(field_id)
        self.thisptr.AddElement[int8_t](field, e, d)

    def PrependInt16Slot(self, voffset_t field_id, int16_t e, int16_t d):
        cdef voffset_t field
        field = FieldIndexToOffset(field_id)
        self.thisptr.AddElement[int16_t](field, e, d)

    def PrependInt32Slot(self, voffset_t field_id, int32_t e, int32_t d):
        cdef voffset_t field
        field = FieldIndexToOffset(field_id)
        self.thisptr.AddElement[int32_t](field, e, d)

    def PrependInt64Slot(self, voffset_t field_id, int64_t e, int64_t d):
        cdef voffset_t field
        field = FieldIndexToOffset(field_id)
        self.thisptr.AddElement[int64_t](field, e, d)

    def PrependFloat32Slot(self, voffset_t field_id, float e, float d):
        cdef voffset_t field
        field = FieldIndexToOffset(field_id)
        self.thisptr.AddElement[float](field, e, d)

    def PrependFloat64Slot(self, voffset_t field_id, double e, double d):
        cdef voffset_t field
        field = FieldIndexToOffset(field_id)
        self.thisptr.AddElement[double](field, e, d)

    def PrependUOffsetTRelativeSlot(self, voffset_t field_id, uoffset_t e, uoffset_t d):
        cdef voffset_t field
        field = FieldIndexToOffset(field_id)
        self.thisptr.AddOffset[void](field, Offset[void](e))

    def PrependStructSlot(self, voffset_t field_id, uoffset_t e, uoffset_t d):
        cdef voffset_t field
        if e != d:
            if self.thisptr.GetSize() != e:
                raise StructIsNotInlineError("flatbuffers: Tried to write a "
                    "Struct at an Offset that is different from the current "
                    "Offset of the Builder.")
            field = FieldIndexToOffset(field_id)
            self.thisptr.AddStructOffset(field, e)

    def PrependBool(self, bool_t e):
        self.thisptr.PushElement[uint8_t](e)

    def PrependByte(self, uint8_t e):
        self.thisptr.PushElement[uint8_t](e)

    def PrependUint8(self, uint8_t e):
        self.thisptr.PushElement[uint8_t](e)

    def PrependUint16(self, uint16_t e):
        self.thisptr.PushElement[uint16_t](e)

    def PrependUint32(self, uint32_t e):
        self.thisptr.PushElement[uint32_t](e)

    def PrependUint64(self, uint64_t e):
        self.thisptr.PushElement[uint64_t](e)

    def PrependInt8(self, int8_t e):
        self.thisptr.PushElement[int8_t](e)

    def PrependInt16(self, int16_t e):
        self.thisptr.PushElement[int16_t](e)

    def PrependInt32(self, int32_t e):
        self.thisptr.PushElement[int32_t](e)

    def PrependInt64(self, int64_t e):
        self.thisptr.PushElement[int64_t](e)

    def PrependFloat32(self, float e):
        self.thisptr.PushElement[float](e)

    def PrependFloat64(self, double e):
        self.thisptr.PushElement[double](e)

    def PrependVOffsetT(self, voffset_t e):
        self.thisptr.PushElement[voffset_t](e)


def _FastTable_GetRoot(cls, bytes buf not None, uoffset_t pos):
    cdef size_t size
    cdef uoffset_t off
    cdef const uint8_t *raw
    cdef const uint8_t *tab

    size = <size_t>len(buf)
    if sizeof(uoffset_t) > size \
            or pos > size - sizeof(uoffset_t):
        raise IndexError
    raw = <const uint8_t*>PyBytes_AS_STRING(buf)
    tab = <const uint8_t*>GetTable(raw + pos)
    off = <uoffset_t>(tab - raw)
    if off < pos \
            or off > size - sizeof(soffset_t) \
            or (<Table*>tab).GetVTable() < raw + pos:
        raise IndexError
    return cls(buf, off)


cdef class FastTable(object):
    cdef readonly bytes Bytes
    cdef readonly uoffset_t Pos
    cdef size_t size
    cdef const uint8_t *tab

    def __cinit__(self, bytes buf not None, uoffset_t pos):
        self.Bytes = buf
        self.Pos = pos
        if pos < <size_t>len(buf):
            self.tab = <const uint8_t*>PyBytes_AS_STRING(buf) + pos
            self.size = <size_t>len(buf) - pos
        else:
            self.tab = NULL
            self.size = 0

    GetRoot = classmethod(_FastTable_GetRoot)

    def Offset(self, voffset_t field):
        return (<const Table*>self.tab).GetOptionalFieldOffset(field)

    def Indirect(self, uoffset_t off):
        cdef uoffset_t body

        if sizeof(uoffset_t) > self.size or off > self.size - sizeof(uoffset_t):
            raise IndexError
        body = (<const Struct*>self.tab).GetField[uoffset_t](off) + off
        if body < off or body > self.size:
            raise IndexError
        return body

    def String(self, uoffset_t off):
        cdef uoffset_t body
        cdef uoffset_t start
        cdef uoffset_t end

        if sizeof(uoffset_t) > self.size or off > self.size - sizeof(uoffset_t):
            raise IndexError
        body = (<const Struct*>self.tab).GetField[uoffset_t](off) + off
        if body < off or body > self.size - sizeof(uoffset_t):
            raise IndexError
        start = body + sizeof(uoffset_t)
        end = start + (<const Struct*>self.tab).GetField[uoffset_t](body)
        if end < start or end > self.size:
            raise IndexError
        return self.tab[start:end]

    def VectorLen(self, uoffset_t off):
        cdef uoffset_t body

        if sizeof(uoffset_t) > self.size or off > self.size - sizeof(uoffset_t):
            raise IndexError
        body = (<const Struct*>self.tab).GetField[uoffset_t](off) + off
        if body < off or body > self.size - sizeof(uoffset_t):
            raise IndexError
        return (<const Struct*>self.tab).GetField[uoffset_t](body)

    def Vector(self, uoffset_t off):
        cdef uoffset_t body

        if sizeof(uoffset_t) > self.size or off > self.size - sizeof(uoffset_t):
            raise IndexError
        body = (<const Struct*>self.tab).GetField[uoffset_t](off) + off
        if body < off or body > self.size - sizeof(uoffset_t):
            raise IndexError
        return body + sizeof(uoffset_t)

    def Union(self, uoffset_t off):
        cdef uoffset_t body

        if sizeof(uoffset_t) > self.size or off > self.size - sizeof(uoffset_t):
            raise IndexError
        body = (<const Struct*>self.tab).GetField[uoffset_t](off) + off
        if body < off or body > self.size:
            raise IndexError
        return FastTable(self.Bytes, self.Pos + body)

    def GetBool(self, uoffset_t off):
        if sizeof(bool_t) > self.size or off > self.size - sizeof(bool_t):
            raise IndexError
        return (<const Struct*>self.tab).GetField[bool_t](off)

    def GetByte(self, uoffset_t off):
        if sizeof(uint8_t) > self.size or off > self.size - sizeof(uint8_t):
            raise IndexError
        return (<const Struct*>self.tab).GetField[uint8_t](off)

    def GetUint8(self, uoffset_t off):
        if sizeof(uint8_t) > self.size or off > self.size - sizeof(uint8_t):
            raise IndexError
        return (<const Struct*>self.tab).GetField[uint8_t](off)

    def GetUint16(self, uoffset_t off):
        if sizeof(uint16_t) > self.size or off > self.size - sizeof(uint16_t):
            raise IndexError
        return (<const Struct*>self.tab).GetField[uint16_t](off)

    def GetUint32(self, uoffset_t off):
        if sizeof(uint32_t) > self.size or off > self.size - sizeof(uint32_t):
            raise IndexError
        return (<const Struct*>self.tab).GetField[uint32_t](off)

    def GetUint64(self, uoffset_t off):
        if sizeof(uint64_t) > self.size or off > self.size - sizeof(uint64_t):
            raise IndexError
        return (<const Struct*>self.tab).GetField[uint64_t](off)

    def GetInt8(self, uoffset_t off):
        if sizeof(int8_t) > self.size or off > self.size - sizeof(int8_t):
            raise IndexError
        return (<const Struct*>self.tab).GetField[int8_t](off)

    def GetInt16(self, uoffset_t off):
        if sizeof(int16_t) > self.size or off > self.size - sizeof(int16_t):
            raise IndexError
        return (<const Struct*>self.tab).GetField[int16_t](off)

    def GetInt32(self, uoffset_t off):
        if sizeof(int32_t) > self.size or off > self.size - sizeof(int32_t):
            raise IndexError
        return (<const Struct*>self.tab).GetField[int32_t](off)

    def GetInt64(self, uoffset_t off):
        if sizeof(int64_t) > self.size or off > self.size - sizeof(int64_t):
            raise IndexError
        return (<const Struct*>self.tab).GetField[int64_t](off)

    def GetFloat32(self, uoffset_t off):
        if sizeof(float) > self.size or off > self.size - sizeof(float):
            raise IndexError
        return (<const Struct*>self.tab).GetField[float](off)

    def GetFloat64(self, uoffset_t off):
        if sizeof(double) > self.size or off > self.size - sizeof(double):
            raise IndexError
        return (<const Struct*>self.tab).GetField[double](off)

    def GetUOffsetT(self, uoffset_t off):
        if sizeof(uoffset_t) > self.size or off > self.size - sizeof(uoffset_t):
            raise IndexError
        return (<const Struct*>self.tab).GetField[uoffset_t](off)

    def GetVOffsetT(self, uoffset_t off):
        if sizeof(voffset_t) > self.size or off > self.size - sizeof(voffset_t):
            raise IndexError
        return (<const Struct*>self.tab).GetField[voffset_t](off)

    def GetSOffsetT(self, uoffset_t off):
        if sizeof(soffset_t) > self.size or off > self.size - sizeof(soffset_t):
            raise IndexError
        return (<const Struct*>self.tab).GetField[soffset_t](off)

    def GetBoolSlot(self, voffset_t field, default):
        cdef uoffset_t off

        <bool_t>default
        off = (<const Table*>self.tab).GetOptionalFieldOffset(field)
        if off != 0:
            if sizeof(bool_t) > self.size or off > self.size - sizeof(bool_t):
                raise IndexError
            return (<const Struct*>self.tab).GetField[bool_t](off)
        return default

    def GetByteSlot(self, voffset_t field, default):
        cdef uoffset_t off

        <uint8_t>default
        off = (<const Table*>self.tab).GetOptionalFieldOffset(field)
        if off != 0:
            if sizeof(uint8_t) > self.size or off > self.size - sizeof(uint8_t):
                raise IndexError
            return (<const Struct*>self.tab).GetField[uint8_t](off)
        return default

    def GetUint8Slot(self, voffset_t field, default):
        cdef uoffset_t off

        <uint8_t>default
        off = (<const Table*>self.tab).GetOptionalFieldOffset(field)
        if off != 0:
            if sizeof(uint8_t) > self.size or off > self.size - sizeof(uint8_t):
                raise IndexError
            return (<const Struct*>self.tab).GetField[uint8_t](off)
        return default

    def GetUint16Slot(self, voffset_t field, default):
        cdef uoffset_t off

        <uint16_t>default
        off = (<const Table*>self.tab).GetOptionalFieldOffset(field)
        if off != 0:
            if sizeof(uint16_t) > self.size or off > self.size - sizeof(uint16_t):
                raise IndexError
            return (<const Struct*>self.tab).GetField[uint16_t](off)
        return default

    def GetUint32Slot(self, voffset_t field, default):
        cdef uoffset_t off

        <uint32_t>default
        off = (<const Table*>self.tab).GetOptionalFieldOffset(field)
        if off != 0:
            if sizeof(uint32_t) > self.size or off > self.size - sizeof(uint32_t):
                raise IndexError
            return (<const Struct*>self.tab).GetField[uint32_t](off)
        return default

    def GetUint64Slot(self, voffset_t field, default):
        cdef uoffset_t off

        <uint64_t>default
        off = (<const Table*>self.tab).GetOptionalFieldOffset(field)
        if off != 0:
            if sizeof(uint64_t) > self.size or off > self.size - sizeof(uint64_t):
                raise IndexError
            return (<const Struct*>self.tab).GetField[uint64_t](off)
        return default

    def GetInt8Slot(self, voffset_t field, default):
        cdef uoffset_t off

        <int8_t>default
        off = (<const Table*>self.tab).GetOptionalFieldOffset(field)
        if off != 0:
            if sizeof(int8_t) > self.size or off > self.size - sizeof(int8_t):
                raise IndexError
            return (<const Struct*>self.tab).GetField[int8_t](off)
        return default

    def GetInt16Slot(self, voffset_t field, default):
        cdef uoffset_t off

        <int16_t>default
        off = (<const Table*>self.tab).GetOptionalFieldOffset(field)
        if off != 0:
            if sizeof(int16_t) > self.size or off > self.size - sizeof(int16_t):
                raise IndexError
            return (<const Struct*>self.tab).GetField[int16_t](off)
        return default

    def GetInt32Slot(self, voffset_t field, default):
        cdef uoffset_t off

        <int32_t>default
        off = (<const Table*>self.tab).GetOptionalFieldOffset(field)
        if off != 0:
            if sizeof(int32_t) > self.size or off > self.size - sizeof(int32_t):
                raise IndexError
            return (<const Struct*>self.tab).GetField[int32_t](off)
        return default

    def GetInt64Slot(self, voffset_t field, int64_t default):
        cdef uoffset_t off

        <int64_t>default
        off = (<const Table*>self.tab).GetOptionalFieldOffset(field)
        if off != 0:
            if sizeof(int64_t) > self.size or off > self.size - sizeof(int64_t):
                raise IndexError
            return (<const Struct*>self.tab).GetField[int64_t](off)
        return default

    def GetFloat32Slot(self, voffset_t field, default):
        cdef uoffset_t off

        <float>default
        off = (<const Table*>self.tab).GetOptionalFieldOffset(field)
        if off != 0:
            if sizeof(float) > self.size or off > self.size - sizeof(float):
                raise IndexError
            return (<const Struct*>self.tab).GetField[float](off)
        return default

    def GetFloat64Slot(self, voffset_t field, default):
        cdef uoffset_t off

        <double>default
        off = (<const Table*>self.tab).GetOptionalFieldOffset(field)
        if off != 0:
            if sizeof(double) > self.size or off > self.size - sizeof(double):
                raise IndexError
            return (<const Struct*>self.tab).GetField[double](off)
        return default

    def GetVOffsetTSlot(self, voffset_t field, default):
        cdef uoffset_t off

        <voffset_t>default
        off = (<const Table*>self.tab).GetOptionalFieldOffset(field)
        if off != 0:
            return off
        return default
