from __future__ import annotations

import flatbuffers
import numpy as np

import typing
from enum import IntEnum

uoffset: typing.TypeAlias = flatbuffers.number_types.UOffsetTFlags.py_type

class TestEnum(IntEnum):
  A: TestEnum
  B: TestEnum
  C: TestEnum

