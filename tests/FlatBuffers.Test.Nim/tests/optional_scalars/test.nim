discard """
  cmd:      "nim c -r --panics:on $options $file"
  matrix:   "--gc:refc"
  targets:  "c"
  action:   "run"
  exitcode: 0
  timeout:  60.0
"""
import std/unittest
import flatbuffers
import ../../generated/optional_scalars/ScalarStuff_generated

# Optionals could be supported with import 'std/options' but that would make the usage more awkward

suite "TestOptionalScalars":

  test "OptionalScalars":
    var builder = newBuilder(1024)
    builder.ScalarStuffStart()
    let root = builder.ScalarStuffEnd()
    builder.Finish(root)

    var optionals: ScalarStuff
    optionals.GetRootAs(builder.FinishedBytes(), 0)

    # Creates a flatbuffer with optional values.
    check(optionals.justI8 == 0)
    # check(optionals.maybeF32() is None)
    check(optionals.defaultBool == true)
    check(optionals.justU16 == 0)
    # check(optionals.maybe_enum is None)
    check(optionals.defaultU64 == 42)
