#!/bin/bash

set -e

../flatc --php -o php_preserve_case union_vector/union_vector.fbs
php phpUnionVectorTest_preserve_case.php

echo 'PHP union vector test passed'
