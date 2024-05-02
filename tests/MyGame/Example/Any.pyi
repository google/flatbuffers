from __future__ import annotations

import flatbuffers
import numpy as np

from flatbuffers import table

class Any(object):
  NONE: int
  Monster: int
  TestSimpleTableWithEnum: int
  MyGame_Example2_Monster: int
def AnyCreator(unionType: int, table: table.Table): ...

