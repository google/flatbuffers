/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#include "monster_generated.h" // Already includes "flatbuffers/flatbuffers.h".

using namespace MyGame::Sample;

// This is an example of parsing text straight into a buffer and then
// generating flatbuffer (JSON) text from the buffer.
int main(int /*argc*/, const char * /*argv*/[]) {
  // load FlatBuffer schema (.fbs) and JSON from disk
  std::string schemafile;
  std::string jsonfile;
  bool ok = flatbuffers::LoadFile("samples/monster.fbs", false, &schemafile) &&
    flatbuffers::LoadFile("samples/monsterdata.json", false, &jsonfile);
  if (!ok) {
    printf("couldn't load files!\n");
    return 1;
  }

  // parse schema first, so we can use it to parse the data after
  flatbuffers::Parser parser1;
  const char *include_directories[] = { "samples", nullptr };
  ok = parser1.Parse(schemafile.c_str(), include_directories);
  //    &&
  //       parser.Parse(jsonfile.c_str(), include_directories);
  assert(ok);
  parser1.Serialize();

  ok = flatbuffers::GenerateBinary(parser1, "samples/", "monster1");
  assert(ok);

  flatbuffers::Parser parser2;
  std::string bfbsfile1;
  ok = flatbuffers::LoadFile("samples/monster1.bin", true, &bfbsfile1);
  assert(ok);
  parser2.Deserialize((uint8_t*)bfbsfile1.c_str(), bfbsfile1.length());

  parser2.Serialize();

  ok = flatbuffers::GenerateBinary(parser1, "samples/", "monster2");
  assert(ok);
  std::string bfbsfile2;
  ok = flatbuffers::LoadFile("samples/monster2.bin", true, &bfbsfile2);
  assert(ok);
  assert(bfbsfile1 == bfbsfile2);
  ok = parser1.Parse(jsonfile.c_str(), include_directories);
  ok = parser2.Parse(jsonfile.c_str(), include_directories);
  assert(ok);



  // to ensure it is correct, we now generate text back from the binary,
  // and compare the two:
  std::string jsongen1;
  if (!GenerateText(parser1, parser1.builder_.GetBufferPointer(), &jsongen1)) {
    printf("Couldn't serialize parsed data to JSON!\n");
    return 1;
  }

  std::string jsongen2;
  if (!GenerateText(parser2, parser2.builder_.GetBufferPointer(), &jsongen2)) {
    printf("Couldn't serialize parsed data to JSON!\n");
    return 1;
  }

  if (jsongen1 != jsongen2) {
    printf("%s----------------\n%s", jsongen1.c_str(), jsongen2.c_str());
  }

  printf("The FlatBuffer has been parsed from JSON successfully.\n");
}
