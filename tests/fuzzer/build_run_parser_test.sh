#!/bin/bash
#
# Copyright 2015 Google Inc. All rights reserved.
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

clang++ -fsanitize-coverage=edge -fsanitize=address -std=c++11 -stdlib=libstdc++ -I.. -I../../include flatbuffers_parser_fuzzer.cc ../../src/idl_parser.cpp ../../src/util.cpp libFuzzer.a -o fuzz_parser
mkdir -p parser_corpus
cp ../*.json ../*.fbs parser_corpus
./fuzz_parser parser_corpus
