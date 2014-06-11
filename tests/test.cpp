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
#include "flatbuffers/util.h"

#include "monster_test_generated.h"

#include <random>

using namespace MyGame::Example;

#ifdef __ANDROID__
  #include <android/log.h>
  #define TEST_OUTPUT_LINE(...) \
    __android_log_print(ANDROID_LOG_INFO, "FlatBuffers", __VA_ARGS__)
#else
  #define TEST_OUTPUT_LINE(...) \
    { printf(__VA_ARGS__); printf("\n"); }
#endif

int testing_fails = 0;

template<typename T, typename U>
void TestEq(T expval, U val, const char *exp, const char *file, int line) {
  if (expval != val) {
    auto expval_str = flatbuffers::NumToString(expval);
    auto val_str = flatbuffers::NumToString(val);
    TEST_OUTPUT_LINE("TEST FAILED: %s:%d, %s (%s) != %s", file, line,
                     exp, expval_str.c_str(), val_str.c_str());
    assert(0);
    testing_fails++;
  }
}

#define TEST_EQ(exp, val) TestEq( exp, val,   #exp, __FILE__, __LINE__)
#define TEST_NOTNULL(exp) TestEq(!exp, false, #exp, __FILE__, __LINE__)

// Include simple random number generator to ensure results will be the
// same cross platform.
// http://en.wikipedia.org/wiki/Park%E2%80%93Miller_random_number_generator
uint32_t lcg_seed = 48271;
uint32_t lcg_rand() {
    return lcg_seed = ((uint64_t)lcg_seed * 279470273UL) % 4294967291UL;
}
void lcg_reset() { lcg_seed = 48271; }

// example of how to build up a serialized buffer algorithmically:
std::string CreateFlatBufferTest() {
  flatbuffers::FlatBufferBuilder builder;

  auto vec = Vec3(1, 2, 3, 0, 0, Test(10, 20));

  auto name = builder.CreateString("MyMonster");

  unsigned char inv_data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  auto inventory = builder.CreateVector(inv_data, 10);

  Test tests[] = { Test(10, 20), Test(30, 40) };
  auto testv = builder.CreateVectorOfStructs(tests, 2);

  // create monster with very few fields set:
  // (same functionality as CreateMonster below, but sets fields manually)
  MonsterBuilder mb(builder);
  mb.add_hp(20);
  auto mloc2 = mb.Finish();

  // shortcut for creating monster with all fields set:
  auto mloc = CreateMonster(builder, &vec, 150, 80, name, inventory, Color_Blue,
                            Any_Monster, mloc2.Union(), // Store a union.
                            testv);

  builder.Finish(mloc);

  #ifdef FLATBUFFERS_TEST_VERBOSE
  // print byte data for debugging:
  auto p = builder.GetBufferPointer();
  for (flatbuffers::uoffset_t i = 0; i < builder.GetSize(); i++)
    printf("%d ", p[i]);
  #endif

  // return the buffer for the caller to use.
  return std::string(reinterpret_cast<const char *>(builder.GetBufferPointer()),
                     builder.GetSize());
}

//  example of accessing a buffer loaded in memory:
void AccessFlatBufferTest(const std::string &flatbuf) {

  auto monster = GetMonster(flatbuf.c_str());

  TEST_EQ(monster->hp(), 80);
  TEST_EQ(monster->mana(), 150);  // default
  TEST_EQ(strcmp(monster->name()->c_str(), "MyMonster"), 0);
  // Can't access the following field, it is deprecated in the schema,
  // which means accessors are not generated:
  // monster.friendly()

  auto pos = monster->pos();
  TEST_NOTNULL(pos);
  TEST_EQ(pos->z(), 3);
  TEST_EQ(pos->test3().a(), 10);
  TEST_EQ(pos->test3().b(), 20);

  auto inventory = monster->inventory();
  TEST_NOTNULL(inventory);
  unsigned char inv_data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  for (flatbuffers::uoffset_t i = 0; i < inventory->Length(); i++)
    TEST_EQ(inventory->Get(i), inv_data[i]);

  // Example of accessing a union:
  TEST_EQ(monster->test_type(), Any_Monster);  // First make sure which it is.
  auto monster2 = reinterpret_cast<const Monster *>(monster->test());
  TEST_NOTNULL(monster2);
  TEST_EQ(monster2->hp(), 20);

  // Since Flatbuffers uses explicit mechanisms to override the default
  // compiler alignment, double check that the compiler indeed obeys them:
  // (Test consists of a short and byte):
  TEST_EQ(flatbuffers::AlignOf<Test>(), static_cast<size_t>(2));
  TEST_EQ(sizeof(Test), static_cast<size_t>(4));

  auto tests = monster->test4();
  TEST_NOTNULL(tests);
  auto &test_0 = tests->Get(0);
  auto &test_1 = tests->Get(1);
  TEST_EQ(test_0.a(), 10);
  TEST_EQ(test_0.b(), 20);
  TEST_EQ(test_1.a(), 30);
  TEST_EQ(test_1.b(), 40);
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
    "tests/monsterdata_test.json", false, &jsonfile), true);

  // parse schema first, so we can use it to parse the data after
  flatbuffers::Parser parser;
  TEST_EQ(parser.Parse(schemafile.c_str()), true);
  TEST_EQ(parser.Parse(jsonfile.c_str()), true);

  // here, parser.builder_ contains a binary buffer that is the parsed data.

  // to ensure it is correct, we now generate text back from the binary,
  // and compare the two:
  std::string jsongen;
  GenerateText(parser, parser.builder_.GetBufferPointer(), 2, &jsongen);

  if (jsongen != jsonfile) {
    printf("%s----------------\n%s", jsongen.c_str(), jsonfile.c_str());
    TEST_NOTNULL(NULL);
  }
}

template<typename T> void CompareTableFieldValue(flatbuffers::Table *table,
                                                 int voffset, T val) {
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
  const int64_t  long_val   = 0x8444444444444444;
  const uint64_t ulong_val  = 0xFCCCCCCCCCCCCCCC;
  const float    float_val  = 3.14159f;
  const double   double_val = 3.14159265359;

  const int test_values_max = 11;
  const int fields_per_object = 4;
  const int num_fuzz_objects = 10000;  // The higher, the more thorough :)

  flatbuffers::FlatBufferBuilder builder;

  lcg_reset();  // Keep it deterministic.

  flatbuffers::uoffset_t objects[num_fuzz_objects];

  // Generate num_fuzz_objects random objects each consisting of
  // fields_per_object fields, each of a random type.
  for (int i = 0; i < num_fuzz_objects; i++) {
    auto start = builder.StartTable();
    for (int f = 0; f < fields_per_object; f++) {
      int choice = lcg_rand() % test_values_max;
      flatbuffers::voffset_t off = flatbuffers::FieldIndexToOffset(f);
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

  uint8_t *eob = builder.GetBufferPointer() + builder.GetSize();

  // Test that all objects we generated are readable and return the
  // expected values. We generate random objects in the same order
  // so this is deterministic.
  for (int i = 0; i < num_fuzz_objects; i++) {
    auto table = reinterpret_cast<flatbuffers::Table *>(eob - objects[i]);
    for (int f = 0; f < fields_per_object; f++) {
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

  std::string schema = "namespace test;\n\n";

  struct RndDef {
    std::string instances[instances_per_definition];
  };

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
    // Since we're generating schema & and corresponding data in tandem,
    // this convenience function adds strings to both at once.
    auto AddToSchemaAndInstances = [&](const char *schema_add,
                                       const char *instance_add) {
      schema += schema_add;
      for (int i = 0; i < instances_per_definition; i++)
        definitions[definition].instances[i] += instance_add;
    };
    // Generate a default type if we can't generate something else.
    auto Dummy = [&]() { AddToSchemaAndInstances("byte", "1"); };

    std::string definition_name = "D" + flatbuffers::NumToString(definition);

    bool is_struct = definition < num_struct_definitions;

    AddToSchemaAndInstances(
      ((is_struct ? "struct " : "table ") + definition_name + " {\n").c_str(),
      "{\n");

    for (int field = 0; field < fields_per_definition; field++) {
      std::string field_name = "f" + flatbuffers::NumToString(field);
      AddToSchemaAndInstances(("  " + field_name + ":").c_str(),
                              (field_name + ": ").c_str());
      // Pick random type:
      int base_type = lcg_rand() % (flatbuffers::BASE_TYPE_UNION + 1);
      switch (base_type) {
        case flatbuffers::BASE_TYPE_STRING:
          if (is_struct) {
            Dummy();  // No strings in structs,
          } else {
            AddToSchemaAndInstances("string", "\"hi\"");
          }
          break;
        case flatbuffers::BASE_TYPE_NONE:
        case flatbuffers::BASE_TYPE_UTYPE:
        case flatbuffers::BASE_TYPE_STRUCT:
        case flatbuffers::BASE_TYPE_UNION:
        case flatbuffers::BASE_TYPE_VECTOR:
          if (definition) {
            // Pick a random previous definition and random data instance of
            // that definition.
            int defref = lcg_rand() % definition;
            int instance = lcg_rand() % instances_per_definition;
            AddToSchemaAndInstances(
              ("D" + flatbuffers::NumToString(defref)).c_str(),
              definitions[defref].instances[instance].c_str());
          } else {
            // If this is the first definition, we have no definition we can
            // refer to.
            Dummy();
          }
          break;
        default:
          // All the scalar types.
          AddToSchemaAndInstances(
            flatbuffers::kTypeNames[base_type],
            flatbuffers::NumToString(lcg_rand() % 128).c_str());
      }
      AddToSchemaAndInstances(
        ";\n",
        field == fields_per_definition - 1 ? "\n" : ",\n");
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
  GenerateText(parser, parser.builder_.GetBufferPointer(), 0, &jsongen);

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
void TestError(const char *src, const char *error_substr) {
  flatbuffers::Parser parser;
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
  TestError("union Z { X } table X { Y:Z; } root_type X; { Y: {",
            "missing type field");
  TestError("union Z { X } table X { Y:Z; } root_type X; { Y_type: 99, Y: {",
            "type id");
  TestError("table X { Y:int; } root_type X; { Z:", "unknown field");
  TestError("struct X { Y:int; Z:int; } table W { V:X; } root_type W; "
            "{ V:{ Y:1 } }", "incomplete");
  TestError("table X { Y:byte; } root_type X; { Y:U }", "valid enum");
  TestError("table X { Y:byte; } root_type X; { Y:; }", "starting");
  TestError("enum X:byte { Y } enum X {", "enum already");
  TestError("enum X:float {}", "underlying");
  TestError("enum X:byte { Y, Y }", "value already");
  TestError("enum X:byte { Y=2, Z=1 }", "ascending");
  TestError("table X { Y:int; } table X {", "datatype already");
  TestError("struct X (force_align: 7) { Y:int; }", "force_align");
  TestError("{}", "no root");
  TestError("table X { Y:byte; } root_type X; { Y:1 } { Y:1 }", "one json");
  TestError("root_type X;", "unknown root");
  TestError("struct X { Y:int; } root_type X;", "a table");
  TestError("union X { Y }", "referenced");
  TestError("union Z { X } struct X { Y:int; }", "only tables");
}

int main(int argc, const char *argv[]) {
  // Run our various test suites:

  auto flatbuf = CreateFlatBufferTest();
  AccessFlatBufferTest(flatbuf);

  #ifndef __ANDROID__  // requires file access
  ParseAndGenerateTextTest();
  #endif

  FuzzTest1();
  FuzzTest2();

  ErrorTest();

  if (!testing_fails) {
    TEST_OUTPUT_LINE("ALL TESTS PASSED");
    return 0;
  } else {
    TEST_OUTPUT_LINE("%d FAILED TESTS", testing_fails);
    return 1;
  }
}

