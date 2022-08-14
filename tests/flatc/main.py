#!/usr/bin/env python3

import sys;

from flatc_test import run_all
from flatc_cpp_tests import CppTests

passing, failing = run_all(CppTests)

if failing > 0:
  sys.exit(1)
