import array
import codecs
import sys

from . import numtypes

_LITTLE_ENDIAN = (sys.byteorder == 'little')


def read_byte(buf, offset):
    return numtypes.byte_t.unpack_from(buf, offset)[0]


def read_ubyte(buf, offset):
    return numtypes.ubyte_t.unpack_from(buf, offset)[0]


def read_short(buf, offset):
    return numtypes.short_t.unpack_from(buf, offset)[0]


def read_ushort(buf, offset):
    return numtypes.ushort_t.unpack_from(buf, offset)[0]


def read_int(buf, offset):
    return numtypes.int_t.unpack_from(buf, offset)[0]


def read_uint(buf, offset):
    return numtypes.uint_t.unpack_from(buf, offset)[0]


def read_float(buf, offset):
    return numtypes.float_t.unpack_from(buf, offset)[0]


def read_long(buf, offset):
    return numtypes.long_t.unpack_from(buf, offset)[0]


def read_ulong(buf, offset):
    return numtypes.ulong_t.unpack_from(buf, offset)[0]


def read_double(buf, offset):
    return numtypes.double_t.unpack_from(buf, offset)[0]


def read_uoffset(buf, offset):
    return numtypes.uoffset_t.unpack_from(buf, offset)[0]


def read_soffset(buf, offset):
    return numtypes.soffset_t.unpack_from(buf, offset)[0]


utf8_codec = codecs.lookup('utf-8')


def read_string(buf, offset):
    size = read_uint(buf, offset)
    data_offset = offset + numtypes.uoffset_t.size
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


def padding_bytes(size, scalar_size):
    return ((~size) + 1) & (scalar_size - 1)


class DownwardArray(object):
    """
    Bytearray-like class for buffer growing downwards.
    """

    _BLOCK_SIZE = 1024
    _PADDING = bytearray(_BLOCK_SIZE)
    _EMPTY = bytearray()

    def __init__(self, initial_size=_BLOCK_SIZE):
        self._minalign = 1
        self._buf = bytearray(initial_size)
        self._cur = 0

    def __len__(self):
        return self._cur

    def _push_impl(self, value, size):
        self.make_space(size)
        newcur = self._cur + size
        self._buf[-newcur:-self._cur] = value
        self._cur = newcur
        return self._cur

    def push_bytes(self, value):
        return self._push_impl(self, len(value))

    def push_array(self, arr):
        if not _LITTLE_ENDIAN:
            arr.byteswap()
        if hasattr(arr, 'tostring'):
            raw = arr.tostring()
            return self._push_impl(raw, len(raw))
        return self._push_impl(arr, len(arr) * arr.itemsize)

    def push_struct(self, fmt, *values):
        self.align(fmt.size)
        self.make_space(fmt.size)
        newcur = self._cur + fmt.size
        fmt.pack_into(self._buf, -newcur, *values)
        self._cur = newcur
        return self._cur

    def fill(self, size):
        self.make_space(size)
        self._cur += size
        return self._cur

    def pre_align(self, size, scalar_size):
        padding = padding_bytes(self._cur + size, scalar_size)
        if padding != 0:
            self.fill(padding)

    def align(self, scalar_size):
        self._minalign = max(self._minalign, scalar_size)
        padding = padding_bytes(self._cur, scalar_size)
        if padding != 0:
            self.fill(padding)

    def make_space(self, size):
        size += self._cur
        original_size = len(self._buf)
        if size <= original_size:
            return

        while size > 0:
            self._buf[len(self._buf):] = self._PADDING
            size -= self._BLOCK_SIZE

        if original_size != 0:
            self._buf[-original_size:] = self._buf[0:original_size]

    def data(self):
        if self._cur == 0:
            return self._EMPTY
        return self._buf[-self._cur:]

    def clear(self):
        self._cur = 0
