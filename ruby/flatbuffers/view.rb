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

module FlatBuffers
  class View
    OFFSET_BYTE_SIZE = 4
    VIRTUAL_OFFSET_BYTE_SIZE = 2

    def initialize(data, offset, have_vtable: false)
      @data = data
      @offset = offset
      if have_vtable
        vtable_offset = unpack_signed_offset(0)
        @vtable_start = @offset - vtable_offset
        # We assume vtable must have length.
        @vtable_max_offset = VIRTUAL_OFFSET_BYTE_SIZE
        @vtable_length = unpack_virtual_offset(0)
        @vtable_max_offset = @vtable_length - VIRTUAL_OFFSET_BYTE_SIZE
        @table_length = unpack_virtual_offset(VIRTUAL_OFFSET_BYTE_SIZE)
      end
    end

    def unpack_virtual_offset(vtable_offset)
      return 0 if vtable_offset > @vtable_max_offset
      @data.get_value(:u16, @vtable_start + vtable_offset)
    end

    def resolve_indirect(offset)
      @offset + offset + unpack_offset(offset)
    end

    def unpack_offset_raw(offset)
      @data.get_value(:u32, offset)
    end

    def unpack_offset(offset)
      unpack_offset_raw(@offset + offset)
    end

    def unpack_signed_offset(offset)
      @data.get_value(:s32, @offset + offset)
    end

    def unpack_bool(offset)
      unpack_byte(offset) != 0
    end

    def unpack_utype(offset)
      unpack_ubyte(offset)
    end

    def unpack_byte(offset)
      @data.get_value(:S8, @offset + offset)
    end

    def unpack_ubyte(offset)
      @data.get_value(:U8, @offset + offset)
    end

    def unpack_short(offset)
      @data.get_value(:s16, @offset + offset)
    end

    def unpack_ushort(offset)
      @data.get_value(:u16, @offset + offset)
    end

    def unpack_int(offset)
      @data.get_value(:s32, @offset + offset)
    end

    def unpack_uint(offset)
      @data.get_value(:u32, @offset + offset)
    end

    def unpack_long(offset)
      @data.get_value(:s64, @offset + offset)
    end

    def unpack_ulong(offset)
      @data.get_value(:u64, @offset + offset)
    end

    def unpack_float(offset)
      @data.get_value(:f32, @offset + offset)
    end

    def unpack_double(offset)
      @data.get_value(:f64, @offset + offset)
    end

    def unpack_string(offset)
      value_offset = resolve_indirect(offset)
      length = unpack_offset_raw(value_offset)
      @data.get_string(value_offset + OFFSET_BYTE_SIZE,
                       length)
    end

    def unpack_table(klass, offset)
      sub_view = self.class.new(@data,
                                resolve_indirect(offset),
                                have_vtable: true)
      klass.new(sub_view)
    end

    def unpack_struct(klass, offset)
      sub_view = self.class.new(@data,
                                @offset + offset,
                                have_vtable: false)
      klass.new(sub_view)
    end

    def unpack_union(klass, offset)
      unpack_table(klass, offset)
    end

    def unpack_vector_length(offset)
      vector_offset = resolve_indirect(offset)
      unpack_offset_raw(vector_offset)
    end

    def unpack_vector(offset, element_size)
      relative_vector_offset = offset + unpack_offset(offset)
      length = unpack_offset(relative_vector_offset)
      return nil if length.zero?

      relative_vector_body_offset = relative_vector_offset + OFFSET_BYTE_SIZE
      length.times.collect do |i|
        relative_element_offset =
          relative_vector_body_offset + (element_size * i)
        yield(relative_element_offset)
      end
    end

    def unpack_bool_vector(offset, element_size)
      unpack_vector(offset, element_size) do |relative_element_offset|
        unpack_bool(relative_element_offset)
      end
    end

    def unpack_ubyte_vector(offset, element_size)
      unpack_vector(offset, element_size) do |relative_element_offset|
        unpack_ubyte(relative_element_offset)
      end
    end

    def unpack_string_vector(offset, element_size)
      unpack_vector(offset, element_size) do |relative_element_offset|
        unpack_string(relative_element_offset)
      end
    end

    def unpack_table_vector(offset, element_size, klass)
      unpack_vector(offset, element_size) do |relative_element_offset|
        unpack_table(klass, relative_element_offset)
      end
    end

    def unpack_struct_vector(offset, element_size, klass)
      unpack_vector(offset, element_size) do |relative_element_offset|
        unpack_struct(klass, relative_element_offset)
      end
    end
  end
end
