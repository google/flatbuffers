#ifndef FLATBUFFERS_NAMER
#define FLATBUFFERS_NAMER

#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

// Options for Namer::File.
enum class SkipFile {
  None = 0,
  Suffix = 1,
  Extension = 2,
  SuffixAndExtension = 3,
};
SkipFile operator&(SkipFile a, SkipFile b) {
  return static_cast<SkipFile>(static_cast<int>(a) & static_cast<int>(b));
}

// `Namer` applies style configuration to symbols in generated code. It manages
// casing, escapes keywords, and object API naming.
// TODO: Refactor all code generators to use this.
class Namer {
 public:
  struct Config {
    // Symbols in code.

    // Case style for flatbuffers-defined types.
    // e.g. `class TableA {}`
    Case types;
    // Case style for flatbuffers-defined constants.
    // e.g. `uint64_t ENUM_A_MAX`;
    Case constants;
    // Case style for flatbuffers-defined methods.
    // e.g. `class TableA { int field_a(); }`
    Case methods;
    // Case style for flatbuffers-defined functions.
    // e.g. `TableA* get_table_a_root()`;
    Case functions;
    // Case style for flatbuffers-defined fields.
    // e.g. `struct Struct { int my_field; }`
    Case fields;
    // Case style for flatbuffers-defined variants.
    // e.g. `enum class Enum { MyVariant, }`
    Case variants;
    // Seperator for qualified enum names.
    // e.g. `Enum::MyVariant` uses `::`.
    std::string enum_variant_seperator;

    // Namespaces

    // e.g. `namespace my_namespace {}`
    Case namespaces;
    // The seperator between namespaces in a namespace path.
    std::string namespace_seperator;

    // Object API.
    // Native versions flatbuffers types have this prefix.
    // e.g. "" (it's usually empty string)
    std::string object_prefix;
    // Native versions flatbuffers types have this suffix.
    // e.g. "T"
    std::string object_suffix;

    // Keywords.
    // Prefix used to escape keywords. It is usually empty string.
    std::string keyword_prefix;
    // Suffix used to escape keywords. It is usually "_".
    std::string keyword_suffix;

    // Files.

    // Case style for filenames. e.g. `foo_bar_generated.rs`
    Case filenames;
    // Case style for directories, e.g. `output_files/foo_bar/baz/`
    Case directories;
    // The directory within which we will generate files.
    std::string output_path;
    // Suffix for generated file names, e.g. "_generated".
    std::string filename_suffix;
    // Extension for generated files, e.g. ".cpp" or ".rs".
    std::string filename_extension;

    // This is a temporary helper function for code generators to call until all
    // code generators are using `Namer`. After that point, we can centralize
    // flag-overriding logic into flatc.cpp
    Config WithFlagOptions(const IDLOptions &opts,
                           const std::string &path) const {
      Config result = *this;
      result.object_prefix = opts.object_prefix;
      result.object_suffix = opts.object_suffix;
      result.output_path = path;
      result.filename_suffix = opts.filename_suffix;
      return result;
    }
  };
  Namer(Config config, std::set<std::string> keywords)
      : config_(config), keywords_(std::move(keywords)) {}

  std::string Type(const std::string &s) const {
    return Format(s, config_.types);
  }

  std::string Method(const std::string &s) const {
    return Format(s, config_.methods);
  }

  std::string Constant(const std::string &s) const {
    return Format(s, config_.constants);
  }

  std::string Function(const std::string &s) const {
    return Format(s, config_.functions);
  }

  std::string Field(const std::string &s) const {
    return Format(s, config_.fields);
  }

  std::string Variant(const std::string &s) const {
    return Format(s, config_.variants);
  }

  std::string EnumVariant(const std::string &e, const std::string v) const {
    return Type(e) + config_.enum_variant_seperator + Variant(v);
  }

  std::string ObjectType(const std::string &s) const {
    return config_.object_prefix + Type(s) + config_.object_suffix;
  }

  std::string Namespace(const std::string &s) const {
    return Format(s, config_.namespaces);
  }

  std::string Namespace(const std::vector<std::string> &ns) const {
    std::string result;
    for (auto it = ns.begin(); it != ns.end(); it++) {
      if (it != ns.begin()) result += config_.namespace_seperator;
      result += Namespace(*it);
    }
    return result;
  }

  // Returns `filename` with the right casing, suffix, and extension.
  std::string File(const std::string &filename,
                   SkipFile skips = SkipFile::None) const {
    const bool skip_suffix = (skips & SkipFile::Suffix) != SkipFile::None;
    const bool skip_ext = (skips & SkipFile::Extension) != SkipFile::None;
    return ConvertCase(filename, config_.filenames, Case::kUpperCamel) +
           (skip_suffix ? "" : config_.filename_suffix) +
           (skip_ext ? "" : config_.filename_extension);
  }
  // Formats `directories` and returns a filepath with the right seperator.
  // Callers may want to use `EnsureDirExists` with the result.
  std::string Directories(const std::vector<std::string> &directories) const {
    std::string result = config_.output_path;
    for (auto d = directories.begin(); d != directories.end(); d++) {
      result += ConvertCase(*d, config_.directories, Case::kUpperCamel);
      result.push_back(kPathSeparator);
    }
    return result;
  }

  std::string EscapeKeyword(const std::string &name) const {
    if (keywords_.find(name) == keywords_.end()) {
      return name;
    } else {
      return config_.keyword_prefix + name + config_.keyword_suffix;
    }
  }

 private:
  std::string Format(const std::string &s, Case casing) const {
    // NOTE: If you need to escape keywords after converting case, which would
    // make more sense than this, make it a config option.
    return ConvertCase(EscapeKeyword(s), casing, Case::kLowerCamel);
  }
  const Config config_;
  const std::set<std::string> keywords_;
};

}  // namespace flatbuffers

#endif  // FLATBUFFERS_NAMER
