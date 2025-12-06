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
  module Inspectable
    def inspect
      inspected = +"<#{self.class}:"
      public_methods(false).each do |name|
        next unless method(name).arity.zero?
        inspected << " #{name}=#{__send__(name).inspect}"
      end
      inspected << ">"
      inspected
    end

    def pretty_print(q)
      q.object_group(self) do
        targets = public_methods(false).select do |name|
          method(name).arity.zero?
        end
        q.seplist(targets, lambda {q.text(",")}) do |name|
          q.breakable
          q.text(name.to_s)
          q.text("=")
          q.pp(__send__(name))
        end
      end
    end
  end
end
