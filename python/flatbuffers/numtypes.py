import struct

byte_fmt = struct.Struct('<b')
ubyte_fmt = struct.Struct('<B')

short_fmt = struct.Struct('<h')
ushort_fmt = struct.Struct('<H')

int_fmt = struct.Struct('<i')
uint_fmt = struct.Struct('<I')
float_fmt = struct.Struct('<f')

long_fmt = struct.Struct('<q')
ulong_fmt = struct.Struct('<Q')
double_fmt = struct.Struct('<d')

largest_scalar_size = ulong_fmt.size

uoffset_fmt = uint_fmt
soffset_fmt = int_fmt
voffset_fmt = ushort_fmt
