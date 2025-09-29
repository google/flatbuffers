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
  class Union
    class << self
      def try_convert(value)
        case value
        when Symbol, String
          @name_to_union[value.to_s]
        when Integer
          @value_to_union[value]
        when self
          value
        else
          nil
        end
      end

      def register(name, value, table_class_name, require_path)
        object = new(name, value, table_class_name, require_path)
        (@name_to_union ||= {})[name] = object
        (@value_to_union ||= {})[value] = object
        object
      end
    end

    attr_reader :name
    attr_reader :value
    def initialize(name, value, table_class_name, require_path)
      @name = name
      @value = value
      @table_class_name = table_class_name
      @require_path = require_path
    end

    def table_class
      @table_class ||= resolve_table_class
    end

    private def resolve_table_class
      return nil if @table_class_name.nil?

      require_table_class
      Object.const_get(@table_class_name)
    end

    alias_method :to_i, :value
    alias_method :to_int, :value
  end
end
