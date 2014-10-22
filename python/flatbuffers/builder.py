import sys
from struct import Struct
from array import array


_little_endian = (sys.byteorder == 'little')

if hasattr(array, 'tobytes'):
    _tobytes = lambda arr: arr.tobytes()
else:
    _tobytes = lambda arr: arr.tostring()


class Builder(object):
    def __init__(self, block_size=1024):
        self._block_size = block_size
        self._padding = bytearray(block_size)
        self._buffer = bytearray(block_size)
        self._offset = 0

    def data(self):
        return self._buffer[-self._offset:]

    def _pre_align(self, size, scalar_size):
        padding = (~(self._offset + size) + 1) & (scalar_size - 1)
        self._make_space(self._offset + size + padding)

        if padding != 0:
            self._buffer[-self._offset - padding:-self._offset] = self._padding[:padding]
            self._offset += padding

    def _make_space(self, size):
        original_size = len(self._buffer)
        if(size <= original_size):
            return

        while size > 0:
            self._buffer[len(self._buffer):] = self._padding
            size -= self._block_size

        self._buffer[-original_size:] = self._buffer[0:original_size]

    def _make_prependers(fmt):
        scalar_fmt = Struct('<{}'.format(fmt))

        def prepend_scalar(self, value):
            size = scalar_fmt.size
            self._pre_align(size, size)
            self._offset += size
            scalar_fmt.pack_into(self._buffer, -self._offset, value)
            return self._offset

        def prepend_scalar_vector(self, value):
            arr = array(fmt, value)
            if not _little_endian:
                arr.byteswap()

            raw = _tobytes(arr)
            self._pre_align(len(raw), arr.itemsize)

            old_offset = self._offset
            self._offset += len(raw)
            self._buffer[-self._offset:-old_offset] = raw
            return prepend_scalar(self, len(value))

        return prepend_scalar, prepend_scalar_vector

    # 8 bit
    prepend_byte, prepend_byte_vector = _make_prependers('b')
    prepend_ubyte, prepend_ubyte_vector = _make_prependers('B')

    # 16 bit
    prepend_short, prepend_short_vector = _make_prependers('h')
    prepend_ushort, prepend_ushort_vector = _make_prependers('H')

    # 32 bit
    prepend_int, prepend_int_vector = _make_prependers('i')
    prepend_uint, prepend_uint_vector = _make_prependers('I')
    prepend_float, prepend_float_vector = _make_prependers('f')

    # 64 bit
    prepend_long, prepend_long_vector = _make_prependers('q')
    prepend_ulong, prepend_ulong_vector = _make_prependers('Q')
    prepend_double, prepend_double_vector = _make_prependers('d')
