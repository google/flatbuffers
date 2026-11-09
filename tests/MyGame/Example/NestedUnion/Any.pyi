from __future__ import annotations

import flatbuffers

import typing
from MyGame.Example.NestedUnion.TestSimpleTableWithEnum import TestSimpleTableWithEnum
from MyGame.Example.NestedUnion.Vec3 import Vec3
from flatbuffers import table
from typing import cast

class Any(object):
  NONE = cast(int, ...)
  Vec3 = cast(int, ...)
  TestSimpleTableWithEnum = cast(int, ...)
def AnyCreator(union_type: typing.Literal[Any.NONE, Any.Vec3, Any.TestSimpleTableWithEnum], table: table.Table) -> typing.Union[None, Vec3, TestSimpleTableWithEnum]: ...

