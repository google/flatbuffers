from __future__ import annotations

import flatbuffers
import numpy as np

import typing
from enum import IntFlag
from typing import Final

uoffset: typing.TypeAlias = flatbuffers.number_types.UOffsetTFlags.py_type

class Color(IntFlag):
  Red: Final[Color]
  Green: Final[Color]
  Blue: Final[Color]
  def __new__(cls, value: int) -> Color: ...

