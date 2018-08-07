// Copyright 2018 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

include from "../lobster/"
include "monster_generated.lobster"

// Example how to interop with JSON.

// Test loading some JSON, converting it to a binary FlatBuffer and back again.

// First read the schema and JSON data.
schema := read_file("monster.fbs", true)
json := read_file("monsterdata.json", true)
assert schema and json

// Parse JSON  to binary:
fb, err1 := flatbuffers_json_to_binary(schema, json, [])
assert not err1

// Access one field in it, just to check:
let monster = MyGame_Sample_GetRootAsMonster(fb)
assert monster.name == "Orc"

// Convert binary back to JSON:
json2, err2 := flatbuffers_binary_to_json(schema, fb, [])
assert not err2

// The generated JSON should be exactly equal to the original!
assert json == json2

// Print what we've been converting for good measure:
print json
