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

// This struct allows us to create a table of all possible output generators
// for the various programming languages and formats we support.
struct Generator {
  bool (*generate)(const flatbuffers::Parser &parser,
                   const std::string &path,
                   const std::string &file_name,
                   const flatbuffers::GeneratorOptions &opts);
  const char *generator_opt;
  const char *lang_name;
  flatbuffers::GeneratorOptions::Language lang;
  const char *generator_help;

  std::string (*make_rule)(const flatbuffers::Parser &parser,
                           const std::string &path,
                           const std::string &file_name,
                           const flatbuffers::GeneratorOptions &opts);
};

const Generator generators[] = {
  { flatbuffers::GenerateBinary,   "-b", "binary",
    flatbuffers::GeneratorOptions::kMAX,
    "Generate wire format binaries for any data definitions",
    flatbuffers::BinaryMakeRule },
  { flatbuffers::GenerateTextFile, "-t", "text",
    flatbuffers::GeneratorOptions::kMAX,
    "Generate text output for any data definitions",
    flatbuffers::TextMakeRule },
  { flatbuffers::GenerateCPP,      "-c", "C++",
    flatbuffers::GeneratorOptions::kMAX,
    "Generate C++ headers for tables/structs",
    flatbuffers::CPPMakeRule },
  { flatbuffers::GenerateGo,       "-g", "Go",
    flatbuffers::GeneratorOptions::kGo,
    "Generate Go files for tables/structs",
    flatbuffers::GeneralMakeRule },
  { flatbuffers::GenerateGeneral,  "-j", "Java",
    flatbuffers::GeneratorOptions::kJava,
    "Generate Java classes for tables/structs",
    flatbuffers::GeneralMakeRule },
  { flatbuffers::GenerateGeneral,  "-n", "C#",
    flatbuffers::GeneratorOptions::kCSharp,
    "Generate C# classes for tables/structs",
    flatbuffers::GeneralMakeRule },
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
      printf("  %s              %s.\n",
             generators[i].generator_opt,
             generators[i].generator_help);
    printf(
      "  -o PATH         Prefix PATH to all generated files.\n"
      "  -I PATH         Search for includes in the specified path.\n"
      "  -M              Print make rules for generated files.\n"
      "  --strict-json   Strict JSON: field names must be / will be quoted,\n"
      "                  no trailing commas in tables/vectors.\n"
      "  --no-prefix     Don\'t prefix enum values with the enum type in C++.\n"
      "  --gen-includes  Generate include statements for included schemas the\n"
      "                  generated file depends on (C++).\n"
      "  --gen-mutable   Generate accessors that can mutate buffers in-place.\n"
      "  --proto         Input is a .proto, translate to .fbs.\n"
      "FILEs may depend on declarations in earlier files.\n"
      "FILEs after the -- must be binary flatbuffer format files.\n"
      "Output files are named using the base file name of the input,\n"
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
  bool print_make_rules = false;
  bool proto_mode = false;
  std::vector<std::string> filenames;
  std::vector<const char *> include_directories;
  size_t binary_files_from = std::numeric_limits<size_t>::max();
  for (int argi = 1; argi < argc; argi++) {
    const char *arg = argv[argi];
    if (arg[0] == '-') {
      if (filenames.size() && arg[1] != '-')
        Error("invalid option location", arg, true);
      std::string opt = arg;
      if (opt == "-o") {
        if (++argi >= argc) Error("missing path following", arg, true);
        output_path = flatbuffers::ConCatPathFileName(argv[argi], "");
      } else if(opt == "-I") {
        if (++argi >= argc) Error("missing path following", arg, true);
        include_directories.push_back(argv[argi]);
      } else if(opt == "--strict-json") {
        opts.strict_json = true;
      } else if(opt == "--no-prefix") {
        opts.prefixed_enums = false;
      } else if(opt == "--gen-mutable") {
        opts.mutable_buffer = true;
      } else if(opt == "--gen-includes") {
        opts.include_dependence_headers = true;
      } else if(opt == "--") {  // Separator between text and binary inputs.
        binary_files_from = filenames.size();
      } else if(opt == "--proto") {
        proto_mode = true;
        any_generator = true;
      } else if(opt == "-M") {
        print_make_rules = true;
      } else {
        for (size_t i = 0; i < num_generators; ++i) {
          if (opt == generators[i].generator_opt) {
            generator_enabled[i] = true;
            any_generator = true;
            goto found;
          }
        }
        Error("unknown commandline argument", arg, true);
        found:;
      }
    } else {
      filenames.push_back(argv[argi]);
    }
  }

  if (!filenames.size()) Error("missing input files", nullptr, true);

  if (!any_generator)
    Error("no options", "specify one of -c -g -j -t -b etc.", true);

  // Now process the files:
  flatbuffers::Parser parser(opts.strict_json, proto_mode);
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
        opts.lang = generators[i].lang;
        if (generator_enabled[i]) {
          if (!print_make_rules) {
            flatbuffers::EnsureDirExists(output_path);
            if (!generators[i].generate(parser, output_path, filebase, opts)) {
              Error((std::string("Unable to generate ") +
                     generators[i].lang_name +
                     " for " +
                     filebase).c_str());
            }
          } else {
            std::string make_rule = generators[i].make_rule(
                parser, output_path, *file_it, opts);
            if (!make_rule.empty())
              printf("%s\n", flatbuffers::WordWrap(
                  make_rule, 80, " ", " \\").c_str());
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
