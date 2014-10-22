from nose.tools import assert_equal, assert_is_none, assert_is_not_none

import sys
sys.path.append('../python')

from MyGame.Example import Color, Any, get_root_as_Monster


def lcg_rand(seed=48271):
    """
    Include simple random number generator to ensure results will be the same cross platform.
    http://en.wikipedia.org/wiki/Park%E2%80%93Miller_random_number_generator
    """
    x = seed
    while True:
        yield (x * 279470273) % 4294967291


def check_read_buffer(buf):
    monster = get_root_as_Monster(buf)

    assert_equal(80, monster.hp)
    assert_equal(150, monster.mana)
    assert_equal("MyMonster", monster.name)

    pos = monster.pos
    assert_is_not_none(pos)
    assert_equal(1.0, pos.x)
    assert_equal(2.0, pos.y)
    assert_equal(3.0, pos.z)
    assert_equal(3.0, pos.test1)
    assert_equal(Color.Green, pos.test2)
    assert_equal((5, 6), pos.test3)

    assert_is_not_none(monster.inventory)
    assert_equal(5, len(monster.inventory))
    assert_equal([0, 1, 2, 3, 4], list(monster.inventory))

    assert_equal(Any.Monster, monster.test_type)
    assert_is_not_none(monster.test)
    assert_equal("Fred", monster.test.name)

    assert_is_not_none(monster.test4)
    assert_equal(2, len(monster.test4))
    assert_equal([(10, 20), (30, 40)], monster.test4)

    assert_equal(["test1", "test2"], monster.testarrayofstring)

    assert_is_none(monster.testempty)


def test_cppdata():
    with open("monsterdata_test.mon", 'rb') as cppdata:
        buf = cppdata.read()
        check_read_buffer(buf)


def test_fuzz():
    bool_val = True
    byte_val = -127  # 0x81
    ubyte_val = 0xFF
    short_val = -32222  # 0x8222;
    ushort_val = 0xFEEE
    int_val = 0x83333333
    uint_val = 0xFDDDDDDD
    long_val = 0x8444444444444444
    ulong_val = 0xFCCCCCCCCCCCCCCC
    float_val = 3.14159
    double_val = 3.14159265359

    test_values_max = 11
    fields_per_object = 4
    num_fuzz_objects = 10000

import nose
nose.main()
