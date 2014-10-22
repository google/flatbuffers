import sys
import codecs
import collections
import struct
import array
import functools


_little_endian = (sys.byteorder == 'little')

if hasattr(array, 'frombytes'):
    _frombytes = lambda arr, view: arr.frombytes(view)
else:
    _frombytes = lambda arr, view: arr.fromstring(view.tobytes())


def ensure_view(source):
    return source if type(source) is memoryview else memoryview(source)


def read_array(arr, view):
    assert type(view) is memoryview
    _frombytes(arr, view)
    if not _little_endian:
        arr.byteswap()
    return arr


def make_readers(fmt):
    scalar_fmt = struct.Struct('<{}'.format(fmt))

    def read_scalar(view, offset):
        return scalar_fmt.unpack_from(view, offset)[0]

    def read_vector(view, offset):
        size = read_uint(view, offset)
        arr = array.array(fmt)
        return read_array(arr, view[offset + 4:offset + 4 + (size * arr.itemsize)])

    return read_scalar, read_vector

# 8 bit
read_byte, read_byte_vector = make_readers('b')
read_ubyte, read_ubyte_vector = make_readers('B')

# 16 bit
read_short, read_short_vector = make_readers('h')
read_ushort, read_ushort_vector = make_readers('H')

# 32 bit
read_int, read_int_vector = make_readers('i')
read_uint, read_uint_vector = make_readers('I')
read_float, read_float_vector = make_readers('f')

# 64 bit
read_long, read_long_vector = make_readers('q')
read_ulong, read_ulong_vector = make_readers('Q')
read_double, read_double_vector = make_readers('d')


def make_fixed_vector(fmt, size):
    def impl(view, offset):
        res = array.array(fmt)
        read_array(res, view, offset, size * res.itemsize)
        return res


def read_vtable(view, offset):
    size = (read_ushort(view, offset) / 2) - 2
    return make_fixed_vector('H', size)(view, offset)


decode_utf8 = codecs.lookup('utf-8').decode


def read_unicode(view, offset):
    size = read_uint(view, offset)
    return decode_utf8(view[offset + 4:offset + 4 + size])[0]


def _offset(vtable, index):
    return vtable[index] if index < len(vtable) else 0


def indirect(target):
    def impl(source, offset):
        return target(source, offset + read_uint(source, offset))
    return impl


def vector(target, itemsize=4):
    return functools.partial(Vector, target, itemsize)


class Vector(tuple, collections.Sequence):
    def __new__(cls, target, itemsize, source, offset):
        view = ensure_view(source)
        size = read_uint(view, offset)
        return tuple.__new__(cls, (target, view, offset + 4, size, itemsize))

    def __eq__(self, other):
        target, view, base, size, itemsize = self
        if size != len(other):
            return False

        for e, off in zip(other, range(0, size * itemsize, itemsize)):
            if target(view, base + off) != e:
                return False

        return True

    def __getitem__(self, index):
        target, view, base, size, itemsize = self
        if index >= size:
            raise IndexError('vector index out of range')
        return target(view, base + index * itemsize)

    def __len__(self):
        _, _, _, size, _ = self
        return size


class Struct(tuple):
    pass


class Table(tuple):
    def __new__(cls, source, offset):
        view = ensure_view(source)
        vt_base = offset - read_uint(view, offset)
        vt_size = read_ushort(view, vt_base) * 2
        vtable = read_array(array.array('H'), view[vt_base + 4:vt_base + vt_size])
        return tuple.__new__(cls, (view, offset, vtable))

    def _read_field(self, index, target, default=None):
        view, base, vtable = self
        offset = _offset(vtable, index)
        return target(view, base + offset) if offset != 0 else default
