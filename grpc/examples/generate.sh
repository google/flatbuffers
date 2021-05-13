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

current_dir=`pwd`

cd ../..

main_dir=`pwd`

cd ${current_dir}

# Looks for flatc within the root dir & debug
if [ -e ${main_dir}/flatc ]; then
  alias fbc='${main_dir}/flatc'
elif [ -e ${main_dir}/Debug/flatc ]; then
  alias fbc='${main_dir}/Debug/flatc'
else
  echo 'flatc' could not be found. Make sure to build FlatBuffers from the \
       $rootdir directory.
  exit 1
fi

generator="--grpc $current_dir/greeter.fbs"

# Regenerate Go lang code
cd go

cd greeter
fbc --go ${generator}

cd ${current_dir}

# Regenerate Python code
cd python

cd greeter

fbc --python ${generator}

cd ${current_dir}

# Regenerate Swift code
cd swift

cd Greeter/Sources/Model
fbc --swift ${generator}

cd ${current_dir}

# Regenerate Typescript code
cd ts

cd greeter/src
fbc --ts ${generator}

cd ${current_dir}
