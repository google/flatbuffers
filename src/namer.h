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
inline SkipFile operator&(SkipFile a, SkipFile b) {
  return static_cast<SkipFile>(static_cast<int>(a) & static_cast<int>(b));
}
// Options for Namer::Directories
enum class SkipDir {
  None = 0,
  // Skip prefixing the -o $output_path.
  OutputPath = 1,
  // Skip trailing path seperator.
  TrailingPathSeperator = 2,
  OutputPathAndTrailingPathSeparator = 3,
};
inline SkipDir operator&(SkipDir a, SkipDir b) {
  return static_cast<SkipDir>(static_cast<int>(a) & static_cast<int>(b));
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
    // Case style for flatbuffers-defined variables.
    // e.g. `int my_variable = 2`
    Case variables;
    // Case style for flatbuffers-defined variants.
    // e.g. `enum class Enum { MyVariant, }`
    Case variants;
    // Seperator for qualified enum names.
    // e.g. `Enum::MyVariant` uses `::`.
    std::string enum_variant_seperator;

    // Configures, when formatting code, whether symbols are checked against
    // keywords and escaped before or after case conversion. It does not make
    // sense to do so before, but its legacy behavior. :shrug:
    // TODO(caspern): Deprecate.
    enum class Escape {
      BeforeConvertingCase,
      AfterConvertingCase,
    };
    Escape escape_keywords;

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

  // Types are always structs or enums so we can only expose these two
  // overloads.
  std::string Type(const StructDef &d) const { return Type(d.name); }
  std::string Type(const EnumDef &d) const { return Type(d.name); }

  template<typename T> std::string Method(const T &s) const {
    return Method(s.name);
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

  std::string Function(const Definition &s) const { return Function(s.name); }

  std::string Field(const FieldDef &s) const { return Field(s.name); }

  std::string Variable(const std::string &s) const {
    return Format(s, config_.variables);
  }

  std::string Variable(const FieldDef &s) const { return Variable(s.name); }

  std::string Variable(const StructDef &s) const { return Variable(s.name); }

  std::string Variant(const EnumVal &s) const { return Variant(s.name); }

  std::string EnumVariant(const EnumDef &e, const EnumVal &v) const {
    return Type(e) + config_.enum_variant_seperator + Variant(v);
  }

  std::string ObjectType(const StructDef &d) const {
    return ObjectType(d.name);
  }
  std::string ObjectType(const EnumDef &d) const { return ObjectType(d.name); }

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

  std::string Namespace(const struct Namespace &ns) const {
    return Namespace(ns.components);
  }

  std::string NamespacedType(const Definition &def) const {
    return NamespacedString(def.defined_namespace, Type(def.name));
  }

  std::string NamespacedType(const std::vector<std::string> &ns,
                             const std::string &s) const {
    return (ns.empty() ? "" : (Namespace(ns) + config_.namespace_seperator)) +
           Type(s);
  }

  std::string NamespacedObjectType(const Definition &def) const {
    return NamespacedString(def.defined_namespace, ObjectType(def.name));
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
  template<typename T>
  std::string File(const T &f, SkipFile skips = SkipFile::None) const {
    return File(f.name, skips);
  }

  // Formats `directories` prefixed with the output_path and joined with the
  // right seperator. Output path prefixing and the trailing separator may be
  // skiped using `skips`.
  // Callers may want to use `EnsureDirExists` with the result.
  std::string Directories(const std::vector<std::string> &directories,
                          SkipDir skips = SkipDir::None) const {
    const bool skip_output_path =
        (skips & SkipDir::OutputPath) != SkipDir::None;
    const bool skip_trailing_seperator =
        (skips & SkipDir::TrailingPathSeperator) != SkipDir::None;
    std::string result = skip_output_path ? "" : config_.output_path;
    for (auto d = directories.begin(); d != directories.end(); d++) {
      result += ConvertCase(*d, config_.directories, Case::kUpperCamel);
      result.push_back(kPathSeparator);
    }
    if (skip_trailing_seperator) result.pop_back();
    return result;
  }

  std::string Directories(const struct Namespace &ns,
                          SkipDir skips = SkipDir::None) const {
    return Directories(ns.components, skips);
  }

  std::string EscapeKeyword(const std::string &name) const {
    if (keywords_.find(name) == keywords_.end()) {
      return name;
    } else {
      return config_.keyword_prefix + name + config_.keyword_suffix;
    }
  }

  // Legacy fields do not really follow the usual config and should be
  // considered for deprecation.

  std::string LegacyRustNativeVariant(const EnumVal &v) const {
    return ConvertCase(EscapeKeyword(v.name), Case::kUpperCamel);
  }

  std::string LegacyRustFieldOffsetName(const FieldDef& field) const {

    return "VT_" + ConvertCase(EscapeKeyword(field.name), Case::kAllUpper);
  }

  // TODO(caspern): What's up with this case style?
  std::string LegacySwiftVariant(const EnumVal &ev) const {
    auto name = ev.name;
    if (isupper(name.front())) {
      std::transform(name.begin(), name.end(), name.begin(), CharToLower);
    }
    return EscapeKeyword(ConvertCase(name, Case::kLowerCamel));
  }

 private:
  std::string Type(const std::string &s) const {
    return Format(s, config_.types);
  }

  std::string ObjectType(const std::string &s) const {
    return config_.object_prefix + Type(s) + config_.object_suffix;
  }

  std::string Field(const std::string &s) const {
    return Format(s, config_.fields);
  }

  std::string Variant(const std::string &s) const {
    return Format(s, config_.variants);
  }

  std::string NamespacedString(const struct Namespace *ns,
                               const std::string &str) const {
    std::string ret;
    if (ns != nullptr) { ret += Namespace(ns->components); }
    if (!ret.empty()) ret += config_.namespace_seperator;
    return ret + str;
  }

  std::string Format(const std::string &s, Case casing) const {
    if (config_.escape_keywords == Config::Escape::BeforeConvertingCase) {
      return ConvertCase(EscapeKeyword(s), casing, Case::kLowerCamel);
    } else {
      return EscapeKeyword(ConvertCase(s, casing, Case::kLowerCamel));
    }
  }
  const Config config_;
  const std::set<std::string> keywords_;
};

}  // namespace flatbuffers

#endif  // FLATBUFFERS_NAMER
