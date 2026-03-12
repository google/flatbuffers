from __future__ import annotations

import flatbuffers
import numpy as np

import typing
from MyGame.Example.NestedUnion.TestSimpleTableWithEnum import TestSimpleTableWithEnum
from MyGame.Example.NestedUnion.Vec3 import Vec3
from enum import IntEnum
from flatbuffers import table

uoffset: typing.TypeAlias = flatbuffers.number_types.UOffsetTFlags.py_type

class Any(IntEnum):
  NONE: Any
  Vec3: Any
  TestSimpleTableWithEnum: Any

def AnyCreator(union_type: Any, table: table.Table) -> typing.Union[None, Vec3, TestSimpleTableWithEnum]: ...

