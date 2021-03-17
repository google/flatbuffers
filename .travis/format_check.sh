#!/bin/bash
#
# Copyright 2021 Google Inc. All rights reserved.
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

set -e

# HACKY solution to make nodejs work.
source ~/.nvm/nvm.sh
nvm alias default node
nvm use default

sh src/clang-format-git.sh

# Check formatting for go lang

cd go
gofmt -w .
cd ..
cd grpc/examples/go
sh format.sh
cd ../../..

node_modules/.bin/eslint ts/** --ext .ts --quiet --fix

#PYTHON IS DISABLED UNTIL WE CREATE A .pylintrc FILE FOR IT
pylint python/** --disable=all

swiftformat --config swift.swiftformat .


if ! git diff --quiet; then
  echo >&2
  echo "ERROR: ********************************************************" >&2
  echo "ERROR: The following differences were found after running" >&2
  echo "ERROR: .travis/format_check.sh script. Maybe you forgot to format" >&2
  echo "ERROR: the code after making changes? please check Formatters.md" >&2
  echo "ERROR: ********************************************************" >&2
  echo >&2
  git diff --binary --exit-code
fi