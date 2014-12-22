import sys
from struct import Struct
from array import array


_little_endian = (sys.byteorder == 'little')

if hasattr(array, 'tobytes'):
    _tobytes = lambda arr: arr.tobytes()
else:
    _tobytes = lambda arr: arr.tostring()


def padding_bytes(size, scalar_size):
    return (~(size + 1) & (scalar_size - 1))


class Builder(object):
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
            self.fill(padding)

    def align(self, scalar_size):
        self._minalign = max(self._minalign, scalar_size)
        padding = padding_bytes(len(self._buf), scalar_size)
        if padding != 0:
            self._buf.fill(padding)

    def track_field(self, field, offset):
        assert(field not in self._vtable)
        self._vtable[field] = offset

    def not_nested(self):
        assert(not self._vtable)

    def start_table(self):
        self.not_nested()
        return len(self._buf)

    def end_table(self):
        # if self._vtable in self._vtables:
        pass

    def refer_to(self, offset):
        self.align(4)
        assert(offset <= len(self._buf))
        return len(self._buf) - offset + 4

    def _make_elements(fmt):
        scalar_fmt = Struct('<{}'.format(fmt))

        def push_element(self, value):
            self.align(scalar_fmt.size)
            return self._buf.extend_struct(scalar_fmt, value)

        def add_element(self, field, value, default):
            if field == default and not self._force_defaults:
                return

            off = push_element(value)
            self.track_field(field, off)

        return push_element, add_element

    # 8 bit
    push_byte, add_byte = _make_elements('b')
    push_ubyte, add_ubyte = _make_elements('B')

    # 16 bit
    push_short, add_short = _make_elements('h')
    push_ushort, add_ushort = _make_elements('H')

    # 32 bit
    push_int, add_int = _make_elements('i')
    push_uint, add_uint = _make_elements('I')
    push_float, add_float = _make_elements('f')

    # 64 bit
    push_long, add_long = _make_elements('q')
    push_ulong, add_ulong = _make_elements('Q')
    push_double, add_double = _make_elements('d')


class DownwardArray(object):
    def __init__(self, block_size=1024):
        self._block_size = block_size
        self._padding = bytearray(block_size)
        self._buf = bytearray(block_size)
        self._cur = 0

    def __len__(self):
        return self._cur

    def extend(self, b):
        self.make_space(len(b))
        newcur = self._cur + len(b)
        self.buf_[-newcur:-self._cur] = b
        self._cur = newcur
        return self._cur

    def extend_struct(self, fmt, value):
        self.make_space(fmt.size)
        newcur = self._cur + fmt.size
        fmt.pack_into(self._buf, -newcur, value)
        self._cur = newcur
        return self._cur

    def fill(self, size):
        self.make_space(size)
        self._cur += size
        return self._cur

    def make_space(self, size):
        size += self._cur
        original_size = len(self._buf)
        if(size <= original_size):
            return

        while size > 0:
            self._buf[len(self._buf):] = self._padding
            size -= self._block_size

        self._buf[-original_size:] = self._buf[0:original_size]

    def data(self):
        return self._buf[-self._cur:]
