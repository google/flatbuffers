# Copyright 2025 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

require "my_game/example/monster"

class TestMonster < Test::Unit::TestCase
  def setup
    data_mon = File.join(__dir__, "..", "monsterdata_test.mon")
    data = File.open(data_mon, "rb", &:read)
    @monster = MyGame::Example::Monster.new(data)
  end

  def test_pos
    assert_equal(1.0, @monster.pos.x)
  end

  def test_hp
    assert_equal(80, @monster.hp)
  end

  def test_mana
    assert_equal(150, @monster.mana)
  end

  def test_name
    assert_equal("MyMonster", @monster.name)
  end

  def test_color
    assert_equal("Blue", @monster.color.name)
  end

  def test_inventory
    assert_equal([0, 1, 2, 3, 4], @monster.inventory)
  end

  def test_friendly?
    # Deprecated field isn't generated.
    assert do
      not @monster.respond_to?(:friendly?)
    end
  end

  def test_test_array_of_tables
    assert_nil(@monster.testarrayoftables)
  end

  def test_test_array_of_strings
    assert_equal(["test1", "test2"], @monster.testarrayofstring)
  end

  def test_test_array_of_strings2
    assert_nil(@monster.testarrayofstring_2)
  end

  def test_test_array_of_bools
    assert_equal([true, false, true], @monster.testarrayofbools)
  end

  def test_test_array_of_sorted_structs
    assert_equal([45, 21, 12],
                 @monster.testarrayofsortedstruct.collect(&:distance))
  end

  def test_enemy
    assert_equal("Fred", @monster.enemy.name)
  end

  def test_test_type
    assert_equal(MyGame::Example::Any::MONSTER, @monster.test_type)
  end

  def test_test
    assert_equal("Fred", @monster.test.name)
  end

  def test_test_4
    assert_equal([
                   {a: 10, b: 20},
                   {a: 30, b: 40},
                 ],
                 @monster.test_4.collect {|test| {a: test.a, b: test.b}})
  end

  def test_test_5
    assert_equal([
                   {a: 10, b: 20},
                   {a: 30, b: 40},
                 ],
                 @monster.test_5.collect {|test| {a: test.a, b: test.b}})
  end

  def test_test_nested_flatbuffer
    assert_nil(@monster.testnestedflatbuffer)
  end

  def test_test_empty
    assert_nil(@monster.testempty)
  end

  def test_test_bool?
    assert do
      @monster.testbool?
    end
  end

  def test_test_hash_s32_fnv1
    assert_equal(-579221183, @monster.testhashs_32_fnv_1)
  end

  def test_test_hash_u32_fnv1
    assert_equal(3715746113, @monster.testhashu_32_fnv_1)
  end

  def test_test_hash_s64_fnv1
    assert_equal(7930699090847568257, @monster.testhashs_64_fnv_1)
  end

  def test_test_hash_u64_fnv1
    assert_equal(7930699090847568257, @monster.testhashu_64_fnv_1)
  end

  def test_test_hash_s32_fnv1a
    assert_equal(-1904106383, @monster.testhashs_32_fnv_1a)
  end

  def test_test_hash_u32_fnv1a
    assert_equal(2390860913, @monster.testhashu_32_fnv_1a)
  end

  def test_test_hash_s64_fnv1a
    assert_equal(4898026182817603057, @monster.testhashs_64_fnv_1a)
  end

  def test_test_hash_u64_fnv1a
    assert_equal(4898026182817603057, @monster.testhashu_64_fnv_1a)
  end

  def test_test_f
    assert_equal(3.14159, @monster.testf)
  end

  def test_test_f2
    assert_equal(3.0, @monster.testf_2)
  end

  def test_test_f3
    assert_equal(0.0, @monster.testf_3)
  end

  def test_flex
    assert_nil(@monster.flex)
  end

  def test_vector_of_longs
    assert_equal([1, 100, 10000, 1000000, 100000000],
                 @monster.vector_of_longs)
  end

  def test_vector_of_double
    assert_equal([-1.7976931348623157e+308, 0.0, 1.7976931348623157e+308],
                 @monster.vector_of_doubles)
  end

  def test_parent_namespace_test
    assert_nil(@monster.parent_namespace_test)
  end

  def test_vector_of_referrables
    assert_nil(@monster.vector_of_referrables)
  end

  def test_single_weak_reference
    assert_equal(0, @monster.single_weak_reference)
  end

  def test_vector_of_weak_references
    assert_nil(@monster.vector_of_weak_references)
  end

  def test_vector_of_strong_referrables
    assert_nil(@monster.vector_of_strong_referrables)
  end

  def test_co_owning_reference
    assert_equal(0, @monster.co_owning_reference)
  end

  def test_vector_of_co_owning_references
    assert_nil(@monster.vector_of_co_owning_references)
  end

  def test_non_owning_reference
    assert_equal(0, @monster.non_owning_reference)
  end

  def test_vector_of_non_owning_references
    assert_nil(@monster.vector_of_non_owning_references)
  end

  def test_any_unique
    assert_nil(@monster.any_unique)
  end

  def test_any_ambiguous
    assert_nil(@monster.any_ambiguous)
  end

  def test_vector_of_enums
    assert_nil(@monster.vector_of_enums)
  end

  def test_signed_enum
    assert_equal(MyGame::Example::Race::NONE,
                 @monster.signed_enum)
  end

  def test_test_required_nested_flatbuffer
    assert_nil(@monster.testrequirednestedflatbuffer)
  end

  def test_scalar_key_sorted_tables
    stats = @monster.scalar_key_sorted_tables
    assert_equal([[0, "miss"], [10, "hit"]],
                 stats.collect {|stat| [stat.val, stat.id]})
  end

  def test_native_inline
    test = @monster.native_inline
    assert_equal([1, 2],
                 [test.a, test.b])
  end

  def test_long_enum_non_enum_default
    assert_equal(0, @monster.long_enum_non_enum_default)
  end

  def test_long_enum_normal_default
    assert_equal(MyGame::Example::LongEnum::LONG_ONE,
                 @monster.long_enum_normal_default)
  end

  def test_nan_default
    assert do
      @monster.nan_default.nan?
    end
  end

  def test_inf_default
    value = @monster.inf_default
    assert do
      value.positive? and value.infinite?
    end
  end

  def test_positive_inf_default
    value = @monster.positive_inf_default
    assert do
      value.positive? and value.infinite?
    end
  end

  def test_infinity_default
    value = @monster.infinity_default
    assert do
      value.positive? and value.infinite?
    end
  end

  def test_positive_infinity_default
    value = @monster.positive_infinity_default
    assert do
      value.positive? and value.infinite?
    end
  end

  def test_negative_inf_default
    value = @monster.negative_inf_default
    assert do
      !value.positive? and value.infinite?
    end
  end

  def test_negative_infinity_default
    value = @monster.negative_infinity_default
    assert do
      !value.positive? and value.infinite?
    end
  end

  def test_double_inf_default
    value = @monster.double_inf_default
    assert do
      value.positive? and value.infinite?
    end
  end

  class TestVec3 < self
    def setup
      super
      @vec3 = @monster.pos
    end

    def test_x
      assert_equal(1.0, @vec3.x)
    end

    def test_y
      assert_equal(2.0, @vec3.y)
    end

    def test_z
      assert_equal(3.0, @vec3.z)
    end

    def test_test_1
      assert_equal(3.0, @vec3.test_1)
    end

    def test_test_2
      assert_equal("Green", @vec3.test_2.name)
    end

    def test_test_3
      assert_equal(5, @vec3.test_3.a)
    end
  end

  class TestTest < self
    def setup
      super
      @test = @monster.pos.test_3
    end

    def test_a
      assert_equal(5, @test.a)
    end

    def test_b
      assert_equal(6, @test.b)
    end
  end
end
