local m = {}

m.Builder = require("flatbuffers.builder").New
m.encode = require("flatbuffers.encode")
m.N = require("flatbuffers.numTypes")
m.view = require("flatbuffers.view")
m.binaryArray = require("flatbuffers.binaryarray")

return m