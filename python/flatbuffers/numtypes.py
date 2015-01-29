import struct

byte_t = struct.Struct('<b')
ubyte_t = struct.Struct('<B')

short_t = struct.Struct('<h')
ushort_t = struct.Struct('<H')

int_t = struct.Struct('<i')
uint_t = struct.Struct('<I')
float_t = struct.Struct('<f')

long_t = struct.Struct('<q')
ulong_t = struct.Struct('<Q')
double_t = struct.Struct('<d')

uoffset_t = uint_t
soffset_t = int_t
voffset_t = ushort_t

types = frozenset([byte_t, ubyte_t, short_t, ushort_t, int_t, uint_t, float_t,
                   long_t, ulong_t, double_t, uoffset_t, soffset_t, voffset_t])

largest_scalar_size = ulong_t.size
