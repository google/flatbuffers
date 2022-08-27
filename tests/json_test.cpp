#include "json_test.h"

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "monster_test_generated.h"
#include "optional_scalars_generated.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {

using namespace MyGame::Example;

// Check stringify of an default enum value to json
void JsonDefaultTest(const std::string& tests_data_path) {
  // load FlatBuffer schema (.fbs) from disk
  std::string schemafile;
  TEST_EQ(flatbuffers::LoadFile((tests_data_path + "monster_test.fbs").c_str(),
                                false, &schemafile),
          true);
  // parse schema first, so we can use it to parse the data after
  flatbuffers::Parser parser;
  auto include_test_path =
      flatbuffers::ConCatPathFileName(tests_data_path, "include_test");
  const char *include_directories[] = { tests_data_path.c_str(),
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

void JsonEnumsTest(const std::string& tests_data_path) {
  // load FlatBuffer schema (.fbs) from disk
  std::string schemafile;
  TEST_EQ(flatbuffers::LoadFile((tests_data_path + "monster_test.fbs").c_str(),
                                false, &schemafile),
          true);
  // parse schema first, so we can use it to parse the data after
  flatbuffers::Parser parser;
  auto include_test_path =
      flatbuffers::ConCatPathFileName(tests_data_path, "include_test");
  const char *include_directories[] = { tests_data_path.c_str(),
                                        include_test_path.c_str(), nullptr };
  parser.opts.output_enum_identifiers = true;
  TEST_EQ(parser.Parse(schemafile.c_str(), include_directories), true);
  flatbuffers::FlatBufferBuilder builder;
  auto name = builder.CreateString("bitflag_enum");
  MonsterBuilder color_monster(builder);
  color_monster.add_name(name);
  color_monster.add_color(Color(Color_Blue | Color_Red));
  FinishMonsterBuffer(builder, color_monster.Finish());
  std::string jsongen;
  auto result = GenerateText(parser, builder.GetBufferPointer(), &jsongen);
  TEST_EQ(result, true);
  TEST_EQ(std::string::npos != jsongen.find("color: \"Red Blue\""), true);
  // Test forward compatibility with 'output_enum_identifiers = true'.
  // Current Color doesn't have '(1u << 2)' field, let's add it.
  builder.Clear();
  std::string future_json;
  auto future_name = builder.CreateString("future bitflag_enum");
  MonsterBuilder future_color(builder);
  future_color.add_name(future_name);
  future_color.add_color(
      static_cast<Color>((1u << 2) | Color_Blue | Color_Red));
  FinishMonsterBuffer(builder, future_color.Finish());
  result = GenerateText(parser, builder.GetBufferPointer(), &future_json);
  TEST_EQ(result, true);
  TEST_EQ(std::string::npos != future_json.find("color: 13"), true);
}

void JsonOptionalTest(const std::string& tests_data_path, bool default_scalars) {
  // load FlatBuffer schema (.fbs) and JSON from disk
  std::string schemafile;
  std::string jsonfile;
  TEST_EQ(
      flatbuffers::LoadFile((tests_data_path + "optional_scalars.fbs").c_str(),
                            false, &schemafile),
      true);
  TEST_EQ(flatbuffers::LoadFile((tests_data_path + "optional_scalars" +
                                 (default_scalars ? "_defaults" : "") + ".json")
                                    .c_str(),
                                false, &jsonfile),
          true);

  auto include_test_path =
      flatbuffers::ConCatPathFileName(tests_data_path, "include_test");
  const char *include_directories[] = { tests_data_path.c_str(),
                                        include_test_path.c_str(), nullptr };

  // parse schema first, so we can use it to parse the data after
  flatbuffers::Parser parser;
  parser.opts.output_default_scalars_in_json = default_scalars;
  TEST_EQ(parser.Parse(schemafile.c_str(), include_directories), true);
  TEST_EQ(parser.ParseJson(jsonfile.c_str()), true);

  // here, parser.builder_ contains a binary buffer that is the parsed data.

  // First, verify it, just in case:
  flatbuffers::Verifier verifier(parser.builder_.GetBufferPointer(),
                                 parser.builder_.GetSize());
  TEST_EQ(optional_scalars::VerifyScalarStuffBuffer(verifier), true);

  // to ensure it is correct, we now generate text back from the binary,
  // and compare the two:
  std::string jsongen;
  auto result =
      GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen);
  TEST_EQ(result, true);
  TEST_EQ_STR(jsongen.c_str(), jsonfile.c_str());
}

}  // namespace tests
}  // namespace flatbuffers