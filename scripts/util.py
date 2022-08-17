# Copyright 2022 Google Inc. All rights reserved.
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

import platform
import subprocess
from pathlib import Path

# Get the path where this script is located so we can invoke the script from
# any directory and have the paths work correctly.
script_path = Path(__file__).parent.resolve()

# Get the root path as an absolute path, so all derived paths are absolute.
root_path = script_path.parent.absolute()

# Get the location of the flatc executable, reading from the first command line
# argument or defaulting to default names.
flatc_exe = Path("flatc" if not platform.system() == "Windows" else "flatc.exe")

# Find and assert flatc compiler is present.
if root_path in flatc_exe.parents:
    flatc_exe = flatc_exe.relative_to(root_path)
flatc_path = Path(root_path, flatc_exe)
assert flatc_path.exists(), "Cannot find the flatc compiler " + str(flatc_path)

# Execute the flatc compiler with the specified parameters
def flatc(options, schema, prefix=None, include=None, data=None, cwd=root_path):
    cmd = [str(flatc_path)] + options
    if prefix:
        cmd += ["-o"] + [prefix]
    if include:
        cmd += ["-I"] + [include]
    if isinstance(schema, Path):
      cmd += [str(schema)]
    elif isinstance(schema, str):
      cmd += [schema]
    else:
      cmd += schema
    if data:
        cmd += [data] if isinstance(data, str) else data
    return subprocess.check_call(cmd, cwd=str(cwd))
