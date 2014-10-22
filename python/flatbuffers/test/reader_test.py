from nose.tools import assert_equal

import flatbuffers


def test_scalars():
    def check(expected, reader, raw):
        actual = reader(bytearray(raw), 0)
        assert_equal(expected, actual)

    # 8 bit
    check(42, flatbuffers.read_byte, [0x2A])
    check(-1, flatbuffers.read_byte, [0xFF])
    check(42, flatbuffers.read_ubyte, [0x2A])
    check(255, flatbuffers.read_ubyte, [0xFF])

    # 16 bit
    check(420, flatbuffers.read_short, [0xA4, 0x01])
    check(-1, flatbuffers.read_short, [0xFF, 0xFF])
    check(420, flatbuffers.read_ushort, [0xA4, 0x01])
    check(65535, flatbuffers.read_ushort, [0xFF, 0xFF])

    # 32 bit
    check(42000000, flatbuffers.read_int, [0x80, 0xDE, 0x80, 0x02])
    check(-1, flatbuffers.read_int, [0xFF, 0xFF, 0xFF, 0xFF])
    check(42000000, flatbuffers.read_uint, [0x80, 0xDE, 0x80, 0x02])
    check(4294967295, flatbuffers.read_uint, [0xFF, 0xFF, 0xFF, 0xFF])
    check(4.25, flatbuffers.read_float, [0x00, 0x00, 0x88, 0x40])

    # 64 bit
    check(420000000000000000, flatbuffers.read_long, [0x00, 0x00, 0xAA, 0x55, 0xC6, 0x23, 0xD4, 0x05])
    check(-1, flatbuffers.read_long, [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF])
    check(420000000000000000, flatbuffers.read_ulong, [0x00, 0x00, 0xAA, 0x55, 0xC6, 0x23, 0xD4, 0x05])
    check(18446744073709551615, flatbuffers.read_ulong, [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF])
    check(4.25, flatbuffers.read_double, [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x40])
