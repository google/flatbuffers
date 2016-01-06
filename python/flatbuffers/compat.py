""" A tiny version of `six` to help with backwards compability. """

import sys

PY2 = sys.version_info[0] == 2
PY26 = sys.version_info[0:2] == (2, 6)
PY27 = sys.version_info[0:2] == (2, 7)
PY3 = sys.version_info[0] == 3
PY34 = sys.version_info[0:2] >= (3, 4)

if PY3:
    string_types = (str,)
    binary_type = bytes
    range_func = range
    memoryview_type = memoryview
    struct_bool_decl = "?"
else:
    string_types = (basestring,)
    binary_type = str
    range_func = xrange
    if PY26 or PY27:
        memoryview_type = buffer
        struct_bool_decl = "<b"
    else:
        memoryview_type = memoryview
        struct_bool_decl = "?"

# NOTE: Future Jython support may require code here (look at `six`).
