from __future__ import annotations

import flatbuffers
import numpy as np

import flatbuffers
import typing
from MyGame.Example.NestedUnion.Any import Any
from MyGame.Example.NestedUnion.TestSimpleTableWithEnum import TestSimpleTableWithEnum
from MyGame.Example.NestedUnion.Vec3 import Vec3
from flatbuffers import table

uoffset: typing.TypeAlias = flatbuffers.number_types.UOffsetTFlags.py_type

class Any(object):
  NONE: int
  Vec3: int
  TestSimpleTableWithEnum: int
def AnyCreator(union_type: typing.Literal[Any.NONE, Any.Vec3, Any.TestSimpleTableWithEnum], table: table.Table) -> typing.Union[None, Vec3, TestSimpleTableWithEnum]: ...

