#!/bin/bash

set -eu

preserve_case_flag="--preserve-case"
output_dir="php_preserve_case"
echo "Running PHP union vector test with preserve-case enabled."

rm -rf "${output_dir}"
mkdir -p "${output_dir}"

../flatc --php ${preserve_case_flag} -o "${output_dir}" union_vector/union_vector.fbs
PHP_UNION_GENERATED_DIR="$(pwd)/${output_dir}" php phpUnionVectorTest.php

echo 'PHP union vector test passed'
