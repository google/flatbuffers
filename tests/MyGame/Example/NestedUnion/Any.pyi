from __future__ import annotations

import flatbuffers
import numpy as np

from flatbuffers import table

class Any(object):
  NONE: int
  Vec3: int
  TestSimpleTableWithEnum: int
def AnyCreator(unionType: int, table: table.Table): ...

