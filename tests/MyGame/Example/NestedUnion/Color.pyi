from __future__ import annotations

import flatbuffers
import numpy as np

import typing
from enum import IntFlag

uoffset: typing.TypeAlias = flatbuffers.number_types.UOffsetTFlags.py_type

class Color(IntFlag):
  Red: Color
  Green: Color
  Blue: Color

