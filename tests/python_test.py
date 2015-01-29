import sys
sys.path.append('../python')

import unittest

import flatbuffers
from MyGame.Example import Color, Any, get_root_as_Monster


def lcg_rand(seed=48271):
    """
    Include simple random number generator to ensure results will be the same cross platform.
    http://en.wikipedia.org/wiki/Park%E2%80%93Miller_random_number_generator
    """
    x = seed
    while True:
        x = (x * 279470273) % 4294967291
        yield x


class TestFlatbuffers(unittest.TestCase):
    def check_read_buffer(self, buf):
        monster = get_root_as_Monster(buf)

        self.assertEqual(80, monster.get_hp())
        self.assertEqual(150, monster.get_mana())
        self.assertEqual("MyMonster", monster.get_name())

        pos = monster.get_pos()
        self.assertIsNotNone(pos)
        self.assertEqual(1.0, pos.get_x())
        self.assertEqual(2.0, pos.get_y())
        self.assertEqual(3.0, pos.get_z())
        self.assertEqual(3.0, pos.get_test1())
        self.assertEqual(Color.Green, pos.get_test2())
        self.assertEqual((5, 6), pos.get_test3())

        self.assertIsNotNone(monster.get_inventory())
        self.assertEqual(5, len(monster.get_inventory()))
        self.assertEqual([0, 1, 2, 3, 4], list(monster.get_inventory()))

        self.assertEqual(Any.Monster, monster.get_test_type())
        self.assertIsNotNone(monster.get_test())
        self.assertEqual("Fred", monster.get_test().get_name())

        self.assertIsNotNone(monster.get_test4())
        self.assertEqual(2, len(monster.get_test4()))
        for expected, actual in zip([(10, 20), (30, 40)], monster.get_test4()):
            self.assertEqual(expected, actual)

        arrayofstring = monster.get_testarrayofstring()
        for expected, actual in zip(["test1", "test2"], arrayofstring):
            self.assertEqual(expected, actual)

        self.assertIsNone(monster.get_testempty())

    def test_cppdata(self):
        with open("monsterdata_test.mon", 'rb') as cppdata:
            buf = cppdata.read()
            self.check_read_buffer(buf)

    def test_fuzz(self):
        bool_val = True
        byte_val = -127  # 0x81
        ubyte_val = 0xFF
        short_val = -32222  # 0x8222;
        ushort_val = 0xFEEE
        int_val = -2093796557  # 0x83333333
        uint_val = 0xFDDDDDDD
        long_val = -8915926302292949948  # 0x8444444444444444
        ulong_val = 0xFCCCCCCCCCCCCCCC
        float_val = 3.14159
        double_val = 3.14159265359

        test_values_max = 11
        fields_per_object = 4
        num_fuzz_objects = 10000

        objects = []
        builder = flatbuffers.Builder()
        lcg = lcg_rand()

        for _ in range(num_fuzz_objects):
            builder.start_table()
            for f in range(fields_per_object):
                choice = next(lcg) % test_values_max
                if choice == 0:
                    builder.add_bool(f, bool_val, False)
                elif choice == 1:
                    builder.add_byte(f, byte_val, 0)
                elif choice == 2:
                    builder.add_ubyte(f, ubyte_val, 0)
                elif choice == 3:
                    builder.add_short(f, short_val, 0)
                elif choice == 4:
                    builder.add_ushort(f, ushort_val, 0)
                elif choice == 5:
                    builder.add_int(f, int_val, 0)
                elif choice == 6:
                    builder.add_uint(f, uint_val, 0)
                elif choice == 7:
                    builder.add_long(f, long_val, 0)
                elif choice == 8:
                    builder.add_ulong(f, ulong_val, 0)
                elif choice == 9:
                    builder.add_float(f, float_val, 0.0)
                elif choice == 10:
                    builder.add_double(f, double_val, 0.0)
            objects.append(builder.end_table())

        buf = memoryview(builder.data())
        lcg = lcg_rand()

        for i, off in enumerate(objects):
            table = flatbuffers.Table(buf, len(buf) - off)

            for f in range(fields_per_object):
                choice = next(lcg) % test_values_max
                if choice == 0:
                    self.assertEqual(bool_val, table.read_bool_field(f, False))
                elif choice == 1:
                    self.assertEqual(byte_val, table.read_byte_field(f, 0))
                elif choice == 2:
                    self.assertEqual(ubyte_val, table.read_ubyte_field(f, 0))
                elif choice == 3:
                    self.assertEqual(short_val, table.read_short_field(f, 0))
                elif choice == 4:
                    self.assertEqual(ushort_val, table.read_ushort_field(f, 0))
                elif choice == 5:
                    self.assertEqual(int_val, table.read_int_field(f, 0))
                elif choice == 6:
                    self.assertEqual(uint_val, table.read_uint_field(f, 0))
                elif choice == 7:
                    self.assertEqual(long_val, table.read_long_field(f, 0))
                elif choice == 8:
                    self.assertEqual(ulong_val, table.read_ulong_field(f, 0))
                elif choice == 9:
                    self.assertAlmostEqual(float_val, table.read_float_field(f, 0.0), places=5)
                elif choice == 10:
                    self.assertEqual(double_val, table.read_double_field(f, 0.0))

if __name__ == "__main__":
    unittest.main()
