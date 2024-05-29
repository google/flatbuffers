from __future__ import annotations

import flatbuffers
import numpy as np

import flatbuffers
import typing
from MyGame.Example.ArrayStruct import ArrayStruct, ArrayStructT
from MyGame.Example.ArrayTable import ArrayTable

uoffset: typing.TypeAlias = flatbuffers.number_types.UOffsetTFlags.py_type

class ArrayTable(object):
  @classmethod
  def GetRootAs(cls, buf: bytes, offset: int) -> ArrayTable: ...
  @classmethod
  def GetRootAsArrayTable(cls, buf: bytes, offset: int) -> ArrayTable: ...
  @classmethod
  def ArrayTableBufferHasIdentifier(cls, buf: bytes, offset: int, size_prefixed: bool) -> bool: ...
  def Init(self, buf: bytes, pos: int) -> None: ...
  def A(self) -> ArrayStruct | None: ...
class ArrayTableT(object):
  a: ArrayStructT | None
  @classmethod
  def InitFromBuf(cls, buf: bytes, pos: int) -> ArrayTableT: ...
  @classmethod
  def InitFromPackedBuf(cls, buf: bytes, pos: int = 0) -> ArrayTableT: ...
  @classmethod
  def InitFromObj(cls, arrayTable: ArrayTable) -> ArrayTableT: ...
  def _UnPack(self, arrayTable: ArrayTable) -> None: ...
  def Pack(self, builder: flatbuffers.Builder) -> None: ...
def ArrayTableStart(builder: flatbuffers.Builder) -> None: ...
def Start(builder: flatbuffers.Builder) -> None: ...
def ArrayTableAddA(builder: flatbuffers.Builder, a: uoffset) -> None: ...
def ArrayTableEnd(builder: flatbuffers.Builder) -> uoffset: ...
def End(builder: flatbuffers.Builder) -> uoffset: ...

