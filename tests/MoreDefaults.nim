#[ MoreDefaults
  Automatically generated by the FlatBuffers compiler, do not modify.
  Or modify. I'm a message, not a cop.

  flatc version: 23.5.8

  Declared by  : 
]#

import Abc as Abc
import flatbuffers

type MoreDefaults* = object of FlatObj
func intsLength*(self: MoreDefaults): int = 
  let o = self.tab.Offset(4)
  if o != 0:
    return self.tab.VectorLen(o)
func ints*(self: MoreDefaults, j: int): int32 = 
  let o = self.tab.Offset(4)
  if o != 0:
    var x = self.tab.Vector(o)
    x += j.uoffset * 4.uoffset
    return Get[int32](self.tab, x)
func ints*(self: MoreDefaults): seq[int32] = 
  let len = self.intsLength
  for i in countup(0, len - 1):
    result.add(self.ints(i))
func floatsLength*(self: MoreDefaults): int = 
  let o = self.tab.Offset(6)
  if o != 0:
    return self.tab.VectorLen(o)
func floats*(self: MoreDefaults, j: int): float32 = 
  let o = self.tab.Offset(6)
  if o != 0:
    var x = self.tab.Vector(o)
    x += j.uoffset * 4.uoffset
    return Get[float32](self.tab, x)
func floats*(self: MoreDefaults): seq[float32] = 
  let len = self.floatsLength
  for i in countup(0, len - 1):
    result.add(self.floats(i))
func emptyString*(self: MoreDefaults): string =
  let o = self.tab.Offset(8)
  if o != 0:
    return self.tab.String(self.tab.Pos + o)
  return ""
func someString*(self: MoreDefaults): string =
  let o = self.tab.Offset(10)
  if o != 0:
    return self.tab.String(self.tab.Pos + o)
  return ""
func abcsLength*(self: MoreDefaults): int = 
  let o = self.tab.Offset(12)
  if o != 0:
    return self.tab.VectorLen(o)
func abcs*(self: MoreDefaults, j: int): Abc.Abc = 
  let o = self.tab.Offset(12)
  if o != 0:
    var x = self.tab.Vector(o)
    x += j.uoffset * 4.uoffset
    return Abc.Abc(Get[int32](self.tab, x))
func abcs*(self: MoreDefaults): seq[Abc.Abc] = 
  let len = self.abcsLength
  for i in countup(0, len - 1):
    result.add(self.abcs(i))
func boolsLength*(self: MoreDefaults): int = 
  let o = self.tab.Offset(14)
  if o != 0:
    return self.tab.VectorLen(o)
func bools*(self: MoreDefaults, j: int): bool = 
  let o = self.tab.Offset(14)
  if o != 0:
    var x = self.tab.Vector(o)
    x += j.uoffset * 1.uoffset
    return Get[bool](self.tab, x)
func bools*(self: MoreDefaults): seq[bool] = 
  let len = self.boolsLength
  for i in countup(0, len - 1):
    result.add(self.bools(i))
proc MoreDefaultsStart*(builder: var Builder) =
  builder.StartObject(6)
proc MoreDefaultsAddints*(builder: var Builder, ints: uoffset) =
  builder.PrependSlot(0, ints, default(uoffset))
proc MoreDefaultsStartintsVector*(builder: var Builder, numElems: uoffset) =
  builder.StartVector(4, numElems, 4)
proc MoreDefaultsAddfloats*(builder: var Builder, floats: uoffset) =
  builder.PrependSlot(1, floats, default(uoffset))
proc MoreDefaultsStartfloatsVector*(builder: var Builder, numElems: uoffset) =
  builder.StartVector(4, numElems, 4)
proc MoreDefaultsAddemptyString*(builder: var Builder, emptyString: uoffset) =
  builder.PrependSlot(2, emptyString, default(uoffset))
proc MoreDefaultsAddsomeString*(builder: var Builder, someString: uoffset) =
  builder.PrependSlot(3, someString, default(uoffset))
proc MoreDefaultsAddabcs*(builder: var Builder, abcs: uoffset) =
  builder.PrependSlot(4, abcs, default(uoffset))
proc MoreDefaultsStartabcsVector*(builder: var Builder, numElems: uoffset) =
  builder.StartVector(4, numElems, 4)
proc MoreDefaultsAddbools*(builder: var Builder, bools: uoffset) =
  builder.PrependSlot(5, bools, default(uoffset))
proc MoreDefaultsStartboolsVector*(builder: var Builder, numElems: uoffset) =
  builder.StartVector(1, numElems, 1)
proc MoreDefaultsEnd*(builder: var Builder): uoffset =
  return builder.EndObject()
