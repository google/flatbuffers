import array
import unittest

import flatbuffers


class TestByteLayout(unittest.TestCase):
    def test_numbers(self):
        b = flatbuffers.Builder()
        b.push_bool(True)
        self.assertBuilderEquals(b, [0x01])
        b.push_byte(-127)
        self.assertBuilderEquals(b, [0x81, 0x01])
        b.push_ubyte(255)
        self.assertBuilderEquals(b, [0xFF, 0x81, 0x01])
        b.push_short(-32222)
        self.assertBuilderEquals(b, [0x22, 0x82, 0x00, 0xFF, 0x81, 0x01])
        b.push_ushort(65262)
        self.assertBuilderEquals(b, [0xEE, 0xFE, 0x22, 0x82, 0x00, 0xFF, 0x81, 0x01])
        b.push_int(-53687092)
        self.assertBuilderEquals(b, [0xCC, 0xCC, 0xCC, 0xFC, 0xEE, 0xFE, 0x22, 0x82, 0x00, 0xFF, 0x81, 0x01])
        b.push_uint(2557891634)
        self.assertBuilderEquals(b, [0x32, 0x54, 0x76, 0x98, 0xCC, 0xCC, 0xCC, 0xFC, 0xEE, 0xFE, 0x22, 0x82, 0x00, 0xFF, 0x81, 0x01])

    def test_numbers64(self):
        b = flatbuffers.Builder()
        b.push_ulong(1234605616436508552)
        self.assertBuilderEquals(b, [0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11])

        b = flatbuffers.Builder()
        b.push_long(-8613303245920329199)
        self.assertBuilderEquals(b, [0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88])

    def test_1xbyte_vector(self):
        b = flatbuffers.Builder()
        self.assertBuilderEquals(b, [])
        b.create_vector(array.array('b', [1]))
        self.assertBuilderEquals(b, [1, 0, 0, 0, 1, 0, 0, 0])

    def test_2xbyte_vector(self):
        b = flatbuffers.Builder()
        b.start_vector(flatbuffers.numtypes.byte_t, 2)
        self.assertBuilderEquals(b, [0, 0])  # align to 4bytes
        b.push_byte(1)
        self.assertBuilderEquals(b, [1, 0, 0])
        b.push_byte(2)
        self.assertBuilderEquals(b, [2, 1, 0, 0])
        b.end_vector(2)
        self.assertBuilderEquals(b, [2, 0, 0, 0, 2, 1, 0, 0])  # padding

    def test_1xuint16_vector(self):
        b = flatbuffers.Builder()
        b.start_vector(flatbuffers.numtypes.ushort_t, 1)
        self.assertBuilderEquals(b, [0, 0])  # align to 4bytes
        b.push_ushort(1)
        self.assertBuilderEquals(b, [1, 0, 0, 0])
        b.end_vector(1)
        self.assertBuilderEquals(b, [1, 0, 0, 0, 1, 0, 0, 0])  # padding

    def test_2xuint16_vector(self):
        b = flatbuffers.Builder(0)
        b.start_vector(flatbuffers.numtypes.ushort_t, 2)
        self.assertBuilderEquals(b, [])  # align to 4bytes
        b.push_ushort(0xABCD)
        self.assertBuilderEquals(b, [0xCD, 0xAB])
        b.push_ushort(0xDCBA)
        self.assertBuilderEquals(b, [0xBA, 0xDC, 0xCD, 0xAB])
        b.end_vector(2)
        self.assertBuilderEquals(b, [2, 0, 0, 0, 0xBA, 0xDC, 0xCD, 0xAB])

    def test_create_string(self):
        b = flatbuffers.Builder(0)
        b.create_string("foo")
        self.assertBuilderEquals(b, [3, 0, 0, 0, 'f', 'o', 'o', 0]) # 0-terminated, no pad
        b.create_string("moop")
        self.assertBuilderEquals(b, [4, 0, 0, 0, 'm', 'o', 'o', 'p', 0, 0, 0, 0,  # 0-terminated, 3-byte pad
                                     3, 0, 0, 0, 'f', 'o', 'o', 0])

    def test_empty_vtable(self):
        b = flatbuffers.Builder(0)
        b.start_table()
        self.assertBuilderEquals(b, [])
        b.end_table()
        self.assertBuilderEquals(b, [4, 0, 4, 0, 4, 0, 0, 0])

    def test_vtable_with_one_true_bool(self):
        b = flatbuffers.Builder(0)
        self.assertBuilderEquals(b, [])
        b.start_table()
        self.assertBuilderEquals(b, [])
        b.add_bool(0, True, False)
        b.end_table()
        self.assertBuilderEquals(b, [
            6, 0,  # vtable bytes
            8, 0,  # length of object including vtable offset
            7, 0,  # start of bool value
            6, 0, 0, 0,  # offset for start of vtable (int32)
            0, 0, 0,  # padded to 4 bytes
            1,  # bool value
        ])

    def test_vtable_with_one_default_bool(self):
        b = flatbuffers.Builder(0)
        self.assertBuilderEquals(b, [])
        b.start_table()
        self.assertBuilderEquals(b, [])
        b.add_bool(0, False, False)
        b.end_table()
        self.assertBuilderEquals(b, [
            6, 0,  # vtable bytes
            4, 0,  # end of object from here
            0, 0,  # entry 1 is zero
            6, 0, 0, 0,  # offset for start of vtable (int32)
        ])

    def test_vtable_with_one_int16(self):
        b = flatbuffers.Builder(0)
        b.start_table()
        b.add_short(0, 0x789A, 0)
        b.end_table()
        self.assertBuilderEquals(b, [
            6, 0,  # vtable bytes
            8, 0,  # end of object from here
            6, 0,  # offset to value
            6, 0, 0, 0,  # offset for start of vtable (int32)
            0, 0,  # padding to 4 bytes
            0x9A, 0x78,
        ])

    def test_vtable_with_two_int16(self):
        b = flatbuffers.Builder(0)
        b.start_table()
        b.add_short(0, 0x3456, 0)
        b.add_short(1, 0x789A, 0)
        b.end_table()
        self.assertBuilderEquals(b, [
            8, 0,  # vtable bytes
            8, 0,  # end of object from here
            6, 0,  # offset to value 0
            4, 0,  # offset to value 1
            8, 0, 0, 0,  # offset for start of vtable (int32)
            0x9A, 0x78,  # value 1
            0x56, 0x34,  # value 0
        ])

    def test_vtable_with_int16_and_bool(self):
        b = flatbuffers.Builder(0)
        b.start_table()
        b.add_short(0, 0x3456, 0)
        b.add_bool(1, True, False)
        b.end_table()
        self.assertBuilderEquals(b, [
            8, 0,  # vtable bytes
            8, 0,  # end of object from here
            6, 0,  # offset to value 0
            5, 0,  # offset to value 1
            8, 0, 0, 0,  # offset for start of vtable (int32)
            0,          # padding
            1,          # value 1
            0x56, 0x34,  # value 0
        ])

    def test_vtable_with_empty_vector(self):
        b = flatbuffers.Builder(0)
        b.start_vector(flatbuffers.numtypes.byte_t, 0)
        vecend = b.end_vector(0)
        b.start_table()
        b.add_uoffset(0, vecend, 0)
        b.end_table()
        self.assertBuilderEquals(b, [
            6, 0,  # vtable bytes
            8, 0,
            4, 0,  # offset to vector offset
            6, 0, 0, 0,  # offset for start of vtable (int32)
            4, 0, 0, 0,
            0, 0, 0, 0,  # length of vector (not in struct)
        ])

    def test_vtable_with_empty_vector_of_byte_and_some_scalars(self):
        b = flatbuffers.Builder(0)
        b.start_vector(flatbuffers.numtypes.byte_t, 0)
        vecend = b.end_vector(0)
        b.start_table()
        b.add_short(0, 55, 0)
        b.add_uoffset(1, vecend, 0)
        b.end_table()
        self.assertBuilderEquals(b, [
            8, 0,  # vtable bytes
            12, 0,
            10, 0,  # offset to value 0
            4, 0,  # offset to vector offset
            8, 0, 0, 0,  # vtable loc
            8, 0, 0, 0,  # value 1
            0, 0, 55, 0,  # value 0

            0, 0, 0, 0,  # length of vector (not in struct)
        ])

    def test_vtable_with_1_int16_and_2vector_of_int16(self):
        b = flatbuffers.Builder(0)
        b.start_vector(flatbuffers.numtypes.short_t, 2)
        b.push_short(0x1234)
        b.push_short(0x5678)
        vecend = b.end_vector(2)
        b.start_table()
        b.add_uoffset(1, vecend, 0)
        b.add_short(0, 55, 0)
        b.end_table()
        self.assertBuilderEquals(b, [
            8, 0,  # vtable bytes
            12, 0,  # length of object
            6, 0,  # start of value 0 from end of vtable
            8, 0,  # start of value 1 from end of buffer
            8, 0, 0, 0,  # offset for start of vtable (int32)
            0, 0,  # padding
            55, 0,  # value 0
            4, 0, 0, 0,  # vector position from here
            2, 0, 0, 0,  # length of vector (uint32)
            0x78, 0x56,  # vector value 1
            0x34, 0x12,  # vector value 0
        ])

    def test_vtable_with_1_struct_of_1_int8__1_int16__1_int32(self):
        b = flatbuffers.Builder(0)
        b.start_table()
        b.push_byte(55)
        b.fill(3)
        b.push_short(0x1234)
        b.fill(2)
        b.push_int(0x12345678)
        b.add_offset(0, b.offset())
        b.end_table()
        self.assertBuilderEquals(b, [
            6, 0,  # vtable bytes
            16, 0,  # end of object from here
            4, 0,  # start of struct from here
            6, 0, 0, 0,  # offset for start of vtable (int32)
            0x78, 0x56, 0x34, 0x12,  # value 2
            0, 0,  # padding
            0x34, 0x12,  # value 1
            0, 0, 0,  # padding
            55,  # value 0
        ])

    def test_vtable_with_1_vector_of_2_struct_of_2_int8(self):
        b = flatbuffers.Builder(0)
        b.start_vector(struct.struct('<bb'), 2)
        b.push_byte(33)
        b.push_byte(44)
        b.push_byte(55)
        b.push_byte(66)
        vecend = b.end_vector(2)
        b.start_table()
        b.add_uoffset(0, vecend)
        b.end_table()
        self.assertBuilderEquals(b, [
            6, 0,  # vtable bytes
            8, 0,
            4, 0,  # offset of vector offset
            6, 0, 0, 0,  # offset for start of vtable (int32)
            4, 0, 0, 0,  # vector start offset

            2, 0, 0, 0,  # vector length
            66,  # vector value 1,1
            55,  # vector value 1,0
            44,  # vector value 0,1
            33,  # vector value 0,0
        ])

    def test_table_with_some_elements(self):
        b = flatbuffers.Builder(0)
        b.start_table()
        b.add_byte(0, 33, 0)
        b.add_short(1, 66, 0)
        off = b.end_table()
        b.finish(off)

        self.assertBuilderEquals(b, [
            12, 0, 0, 0,  # root of table: points to vtable offset

            8, 0,  # vtable bytes
            8, 0,  # end of object from here
            7, 0,  # start of value 0
            4, 0,  # start of value 1

            8, 0, 0, 0,  # offset for start of vtable (int32)

            66, 0,  # value 1
            0,  # padding
            33,  # value 0
        ])

    def test__one_unfinished_table_and_one_finished_table(self):
        b = flatbuffers.Builder(0)
        b.start_table()
        b.add_byte(0, 33, 0)
        b.add_byte(1, 44, 0)
        off = b.end_table()
        b.finish(off)

        b.start_table(3)
        b.add_byte(0, 55, 0)
        b.add_byte(1, 66, 0)
        b.add_byte(2, 77, 0)
        off = b.end_table()
        b.finish(off)

        self.assertBuilderEquals(b, [
            16, 0, 0, 0,  # root of table: points to object
            0, 0,  # padding

            10, 0,  # vtable bytes
            8, 0,  # size of object
            7, 0,  # start of value 0
            6, 0,  # start of value 1
            5, 0,  # start of value 2
            10, 0, 0, 0,  # offset for start of vtable (int32)
            0,  # padding
            77,  # value 2
            66,  # value 1
            55,  # value 0

            12, 0, 0, 0,  # root of table: points to object

            8, 0,  # vtable bytes
            8, 0,  # size of object
            7, 0,  # start of value 0
            6, 0,  # start of value 1
            8, 0, 0, 0,  # offset for start of vtable (int32)
            0, 0,  # padding
            44,  # value 1
            33,  # value 0
        ])

    def test_a_bunch_of_bools(self):
        b = flatbuffers.Builder(0)
        b.start_table()
        b.add_bool(0, True, False)
        b.add_bool(1, True, False)
        b.add_bool(2, True, False)
        b.add_bool(3, True, False)
        b.add_bool(4, True, False)
        b.add_bool(5, True, False)
        b.add_bool(6, True, False)
        b.add_bool(7, True, False)
        off = b.end_table()
        b.finish(off)

        self.assertBuilderEquals(b, [
            24, 0, 0, 0,  # root of table: points to vtable offset

            20, 0,  # vtable bytes
            12, 0,  # size of object
            11, 0,  # start of value 0
            10, 0,  # start of value 1
            9, 0,  # start of value 2
            8, 0,  # start of value 3
            7, 0,  # start of value 4
            6, 0,  # start of value 5
            5, 0,  # start of value 6
            4, 0,  # start of value 7
            20, 0, 0, 0,  # vtable offset

            1,  # value 7
            1,  # value 6
            1,  # value 5
            1,  # value 4
            1,  # value 3
            1,  # value 2
            1,  # value 1
            1,  # value 0
        ])

    def test_three_bools(self):
        b = flatbuffers.Builder(0)
        b.start_table()
        b.add_bool(0, True, False)
        b.add_bool(1, True, False)
        b.add_bool(2, True, False)
        off = b.end_table()
        b.finish(off)

        self.assertBuilderEquals(b, [
            16, 0, 0, 0,  # root of table: points to vtable offset

            0, 0,  # padding

            10, 0,  # vtable bytes
            8, 0,  # size of object
            7, 0,  # start of value 0
            6, 0,  # start of value 1
            5, 0,  # start of value 2
            10, 0, 0, 0,  # vtable offset from here

            0,  # padding
            1,  # value 2
            1,  # value 1
            1,  # value 0
        ])

    def test_some_floats(self):
        b = flatbuffers.Builder(0)
        start = b.start_table()
        b.add_float(0, 1.0, 0.0)
        b.end_table(start)

        self.assertBuilderEquals(b, [
            6, 0,  # vtable bytes
            8, 0,  # size of object
            4, 0,  # start of value 0
            6, 0, 0, 0,  # vtable offset

            0, 0, 128, 63,  # value 0
        ])

    def assertBuilderEquals(self, arr, expected):
        self.assertEquals(list(arr.data()), expected)
