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
#include <limits>

#define FLATC_VERSION "1.3.0 (" __DATE__ ")"

static void Error(const std::string &err, bool usage = false,
                  bool show_exe_name = true);

// This struct allows us to create a table of all possible output generators
// for the various programming languages and formats we support.
struct Generator {
  bool (*generate)(const flatbuffers::Parser &parser,
                   const std::string &path,
                   const std::string &file_name);
  const char *generator_opt_short;
  const char *generator_opt_long;
  const char *lang_name;
  flatbuffers::IDLOptions::Language lang;
  const char *generator_help;

  std::string (*make_rule)(const flatbuffers::Parser &parser,
                           const std::string &path,
                           const std::string &file_name);
};

const Generator generators[] = {
  { flatbuffers::GenerateBinary,   "-b", "--binary", "binary",
    flatbuffers::IDLOptions::kMAX,
    "Generate wire format binaries for any data definitions",
    flatbuffers::BinaryMakeRule },
  { flatbuffers::GenerateTextFile, "-t", "--json", "text",
    flatbuffers::IDLOptions::kMAX,
    "Generate text output for any data definitions",
    flatbuffers::TextMakeRule },
  { flatbuffers::GenerateCPP,      "-c", "--cpp", "C++",
    flatbuffers::IDLOptions::kMAX,
    "Generate C++ headers for tables/structs",
    flatbuffers::CPPMakeRule },
  { flatbuffers::GenerateGo,       "-g", "--go", "Go",
    flatbuffers::IDLOptions::kGo,
    "Generate Go files for tables/structs",
    flatbuffers::GeneralMakeRule },
  { flatbuffers::GenerateGeneral,  "-j", "--java", "Java",
    flatbuffers::IDLOptions::kJava,
    "Generate Java classes for tables/structs",
    flatbuffers::GeneralMakeRule },
  { flatbuffers::GenerateJS,       "-s", "--js", "JavaScript",
    flatbuffers::IDLOptions::kMAX,
    "Generate JavaScript code for tables/structs",
    flatbuffers::JSMakeRule },
  { flatbuffers::GenerateGeneral,  "-n", "--csharp", "C#",
    flatbuffers::IDLOptions::kCSharp,
    "Generate C# classes for tables/structs",
    flatbuffers::GeneralMakeRule },
  { flatbuffers::GeneratePython,   "-p", "--python", "Python",
    flatbuffers::IDLOptions::kMAX,
    "Generate Python files for tables/structs",
    flatbuffers::GeneralMakeRule },
    { flatbuffers::GeneratePhp, nullptr, "--php", "PHP",
    flatbuffers::IDLOptions::kMAX,
    "Generate PHP files for tables/structs",
    flatbuffers::GeneralMakeRule },
};

const char *program_name = nullptr;
flatbuffers::Parser *parser = nullptr;

static void Error(const std::string &err, bool usage, bool show_exe_name) {
  if (show_exe_name) printf("%s: ", program_name);
  printf("%s\n", err.c_str());
  if (usage) {
    printf("usage: %s [OPTION]... FILE... [-- FILE...]\n", program_name);
    for (size_t i = 0; i < sizeof(generators) / sizeof(generators[0]); ++i)
      printf("  %-12s %s %s.\n",
             generators[i].generator_opt_long,
             generators[i].generator_opt_short
               ? generators[i].generator_opt_short
               : "  ",
             generators[i].generator_help);
    printf(
      "  -o PATH            Prefix PATH to all generated files.\n"
      "  -I PATH            Search for includes in the specified path.\n"
      "  -M                 Print make rules for generated files.\n"
      "  --version          Print the version number of flatc and exit.\n"
      "  --strict-json      Strict JSON: field names must be / will be quoted,\n"
      "                     no trailing commas in tables/vectors.\n"
      "  --defaults-json    Output fields whose value is the default when\n"
      "                     writing JSON\n"
      "  --unknown-json     Allow fields in JSON that are not defined in the\n"
      "                     schema. These fields will be discared when generating\n"
      "                     binaries.\n"
      "  --no-prefix        Don\'t prefix enum values with the enum type in C++.\n"
      "  --scoped-enums     Use C++11 style scoped and strongly typed enums.\n"
      "                     also implies --no-prefix.\n"
      "  --gen-includes     (deprecated), this is the default behavior.\n"
      "                     If the original behavior is required (no include\n"
      "                     statements) use --no-includes.\n"
      "  --no-includes      Don\'t generate include statements for included\n"
      "                     schemas the generated file depends on (C++).\n"
      "  --gen-mutable      Generate accessors that can mutate buffers in-place.\n"
      "  --gen-onefile      Generate single output file for C#\n"
      "  --gen-name-strings Generate type name functions for C++.\n"
      "  --raw-binary       Allow binaries without file_indentifier to be read.\n"
      "                     This may crash flatc given a mismatched schema.\n"
      "  --proto            Input is a .proto, translate to .fbs.\n"
      "  --schema           Serialize schemas instead of JSON (use with -b)\n"
      "FILEs may be schemas, or JSON files (conforming to preceding schema)\n"
      "FILEs after the -- must be binary flatbuffer format files.\n"
      "Output files are named using the base file name of the input,\n"
      "and written to the current directory or the path given by -o.\n"
      "example: %s -c -b schema1.fbs schema2.fbs data.json\n",
      program_name);
  }
  if (parser) delete parser;
  exit(1);
}

int main(int argc, const char *argv[]) {
  program_name = argv[0];
  flatbuffers::IDLOptions opts;
  std::string output_path;
  const size_t num_generators = sizeof(generators) / sizeof(generators[0]);
  bool generator_enabled[num_generators] = { false };
  bool any_generator = false;
  bool print_make_rules = false;
  bool raw_binary = false;
  bool schema_binary = false;
  std::vector<std::string> filenames;
  std::vector<const char *> include_directories;
  size_t binary_files_from = std::numeric_limits<size_t>::max();
  for (int argi = 1; argi < argc; argi++) {
    std::string arg = argv[argi];
    if (arg[0] == '-') {
      if (filenames.size() && arg[1] != '-')
        Error("invalid option location: " + arg, true);
      if (arg == "-o") {
        if (++argi >= argc) Error("missing path following: " + arg, true);
        output_path = flatbuffers::ConCatPathFileName(argv[argi], "");
      } else if(arg == "-I") {
        if (++argi >= argc) Error("missing path following" + arg, true);
        include_directories.push_back(argv[argi]);
      } else if(arg == "--strict-json") {
        opts.strict_json = true;
      } else if(arg == "--no-js-exports") {
        opts.skip_js_exports = true;
      } else if(arg == "--defaults-json") {
        opts.output_default_scalars_in_json = true;
      } else if (arg == "--unknown-json") {
        opts.skip_unexpected_fields_in_json = true;
      } else if(arg == "--no-prefix") {
        opts.prefixed_enums = false;
      } else if(arg == "--scoped-enums") {
        opts.prefixed_enums = false;
        opts.scoped_enums = true;
      } else if(arg == "--gen-mutable") {
        opts.mutable_buffer = true;
      } else if(arg == "--gen-name-strings") {
        opts.generate_name_strings = true;
      } else if(arg == "--gen-all") {
        opts.generate_all = true;
        opts.include_dependence_headers = false;
      } else if(arg == "--gen-includes") {
        // Deprecated, remove this option some time in the future.
        printf("warning: --gen-includes is deprecated (it is now default)\n");
      } else if(arg == "--no-includes") {
        opts.include_dependence_headers = false;
      } else if (arg == "--gen-onefile") {
        opts.one_file = true;
      } else if (arg == "--raw-binary") {
        raw_binary = true;
      } else if(arg == "--") {  // Separator between text and binary inputs.
        binary_files_from = filenames.size();
      } else if(arg == "--proto") {
        opts.proto_mode = true;
      } else if(arg == "--schema") {
        schema_binary = true;
      } else if(arg == "-M") {
        print_make_rules = true;
      } else if(arg == "--version") {
        printf("flatc version %s\n", FLATC_VERSION);
        exit(0);
      } else {
        for (size_t i = 0; i < num_generators; ++i) {
          if (arg == generators[i].generator_opt_long ||
              (generators[i].generator_opt_short &&
               arg == generators[i].generator_opt_short)) {
            generator_enabled[i] = true;
            any_generator = true;
            goto found;
          }
        }
        Error("unknown commandline argument" + arg, true);
        found:;
      }
    } else {
      filenames.push_back(argv[argi]);
    }
  }

  if (!filenames.size()) Error("missing input files", false, true);

  if (opts.proto_mode) {
    if (any_generator)
      Error("cannot generate code directly from .proto files", true);
  } else if (!any_generator) {
    Error("no options: specify at least one generator.", true);
  }

  // Now process the files:
  parser = new flatbuffers::Parser(opts);
  for (auto file_it = filenames.begin();
            file_it != filenames.end();
          ++file_it) {
      std::string contents;
      if (!flatbuffers::LoadFile(file_it->c_str(), true, &contents))
        Error("unable to load file: " + *file_it);

      bool is_binary = static_cast<size_t>(file_it - filenames.begin()) >=
                       binary_files_from;
      if (is_binary) {
        parser->builder_.Clear();
        parser->builder_.PushFlatBuffer(
          reinterpret_cast<const uint8_t *>(contents.c_str()),
          contents.length());
        if (!raw_binary) {
          // Generally reading binaries that do not correspond to the schema
          // will crash, and sadly there's no way around that when the binary
          // does not contain a file identifier.
          // We'd expect that typically any binary used as a file would have
          // such an identifier, so by default we require them to match.
          if (!parser->file_identifier_.length()) {
            Error("current schema has no file_identifier: cannot test if \"" +
                 *file_it +
                 "\" matches the schema, use --raw-binary to read this file"
                 " anyway.");
          } else if (!flatbuffers::BufferHasIdentifier(contents.c_str(),
                                             parser->file_identifier_.c_str())) {
            Error("binary \"" +
                 *file_it +
                 "\" does not have expected file_identifier \"" +
                 parser->file_identifier_ +
                 "\", use --raw-binary to read this file anyway.");
          }
        }
      } else {
        // Check if file contains 0 bytes.
        if (contents.length() != strlen(contents.c_str())) {
          Error("input file appears to be binary: " + *file_it, true);
        }
        if (flatbuffers::GetExtension(*file_it) == "fbs") {
          // If we're processing multiple schemas, make sure to start each
          // one from scratch. If it depends on previous schemas it must do
          // so explicitly using an include.
          delete parser;
          parser = new flatbuffers::Parser(opts);
        }
        auto local_include_directory = flatbuffers::StripFileName(*file_it);
        include_directories.push_back(local_include_directory.c_str());
        include_directories.push_back(nullptr);
        if (!parser->Parse(contents.c_str(), &include_directories[0],
                          file_it->c_str()))
          Error(parser->error_, false, false);
        if (schema_binary) {
          parser->Serialize();
          parser->file_extension_ = reflection::SchemaExtension();
        }
        include_directories.pop_back();
        include_directories.pop_back();
      }

      std::string filebase = flatbuffers::StripPath(
                               flatbuffers::StripExtension(*file_it));

      for (size_t i = 0; i < num_generators; ++i) {
        parser->opts.lang = generators[i].lang;
        if (generator_enabled[i]) {
          if (!print_make_rules) {
            flatbuffers::EnsureDirExists(output_path);
            if (!generators[i].generate(*parser, output_path, filebase)) {
              Error(std::string("Unable to generate ") +
                    generators[i].lang_name +
                    " for " +
                    filebase);
            }
          } else {
            std::string make_rule = generators[i].make_rule(
                *parser, output_path, *file_it);
            if (!make_rule.empty())
              printf("%s\n", flatbuffers::WordWrap(
                  make_rule, 80, " ", " \\").c_str());
          }
        }
      }

      if (opts.proto_mode) GenerateFBS(*parser, output_path, filebase);

      // We do not want to generate code for the definitions in this file
      // in any files coming up next.
      parser->MarkGenerated();
  }

  delete parser;
  return 0;
}
