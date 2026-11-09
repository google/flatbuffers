#include "file_identifier_escape_test.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatc.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/reflection_generated.h"
#include "flatbuffers/util.h"
#include "idl_gen_cpp.h"
#include "idl_gen_csharp.h"
#include "idl_gen_go.h"
#include "idl_gen_java.h"
#include "idl_gen_kotlin.h"
#include "idl_gen_php.h"
#include "idl_gen_rust.h"
#include "idl_gen_swift.h"
#include "idl_gen_ts.h"
#include "test_assert.h"

namespace flatbuffers {

// The C++ generator reports some conditions through this logging hook, which
// flatc_main.cpp defines for the flatc binary. The test binary logs to stderr.
void LogCompilerError(const std::string &err) {
  TEST_OUTPUT_LINE("flatc error: %s", err.c_str());
}

namespace tests {
namespace {

// In-memory FileSaver so the tests can inspect generated source without
// writing any files. (The generators only emit code through
// parser.opts.file_saver.)
class CapturingFileSaver : public FileSaver {
 public:
  bool SaveFile(const char *name, const char *buf, size_t len,
                bool binary) final {
    (void)binary;
    files_[name].assign(buf, len);
    return true;
  }

  // All emitted file contents, concatenated.
  std::string Concat() const {
    std::string all;
    for (std::map<std::string, std::string>::const_iterator it = files_.begin();
         it != files_.end(); ++it) {
      all += it->second;
    }
    return all;
  }

  size_t Count() const { return files_.size(); }

 private:
  std::map<std::string, std::string> files_;
};

struct LanguageGenerator {
  const char *name;
  std::unique_ptr<CodeGenerator> (*make)();
};

std::vector<LanguageGenerator> AllLanguageGenerators() {
  std::vector<LanguageGenerator> generators;
  LanguageGenerator entries[] = {
      { "cpp", &NewCppCodeGenerator },
      { "csharp", &NewCSharpCodeGenerator },
      { "go", &NewGoCodeGenerator },
      { "java", &NewJavaCodeGenerator },
      { "kotlin", &NewKotlinCodeGenerator },
      { "kotlin-kmp", &NewKotlinKMPCodeGenerator },
      { "php", &NewPhpCodeGenerator },
      { "rust", &NewRustCodeGenerator },
      { "swift", &NewSwiftCodeGenerator },
      { "ts", &NewTsCodeGenerator },
  };
  for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); ++i) {
    generators.push_back(entries[i]);
  }
  return generators;
}

// Generates `schema` with `generator`, returning the concatenation of every
// emitted file's content. The schema is parsed fresh each time so each
// generator sees an identical parser.
std::string GenerateAll(CodeGenerator &generator, const char *schema) {
  Parser parser;
  TEST_EQ(parser.Parse(schema), true);
  CapturingFileSaver saver;
  parser.opts.file_saver = &saver;
  const CodeGenerator::Status status = generator.GenerateCode(parser, "", "");
  TEST_EQ(static_cast<int>(status),
          static_cast<int>(CodeGenerator::Status::OK));
  TEST_ASSERT(saver.Count() > 0);
  return saver.Concat();
}

// Builds a reflection binary schema (.bfbs) with a single empty root table
// "Root" and `file_ident` as the file identifier. Mirrors what flatc
// serializes for a .fbs input, except that file_ident is deliberately not
// length-checked here.
std::string BuildBfbs(const std::string &file_ident) {
  FlatBufferBuilder fbb;
  auto name = fbb.CreateString("Root");
  auto fields = fbb.CreateVector<Offset<reflection::Field>>({});
  auto root = reflection::CreateObject(fbb, name, fields);
  std::vector<Offset<reflection::Object>> objects_vec;
  objects_vec.push_back(root);
  auto objects = fbb.CreateVector(objects_vec);
  auto enums = fbb.CreateVector<Offset<reflection::Enum>>({});
  auto ident = fbb.CreateString(file_ident.data(), file_ident.size());
  auto schema = reflection::CreateSchema(fbb, objects, enums, ident, 0, root);
  fbb.Finish(schema, reflection::SchemaIdentifier());
  return std::string(reinterpret_cast<const char *>(fbb.GetBufferPointer()),
                     fbb.GetSize());
}

// Asserts that every generator emits the identifier bytes as exactly the
// canonical escaped literal `escaped`, and never in the raw form `raw_break`
// (which would terminate the literal early / inject tokens).
void CheckAllGeneratorsEscape(const char *schema, const std::string &escaped,
                              const std::string &raw_break) {
  std::vector<LanguageGenerator> generators = AllLanguageGenerators();
  for (size_t i = 0; i < generators.size(); ++i) {
    std::unique_ptr<CodeGenerator> generator = generators[i].make();
    const std::string code = GenerateAll(*generator, schema);
    TEST_OUTPUT_LINE("file_identifier escape: %s", generators[i].name);
    // The identifier text appears, correctly escaped, as a well-formed
    // literal.
    TEST_ASSERT(code.find(escaped) != std::string::npos);
    // No unescaped occurrence survives (the raw form is what breaks the
    // generated literal).
    TEST_ASSERT(code.find(raw_break) == std::string::npos);
  }
}

}  // namespace

void FileIdentifierEscapeTest() {
  // Fixture 1: the .fbs text parser restricts file_identifier to
  // kFileIdentifierLength bytes but not to a safe character set. This is the
  // 4-byte identifier ab"x (a b " x).
  {
    const char *schema =
        "table Root {\n"
        "  x:int;\n"
        "}\n"
        "root_type Root;\n"
        "file_identifier \"ab\\\"x\";\n";
    Parser parser;
    TEST_EQ(parser.Parse(schema), true);
    TEST_EQ(parser.file_identifier_, std::string("ab\"x"));
    // Canonical escaped literal: "ab\"x"
    CheckAllGeneratorsEscape(schema, "\"ab\\\"x\"",
                             std::string("\"ab\"x\""));
  }

  // Fixture 2: backslash, newline and a control byte in the same 4 bytes:
  // a \ \n \x01.
  {
    const char *schema =
        "table Root {\n"
        "  x:int;\n"
        "}\n"
        "root_type Root;\n"
        "file_identifier \"a\\\\\\n\\x01\";\n";
    Parser parser;
    TEST_EQ(parser.Parse(schema), true);
    TEST_EQ(parser.file_identifier_, std::string("a\\\n\x01"));
    // Canonical escaped literal: "a\\\n\u0001"
    CheckAllGeneratorsEscape(schema, "\"a\\\\\\n\\u0001\"",
                             std::string("a\\\n"));
  }
}

void FileIdentifierBfbsEscapeTest() {
  // The .bfbs route delivers the same bytes to the generators through
  // Parser::Deserialize() (no character-set check there either).
  const std::string bfbs = BuildBfbs("ab\"x");
  Parser parser;
  TEST_EQ(parser.Deserialize(
              reinterpret_cast<const uint8_t *>(bfbs.data()), bfbs.size()),
          true);
  TEST_EQ(parser.file_identifier_, std::string("ab\"x"));

  CapturingFileSaver saver;
  parser.opts.file_saver = &saver;
  std::unique_ptr<CodeGenerator> cpp = NewCppCodeGenerator();
  TEST_EQ(static_cast<int>(cpp->GenerateCode(parser, "", "")),
          static_cast<int>(CodeGenerator::Status::OK));
  const std::string code = saver.Concat();
  TEST_ASSERT(code.find("\"ab\\\"x\"") != std::string::npos);
  TEST_ASSERT(code.find("\"ab\"x\"") == std::string::npos);
}

void FileIdentifierBfbsLengthTest() {
  // A FlatBuffer file identifier is exactly kFileIdentifierLength (4) bytes;
  // the .fbs parser enforces that, and so must the .bfbs reader. Identifiers
  // that cannot be represented are rejected instead of being embedded.
  {
    const std::string bfbs = BuildBfbs("ab\"x");  // 4 bytes: legal.
    Parser parser;
    TEST_EQ(parser.Deserialize(
                reinterpret_cast<const uint8_t *>(bfbs.data()), bfbs.size()),
            true);
    TEST_EQ(parser.file_identifier_.length(),
            static_cast<size_t>(kFileIdentifierLength));
  }
  {
    // Over-long (16 bytes): must be rejected.
    const std::string bfbs = BuildBfbs("ab\"x\");\n//CANARY");
    Parser parser;
    TEST_EQ(parser.Deserialize(
                reinterpret_cast<const uint8_t *>(bfbs.data()), bfbs.size()),
            false);
  }
  {
    // Too short (3 bytes): must be rejected.
    const std::string bfbs = BuildBfbs("abc");
    Parser parser;
    TEST_EQ(parser.Deserialize(
                reinterpret_cast<const uint8_t *>(bfbs.data()), bfbs.size()),
            false);
  }
  {
    // Empty: legal (no identifier).
    const std::string bfbs = BuildBfbs("");
    Parser parser;
    TEST_EQ(parser.Deserialize(
                reinterpret_cast<const uint8_t *>(bfbs.data()), bfbs.size()),
            true);
    TEST_EQ(parser.file_identifier_.length(), static_cast<size_t>(0));
  }
}

}  // namespace tests
}  // namespace flatbuffers
