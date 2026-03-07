from __future__ import annotations

import flatbuffers
import numpy as np

import typing
from enum import IntEnum
from typing import Final

uoffset: typing.TypeAlias = flatbuffers.number_types.UOffsetTFlags.py_type

class TestEnum(IntEnum):
  A: Final[TestEnum]
  B: Final[TestEnum]
  C: Final[TestEnum]
  def __new__(cls, value: int) -> TestEnum: ...

