import unittest

from flatbuffers import encode


class TestReader(unittest.TestCase):
    def test_scalars(self):
        def check(expected, reader, raw):
            actual = reader(bytearray(raw), 0)
            self.assertEqual(expected, actual)

        # 8 bit
        check(42, encode.read_byte, [0x2A])
        check(-1, encode.read_byte, [0xFF])
        check(42, encode.read_ubyte, [0x2A])
        check(255, encode.read_ubyte, [0xFF])

        # 16 bit
        check(420, encode.read_short, [0xA4, 0x01])
        check(-1, encode.read_short, [0xFF, 0xFF])
        check(420, encode.read_ushort, [0xA4, 0x01])
        check(65535, encode.read_ushort, [0xFF, 0xFF])

        # 32 bit
        check(42000000, encode.read_int, [0x80, 0xDE, 0x80, 0x02])
        check(-1, encode.read_int, [0xFF, 0xFF, 0xFF, 0xFF])
        check(42000000, encode.read_uint, [0x80, 0xDE, 0x80, 0x02])
        check(4294967295, encode.read_uint, [0xFF, 0xFF, 0xFF, 0xFF])
        check(4.25, encode.read_float, [0x00, 0x00, 0x88, 0x40])

        # 64 bit
        check(420000000000000000,
              encode.read_long,
              [0x00, 0x00, 0xAA, 0x55, 0xC6, 0x23, 0xD4, 0x05])
        check(-1,
              encode.read_long,
              [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF])
        check(420000000000000000,
              encode.read_ulong,
              [0x00, 0x00, 0xAA, 0x55, 0xC6, 0x23, 0xD4, 0x05])
        check(18446744073709551615,
              encode.read_ulong,
              [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF])
        check(4.25,
              encode.read_double,
              [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x40])
