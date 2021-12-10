/*
 * Copyright 2017 Google Inc. All rights reserved.
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

#include <memory>

#include "bfbs_gen_lua.h"
#include "flatbuffers/base.h"
#include "flatbuffers/flatc.h"
#include "flatbuffers/util.h"

static const char *g_program_name = nullptr;

static void Warn(const flatbuffers::FlatCompiler *flatc,
                 const std::string &warn, bool show_exe_name) {
  (void)flatc;
  if (show_exe_name) { printf("%s: ", g_program_name); }
  fprintf(stderr, "warning:\n  %s\n\n", warn.c_str());
}

static void Error(const flatbuffers::FlatCompiler *flatc,
                  const std::string &err, bool usage, bool show_exe_name) {
  if (show_exe_name) { printf("%s: ", g_program_name); }
  if (usage && flatc) {
    fprintf(stderr, "%s\n", flatc->GetUsageString(g_program_name).c_str());
  }
  fprintf(stderr, "error:\n  %s\n\n", err.c_str());
  exit(1);
}

namespace flatbuffers {
void LogCompilerWarn(const std::string &warn) {
  Warn(static_cast<const flatbuffers::FlatCompiler *>(nullptr), warn, true);
}
void LogCompilerError(const std::string &err) {
  Error(static_cast<const flatbuffers::FlatCompiler *>(nullptr), err, false,
        true);
}
}  // namespace flatbuffers

int main(int argc, const char *argv[]) {
  // Prevent Appveyor-CI hangs.
  flatbuffers::SetupDefaultCRTReportMode();

  const std::string flatbuffers_version(flatbuffers::FLATBUFFERS_VERSION());

  std::unique_ptr<flatbuffers::BfbsGenerator> bfbs_gen_lua =
      flatbuffers::NewLuaBfbsGenerator(flatbuffers_version);

  g_program_name = argv[0];

  const flatbuffers::FlatCompiler::Generator generators[] = {
    { flatbuffers::GenerateBinary, "binary", false, nullptr,
      flatbuffers::IDLOptions::kBinary,
      flatbuffers::FlatCOption{
          "b", "binary", "",
          "Generate wire format binaries for any data definitions" },
      flatbuffers::BinaryMakeRule, nullptr },
    { flatbuffers::GenerateTextFile, "text", false, nullptr,
      flatbuffers::IDLOptions::kJson,
      flatbuffers::FlatCOption{
          "t", "json", "", "Generate text output for any data definitions" },

      flatbuffers::TextMakeRule, nullptr },
    { flatbuffers::GenerateCPP, "C++", true, flatbuffers::GenerateCppGRPC,
      flatbuffers::IDLOptions::kCpp,
      flatbuffers::FlatCOption{ "c", "cpp", "",
                                "Generate C++ headers for tables/structs" },
      flatbuffers::CPPMakeRule, nullptr },
    { flatbuffers::GenerateGo, "Go", true, flatbuffers::GenerateGoGRPC,
      flatbuffers::IDLOptions::kGo,
      flatbuffers::FlatCOption{ "g", "go", "",
                                "Generate Go files for tables/structs" },
      nullptr, nullptr },
    { flatbuffers::GenerateJava, "Java", true, flatbuffers::GenerateJavaGRPC,
      flatbuffers::IDLOptions::kJava,
      flatbuffers::FlatCOption{ "j", "java", "",
                                "Generate Java classes for tables/structs" },
      flatbuffers::JavaMakeRule, nullptr },
    { flatbuffers::GenerateDart, "Dart", true, nullptr,
      flatbuffers::IDLOptions::kDart,
      flatbuffers::FlatCOption{ "d", "dart", "",
                                "Generate Dart classes for tables/structs" },
      flatbuffers::DartMakeRule, nullptr },
    { flatbuffers::GenerateTS, "TypeScript", true, flatbuffers::GenerateTSGRPC,
      flatbuffers::IDLOptions::kTs,
      flatbuffers::FlatCOption{ "T", "ts", "",
                                "Generate TypeScript code for tables/structs" },
      flatbuffers::TSMakeRule, nullptr },
    { flatbuffers::GenerateCSharp, "C#", true, nullptr,
      flatbuffers::IDLOptions::kCSharp,
      flatbuffers::FlatCOption{ "n", "csharp", "",
                                "Generate C# classes for tables/structs" },
      flatbuffers::CSharpMakeRule, nullptr },
    { flatbuffers::GeneratePython, "Python", true,
      flatbuffers::GeneratePythonGRPC, flatbuffers::IDLOptions::kPython,
      flatbuffers::FlatCOption{ "p", "python", "",
                                "Generate Python files for tables/structs" },
      nullptr, nullptr },
    { flatbuffers::GenerateLobster, "Lobster", true, nullptr,
      flatbuffers::IDLOptions::kLobster,
      flatbuffers::FlatCOption{ "", "lobster", "",
                                "Generate Lobster files for tables/structs" },
      nullptr, nullptr },
    { flatbuffers::GenerateLua, "Lua", true, nullptr,
      flatbuffers::IDLOptions::kLua,
      flatbuffers::FlatCOption{ "l", "lua", "",
                                "Generate Lua files for tables/structs" },
      nullptr, bfbs_gen_lua.get() },
    { flatbuffers::GenerateRust, "Rust", true, nullptr,
      flatbuffers::IDLOptions::kRust,
      flatbuffers::FlatCOption{ "r", "rust", "",
                                "Generate Rust files for tables/structs" },
      flatbuffers::RustMakeRule, nullptr },
    { flatbuffers::GeneratePhp, "PHP", true, nullptr,
      flatbuffers::IDLOptions::kPhp,
      flatbuffers::FlatCOption{ "", "php", "",
                                "Generate PHP files for tables/structs" },
      nullptr, nullptr },
    { flatbuffers::GenerateKotlin, "Kotlin", true, nullptr,
      flatbuffers::IDLOptions::kKotlin,
      flatbuffers::FlatCOption{ "", "kotlin", "",
                                "Generate Kotlin classes for tables/structs" },
      nullptr, nullptr },
    { flatbuffers::GenerateJsonSchema, "JsonSchema", true, nullptr,
      flatbuffers::IDLOptions::kJsonSchema,
      flatbuffers::FlatCOption{ "", "jsonschema", "", "Generate Json schema" },
      nullptr, nullptr },
    { flatbuffers::GenerateSwift, "swift", true, flatbuffers::GenerateSwiftGRPC,
      flatbuffers::IDLOptions::kSwift,
      flatbuffers::FlatCOption{ "", "swift", "",
                                "Generate Swift files for tables/structs" },
      nullptr, nullptr },
  };

  flatbuffers::FlatCompiler::InitParams params;
  params.generators = generators;
  params.num_generators = sizeof(generators) / sizeof(generators[0]);
  params.warn_fn = Warn;
  params.error_fn = Error;

  flatbuffers::FlatCompiler flatc(params);
  return flatc.Compile(argc, argv);
}
