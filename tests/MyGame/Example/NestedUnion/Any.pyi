from __future__ import annotations

import flatbuffers
import numpy as np

import typing
from MyGame.Example.NestedUnion.TestSimpleTableWithEnum import TestSimpleTableWithEnum
from MyGame.Example.NestedUnion.Vec3 import Vec3
from enum import IntEnum
from flatbuffers import table
from typing import Final

uoffset: typing.TypeAlias = flatbuffers.number_types.UOffsetTFlags.py_type

class Any(IntEnum):
  NONE: Final[Any]
  Vec3: Final[Any]
  TestSimpleTableWithEnum: Final[Any]
  def __new__(cls, value: int) -> Any: ...

def AnyCreator(union_type: Any, table: table.Table) -> typing.Union[None, Vec3, TestSimpleTableWithEnum]: ...

