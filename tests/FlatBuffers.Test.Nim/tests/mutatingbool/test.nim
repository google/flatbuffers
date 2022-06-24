discard """
  action:   "run"
  exitcode: 0
  timeout:  60.0
"""
import std/unittest
import std/options
import flatbuffers
import ../../generated/TestMutatingBool
import ../../generated/Property

proc get(): TestMutatingBoolT =
  discard

suite "TestMutatingBool":

  test "MutatingBool":
    var builder = newBuilder(1024)
    builder.TestMutatingBoolStart()
    builder.TestMutatingBoolAddB(builder.CreateProperty(false))
    let root = builder.TestMutatingBoolEnd()
    builder.Finish(root)

    var test_mutating_bool: TestMutatingBool
    GetRootAs(test_mutating_bool, builder.FinishedBytes(), 0)
    check(test_mutating_bool.b.isSome)
    var prop2 = test_mutating_bool.b.get()
    check(prop2.property == false)
    discard (prop2.property = false)
    check(prop2.property == false)
    discard (prop2.property = true)
    check(prop2.property == true)

  test "EmptyBool":
    var builder = newBuilder(1024)
    builder.TestMutatingBoolStart()
    let root = builder.TestMutatingBoolEnd()
    builder.Finish(root)

    var test_mutating_bool: TestMutatingBool
    GetRootAs(test_mutating_bool, builder.FinishedBytes(), 0)
    check(test_mutating_bool.b.isNone)

  test "defaultValue":
    let mutatingbool = newTestMutatingBoolT()
    check(mutatingbool.b.isNone)
