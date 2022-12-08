#include "proto_test.h"

#include "test_assert.h"

namespace flatbuffers {
namespace tests {

void RunTest(const flatbuffers::IDLOptions &opts, const std::string &protopath,
             const std::string &protofile, const std::string &goldenfile,
             const std::string importprotofile) {
  const char *include_directories[] = { protopath.c_str(), nullptr };

  // Parse proto.
  flatbuffers::Parser parser(opts);
  TEST_EQ(parser.Parse(protofile.c_str(), include_directories), true);
  // Generate fbs.
  auto fbs = flatbuffers::GenerateFBS(parser, "test");

  // Ensure generated file is parsable.
  flatbuffers::Parser parser2;

  if (!importprotofile.empty()) {
    // Generate fbs from import.proto
    flatbuffers::Parser import_parser(opts);
    TEST_EQ(import_parser.Parse(importprotofile.c_str(), include_directories),
            true);
    auto import_fbs = flatbuffers::GenerateFBS(import_parser, "test");
    // Since `imported.fbs` isn't in the filesystem AbsolutePath can't figure it
    // out by itself. We manually construct it so Parser works.
    std::string imported_fbs = flatbuffers::PosixPath(
        flatbuffers::AbsolutePath(protopath) + "/imported.fbs");
    TEST_EQ(parser2.Parse(import_fbs.c_str(), include_directories,
                          imported_fbs.c_str()),
            true);
  }

  TEST_EQ(parser2.Parse(fbs.c_str(), nullptr), true);
  TEST_EQ_STR(fbs.c_str(), goldenfile.c_str());
}

void proto_test(std::string protopath, const std::string &protofile) {
  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = false;
  opts.proto_mode = true;

  // load the .proto and the golden file from disk
  std::string goldenfile;
  TEST_EQ(flatbuffers::LoadFile((protopath + "test.golden").c_str(), false,
                                &goldenfile),
          true);

  RunTest(opts, protopath, protofile, goldenfile);
}

void proto_test_id(std::string protopath, const std::string &protofile) {
  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = false;
  opts.proto_mode = true;
  opts.keep_proto_id = true;

  // load the .proto and the golden file from disk
  std::string goldenfile;
  TEST_EQ(flatbuffers::LoadFile((protopath + "test_id.golden").c_str(), false,
                                &goldenfile),
          true);

  RunTest(opts, protopath, protofile, goldenfile);
}

void proto_test_union(std::string protopath, const std::string &protofile) {
  // Parse proto with --oneof-union option.
  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = false;
  opts.proto_mode = true;
  opts.proto_oneof_union = true;

  std::string goldenfile;
  TEST_EQ(flatbuffers::LoadFile((protopath + "test_union.golden").c_str(),
                                false, &goldenfile),
          true);
  RunTest(opts, protopath, protofile, goldenfile);
}

void proto_test_union_id(std::string protopath, const std::string &protofile) {
  // Parse proto with --oneof-union option.
  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = false;
  opts.proto_mode = true;
  opts.proto_oneof_union = true;
  opts.keep_proto_id = true;

  std::string goldenfile;
  TEST_EQ(flatbuffers::LoadFile((protopath + "test_union_id.golden").c_str(),
                                false, &goldenfile),
          true);
  RunTest(opts, protopath, protofile, goldenfile);
}

void proto_test_union_suffix(std::string protopath,
                             const std::string &protofile) {
  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = false;
  opts.proto_mode = true;
  opts.proto_namespace_suffix = "test_namespace_suffix";
  opts.proto_oneof_union = true;

  std::string goldenfile;
  TEST_EQ(
      flatbuffers::LoadFile((protopath + "test_union_suffix.golden").c_str(),
                            false, &goldenfile),
      true);
  RunTest(opts, protopath, protofile, goldenfile);
}

void proto_test_union_suffix_id(std::string protopath,
                                const std::string &protofile) {
  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = false;
  opts.proto_mode = true;
  opts.proto_namespace_suffix = "test_namespace_suffix";
  opts.proto_oneof_union = true;
  opts.keep_proto_id = true;

  std::string goldenfile;
  TEST_EQ(
      flatbuffers::LoadFile((protopath + "test_union_suffix_id.golden").c_str(),
                            false, &goldenfile),
      true);
  RunTest(opts, protopath, protofile, goldenfile);
}

void proto_test_include(std::string protopath, const std::string &protofile,
                        const std::string &importprotofile) {
  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = true;
  opts.proto_mode = true;

  std::string goldenfile;
  TEST_EQ(flatbuffers::LoadFile((protopath + "test_include.golden").c_str(),
                                false, &goldenfile),
          true);

  RunTest(opts, protopath, protofile, goldenfile, importprotofile);
}

void proto_test_include_id(std::string protopath, const std::string &protofile,
                           const std::string &importprotofile) {
  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = true;
  opts.proto_mode = true;
  opts.keep_proto_id = true;

  std::string goldenfile;
  TEST_EQ(flatbuffers::LoadFile((protopath + "test_include_id.golden").c_str(),
                                false, &goldenfile),
          true);

  RunTest(opts, protopath, protofile, goldenfile, importprotofile);
}

void proto_test_include_union(std::string protopath, std::string protofile,
                              std::string importprotofile) {
  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = true;
  opts.proto_mode = true;
  opts.proto_oneof_union = true;

  std::string goldenfile;
  TEST_EQ(
      flatbuffers::LoadFile((protopath + "test_union_include.golden").c_str(),
                            false, &goldenfile),
      true);

  RunTest(opts, protopath, protofile, goldenfile, importprotofile);
}

void proto_test_include_union_id(std::string protopath, std::string protofile,
                                 std::string importprotofile) {
  flatbuffers::IDLOptions opts;
  opts.include_dependence_headers = true;
  opts.proto_mode = true;
  opts.proto_oneof_union = true;
  opts.keep_proto_id = true;

  std::string goldenfile;
  TEST_EQ(flatbuffers::LoadFile(
              (protopath + "test_union_include_id.golden").c_str(), false,
              &goldenfile),
          true);

  RunTest(opts, protopath, protofile, goldenfile, importprotofile);
}

// Parse a .proto schema, output as .fbs
void ParseProtoTest(const std::string &tests_data_path) {
  auto protopath = tests_data_path + "prototest/";
  std::string protofile;
  TEST_EQ(
      flatbuffers::LoadFile((tests_data_path + "prototest/test.proto").c_str(),
                            false, &protofile),
      true);

  std::string importprotofile;
  TEST_EQ(flatbuffers::LoadFile(
              (tests_data_path + "prototest/imported.proto").c_str(), false,
              &importprotofile),
          true);

  proto_test(protopath, protofile);
  proto_test_union(protopath, protofile);
  proto_test_union_suffix(protopath, protofile);
  proto_test_include(protopath, protofile, importprotofile);
  proto_test_include_union(protopath, protofile, importprotofile);

  proto_test_id(protopath, protofile);
  proto_test_union_id(protopath, protofile);
  proto_test_union_suffix_id(protopath, protofile);
  proto_test_include_id(protopath, protofile, importprotofile);
  proto_test_include_union_id(protopath, protofile, importprotofile);
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
