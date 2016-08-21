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

#define FLATBUFFERS_DEBUG_VERIFICATION_FAILURE 1
#define FLATBUFFERS_TRACK_VERIFIER_BUFFER_SIZE

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#include "monster_test_generated.h"
#include "namespace_test/namespace_test1_generated.h"
#include "namespace_test/namespace_test2_generated.h"
#include "arrays_test_generated.h"

#ifndef FLATBUFFERS_CPP98_STL
  #include <random>
#endif

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

int testing_fails = 0;

void TestFail(const char *expval, const char *val, const char *exp,
              const char *file, int line) {
  TEST_OUTPUT_LINE("TEST FAILED: %s:%d, %s (%s) != %s", file, line,
                   exp, expval, val);
  assert(0);
  testing_fails++;
}

void TestEqStr(const char *expval, const char *val, const char *exp,
               const char *file, int line) {
  if (strcmp(expval, val) != 0) {
    TestFail(expval, val, exp, file, line);
  }
}

template<typename T, typename U>
void TestEq(T expval, U val, const char *exp, const char *file, int line) {
  if (U(expval) != val) {
    TestFail(flatbuffers::NumToString(expval).c_str(),
             flatbuffers::NumToString(val).c_str(),
             exp, file, line);
  }
}

#define TEST_EQ(exp, val) TestEq(exp,         val,   #exp, __FILE__, __LINE__)
#define TEST_NOTNULL(exp) TestEq(exp == NULL, false, #exp, __FILE__, __LINE__)
#define TEST_EQ_STR(exp, val) TestEqStr(exp,  val,   #exp, __FILE__, __LINE__)

// Include simple random number generator to ensure results will be the
// same cross platform.
// http://en.wikipedia.org/wiki/Park%E2%80%93Miller_random_number_generator
uint32_t lcg_seed = 48271;
uint32_t lcg_rand() {
    return lcg_seed = ((uint64_t)lcg_seed * 279470273UL) % 4294967291UL;
}
void lcg_reset() { lcg_seed = 48271; }

// example of how to build up a serialized buffer algorithmically:
flatbuffers::unique_ptr_t CreateFlatBufferTest(std::string &buffer) {
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
  const char *names[] = { "bob", "fred", "bob", "fred" };
  auto vecofstrings =
      builder.CreateVector<flatbuffers::Offset<flatbuffers::String>>(4,
        [&](size_t i) {
    return builder.CreateSharedString(names[i]);
  });

  // Creating vectors of strings in one convenient call.
  std::vector<std::string> names2;
  names2.push_back("jane");
  names2.push_back("mary");
  auto vecofstrings2 = builder.CreateVectorOfStrings(names2);

  // Create an array of sorted tables, can be used with binary search when read:
  auto vecoftables = builder.CreateVectorOfSortedTables(mlocs, 3);

  // shortcut for creating monster with all fields set:
  auto mloc = CreateMonster(builder, &vec, 150, 80, name, inventory, Color_Blue,
                            Any_Monster, mlocs[1].Union(), // Store a union.
                            testv, vecofstrings, vecoftables, 0, 0, 0, false,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 3.14159f, 3.0f, 0.0f,
                            vecofstrings2);

  FinishMonsterBuffer(builder, mloc);

  #ifdef FLATBUFFERS_TEST_VERBOSE
  // print byte data for debugging:
  auto p = builder.GetBufferPointer();
  for (flatbuffers::uoffset_t i = 0; i < builder.GetSize(); i++)
    printf("%d ", p[i]);
  #endif

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
  std::memcpy(&test_buff[0], flatbuf , length);
  std::memcpy(&test_buff[length], flatbuf , length);

  flatbuffers::Verifier verifierl(&test_buff[0], length - 1);
  TEST_EQ(VerifyMonsterBuffer(verifierl), false);
  TEST_EQ(verifierl.GetComputedSize(), 0);

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

  // Since Flatbuffers uses explicit mechanisms to override the default
  // compiler alignment, double check that the compiler indeed obeys them:
  // (Test consists of a short and byte):
  TEST_EQ(flatbuffers::AlignOf<Test>(), 2UL);
  TEST_EQ(sizeof(Test), 4UL);

  auto tests = monster->test4();
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

  // Checking for presence of fields:
  TEST_EQ(flatbuffers::IsFieldPresent(monster, Monster::VT_HP), true);
  TEST_EQ(flatbuffers::IsFieldPresent(monster, Monster::VT_MANA), false);
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
  monster->mutate_hp(80);

  auto mana_ok = monster->mutate_mana(10);
  TEST_EQ(mana_ok, false);  // Field was NOT present, because default value.

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
  // Turn a buffer into C++ objects.
  auto monster1 = GetMonster(flatbuf)->UnPack();

  // Re-serialize the data.
  flatbuffers::FlatBufferBuilder fbb1;
  fbb1.Finish(CreateMonster(fbb1, monster1.get()), MonsterIdentifier());

  // Unpack again, and re-serialize again.
  auto monster2 = GetMonster(fbb1.GetBufferPointer())->UnPack();
  flatbuffers::FlatBufferBuilder fbb2;
  fbb2.Finish(CreateMonster(fbb2, monster2.get()), MonsterIdentifier());

  // Now we've gone full round-trip, the two buffers should match.
  auto len1 = fbb1.GetSize();
  auto len2 = fbb2.GetSize();
  TEST_EQ(len1, len2);
  TEST_EQ(memcmp(fbb1.GetBufferPointer(), fbb2.GetBufferPointer(),
                 len1), 0);

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

// example of parsing text straight into a buffer, and generating
// text back from it:
void ParseAndGenerateTextTest() {
  // load FlatBuffer schema (.fbs) and JSON from disk
  std::string schemafile;
  std::string jsonfile;
  TEST_EQ(flatbuffers::LoadFile(
    "tests/monster_test.fbs", false, &schemafile), true);
  TEST_EQ(flatbuffers::LoadFile(
    "tests/monsterdata_test.golden", false, &jsonfile), true);

  // parse schema first, so we can use it to parse the data after
  flatbuffers::Parser parser;
  const char *include_directories[] = { "tests", nullptr };
  TEST_EQ(parser.Parse(schemafile.c_str(), include_directories), true);
  TEST_EQ(parser.Parse(jsonfile.c_str(), include_directories), true);

  // here, parser.builder_ contains a binary buffer that is the parsed data.

  // First, verify it, just in case:
  flatbuffers::Verifier verifier(parser.builder_.GetBufferPointer(),
                                 parser.builder_.GetSize());
  TEST_EQ(VerifyMonsterBuffer(verifier), true);

  // to ensure it is correct, we now generate text back from the binary,
  // and compare the two:
  std::string jsongen;
  GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen);

  if (jsongen != jsonfile) {
    printf("%s----------------\n%s", jsongen.c_str(), jsonfile.c_str());
    TEST_NOTNULL(NULL);
  }
}

void ReflectionTest(uint8_t *flatbuf, size_t length) {
  // Load a binary schema.
  std::string bfbsfile;
  TEST_EQ(flatbuffers::LoadFile(
    "tests/monster_test.bfbs", true, &bfbsfile), true);

  // Verify it, just in case:
  flatbuffers::Verifier verifier(
    reinterpret_cast<const uint8_t *>(bfbsfile.c_str()), bfbsfile.length());
  TEST_EQ(reflection::VerifySchemaBuffer(verifier), true);

  // Make sure the schema is what we expect it to be.
  auto &schema = *reflection::GetSchema(bfbsfile.c_str());
  auto root_table = schema.root_table();
  TEST_EQ_STR(root_table->name()->c_str(), "Monster");
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

  // Now use it to dynamically access a buffer.
  auto &root = *flatbuffers::GetAnyRoot(flatbuf);
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
  auto rroot = flatbuffers::piv(flatbuffers::GetAnyRoot(resizingbuf.data()),
                                resizingbuf);
  SetString(schema, "totally new string", GetFieldS(**rroot, name_field),
            &resizingbuf);
  // Here resizingbuf has changed, but rroot is still valid.
  TEST_EQ_STR(GetFieldS(**rroot, name_field)->c_str(), "totally new string");
  // Now lets extend a vector by 100 elements (10 -> 110).
  auto &inventory_field = *fields->LookupByKey("inventory");
  auto rinventory = flatbuffers::piv(
                     flatbuffers::GetFieldV<uint8_t>(**rroot, inventory_field),
                     resizingbuf);
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
  auto string_ptr = flatbuffers::AddFlatBuffer(resizingbuf,
                                               stringfbb.GetBufferPointer(),
                                               stringfbb.GetSize());
  // Finally, set the new value in the vector.
  rtestarrayofstring->MutateOffset(2, string_ptr);
  TEST_EQ_STR(rtestarrayofstring->Get(0)->c_str(), "bob");
  TEST_EQ_STR(rtestarrayofstring->Get(2)->c_str(), "hank");
  // Test integrity of all resize operations above.
  flatbuffers::Verifier resize_verifier(
        reinterpret_cast<const uint8_t *>(resizingbuf.data()),
        resizingbuf.size());
  TEST_EQ(VerifyMonsterBuffer(resize_verifier), true);
  // As an additional test, also set it on the name field.
  // Note: unlike the name change above, this just overwrites the offset,
  // rather than changing the string in-place.
  SetFieldT(*rroot, name_field, string_ptr);
  TEST_EQ_STR(GetFieldS(**rroot, name_field)->c_str(), "hank");

  // Using reflection, rather than mutating binary FlatBuffers, we can also copy
  // tables and other things out of other FlatBuffers into a FlatBufferBuilder,
  // either part or whole.
  flatbuffers::FlatBufferBuilder fbb;
  auto root_offset = flatbuffers::CopyTable(fbb, schema, *root_table,
                                            *flatbuffers::GetAnyRoot(flatbuf),
                                            true);
  fbb.Finish(root_offset, MonsterIdentifier());
  // Test that it was copied correctly:
  AccessFlatBufferTest(fbb.GetBufferPointer(), fbb.GetSize());
}

// Parse a .proto schema, output as .fbs
void ParseProtoTest() {
  // load the .proto and the golden file from disk
  std::string protofile;
  std::string goldenfile;
  TEST_EQ(flatbuffers::LoadFile(
    "tests/prototest/test.proto", false, &protofile), true);
  TEST_EQ(flatbuffers::LoadFile(
    "tests/prototest/test.golden", false, &goldenfile), true);

  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = false;
  opts.proto_mode = true;

  // Parse proto.
  flatbuffers::Parser parser(opts);
  const char *include_directories[] = { "tests/prototest", nullptr };
  TEST_EQ(parser.Parse(protofile.c_str(), include_directories), true);

  // Generate fbs.
  auto fbs = flatbuffers::GenerateFBS(parser, "test");

  // Ensure generated file is parsable.
  flatbuffers::Parser parser2;
  TEST_EQ(parser2.Parse(fbs.c_str(), nullptr), true);

  if (fbs != goldenfile) {
    printf("%s----------------\n%s", fbs.c_str(), goldenfile.c_str());
    TEST_NOTNULL(NULL);
  }
}

template<typename T> void CompareTableFieldValue(flatbuffers::Table *table,
                                                 flatbuffers::voffset_t voffset,
                                                 T val) {
  T read = table->GetField(voffset, static_cast<T>(0));
  TEST_EQ(read, val);
}

// Low level stress/fuzz test: serialize/deserialize a variety of
// different kinds of data in different combinations
void FuzzTest1() {

  // Values we're testing against: chosen to ensure no bits get chopped
  // off anywhere, and also be different from eachother.
  const uint8_t  bool_val   = true;
  const int8_t   char_val   = -127;  // 0x81
  const uint8_t  uchar_val  = 0xFF;
  const int16_t  short_val  = -32222; // 0x8222;
  const uint16_t ushort_val = 0xFEEE;
  const int32_t  int_val    = 0x83333333;
  const uint32_t uint_val   = 0xFDDDDDDD;
  const int64_t  long_val   = 0x8444444444444444LL;
  const uint64_t ulong_val  = 0xFCCCCCCCCCCCCCCCULL;
  const float    float_val  = 3.14159f;
  const double   double_val = 3.14159265359;

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
        case 0:  builder.AddElement<uint8_t >(off, bool_val,   0); break;
        case 1:  builder.AddElement<int8_t  >(off, char_val,   0); break;
        case 2:  builder.AddElement<uint8_t >(off, uchar_val,  0); break;
        case 3:  builder.AddElement<int16_t >(off, short_val,  0); break;
        case 4:  builder.AddElement<uint16_t>(off, ushort_val, 0); break;
        case 5:  builder.AddElement<int32_t >(off, int_val,    0); break;
        case 6:  builder.AddElement<uint32_t>(off, uint_val,   0); break;
        case 7:  builder.AddElement<int64_t >(off, long_val,   0); break;
        case 8:  builder.AddElement<uint64_t>(off, ulong_val,  0); break;
        case 9:  builder.AddElement<float   >(off, float_val,  0); break;
        case 10: builder.AddElement<double  >(off, double_val, 0); break;
      }
    }
    objects[i] = builder.EndTable(start, fields_per_object);
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
        case 0:  CompareTableFieldValue(table, off, bool_val  ); break;
        case 1:  CompareTableFieldValue(table, off, char_val  ); break;
        case 2:  CompareTableFieldValue(table, off, uchar_val ); break;
        case 3:  CompareTableFieldValue(table, off, short_val ); break;
        case 4:  CompareTableFieldValue(table, off, ushort_val); break;
        case 5:  CompareTableFieldValue(table, off, int_val   ); break;
        case 6:  CompareTableFieldValue(table, off, uint_val  ); break;
        case 7:  CompareTableFieldValue(table, off, long_val  ); break;
        case 8:  CompareTableFieldValue(table, off, ulong_val ); break;
        case 9:  CompareTableFieldValue(table, off, float_val ); break;
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
  const int deprecation_rate = 10;        // 1 in deprecation_rate fields will
                                          // be deprecated.

  std::string schema = "namespace test;\n\n";

  struct RndDef {
    std::string instances[instances_per_definition];

    // Since we're generating schema and corresponding data in tandem,
    // this convenience function adds strings to both at once.
    static void Add(RndDef (&definitions_l)[num_definitions],
                    std::string &schema_l,
                    const int instances_per_definition_l,
                    const char *schema_add, const char *instance_add,
                    int definition) {
      schema_l += schema_add;
      for (int i = 0; i < instances_per_definition_l; i++)
        definitions_l[definition].instances[i] += instance_add;
    }
  };

  #define AddToSchemaAndInstances(schema_add, instance_add) \
    RndDef::Add(definitions, schema, instances_per_definition, \
                schema_add, instance_add, definition)

  #define Dummy() \
    RndDef::Add(definitions, schema, instances_per_definition, \
                "byte", "1", definition)

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
      const bool deprecated = !is_struct &&
                              !is_last_field &&
                              (lcg_rand() % deprecation_rate == 0);

      std::string field_name = "f" + flatbuffers::NumToString(field);
      AddToSchemaAndInstances(("  " + field_name + ":").c_str(),
                              deprecated ? "" : (field_name + ": ").c_str());
      // Pick random type:
      int base_type = lcg_rand() % (flatbuffers::BASE_TYPE_UNION + 1);
      switch (base_type) {
        case flatbuffers::BASE_TYPE_ARRAY:
          if (!is_struct) {
            AddToSchemaAndInstances("ubyte",
              deprecated ? "" : "255");  // No fixed-length arrays in tables.
          } else {
            AddToSchemaAndInstances("[int:3]",
              deprecated ? "" : "[\n,\n,\n]");
          }
          break;
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
          }
          else {
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
              deprecated
                ? ""
                : definitions[defref].instances[instance].c_str());
          } else {
            // If this is the first definition, we have no definition we can
            // refer to.
            Dummy();
          }
          break;
        case flatbuffers::BASE_TYPE_BOOL:
          AddToSchemaAndInstances("bool", deprecated
                                  ? ""
                                  : (lcg_rand() % 2 ? "true" : "false"));
          break;
        default:
          // All the scalar types.
          schema += flatbuffers::kTypeNames[base_type];

          if (!deprecated) {
            // We want each instance to use its own random value.
            for (int inst = 0; inst < instances_per_definition; inst++)
              definitions[definition].instances[inst] +=
              flatbuffers::NumToString(lcg_rand() % 128).c_str();
          }
      }
      AddToSchemaAndInstances(
        deprecated ? "(deprecated);\n" : ";\n",
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
  GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen);

  if (jsongen != json) {
    // These strings are larger than a megabyte, so we show the bytes around
    // the first bytes that are different rather than the whole string.
    size_t len = std::min(json.length(), jsongen.length());
    for (size_t i = 0; i < len; i++) {
      if (json[i] != jsongen[i]) {
        i -= std::min(static_cast<size_t>(10), i); // show some context;
        size_t end = std::min(len, i + 20);
        for (; i < end; i++)
          printf("at %d: found \"%c\", expected \"%c\"\n",
               static_cast<int>(i), jsongen[i], json[i]);
        break;
      }
    }
    TEST_NOTNULL(NULL);
  }

  printf("%dk schema tested with %dk of json\n",
         static_cast<int>(schema.length() / 1024),
         static_cast<int>(json.length() / 1024));
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
  TestError("table X { Y:byte; } root_type X; { Y: 999 }", "bit field");
  TestError(".0", "floating point");
  TestError("\"\0", "illegal");
  TestError("\"\\q", "escape code");
  TestError("table ///", "documentation");
  TestError("@", "illegal");
  TestError("table 1", "expecting");
  TestError("table X { Y:[[int]]; }", "nested vector");
  TestError("union Z { X } table X { Y:[Z]; }", "vector of union");
  TestError("table X { Y:1; }", "illegal type");
  TestError("table X { Y:int; Y:int; }", "field already");
  TestError("struct X { Y:string; }", "only scalar");
  TestError("struct X { Y:int (deprecated); }", "deprecate");
  TestError("union Z { X } table X { Y:Z; } root_type X; { Y: {}, A:1 }",
            "missing type field");
  TestError("union Z { X } table X { Y:Z; } root_type X; { Y_type: 99, Y: {",
            "type id");
  TestError("table X { Y:int; } root_type X; { Z:", "unknown field");
  TestError("table X { Y:int; } root_type X; { Y:", "string constant", true);
  TestError("table X { Y:int; } root_type X; { \"Y\":1, }", "string constant",
            true);
  TestError("struct X { Y:int; Z:int; } table W { V:X; } root_type W; "
            "{ V:{ Y:1 } }", "wrong number");
  TestError("enum E:byte { A } table X { Y:E; } root_type X; { Y:U }",
            "unknown enum value");
  TestError("table X { Y:byte; } root_type X; { Y:; }", "starting");
  TestError("enum X:byte { Y } enum X {", "enum already");
  TestError("enum X:float {}", "underlying");
  TestError("enum X:byte { Y, Y }", "value already");
  TestError("enum X:byte { Y=2, Z=1 }", "ascending");
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
  TestError("table X { Y:string = 1; }", "scalar");
  TestError("table X { Y:byte; } root_type X; { Y:1, Y:2 }", "more than once");
}

template<typename T> T TestValue(const char *json, const char *type_name) {
  flatbuffers::Parser parser;

  // Simple schema.
  TEST_EQ(parser.Parse(std::string("table X { Y:" + std::string(type_name) + "; } root_type X;").c_str()), true);

  TEST_EQ(parser.Parse(json), true);
  auto root = flatbuffers::GetRoot<T>(parser.builder_.GetBufferPointer());
  // root will point to the table, which is a 32bit vtable offset followed
  // by a float:
  TEST_EQ(sizeof(flatbuffers::soffset_t), 4);  // Test assumes 32bit offsets
  return root[1];
}

bool FloatCompare(float a, float b) { return fabs(a - b) < 0.001; }

// Additional parser testing not covered elsewhere.
void ValueTest() {
  // Test scientific notation numbers.
  TEST_EQ(FloatCompare(TestValue<float>("{ Y:0.0314159e+2 }","float"), (float)3.14159), true);

  // Test conversion functions.
  TEST_EQ(FloatCompare(TestValue<float>("{ Y:cos(rad(180)) }","float"), -1), true);

  // Test negative hex constant.
  TEST_EQ(TestValue<int>("{ Y:-0x80 }","int") == -128, true);
}

void EnumStringsTest() {
  flatbuffers::Parser parser1;
  TEST_EQ(parser1.Parse("enum E:byte { A, B, C } table T { F:[E]; }"
                        "root_type T;"
                        "{ F:[ A, B, \"C\", \"A B C\" ] }"), true);
  flatbuffers::Parser parser2;
  TEST_EQ(parser2.Parse("enum E:byte { A, B, C } table T { F:[int]; }"
                        "root_type T;"
                        "{ F:[ \"E.C\", \"E.A E.B E.C\" ] }"), true);
}

void IntegerOutOfRangeTest() {
  TestError("table T { F:byte; } root_type T; { F:256 }",
            "constant does not fit");
  TestError("table T { F:byte; } root_type T; { F:-257 }",
            "constant does not fit");
  TestError("table T { F:ubyte; } root_type T; { F:256 }",
            "constant does not fit");
  TestError("table T { F:ubyte; } root_type T; { F:-257 }",
            "constant does not fit");
  TestError("table T { F:short; } root_type T; { F:65536 }",
            "constant does not fit");
  TestError("table T { F:short; } root_type T; { F:-65537 }",
            "constant does not fit");
  TestError("table T { F:ushort; } root_type T; { F:65536 }",
            "constant does not fit");
  TestError("table T { F:ushort; } root_type T; { F:-65537 }",
            "constant does not fit");
  TestError("table T { F:int; } root_type T; { F:4294967296 }",
            "constant does not fit");
  TestError("table T { F:int; } root_type T; { F:-4294967297 }",
            "constant does not fit");
  TestError("table T { F:uint; } root_type T; { F:4294967296 }",
            "constant does not fit");
  TestError("table T { F:uint; } root_type T; { F:-4294967297 }",
            "constant does not fit");
}

void UnicodeTest() {
  flatbuffers::Parser parser;
  // Without setting allow_non_utf8 = true, we treat \x sequences as byte sequences
  // which are then validated as UTF-8.
  TEST_EQ(parser.Parse("table T { F:string; }"
                       "root_type T;"
                       "{ F:\"\\u20AC\\u00A2\\u30E6\\u30FC\\u30B6\\u30FC"
                       "\\u5225\\u30B5\\u30A4\\u30C8\\xE2\\x82\\xAC\\u0080\\uD83D\\uDE0E\" }"),
          true);
  std::string jsongen;
  parser.opts.indent_step = -1;
  GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen);
  TEST_EQ(jsongen,
          std::string(
            "{F: \"\\u20AC\\u00A2\\u30E6\\u30FC\\u30B6\\u30FC"
            "\\u5225\\u30B5\\u30A4\\u30C8\\u20AC\\u0080\\uD83D\\uDE0E\"}"));
}

void UnicodeTestAllowNonUTF8() {
  flatbuffers::Parser parser;
  parser.opts.allow_non_utf8 = true;
  TEST_EQ(parser.Parse("table T { F:string; }"
                       "root_type T;"
                       "{ F:\"\\u20AC\\u00A2\\u30E6\\u30FC\\u30B6\\u30FC"
                       "\\u5225\\u30B5\\u30A4\\u30C8\\x01\\x80\\u0080\\uD83D\\uDE0E\" }"), true);
  std::string jsongen;
  parser.opts.indent_step = -1;
  GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen);
  TEST_EQ(jsongen,
          std::string(
            "{F: \"\\u20AC\\u00A2\\u30E6\\u30FC\\u30B6\\u30FC"
            "\\u5225\\u30B5\\u30A4\\u30C8\\u0001\\x80\\u0080\\uD83D\\uDE0E\"}"));
}

void UnicodeSurrogatesTest() {
  flatbuffers::Parser parser;

  TEST_EQ(
    parser.Parse(
      "table T { F:string (id: 0); }"
      "root_type T;"
      "{ F:\"\\uD83D\\uDCA9\"}"), true);
  auto root = flatbuffers::GetRoot<flatbuffers::Table>(
    parser.builder_.GetBufferPointer());
  auto string = root->GetPointer<flatbuffers::String *>(
    flatbuffers::FieldIndexToOffset(0));
  TEST_EQ(strcmp(string->c_str(), "\xF0\x9F\x92\xA9"), 0);
}

void UnicodeInvalidSurrogatesTest() {
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\\uD800\"}", "unpaired high surrogate");
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\\uD800abcd\"}", "unpaired high surrogate");
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\\uD800\\n\"}", "unpaired high surrogate");
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\\uD800\\uD800\"}", "multiple high surrogates");
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\\uDC00\"}", "unpaired low surrogate");
}

void InvalidUTF8Test() {
  // "1 byte" pattern, under min length of 2 bytes
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\x80\"}", "illegal UTF-8 sequence");
  // 2 byte pattern, string too short
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xDF\"}", "illegal UTF-8 sequence");
  // 3 byte pattern, string too short
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xEF\xBF\"}", "illegal UTF-8 sequence");
  // 4 byte pattern, string too short
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xF7\xBF\xBF\"}", "illegal UTF-8 sequence");
  // "5 byte" pattern, string too short
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xFB\xBF\xBF\xBF\"}", "illegal UTF-8 sequence");
  // "6 byte" pattern, string too short
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xFD\xBF\xBF\xBF\xBF\"}", "illegal UTF-8 sequence");
  // "7 byte" pattern, string too short
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xFE\xBF\xBF\xBF\xBF\xBF\"}", "illegal UTF-8 sequence");
  // "5 byte" pattern, over max length of 4 bytes
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xFB\xBF\xBF\xBF\xBF\"}", "illegal UTF-8 sequence");
  // "6 byte" pattern, over max length of 4 bytes
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xFD\xBF\xBF\xBF\xBF\xBF\"}", "illegal UTF-8 sequence");
  // "7 byte" pattern, over max length of 4 bytes
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xFE\xBF\xBF\xBF\xBF\xBF\xBF\"}", "illegal UTF-8 sequence");

  // Three invalid encodings for U+000A (\n, aka NEWLINE)
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xC0\x8A\"}", "illegal UTF-8 sequence");
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xE0\x80\x8A\"}", "illegal UTF-8 sequence");
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xF0\x80\x80\x8A\"}", "illegal UTF-8 sequence");

  // Two invalid encodings for U+00A9 (COPYRIGHT SYMBOL)
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xE0\x81\xA9\"}", "illegal UTF-8 sequence");
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xF0\x80\x81\xA9\"}", "illegal UTF-8 sequence");

  // Invalid encoding for U+20AC (EURO SYMBOL)
  TestError(
    "table T { F:string; }"
    "root_type T;"
    "{ F:\"\xF0\x82\x82\xAC\"}", "illegal UTF-8 sequence");

  // UTF-16 surrogate values between U+D800 and U+DFFF cannot be encoded in UTF-8
  TestError(
    "table T { F:string; }"
    "root_type T;"
    // U+10400 "encoded" as U+D801 U+DC00
    "{ F:\"\xED\xA0\x81\xED\xB0\x80\"}", "illegal UTF-8 sequence");
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
                       "i:10}"), true);

  std::string jsongen;
  parser.opts.indent_step = -1;
  GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen);
  TEST_EQ(jsongen == "{str: \"test\",i: 10}", true);
}

void ParseUnionTest() {
  // Unions must be parseable with the type field following the object.
  flatbuffers::Parser parser;
  TEST_EQ(parser.Parse("table T { A:int; }"
                       "union U { T }"
                       "table V { X:U; }"
                       "root_type V;"
                       "{ X:{ A:1 }, X_type: T }"), true);
  // Unions must be parsable with prefixed namespace.
  flatbuffers::Parser parser2;
  TEST_EQ(parser2.Parse("namespace N; table A {} namespace; union U { N.A }"
                        "table B { e:U; } root_type B;"
                        "{ e_type: N_A, e: {} }"), true);
}

void ConformTest() {
  flatbuffers::Parser parser;
  TEST_EQ(parser.Parse("table T { A:int; } enum E:byte { A }"), true);

  auto test_conform = [&](const char *test, const char *expected_err) {
    flatbuffers::Parser parser2;
    TEST_EQ(parser2.Parse(test), true);
    auto err = parser2.ConformTo(parser);
    TEST_NOTNULL(strstr(err.c_str(), expected_err));
  };

  test_conform("table T { A:byte; }", "types differ for field");
  test_conform("table T { B:int; A:int; }", "offsets differ for field");
  test_conform("table T { A:int = 1; }", "defaults differ for field");
  test_conform("table T { B:float; }", "field renamed to different type");
  test_conform("enum E:byte { B, A }", "values differ for enum");
}

void FixedLengthArrayTest() {
  flatbuffers::FlatBufferBuilder fbb;
  int array[15];
  for (int i = 0; i < 15; i++) array[i] = i + 1;
  ArraysTest::Test1::ArrayStruct aStruct(2, array, 12);
  auto aTable = ArraysTest::Test1::CreateArrayTable(fbb, &aStruct);
  fbb.Finish(aTable);
  auto p = ArraysTest::Test1::GetMutableArrayTable(fbb.GetBufferPointer());
  auto mArStruct = p->mutable_a();
  mArStruct->mutable_b()[14] = -14;
  TEST_EQ(mArStruct->a(), 2);
  TEST_EQ(mArStruct->b_length(), 15);
  TEST_EQ(mArStruct->c(), 12);
  TEST_EQ(mArStruct->b()[14], -14);
  for (int i = 0; i < 14; i++) TEST_EQ(mArStruct->b()[i], i + 1);
}

int main(int /*argc*/, const char * /*argv*/[]) {
  // Run our various test suites:

  std::string rawbuf;
  auto flatbuf = CreateFlatBufferTest(rawbuf);
  AccessFlatBufferTest(reinterpret_cast<const uint8_t *>(rawbuf.c_str()),
                       rawbuf.length());
  AccessFlatBufferTest(flatbuf.get(), rawbuf.length());

  MutateFlatBuffersTest(flatbuf.get(), rawbuf.length());

  ObjectFlatBuffersTest(flatbuf.get());

  #ifndef FLATBUFFERS_NO_FILE_TESTS
  ParseAndGenerateTextTest();
  ReflectionTest(flatbuf.get(), rawbuf.length());
  ParseProtoTest();
  #endif

  FuzzTest1();
  FuzzTest2();

  ErrorTest();
  ValueTest();
  EnumStringsTest();
  IntegerOutOfRangeTest();
  UnicodeTest();
  UnicodeTestAllowNonUTF8();
  UnicodeSurrogatesTest();
  UnicodeInvalidSurrogatesTest();
  InvalidUTF8Test();
  UnknownFieldsTest();
  ParseUnionTest();
  ConformTest();
  FixedLengthArrayTest();

  if (!testing_fails) {
    TEST_OUTPUT_LINE("ALL TESTS PASSED");
    return 0;
  } else {
    TEST_OUTPUT_LINE("%d FAILED TESTS", testing_fails);
    return 1;
  }
}

