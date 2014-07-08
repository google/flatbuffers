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
                  bool usage = false);

namespace flatbuffers {

bool GenerateBinary(const Parser &parser,
                    const std::string &path,
                    const std::string &file_name,
                    const GeneratorOptions & /*opts*/) {
  return !parser.builder_.GetSize() ||
         flatbuffers::SaveFile(
           (path + file_name + "_wire.bin").c_str(),
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
  return flatbuffers::SaveFile((path + file_name + "_wire.txt").c_str(),
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
  const char *extension;
  const char *name;
  const char *help;
};

const Generator generators[] = {
  { flatbuffers::GenerateBinary,   "b", "binary",
    "Generate wire format binaries for any data definitions" },
  { flatbuffers::GenerateTextFile, "t", "text",
    "Generate text output for any data definitions" },
  { flatbuffers::GenerateCPP,      "c", "C++",
    "Generate C++ headers for tables/structs" },
  { flatbuffers::GenerateJava,     "j", "Java",
    "Generate Java classes for tables/structs" },
};

const char *program_name = NULL;

static void Error(const char *err, const char *obj, bool usage) {
  printf("%s: %s\n", program_name, err);
  if (obj) printf(": %s", obj);
  printf("\n");
  if (usage) {
    printf("usage: %s [OPTION]... FILE...\n", program_name);
    for (size_t i = 0; i < sizeof(generators) / sizeof(generators[0]); ++i)
      printf("  -%s      %s.\n", generators[i].extension, generators[i].help);
    printf("  -o PATH Prefix PATH to all generated files.\n"
           "  -S      Strict JSON: add quotes to field names.\n"
           "FILEs may depend on declarations in earlier files.\n"
           "Output files are named using the base file name of the input,"
           "and written to the current directory or the path given by -o.\n"
           "example: %s -c -b schema1.fbs schema2.fbs data.json\n",
           program_name);
  }
  exit(1);
}

std::string StripExtension(const std::string &filename) {
  size_t i = filename.find_last_of(".");
  return i != std::string::npos ? filename.substr(0, i) : filename;
}

std::string StripPath(const std::string &filename) {
  size_t i = filename.find_last_of(
    #ifdef WIN32
      "\\:"
    #else
      "/"
    #endif
    );
  return i != std::string::npos ? filename.substr(i + 1) : filename;
}

int main(int argc, const char *argv[]) {
  program_name = argv[0];
  flatbuffers::Parser parser;
  flatbuffers::GeneratorOptions opts;
  std::string output_path;
  const size_t num_generators = sizeof(generators) / sizeof(generators[0]);
  bool generator_enabled[num_generators] = { false };
  bool any_generator = false;
  std::vector<std::string> filenames;
  for (int i = 1; i < argc; i++) {
    const char *arg = argv[i];
    if (arg[0] == '-') {
      if (filenames.size())
        Error("invalid option location", arg, true);
      if (strlen(arg) != 2)
        Error("invalid commandline argument", arg, true);
      switch (arg[1]) {
        case 'o':
          if (++i >= argc) Error("missing path following", arg, true);
          output_path = argv[i];
          break;
        case 'S':
          opts.strict_json = true;
          break;
        default:
          for (size_t i = 0; i < num_generators; ++i) {
            if(!strcmp(arg+1, generators[i].extension)) {
              generator_enabled[i] = true;
              any_generator = true;
              goto found;
            }
          }
          Error("unknown commandline argument", arg, true);
          found:
          break;
      }
    } else {
      filenames.push_back(argv[i]);
    }
  }

  if (!filenames.size()) Error("missing input files", nullptr, true);

  if (!any_generator)
    Error("no options: no output files generated.",
          "specify one of -c -j -t -b etc.", true);

  // Now process the files:
  for (auto file_it = filenames.begin();
            file_it != filenames.end();
          ++file_it) {
      std::string contents;
      if (!flatbuffers::LoadFile(file_it->c_str(), true, &contents))
        Error("unable to load file", file_it->c_str());

      if (!parser.Parse(contents.c_str()))
        Error(parser.error_.c_str());

      std::string filebase = StripPath(StripExtension(*file_it));

      for (size_t i = 0; i < num_generators; ++i) {
        if (generator_enabled[i]) {
          if (!generators[i].generate(parser, output_path, filebase, opts)) {
            Error((std::string("Unable to generate ") +
                   generators[i].name +
                   " for " +
                   filebase).c_str());
          }
        }
      }

      // Since the Parser object retains definitions across files, we must
      // ensure we only output code for these once, in the file they are first
      // declared:
      for (auto it = parser.enums_.vec.begin();
               it != parser.enums_.vec.end(); ++it) {
        (*it)->generated = true;
      }
      for (auto it = parser.structs_.vec.begin();
               it != parser.structs_.vec.end(); ++it) {
        (*it)->generated = true;
      }
  }

  return 0;
}

