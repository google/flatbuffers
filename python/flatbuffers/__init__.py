from __future__ import absolute_import
from builtins import range, object

import array
import codecs
import collections
import sys

from . import numtypes

_LITTLE_ENDIAN = (sys.byteorder == 'little')


def read_byte(buf, offset):
    return numtypes.byte_fmt.unpack_from(buf, offset)[0]


def read_ubyte(buf, offset):
    return numtypes.ubyte_fmt.unpack_from(buf, offset)[0]


def read_short(buf, offset):
    return numtypes.short_fmt.unpack_from(buf, offset)[0]


def read_ushort(buf, offset):
    return numtypes.ushort_fmt.unpack_from(buf, offset)[0]


def read_int(buf, offset):
    return numtypes.int_fmt.unpack_from(buf, offset)[0]


def read_uint(buf, offset):
    return numtypes.uint_fmt.unpack_from(buf, offset)[0]


def read_float(buf, offset):
    return numtypes.float_fmt.unpack_from(buf, offset)[0]


def read_long(buf, offset):
    return numtypes.long_fmt.unpack_from(buf, offset)[0]


def read_ulong(buf, offset):
    return numtypes.ulong_fmt.unpack_from(buf, offset)[0]


def read_double(buf, offset):
    return numtypes.double_fmt.unpack_from(buf, offset)[0]


def read_uoffset(buf, offset):
    return numtypes.uoffset_fmt.unpack_from(buf, offset)[0]


def read_soffset(buf, offset):
    return numtypes.soffset_fmt.unpack_from(buf, offset)[0]


utf8_codec = codecs.lookup('utf-8')


def read_string(buf, offset):
    size = read_uint(buf, offset)
    data_offset = offset + numtypes.uoffset_fmt.size
    return utf8_codec.decode(buf[data_offset:data_offset + size])[0]


def read_array(typecode, buf, off, size):
    arr = array.array(typecode)
    data = buf[off:off + size * arr.itemsize]
    if hasattr(arr, 'frombytes'):
        arr.frombytes(data)
    else:
        arr.fromstring(data.tobytes())
    if not _LITTLE_ENDIAN:
        arr.byteswap()
    return arr


def read_vector(typecode, buf, offset):
    offset += read_uoffset(buf, offset)
    size = read_uint(buf, offset)
    data_offset = offset + numtypes.uoffset_fmt.size
    return read_array(typecode, buf, data_offset, size)


def read_vtable(buf, offset):
    data_offset = offset - read_soffset(buf, offset)
    size = read_ushort(buf, data_offset)
    vtable = read_array('H', buf, data_offset, size // 2)
    return vtable[2:]


class StructVector(collections.Sequence):
    def __init__(self, target, itemsize, buf, offset):
        self._target = target
        self._itemsize = itemsize
        self._buf = buf

        data_offset = offset + read_uoffset(buf, offset)
        self._size = read_uint(buf, data_offset)
        self._offset = data_offset + numtypes.uint_fmt.size

    def __getitem__(self, index):
        if index >= self._size:
            raise IndexError('vector index out of range')
        data_offset = self._offset + index * self._itemsize
        return self._target(self._buf, data_offset)

    def __len__(self):
        return self._size


class IndirectVector(collections.Sequence):
    def __init__(self, target, buf, offset):
        self._target = target
        self._buf = buf

        self._offset = offset + numtypes.uoffset_fmt.size
        self._locs = read_vector('I', buf, offset)

    def __getitem__(self, index):
        if index >= len(self._locs):
            raise IndexError('vector index out of range')
        index_offset = (index + 1) * numtypes.uoffset_fmt.size
        data_offset = self._offset + index_offset + self._locs[index]
        return self._target(self._buf, data_offset)

    def __len__(self):
        return len(self._locs)


class Struct(tuple):
    """
    Struct is a flat structures that do not have an offset table, thus
    always have all members present and do not support forwards/backwards
    compatible extensions.
    """
    pass


class Table(object):
    """
    Table uses an offset table (possibly shared) that allows fields to be
    omitted and added at will at the cost of an extra indirection to read.
    """
    def __init__(self, buf, offset):
        self._buf = buf
        self._offset = offset
        self._vtable = read_vtable(buf, self._offset)

    def get_offset(self, field):
        return self._vtable[field] if field < len(self._vtable) else 0

    def read_field(self, field, target, default=None):
        offset = self.get_offset(field)
        if offset == 0:
            return default
        return target(self._buf, self._offset + offset)

    def read_bool_field(self, field, default):
        offset = self.get_offset(field)
        if offset == 0:
            return default
        return bool(read_ubyte(self._buf, self._offset + offset))

    def read_byte_field(self, field, default):
        return self.read_field(field, read_byte, default)

    def read_ubyte_field(self, field, default):
        return self.read_field(field, read_ubyte, default)

    def read_short_field(self, field, default):
        return self.read_field(field, read_short, default)

    def read_ushort_field(self, field, default):
        return self.read_field(field, read_ushort, default)

    def read_int_field(self, field, default):
        return self.read_field(field, read_int, default)

    def read_uint_field(self, field, default):
        return self.read_field(field, read_uint, default)

    def read_float_field(self, field, default):
        return self.read_field(field, read_float, default)

    def read_long_field(self, field, default):
        return self.read_field(field, read_long, default)

    def read_ulong_field(self, field, default):
        return self.read_field(field, read_ulong, default)

    def read_double_field(self, field, default):
        return self.read_field(field, read_double, default)


def padding_bytes(size, scalar_size):
    return ~(size + 1) & (scalar_size - 1)


def _make_vtable(src, obj_size, offset):
    if src:
        vt_size = max(getattr(dict, 'viewkeys', dict.keys)(src)) + 1
    else:
        vt_size = 0
    yield (vt_size + 2) * 2
    yield obj_size
    for i in range(vt_size):
        try:
            field_off = src[i]
            yield offset - field_off
        except KeyError:
            yield 0


class Builder(object):
    """
    Helper class to hold data needed in creation of a flat buffer.
    To serialize data, you typically call one of the create_*() functions in
    the generated code, which in turn call a sequence of start_table/
    push_*/add_*/end_table, or the builtin create_string/
    create_vector functions.
    Do this is depth-first order to build up a tree to the root.
    finish() wraps up the buffer ready for transport.
    """

    def __init__(self, block_size=1024, force_defaults=False):
        self._buf = DownwardArray(block_size)
        self._force_defaults = force_defaults
        self._minalign = 1
        self._vtable = {}
        self._vtables = {}

    def data(self):
        return self._buf.data()

    def pre_align(self, size, scalar_size):
        padding = padding_bytes(len(self._buf) + size, scalar_size)
        if padding != 0:
            self._buf.fill(padding)

    def align(self, scalar_size):
        self._minalign = max(self._minalign, scalar_size)
        padding = padding_bytes(len(self._buf), scalar_size)
        if padding != 0:
            self._buf.fill(padding)

    def track_field(self, field, offset):
        """
        When writing fields, we track where they are, so we can create correct
        vtables later.
        """
        assert field not in self._vtable
        self._vtable[field] = offset

    def not_nested(self):
        assert not self._vtable

    def start_table(self):
        """
        From generated code (or from the parser), we call StartTable/EndTable
        with a sequence of AddElement calls in between.
        """
        self.not_nested()
        return len(self._buf)

    def end_table(self, start):
        """
        This finishes one serialized object by generating the vtable if it's a
        table, comparing it against existing vtables, and writing the
        resulting vtable offset.
        """
        self.align(numtypes.soffset_fmt.size)
        vt_off_loc = len(self._buf) + numtypes.soffset_fmt.size
        obj_size = vt_off_loc - start
        assert obj_size < 0x10000

        vtable_data = tuple(_make_vtable(self._vtable, obj_size, vt_off_loc))
        vtable = array.array('H', vtable_data)
        try:
            vt_offset = self._vtables[vtable_data]
            self.push_soffset(vt_offset - vt_off_loc)
        except KeyError:
            self.push_soffset(len(vtable) * vtable.itemsize)
            vt_offset = self._buf.extend_array(vtable)
            self._vtables[vtable_data] = vt_offset
        self._vtable = {}
        return vt_off_loc

    def refer_to(self, offset):
        """
        Offsets initially are relative to the end of the buffer (downwards).
        This function converts them to be relative to the current location
        in the buffer (when stored here), pointing upwards.
        """
        self.align(numtypes.uoffset_fmt.size)
        assert offset <= len(self._buf)
        return len(self._buf) - offset + numtypes.uoffset_fmt.size

    def _make_writers(fmt):
        def push_element(self, value):
            self.align(fmt.size)
            return self._buf.extend_struct(fmt, value)

        def add_element(self, field, value, default):
            if value == default and not self._force_defaults:
                return

            off = push_element(self, value)
            self.track_field(field, off)

        return push_element, add_element

    # 8 bit
    push_byte, add_byte = _make_writers(numtypes.byte_fmt)
    push_ubyte, add_ubyte = _make_writers(numtypes.ubyte_fmt)

    # 16 bit
    push_short, add_short = _make_writers(numtypes.short_fmt)
    push_ushort, add_ushort = _make_writers(numtypes.ushort_fmt)

    # 32 bit
    push_int, add_int = _make_writers(numtypes.int_fmt)
    push_uint, add_uint = _make_writers(numtypes.uint_fmt)
    push_float, add_float = _make_writers(numtypes.float_fmt)
    push_soffset, add_soffset = _make_writers(numtypes.soffset_fmt)

    # 64 bit
    push_long, add_long = _make_writers(numtypes.long_fmt)
    push_ulong, add_ulong = _make_writers(numtypes.ulong_fmt)
    push_double, add_double = _make_writers(numtypes.double_fmt)

    def push_bool(self, value):
        assert(type(value) is bool)
        return self.push_ubyte(int(value))

    def add_bool(self, field, value, default):
        assert(type(value) is bool)
        return self.add_ubyte(field, int(value), int(default))

    def push_uoffset(self, value):
        return self.push_uint(self.refer_to(value))

    def add_uoffset(self, field, value):
        if value == 0:
            return
        return self.add_uint(field, self.refer_to(value), 0)

    def create_string(self, value):
        self.not_nested()
        raw = utf8_codec.encode(value)
        self.pre_align(len(raw) + 1, 1)
        self._buf.fill(1)
        self._buf.extend(raw)
        return self.push_uint(len(raw))

    def create_vector(self, value):
        self.not_nested()
        self._buf.extend_array(value)
        return self.push_uint(len(value))

    def create_vector_of_structs(self, tpe, value):
        self.not_nested()
        fmt = tpe._format
        for rec in reversed(value):
            self._buf.extend_struct(fmt, *rec)
        return self.push_uint(len(value))


class DownwardArray(object):
    """
    Bytearray-like class for buffer growing downwards.
    """

    _BLOCK_SIZE = 1024
    _PADDING = bytearray(_BLOCK_SIZE)

    def __init__(self, initial_size=_BLOCK_SIZE):
        self._buf = bytearray(initial_size)
        self._cur = 0

    def __len__(self):
        return self._cur

    def _extend_impl(self, b, size):
        self.make_space(size)
        newcur = self._cur + size
        self._buf[-newcur:-self._cur] = b
        self._cur = newcur
        return self._cur

    def extend(self, b):
        return self._extend_impl(b, len(b))

    def extend_array(self, arr):
        if not _LITTLE_ENDIAN:
            arr.byteswap()
        if hasattr(arr, 'tostring'):
            raw = arr.tostring()
            return self._extend_impl(raw, len(raw))
        return self._extend_impl(arr, len(arr) * arr.itemsize)

    def extend_struct(self, fmt, *values):
        self.make_space(fmt.size)
        newcur = self._cur + fmt.size
        fmt.pack_into(self._buf, -newcur, *values)
        self._cur = newcur
        return self._cur

    def fill(self, size):
        self.make_space(size)
        self._cur += size
        return self._cur

    def make_space(self, size):
        size += self._cur
        original_size = len(self._buf)
        if size <= original_size:
            return

        while size > 0:
            self._buf[len(self._buf):] = self._PADDING
            size -= self._BLOCK_SIZE

        self._buf[-original_size:] = self._buf[0:original_size]

    def data(self):
        return self._buf[-self._cur:]

    def clear(self):
        self._cur = 0
