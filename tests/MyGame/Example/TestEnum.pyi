from __future__ import annotations

import flatbuffers
import numpy as np

import typing
from enum import IntEnum
from typing import cast

uoffset: typing.TypeAlias = flatbuffers.number_types.UOffsetTFlags.py_type

class TestEnum(IntEnum):
  A = cast(int, ...)
  B = cast(int, ...)
  C = cast(int, ...)

