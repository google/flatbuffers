import array
import collections

import six
from six.moves import range

from . import encode
from . import numtypes


def read_vtable(buf, offset):
    data_offset = offset - encode.read_soffset(buf, offset)
    size = encode.read_ushort(buf, data_offset)
    vtable = encode.read_array('H', buf, data_offset, size // 2)
    return vtable[2:]


class StructVector(collections.Sequence):
    def __init__(self, target, buf, offset):
        assert issubclass(target, Struct)
        self._target = target
        self._buf = buf

        data_offset = offset + encode.read_uoffset(buf, offset)
        self._size = encode.read_uint(buf, data_offset)
        self._offset = data_offset + numtypes.uint_t.size

    def __getitem__(self, index):
        if index >= self._size:
            raise IndexError('vector index out of range')
        data_offset = self._offset + index * self._target.size()
        return self._target(self._buf, data_offset)

    def __len__(self):
        return self._size


class IndirectVector(collections.Sequence):
    def __init__(self, target, buf, offset):
        self._target = target
        self._buf = buf

        self._offset = offset + numtypes.uoffset_t.size
        self._locs = encode.read_scalar_vector('I', buf, offset)

    def __getitem__(self, index):
        if index >= len(self._locs):
            raise IndexError('vector index out of range')
        index_offset = (index + 1) * numtypes.uoffset_t.size
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

    def __new__(cls, buf, offset):
        return tuple.__new__(cls, cls.format().unpack_from(buf, offset))

    @classmethod
    def format(cls):
        raise NotImplementedError()

    @classmethod
    def size(cls):
        return cls.format().size


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
        return bool(encode.read_ubyte(self._buf, self._offset + offset))

    def read_byte_field(self, field, default):
        return self.read_field(field, encode.read_byte, default)

    def read_ubyte_field(self, field, default):
        return self.read_field(field, encode.read_ubyte, default)

    def read_short_field(self, field, default):
        return self.read_field(field, encode.read_short, default)

    def read_ushort_field(self, field, default):
        return self.read_field(field, encode.read_ushort, default)

    def read_int_field(self, field, default):
        return self.read_field(field, encode.read_int, default)

    def read_uint_field(self, field, default):
        return self.read_field(field, encode.read_uint, default)

    def read_float_field(self, field, default):
        return self.read_field(field, encode.read_float, default)

    def read_long_field(self, field, default):
        return self.read_field(field, encode.read_long, default)

    def read_ulong_field(self, field, default):
        return self.read_field(field, encode.read_ulong, default)

    def read_double_field(self, field, default):
        return self.read_field(field, encode.read_double, default)


def _make_vtable(src, obj_size, offset):
    if src:
        vt_size = max(six.viewkeys(src)) + 1
    else:
        vt_size = 0
    yield (vt_size + 2) * numtypes.voffset_t.size
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
        self._buf = encode.DownwardArray(block_size)
        self._force_defaults = force_defaults
        self._start = None
        self._vtable = None
        self._vtables = {}

    def data(self):
        return self._buf.data()

    def track_field(self, field, offset):
        """
        When writing fields, we track where they are, so we can create correct
        vtables later.
        """
        if offset != 0:
            self._vtable[field] = self.refer_to(offset)

    def not_nested(self):
        assert self._start is None

    def start_table(self):
        """
        From generated code (or from the parser), we call start_table/end_table
        with a sequence of add_* calls in between.
        """
        self.not_nested()
        self._start = len(self._buf)
        self._vtable = {}
        return len(self._buf)

    def end_table(self):
        """
        This finishes one serialized object by generating the vtable if it's a
        table, comparing it against existing vtables, and writing the
        resulting vtable offset.
        """
        self._buf.align(numtypes.soffset_t.size)
        vt_off_loc = len(self._buf) + numtypes.soffset_t.size
        obj_size = vt_off_loc - self._start
        assert obj_size < 0x10000

        vtable_data = tuple(_make_vtable(self._vtable, obj_size, vt_off_loc))
        vtable = array.array('H', vtable_data)
        try:
            vt_offset = self._vtables[vtable_data]
            self.push_soffset(vt_offset - vt_off_loc)
        except KeyError:
            self.push_soffset(len(vtable) * vtable.itemsize)
            vt_offset = self._buf.push_array(vtable)
            self._vtables[vtable_data] = vt_offset
        self._vtable = None
        self._start = None
        return vt_off_loc

    def start_vector(self, tpe, size):
        assert tpe in numtypes.types or issubclass(tpe, Struct)
        scalar_size = max(tpe.size, numtypes.uoffset_t.size)
        self._buf.pre_align(tpe.size * size, scalar_size)

    def end_vector(self, size):
        self.push_uint(size)

    def push_bytes(self, value):
        assert type(value) is six.binary_type
        return self._buf.push_bytes(value)

    def push_string(self, value):
        if isinstance(value, six.text_type):
            value = value.encode('utf-8')

        assert isinstance(value, six.string_types)
        return self._buf.push_bytes(value)

    def push_bool(self, value):
        assert type(value) is bool
        return self.push_ubyte(int(value))

    def push_byte(self, value):
        return self._buf.push_struct(numtypes.byte_t, value)

    def push_ubyte(self, value):
        return self._buf.push_struct(numtypes.ubyte_t, value)

    def push_short(self, value):
        return self._buf.push_struct(numtypes.short_t, value)

    def push_ushort(self, value):
        return self._buf.push_struct(numtypes.ushort_t, value)

    def push_int(self, value):
        return self._buf.push_struct(numtypes.int_t, value)

    def push_uint(self, value):
        return self._buf.push_struct(numtypes.uint_t, value)

    def push_float(self, value):
        return self._buf.push_struct(numtypes.float_t, value)

    def push_long(self, value):
        return self._buf.push_struct(numtypes.long_t, value)

    def push_ulong(self, value):
        return self._buf.push_struct(numtypes.ulong_t, value)

    def push_double(self, value):
        return self._buf.push_struct(numtypes.double_t, value)

    def push_uoffset(self, value):
        return self._buf.push_struct(numtypes.uoffset_t, value)

    def push_soffset(self, value):
        return self._buf.push_struct(numtypes.soffset_t, value)

    def push_struct(self, tpe, *values):
        assert issubclass(tpe, Struct)
        return self._buf.push_struct(tpe.format(), *values)

    def fill(self, size):
        return self._buf.fill(size)

    def offset(self):
        return len(self._buf)

    def refer_to(self, offset):
        """
        Offsets initially are relative to the end of the buffer (downwards).
        This function converts them to be relative to the current location
        in the buffer (when stored here), pointing upwards.
        """
        self._buf.align(numtypes.uoffset_t.size)
        assert offset <= len(self._buf)
        return len(self._buf) - offset + numtypes.uoffset_t.size

    def _add_element(self, fn, field, value, default):
        assert field not in self._vtable
        if value == default and not self._force_defaults:
            return

        offset = fn(value)
        self._vtable[field] = offset

    def add_bool(self, field, value, default):
        assert type(value) is bool
        return self.add_ubyte(field, int(value), int(default))

    def add_byte(self, field, value, default):
        self._add_element(self.push_byte, field, value, default)

    def add_ubyte(self, field, value, default):
        self._add_element(self.push_ubyte, field, value, default)

    def add_short(self, field, value, default):
        self._add_element(self.push_short, field, value, default)

    def add_ushort(self, field, value, default):
        self._add_element(self.push_ushort, field, value, default)

    def add_int(self, field, value, default):
        self._add_element(self.push_int, field, value, default)

    def add_uint(self, field, value, default):
        self._add_element(self.push_uint, field, value, default)

    def add_float(self, field, value, default):
        self._add_element(self.push_byte, field, value, default)

    def add_long(self, field, value, default):
        self._add_element(self.push_long, field, value, default)

    def add_ulong(self, field, value, default):
        self._add_element(self.push_ulong, field, value, default)

    def add_double(self, field, value, default):
        self._add_element(self.push_byte, field, value, default)

    def add_offset(self, field, value):
        if value == 0:
            return

        off = self.refer_to(value)
        self._add_element(self.push_uoffset, field, off, 0)

    def create_string(self, value):
        self.not_nested()
        return self.push_string(value)

    def create_vector(self, value):
        assert type(value) is array.array
        self.not_nested()
        scalar_size = max(value.itemsize, numtypes.uoffset_t.size)
        self._buf.pre_align(len(value) * value.itemsize, scalar_size)
        self._buf.push_array(value)
        return self.push_uint(len(value))

    def create_vector_of_structs(self, tpe, value):
        assert issubclass(tpe, Struct)
        self.not_nested()
        scalar_size = max(tpe.size(), numtypes.uoffset_t.size)
        self._buf.pre_align(len(value) * tpe.size(), scalar_size)
        for rec in reversed(value):
            self._buf.push_struct(tpe.format(), *rec)
        return self.push_uint(len(value))
