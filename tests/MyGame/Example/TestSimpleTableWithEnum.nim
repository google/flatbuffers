#[ MyGame.Example.TestSimpleTableWithEnum
  Automatically generated by the FlatBuffers compiler, do not modify.
  Or modify. I'm a message, not a cop.

  flatc version: 23.1.20

  Declared by  : 
  Rooting type : MyGame.Example.Monster ()
]#

import Color as MyGame_Example_Color
import flatbuffers

type TestSimpleTableWithEnum* = object of FlatObj
func color*(self: TestSimpleTableWithEnum): MyGame_Example_Color.Color =
  let o = self.tab.Offset(4)
  if o != 0:
    return MyGame_Example_Color.Color(Get[uint8](self.tab, self.tab.Pos + o))
  return type(result)(2)
func `color=`*(self: var TestSimpleTableWithEnum, n: MyGame_Example_Color.Color): bool =
  return self.tab.MutateSlot(4, n)
proc TestSimpleTableWithEnumStart*(builder: var Builder) =
  builder.StartObject(1)
proc TestSimpleTableWithEnumAddcolor*(builder: var Builder, color: uint8) =
  builder.PrependSlot(0, color, default(uint8))
proc TestSimpleTableWithEnumEnd*(builder: var Builder): uoffset =
  return builder.EndObject()
