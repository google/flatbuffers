#[ MyGame.InParentNamespace
  Automatically generated by the FlatBuffers compiler, do not modify.
  Or modify. I'm a message, not a cop.

  flatc version: 25.2.10

  Declared by  : 
  Rooting type : MyGame.Example.Monster ()
]#

import flatbuffers

type InParentNamespace* = object of FlatObj
proc InParentNamespaceStart*(builder: var Builder) =
  builder.StartObject(0)
proc InParentNamespaceEnd*(builder: var Builder): uoffset =
  return builder.EndObject()
