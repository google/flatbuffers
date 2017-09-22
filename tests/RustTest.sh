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

pushd "$(dirname $0)" >/dev/null
test_dir="$(pwd)"
rust_path=${test_dir}/rust_gen
rust_src=${rust_path}/src

# Emit Rust code for the example schema in the test dir:
../flatc -r monster_test.fbs

mkdir -p ${rust_src}/bin/test/MyGame/Example

sync_cmd='cp -u'
unamestr=`uname`
if [[ "$unamestr" == 'Darwin' ]]; then
  sync_cmd='rsync -u'
fi

${sync_cmd} MyGame/Example/*.rs ${rust_src}/bin/test/MyGame/Example/
${sync_cmd} MyGame/*.rs ${rust_src}/bin/test/MyGame/
${sync_cmd} ../rust/* ${rust_path}
${sync_cmd} ../rust/macros/* ${rust_path}/macros 
#{sync_cmd} ../rust/macros/src/* ${rust_path}/macros/src
${sync_cmd} ../rust/src/* ${rust_src}
${sync_cmd} ../rust/src/bin/* ${rust_src}/bin/
cp *.mon ${rust_path}/

#run tests
( cd $rust_path ; cargo test --features test_idl_gen -- --nocapture)
#run bench
# cant bench on stable rust as of 1.9.0

echo "Ok: Rust tests passed."
