#include "evolution_test.h"

#include "evolution_test/evolution_v1_generated.h"
#include "evolution_test/evolution_v2_generated.h"
#include "flatbuffers/idl.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {

void EvolutionTest(const std::string &tests_data_path) {
  // VS10 does not support typed enums, exclude from tests
#if !defined(_MSC_VER) || _MSC_VER >= 1700
  const int NUM_VERSIONS = 2;
  std::string schemas[NUM_VERSIONS];
  std::string jsonfiles[NUM_VERSIONS];
  std::vector<uint8_t> binaries[NUM_VERSIONS];

  flatbuffers::IDLOptions idl_opts;
  idl_opts.lang_to_generate |= flatbuffers::IDLOptions::kBinary;
  flatbuffers::Parser parser(idl_opts);

  // Load all the schema versions and their associated data.
  for (int i = 0; i < NUM_VERSIONS; ++i) {
    std::string schema = tests_data_path + "evolution_test/evolution_v" +
                         flatbuffers::NumToString(i + 1) + ".fbs";
    TEST_ASSERT(flatbuffers::LoadFile(schema.c_str(), false, &schemas[i]));
    std::string json = tests_data_path + "evolution_test/evolution_v" +
                       flatbuffers::NumToString(i + 1) + ".json";
    TEST_ASSERT(flatbuffers::LoadFile(json.c_str(), false, &jsonfiles[i]));

    TEST_ASSERT(parser.Parse(schemas[i].c_str()));
    TEST_ASSERT(parser.Parse(jsonfiles[i].c_str()));

    auto bufLen = parser.builder_.GetSize();
    auto buf = parser.builder_.GetBufferPointer();
    binaries[i].reserve(bufLen);
    std::copy(buf, buf + bufLen, std::back_inserter(binaries[i]));
  }

  // Assert that all the verifiers for the different schema versions properly
  // verify any version data.
  for (int i = 0; i < NUM_VERSIONS; ++i) {
    flatbuffers::Verifier verifier(&binaries[i].front(), binaries[i].size());
    TEST_ASSERT(Evolution::V1::VerifyRootBuffer(verifier));
    TEST_ASSERT(Evolution::V2::VerifyRootBuffer(verifier));
  }

  // Test backwards compatibility by reading old data with an evolved schema.
  auto root_v1_viewed_from_v2 = Evolution::V2::GetRoot(&binaries[0].front());
  // field 'k' is new in version 2, so it should be null.
  TEST_ASSERT(nullptr == root_v1_viewed_from_v2->k());
  // field 'l' is new in version 2 with a default of 56.
  TEST_EQ(root_v1_viewed_from_v2->l(), 56);
  // field 'c' of 'TableA' is new in version 2, so it should be null.
  TEST_ASSERT(nullptr == root_v1_viewed_from_v2->e()->c());
  // 'TableC' was added to field 'c' union in version 2, so it should be null.
  TEST_ASSERT(nullptr == root_v1_viewed_from_v2->c_as_TableC());
  // The field 'c' union should be of type 'TableB' regardless of schema version
  TEST_ASSERT(root_v1_viewed_from_v2->c_type() == Evolution::V2::Union::TableB);
  // The field 'f' was renamed to 'ff' in version 2, it should still be
  // readable.
  TEST_EQ(root_v1_viewed_from_v2->ff()->a(), 16);

  // Test forwards compatibility by reading new data with an old schema.
  auto root_v2_viewed_from_v1 = Evolution::V1::GetRoot(&binaries[1].front());
  // The field 'c' union in version 2 is a new table (index = 3) and should
  // still be accessible, but not interpretable.
  TEST_EQ(static_cast<uint8_t>(root_v2_viewed_from_v1->c_type()), 3);
  TEST_NOTNULL(root_v2_viewed_from_v1->c());
  // The field 'd' enum in verison 2 has new members and should still be
  // accessible, but not interpretable.
  TEST_EQ(static_cast<int8_t>(root_v2_viewed_from_v1->d()), 3);
  // The field 'a' in version 2 is deprecated and should return the default
  // value (0) instead of the value stored in the in the buffer (42).
  TEST_EQ(root_v2_viewed_from_v1->a(), 0);
  // The field 'ff' was originally named 'f' in version 1, it should still be
  // readable.
  TEST_EQ(root_v2_viewed_from_v1->f()->a(), 35);
#endif
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
  test_conform(parser, "table T { }", "field deleted");
  test_conform(parser, "table T { B:int; }", ""); //renaming a field is allowed
}

void UnionDeprecationTest(const std::string& tests_data_path) {
  const int NUM_VERSIONS = 2;
  std::string schemas[NUM_VERSIONS];
  std::string jsonfiles[NUM_VERSIONS];
  std::vector<uint8_t> binaries[NUM_VERSIONS];

  flatbuffers::IDLOptions idl_opts;
  idl_opts.lang_to_generate |= flatbuffers::IDLOptions::kBinary;
  flatbuffers::Parser parser(idl_opts);

  // Load all the schema versions and their associated data.
  for (int i = 0; i < NUM_VERSIONS; ++i) {
    std::string schema = tests_data_path + "evolution_test/evolution_v" +
                         flatbuffers::NumToString(i + 1) + ".fbs";
    TEST_ASSERT(flatbuffers::LoadFile(schema.c_str(), false, &schemas[i]));
    std::string json = tests_data_path + "evolution_test/evolution_v" +
                       flatbuffers::NumToString(i + 1) + ".json";
    TEST_ASSERT(flatbuffers::LoadFile(json.c_str(), false, &jsonfiles[i]));

    TEST_ASSERT(parser.Parse(schemas[i].c_str()));
    TEST_ASSERT(parser.Parse(jsonfiles[i].c_str()));

    auto bufLen = parser.builder_.GetSize();
    auto buf = parser.builder_.GetBufferPointer();
    binaries[i].reserve(bufLen);
    std::copy(buf, buf + bufLen, std::back_inserter(binaries[i]));
  }

  auto v2 = parser.LookupStruct("Evolution.V2.Root");
  TEST_NOTNULL(v2);
  auto j_type_field = v2->fields.Lookup("j_type");
  TEST_NOTNULL(j_type_field);
  TEST_ASSERT(j_type_field->deprecated);
}

}  // namespace tests
}  // namespace flatbuffers