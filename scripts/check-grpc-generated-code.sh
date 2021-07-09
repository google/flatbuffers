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

echo "Checks generated grpc code"
cd grpc/examples
sh generate.sh
cd ..

if ! git diff --quiet; then
  echo >&2
  echo "ERROR: ********************************************************" >&2
  echo "ERROR: The following differences were found after running the" >&2
  echo "ERROR: grpc/example/generate.sh script. Maybe you forgot to run" >&2
  echo "ERROR: it after making changes in a generator or schema?" >&2
  echo "ERROR: ********************************************************" >&2
  echo >&2
  git diff --binary --exit-code
fi
