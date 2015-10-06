# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

__all__ = [
    'Table',
    'Builder',
    'implementation',
    'force_implementation',
]


def force_implementation(implname):
    """Forces flatbuffers to use a specific implementation if it's available"""
    global Table, Builder, implementation

    if implname == 'python':
        from . import table, builder
        Table, Builder = table.Table, builder.Builder
        implementation = 'python'
    elif implname == 'cython':
        from . import fastcodec
        Table, Builder = fastcodec.FastTable, fastcodec.FastBuilder
        implementation = 'cython'
    else:
        raise ImportError("No implementation named: %r" % implname)


try:
    force_implementation('cython')
except ImportError:
    force_implementation('python')
