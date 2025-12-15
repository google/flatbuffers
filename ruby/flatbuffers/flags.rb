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
  class Flags
    class << self
      def try_convert(value)
        case value
        when Array
          value.inject(new) do |previous, v|
            flag = try_convert(v)
            return nil if flag.nil?
            previous | flag
          end
        when Symbol, String
          try_convert(@name_to_value[value.to_s])
        when Integer
          new(value)
        when self
          value
        else
          nil
        end
      end

      def register(name, value)
        (@name_to_value ||= {})[name] = value
        new(value)
      end

      def names
        @name_to_value.keys
      end

      def resolve_names(value)
        @name_to_value.select do |name, v|
          not (value & v).zero?
        end
      end
    end

    attr_reader :value
    def initialize(value=0)
      @value = value
    end

    def names
      @names ||= self.class.resolve_names(@value)
    end

    alias_method :to_i, :value
    alias_method :to_int, :value

    def |(other)
      self.class.new(@value | Integer(other))
    end
  end
end
