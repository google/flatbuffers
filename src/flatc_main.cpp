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

#include "flatbuffers/flatc.h"

static const char *g_program_name = nullptr;

static void Warn(const flatbuffers::FlatCompiler *flatc,
                 const std::string &warn,
                 bool show_exe_name) {
  (void)flatc;
  if (show_exe_name) {
    printf("%s: ", g_program_name);
  }
  printf("warning: %s\n", warn.c_str());
}

static void Error(const flatbuffers::FlatCompiler *flatc,
                  const std::string &err,
                  bool usage,
                  bool show_exe_name) {
  if (show_exe_name) {
    printf("%s: ", g_program_name);
  }
  printf("error: %s\n", err.c_str());
  if (usage) {
    printf("%s", flatc->GetUsageString(g_program_name).c_str());
  }
  exit(1);
}

int main(int argc, const char *argv[]) {
  g_program_name = argv[0];

  const flatbuffers::FlatCompiler::Generator generators[] = {
    { flatbuffers::GenerateBinary,   "-b", "--binary", "binary",
      nullptr,
      flatbuffers::IDLOptions::kBinary,
      "Generate wire format binaries for any data definitions",
      flatbuffers::BinaryMakeRule },
    { flatbuffers::GenerateTextFile, "-t", "--json", "text",
      nullptr,
      flatbuffers::IDLOptions::kJson,
      "Generate text output for any data definitions",
      flatbuffers::TextMakeRule },
    { flatbuffers::GenerateCPP,      "-c", "--cpp", "C++",
      flatbuffers::GenerateCppGRPC,
      flatbuffers::IDLOptions::kCpp,
      "Generate C++ headers for tables/structs",
      flatbuffers::CPPMakeRule },
    { flatbuffers::GenerateGo,       "-g", "--go", "Go",
      flatbuffers::GenerateGoGRPC,
      flatbuffers::IDLOptions::kGo,
      "Generate Go files for tables/structs",
      flatbuffers::GeneralMakeRule },
    { flatbuffers::GenerateGeneral,  "-j", "--java", "Java",
      nullptr,
      flatbuffers::IDLOptions::kJava,
      "Generate Java classes for tables/structs",
      flatbuffers::GeneralMakeRule },
    { flatbuffers::GenerateJS,       "-s", "--js", "JavaScript",
      nullptr,
      flatbuffers::IDLOptions::kJs,
      "Generate JavaScript code for tables/structs",
      flatbuffers::JSMakeRule },
    { flatbuffers::GenerateGeneral,  "-n", "--csharp", "C#",
      nullptr,
      flatbuffers::IDLOptions::kCSharp,
      "Generate C# classes for tables/structs",
      flatbuffers::GeneralMakeRule },
    { flatbuffers::GeneratePython,   "-p", "--python", "Python",
      nullptr,
      flatbuffers::IDLOptions::kPython,
      "Generate Python files for tables/structs",
      flatbuffers::GeneralMakeRule },
    { flatbuffers::GeneratePhp, nullptr, "--php", "PHP",
      nullptr,
      flatbuffers::IDLOptions::kPhp,
      "Generate PHP files for tables/structs",
      flatbuffers::GeneralMakeRule },
    };

  flatbuffers::FlatCompiler::InitParams params;
  params.generators = generators;
  params.num_generators = sizeof(generators) / sizeof(generators[0]);
  params.warn_fn = Warn;
  params.error_fn = Error;

  flatbuffers::FlatCompiler flatc(params);
  return flatc.Compile(argc - 1, argv + 1);
}
