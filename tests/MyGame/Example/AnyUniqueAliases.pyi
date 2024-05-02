from __future__ import annotations

import flatbuffers
import numpy as np

from flatbuffers import table

class AnyUniqueAliases(object):
  NONE: int
  M: int
  TS: int
  M2: int
def AnyUniqueAliasesCreator(unionType: int, table: table.Table): ...

