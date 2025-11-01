#!/bin/bash -eu
#
# Runs the C++ FlatBuffers tests with both default naming and the
# --preserve-case option.

pushd "$(dirname "$0")" >/dev/null
test_dir="$(pwd)"
repo_dir="$(cd .. && pwd)"

echo "Running C++ tests (default naming)"
if [[ ! -x "${repo_dir}/flattests" ]]; then
  echo "error: ${repo_dir}/flattests not found. Build the project first (e.g. cmake --build ./build --target flattests)." >&2
  exit 1
fi
"${repo_dir}/flattests"

echo "Running C++ tests (preserve-case)"
generated_dir="${test_dir}/preserve_case_cpp"
rm -rf "${generated_dir}"
mkdir -p "${generated_dir}"

flatc_bin="${repo_dir}/flatc"
if [[ ! -x "${flatc_bin}" ]]; then
  echo "error: ${flatc_bin} not found. Build flatc first." >&2
  exit 1
fi

base_opts=(
  --cpp
  --gen-mutable
  --gen-object-api
  --reflect-names
  --gen-compare
  --preserve-case
  -o "${generated_dir}"
  -I "${test_dir}/include_test"
)

"${flatc_bin}" "${base_opts[@]}" \
  "${test_dir}/include_test/include_test1.fbs" \
  "${test_dir}/include_test/sub/include_test2.fbs" \
  "${test_dir}/monster_test.fbs"

cxx=${CXX:-c++}
cxxflags=${CXXFLAGS:-}

"${cxx}" ${cxxflags} -std=c++17 -Wall -Wextra \
  -I "${repo_dir}/include" \
  -I "${test_dir}" \
  -I "${generated_dir}" \
  "${test_dir}/CppPreserveCaseTest.cpp" \
  -o "${generated_dir}/CppPreserveCaseTest"

"${generated_dir}/CppPreserveCaseTest"

popd >/dev/null
