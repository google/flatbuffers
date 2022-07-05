discard """
  action:   "run"
  exitcode: 0
  timeout:  60.0
"""
import std/unittest
import flatbuffers
import ../../generated/MoreDefaults

suite "TestMoreDefaults":

  test "testFlatbuffersObject":
    var fbb = newBuilder(0)
    fbb.MoreDefaultsStart()
    let root = fbb.MoreDefaultsEnd()
    fbb.Finish(root)

    var defaults: MoreDefaults
    defaults.GetRootAs(fbb.FinishedBytes(), 0)
    check(defaults.emptyString == "")
    check(defaults.someString == "some")
    check(defaults.ints == [])
    check(defaults.floats == [])
    check(defaults.bools == [])
    check(defaults.intsLength == 0)
    check(defaults.floatsLength == 0)
    check(defaults.abcsLength == 0)
    check(defaults.boolsLength == 0)

  # test "testFlatbuffersObjectAPI":
  #   let defaults = newMoreDefaultsT()
  #   check(defaults.emptyString.isSome)
  #   check(defaults.emptyString.get() == "")
  #   check(defaults.someString.isSome)
  #   check(defaults.someString.get() == "some")
  #   check(defaults.ints == [])
  #   check(defaults.floats == [])
  #   check(defaults.abcs == [])
  #   check(defaults.bools == [])

  #   var fbb = newBuilder(0)
  #   fbb.Finish(defaults.Pack(fbb))
  #   var fDefaults: MoreDefaults
  #   fDefaults.GetRootAs(fbb.FinishedBytes(), 0)
  #   check(fDefaults.emptyString.isSome)
  #   check(fDefaults.emptyString.get() == "")
  #   check(fDefaults.someString.isSome)
  #   check(fDefaults.someString.get() == "some")
  #   check(fDefaults.ints == [])
  #   check(fDefaults.floats == [])
  #   check(fDefaults.bools == [])
  #   check(fDefaults.intsLength == 0)
  #   check(fDefaults.floatsLength == 0)
  #   check(fDefaults.abcsLength == 0)
  #   check(fDefaults.boolsLength == 0)
