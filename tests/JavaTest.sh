#!/bin/bash
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

echo Compile then run the Java test.

java -version

testdir=$(readlink -fn `dirname $0`)
thisdir=$(readlink -fn `pwd`)

targetdir=${testdir}/target

if [[ "$testdir" != "$thisdir" ]]; then
	echo error: must be run from inside the ${testdir} directory
	echo you ran it from ${thisdir}
	exit 1
fi

find .. -type f -name "*.class" -exec rm  {} \;

if [[ -e "${targetdir}" ]]; then
    echo "clean target"
    rm -rf ${targetdir}
fi

mkdir ${targetdir}

javac -d ${targetdir} -classpath ${testdir}/../java:${testdir}:${testdir}/namespace_test:${testdir}/keysearch_test JavaTest.java
java -classpath ${targetdir} -ea JavaTest

rm -rf ${targetdir}
