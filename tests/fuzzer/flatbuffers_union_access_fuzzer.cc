/*
 * Copyright 2026 Google LLC
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

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "cpp17/generated_cpp17/monster_test_generated.h"

namespace {

void AccessUnionTable(const MyGame::Example::Monster* root) {
  const auto* raw_union = root->test();
  if (!raw_union) return;

  const auto* union_table =
      static_cast<const MyGame::Example::Monster*>(raw_union);
  volatile auto hp = union_table->hp();
  volatile auto mana = union_table->mana();
  volatile auto hash = union_table->testhashs64_fnv1a();
  if (const auto* name = union_table->name()) {
    volatile auto name_size = name->size();
    if (name_size != 0) {
      volatile auto c = name->c_str()[0];
      (void)c;
    }
  }
  if (const auto* inventory = union_table->inventory()) {
    volatile auto inventory_size = inventory->size();
    (void)inventory_size;
  }
  (void)hp;
  (void)mana;
  (void)hash;
}

void MutateUnionTable(const uint8_t* data, size_t size) {
  std::vector<uint8_t> mutable_copy(data, data + size);
  flatbuffers::Verifier verifier(mutable_copy.data(), mutable_copy.size());
  if (!MyGame::Example::VerifyMonsterBuffer(verifier)) return;

  auto* root = MyGame::Example::GetMutableMonster(mutable_copy.data());
  if (!root) return;

  auto* raw_union = root->mutable_test();
  if (!raw_union) return;

  auto* union_table = static_cast<MyGame::Example::Monster*>(raw_union);
  (void)union_table->mutate_hp(42);
  (void)union_table->mutate_mana(17);
  (void)union_table->mutate_testhashs64_fnv1a(0x1122334455667788LL);
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  flatbuffers::Verifier verifier(data, size);
  if (!MyGame::Example::VerifyMonsterBuffer(verifier)) return 0;

  flatbuffers::SizeVerifier size_verifier(data, size);
  (void)MyGame::Example::VerifyMonsterBuffer(size_verifier);

  const auto* root = MyGame::Example::GetMonster(data);
  if (!root) return 0;

  MutateUnionTable(data, size);
  AccessUnionTable(root);
  return 0;
}
