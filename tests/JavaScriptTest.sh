#!/bin/sh

pushd "$(dirname $0)" >/dev/null
../flatc -b monster_test.fbs unicode_test.json
node JavaScriptTest
