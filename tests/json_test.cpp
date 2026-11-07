#include "json_test.h"

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/flexbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include "monster_test_bfbs_generated.h"
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
  const char* include_directories[] = {tests_data_path.c_str(),
                                       include_test_path.c_str(), nullptr};

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
  auto result = GenText(parser, builder.GetBufferPointer(), &jsongen);
  TEST_NULL(result);
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
  const char* include_directories[] = {tests_data_path.c_str(),
                                       include_test_path.c_str(), nullptr};
  parser.opts.output_enum_identifiers = true;
  TEST_EQ(parser.Parse(schemafile.c_str(), include_directories), true);
  flatbuffers::FlatBufferBuilder builder;
  auto name = builder.CreateString("bitflag_enum");
  MonsterBuilder color_monster(builder);
  color_monster.add_name(name);
  color_monster.add_color(Color(Color_Blue | Color_Red));
  FinishMonsterBuffer(builder, color_monster.Finish());
  std::string jsongen;
  auto result = GenText(parser, builder.GetBufferPointer(), &jsongen);
  TEST_NULL(result);
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
  result = GenText(parser, builder.GetBufferPointer(), &future_json);
  TEST_NULL(result);
  TEST_EQ(std::string::npos != future_json.find("color: 13"), true);
}

void JsonOptionalTest(const std::string& tests_data_path,
                      bool default_scalars) {
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
  const char* include_directories[] = {tests_data_path.c_str(),
                                       include_test_path.c_str(), nullptr};

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
  auto result = GenText(parser, parser.builder_.GetBufferPointer(), &jsongen);
  TEST_NULL(result);
  TEST_EQ_STR(jsongen.c_str(), jsonfile.c_str());
}

void ParseIncorrectMonsterJsonTest(const std::string& tests_data_path) {
  std::string schemafile;
  TEST_EQ(flatbuffers::LoadFile((tests_data_path + "monster_test.bfbs").c_str(),
                                true, &schemafile),
          true);
  flatbuffers::Parser parser;
  flatbuffers::Verifier verifier(
      reinterpret_cast<const uint8_t*>(schemafile.c_str()), schemafile.size());
  TEST_EQ(reflection::VerifySchemaBuffer(verifier), true);
  TEST_EQ(
      parser.Deserialize(reinterpret_cast<const uint8_t*>(schemafile.c_str()),
                         schemafile.size()),
      true);
  TEST_EQ(parser.ParseJson("{name:\"monster\"}"), true);
  TEST_EQ(parser.ParseJson(""), false);
  TEST_EQ(parser.ParseJson("{name: 1}"), false);
  TEST_EQ(parser.ParseJson("{name:+1}"), false);
  TEST_EQ(parser.ParseJson("{name:-1}"), false);
  TEST_EQ(parser.ParseJson("{name:-f}"), false);
  TEST_EQ(parser.ParseJson("{name:+f}"), false);
}

void JsonUnsortedArrayTest() {
  flatbuffers::Parser parser;
  TEST_EQ(parser.Deserialize(MyGame::Example::MonsterBinarySchema::data(),
                             MyGame::Example::MonsterBinarySchema::size()),
          true);
  auto jsonStr = R"(
  {
    "name": "lookupTest",
    "testarrayoftables": [
      { "name": "aaa" },
      { "name": "ccc" },
      { "name": "bbb" }
    ]
  }
  )";
  TEST_EQ(parser.ParseJson(jsonStr), true);
  auto monster = flatbuffers::GetRoot<MyGame::Example::Monster>(
      parser.builder_.GetBufferPointer());

  TEST_NOTNULL(monster->testarrayoftables()->LookupByKey("aaa"));
  TEST_NOTNULL(monster->testarrayoftables()->LookupByKey("bbb"));
  TEST_NOTNULL(monster->testarrayoftables()->LookupByKey("ccc"));
}

void JsonUnionStructTest() {
  // schema to parse data
  auto schema = R"(
struct MyStruct { field: int; }
union UnionWithStruct { MyStruct }
table JsonUnionStructTest { union_with_struct: UnionWithStruct; }
root_type JsonUnionStructTest;
)";
  // source text to parse and expected result of generation text back
  auto json_source = R"({
  union_with_struct_type: "MyStruct",
  union_with_struct: {
    field: 12345
  }
}
)";

  flatbuffers::Parser parser;
  // set output language to JSON, so we assure that is supported
  parser.opts.lang_to_generate = IDLOptions::kJson;
  // parse schema first, so we assure that output language is supported
  // and can use it to parse the data after
  TEST_EQ(true, parser.Parse(schema));
  TEST_EQ(true, parser.ParseJson(json_source));

  // now generate text back from the binary, and compare the two:
  std::string json_generated;
  auto generate_result =
      GenText(parser, parser.builder_.GetBufferPointer(), &json_generated);
  TEST_NULL(generate_result);
  TEST_EQ_STR(json_source, json_generated.c_str());

  // Positive control: valid nested FlexBuffer and GenerateTextFromTable.
  const char* flex_schema =
      "table FlexHolder { flex:[ubyte] (flexbuffer); }\n"
      "root_type FlexHolder;\n";
  flatbuffers::Parser flex_parser;
  TEST_EQ(flex_parser.Parse(flex_schema), true);
  flexbuffers::Builder valid_slb;
  valid_slb.Int(42);
  valid_slb.Finish();
  flatbuffers::FlatBufferBuilder valid_flex_fbb;
  auto valid_flex_vec = valid_flex_fbb.CreateVector(valid_slb.GetBuffer());
  auto valid_flex_start = valid_flex_fbb.StartTable();
  valid_flex_fbb.AddOffset(4, valid_flex_vec);
  auto valid_flex_root = valid_flex_fbb.EndTable(valid_flex_start);
  valid_flex_fbb.Finish(flatbuffers::Offset<flatbuffers::Table>(valid_flex_root));
  std::string valid_flex_json;
  TEST_NULL(GenText(flex_parser, valid_flex_fbb.GetBufferPointer(), &valid_flex_json));
  TEST_EQ(valid_flex_json.find("42") != std::string::npos, true);
  std::string valid_table_json;
  TEST_EQ(GenerateTextFromTable(
              flex_parser,
              flatbuffers::GetRoot<flatbuffers::Table>(valid_flex_fbb.GetBufferPointer()),
              "FlexHolder", &valid_table_json),
          true);
  TEST_EQ(GenerateTextFromTable(
              flex_parser,
              flatbuffers::GetRoot<flatbuffers::Table>(valid_flex_fbb.GetBufferPointer()),
              "UnknownTable", &valid_table_json),
          false);

  // Negative test: reject malformed nested FlexBuffer payload in GenFieldOffset.
  flatbuffers::FlatBufferBuilder invalid_flex_fbb;
  const uint8_t invalid_flex_bytes[] = {0, 0};
  auto invalid_flex_vec = invalid_flex_fbb.CreateVector(invalid_flex_bytes, 2);
  auto invalid_flex_start = invalid_flex_fbb.StartTable();
  invalid_flex_fbb.AddOffset(4, invalid_flex_vec);
  auto invalid_flex_root = invalid_flex_fbb.EndTable(invalid_flex_start);
  invalid_flex_fbb.Finish(
      flatbuffers::Offset<flatbuffers::Table>(invalid_flex_root));
  std::string invalid_flex_json;
  const char* flex_err = GenText(
      flex_parser, invalid_flex_fbb.GetBufferPointer(), &invalid_flex_json);
  TEST_NOTNULL(flex_err);
  if (flex_err) {
    TEST_EQ_STR(flex_err, "invalid nested flexbuffer");
  }

  // Negative test: reject union value vectors longer than their union type vector.
  const char* union_schema =
      "table Inner { x:int; }\n"
      "union U { Inner }\n"
      "table UnionBoundsHolder { tag:ubyte; u:U; uv:[U]; }\n"
      "root_type UnionBoundsHolder;\n";
  flatbuffers::Parser union_parser;
  TEST_EQ(union_parser.Parse(union_schema), true);

  flatbuffers::FlatBufferBuilder short_uvec_fbb;
  auto in1_start = short_uvec_fbb.StartTable();
  short_uvec_fbb.AddElement<int32_t>(4, 11, 0);
  auto in1 = short_uvec_fbb.EndTable(in1_start);
  auto in2_start = short_uvec_fbb.StartTable();
  short_uvec_fbb.AddElement<int32_t>(4, 22, 0);
  auto in2 = short_uvec_fbb.EndTable(in2_start);
  const uint8_t short_types[] = {1};
  auto uv_type_off = short_uvec_fbb.CreateVector(short_types, 1);
  // Overwrite the first trailing alignment-padding byte after the 1-element
  // uint8_t vector payload with 1 (Inner) so an unpatched out-of-bounds
  // type_vec->Get(1) read deterministically sees a non-NONE union tag.
  short_uvec_fbb.GetCurrentBufferPointer()[5] = 1;
  const flatbuffers::Offset<void> uv_vals[] = {
      flatbuffers::Offset<void>(in1), flatbuffers::Offset<void>(in2)};
  auto uv_val_off = short_uvec_fbb.CreateVector(uv_vals, 2);
  auto uv_holder_start = short_uvec_fbb.StartTable();
  short_uvec_fbb.AddOffset(10, uv_type_off);
  short_uvec_fbb.AddOffset(12, uv_val_off);
  auto uv_holder = short_uvec_fbb.EndTable(uv_holder_start);
  short_uvec_fbb.Finish(flatbuffers::Offset<flatbuffers::Table>(uv_holder));
  std::string short_uvec_json;
  const char* short_uvec_err = GenText(
      union_parser, short_uvec_fbb.GetBufferPointer(), &short_uvec_json);
  TEST_NOTNULL(short_uvec_err);
  if (short_uvec_err) {
    TEST_EQ_STR(short_uvec_err, "union type vector too short");
  }

  // Positive control and negative boundary tests for RelativeToRootPath and ConvertCase.
  TEST_EQ_STR(RelativeToRootPath("/a/b", "/a/c/output.fbs").c_str(),
              "//../c/output.fbs");
  const std::string expected_empty_project =
      "//" + flatbuffers::PosixPath(flatbuffers::AbsolutePath("output.fbs"));
  TEST_EQ_STR(flatbuffers::RelativeToRootPath("", "output.fbs").c_str(),
              expected_empty_project.c_str());
  std::string empty_filepath_res;
  try {
    empty_filepath_res = RelativeToRootPath("/a/b", "");
  } catch (const std::exception&) {
    empty_filepath_res = "out_of_range";
  }
  TEST_EQ_STR(empty_filepath_res.c_str(), "//");
  const std::string high_byte_input("a\x80z", 3);
  TEST_EQ(ConvertCase(high_byte_input, Case::kScreamingSnake).empty(), false);
  TEST_EQ(ConvertCase(high_byte_input, Case::kSnake).empty(), false);
  TEST_EQ(ConvertCase(high_byte_input, Case::kDasher).empty(), false);
  TEST_EQ(ConvertCase("a_1\x80z", Case::kSnake2, Case::kSnake).empty(), false);
}

}  // namespace tests
}  // namespace flatbuffers
