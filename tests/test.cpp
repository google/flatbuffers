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
#include "flatbuffers/idl.h"
#include "flatbuffers/minireflect.h"
#include "flatbuffers/registry.h"
#include "flatbuffers/util.h"

#include "monster_test_generated.h"
#include "namespace_test/namespace_test1_generated.h"
#include "namespace_test/namespace_test2_generated.h"
#include "union_vector/union_vector_generated.h"

// clang-format off
#ifndef FLATBUFFERS_CPP98_STL
  #include <random>
#endif

#include "flatbuffers/flexbuffers.h"

using namespace MyGame::Example;

#ifdef __ANDROID__
  #include <android/log.h>
  #define TEST_OUTPUT_LINE(...) \
    __android_log_print(ANDROID_LOG_INFO, "FlatBuffers", __VA_ARGS__)
  #define FLATBUFFERS_NO_FILE_TESTS
#else
  #define TEST_OUTPUT_LINE(...) \
    { printf(__VA_ARGS__); printf("\n"); }
#endif
// clang-format on

int testing_fails = 0;

void TestFail(const char *expval, const char *val, const char *exp,
              const char *file, int line) {
  TEST_OUTPUT_LINE("VALUE: \"%s\"", expval);
  TEST_OUTPUT_LINE("EXPECTED: \"%s\"", val);
  TEST_OUTPUT_LINE("TEST FAILED: %s:%d, %s", file, line, exp);
  assert(0);
  testing_fails++;
}

void TestEqStr(const char *expval, const char *val, const char *exp,
               const char *file, int line) {
  if (strcmp(expval, val) != 0) { TestFail(expval, val, exp, file, line); }
}

template<typename T, typename U>
void TestEq(T expval, U val, const char *exp, const char *file, int line) {
  if (U(expval) != val) {
    TestFail(flatbuffers::NumToString(expval).c_str(),
             flatbuffers::NumToString(val).c_str(), exp, file, line);
  }
}

#define TEST_EQ(exp, val) TestEq(exp, val, #exp, __FILE__, __LINE__)
#define TEST_NOTNULL(exp) TestEq(exp == NULL, false, #exp, __FILE__, __LINE__)
#define TEST_EQ_STR(exp, val) TestEqStr(exp, val, #exp, __FILE__, __LINE__)

// Include simple random number generator to ensure results will be the
// same cross platform.
// http://en.wikipedia.org/wiki/Park%E2%80%93Miller_random_number_generator
uint32_t lcg_seed = 48271;
uint32_t lcg_rand() {
  return lcg_seed = ((uint64_t)lcg_seed * 279470273UL) % 4294967291UL;
}
void lcg_reset() { lcg_seed = 48271; }

std::string test_data_path = "tests/";

// example of how to build up a serialized buffer algorithmically:
flatbuffers::DetachedBuffer CreateFlatBufferTest(std::string &buffer) {
  flatbuffers::FlatBufferBuilder builder;

  auto vec = Vec3(1, 2, 3, 0, Color_Red, Test(10, 20));

  auto name = builder.CreateString("MyMonster");

  unsigned char inv_data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  auto inventory = builder.CreateVector(inv_data, 10);

  // Alternatively, create the vector first, and fill in data later:
  // unsigned char *inv_buf = nullptr;
  // auto inventory = builder.CreateUninitializedVector<unsigned char>(
  //                                                              10, &inv_buf);
  // memcpy(inv_buf, inv_data, 10);

  Test tests[] = { Test(10, 20), Test(30, 40) };
  auto testv = builder.CreateVectorOfStructs(tests, 2);

  // clang-format off
  #ifndef FLATBUFFERS_CPP98_STL
    // Create a vector of structures from a lambda.
    auto testv2 = builder.CreateVectorOfStructs<Test>(
          2, [&](size_t i, Test* s) -> void {
            *s = tests[i];
          });
  #else
    // Create a vector of structures using a plain old C++ function.
    auto testv2 = builder.CreateVectorOfStructs<Test>(
          2, [](size_t i, Test* s, void *state) -> void {
            *s = (reinterpret_cast<Test*>(state))[i];
          }, tests);
  #endif  // FLATBUFFERS_CPP98_STL
  // clang-format on

  // create monster with very few fields set:
  // (same functionality as CreateMonster below, but sets fields manually)
  flatbuffers::Offset<Monster> mlocs[3];
  auto fred = builder.CreateString("Fred");
  auto barney = builder.CreateString("Barney");
  auto wilma = builder.CreateString("Wilma");
  MonsterBuilder mb1(builder);
  mb1.add_name(fred);
  mlocs[0] = mb1.Finish();
  MonsterBuilder mb2(builder);
  mb2.add_name(barney);
  mb2.add_hp(1000);
  mlocs[1] = mb2.Finish();
  MonsterBuilder mb3(builder);
  mb3.add_name(wilma);
  mlocs[2] = mb3.Finish();

  // Create an array of strings. Also test string pooling, and lambdas.
  auto vecofstrings =
      builder.CreateVector<flatbuffers::Offset<flatbuffers::String>>(
          4,
          [](size_t i, flatbuffers::FlatBufferBuilder *b)
              -> flatbuffers::Offset<flatbuffers::String> {
            static const char *names[] = { "bob", "fred", "bob", "fred" };
            return b->CreateSharedString(names[i]);
          },
          &builder);

  // Creating vectors of strings in one convenient call.
  std::vector<std::string> names2;
  names2.push_back("jane");
  names2.push_back("mary");
  auto vecofstrings2 = builder.CreateVectorOfStrings(names2);

  // Create an array of sorted tables, can be used with binary search when read:
  auto vecoftables = builder.CreateVectorOfSortedTables(mlocs, 3);

  // Create an array of sorted structs,
  // can be used with binary search when read:
  std::vector<Ability> abilities;
  abilities.push_back(Ability(4, 40));
  abilities.push_back(Ability(3, 30));
  abilities.push_back(Ability(2, 20));
  abilities.push_back(Ability(1, 10));
  auto vecofstructs = builder.CreateVectorOfSortedStructs(&abilities);

  // Create a nested FlatBuffer.
  // Nested FlatBuffers are stored in a ubyte vector, which can be convenient
  // since they can be memcpy'd around much easier than other FlatBuffer
  // values. They have little overhead compared to storing the table directly.
  // As a test, create a mostly empty Monster buffer:
  flatbuffers::FlatBufferBuilder nested_builder;
  auto nmloc = CreateMonster(nested_builder, nullptr, 0, 0,
                             nested_builder.CreateString("NestedMonster"));
  FinishMonsterBuffer(nested_builder, nmloc);
  // Now we can store the buffer in the parent. Note that by default, vectors
  // are only aligned to their elements or size field, so in this case if the
  // buffer contains 64-bit elements, they may not be correctly aligned. We fix
  // that with:
  builder.ForceVectorAlignment(nested_builder.GetSize(), sizeof(uint8_t),
                               nested_builder.GetBufferMinAlignment());
  // If for whatever reason you don't have the nested_builder available, you
  // can substitute flatbuffers::largest_scalar_t (64-bit) for the alignment, or
  // the largest force_align value in your schema if you're using it.
  auto nested_flatbuffer_vector = builder.CreateVector(
      nested_builder.GetBufferPointer(), nested_builder.GetSize());

  // Test a nested FlexBuffer:
  flexbuffers::Builder flexbuild;
  flexbuild.Int(1234);
  flexbuild.Finish();
  auto flex = builder.CreateVector(flexbuild.GetBuffer());

  // shortcut for creating monster with all fields set:
  auto mloc = CreateMonster(builder, &vec, 150, 80, name, inventory, Color_Blue,
                            Any_Monster, mlocs[1].Union(),  // Store a union.
                            testv, vecofstrings, vecoftables, 0,
                            nested_flatbuffer_vector, 0, false, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 3.14159f, 3.0f, 0.0f, vecofstrings2,
                            vecofstructs, flex, testv2);

  FinishMonsterBuffer(builder, mloc);

  // clang-format off
  #ifdef FLATBUFFERS_TEST_VERBOSE
  // print byte data for debugging:
  auto p = builder.GetBufferPointer();
  for (flatbuffers::uoffset_t i = 0; i < builder.GetSize(); i++)
    printf("%d ", p[i]);
  #endif
  // clang-format on

  // return the buffer for the caller to use.
  auto bufferpointer =
      reinterpret_cast<const char *>(builder.GetBufferPointer());
  buffer.assign(bufferpointer, bufferpointer + builder.GetSize());

  return builder.ReleaseBufferPointer();
}

//  example of accessing a buffer loaded in memory:
void AccessFlatBufferTest(const uint8_t *flatbuf, size_t length,
                          bool pooled = true) {
  // First, verify the buffers integrity (optional)
  flatbuffers::Verifier verifier(flatbuf, length);
  TEST_EQ(VerifyMonsterBuffer(verifier), true);

  std::vector<uint8_t> test_buff;
  test_buff.resize(length * 2);
  std::memcpy(&test_buff[0], flatbuf, length);
  std::memcpy(&test_buff[length], flatbuf, length);

  flatbuffers::Verifier verifier1(&test_buff[0], length);
  TEST_EQ(VerifyMonsterBuffer(verifier1), true);
  TEST_EQ(verifier1.GetComputedSize(), length);

  flatbuffers::Verifier verifier2(&test_buff[length], length);
  TEST_EQ(VerifyMonsterBuffer(verifier2), true);
  TEST_EQ(verifier2.GetComputedSize(), length);

  TEST_EQ(strcmp(MonsterIdentifier(), "MONS"), 0);
  TEST_EQ(MonsterBufferHasIdentifier(flatbuf), true);
  TEST_EQ(strcmp(MonsterExtension(), "mon"), 0);

  // Access the buffer from the root.
  auto monster = GetMonster(flatbuf);

  TEST_EQ(monster->hp(), 80);
  TEST_EQ(monster->mana(), 150);  // default
  TEST_EQ_STR(monster->name()->c_str(), "MyMonster");
  // Can't access the following field, it is deprecated in the schema,
  // which means accessors are not generated:
  // monster.friendly()

  auto pos = monster->pos();
  TEST_NOTNULL(pos);
  TEST_EQ(pos->z(), 3);
  TEST_EQ(pos->test3().a(), 10);
  TEST_EQ(pos->test3().b(), 20);

  auto inventory = monster->inventory();
  TEST_EQ(VectorLength(inventory), 10UL);  // Works even if inventory is null.
  TEST_NOTNULL(inventory);
  unsigned char inv_data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  for (auto it = inventory->begin(); it != inventory->end(); ++it)
    TEST_EQ(*it, inv_data[it - inventory->begin()]);

  TEST_EQ(monster->color(), Color_Blue);

  // Example of accessing a union:
  TEST_EQ(monster->test_type(), Any_Monster);  // First make sure which it is.
  auto monster2 = reinterpret_cast<const Monster *>(monster->test());
  TEST_NOTNULL(monster2);
  TEST_EQ_STR(monster2->name()->c_str(), "Fred");

  // Example of accessing a vector of strings:
  auto vecofstrings = monster->testarrayofstring();
  TEST_EQ(vecofstrings->Length(), 4U);
  TEST_EQ_STR(vecofstrings->Get(0)->c_str(), "bob");
  TEST_EQ_STR(vecofstrings->Get(1)->c_str(), "fred");
  if (pooled) {
    // These should have pointer equality because of string pooling.
    TEST_EQ(vecofstrings->Get(0)->c_str(), vecofstrings->Get(2)->c_str());
    TEST_EQ(vecofstrings->Get(1)->c_str(), vecofstrings->Get(3)->c_str());
  }

  auto vecofstrings2 = monster->testarrayofstring2();
  if (vecofstrings2) {
    TEST_EQ(vecofstrings2->Length(), 2U);
    TEST_EQ_STR(vecofstrings2->Get(0)->c_str(), "jane");
    TEST_EQ_STR(vecofstrings2->Get(1)->c_str(), "mary");
  }

  // Example of accessing a vector of tables:
  auto vecoftables = monster->testarrayoftables();
  TEST_EQ(vecoftables->Length(), 3U);
  for (auto it = vecoftables->begin(); it != vecoftables->end(); ++it)
    TEST_EQ(strlen(it->name()->c_str()) >= 4, true);
  TEST_EQ_STR(vecoftables->Get(0)->name()->c_str(), "Barney");
  TEST_EQ(vecoftables->Get(0)->hp(), 1000);
  TEST_EQ_STR(vecoftables->Get(1)->name()->c_str(), "Fred");
  TEST_EQ_STR(vecoftables->Get(2)->name()->c_str(), "Wilma");
  TEST_NOTNULL(vecoftables->LookupByKey("Barney"));
  TEST_NOTNULL(vecoftables->LookupByKey("Fred"));
  TEST_NOTNULL(vecoftables->LookupByKey("Wilma"));

  // Test accessing a vector of sorted structs
  auto vecofstructs = monster->testarrayofsortedstruct();
  if (vecofstructs) {  // not filled in monster_test.bfbs
    for (flatbuffers::uoffset_t i = 0; i < vecofstructs->size() - 1; i++) {
      auto left = vecofstructs->Get(i);
      auto right = vecofstructs->Get(i + 1);
      TEST_EQ(true, (left->KeyCompareLessThan(right)));
    }
    TEST_NOTNULL(vecofstructs->LookupByKey(3));
    TEST_EQ(static_cast<const Ability *>(nullptr),
            vecofstructs->LookupByKey(5));
  }

  // Test nested FlatBuffers if available:
  auto nested_buffer = monster->testnestedflatbuffer();
  if (nested_buffer) {
    // nested_buffer is a vector of bytes you can memcpy. However, if you
    // actually want to access the nested data, this is a convenient
    // accessor that directly gives you the root table:
    auto nested_monster = monster->testnestedflatbuffer_nested_root();
    TEST_EQ_STR(nested_monster->name()->c_str(), "NestedMonster");
  }

  // Test flexbuffer if available:
  auto flex = monster->flex();
  // flex is a vector of bytes you can memcpy etc.
  TEST_EQ(flex->size(), 4);  // Encoded FlexBuffer bytes.
  // However, if you actually want to access the nested data, this is a
  // convenient accessor that directly gives you the root value:
  TEST_EQ(monster->flex_flexbuffer_root().AsInt16(), 1234);

  // Since Flatbuffers uses explicit mechanisms to override the default
  // compiler alignment, double check that the compiler indeed obeys them:
  // (Test consists of a short and byte):
  TEST_EQ(flatbuffers::AlignOf<Test>(), 2UL);
  TEST_EQ(sizeof(Test), 4UL);

  const flatbuffers::Vector<const Test *> *tests_array[] = {
    monster->test4(),
    monster->test5(),
  };
  for (size_t i = 0; i < sizeof(tests_array) / sizeof(tests_array[0]); ++i) {
    auto tests = tests_array[i];
    TEST_NOTNULL(tests);
    auto test_0 = tests->Get(0);
    auto test_1 = tests->Get(1);
    TEST_EQ(test_0->a(), 10);
    TEST_EQ(test_0->b(), 20);
    TEST_EQ(test_1->a(), 30);
    TEST_EQ(test_1->b(), 40);
    for (auto it = tests->begin(); it != tests->end(); ++it) {
      TEST_EQ(it->a() == 10 || it->a() == 30, true);  // Just testing iterators.
    }
  }

  // Checking for presence of fields:
  TEST_EQ(flatbuffers::IsFieldPresent(monster, Monster::VT_HP), true);
  TEST_EQ(flatbuffers::IsFieldPresent(monster, Monster::VT_MANA), false);

  // Obtaining a buffer from a root:
  TEST_EQ(GetBufferStartFromRootPointer(monster), flatbuf);
}

// Change a FlatBuffer in-place, after it has been constructed.
void MutateFlatBuffersTest(uint8_t *flatbuf, std::size_t length) {
  // Get non-const pointer to root.
  auto monster = GetMutableMonster(flatbuf);

  // Each of these tests mutates, then tests, then set back to the original,
  // so we can test that the buffer in the end still passes our original test.
  auto hp_ok = monster->mutate_hp(10);
  TEST_EQ(hp_ok, true);  // Field was present.
  TEST_EQ(monster->hp(), 10);
  // Mutate to default value
  auto hp_ok_default = monster->mutate_hp(100);
  TEST_EQ(hp_ok_default, true);  // Field was present.
  TEST_EQ(monster->hp(), 100);
  // Test that mutate to default above keeps field valid for further mutations
  auto hp_ok_2 = monster->mutate_hp(20);
  TEST_EQ(hp_ok_2, true);
  TEST_EQ(monster->hp(), 20);
  monster->mutate_hp(80);

  // Monster originally at 150 mana (default value)
  auto mana_default_ok = monster->mutate_mana(150);  // Mutate to default value.
  TEST_EQ(mana_default_ok,
          true);  // Mutation should succeed, because default value.
  TEST_EQ(monster->mana(), 150);
  auto mana_ok = monster->mutate_mana(10);
  TEST_EQ(mana_ok, false);  // Field was NOT present, because default value.
  TEST_EQ(monster->mana(), 150);

  // Mutate structs.
  auto pos = monster->mutable_pos();
  auto test3 = pos->mutable_test3();  // Struct inside a struct.
  test3.mutate_a(50);                 // Struct fields never fail.
  TEST_EQ(test3.a(), 50);
  test3.mutate_a(10);

  // Mutate vectors.
  auto inventory = monster->mutable_inventory();
  inventory->Mutate(9, 100);
  TEST_EQ(inventory->Get(9), 100);
  inventory->Mutate(9, 9);

  auto tables = monster->mutable_testarrayoftables();
  auto first = tables->GetMutableObject(0);
  TEST_EQ(first->hp(), 1000);
  first->mutate_hp(0);
  TEST_EQ(first->hp(), 0);
  first->mutate_hp(1000);

  // Run the verifier and the regular test to make sure we didn't trample on
  // anything.
  AccessFlatBufferTest(flatbuf, length);
}

// Unpack a FlatBuffer into objects.
void ObjectFlatBuffersTest(uint8_t *flatbuf) {
  // Optional: we can specify resolver and rehasher functions to turn hashed
  // strings into object pointers and back, to implement remote references
  // and such.
  auto resolver = flatbuffers::resolver_function_t(
      [](void **pointer_adr, flatbuffers::hash_value_t hash) {
        (void)pointer_adr;
        (void)hash;
        // Don't actually do anything, leave variable null.
      });
  auto rehasher = flatbuffers::rehasher_function_t(
      [](void *pointer) -> flatbuffers::hash_value_t {
        (void)pointer;
        return 0;
      });

  // Turn a buffer into C++ objects.
  auto monster1 = UnPackMonster(flatbuf, &resolver);

  // Re-serialize the data.
  flatbuffers::FlatBufferBuilder fbb1;
  fbb1.Finish(CreateMonster(fbb1, monster1.get(), &rehasher),
              MonsterIdentifier());

  // Unpack again, and re-serialize again.
  auto monster2 = UnPackMonster(fbb1.GetBufferPointer(), &resolver);
  flatbuffers::FlatBufferBuilder fbb2;
  fbb2.Finish(CreateMonster(fbb2, monster2.get(), &rehasher),
              MonsterIdentifier());

  // Now we've gone full round-trip, the two buffers should match.
  auto len1 = fbb1.GetSize();
  auto len2 = fbb2.GetSize();
  TEST_EQ(len1, len2);
  TEST_EQ(memcmp(fbb1.GetBufferPointer(), fbb2.GetBufferPointer(), len1), 0);

  // Test it with the original buffer test to make sure all data survived.
  AccessFlatBufferTest(fbb2.GetBufferPointer(), len2, false);

  // Test accessing fields, similar to AccessFlatBufferTest above.
  TEST_EQ(monster2->hp, 80);
  TEST_EQ(monster2->mana, 150);  // default
  TEST_EQ_STR(monster2->name.c_str(), "MyMonster");

  auto &pos = monster2->pos;
  TEST_NOTNULL(pos);
  TEST_EQ(pos->z(), 3);
  TEST_EQ(pos->test3().a(), 10);
  TEST_EQ(pos->test3().b(), 20);

  auto &inventory = monster2->inventory;
  TEST_EQ(inventory.size(), 10UL);
  unsigned char inv_data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  for (auto it = inventory.begin(); it != inventory.end(); ++it)
    TEST_EQ(*it, inv_data[it - inventory.begin()]);

  TEST_EQ(monster2->color, Color_Blue);

  auto monster3 = monster2->test.AsMonster();
  TEST_NOTNULL(monster3);
  TEST_EQ_STR(monster3->name.c_str(), "Fred");

  auto &vecofstrings = monster2->testarrayofstring;
  TEST_EQ(vecofstrings.size(), 4U);
  TEST_EQ_STR(vecofstrings[0].c_str(), "bob");
  TEST_EQ_STR(vecofstrings[1].c_str(), "fred");

  auto &vecofstrings2 = monster2->testarrayofstring2;
  TEST_EQ(vecofstrings2.size(), 2U);
  TEST_EQ_STR(vecofstrings2[0].c_str(), "jane");
  TEST_EQ_STR(vecofstrings2[1].c_str(), "mary");

  auto &vecoftables = monster2->testarrayoftables;
  TEST_EQ(vecoftables.size(), 3U);
  TEST_EQ_STR(vecoftables[0]->name.c_str(), "Barney");
  TEST_EQ(vecoftables[0]->hp, 1000);
  TEST_EQ_STR(vecoftables[1]->name.c_str(), "Fred");
  TEST_EQ_STR(vecoftables[2]->name.c_str(), "Wilma");

  auto &tests = monster2->test4;
  TEST_EQ(tests[0].a(), 10);
  TEST_EQ(tests[0].b(), 20);
  TEST_EQ(tests[1].a(), 30);
  TEST_EQ(tests[1].b(), 40);
}

// Prefix a FlatBuffer with a size field.
void SizePrefixedTest() {
  // Create size prefixed buffer.
  flatbuffers::FlatBufferBuilder fbb;
  fbb.FinishSizePrefixed(
      CreateMonster(fbb, 0, 200, 300, fbb.CreateString("bob")));

  // Verify it.
  flatbuffers::Verifier verifier(fbb.GetBufferPointer(), fbb.GetSize());
  TEST_EQ(verifier.VerifySizePrefixedBuffer<Monster>(nullptr), true);

  // Access it.
  auto m = flatbuffers::GetSizePrefixedRoot<MyGame::Example::Monster>(
      fbb.GetBufferPointer());
  TEST_EQ(m->mana(), 200);
  TEST_EQ(m->hp(), 300);
  TEST_EQ_STR(m->name()->c_str(), "bob");
}

void TriviallyCopyableTest() {
  // clang-format off
  #if __GNUG__ && __GNUC__ < 5
    TEST_EQ(__has_trivial_copy(Vec3), true);
  #else
    #if __cplusplus >= 201103L
      TEST_EQ(std::is_trivially_copyable<Vec3>::value, true);
    #endif
  #endif
  // clang-format on
}

// Check stringify of an default enum value to json
void JsonDefaultTest() {
  // load FlatBuffer schema (.fbs) from disk
  std::string schemafile;
  TEST_EQ(flatbuffers::LoadFile((test_data_path + "monster_test.fbs").c_str(),
                                false, &schemafile), true);
  // parse schema first, so we can use it to parse the data after
  flatbuffers::Parser parser;
  auto include_test_path =
      flatbuffers::ConCatPathFileName(test_data_path, "include_test");
  const char *include_directories[] = { test_data_path.c_str(),
                                        include_test_path.c_str(), nullptr };

  TEST_EQ(parser.Parse(schemafile.c_str(), include_directories), true);
  // create incomplete monster and store to json
  parser.opts.output_default_scalars_in_json = true;
  parser.opts.output_enum_identifiers = true;
  flatbuffers::FlatBufferBuilder builder;
  auto name = builder.CreateString("default_enum");
  MonsterBuilder color_monster(builder);
  color_monster.add_name(name);
  FinishMonsterBuffer(builder, color_monster.Finish());
  std::string jsongen;
  auto result = GenerateText(parser, builder.GetBufferPointer(), &jsongen);
  TEST_EQ(result, true);
  // default value of the "color" field is Blue
  TEST_EQ(std::string::npos != jsongen.find("color: \"Blue\""), true);
  // default value of the "testf" field is 3.14159
  TEST_EQ(std::string::npos != jsongen.find("testf: 3.14159"), true);
}

// example of parsing text straight into a buffer, and generating
// text back from it:
void ParseAndGenerateTextTest() {
  // load FlatBuffer schema (.fbs) and JSON from disk
  std::string schemafile;
  std::string jsonfile;
  TEST_EQ(flatbuffers::LoadFile((test_data_path + "monster_test.fbs").c_str(),
                                false, &schemafile),
          true);
  TEST_EQ(flatbuffers::LoadFile(
              (test_data_path + "monsterdata_test.golden").c_str(), false,
              &jsonfile),
          true);

  // parse schema first, so we can use it to parse the data after
  flatbuffers::Parser parser;
  auto include_test_path =
      flatbuffers::ConCatPathFileName(test_data_path, "include_test");
  const char *include_directories[] = { test_data_path.c_str(),
                                        include_test_path.c_str(), nullptr };
  TEST_EQ(parser.Parse(schemafile.c_str(), include_directories), true);
  TEST_EQ(parser.Parse(jsonfile.c_str(), include_directories), true);

  // here, parser.builder_ contains a binary buffer that is the parsed data.

  // First, verify it, just in case:
  flatbuffers::Verifier verifier(parser.builder_.GetBufferPointer(),
                                 parser.builder_.GetSize());
  TEST_EQ(VerifyMonsterBuffer(verifier), true);

  AccessFlatBufferTest(parser.builder_.GetBufferPointer(),
                       parser.builder_.GetSize(), false);

  // to ensure it is correct, we now generate text back from the binary,
  // and compare the two:
  std::string jsongen;
  auto result =
      GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen);
  TEST_EQ(result, true);
  TEST_EQ_STR(jsongen.c_str(), jsonfile.c_str());

  // We can also do the above using the convenient Registry that knows about
  // a set of file_identifiers mapped to schemas.
  flatbuffers::Registry registry;
  // Make sure schemas can find their includes.
  registry.AddIncludeDirectory(test_data_path.c_str());
  registry.AddIncludeDirectory(include_test_path.c_str());
  // Call this with many schemas if possible.
  registry.Register(MonsterIdentifier(),
                    (test_data_path + "monster_test.fbs").c_str());
  // Now we got this set up, we can parse by just specifying the identifier,
  // the correct schema will be loaded on the fly:
  auto buf = registry.TextToFlatBuffer(jsonfile.c_str(), MonsterIdentifier());
  // If this fails, check registry.lasterror_.
  TEST_NOTNULL(buf.data());
  // Test the buffer, to be sure:
  AccessFlatBufferTest(buf.data(), buf.size(), false);
  // We can use the registry to turn this back into text, in this case it
  // will get the file_identifier from the binary:
  std::string text;
  auto ok = registry.FlatBufferToText(buf.data(), buf.size(), &text);
  // If this fails, check registry.lasterror_.
  TEST_EQ(ok, true);
  TEST_EQ_STR(text.c_str(), jsonfile.c_str());
}

void ReflectionTest(uint8_t *flatbuf, size_t length) {
  // Load a binary schema.
  std::string bfbsfile;
  TEST_EQ(flatbuffers::LoadFile((test_data_path + "monster_test.bfbs").c_str(),
                                true, &bfbsfile),
          true);

  // Verify it, just in case:
  flatbuffers::Verifier verifier(
      reinterpret_cast<const uint8_t *>(bfbsfile.c_str()), bfbsfile.length());
  TEST_EQ(reflection::VerifySchemaBuffer(verifier), true);

  // Make sure the schema is what we expect it to be.
  auto &schema = *reflection::GetSchema(bfbsfile.c_str());
  auto root_table = schema.root_table();
  TEST_EQ_STR(root_table->name()->c_str(), "MyGame.Example.Monster");
  auto fields = root_table->fields();
  auto hp_field_ptr = fields->LookupByKey("hp");
  TEST_NOTNULL(hp_field_ptr);
  auto &hp_field = *hp_field_ptr;
  TEST_EQ_STR(hp_field.name()->c_str(), "hp");
  TEST_EQ(hp_field.id(), 2);
  TEST_EQ(hp_field.type()->base_type(), reflection::Short);
  auto friendly_field_ptr = fields->LookupByKey("friendly");
  TEST_NOTNULL(friendly_field_ptr);
  TEST_NOTNULL(friendly_field_ptr->attributes());
  TEST_NOTNULL(friendly_field_ptr->attributes()->LookupByKey("priority"));

  // Make sure the table index is what we expect it to be.
  auto pos_field_ptr = fields->LookupByKey("pos");
  TEST_NOTNULL(pos_field_ptr);
  TEST_EQ(pos_field_ptr->type()->base_type(), reflection::Obj);
  auto pos_table_ptr = schema.objects()->Get(pos_field_ptr->type()->index());
  TEST_NOTNULL(pos_table_ptr);
  TEST_EQ_STR(pos_table_ptr->name()->c_str(), "MyGame.Example.Vec3");

  // Now use it to dynamically access a buffer.
  auto &root = *flatbuffers::GetAnyRoot(flatbuf);

  // Verify the buffer first using reflection based verification
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), flatbuf, length),
          true);

  auto hp = flatbuffers::GetFieldI<uint16_t>(root, hp_field);
  TEST_EQ(hp, 80);

  // Rather than needing to know the type, we can also get the value of
  // any field as an int64_t/double/string, regardless of what it actually is.
  auto hp_int64 = flatbuffers::GetAnyFieldI(root, hp_field);
  TEST_EQ(hp_int64, 80);
  auto hp_double = flatbuffers::GetAnyFieldF(root, hp_field);
  TEST_EQ(hp_double, 80.0);
  auto hp_string = flatbuffers::GetAnyFieldS(root, hp_field, &schema);
  TEST_EQ_STR(hp_string.c_str(), "80");

  // Get struct field through reflection
  auto pos_struct = flatbuffers::GetFieldStruct(root, *pos_field_ptr);
  TEST_NOTNULL(pos_struct);
  TEST_EQ(flatbuffers::GetAnyFieldF(*pos_struct,
                                    *pos_table_ptr->fields()->LookupByKey("z")),
          3.0f);

  auto test3_field = pos_table_ptr->fields()->LookupByKey("test3");
  auto test3_struct = flatbuffers::GetFieldStruct(*pos_struct, *test3_field);
  TEST_NOTNULL(test3_struct);
  auto test3_object = schema.objects()->Get(test3_field->type()->index());

  TEST_EQ(flatbuffers::GetAnyFieldF(*test3_struct,
                                    *test3_object->fields()->LookupByKey("a")),
          10);

  // We can also modify it.
  flatbuffers::SetField<uint16_t>(&root, hp_field, 200);
  hp = flatbuffers::GetFieldI<uint16_t>(root, hp_field);
  TEST_EQ(hp, 200);

  // We can also set fields generically:
  flatbuffers::SetAnyFieldI(&root, hp_field, 300);
  hp_int64 = flatbuffers::GetAnyFieldI(root, hp_field);
  TEST_EQ(hp_int64, 300);
  flatbuffers::SetAnyFieldF(&root, hp_field, 300.5);
  hp_int64 = flatbuffers::GetAnyFieldI(root, hp_field);
  TEST_EQ(hp_int64, 300);
  flatbuffers::SetAnyFieldS(&root, hp_field, "300");
  hp_int64 = flatbuffers::GetAnyFieldI(root, hp_field);
  TEST_EQ(hp_int64, 300);

  // Test buffer is valid after the modifications
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(), flatbuf, length),
          true);

  // Reset it, for further tests.
  flatbuffers::SetField<uint16_t>(&root, hp_field, 80);

  // More advanced functionality: changing the size of items in-line!
  // First we put the FlatBuffer inside an std::vector.
  std::vector<uint8_t> resizingbuf(flatbuf, flatbuf + length);
  // Find the field we want to modify.
  auto &name_field = *fields->LookupByKey("name");
  // Get the root.
  // This time we wrap the result from GetAnyRoot in a smartpointer that
  // will keep rroot valid as resizingbuf resizes.
  auto rroot = flatbuffers::piv(
      flatbuffers::GetAnyRoot(flatbuffers::vector_data(resizingbuf)),
      resizingbuf);
  SetString(schema, "totally new string", GetFieldS(**rroot, name_field),
            &resizingbuf);
  // Here resizingbuf has changed, but rroot is still valid.
  TEST_EQ_STR(GetFieldS(**rroot, name_field)->c_str(), "totally new string");
  // Now lets extend a vector by 100 elements (10 -> 110).
  auto &inventory_field = *fields->LookupByKey("inventory");
  auto rinventory = flatbuffers::piv(
      flatbuffers::GetFieldV<uint8_t>(**rroot, inventory_field), resizingbuf);
  flatbuffers::ResizeVector<uint8_t>(schema, 110, 50, *rinventory,
                                     &resizingbuf);
  // rinventory still valid, so lets read from it.
  TEST_EQ(rinventory->Get(10), 50);

  // For reflection uses not covered already, there is a more powerful way:
  // we can simply generate whatever object we want to add/modify in a
  // FlatBuffer of its own, then add that to an existing FlatBuffer:
  // As an example, let's add a string to an array of strings.
  // First, find our field:
  auto &testarrayofstring_field = *fields->LookupByKey("testarrayofstring");
  // Find the vector value:
  auto rtestarrayofstring = flatbuffers::piv(
      flatbuffers::GetFieldV<flatbuffers::Offset<flatbuffers::String>>(
          **rroot, testarrayofstring_field),
      resizingbuf);
  // It's a vector of 2 strings, to which we add one more, initialized to
  // offset 0.
  flatbuffers::ResizeVector<flatbuffers::Offset<flatbuffers::String>>(
      schema, 3, 0, *rtestarrayofstring, &resizingbuf);
  // Here we just create a buffer that contans a single string, but this
  // could also be any complex set of tables and other values.
  flatbuffers::FlatBufferBuilder stringfbb;
  stringfbb.Finish(stringfbb.CreateString("hank"));
  // Add the contents of it to our existing FlatBuffer.
  // We do this last, so the pointer doesn't get invalidated (since it is
  // at the end of the buffer):
  auto string_ptr = flatbuffers::AddFlatBuffer(
      resizingbuf, stringfbb.GetBufferPointer(), stringfbb.GetSize());
  // Finally, set the new value in the vector.
  rtestarrayofstring->MutateOffset(2, string_ptr);
  TEST_EQ_STR(rtestarrayofstring->Get(0)->c_str(), "bob");
  TEST_EQ_STR(rtestarrayofstring->Get(2)->c_str(), "hank");
  // Test integrity of all resize operations above.
  flatbuffers::Verifier resize_verifier(
      reinterpret_cast<const uint8_t *>(flatbuffers::vector_data(resizingbuf)),
      resizingbuf.size());
  TEST_EQ(VerifyMonsterBuffer(resize_verifier), true);

  // Test buffer is valid using reflection as well
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(),
                              flatbuffers::vector_data(resizingbuf),
                              resizingbuf.size()),
          true);

  // As an additional test, also set it on the name field.
  // Note: unlike the name change above, this just overwrites the offset,
  // rather than changing the string in-place.
  SetFieldT(*rroot, name_field, string_ptr);
  TEST_EQ_STR(GetFieldS(**rroot, name_field)->c_str(), "hank");

  // Using reflection, rather than mutating binary FlatBuffers, we can also copy
  // tables and other things out of other FlatBuffers into a FlatBufferBuilder,
  // either part or whole.
  flatbuffers::FlatBufferBuilder fbb;
  auto root_offset = flatbuffers::CopyTable(
      fbb, schema, *root_table, *flatbuffers::GetAnyRoot(flatbuf), true);
  fbb.Finish(root_offset, MonsterIdentifier());
  // Test that it was copied correctly:
  AccessFlatBufferTest(fbb.GetBufferPointer(), fbb.GetSize());

  // Test buffer is valid using reflection as well
  TEST_EQ(flatbuffers::Verify(schema, *schema.root_table(),
                              fbb.GetBufferPointer(), fbb.GetSize()),
          true);
}

void MiniReflectFlatBuffersTest(uint8_t *flatbuf) {
  auto s = flatbuffers::FlatBufferToString(flatbuf, MonsterTypeTable());
  TEST_EQ_STR(
      s.c_str(),
      "{ "
      "pos: { x: 1.0, y: 2.0, z: 3.0, test1: 0.0, test2: Red, test3: "
      "{ a: 10, b: 20 } }, "
      "hp: 80, "
      "name: \"MyMonster\", "
      "inventory: [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ], "
      "test_type: Monster, "
      "test: { name: \"Fred\" }, "
      "test4: [ { a: 10, b: 20 }, { a: 30, b: 40 } ], "
      "testarrayofstring: [ \"bob\", \"fred\", \"bob\", \"fred\" ], "
      "testarrayoftables: [ { hp: 1000, name: \"Barney\" }, { name: \"Fred\" "
      "}, "
      "{ name: \"Wilma\" } ], "
      // TODO(wvo): should really print this nested buffer correctly.
      "testnestedflatbuffer: [ 20, 0, 0, 0, 77, 79, 78, 83, 12, 0, 12, 0, 0, "
      "0, "
      "4, 0, 6, 0, 8, 0, 12, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 13, 0, 0, 0, 78, "
      "101, 115, 116, 101, 100, 77, 111, 110, 115, 116, 101, 114, 0, 0, 0 ], "
      "testarrayofstring2: [ \"jane\", \"mary\" ], "
      "testarrayofsortedstruct: [ { id: 1, distance: 10 }, "
      "{ id: 2, distance: 20 }, { id: 3, distance: 30 }, "
      "{ id: 4, distance: 40 } ], "
      "flex: [ 210, 4, 5, 2 ], "
      "test5: [ { a: 10, b: 20 }, { a: 30, b: 40 } ] "
      "}");
}

// Parse a .proto schema, output as .fbs
void ParseProtoTest() {
  // load the .proto and the golden file from disk
  std::string protofile;
  std::string goldenfile;
  TEST_EQ(
      flatbuffers::LoadFile((test_data_path + "prototest/test.proto").c_str(),
                            false, &protofile),
      true);
  TEST_EQ(
      flatbuffers::LoadFile((test_data_path + "prototest/test.golden").c_str(),
                            false, &goldenfile),
      true);

  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = false;
  opts.proto_mode = true;

  // Parse proto.
  flatbuffers::Parser parser(opts);
  auto protopath = test_data_path + "prototest/";
  const char *include_directories[] = { protopath.c_str(), nullptr };
  TEST_EQ(parser.Parse(protofile.c_str(), include_directories), true);

  // Generate fbs.
  auto fbs = flatbuffers::GenerateFBS(parser, "test");

  // Ensure generated file is parsable.
  flatbuffers::Parser parser2;
  TEST_EQ(parser2.Parse(fbs.c_str(), nullptr), true);
  TEST_EQ_STR(fbs.c_str(), goldenfile.c_str());
}

template<typename T>
void CompareTableFieldValue(flatbuffers::Table *table,
                            flatbuffers::voffset_t voffset, T val) {
  T read = table->GetField(voffset, static_cast<T>(0));
  TEST_EQ(read, val);
}

// Low level stress/fuzz test: serialize/deserialize a variety of
// different kinds of data in different combinations
void FuzzTest1() {
  // Values we're testing against: chosen to ensure no bits get chopped
  // off anywhere, and also be different from eachother.
  const uint8_t bool_val = true;
  const int8_t char_val = -127;  // 0x81
  const uint8_t uchar_val = 0xFF;
  const int16_t short_val = -32222;  // 0x8222;
  const uint16_t ushort_val = 0xFEEE;
  const int32_t int_val = 0x83333333;
  const uint32_t uint_val = 0xFDDDDDDD;
  const int64_t long_val = 0x8444444444444444LL;
  const uint64_t ulong_val = 0xFCCCCCCCCCCCCCCCULL;
  const float float_val = 3.14159f;
  const double double_val = 3.14159265359;

  const int test_values_max = 11;
  const flatbuffers::voffset_t fields_per_object = 4;
  const int num_fuzz_objects = 10000;  // The higher, the more thorough :)

  flatbuffers::FlatBufferBuilder builder;

  lcg_reset();  // Keep it deterministic.

  flatbuffers::uoffset_t objects[num_fuzz_objects];

  // Generate num_fuzz_objects random objects each consisting of
  // fields_per_object fields, each of a random type.
  for (int i = 0; i < num_fuzz_objects; i++) {
    auto start = builder.StartTable();
    for (flatbuffers::voffset_t f = 0; f < fields_per_object; f++) {
      int choice = lcg_rand() % test_values_max;
      auto off = flatbuffers::FieldIndexToOffset(f);
      switch (choice) {
        case 0: builder.AddElement<uint8_t>(off, bool_val, 0); break;
        case 1: builder.AddElement<int8_t>(off, char_val, 0); break;
        case 2: builder.AddElement<uint8_t>(off, uchar_val, 0); break;
        case 3: builder.AddElement<int16_t>(off, short_val, 0); break;
        case 4: builder.AddElement<uint16_t>(off, ushort_val, 0); break;
        case 5: builder.AddElement<int32_t>(off, int_val, 0); break;
        case 6: builder.AddElement<uint32_t>(off, uint_val, 0); break;
        case 7: builder.AddElement<int64_t>(off, long_val, 0); break;
        case 8: builder.AddElement<uint64_t>(off, ulong_val, 0); break;
        case 9: builder.AddElement<float>(off, float_val, 0); break;
        case 10: builder.AddElement<double>(off, double_val, 0); break;
      }
    }
    objects[i] = builder.EndTable(start);
  }
  builder.PreAlign<flatbuffers::largest_scalar_t>(0);  // Align whole buffer.

  lcg_reset();  // Reset.

  uint8_t *eob = builder.GetCurrentBufferPointer() + builder.GetSize();

  // Test that all objects we generated are readable and return the
  // expected values. We generate random objects in the same order
  // so this is deterministic.
  for (int i = 0; i < num_fuzz_objects; i++) {
    auto table = reinterpret_cast<flatbuffers::Table *>(eob - objects[i]);
    for (flatbuffers::voffset_t f = 0; f < fields_per_object; f++) {
      int choice = lcg_rand() % test_values_max;
      flatbuffers::voffset_t off = flatbuffers::FieldIndexToOffset(f);
      switch (choice) {
        case 0: CompareTableFieldValue(table, off, bool_val); break;
        case 1: CompareTableFieldValue(table, off, char_val); break;
        case 2: CompareTableFieldValue(table, off, uchar_val); break;
        case 3: CompareTableFieldValue(table, off, short_val); break;
        case 4: CompareTableFieldValue(table, off, ushort_val); break;
        case 5: CompareTableFieldValue(table, off, int_val); break;
        case 6: CompareTableFieldValue(table, off, uint_val); break;
        case 7: CompareTableFieldValue(table, off, long_val); break;
        case 8: CompareTableFieldValue(table, off, ulong_val); break;
        case 9: CompareTableFieldValue(table, off, float_val); break;
        case 10: CompareTableFieldValue(table, off, double_val); break;
      }
    }
  }
}

// High level stress/fuzz test: generate a big schema and
// matching json data in random combinations, then parse both,
// generate json back from the binary, and compare with the original.
void FuzzTest2() {
  lcg_reset();  // Keep it deterministic.

  const int num_definitions = 30;
  const int num_struct_definitions = 5;  // Subset of num_definitions.
  const int fields_per_definition = 15;
  const int instances_per_definition = 5;
  const int deprecation_rate = 10;  // 1 in deprecation_rate fields will
                                    // be deprecated.

  std::string schema = "namespace test;\n\n";

  struct RndDef {
    std::string instances[instances_per_definition];

    // Since we're generating schema and corresponding data in tandem,
    // this convenience function adds strings to both at once.
    static void Add(RndDef (&definitions_l)[num_definitions],
                    std::string &schema_l, const int instances_per_definition_l,
                    const char *schema_add, const char *instance_add,
                    int definition) {
      schema_l += schema_add;
      for (int i = 0; i < instances_per_definition_l; i++)
        definitions_l[definition].instances[i] += instance_add;
    }
  };

  // clang-format off
  #define AddToSchemaAndInstances(schema_add, instance_add) \
    RndDef::Add(definitions, schema, instances_per_definition, \
                schema_add, instance_add, definition)

  #define Dummy() \
    RndDef::Add(definitions, schema, instances_per_definition, \
                "byte", "1", definition)
  // clang-format on

  RndDef definitions[num_definitions];

  // We are going to generate num_definitions, the first
  // num_struct_definitions will be structs, the rest tables. For each
  // generate random fields, some of which may be struct/table types
  // referring to previously generated structs/tables.
  // Simultanenously, we generate instances_per_definition JSON data
  // definitions, which will have identical structure to the schema
  // being generated. We generate multiple instances such that when creating
  // hierarchy, we get some variety by picking one randomly.
  for (int definition = 0; definition < num_definitions; definition++) {
    std::string definition_name = "D" + flatbuffers::NumToString(definition);

    bool is_struct = definition < num_struct_definitions;

    AddToSchemaAndInstances(
        ((is_struct ? "struct " : "table ") + definition_name + " {\n").c_str(),
        "{\n");

    for (int field = 0; field < fields_per_definition; field++) {
      const bool is_last_field = field == fields_per_definition - 1;

      // Deprecate 1 in deprecation_rate fields. Only table fields can be
      // deprecated.
      // Don't deprecate the last field to avoid dangling commas in JSON.
      const bool deprecated =
          !is_struct && !is_last_field && (lcg_rand() % deprecation_rate == 0);

      std::string field_name = "f" + flatbuffers::NumToString(field);
      AddToSchemaAndInstances(("  " + field_name + ":").c_str(),
                              deprecated ? "" : (field_name + ": ").c_str());
      // Pick random type:
      auto base_type = static_cast<flatbuffers::BaseType>(
          lcg_rand() % (flatbuffers::BASE_TYPE_UNION + 1));
      switch (base_type) {
        case flatbuffers::BASE_TYPE_STRING:
          if (is_struct) {
            Dummy();  // No strings in structs.
          } else {
            AddToSchemaAndInstances("string", deprecated ? "" : "\"hi\"");
          }
          break;
        case flatbuffers::BASE_TYPE_VECTOR:
          if (is_struct) {
            Dummy();  // No vectors in structs.
          } else {
            AddToSchemaAndInstances("[ubyte]",
                                    deprecated ? "" : "[\n0,\n1,\n255\n]");
          }
          break;
        case flatbuffers::BASE_TYPE_NONE:
        case flatbuffers::BASE_TYPE_UTYPE:
        case flatbuffers::BASE_TYPE_STRUCT:
        case flatbuffers::BASE_TYPE_UNION:
          if (definition) {
            // Pick a random previous definition and random data instance of
            // that definition.
            int defref = lcg_rand() % definition;
            int instance = lcg_rand() % instances_per_definition;
            AddToSchemaAndInstances(
                ("D" + flatbuffers::NumToString(defref)).c_str(),
                deprecated ? ""
                           : definitions[defref].instances[instance].c_str());
          } else {
            // If this is the first definition, we have no definition we can
            // refer to.
            Dummy();
          }
          break;
        case flatbuffers::BASE_TYPE_BOOL:
          AddToSchemaAndInstances(
              "bool", deprecated ? "" : (lcg_rand() % 2 ? "true" : "false"));
          break;
        default:
          // All the scalar types.
          schema += flatbuffers::kTypeNames[base_type];

          if (!deprecated) {
            // We want each instance to use its own random value.
            for (int inst = 0; inst < instances_per_definition; inst++)
              definitions[definition].instances[inst] +=
                  flatbuffers::IsFloat(base_type)
                      ? flatbuffers::NumToString<double>(lcg_rand() % 128)
                            .c_str()
                      : flatbuffers::NumToString<int>(lcg_rand() % 128).c_str();
          }
      }
      AddToSchemaAndInstances(deprecated ? "(deprecated);\n" : ";\n",
                              deprecated ? "" : is_last_field ? "\n" : ",\n");
    }
    AddToSchemaAndInstances("}\n\n", "}");
  }

  schema += "root_type D" + flatbuffers::NumToString(num_definitions - 1);
  schema += ";\n";

  flatbuffers::Parser parser;

  // Will not compare against the original if we don't write defaults
  parser.builder_.ForceDefaults(true);

  // Parse the schema, parse the generated data, then generate text back
  // from the binary and compare against the original.
  TEST_EQ(parser.Parse(schema.c_str()), true);

  const std::string &json =
      definitions[num_definitions - 1].instances[0] + "\n";

  TEST_EQ(parser.Parse(json.c_str()), true);

  std::string jsongen;
  parser.opts.indent_step = 0;
  auto result =
      GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen);
  TEST_EQ(result, true);

  if (jsongen != json) {
    // These strings are larger than a megabyte, so we show the bytes around
    // the first bytes that are different rather than the whole string.
    size_t len = std::min(json.length(), jsongen.length());
    for (size_t i = 0; i < len; i++) {
      if (json[i] != jsongen[i]) {
        i -= std::min(static_cast<size_t>(10), i);  // show some context;
        size_t end = std::min(len, i + 20);
        for (; i < end; i++)
          TEST_OUTPUT_LINE("at %d: found \"%c\", expected \"%c\"\n",
                           static_cast<int>(i), jsongen[i], json[i]);
        break;
      }
    }
    TEST_NOTNULL(NULL);
  }

  // clang-format off
  #ifdef FLATBUFFERS_TEST_VERBOSE
    TEST_OUTPUT_LINE("%dk schema tested with %dk of json\n",
                     static_cast<int>(schema.length() / 1024),
                     static_cast<int>(json.length() / 1024));
  #endif
  // clang-format on
}

// Test that parser errors are actually generated.
void TestError(const char *src, const char *error_substr,
               bool strict_json = false) {
  flatbuffers::IDLOptions opts;
  opts.strict_json = strict_json;
  flatbuffers::Parser parser(opts);
  TEST_EQ(parser.Parse(src), false);  // Must signal error
  // Must be the error we're expecting
  TEST_NOTNULL(strstr(parser.error_.c_str(), error_substr));
}

// Test that parsing errors occur as we'd expect.
// Also useful for coverage, making sure these paths are run.
void ErrorTest() {
  // In order they appear in idl_parser.cpp
  TestError("table X { Y:byte; } root_type X; { Y: 999 }", "does not fit");
  TestError(".0", "floating point");
  TestError("\"\0", "illegal");
  TestError("\"\\q", "escape code");
  TestError("table ///", "documentation");
  TestError("@", "illegal");
  TestError("table 1", "expecting");
  TestError("table X { Y:[[int]]; }", "nested vector");
  TestError("table X { Y:1; }", "illegal type");
  TestError("table X { Y:int; Y:int; }", "field already");
  TestError("table Y {} table X { Y:int; }", "same as table");
  TestError("struct X { Y:string; }", "only scalar");
  TestError("table X { Y:string = \"\"; }", "default values");
  TestError("enum Y:byte { Z = 1 } table X { y:Y; }", "not part of enum");
  TestError("struct X { Y:int (deprecated); }", "deprecate");
  TestError("union Z { X } table X { Y:Z; } root_type X; { Y: {}, A:1 }",
            "missing type field");
  TestError("union Z { X } table X { Y:Z; } root_type X; { Y_type: 99, Y: {",
            "type id");
  TestError("table X { Y:int; } root_type X; { Z:", "unknown field");
  TestError("table X { Y:int; } root_type X; { Y:", "string constant", true);
  TestError("table X { Y:int; } root_type X; { \"Y\":1, }", "string constant",
            true);
  TestError(
      "struct X { Y:int; Z:int; } table W { V:X; } root_type W; "
      "{ V:{ Y:1 } }",
      "wrong number");
  TestError("enum E:byte { A } table X { Y:E; } root_type X; { Y:U }",
            "unknown enum value");
  TestError("table X { Y:byte; } root_type X; { Y:; }", "starting");
  TestError("enum X:byte { Y } enum X {", "enum already");
  TestError("enum X:float {}", "underlying");
  TestError("enum X:byte { Y, Y }", "value already");
  TestError("enum X:byte { Y=2, Z=1 }", "ascending");
  TestError("union X { Y = 256 }", "must fit");
  TestError("enum X:byte (bit_flags) { Y=8 }", "bit flag out");
  TestError("table X { Y:int; } table X {", "datatype already");
  TestError("struct X (force_align: 7) { Y:int; }", "force_align");
  TestError("{}", "no root");
  TestError("table X { Y:byte; } root_type X; { Y:1 } { Y:1 }", "one json");
  TestError("root_type X;", "unknown root");
  TestError("struct X { Y:int; } root_type X;", "a table");
  TestError("union X { Y }", "referenced");
  TestError("union Z { X } struct X { Y:int; }", "only tables");
  TestError("table X { Y:[int]; YLength:int; }", "clash");
  TestError("table X { Y:byte; } root_type X; { Y:1, Y:2 }", "more than once");
}

template<typename T> T TestValue(const char *json, const char *type_name) {
  flatbuffers::Parser parser;

  // Simple schema.
  TEST_EQ(parser.Parse(std::string("table X { Y:" + std::string(type_name) +
                                   "; } root_type X;")
                           .c_str()),
          true);

  TEST_EQ(parser.Parse(json), true);
  auto root = flatbuffers::GetRoot<flatbuffers::Table>(
      parser.builder_.GetBufferPointer());
  return root->GetField<T>(flatbuffers::FieldIndexToOffset(0), 0);
}

bool FloatCompare(float a, float b) { return fabs(a - b) < 0.001; }

// Additional parser testing not covered elsewhere.
void ValueTest() {
  // Test scientific notation numbers.
  TEST_EQ(FloatCompare(TestValue<float>("{ Y:0.0314159e+2 }", "float"),
                       (float)3.14159),
          true);

  // Test conversion functions.
  TEST_EQ(FloatCompare(TestValue<float>("{ Y:cos(rad(180)) }", "float"), -1),
          true);

  // Test negative hex constant.
  TEST_EQ(TestValue<int>("{ Y:-0x80 }", "int"), -128);

  // Make sure we do unsigned 64bit correctly.
  TEST_EQ(TestValue<uint64_t>("{ Y:12335089644688340133 }", "ulong"),
          12335089644688340133ULL);
}

void NestedListTest() {
  flatbuffers::Parser parser1;
  TEST_EQ(parser1.Parse("struct Test { a:short; b:byte; } table T { F:[Test]; }"
                        "root_type T;"
                        "{ F:[ [10,20], [30,40]] }"),
          true);
}

void EnumStringsTest() {
  flatbuffers::Parser parser1;
  TEST_EQ(parser1.Parse("enum E:byte { A, B, C } table T { F:[E]; }"
                        "root_type T;"
                        "{ F:[ A, B, \"C\", \"A B C\" ] }"),
          true);
  flatbuffers::Parser parser2;
  TEST_EQ(parser2.Parse("enum E:byte { A, B, C } table T { F:[int]; }"
                        "root_type T;"
                        "{ F:[ \"E.C\", \"E.A E.B E.C\" ] }"),
          true);
}

void IntegerOutOfRangeTest() {
  TestError("table T { F:byte; } root_type T; { F:128 }",
            "constant does not fit");
  TestError("table T { F:byte; } root_type T; { F:-129 }",
            "constant does not fit");
  TestError("table T { F:ubyte; } root_type T; { F:256 }",
            "constant does not fit");
  TestError("table T { F:ubyte; } root_type T; { F:-1 }",
            "constant does not fit");
  TestError("table T { F:short; } root_type T; { F:32768 }",
            "constant does not fit");
  TestError("table T { F:short; } root_type T; { F:-32769 }",
            "constant does not fit");
  TestError("table T { F:ushort; } root_type T; { F:65536 }",
            "constant does not fit");
  TestError("table T { F:ushort; } root_type T; { F:-1 }",
            "constant does not fit");
  TestError("table T { F:int; } root_type T; { F:2147483648 }",
            "constant does not fit");
  TestError("table T { F:int; } root_type T; { F:-2147483649 }",
            "constant does not fit");
  TestError("table T { F:uint; } root_type T; { F:4294967296 }",
            "constant does not fit");
  TestError("table T { F:uint; } root_type T; { F:-1 }",
            "constant does not fit");
}

void IntegerBoundaryTest() {
  TEST_EQ(TestValue<int8_t>("{ Y:127 }", "byte"), 127);
  TEST_EQ(TestValue<int8_t>("{ Y:-128 }", "byte"), -128);
  TEST_EQ(TestValue<uint8_t>("{ Y:255 }", "ubyte"), 255);
  TEST_EQ(TestValue<uint8_t>("{ Y:0 }", "ubyte"), 0);
  TEST_EQ(TestValue<int16_t>("{ Y:32767 }", "short"), 32767);
  TEST_EQ(TestValue<int16_t>("{ Y:-32768 }", "short"), -32768);
  TEST_EQ(TestValue<uint16_t>("{ Y:65535 }", "ushort"), 65535);
  TEST_EQ(TestValue<uint16_t>("{ Y:0 }", "ushort"), 0);
  TEST_EQ(TestValue<int32_t>("{ Y:2147483647 }", "int"), 2147483647);
  TEST_EQ(TestValue<int32_t>("{ Y:-2147483648 }", "int"), (-2147483647 - 1));
  TEST_EQ(TestValue<uint32_t>("{ Y:4294967295 }", "uint"), 4294967295);
  TEST_EQ(TestValue<uint32_t>("{ Y:0 }", "uint"), 0);
  TEST_EQ(TestValue<int64_t>("{ Y:9223372036854775807 }", "long"),
          9223372036854775807);
  TEST_EQ(TestValue<int64_t>("{ Y:-9223372036854775808 }", "long"),
          (-9223372036854775807 - 1));
  TEST_EQ(TestValue<uint64_t>("{ Y:18446744073709551615 }", "ulong"),
          18446744073709551615U);
  TEST_EQ(TestValue<uint64_t>("{ Y:0 }", "ulong"), 0);
}

void UnicodeTest() {
  flatbuffers::Parser parser;
  // Without setting allow_non_utf8 = true, we treat \x sequences as byte
  // sequences which are then validated as UTF-8.
  TEST_EQ(parser.Parse("table T { F:string; }"
                       "root_type T;"
                       "{ F:\"\\u20AC\\u00A2\\u30E6\\u30FC\\u30B6\\u30FC"
                       "\\u5225\\u30B5\\u30A4\\u30C8\\xE2\\x82\\xAC\\u0080\\uD8"
                       "3D\\uDE0E\" }"),
          true);
  std::string jsongen;
  parser.opts.indent_step = -1;
  auto result =
      GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen);
  TEST_EQ(result, true);
  TEST_EQ_STR(jsongen.c_str(),
              "{F: \"\\u20AC\\u00A2\\u30E6\\u30FC\\u30B6\\u30FC"
              "\\u5225\\u30B5\\u30A4\\u30C8\\u20AC\\u0080\\uD83D\\uDE0E\"}");
}

void UnicodeTestAllowNonUTF8() {
  flatbuffers::Parser parser;
  parser.opts.allow_non_utf8 = true;
  TEST_EQ(
      parser.Parse(
          "table T { F:string; }"
          "root_type T;"
          "{ F:\"\\u20AC\\u00A2\\u30E6\\u30FC\\u30B6\\u30FC"
          "\\u5225\\u30B5\\u30A4\\u30C8\\x01\\x80\\u0080\\uD83D\\uDE0E\" }"),
      true);
  std::string jsongen;
  parser.opts.indent_step = -1;
  auto result =
      GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen);
  TEST_EQ(result, true);
  TEST_EQ_STR(
      jsongen.c_str(),
      "{F: \"\\u20AC\\u00A2\\u30E6\\u30FC\\u30B6\\u30FC"
      "\\u5225\\u30B5\\u30A4\\u30C8\\u0001\\x80\\u0080\\uD83D\\uDE0E\"}");
}

void UnicodeTestGenerateTextFailsOnNonUTF8() {
  flatbuffers::Parser parser;
  // Allow non-UTF-8 initially to model what happens when we load a binary
  // flatbuffer from disk which contains non-UTF-8 strings.
  parser.opts.allow_non_utf8 = true;
  TEST_EQ(
      parser.Parse(
          "table T { F:string; }"
          "root_type T;"
          "{ F:\"\\u20AC\\u00A2\\u30E6\\u30FC\\u30B6\\u30FC"
          "\\u5225\\u30B5\\u30A4\\u30C8\\x01\\x80\\u0080\\uD83D\\uDE0E\" }"),
      true);
  std::string jsongen;
  parser.opts.indent_step = -1;
  // Now, disallow non-UTF-8 (the default behavior) so GenerateText indicates
  // failure.
  parser.opts.allow_non_utf8 = false;
  auto result =
      GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen);
  TEST_EQ(result, false);
}

void UnicodeSurrogatesTest() {
  flatbuffers::Parser parser;

  TEST_EQ(parser.Parse("table T { F:string (id: 0); }"
                       "root_type T;"
                       "{ F:\"\\uD83D\\uDCA9\"}"),
          true);
  auto root = flatbuffers::GetRoot<flatbuffers::Table>(
      parser.builder_.GetBufferPointer());
  auto string = root->GetPointer<flatbuffers::String *>(
      flatbuffers::FieldIndexToOffset(0));
  TEST_EQ_STR(string->c_str(), "\xF0\x9F\x92\xA9");
}

void UnicodeInvalidSurrogatesTest() {
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\\uD800\"}",
      "unpaired high surrogate");
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\\uD800abcd\"}",
      "unpaired high surrogate");
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\\uD800\\n\"}",
      "unpaired high surrogate");
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\\uD800\\uD800\"}",
      "multiple high surrogates");
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\\uDC00\"}",
      "unpaired low surrogate");
}

void InvalidUTF8Test() {
  // "1 byte" pattern, under min length of 2 bytes
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\x80\"}",
      "illegal UTF-8 sequence");
  // 2 byte pattern, string too short
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xDF\"}",
      "illegal UTF-8 sequence");
  // 3 byte pattern, string too short
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xEF\xBF\"}",
      "illegal UTF-8 sequence");
  // 4 byte pattern, string too short
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xF7\xBF\xBF\"}",
      "illegal UTF-8 sequence");
  // "5 byte" pattern, string too short
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xFB\xBF\xBF\xBF\"}",
      "illegal UTF-8 sequence");
  // "6 byte" pattern, string too short
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xFD\xBF\xBF\xBF\xBF\"}",
      "illegal UTF-8 sequence");
  // "7 byte" pattern, string too short
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xFE\xBF\xBF\xBF\xBF\xBF\"}",
      "illegal UTF-8 sequence");
  // "5 byte" pattern, over max length of 4 bytes
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xFB\xBF\xBF\xBF\xBF\"}",
      "illegal UTF-8 sequence");
  // "6 byte" pattern, over max length of 4 bytes
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xFD\xBF\xBF\xBF\xBF\xBF\"}",
      "illegal UTF-8 sequence");
  // "7 byte" pattern, over max length of 4 bytes
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xFE\xBF\xBF\xBF\xBF\xBF\xBF\"}",
      "illegal UTF-8 sequence");

  // Three invalid encodings for U+000A (\n, aka NEWLINE)
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xC0\x8A\"}",
      "illegal UTF-8 sequence");
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xE0\x80\x8A\"}",
      "illegal UTF-8 sequence");
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xF0\x80\x80\x8A\"}",
      "illegal UTF-8 sequence");

  // Two invalid encodings for U+00A9 (COPYRIGHT SYMBOL)
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xE0\x81\xA9\"}",
      "illegal UTF-8 sequence");
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xF0\x80\x81\xA9\"}",
      "illegal UTF-8 sequence");

  // Invalid encoding for U+20AC (EURO SYMBOL)
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xF0\x82\x82\xAC\"}",
      "illegal UTF-8 sequence");

  // UTF-16 surrogate values between U+D800 and U+DFFF cannot be encoded in
  // UTF-8
  TestError(
      "table T { F:string; }"
      "root_type T;"
      // U+10400 "encoded" as U+D801 U+DC00
      "{ F:\"\xED\xA0\x81\xED\xB0\x80\"}",
      "illegal UTF-8 sequence");
}

void UnknownFieldsTest() {
  flatbuffers::IDLOptions opts;
  opts.skip_unexpected_fields_in_json = true;
  flatbuffers::Parser parser(opts);

  TEST_EQ(parser.Parse("table T { str:string; i:int;}"
                       "root_type T;"
                       "{ str:\"test\","
                       "unknown_string:\"test\","
                       "\"unknown_string\":\"test\","
                       "unknown_int:10,"
                       "unknown_float:1.0,"
                       "unknown_array: [ 1, 2, 3, 4],"
                       "unknown_object: { i: 10 },"
                       "\"unknown_object\": { \"i\": 10 },"
                       "i:10}"),
          true);

  std::string jsongen;
  parser.opts.indent_step = -1;
  auto result =
      GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen);
  TEST_EQ(result, true);
  TEST_EQ_STR(jsongen.c_str(), "{str: \"test\",i: 10}");
}

void ParseUnionTest() {
  // Unions must be parseable with the type field following the object.
  flatbuffers::Parser parser;
  TEST_EQ(parser.Parse("table T { A:int; }"
                       "union U { T }"
                       "table V { X:U; }"
                       "root_type V;"
                       "{ X:{ A:1 }, X_type: T }"),
          true);
  // Unions must be parsable with prefixed namespace.
  flatbuffers::Parser parser2;
  TEST_EQ(parser2.Parse("namespace N; table A {} namespace; union U { N.A }"
                        "table B { e:U; } root_type B;"
                        "{ e_type: N_A, e: {} }"),
          true);
}

void UnionVectorTest() {
  // load FlatBuffer fbs schema.
  // TODO: load a JSON file with such a vector when JSON support is ready.
  std::string schemafile;
  TEST_EQ(flatbuffers::LoadFile(
              (test_data_path + "union_vector/union_vector.fbs").c_str(), false,
              &schemafile),
          true);

  // parse schema.
  flatbuffers::IDLOptions idl_opts;
  idl_opts.lang_to_generate |= flatbuffers::IDLOptions::kCpp;
  flatbuffers::Parser parser(idl_opts);
  TEST_EQ(parser.Parse(schemafile.c_str()), true);

  flatbuffers::FlatBufferBuilder fbb;

  // union types.
  std::vector<uint8_t> types;
  types.push_back(static_cast<uint8_t>(Character_Belle));
  types.push_back(static_cast<uint8_t>(Character_MuLan));
  types.push_back(static_cast<uint8_t>(Character_BookFan));
  types.push_back(static_cast<uint8_t>(Character_Other));
  types.push_back(static_cast<uint8_t>(Character_Unused));

  // union values.
  std::vector<flatbuffers::Offset<void>> characters;
  characters.push_back(fbb.CreateStruct(BookReader(/*books_read=*/7)).Union());
  characters.push_back(CreateAttacker(fbb, /*sword_attack_damage=*/5).Union());
  characters.push_back(fbb.CreateStruct(BookReader(/*books_read=*/2)).Union());
  characters.push_back(fbb.CreateString("Other").Union());
  characters.push_back(fbb.CreateString("Unused").Union());

  // create Movie.
  const auto movie_offset =
      CreateMovie(fbb, Character_Rapunzel,
                  fbb.CreateStruct(Rapunzel(/*hair_length=*/6)).Union(),
                  fbb.CreateVector(types), fbb.CreateVector(characters));
  FinishMovieBuffer(fbb, movie_offset);
  auto buf = fbb.GetBufferPointer();

  flatbuffers::Verifier verifier(buf, fbb.GetSize());
  TEST_EQ(VerifyMovieBuffer(verifier), true);

  auto flat_movie = GetMovie(buf);

  auto TestMovie = [](const Movie *movie) {
    TEST_EQ(movie->main_character_type() == Character_Rapunzel, true);

    auto cts = movie->characters_type();
    TEST_EQ(movie->characters_type()->size(), 5);
    TEST_EQ(cts->GetEnum<Character>(0) == Character_Belle, true);
    TEST_EQ(cts->GetEnum<Character>(1) == Character_MuLan, true);
    TEST_EQ(cts->GetEnum<Character>(2) == Character_BookFan, true);
    TEST_EQ(cts->GetEnum<Character>(3) == Character_Other, true);
    TEST_EQ(cts->GetEnum<Character>(4) == Character_Unused, true);

    auto rapunzel = movie->main_character_as_Rapunzel();
    TEST_EQ(rapunzel->hair_length(), 6);

    auto cs = movie->characters();
    TEST_EQ(cs->size(), 5);
    auto belle = cs->GetAs<BookReader>(0);
    TEST_EQ(belle->books_read(), 7);
    auto mu_lan = cs->GetAs<Attacker>(1);
    TEST_EQ(mu_lan->sword_attack_damage(), 5);
    auto book_fan = cs->GetAs<BookReader>(2);
    TEST_EQ(book_fan->books_read(), 2);
    auto other = cs->GetAsString(3);
    TEST_EQ_STR(other->c_str(), "Other");
    auto unused = cs->GetAsString(4);
    TEST_EQ_STR(unused->c_str(), "Unused");
  };

  TestMovie(flat_movie);

  auto movie_object = flat_movie->UnPack();
  TEST_EQ(movie_object->main_character.AsRapunzel()->hair_length(), 6);
  TEST_EQ(movie_object->characters[0].AsBelle()->books_read(), 7);
  TEST_EQ(movie_object->characters[1].AsMuLan()->sword_attack_damage, 5);
  TEST_EQ(movie_object->characters[2].AsBookFan()->books_read(), 2);
  TEST_EQ_STR(movie_object->characters[3].AsOther()->c_str(), "Other");
  TEST_EQ_STR(movie_object->characters[4].AsUnused()->c_str(), "Unused");

  fbb.Clear();
  fbb.Finish(Movie::Pack(fbb, movie_object));

  delete movie_object;

  auto repacked_movie = GetMovie(fbb.GetBufferPointer());

  TestMovie(repacked_movie);

  auto s =
      flatbuffers::FlatBufferToString(fbb.GetBufferPointer(), MovieTypeTable());
  TEST_EQ_STR(
      s.c_str(),
      "{ main_character_type: Rapunzel, main_character: { hair_length: 6 }, "
      "characters_type: [ Belle, MuLan, BookFan, Other, Unused ], "
      "characters: [ { books_read: 7 }, { sword_attack_damage: 5 }, "
      "{ books_read: 2 }, \"Other\", \"Unused\" ] }");
}

void ConformTest() {
  flatbuffers::Parser parser;
  TEST_EQ(parser.Parse("table T { A:int; } enum E:byte { A }"), true);

  auto test_conform = [](flatbuffers::Parser &parser1, const char *test,
                         const char *expected_err) {
    flatbuffers::Parser parser2;
    TEST_EQ(parser2.Parse(test), true);
    auto err = parser2.ConformTo(parser1);
    TEST_NOTNULL(strstr(err.c_str(), expected_err));
  };

  test_conform(parser, "table T { A:byte; }", "types differ for field");
  test_conform(parser, "table T { B:int; A:int; }", "offsets differ for field");
  test_conform(parser, "table T { A:int = 1; }", "defaults differ for field");
  test_conform(parser, "table T { B:float; }",
               "field renamed to different type");
  test_conform(parser, "enum E:byte { B, A }", "values differ for enum");
}

void ParseProtoBufAsciiTest() {
  // We can put the parser in a mode where it will accept JSON that looks more
  // like Protobuf ASCII, for users that have data in that format.
  // This uses no "" for field names (which we already support by default,
  // omits `,`, `:` before `{` and a couple of other features.
  flatbuffers::Parser parser;
  parser.opts.protobuf_ascii_alike = true;
  TEST_EQ(
      parser.Parse("table S { B:int; } table T { A:[int]; C:S; } root_type T;"),
      true);
  TEST_EQ(parser.Parse("{ A [1 2] C { B:2 }}"), true);
  // Similarly, in text output, it should omit these.
  std::string text;
  auto ok = flatbuffers::GenerateText(
      parser, parser.builder_.GetBufferPointer(), &text);
  TEST_EQ(ok, true);
  TEST_EQ_STR(text.c_str(),
              "{\n  A [\n    1\n    2\n  ]\n  C {\n    B: 2\n  }\n}\n");
}

void FlexBuffersTest() {
  flexbuffers::Builder slb(512,
                           flexbuffers::BUILDER_FLAG_SHARE_KEYS_AND_STRINGS);

  // Write the equivalent of:
  // { vec: [ -100, "Fred", 4.0, false ], bar: [ 1, 2, 3 ], bar3: [ 1, 2, 3 ],
  // foo: 100, bool: true, mymap: { foo: "Fred" } }
  // clang-format off
  #ifndef FLATBUFFERS_CPP98_STL
    // It's possible to do this without std::function support as well.
    slb.Map([&]() {
       slb.Vector("vec", [&]() {
        slb += -100;  // Equivalent to slb.Add(-100) or slb.Int(-100);
        slb += "Fred";
        slb.IndirectFloat(4.0f);
        uint8_t blob[] = { 77 };
        slb.Blob(blob, 1);
        slb += false;
      });
      int ints[] = { 1, 2, 3 };
      slb.Vector("bar", ints, 3);
      slb.FixedTypedVector("bar3", ints, 3);
      bool bools[] = {true, false, true, false};
      slb.Vector("bools", bools, 4);
      slb.Bool("bool", true);
      slb.Double("foo", 100);
      slb.Map("mymap", [&]() {
        slb.String("foo", "Fred");  // Testing key and string reuse.
      });
    });
    slb.Finish();
  #else
    // It's possible to do this without std::function support as well.
    slb.Map([](flexbuffers::Builder& slb2) {
       slb2.Vector("vec", [](flexbuffers::Builder& slb3) {
        slb3 += -100;  // Equivalent to slb.Add(-100) or slb.Int(-100);
        slb3 += "Fred";
        slb3.IndirectFloat(4.0f);
        uint8_t blob[] = { 77 };
        slb3.Blob(blob, 1);
        slb3 += false;
      }, slb2);
      int ints[] = { 1, 2, 3 };
      slb2.Vector("bar", ints, 3);
      slb2.FixedTypedVector("bar3", ints, 3);
      slb2.Bool("bool", true);
      slb2.Double("foo", 100);
      slb2.Map("mymap", [](flexbuffers::Builder& slb3) {
        slb3.String("foo", "Fred");  // Testing key and string reuse.
      }, slb2);
    }, slb);
    slb.Finish();
  #endif  // FLATBUFFERS_CPP98_STL

  #ifdef FLATBUFFERS_TEST_VERBOSE
    for (size_t i = 0; i < slb.GetBuffer().size(); i++)
      printf("%d ", flatbuffers::vector_data(slb.GetBuffer())[i]);
    printf("\n");
  #endif
  // clang-format on

  auto map = flexbuffers::GetRoot(slb.GetBuffer()).AsMap();
  TEST_EQ(map.size(), 7);
  auto vec = map["vec"].AsVector();
  TEST_EQ(vec.size(), 5);
  TEST_EQ(vec[0].AsInt64(), -100);
  TEST_EQ_STR(vec[1].AsString().c_str(), "Fred");
  TEST_EQ(vec[1].AsInt64(), 0);  // Number parsing failed.
  TEST_EQ(vec[2].AsDouble(), 4.0);
  TEST_EQ(vec[2].AsString().IsTheEmptyString(), true);  // Wrong Type.
  TEST_EQ_STR(vec[2].AsString().c_str(), "");     // This still works though.
  TEST_EQ_STR(vec[2].ToString().c_str(), "4.0");  // Or have it converted.

  // Few tests for templated version of As.
  TEST_EQ(vec[0].As<int64_t>(), -100);
  TEST_EQ_STR(vec[1].As<std::string>().c_str(), "Fred");
  TEST_EQ(vec[1].As<int64_t>(), 0);  // Number parsing failed.
  TEST_EQ(vec[2].As<double>(), 4.0);

  // Test that the blob can be accessed.
  TEST_EQ(vec[3].IsBlob(), true);
  auto blob = vec[3].AsBlob();
  TEST_EQ(blob.size(), 1);
  TEST_EQ(blob.data()[0], 77);
  TEST_EQ(vec[4].IsBool(), true);   // Check if type is a bool
  TEST_EQ(vec[4].AsBool(), false);  // Check if value is false
  auto tvec = map["bar"].AsTypedVector();
  TEST_EQ(tvec.size(), 3);
  TEST_EQ(tvec[2].AsInt8(), 3);
  auto tvec3 = map["bar3"].AsFixedTypedVector();
  TEST_EQ(tvec3.size(), 3);
  TEST_EQ(tvec3[2].AsInt8(), 3);
  TEST_EQ(map["bool"].AsBool(), true);
  auto tvecb = map["bools"].AsTypedVector();
  TEST_EQ(tvecb.ElementType(), flexbuffers::TYPE_BOOL);
  TEST_EQ(map["foo"].AsUInt8(), 100);
  TEST_EQ(map["unknown"].IsNull(), true);
  auto mymap = map["mymap"].AsMap();
  // These should be equal by pointer equality, since key and value are shared.
  TEST_EQ(mymap.Keys()[0].AsKey(), map.Keys()[4].AsKey());
  TEST_EQ(mymap.Values()[0].AsString().c_str(), vec[1].AsString().c_str());
  // We can mutate values in the buffer.
  TEST_EQ(vec[0].MutateInt(-99), true);
  TEST_EQ(vec[0].AsInt64(), -99);
  TEST_EQ(vec[1].MutateString("John"), true);  // Size must match.
  TEST_EQ_STR(vec[1].AsString().c_str(), "John");
  TEST_EQ(vec[1].MutateString("Alfred"), false);  // Too long.
  TEST_EQ(vec[2].MutateFloat(2.0f), true);
  TEST_EQ(vec[2].AsFloat(), 2.0f);
  TEST_EQ(vec[2].MutateFloat(3.14159), false);  // Double does not fit in float.
  TEST_EQ(vec[4].AsBool(), false);              // Is false before change
  TEST_EQ(vec[4].MutateBool(true), true);       // Can change a bool
  TEST_EQ(vec[4].AsBool(), true);               // Changed bool is now true

  // Parse from JSON:
  flatbuffers::Parser parser;
  slb.Clear();
  auto jsontest = "{ a: [ 123, 456.0 ], b: \"hello\", c: true, d: false }";
  TEST_EQ(parser.ParseFlexBuffer(jsontest, nullptr, &slb), true);
  auto jroot = flexbuffers::GetRoot(slb.GetBuffer());
  auto jmap = jroot.AsMap();
  auto jvec = jmap["a"].AsVector();
  TEST_EQ(jvec[0].AsInt64(), 123);
  TEST_EQ(jvec[1].AsDouble(), 456.0);
  TEST_EQ_STR(jmap["b"].AsString().c_str(), "hello");
  TEST_EQ(jmap["c"].IsBool(), true);   // Parsed correctly to a bool
  TEST_EQ(jmap["c"].AsBool(), true);   // Parsed correctly to true
  TEST_EQ(jmap["d"].IsBool(), true);   // Parsed correctly to a bool
  TEST_EQ(jmap["d"].AsBool(), false);  // Parsed correctly to false
  // And from FlexBuffer back to JSON:
  auto jsonback = jroot.ToString();
  TEST_EQ_STR(jsontest, jsonback.c_str());
}

void TypeAliasesTest() {
  flatbuffers::FlatBufferBuilder builder;

  builder.Finish(CreateTypeAliases(
      builder, flatbuffers::numeric_limits<int8_t>::min(),
      flatbuffers::numeric_limits<uint8_t>::max(),
      flatbuffers::numeric_limits<int16_t>::min(),
      flatbuffers::numeric_limits<uint16_t>::max(),
      flatbuffers::numeric_limits<int32_t>::min(),
      flatbuffers::numeric_limits<uint32_t>::max(),
      flatbuffers::numeric_limits<int64_t>::min(),
      flatbuffers::numeric_limits<uint64_t>::max(), 2.3f, 2.3));

  auto p = builder.GetBufferPointer();
  auto ta = flatbuffers::GetRoot<TypeAliases>(p);

  TEST_EQ(ta->i8(), flatbuffers::numeric_limits<int8_t>::min());
  TEST_EQ(ta->u8(), flatbuffers::numeric_limits<uint8_t>::max());
  TEST_EQ(ta->i16(), flatbuffers::numeric_limits<int16_t>::min());
  TEST_EQ(ta->u16(), flatbuffers::numeric_limits<uint16_t>::max());
  TEST_EQ(ta->i32(), flatbuffers::numeric_limits<int32_t>::min());
  TEST_EQ(ta->u32(), flatbuffers::numeric_limits<uint32_t>::max());
  TEST_EQ(ta->i64(), flatbuffers::numeric_limits<int64_t>::min());
  TEST_EQ(ta->u64(), flatbuffers::numeric_limits<uint64_t>::max());
  TEST_EQ(ta->f32(), 2.3f);
  TEST_EQ(ta->f64(), 2.3);
  TEST_EQ(sizeof(ta->i8()), 1);
  TEST_EQ(sizeof(ta->i16()), 2);
  TEST_EQ(sizeof(ta->i32()), 4);
  TEST_EQ(sizeof(ta->i64()), 8);
  TEST_EQ(sizeof(ta->u8()), 1);
  TEST_EQ(sizeof(ta->u16()), 2);
  TEST_EQ(sizeof(ta->u32()), 4);
  TEST_EQ(sizeof(ta->u64()), 8);
  TEST_EQ(sizeof(ta->f32()), 4);
  TEST_EQ(sizeof(ta->f64()), 8);
}

void EndianSwapTest() {
  TEST_EQ(flatbuffers::EndianSwap(static_cast<int16_t>(0x1234)), 0x3412);
  TEST_EQ(flatbuffers::EndianSwap(static_cast<int32_t>(0x12345678)),
          0x78563412);
  TEST_EQ(flatbuffers::EndianSwap(static_cast<int64_t>(0x1234567890ABCDEF)),
          0xEFCDAB9078563412);
  TEST_EQ(flatbuffers::EndianSwap(flatbuffers::EndianSwap(3.14f)), 3.14f);
}

int main(int /*argc*/, const char * /*argv*/ []) {
  // clang-format off
  #if defined(FLATBUFFERS_MEMORY_LEAK_TRACKING) && \
      defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF
      // For more thorough checking:
      //| _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_DELAY_FREE_MEM_DF
    );
  #endif

  // Run our various test suites:

  std::string rawbuf;
  auto flatbuf1 = CreateFlatBufferTest(rawbuf);
  #if !defined(FLATBUFFERS_CPP98_STL)
    auto flatbuf = std::move(flatbuf1);  // Test move assignment.
  #else
    auto &flatbuf = flatbuf1;
  #endif // !defined(FLATBUFFERS_CPP98_STL)

  TriviallyCopyableTest();

  AccessFlatBufferTest(reinterpret_cast<const uint8_t *>(rawbuf.c_str()),
                       rawbuf.length());
  AccessFlatBufferTest(flatbuf.data(), flatbuf.size());

  MutateFlatBuffersTest(flatbuf.data(), flatbuf.size());

  ObjectFlatBuffersTest(flatbuf.data());

  MiniReflectFlatBuffersTest(flatbuf.data());

  SizePrefixedTest();

  #ifndef FLATBUFFERS_NO_FILE_TESTS
    #ifdef FLATBUFFERS_TEST_PATH_PREFIX
      test_data_path = FLATBUFFERS_STRING(FLATBUFFERS_TEST_PATH_PREFIX) +
                       test_data_path;
    #endif
    ParseAndGenerateTextTest();
    ReflectionTest(flatbuf.data(), flatbuf.size());
    ParseProtoTest();
    UnionVectorTest();
  #endif
  // clang-format on

  FuzzTest1();
  FuzzTest2();

  ErrorTest();
  ValueTest();
  EnumStringsTest();
  IntegerOutOfRangeTest();
  IntegerBoundaryTest();
  UnicodeTest();
  UnicodeTestAllowNonUTF8();
  UnicodeTestGenerateTextFailsOnNonUTF8();
  UnicodeSurrogatesTest();
  UnicodeInvalidSurrogatesTest();
  InvalidUTF8Test();
  UnknownFieldsTest();
  ParseUnionTest();
  ConformTest();
  ParseProtoBufAsciiTest();
  TypeAliasesTest();
  EndianSwapTest();

  JsonDefaultTest();

  FlexBuffersTest();

  if (!testing_fails) {
    TEST_OUTPUT_LINE("ALL TESTS PASSED");
    return 0;
  } else {
    TEST_OUTPUT_LINE("%d FAILED TESTS", testing_fails);
    return 1;
  }
}
