#!/bin/sh

pushd "$(dirname $0)" >/dev/null
test_dir="$(pwd)"
node ${test_dir}/JavaScriptTest
