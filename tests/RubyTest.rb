#!/usr/bin/env ruby
#
# Copyright 2014 Google Inc. All rights reserved.
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

$VERBOSE = true

require "fileutils"
require "tmpdir"

require "test/unit"

top_dir = File.expand_path(File.join(__dir__, ".."))
lib_dir = File.join(top_dir, "ruby")
$LOAD_PATH.unshift(lib_dir)

build_dir = ENV["BUILD_DIR"] || top_dir
flatc = File.join(build_dir, "flatc")

run_flatc = lambda do |*args|
  unless system(flatc, *args)
    raise "Failed to run flatc: #{flatc} #{args.join(" ")}"
  end
end

module Helper
  module Path
    class << self
      attr_accessor :tmp_dir
      attr_accessor :reflection_dir
      attr_accessor :monster_dir
    end
  end
end

tests_dir = File.join(top_dir, "tests")
Dir.mktmpdir do |tmp_dir|
  Helper::Path.tmp_dir = tmp_dir

  reflection_dir = File.join(tmp_dir, "reflection")
  Helper::Path.reflection_dir = reflection_dir
  FileUtils.mkdir_p(reflection_dir)
  reflection_fbs = File.join(top_dir, "reflection", "reflection.fbs")
  run_flatc.call("-o", reflection_dir,
                 "--ruby",
                 reflection_fbs)
  run_flatc.call("-o", reflection_dir,
                 "--bfbs-builtins",
                 "--bfbs-comments",
                 "--binary",
                 "--schema",
                 reflection_fbs)
  $LOAD_PATH.unshift(reflection_dir)

  monster_dir = File.join(tmp_dir, "monster")
  Helper::Path.monster_dir = monster_dir
  FileUtils.mkdir_p(monster_dir)
  run_flatc.call("-o", monster_dir,
                 "-I", File.join(tests_dir, "include_test"),
                 "--ruby",
                 File.join(tests_dir, "monster_test.fbs"))
  $LOAD_PATH.unshift(monster_dir)

  test_dir = File.join(tests_dir, "ruby")
  exit(Test::Unit::AutoRunner.run(true, test_dir))
end
