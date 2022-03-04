#ifndef FLATBUFFERS_NAMER
#define FLATBUFFERS_NAMER

#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

// `Namer` applies style configuration to symbols in generated code. It manages
// casing, escapes keywords, and object API naming.
// TODO: Refactor all code generators to use this.
class Namer {
 public:
  struct Config {
    // Symbols in code.
    Case types;
    Case constants;
    Case methods;
    Case functions;
    Case fields;
    Case variants;

    // Namespaces
    Case namespaces;
    std::string namespace_seperator;

    // Object API.
    std::string object_prefix;
    std::string object_suffix;

    // Keywords.
    std::string keyword_prefix;
    std::string keyword_suffix;

    // Files.
    Case filenames;
    Case directories;
    std::string output_path;
    std::string filename_suffix;
    std::string filename_extension;

    // DO NOT SUBMIT
    // comment/doc comment.
  };
  Namer(Config config, std::set<std::string> keywords)
      : config_(config), keywords_(std::move(keywords)) {}

  // Returns a mutable reference to the config so it can be modified **before**
  // code generation. The intent is for this class to be const while generating
  // code.
  Config &GetConfig() { return config_; }

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
  std::string ObjectType(const std::string &s) const {
    return config_.object_prefix + Type(s) + config_.object_suffix;
  }
  std::string Namespace(const std::string &s) const {
    return Format(s, config_.namespaces);
  }
  // DO NOT SUBMIT: Add GetRelativeNamespaceTraversal?

  std::string EscapeKeyword(const std::string &name) const {
    if (keywords_.find(name) == keywords_.end()) {
      return name;
    } else {
      return config_.keyword_prefix + name + config_.keyword_suffix;
    }
  }
  std::string Format(const std::string &s, Case casing) const {
    // NOTE: If you need to escape keywords after converting case, which would
    // make more sense than this, make it a config option.
    return ConvertCase(EscapeKeyword(s), casing, Case::kLowerCamel);
  }
  // Returns `filename` with the right casing and suffix and extension.
  std::string File(const std::string &filename, bool skip_suffix = false,
                   bool skip_ext = false) const {
    return ConvertCase(filename, config_.filenames, Case::kUpperCamel) +
           (skip_suffix ? "" : config_.filename_suffix) +
           (skip_ext ? "" : config_.filename_extension);
  }
  // Formats `directories` and returns a filepath with the right seperator.
  // If `mkdir`, ensure intermediate directories exist.
  // TODO(caspern): Maybe mkdir should be handled by caller?
  std::string Directories(const std::vector<std::string> &directories,
                          bool mkdir = true) const {
    std::string result = config_.output_path;
    for (auto d = directories.begin(); d != directories.end(); d++) {
      result.append(ConvertCase(*d, config_.directories, Case::kUpperCamel));
      result.push_back(kPathSeparator);
      if (mkdir) EnsureDirExists(result);
    }
    return result;
  }

  Config config_;
  std::set<std::string> keywords_;
};

// This is a temporary helper function for code generators to call until all
// code generators are using `Namer`. After that point, we can centralize this
// into flatc.cpp
void AddFlagOptions(Namer &namer, const IDLOptions &opts,
                    const std::string &output_path) {
  namer.GetConfig().object_prefix = opts.object_prefix;
  namer.GetConfig().object_suffix = opts.object_suffix;
  namer.GetConfig().output_path = output_path;
  namer.GetConfig().filename_suffix = opts.filename_suffix;
}

}  // namespace flatbuffers

#endif  // FLATBUFFERS_NAMER
