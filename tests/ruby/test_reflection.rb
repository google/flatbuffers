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

require "reflection/schema"

class TestReflection < Test::Unit::TestCase
  def setup
    reflection_bfbs = File.join(Helper::Path.reflection_dir, "reflection.bfbs")
    data = File.open(reflection_bfbs, "rb", &:read)
    @schema = Reflection::Schema.new(data)
  end

  class TestSchema < self
    def test_objects
      assert_equal([
                     "reflection.Enum",
                     "reflection.EnumVal",
                     "reflection.Field",
                     "reflection.KeyValue",
                     "reflection.Object",
                     "reflection.RPCCall",
                     "reflection.Schema",
                     "reflection.SchemaFile",
                     "reflection.Service",
                     "reflection.Type",
                   ],
                   @schema.objects.collect(&:name))
    end

    def test_enums
      assert_equal([
                     "reflection.AdvancedFeatures",
                     "reflection.BaseType",
                   ],
                   @schema.enums.collect(&:name))
    end

    def test_file_ident
      assert_equal("BFBS", @schema.file_ident)
    end

    def test_file_ext
      assert_equal("bfbs", @schema.file_ext)
    end

    class TestRootTable < self
      def setup
        super
        @root_table = @schema.root_table
      end

      def test_name
        assert_equal("reflection.Schema", @root_table.name)
      end
    end

    def test_services
      assert_nil(@schema.services)
    end

    def test_advanced_features
      assert_equal(0,
                   @schema.advanced_features)
    end

    def test_fbs_files
      assert_equal(["//reflection.fbs"],
                   @schema.fbs_files.collect(&:filename))
    end
  end

  class TestObject < self
    def setup
      super
      @object = @schema.objects[0]
    end

    def test_name
      assert_equal("reflection.Enum", @object.name)
    end

    def test_fields
      assert_equal([
                     "attributes",
                     "declaration_file",
                     "documentation",
                     "is_union",
                     "name",
                     "underlying_type",
                     "values",
                   ],
                   @object.fields.collect(&:name))
    end

    def test_struct?
      assert do
        not @object.struct?
      end
    end

    def test_minalign
      assert_equal(1, @object.minalign)
    end

    def test_bytesize
      assert_equal(0, @object.bytesize)
    end

    def test_attributes
      assert_nil(@object.attributes)
    end

    def test_documentation
      assert_nil(@object.documentation)
    end

    def test_declaration_file
      assert_equal("//reflection.fbs", @object.declaration_file)
    end
  end

  class TestField < self
    def setup
      super
      @field = @schema.objects[0].fields[0]
    end

    def test_name
      assert_equal("attributes", @field.name)
    end

    def test_type
      assert_equal(Reflection::BaseType::VECTOR,
                   @field.type.base_type)
    end

    def test_id
      assert_equal(4, @field.id)
    end

    def test_offset
      assert_equal(12, @field.offset)
    end

    def test_default_integer
      assert_equal(0, @field.default_integer)
    end

    def test_default_real
      assert_equal(0.0, @field.default_real)
    end

    def test_deprecated?
      assert do
        not @field.deprecated?
      end
    end

    def test_required?
      assert do
        not @field.required?
      end
    end

    def test_key?
      assert do
        not @field.key?
      end
    end

    def test_attributes
      assert_nil(@field.attributes)
    end

    def test_documentation
      assert_nil(@field.documentation)
    end

    def test_optional?
      assert do
        @field.optional?
      end
    end

    def test_padding
      assert_equal(0, @field.padding)
    end

    def test_offset_64?
      assert do
        not @field.offset_64?
      end
    end
  end

  class TestType < self
    def setup
      super
      @type = @schema.objects[0].fields[0].type
    end

    def test_base_type
      assert_equal(Reflection::BaseType::VECTOR,
                   @type.base_type)
    end

    def test_element
      assert_equal(Reflection::BaseType::OBJ,
                   @type.element)
    end

    def test_index
      assert_equal(3, @type.index)
    end

    def test_fixed_length
      assert_equal(0, @type.fixed_length)
    end

    def test_base_size
      assert_equal(4, @type.base_size)
    end

    def test_element_size
      assert_equal(4, @type.element_size)
    end
  end

  class TestBaseType < self
    def setup
      super
      @base_type = @schema.objects[0].fields[0].type.base_type
    end

    def test_name
      assert_equal("Vector", @base_type.name)
    end

    def test_value
      assert_equal(14, @base_type.value)
    end

    def test_to_i
      assert_equal(14, @base_type.to_i)
    end

    def test_to_int
      assert_equal(14, Integer.try_convert(@base_type))
    end

    class TestTryConvert < self
      def test_symbol
        assert_equal(Reflection::BaseType::VECTOR,
                     Reflection::BaseType.try_convert(:Vector))
      end

      def test_string
        assert_equal(Reflection::BaseType::VECTOR,
                     Reflection::BaseType.try_convert("Vector"))
      end

      def test_integer
        assert_equal(Reflection::BaseType::VECTOR,
                     Reflection::BaseType.try_convert(14))
      end

      def test_base_type
        base_type = Reflection::BaseType::VECTOR
        assert_equal(base_type,
                     Reflection::BaseType.try_convert(base_type))
      end
    end
  end

  class TestEnum < self
    def setup
      super
      @enum = @schema.enums[0]
    end

    def test_name
      assert_equal("reflection.AdvancedFeatures", @enum.name)
    end

    def test_values
      assert_equal([
                     "AdvancedArrayFeatures",
                     "AdvancedUnionFeatures",
                     "OptionalScalars",
                     "DefaultVectorsAndStrings",
                   ],
                   @enum.values.collect(&:name))
    end

    def test_union?
      assert do
        not @enum.union?
      end
    end

    def test_underlying_type
      assert_equal(Reflection::BaseType::ULONG,
                   @enum.underlying_type.base_type)
    end

    def test_attributes
      assert_equal(["bit_flags"],
                   @enum.attributes.collect(&:key))
    end

    def test_documentation
      assert_nil(@enum.documentation)
    end

    def test_declaration_file
      assert_equal("//reflection.fbs", @enum.declaration_file)
    end
  end

  class TestKeyValue < self
    def setup
      super
      @key_value = @schema.enums[0].attributes[0]
    end

    def test_key
      assert_equal("bit_flags", @key_value.key)
    end

    def test_value
      assert_equal("0", @key_value.value)
    end
  end

  class TestEnumVal < self
    def setup
      super
      @enum_val = @schema.enums[0].values[0]
    end

    def test_name
      assert_equal("AdvancedArrayFeatures", @enum_val.name)
    end

    def test_value
      assert_equal(1, @enum_val.value)
    end

    def test_union_type
      assert_equal(Reflection::BaseType::NONE,
                   @enum_val.union_type.base_type)
    end

    def test_documentation
      assert_nil(@enum_val.documentation)
    end

    def test_attributes
      assert_nil(@enum_val.attributes)
    end
  end

  class TestSchemaFile < self
    def setup
      super
      @schema_file = @schema.fbs_files[0]
    end

    def test_filename
      assert_equal("//reflection.fbs", @schema_file.filename)
    end

    def test_included_filenames
      assert_nil(@schema_file.included_filenames)
    end
  end
end
