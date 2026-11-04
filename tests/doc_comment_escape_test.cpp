#include "doc_comment_escape_test.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include "idl_gen_java.h"
#include "idl_gen_kotlin.h"
#include "idl_gen_ts.h"
#include "test_assert.h"

namespace flatbuffers {
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

// The three JVM backends emit doc comments inside block comments; the
// TypeScript generator is the reference implementation of the escape (and is
// checked here to keep it locked).
std::vector<LanguageGenerator> BlockCommentGenerators() {
  std::vector<LanguageGenerator> generators;
  LanguageGenerator entries[] = {
      { "java", &NewJavaCodeGenerator },
      { "kotlin", &NewKotlinCodeGenerator },
      { "kotlin-kmp", &NewKotlinKMPCodeGenerator },
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

}  // namespace

void DocCommentEscapeTest() {
  // Doc text containing the block-comment terminator must not be able to end
  // the generated doc-comment block; the terminator is neutralized (mirroring
  // the TypeScript generator) and the doc text stays inside the comment.
  const char *schema =
      "/// class docs before */ break after\n"
      "table Root {\n"
      "  /// field docs before */ break after\n"
      "  x:int;\n"
      "  /// clean docs pass through\n"
      "  clean:string;\n"
      "}\n"
      "/// enum docs before */ break after\n"
      "enum E:byte { A, B }\n"
      "root_type Root;\n"
      "file_identifier \"WITN\";\n";

  // The terminator as it must appear inside a generated comment, with the
  // slash escaped so the sequence cannot close the comment.
  const std::string neutralized =
      std::string("docs before *") + "\\/" + " break after";
  // The raw form closes a block comment early.
  const std::string raw = std::string("docs before *") + "/" + " break after";

  std::vector<LanguageGenerator> generators = BlockCommentGenerators();
  for (size_t i = 0; i < generators.size(); ++i) {
    std::unique_ptr<CodeGenerator> generator = generators[i].make();
    const std::string code = GenerateAll(*generator, schema);
    TEST_OUTPUT_LINE("doc-comment escape: %s", generators[i].name);
    // Every doc line is present, with the terminator neutralized.
    TEST_ASSERT(code.find("class " + neutralized) != std::string::npos);
    TEST_ASSERT(code.find("field " + neutralized) != std::string::npos);
    TEST_ASSERT(code.find("enum " + neutralized) != std::string::npos);
    // No raw terminator survives in doc text (it would end the comment).
    TEST_ASSERT(code.find("class " + raw) == std::string::npos);
    TEST_ASSERT(code.find("field " + raw) == std::string::npos);
    TEST_ASSERT(code.find("enum " + raw) == std::string::npos);
    // Doc text without the terminator passes through unchanged.
    TEST_ASSERT(code.find("clean docs pass through") != std::string::npos);
  }
}

}  // namespace tests
}  // namespace flatbuffers
