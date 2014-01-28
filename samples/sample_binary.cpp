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

#include "flatbuffers/flatbuffers.h"

#include "monster_generated.h"

using namespace MyGame::Sample;

// Example how to use FlatBuffers to create and read binary buffers.

int main(int argc, const char *argv[]) {
  // Build up a serialized buffer algorithmically:
  flatbuffers::FlatBufferBuilder builder;

  auto vec = Vec3(1, 2, 3);

  auto name = builder.CreateString("MyMonster");

  unsigned char inv_data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  auto inventory = builder.CreateVector(inv_data, 10);

  // Shortcut for creating monster with all fields set:
  auto mloc = CreateMonster(builder, &vec, 150, 80, name, inventory,
                            Color_Blue);

  builder.Finish(mloc);
  // We now have a FlatBuffer we can store or send somewhere.

  // ** file/network code goes here :) **
  // access builder.GetBufferPointer() for builder.GetSize() bytes

  // Instead, we're going to access it straight away.
  // Get access to the root:
  auto monster = GetMonster(builder.GetBufferPointer());

  assert(monster->hp() == 80);
  assert(monster->mana() == 150);  // default
  assert(!strcmp(monster->name()->c_str(), "MyMonster"));

  auto pos = monster->pos();
  assert(pos);
  assert(pos->z() == 3);

  auto inv = monster->inventory();
  assert(inv);
  assert(inv->Get(9) == 9);
}

