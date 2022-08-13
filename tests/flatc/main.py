#!/usr/bin/env python3

import sys;

from flatc_test import run_all
from flatc_cpp_tests import CppTests
from flatc_ts_tests import TsTests

passing, failing = run_all(CppTests)
passing, failing = run_all(TsTests)

if failing > 0:
  sys.exit(1)
