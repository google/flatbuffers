/*
 * Copyright 2014 Google Inc. All rights reserved.
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

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

static void Error(const char *err, const char *obj = nullptr,
                  bool usage = false, bool show_exe_name = true);

namespace flatbuffers {

bool GenerateBinary(const Parser &parser,
                    const std::string &path,
                    const std::string &file_name,
                    const GeneratorOptions & /*opts*/) {
  auto ext = parser.file_extension_.length() ? parser.file_extension_ : "bin";
  return !parser.builder_.GetSize() ||
         flatbuffers::SaveFile(
           (path + file_name + "." + ext).c_str(),
           reinterpret_cast<char *>(parser.builder_.GetBufferPointer()),
           parser.builder_.GetSize(),
           true);
}

bool GenerateTextFile(const Parser &parser,
                      const std::string &path,
                      const std::string &file_name,
                      const GeneratorOptions &opts) {
  if (!parser.builder_.GetSize()) return true;
  if (!parser.root_struct_def) Error("root_type not set");
  std::string text;
  GenerateText(parser, parser.builder_.GetBufferPointer(), opts,
               &text);
  return flatbuffers::SaveFile((path + file_name + ".json").c_str(),
                               text,
                               false);

}

}

// This struct allows us to create a table of all possible output generators
// for the various programming languages and formats we support.
struct Generator {
  bool (*generate)(const flatbuffers::Parser &parser,
                   const std::string &path,
                   const std::string &file_name,
                   const flatbuffers::GeneratorOptions &opts);
  const char *opt;
  const char *name;
  flatbuffers::GeneratorOptions::Language lang;
  const char *help;
};

const Generator generators[] = {
  { flatbuffers::GenerateBinary,   "-b", "binary",
    flatbuffers::GeneratorOptions::kMAX,
    "Generate wire format binaries for any data definitions" },
  { flatbuffers::GenerateTextFile, "-t", "text",
    flatbuffers::GeneratorOptions::kMAX,
    "Generate text output for any data definitions" },
  { flatbuffers::GenerateCPP,      "-c", "C++",
    flatbuffers::GeneratorOptions::kMAX,
    "Generate C++ headers for tables/structs" },
  { flatbuffers::GenerateGo,       "-g", "Go",
    flatbuffers::GeneratorOptions::kMAX,
    "Generate Go files for tables/structs" },
  { flatbuffers::GenerateGeneral,  "-j", "Java",
    flatbuffers::GeneratorOptions::kJava,
    "Generate Java classes for tables/structs" },
  { flatbuffers::GenerateGeneral,  "-n", "C#",
    flatbuffers::GeneratorOptions::kCSharp,
    "Generate C# classes for tables/structs" }
};

const char *program_name = NULL;

static void Error(const char *err, const char *obj, bool usage,
                  bool show_exe_name) {
  if (show_exe_name) printf("%s: ", program_name);
  printf("%s", err);
  if (obj) printf(": %s", obj);
  printf("\n");
  if (usage) {
    printf("usage: %s [OPTION]... FILE... [-- FILE...]\n", program_name);
    for (size_t i = 0; i < sizeof(generators) / sizeof(generators[0]); ++i)
      printf("  %s             %s.\n", generators[i].opt, generators[i].help);
    printf(
      "  -o PATH         Prefix PATH to all generated files.\n"
      "  -I PATH         Search for includes in the specified path.\n"
      "  --strict-json   Strict JSON: add quotes to field names.\n"
      "  --no-prefix     Don\'t prefix enum values with the enum type in C++.\n"
      "  --gen-includes  Generate include statements for included schemas the\n"
      "                  generated file depends on (C++).\n"
      "  --proto         Input is a .proto, translate to .fbs.\n"
      "FILEs may depend on declarations in earlier files.\n"
      "FILEs after the -- must be binary flatbuffer format files.\n"
      "Output files are named using the base file name of the input,"
      "and written to the current directory or the path given by -o.\n"
      "example: %s -c -b schema1.fbs schema2.fbs data.json\n",
      program_name);
  }
  exit(1);
}

int main(int argc, const char *argv[]) {
  program_name = argv[0];
  flatbuffers::GeneratorOptions opts;
  std::string output_path;
  const size_t num_generators = sizeof(generators) / sizeof(generators[0]);
  bool generator_enabled[num_generators] = { false };
  bool any_generator = false;
  bool proto_mode = false;
  std::vector<std::string> filenames;
  std::vector<const char *> include_directories;
  size_t binary_files_from = std::numeric_limits<size_t>::max();
  for (int i = 1; i < argc; i++) {
    const char *arg = argv[i];
    if (arg[0] == '-') {
      if (filenames.size() && arg[1] != '-')
        Error("invalid option location", arg, true);
      std::string opt = arg;
      if (opt == "-o") {
        if (++i >= argc) Error("missing path following", arg, true);
        output_path = flatbuffers::ConCatPathFileName(argv[i], "");
      } else if(opt == "-I") {
        if (++i >= argc) Error("missing path following", arg, true);
        include_directories.push_back(argv[i]);
      } else if(opt == "--strict-json") {
        opts.strict_json = true;
      } else if(opt == "--no-prefix") {
        opts.prefixed_enums = false;
      } else if(opt == "--gen-includes") {
        opts.include_dependence_headers = true;
      } else if(opt == "--") {  // Separator between text and binary inputs.
        binary_files_from = filenames.size();
      } else if(opt == "--proto") {
        proto_mode = true;
        any_generator = true;
      } else {
        for (size_t i = 0; i < num_generators; ++i) {
          if(opt == generators[i].opt) {
            generator_enabled[i] = true;
            any_generator = true;
            goto found;
          }
        }
        Error("unknown commandline argument", arg, true);
        found:;
      }
    } else {
      filenames.push_back(argv[i]);
    }
  }

  if (!filenames.size()) Error("missing input files", nullptr, true);

  if (!any_generator)
    Error("no options: no output files generated.",
          "specify one of -c -g -j -t -b etc.", true);

  // Now process the files:
  flatbuffers::Parser parser(proto_mode);
  for (auto file_it = filenames.begin();
            file_it != filenames.end();
          ++file_it) {
      std::string contents;
      if (!flatbuffers::LoadFile(file_it->c_str(), true, &contents))
        Error("unable to load file", file_it->c_str());

      bool is_binary = static_cast<size_t>(file_it - filenames.begin()) >=
                       binary_files_from;
      if (is_binary) {
        parser.builder_.Clear();
        parser.builder_.PushBytes(
          reinterpret_cast<const uint8_t *>(contents.c_str()),
          contents.length());
      } else {
        auto local_include_directory = flatbuffers::StripFileName(*file_it);
        include_directories.push_back(local_include_directory.c_str());
        include_directories.push_back(nullptr);
        if (!parser.Parse(contents.c_str(), &include_directories[0],
                          file_it->c_str()))
          Error(parser.error_.c_str(), nullptr, false, false);
        include_directories.pop_back();
        include_directories.pop_back();
      }

      std::string filebase = flatbuffers::StripPath(
                               flatbuffers::StripExtension(*file_it));

      for (size_t i = 0; i < num_generators; ++i) {
        if (generator_enabled[i]) {
          flatbuffers::EnsureDirExists(output_path);
          opts.lang = generators[i].lang;
          if (!generators[i].generate(parser, output_path, filebase, opts)) {
            Error((std::string("Unable to generate ") +
                   generators[i].name +
                   " for " +
                   filebase).c_str());
          }
        }
      }

      if (proto_mode) GenerateFBS(parser, output_path, filebase, opts);

      // We do not want to generate code for the definitions in this file
      // in any files coming up next.
      parser.MarkGenerated();
  }

  return 0;
}
