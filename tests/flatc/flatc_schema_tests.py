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
from tempfile import TemporaryDirectory
from flatc_test import *


class SchemaTests:

  def EnumValAttributes(self):
    # Generate .bfbs schema first
    flatc(
        ["--schema", "--binary", "--bfbs-builtins", "enum_val_attributes.fbs"]
    )
    assert_file_exists("enum_val_attributes.bfbs")

    # Then turn it into JSON
    flatc([
        "--json",
        "--strict-json",
        str(reflection_fbs_path()),
        "--",
        "enum_val_attributes.bfbs",
    ])

    # The attributes should be present in JSON
    schema_json = json.loads(get_file_contents("enum_val_attributes.json"))

    assert schema_json["enums"][0]["name"] == "ValAttributes"
    assert schema_json["enums"][0]["values"][0]["name"] == "Val1"
    assert (
        schema_json["enums"][0]["values"][0]["attributes"][0]["key"]
        == "display_name"
    )
    assert (
        schema_json["enums"][0]["values"][0]["attributes"][0]["value"]
        == "Value 1"
    )

    assert schema_json["enums"][0]["values"][1]["name"] == "Val2"
    assert (
        schema_json["enums"][0]["values"][1]["attributes"][0]["key"]
        == "display_name"
    )
    assert (
        schema_json["enums"][0]["values"][1]["attributes"][0]["value"]
        == "Value 2"
    )

    assert schema_json["enums"][0]["values"][2]["name"] == "Val3"
    assert (
        schema_json["enums"][0]["values"][2]["attributes"][0]["key"]
        == "deprecated"
    )
    assert (
        schema_json["enums"][0]["values"][2]["attributes"][1]["key"]
        == "display_name"
    )
    assert (
        schema_json["enums"][0]["values"][2]["attributes"][1]["value"]
        == "Value 3 (deprecated)"
    )

  def CircularStructDependency(self):
    # Missing input files must not be mistaken for rejection of a cycle.
    circular_struct = assert_file_exists("circular_struct_dependency.fbs")
    circular_table = assert_file_exists("circular_table.fbs")
    with TemporaryDirectory() as output:
      result = subprocess.run(
          [str(flatc_path), "--cpp", "-o", output, str(circular_struct)],
          capture_output=True,
          text=True,
      )
      assert result.returncode != 0, "Expected flatc to reject the struct cycle"
      assert (
          "schema has circular struct dependencies" in result.stderr
      ), result.stderr

      # Tables may refer to themselves because their fields store offsets.
      flatc(["--cpp", "-o", output, str(circular_table)])
      assert_file_exists("circular_table_generated.h", Path(output))
