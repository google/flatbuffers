from golden_utils import flatc_golden


def flatc(options, schema):
  # Wrap the golden flatc generator with Lua specifics
  flatc_golden(options=["--lua"] + options, schema=schema, prefix="lua")


def GenerateLua():
  flatc([], "basic.fbs")
  # Test schema that starts with namespace declaration
  flatc([], "namespace_first.fbs")
