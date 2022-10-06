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

import argparse
import platform
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument(
    "--flatc", help="path of the Flat C compiler relative to the root directory"
)

args = parser.parse_args()

# Get the path where this script is located so we can invoke the script from
# any directory and have the paths work correctly.
script_path = Path(__file__).parent.resolve()

# Get the root path as an absolute path, so all derived paths are absolute.
root_path = script_path.parent.parent.absolute()

# Get the location of the flatc executable, reading from the first command line
# argument or defaulting to default names.
flatc_exe = Path(
    ("flatc" if not platform.system() == "Windows" else "flatc.exe")
    if not args.flatc
    else args.flatc
)

# Find and assert flatc compiler is present.
if root_path in flatc_exe.parents:
    flatc_exe = flatc_exe.relative_to(root_path)
flatc_path = Path(root_path, flatc_exe)
assert flatc_path.exists(), "Cannot find the flatc compiler " + str(flatc_path)

# Execute the flatc compiler with the specified parameters
def flatc(options, cwd=script_path):
    cmd = [str(flatc_path)] + options
    subprocess.check_call(cmd, cwd=str(cwd))


def reflection_fbs_path():
    return Path(root_path).joinpath("reflection", "reflection.fbs")


def make_absolute(filename, path=script_path):
    return str(Path(path, filename).absolute())


def assert_file_exists(filename, path=script_path):
    file = Path(path, filename)
    assert file.exists(), "could not find file: " + filename
    return file


def assert_file_doesnt_exists(filename, path=script_path):
    file = Path(path, filename)
    assert not file.exists(), "file exists but shouldn't: " + filename
    return file


def get_file_contents(filename, path=script_path):
    file = Path(path, filename)
    contents = ""
    with open(file) as file:
        contents = file.read()
    return contents


def assert_file_contains(file, needles):
    with open(file) as file:
        contents = file.read()
        for needle in [needles] if isinstance(needles, str) else needles:
            assert needle in contents, (
                "coudn't find '" + needle + "' in file: " + str(file)
            )
    return file


def assert_file_doesnt_contains(file, needles):
    with open(file) as file:
        contents = file.read()
        for needle in [needles] if isinstance(needles, str) else needles:
            assert needle not in contents, (
                "Found unexpected '" + needle + "' in file: " + str(file)
            )
    return file


def assert_file_and_contents(
    file, needle, doesnt_contain=None, path=script_path, unlink=True
):
    assert_file_contains(assert_file_exists(file, path), needle)
    if doesnt_contain:
        assert_file_doesnt_contains(assert_file_exists(file, path), doesnt_contain)
    if unlink:
        Path(path, file).unlink()


def run_all(*modules):
    failing = 0
    passing = 0
    for module in modules:
        methods = [
            func
            for func in dir(module)
            if callable(getattr(module, func)) and not func.startswith("__")
        ]
        module_failing = 0
        module_passing = 0
        for method in methods:
            try:
                print("{0}.{1}".format(module.__name__, method))
                getattr(module, method)(module)
                print(" [PASSED]")
                module_passing = module_passing + 1
            except Exception as e:
                print(" [FAILED]: " + str(e))
                module_failing = module_failing + 1
        print(
            "{0}: {1} of {2} passsed".format(
                module.__name__, module_passing, module_passing + module_failing
            )
        )
        passing = passing + module_passing
        failing = failing + module_failing
    return passing, failing
