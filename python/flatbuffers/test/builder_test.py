from nose.tools import assert_equal

import flatbuffers


def test_scalars():
    builder = flatbuffers.Builder()

    def check(offset, expected):
        assert_equal(expected, list(builder.data()))

    check(builder.prepend_byte(42), [0x2A])
    check(builder.prepend_byte(-1), [0xFF, 0x2A])
    check(builder.prepend_ubyte(255), [0xFF, 0xFF, 0x2A])
    check(builder.prepend_short(-1), [0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x2A])
