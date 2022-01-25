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

#include "flatbuffers/flatc.h"

#include <list>
#include <sstream>

#include "bfbs_gen_lua.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

const char *FLATC_VERSION() { return FLATBUFFERS_VERSION(); }

void FlatCompiler::ParseFile(
    flatbuffers::Parser &parser, const std::string &filename,
    const std::string &contents,
    std::vector<const char *> &include_directories) const {
  auto local_include_directory = flatbuffers::StripFileName(filename);
  include_directories.push_back(local_include_directory.c_str());
  include_directories.push_back(nullptr);
  if (!parser.Parse(contents.c_str(), &include_directories[0],
                    filename.c_str())) {
    Error(parser.error_, false, false);
  }
  if (!parser.error_.empty()) { Warn(parser.error_, false); }
  include_directories.pop_back();
  include_directories.pop_back();
}

void FlatCompiler::LoadBinarySchema(flatbuffers::Parser &parser,
                                    const std::string &filename,
                                    const std::string &contents) {
  if (!parser.Deserialize(reinterpret_cast<const uint8_t *>(contents.c_str()),
                          contents.size())) {
    Error("failed to load binary schema: " + filename, false, false);
  }
}

void FlatCompiler::Warn(const std::string &warn, bool show_exe_name) const {
  params_.warn_fn(this, warn, show_exe_name);
}

void FlatCompiler::Error(const std::string &err, bool usage,
                         bool show_exe_name) const {
  params_.error_fn(this, err, usage, show_exe_name);
}

const static FlatCOption options[] = {
  { "o", "", "PATH", "Prefix PATH to all generated files." },
  { "I", "", "PATH", "Search for includes in the specified path." },
  { "M", "", "", "Print make rules for generated files." },
  { "", "version", "", "Print the version number of flatc and exit." },
  { "", "help", "", "Prints this help text and exit." },
  { "", "string-json", "",
    "Strict JSON: field names must be / will be quoted, no trailing commas in "
    "tables/vectors." },
  { "", "allow-non-utf8", "",
    "Pass non-UTF-8 input through parser and emit nonstandard \\x escapes in "
    "JSON. (Default is to raise parse error on non-UTF-8 input.)" },
  { "", "natural-utf8", "",
    "Output strings with UTF-8 as human-readable strings. By default, UTF-8 "
    "characters are printed as \\uXXXX escapes." },
  { "", "defaults-json", "",
    "Output fields whose value is the default when writing JSON" },
  { "", "unknown-json", "",
    "Allow fields in JSON that are not defined in the schema. These fields "
    "will be discared when generating binaries." },
  { "", "no-prefix", "",
    "Don\'t prefix enum values with the enum type in C++." },
  { "", "scoped-enums", "",
    "Use C++11 style scoped and strongly typed enums. Also implies "
    "--no-prefix." },
  { "", "gen-inclues", "",
    "(deprecated), this is the default behavior. If the original behavior is "
    "required (no include statements) use --no-includes." },
  { "", "no-includes", "",
    "Don\'t generate include statements for included schemas the generated "
    "file depends on (C++ / Python)." },
  { "", "gen-mutable", "",
    "Generate accessors that can mutate buffers in-place." },
  { "", "gen-onefile", "",
    "Generate single output file for C#, Go, and Python." },
  { "", "gen-name-strings", "",
    "Generate type name functions for C++ and Rust." },
  { "", "gen-object-api", "", "Generate an additional object-based API." },
  { "", "gen-compare", "", "Generate operator== for object-based API types." },
  { "", "gen-nullable", "",
    "Add Clang _Nullable for C++ pointer. or @Nullable for Java" },
  { "", "java-checkerframe", "", "Add @Pure for Java." },
  { "", "gen-generated", "", "Add @Generated annotation for Java." },
  { "", "gen-jvmstatic", "",
    "Add @JvmStatic annotation for Kotlin methods in companion object for "
    "interop from Java to Kotlin." },
  { "", "gen-all", "",
    "Generate not just code for the current schema files, but for all files it "
    "includes as well. If the language uses a single file for output (by "
    "default the case for C++ and JS), all code will end up in this one "
    "file." },
  { "", "gen-json-emit", "",
    "Generates encoding code which emits Flatbuffers into JSON" },
  { "", "cpp-include", "", "Adds an #include in generated file." },
  { "", "cpp-ptr-type", "T",
    "Set object API pointer type (default std::unique_ptr)." },
  { "", "cpp-str-type", "T",
    "Set object API string type (default std::string). T::c_str(), T::length() "
    "and T::empty() must be supported. The custom type also needs to be "
    "constructible from std::string (see the --cpp-str-flex-ctor option to "
    "change this behavior)" },
  { "", "cpp-str-flex-ctor", "",
    "Don't construct custom string types by passing std::string from "
    "Flatbuffers, but (char* + length)." },
  { "", "cpp-field-case-style", "STYLE",
    "Generate C++ fields using selected case style. Supported STYLE values: * "
    "'unchanged' - leave unchanged (default) * 'upper' - schema snake_case "
    "emits UpperCamel; * 'lower' - schema snake_case emits lowerCamel." },
  { "", "cpp-std", "CPP_STD",
    "Generate a C++ code using features of selected C++ standard. Supported "
    "CPP_STD values: * 'c++0x' - generate code compatible with old compilers; "
    "'c++11' - use C++11 code generator (default); * 'c++17' - use C++17 "
    "features in generated code (experimental)." },
  { "", "cpp-static-reflection", "",
    "When using C++17, generate extra code to provide compile-time (static) "
    "reflection of Flatbuffers types. Requires --cpp-std to be \"c++17\" or "
    "higher." },
  { "", "object-prefix", "PREFIX",
    "Customize class prefix for C++ object-based API." },
  { "", "object-suffix", "SUFFIX",
    "Customize class suffix for C++ object-based API. Default Value is "
    "\"T\"." },
  { "", "go-namespace", "", "Generate the overriding namespace in Golang." },
  { "", "go-import", "IMPORT",
    "Generate the overriding import for flatbuffers in Golang (default is "
    "\"github.com/google/flatbuffers/go\")." },
  { "", "raw-binary", "",
    "Allow binaries without file_identifier to be read. This may crash flatc "
    "given a mismatched schema." },
  { "", "size-prefixed", "", "Input binaries are size prefixed buffers." },
  { "", "proto", "", "Input is a .proto, translate to .fbs." },
  { "", "proto-namespace-suffix", "SUFFIX",
    "Add this namespace to any flatbuffers generated from protobufs." },
  { "", "oneof-union", "", "Translate .proto oneofs to flatbuffer unions." },
  { "", "grpc", "", "Generate GRPC interfaces for the specified languages." },
  { "", "schema", "", "Serialize schemas instead of JSON (use with -b)." },
  { "", "bfbs-filenames", "PATH",
    "Sets the root path where reflection filenames in reflection.fbs are "
    "relative to. The 'root' is denoted with  `//`. E.g. if PATH=/a/b/c "
    "then /a/d/e.fbs will be serialized as //../d/e.fbs. (PATH defaults to the "
    "directory of the first provided schema file." },
  { "", "bfbs-comments", "", "Add doc comments to the binary schema files." },
  { "", "bfbs-builtins", "",
    "Add builtin attributes to the binary schema files." },
  { "", "bfbs-gen-embed", "",
    "Generate code to embed the bfbs schema to the source." },
  { "", "conform", "FILE",
    "Specify a schema the following schemas should be an evolution of. Gives "
    "errors if not." },
  { "", "conform-includes", "PATH",
    "Include path for the schema given with --conform PATH" },
  { "", "filename-suffix", "SUFFIX",
    "The suffix appended to the generated file names (Default is "
    "'_generated')." },
  { "", "filename-ext", "EXT",
    "The extension appended to the generated file names. Default is "
    "language-specific (e.g., '.h' for C++)" },
  { "", "include-prefix", "PATH",
    "Prefix this PATH to any generated include statements." },
  { "", "keep-prefix", "",
    "Keep original prefix of schema include statement." },
  { "", "reflect-types", "",
    "Add minimal type reflection to code generation." },
  { "", "reflect-names", "", "Add minimal type/name reflection." },
  { "", "root-type", "T", "Select or override the default root_type." },
  { "", "require-explicit-ids", "",
    "When parsing schemas, require explicit ids (id: x)." },
  { "", "force-defaults", "",
    "Emit default values in binary output from JSON" },
  { "", "force-empty", "",
    "When serializing from object API representation, force strings and "
    "vectors to empty rather than null." },
  { "", "force-empty-vectors", "",
    "When serializing from object API representation, force vectors to empty "
    "rather than null." },
  { "", "flexbuffers", "",
    "Used with \"binary\" and \"json\" options, it generates data using "
    "schema-less FlexBuffers." },
  { "", "no-warnings", "", "Inhibit all warnings messages." },
  { "", "warning-as-errors", "", "Treat all warnings as errors." },
  { "", "cs-global-alias", "",
    "Prepend \"global::\" to all user generated csharp classes and "
    "structs." },
  { "", "cs-gen-json-serializer", "",
    "Allows (de)serialization of JSON text in the Object API. (requires "
    "--gen-object-api)." },
  { "", "json-nested-bytes", "",
    "Allow a nested_flatbuffer field to be parsed as a vector of bytes"
    "in JSON, which is unsafe unless checked by a verifier afterwards." },
};

static void AppendTextWrappedString(std::stringstream &ss, std::string &text,
                                    size_t max_col, size_t start_col) {
  size_t max_line_length = max_col - start_col;

  if (text.length() > max_line_length) {
    size_t ideal_break_location = text.rfind(' ', max_line_length);
    size_t length = std::min(max_line_length, ideal_break_location);
    ss << text.substr(0, length) << "\n";
    ss << std::string(start_col, ' ');
    std::string rest_of_description = text.substr(
        ((ideal_break_location < max_line_length || text.at(length) == ' ')
             ? length + 1
             : length));
    AppendTextWrappedString(ss, rest_of_description, max_col, start_col);
  } else {
    ss << text;
  }
}

static void AppendOption(std::stringstream &ss, const FlatCOption &option,
                         size_t max_col, size_t min_col_for_description) {
  size_t chars = 2;
  ss << "  ";
  if (!option.short_opt.empty()) {
    chars += 2 + option.short_opt.length();
    ss << "-" << option.short_opt;
    if (!option.long_opt.empty()) {
      chars++;
      ss << ",";
    }
    ss << " ";
  }
  if (!option.long_opt.empty()) {
    chars += 3 + option.long_opt.length();
    ss << "--" << option.long_opt << " ";
  }
  if (!option.parameter.empty()) {
    chars += 1 + option.parameter.length();
    ss << option.parameter << " ";
  }
  size_t start_of_description = chars;
  if (start_of_description > min_col_for_description) {
    ss << "\n";
    start_of_description = min_col_for_description;
    ss << std::string(start_of_description, ' ');
  } else {
    while (start_of_description < min_col_for_description) {
      ss << " ";
      start_of_description++;
    }
  }
  if (!option.description.empty()) {
    std::string description = option.description;
    AppendTextWrappedString(ss, description, max_col, start_of_description);
  }
  ss << "\n";
}

std::string FlatCompiler::GetUsageString(const char *program_name) const {
  std::stringstream ss;
  ss << "Usage: " << program_name << " [OPTION]... FILE... [-- FILE...]\n";
  for (size_t i = 0; i < params_.num_generators; ++i) {
    const Generator &g = params_.generators[i];
    AppendOption(ss, g.option, 80, 25);
  }

  ss << "\n";
  for (const FlatCOption &option : options) {
    AppendOption(ss, option, 80, 25);
  }
  ss << "\n";

  std::string files_description =
      "FILEs may be schemas (must end in .fbs), binary schemas (must end in "
      ".bfbs) or JSON files (conforming to preceding schema). FILEs after the "
      "-- must be binary flatbuffer format files. Output files are named using "
      "the base file name of the input, and written to the current directory "
      "or the path given by -o. example: " +
      std::string(program_name) + " -c -b schema1.fbs schema2.fbs data.json";
  AppendTextWrappedString(ss, files_description, 80, 0);
  ss << "\n";
  return ss.str();
}

int FlatCompiler::Compile(int argc, const char **argv) {
  if (params_.generators == nullptr || params_.num_generators == 0) {
    return 0;
  }

  flatbuffers::IDLOptions opts;
  std::string output_path;

  bool any_generator = false;
  bool print_make_rules = false;
  bool raw_binary = false;
  bool schema_binary = false;
  bool grpc_enabled = false;
  bool requires_bfbs = false;
  std::vector<std::string> filenames;
  std::list<std::string> include_directories_storage;
  std::vector<const char *> include_directories;
  std::vector<const char *> conform_include_directories;
  std::vector<bool> generator_enabled(params_.num_generators, false);
  size_t binary_files_from = std::numeric_limits<size_t>::max();
  std::string conform_to_schema;

  const char *program_name = argv[0];

  for (int argi = 1; argi < argc; argi++) {
    std::string arg = argv[argi];
    if (arg[0] == '-') {
      if (filenames.size() && arg[1] != '-')
        Error("invalid option location: " + arg, true);
      if (arg == "-o") {
        if (++argi >= argc) Error("missing path following: " + arg, true);
        output_path = flatbuffers::ConCatPathFileName(
            flatbuffers::PosixPath(argv[argi]), "");
      } else if (arg == "-I") {
        if (++argi >= argc) Error("missing path following: " + arg, true);
        include_directories_storage.push_back(
            flatbuffers::PosixPath(argv[argi]));
        include_directories.push_back(
            include_directories_storage.back().c_str());
      } else if (arg == "--bfbs-filenames") {
        if (++argi > argc) Error("missing path following: " + arg, true);
        opts.project_root = argv[argi];
        if (!DirExists(opts.project_root.c_str()))
          Error(arg + " is not a directory: " + opts.project_root);
      } else if (arg == "--conform") {
        if (++argi >= argc) Error("missing path following: " + arg, true);
        conform_to_schema = flatbuffers::PosixPath(argv[argi]);
      } else if (arg == "--conform-includes") {
        if (++argi >= argc) Error("missing path following: " + arg, true);
        include_directories_storage.push_back(
            flatbuffers::PosixPath(argv[argi]));
        conform_include_directories.push_back(
            include_directories_storage.back().c_str());
      } else if (arg == "--include-prefix") {
        if (++argi >= argc) Error("missing path following: " + arg, true);
        opts.include_prefix = flatbuffers::ConCatPathFileName(
            flatbuffers::PosixPath(argv[argi]), "");
      } else if (arg == "--keep-prefix") {
        opts.keep_include_path = true;
      } else if (arg == "--strict-json") {
        opts.strict_json = true;
      } else if (arg == "--allow-non-utf8") {
        opts.allow_non_utf8 = true;
      } else if (arg == "--natural-utf8") {
        opts.natural_utf8 = true;
      } else if (arg == "--go-namespace") {
        if (++argi >= argc) Error("missing golang namespace" + arg, true);
        opts.go_namespace = argv[argi];
      } else if (arg == "--go-import") {
        if (++argi >= argc) Error("missing golang import" + arg, true);
        opts.go_import = argv[argi];
      } else if (arg == "--defaults-json") {
        opts.output_default_scalars_in_json = true;
      } else if (arg == "--unknown-json") {
        opts.skip_unexpected_fields_in_json = true;
      } else if (arg == "--no-prefix") {
        opts.prefixed_enums = false;
      } else if (arg == "--scoped-enums") {
        opts.prefixed_enums = false;
        opts.scoped_enums = true;
      } else if (arg == "--no-union-value-namespacing") {
        opts.union_value_namespacing = false;
      } else if (arg == "--gen-mutable") {
        opts.mutable_buffer = true;
      } else if (arg == "--gen-name-strings") {
        opts.generate_name_strings = true;
      } else if (arg == "--gen-object-api") {
        opts.generate_object_based_api = true;
      } else if (arg == "--gen-compare") {
        opts.gen_compare = true;
      } else if (arg == "--cpp-include") {
        if (++argi >= argc) Error("missing include following: " + arg, true);
        opts.cpp_includes.push_back(argv[argi]);
      } else if (arg == "--cpp-ptr-type") {
        if (++argi >= argc) Error("missing type following: " + arg, true);
        opts.cpp_object_api_pointer_type = argv[argi];
      } else if (arg == "--cpp-str-type") {
        if (++argi >= argc) Error("missing type following: " + arg, true);
        opts.cpp_object_api_string_type = argv[argi];
      } else if (arg == "--cpp-str-flex-ctor") {
        opts.cpp_object_api_string_flexible_constructor = true;
      } else if (arg == "--no-cpp-direct-copy") {
        opts.cpp_direct_copy = false;
      } else if (arg == "--cpp-field-case-style") {
        if (++argi >= argc) Error("missing case style following: " + arg, true);
        if (!strcmp(argv[argi], "unchanged"))
          opts.cpp_object_api_field_case_style =
              IDLOptions::CaseStyle_Unchanged;
        else if (!strcmp(argv[argi], "upper"))
          opts.cpp_object_api_field_case_style = IDLOptions::CaseStyle_Upper;
        else if (!strcmp(argv[argi], "lower"))
          opts.cpp_object_api_field_case_style = IDLOptions::CaseStyle_Lower;
        else
          Error("unknown case style: " + std::string(argv[argi]), true);
      } else if (arg == "--gen-nullable") {
        opts.gen_nullable = true;
      } else if (arg == "--java-checkerframework") {
        opts.java_checkerframework = true;
      } else if (arg == "--gen-generated") {
        opts.gen_generated = true;
      } else if (arg == "--gen-json-emit") {
        opts.gen_json_coders = true;
      } else if (arg == "--object-prefix") {
        if (++argi >= argc) Error("missing prefix following: " + arg, true);
        opts.object_prefix = argv[argi];
      } else if (arg == "--object-suffix") {
        if (++argi >= argc) Error("missing suffix following: " + arg, true);
        opts.object_suffix = argv[argi];
      } else if (arg == "--gen-all") {
        opts.generate_all = true;
        opts.include_dependence_headers = false;
      } else if (arg == "--gen-includes") {
        // Deprecated, remove this option some time in the future.
        Warn("warning: --gen-includes is deprecated (it is now default)\n");
      } else if (arg == "--no-includes") {
        opts.include_dependence_headers = false;
      } else if (arg == "--gen-onefile") {
        opts.one_file = true;
        opts.include_dependence_headers = false;
      } else if (arg == "--raw-binary") {
        raw_binary = true;
      } else if (arg == "--size-prefixed") {
        opts.size_prefixed = true;
      } else if (arg == "--") {  // Separator between text and binary inputs.
        binary_files_from = filenames.size();
      } else if (arg == "--proto") {
        opts.proto_mode = true;
      } else if (arg == "--proto-namespace-suffix") {
        if (++argi >= argc) Error("missing namespace suffix" + arg, true);
        opts.proto_namespace_suffix = argv[argi];
      } else if (arg == "--oneof-union") {
        opts.proto_oneof_union = true;
      } else if (arg == "--schema") {
        schema_binary = true;
      } else if (arg == "-M") {
        print_make_rules = true;
      } else if (arg == "--version") {
        printf("flatc version %s\n", FLATC_VERSION());
        exit(0);
      } else if (arg == "--help") {
        printf("%s\n", GetUsageString(program_name).c_str());
        exit(0);
      } else if (arg == "--grpc") {
        grpc_enabled = true;
      } else if (arg == "--bfbs-comments") {
        opts.binary_schema_comments = true;
      } else if (arg == "--bfbs-builtins") {
        opts.binary_schema_builtins = true;
      } else if (arg == "--bfbs-gen-embed") {
        opts.binary_schema_gen_embed = true;
      } else if (arg == "--reflect-types") {
        opts.mini_reflect = IDLOptions::kTypes;
      } else if (arg == "--reflect-names") {
        opts.mini_reflect = IDLOptions::kTypesAndNames;
      } else if (arg == "--require-explicit-ids") {
        opts.require_explicit_ids = true;
      } else if (arg == "--root-type") {
        if (++argi >= argc) Error("missing type following: " + arg, true);
        opts.root_type = argv[argi];
      } else if (arg == "--filename-suffix") {
        if (++argi >= argc) Error("missing filename suffix: " + arg, true);
        opts.filename_suffix = argv[argi];
      } else if (arg == "--filename-ext") {
        if (++argi >= argc) Error("missing filename extension: " + arg, true);
        opts.filename_extension = argv[argi];
      } else if (arg == "--force-defaults") {
        opts.force_defaults = true;
      } else if (arg == "--force-empty") {
        opts.set_empty_strings_to_null = false;
        opts.set_empty_vectors_to_null = false;
      } else if (arg == "--force-empty-vectors") {
        opts.set_empty_vectors_to_null = false;
      } else if (arg == "--java-primitive-has-method") {
        opts.java_primitive_has_method = true;
      } else if (arg == "--cs-gen-json-serializer") {
        opts.cs_gen_json_serializer = true;
      } else if (arg == "--flexbuffers") {
        opts.use_flexbuffers = true;
      } else if (arg == "--gen-jvmstatic") {
        opts.gen_jvmstatic = true;
      } else if (arg == "--no-warnings") {
        opts.no_warnings = true;
      } else if (arg == "--warnings-as-errors") {
        opts.warnings_as_errors = true;
      } else if (arg == "--cpp-std") {
        if (++argi >= argc)
          Error("missing C++ standard specification" + arg, true);
        opts.cpp_std = argv[argi];
      } else if (arg.rfind("--cpp-std=", 0) == 0) {
        opts.cpp_std = arg.substr(std::string("--cpp-std=").size());
      } else if (arg == "--cpp-static-reflection") {
        opts.cpp_static_reflection = true;
      } else if (arg == "--cs-global-alias") {
        opts.cs_global_alias = true;
      } else if (arg == "--json-nested-bytes") {
        opts.json_nested_legacy_flatbuffers = true;
      } else {
        for (size_t i = 0; i < params_.num_generators; ++i) {
          if (arg == "--" + params_.generators[i].option.long_opt ||
              arg == "-" + params_.generators[i].option.short_opt) {
            generator_enabled[i] = true;
            any_generator = true;
            opts.lang_to_generate |= params_.generators[i].lang;
            if (params_.generators[i].bfbs_generator) {
              opts.binary_schema_comments = true;
              requires_bfbs = true;
            }
            goto found;
          }
        }
        Error("unknown commandline argument: " + arg, true);

      found:;
      }
    } else {
      filenames.push_back(flatbuffers::PosixPath(argv[argi]));
    }
  }

  if (!filenames.size()) Error("missing input files", false, true);

  if (opts.proto_mode) {
    if (any_generator)
      Error("cannot generate code directly from .proto files", true);
  } else if (!any_generator && conform_to_schema.empty()) {
    Error("no options: specify at least one generator.", true);
  }

  if (opts.cs_gen_json_serializer && !opts.generate_object_based_api) {
    Error(
        "--cs-gen-json-serializer requires --gen-object-api to be set as "
        "well.");
  }

  flatbuffers::Parser conform_parser;
  if (!conform_to_schema.empty()) {
    std::string contents;
    if (!flatbuffers::LoadFile(conform_to_schema.c_str(), true, &contents))
      Error("unable to load schema: " + conform_to_schema);

    if (flatbuffers::GetExtension(conform_to_schema) ==
        reflection::SchemaExtension()) {
      LoadBinarySchema(conform_parser, conform_to_schema, contents);
    } else {
      ParseFile(conform_parser, conform_to_schema, contents,
                conform_include_directories);
    }
  }

  std::unique_ptr<flatbuffers::Parser> parser(new flatbuffers::Parser(opts));

  for (auto file_it = filenames.begin(); file_it != filenames.end();
       ++file_it) {
    auto &filename = *file_it;
    std::string contents;
    if (!flatbuffers::LoadFile(filename.c_str(), true, &contents))
      Error("unable to load file: " + filename);

    bool is_binary =
        static_cast<size_t>(file_it - filenames.begin()) >= binary_files_from;
    auto ext = flatbuffers::GetExtension(filename);
    const bool is_schema = ext == "fbs" || ext == "proto";
    if (is_schema && opts.project_root.empty()) {
      opts.project_root = StripFileName(filename);
    }
    const bool is_binary_schema = ext == reflection::SchemaExtension();
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
                filename +
                "\" matches the schema, use --raw-binary to read this file"
                " anyway.");
        } else if (!flatbuffers::BufferHasIdentifier(
                       contents.c_str(), parser->file_identifier_.c_str(),
                       opts.size_prefixed)) {
          Error("binary \"" + filename +
                "\" does not have expected file_identifier \"" +
                parser->file_identifier_ +
                "\", use --raw-binary to read this file anyway.");
        }
      }
    } else {
      // Check if file contains 0 bytes.
      if (!opts.use_flexbuffers && !is_binary_schema &&
          contents.length() != strlen(contents.c_str())) {
        Error("input file appears to be binary: " + filename, true);
      }
      if (is_schema || is_binary_schema) {
        // If we're processing multiple schemas, make sure to start each
        // one from scratch. If it depends on previous schemas it must do
        // so explicitly using an include.
        parser.reset(new flatbuffers::Parser(opts));
      }
      // Try to parse the file contents (binary schema/flexbuffer/textual
      // schema)
      if (is_binary_schema) {
        LoadBinarySchema(*parser.get(), filename, contents);
      } else if (opts.use_flexbuffers) {
        if (opts.lang_to_generate == IDLOptions::kJson) {
          auto data = reinterpret_cast<const uint8_t *>(contents.c_str());
          auto size = contents.size();
          std::vector<uint8_t> reuse_tracker;
          if (!flexbuffers::VerifyBuffer(data, size, &reuse_tracker))
            Error("flexbuffers file failed to verify: " + filename, false);
          parser->flex_root_ = flexbuffers::GetRoot(data, size);
        } else {
          parser->flex_builder_.Clear();
          ParseFile(*parser.get(), filename, contents, include_directories);
        }
      } else {
        ParseFile(*parser.get(), filename, contents, include_directories);
        if (!is_schema && !parser->builder_.GetSize()) {
          // If a file doesn't end in .fbs, it must be json/binary. Ensure we
          // didn't just parse a schema with a different extension.
          Error("input file is neither json nor a .fbs (schema) file: " +
                    filename,
                true);
        }
      }
      if ((is_schema || is_binary_schema) && !conform_to_schema.empty()) {
        auto err = parser->ConformTo(conform_parser);
        if (!err.empty()) Error("schemas don\'t conform: " + err, false);
      }
      if (schema_binary || opts.binary_schema_gen_embed) {
        parser->Serialize();
      }
      if (schema_binary) {
        parser->file_extension_ = reflection::SchemaExtension();
      }
    }
    std::string filebase =
        flatbuffers::StripPath(flatbuffers::StripExtension(filename));

    // If one of the generators uses bfbs, serialize the parser and get
    // the serialized buffer and length.
    const uint8_t *bfbs_buffer = nullptr;
    int64_t bfbs_length = 0;
    if (requires_bfbs) {
      parser->Serialize();
      bfbs_buffer = parser->builder_.GetBufferPointer();
      bfbs_length = parser->builder_.GetSize();
    }

    for (size_t i = 0; i < params_.num_generators; ++i) {
      if (generator_enabled[i]) {
        if (!print_make_rules) {
          flatbuffers::EnsureDirExists(output_path);

          // Prefer bfbs generators if present.
          if (params_.generators[i].bfbs_generator) {
            const GeneratorStatus status =
                params_.generators[i].bfbs_generator->Generate(bfbs_buffer,
                                                               bfbs_length);
            if (status != OK) {
              Error(std::string("Unable to generate ") +
                    params_.generators[i].lang_name + " for " + filebase +
                    " using bfbs generator.");
            }
          } else {
            if ((!params_.generators[i].schema_only ||
                 (is_schema || is_binary_schema)) &&
                !params_.generators[i].generate(*parser.get(), output_path,
                                                filebase)) {
              Error(std::string("Unable to generate ") +
                    params_.generators[i].lang_name + " for " + filebase);
            }
          }
        } else {
          if (params_.generators[i].make_rule == nullptr) {
            Error(std::string("Cannot generate make rule for ") +
                  params_.generators[i].lang_name);
          } else {
            std::string make_rule = params_.generators[i].make_rule(
                *parser.get(), output_path, filename);
            if (!make_rule.empty())
              printf("%s\n",
                     flatbuffers::WordWrap(make_rule, 80, " ", " \\").c_str());
          }
        }
        if (grpc_enabled) {
          if (params_.generators[i].generateGRPC != nullptr) {
            if (!params_.generators[i].generateGRPC(*parser.get(), output_path,
                                                    filebase)) {
              Error(std::string("Unable to generate GRPC interface for") +
                    params_.generators[i].lang_name);
            }
          } else {
            Warn(std::string("GRPC interface generator not implemented for ") +
                 params_.generators[i].lang_name);
          }
        }
      }
    }

    if (!opts.root_type.empty()) {
      if (!parser->SetRootType(opts.root_type.c_str()))
        Error("unknown root type: " + opts.root_type);
      else if (parser->root_struct_def_->fixed)
        Error("root type must be a table");
    }

    if (opts.proto_mode) GenerateFBS(*parser.get(), output_path, filebase);

    // We do not want to generate code for the definitions in this file
    // in any files coming up next.
    parser->MarkGenerated();
  }
  if (opts.lang_to_generate & IDLOptions::kRust && !parser->opts.one_file) {
    GenerateRustModuleRootFile(*parser, output_path);
  }
  return 0;
}

}  // namespace flatbuffers
