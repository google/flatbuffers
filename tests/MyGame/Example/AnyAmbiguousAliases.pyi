from __future__ import annotations

import flatbuffers
import numpy as np

from flatbuffers import table

class AnyAmbiguousAliases(object):
  NONE: int
  M1: int
  M2: int
  M3: int
def AnyAmbiguousAliasesCreator(unionType: int, table: table.Table): ...

