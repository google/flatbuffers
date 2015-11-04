# Copyright 2014 Google Inc. All rights reserved.
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

from . import encode
from . import number_types as N


class Table(object):
    """Table wraps a byte slice and provides read access to its data.

    The variable `Pos` indicates the root of the FlatBuffers object therein."""

    __slots__ = ("Bytes", "Pos")

    def __init__(self, buf, pos):
        N.enforce_number(pos, N.UOffsetTFlags)

        self.Bytes = buf
        self.Pos = pos

    def offset(self, vtableOffset):
        """offset provides access into the Table's vtable.

        Deprecated fields are ignored by checking the vtable's length."""

        vtable = self.Pos - self.get(N.SOffsetTFlags, self.Pos)
        vtableEnd = self.get(N.VOffsetTFlags, vtable)
        if vtableOffset < vtableEnd:
            return self.get(N.VOffsetTFlags, vtable + vtableOffset)
        return 0

    def indirect(self, off):
        """indirect retrieves the relative offset stored at `offset`."""
        N.enforce_number(off, N.UOffsetTFlags)
        return off + encode.get(N.UOffsetTFlags.packer_type, self.Bytes, off)

    def string(self, off):
        """string gets a string from data stored inside the flatbuffer."""
        N.enforce_number(off, N.UOffsetTFlags)
        off += encode.get(N.UOffsetTFlags.packer_type, self.Bytes, off)
        start = off + N.UOffsetTFlags.bytewidth
        length = encode.get(N.UOffsetTFlags.packer_type, self.Bytes, off)
        return bytes(self.Bytes[start:start+length])

    def vector_len(self, off):
        """vector_len retrieves the length of the vector whose offset is stored
           at "off" in this object."""
        N.enforce_number(off, N.UOffsetTFlags)

        off += self.Pos
        off += encode.get(N.UOffsetTFlags.packer_type, self.Bytes, off)
        ret = encode.get(N.UOffsetTFlags.packer_type, self.Bytes, off)
        return ret

    def vector(self, off):
        """vector retrieves the start of data of the vector whose offset is
           stored at "off" in this object."""
        N.enforce_number(off, N.UOffsetTFlags)

        off += self.Pos
        x = off + self.get(N.UOffsetTFlags, off)
        # data starts after metadata containing the vector length
        x += N.UOffsetTFlags.bytewidth
        return x

    def union(self, t2, off):
        """union initializes any Table-derived type to point to the union at
           the given offset."""
        assert type(t2) is Table
        N.enforce_number(off, N.UOffsetTFlags)

        off += self.Pos
        t2.Pos = off + self.get(N.UOffsetTFlags, off)
        t2.Bytes = self.Bytes

    def get(self, flags, off):
        """
        get retrieves a value of the type specified by `flags`  at the
        given offset.
        """
        N.enforce_number(off, N.UOffsetTFlags)
        return flags.py_type(encode.get(flags.packer_type, self.Bytes, off))

    def get_slot(self, slot, d, validator_flags):
        N.enforce_number(slot, N.VOffsetTFlags)
        if validator_flags is not None:
            N.enforce_number(d, validator_flags)
        off = self.offset(slot)
        if off == 0:
            return d
        return self.get(validator_flags, self.Pos + off)

    def get_VOffsetT_slot(self, slot, d):
        """
        get_VOffsetT_slot retrieves the VOffsetT that the given vtable location
        points to. If the vtable value is zero, the default value `d`
        will be returned.
        """

        N.enforce_number(slot, N.VOffsetTFlags)
        N.enforce_number(d, N.VOffsetTFlags)

        off = self.offset(slot)
        if off == 0:
                return d
        return off
