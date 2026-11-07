#include "json_test.h"

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "monster_test_bfbs_generated.h"
#include "monster_test_generated.h"
#include "optional_scalars_generated.h"
#include "test_assert.h"

#if !defined(_WIN32)
#include <pthread.h>
#endif

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


namespace {
struct DeepSortedKeyedVectorArgs {
  bool ok;
  bool sorted;
};

void DeepSortedKeyedVectorRun(DeepSortedKeyedVectorArgs *args) {
  args->ok = false;
  args->sorted = false;
  // Text-schema route: keyed table vector, so Parser::ParseVector sorts the
  // serialized element offsets with SimpleQsort (src/idl_parser.cpp).
  const char *schema =
      "table K { k:int (key); } table W { v:[K]; } root_type W;";
  flatbuffers::Parser parser;
  args->ok = parser.Parse(schema);
  if (!args->ok) return;
  // Order [max, 0..n-4, third-largest, second-largest]: every Hoare
  // partition in SimpleQsort peels one element into the tail-side, so the
  // non-tail recursive call keeps n-1 elements, i.e. O(n) recursion depth.
  const size_t n = 20000;
  std::string json = "{v:[";
  for (size_t i = 0; i < n; i++) {
    size_t id;
    if (i == 0) {
      id = n - 1;
    } else if (i == n - 2) {
      id = n - 3;
    } else if (i == n - 1) {
      id = n - 2;
    } else {
      id = i - 1;
    }
    if (i) json += ',';
    json += "{k:";
    json += flatbuffers::NumToString(id);
    json += "}";
  }
  json += "]}";
  args->ok = parser.ParseJson(json.c_str());
  if (args->ok) {
    // Walk the parsed buffer: W.v is W's first field (voffset 4) and K.k is
    // K's first field (voffset 4). Keys are 0..n-1, so element i must carry
    // key i once the keyed vector is sorted.
    auto root = flatbuffers::GetRoot<flatbuffers::Table>(
        parser.builder_.GetBufferPointer());
    auto v = root->GetPointer<
        const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::Table>> *>(
        4);
    args->sorted = (v != nullptr) && (v->size() == n);
    for (size_t i = 0; args->sorted && i < n; i++) {
      if (v->Get(static_cast<uoffset_t>(i))->GetField<int32_t>(4, 0) !=
          static_cast<int32_t>(i)) {
        args->sorted = false;
      }
    }
  }
}
}  // namespace

void JsonDeepSortedKeyedVectorTest() {
  DeepSortedKeyedVectorArgs args;
#if !defined(_WIN32)
  // Parsing runs on a thread with a small stack so the unbounded recursion
  // depth of SimpleQsort overflows deterministically.
  pthread_attr_t attr;
  TEST_EQ(pthread_attr_init(&attr), 0);
  TEST_EQ(pthread_attr_setstacksize(&attr, 256 * 1024), 0);
  pthread_t thread;
  TEST_EQ(pthread_create(&thread, &attr,
                         [](void *p) -> void * {
                           DeepSortedKeyedVectorRun(
                               static_cast<DeepSortedKeyedVectorArgs *>(p));
                           return nullptr;
                         },
                         &args),
          0);
  TEST_EQ(pthread_join(thread, nullptr), 0);
#else
  DeepSortedKeyedVectorRun(&args);
#endif
  TEST_EQ(args.ok, true);
  TEST_EQ(args.sorted, true);
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
}

}  // namespace tests
}  // namespace flatbuffers
