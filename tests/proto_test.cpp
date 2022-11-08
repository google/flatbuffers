#include "proto_test.h"

#include "flatbuffers/idl.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {

// Parse a .proto schema, output as .fbs
void ParseProtoTest(const std::string &tests_data_path) {
  // load the .proto and the golden file from disk
  std::string protofile;
  std::string goldenfile;
  std::string goldenunionfile;
  TEST_EQ(
      flatbuffers::LoadFile((tests_data_path + "prototest/test.proto").c_str(),
                            false, &protofile),
      true);
  TEST_EQ(
      flatbuffers::LoadFile((tests_data_path + "prototest/test.golden").c_str(),
                            false, &goldenfile),
      true);
  TEST_EQ(flatbuffers::LoadFile(
              (tests_data_path + "prototest/test_union.golden").c_str(), false,
              &goldenunionfile),
          true);

  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = false;
  opts.proto_mode = true;

  // Parse proto.
  flatbuffers::Parser parser(opts);
  auto protopath = tests_data_path + "prototest/";
  const char *include_directories[] = { protopath.c_str(), nullptr };
  TEST_EQ(parser.Parse(protofile.c_str(), include_directories), true);

  // Generate fbs.
  auto fbs = flatbuffers::GenerateFBS(parser, "test");

  // Ensure generated file is parsable.
  flatbuffers::Parser parser2;
  TEST_EQ(parser2.Parse(fbs.c_str(), nullptr), true);
  TEST_EQ_STR(fbs.c_str(), goldenfile.c_str());

  // Parse proto with --oneof-union option.
  opts.proto_oneof_union = true;
  flatbuffers::Parser parser3(opts);
  TEST_EQ(parser3.Parse(protofile.c_str(), include_directories), true);

  // Generate fbs.
  auto fbs_union = flatbuffers::GenerateFBS(parser3, "test");

  // Ensure generated file is parsable.
  flatbuffers::Parser parser4;
  TEST_EQ(parser4.Parse(fbs_union.c_str(), nullptr), true);
  TEST_EQ_STR(fbs_union.c_str(), goldenunionfile.c_str());
}

// Parse a .proto schema, output as .fbs
void ParseProtoTestWithSuffix(const std::string &tests_data_path) {
  // load the .proto and the golden file from disk
  std::string protofile;
  std::string goldenfile;
  std::string goldenunionfile;
  TEST_EQ(
      flatbuffers::LoadFile((tests_data_path + "prototest/test.proto").c_str(),
                            false, &protofile),
      true);
  TEST_EQ(flatbuffers::LoadFile(
              (tests_data_path + "prototest/test_suffix.golden").c_str(), false,
              &goldenfile),
          true);
  TEST_EQ(flatbuffers::LoadFile(
              (tests_data_path + "prototest/test_union_suffix.golden").c_str(),
              false, &goldenunionfile),
          true);

  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = false;
  opts.proto_mode = true;
  opts.proto_namespace_suffix = "test_namespace_suffix";

  // Parse proto.
  flatbuffers::Parser parser(opts);
  auto protopath = tests_data_path + "prototest/";
  const char *include_directories[] = { protopath.c_str(), nullptr };
  TEST_EQ(parser.Parse(protofile.c_str(), include_directories), true);

  // Generate fbs.
  auto fbs = flatbuffers::GenerateFBS(parser, "test");

  // Ensure generated file is parsable.
  flatbuffers::Parser parser2;
  TEST_EQ(parser2.Parse(fbs.c_str(), nullptr), true);
  TEST_EQ_STR(fbs.c_str(), goldenfile.c_str());

  // Parse proto with --oneof-union option.
  opts.proto_oneof_union = true;
  flatbuffers::Parser parser3(opts);
  TEST_EQ(parser3.Parse(protofile.c_str(), include_directories), true);

  // Generate fbs.
  auto fbs_union = flatbuffers::GenerateFBS(parser3, "test");

  // Ensure generated file is parsable.
  flatbuffers::Parser parser4;
  TEST_EQ(parser4.Parse(fbs_union.c_str(), nullptr), true);
  TEST_EQ_STR(fbs_union.c_str(), goldenunionfile.c_str());
}

// Parse a .proto schema, output as .fbs
void ParseProtoTestWithIncludes(const std::string &tests_data_path) {
  // load the .proto and the golden file from disk
  std::string protofile;
  std::string goldenfile;
  std::string goldenunionfile;
  std::string importprotofile;
  TEST_EQ(
      flatbuffers::LoadFile((tests_data_path + "prototest/test.proto").c_str(),
                            false, &protofile),
      true);
  TEST_EQ(flatbuffers::LoadFile(
              (tests_data_path + "prototest/imported.proto").c_str(), false,
              &importprotofile),
          true);
  TEST_EQ(flatbuffers::LoadFile(
              (tests_data_path + "prototest/test_include.golden").c_str(),
              false, &goldenfile),
          true);
  TEST_EQ(flatbuffers::LoadFile(
              (tests_data_path + "prototest/test_union_include.golden").c_str(),
              false, &goldenunionfile),
          true);

  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = true;
  opts.proto_mode = true;

  // Parse proto.
  flatbuffers::Parser parser(opts);
  auto protopath = tests_data_path + "prototest/";
  const char *include_directories[] = { protopath.c_str(), nullptr };
  TEST_EQ(parser.Parse(protofile.c_str(), include_directories), true);

  // Generate fbs.
  auto fbs = flatbuffers::GenerateFBS(parser, "test");

  // Generate fbs from import.proto
  flatbuffers::Parser import_parser(opts);
  TEST_EQ(import_parser.Parse(importprotofile.c_str(), include_directories),
          true);
  auto import_fbs = flatbuffers::GenerateFBS(import_parser, "test");

  // Ensure generated file is parsable.
  flatbuffers::Parser parser2;
  // Since `imported.fbs` isn't in the filesystem AbsolutePath can't figure it
  // out by itself. We manually construct it so Parser works.
  std::string imported_fbs = flatbuffers::PosixPath(
      flatbuffers::AbsolutePath(protopath) + "/imported.fbs");
  TEST_EQ(parser2.Parse(import_fbs.c_str(), include_directories,
                        imported_fbs.c_str()),
          true);
  TEST_EQ(parser2.Parse(fbs.c_str(), nullptr), true);
  TEST_EQ_STR(fbs.c_str(), goldenfile.c_str());

  // Parse proto with --oneof-union option.
  opts.proto_oneof_union = true;
  flatbuffers::Parser parser3(opts);
  TEST_EQ(parser3.Parse(protofile.c_str(), include_directories), true);

  // Generate fbs.
  auto fbs_union = flatbuffers::GenerateFBS(parser3, "test");

  // Ensure generated file is parsable.
  flatbuffers::Parser parser4;
  TEST_EQ(parser4.Parse(import_fbs.c_str(), nullptr, imported_fbs.c_str()),
          true);
  TEST_EQ(parser4.Parse(fbs_union.c_str(), nullptr), true);
  TEST_EQ_STR(fbs_union.c_str(), goldenunionfile.c_str());
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

}  // namespace tests
}  // namespace flatbuffers
