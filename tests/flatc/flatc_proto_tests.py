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

import json
import subprocess
import tempfile
from pathlib import Path

from flatc_test import flatc, flatc_path, reflection_fbs_path


def check_root(directory, schema, name, namespace=""):
  contents = (directory / schema).read_text()
  assert "root_type " + name + ";" in contents, contents
  # Reparse the generated schema and use its root to encode JSON.
  (directory / "data.json").write_text("{}")
  flatc(["--binary", schema, "data.json"], cwd=directory)
  assert (directory / "data.bin").exists()
  # The root declaration must bind to the intended fully qualified table.
  flatc(["--schema", "--binary", schema], cwd=directory)
  flatc([
      "--json", "--strict-json",
      str(reflection_fbs_path()),
      "--", Path(schema).stem + ".bfbs",
  ], cwd=directory)
  reflected = json.loads((directory / (Path(schema).stem + ".json")).read_text())
  qualified = namespace + "." + name if namespace else name
  assert reflected["root_table"]["name"] == qualified


class ProtoTests:

  def RootType(self):
    cases = [
        ("", "Root", [], ""),
        ("package example;", "Root", [], "example"),
        ("package example;", "example.Root", [], "example"),
        ("package example;", "example.Root",
         ["--proto-namespace-suffix", "generated"], "example.generated"),
    ]
    for package, root, options, namespace in cases:
      with tempfile.TemporaryDirectory() as tmp:
        directory = Path(tmp)
        (directory / "root.proto").write_text(
            'syntax = "proto3"; ' + package +
            " message Root { int32 value = 1; }"
        )
        flatc(["--proto", "--root-type", root] + options + ["root.proto"],
              cwd=directory)
        check_root(directory, "root.fbs", "Root", namespace)

  def NestedRootType(self):
    with tempfile.TemporaryDirectory() as tmp:
      directory = Path(tmp)
      (directory / "root.proto").write_text(
          'syntax = "proto3"; package example;'
          " message Outer { message Inner { int32 value = 1; } }"
          " message Last { int32 value = 1; }"
      )
      flatc(["--proto", "--root-type", "example.Outer.Inner",
             "--proto-namespace-suffix", "generated", "root.proto"],
            cwd=directory)
      check_root(directory, "root.fbs", "Inner", "example.generated.Outer_")

  def ImportedRootType(self):
    with tempfile.TemporaryDirectory() as tmp:
      directory = Path(tmp)
      (directory / "dependency.proto").write_text(
          'syntax = "proto3"; package dependency; message Root {}'
      )
      (directory / "main.proto").write_text(
          'syntax = "proto3"; package main; import "dependency.proto";'
          " message Other { dependency.Root value = 1; }"
      )
      flatc(["--proto", "dependency.proto"], cwd=directory)
      flatc(["--proto", "--root-type", "dependency.Root", "main.proto"],
            cwd=directory)
      check_root(directory, "main.fbs", "Root", "dependency")

  def NoRootType(self):
    with tempfile.TemporaryDirectory() as tmp:
      directory = Path(tmp)
      (directory / "root.proto").write_text(
          'syntax = "proto3"; message Root {}'
      )
      flatc(["--proto", "root.proto"], cwd=directory)
      assert "root_type" not in (directory / "root.fbs").read_text()

  def UnknownRootType(self):
    with tempfile.TemporaryDirectory() as tmp:
      directory = Path(tmp)
      (directory / "root.proto").write_text(
          'syntax = "proto3"; message Root {}'
      )
      result = subprocess.run(
          [str(flatc_path), "--proto", "--root-type", "Missing", "root.proto"],
          cwd=directory, capture_output=True, text=True,
      )
      assert result.returncode != 0
      assert "unknown root type: Missing" in result.stderr
      assert not (directory / "root.fbs").exists()
