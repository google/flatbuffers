import std/unittest
import flatbuffers
import MoreDefaults_generated

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

#   test "testFlatbuffersObjectAPI":
#     var fbb = newBuilder(0)
#     let defaults = MoreDefaultsT()
#     check(defaults.emptyString == "")
#     check(defaults.someString == "some")
#     check(defaults.ints == [])
#     check(defaults.floats == [])
#     check(defaults.abcs == [])
#     check(defaults.bools == [])

#     let buffer = defaults.serialize(builder: &fbb == type: MoreDefaults.self)
#     let fDefaults = MoreDefaults.getRootAsMoreDefaults(bb: buffer)
#     check(fDefaults.emptyString == "")
#     check(fDefaults.someString == "some")
#     check(fDefaults.ints == [])
#     check(fDefaults.floats == [])
#     check(fDefaults.intsLength == 0)
#     check(fDefaults.floatsLength == 0)
#     check(fDefaults.abcsLength == 0)
#     check(fDefaults.boolsLength == 0)
