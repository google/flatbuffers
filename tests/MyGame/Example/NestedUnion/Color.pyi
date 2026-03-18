from __future__ import annotations

import flatbuffers
import numpy as np

import typing
from enum import IntFlag
from typing import cast

uoffset: typing.TypeAlias = flatbuffers.number_types.UOffsetTFlags.py_type

class Color(IntFlag):
  Red = cast(int, ...)
  Green = cast(int, ...)
  Blue = cast(int, ...)

