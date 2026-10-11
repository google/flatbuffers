#include "codegen/python.h"

#include <set>
#include <sstream>
#include <string>
#include <utility>

namespace flatbuffers {
namespace python {
Version::Version(const std::string& version) {
  if (version.empty()) {
    valid_ = true;
    return;
  }
  std::stringstream ss(version);
  int maj = 0;
  if (!(ss >> maj)) {
    valid_ = false;
    return;
  }
  major = static_cast<int16_t>(maj);
  char dot1 = 0;
  if (ss >> dot1) {
    if (dot1 != '.') {
      valid_ = false;
      return;
    }
    int min = 0;
    if (!(ss >> min)) {
      valid_ = false;
      return;
    }
    minor = static_cast<int16_t>(min);
    char dot2 = 0;
    if (ss >> dot2) {
      if (dot2 != '.') {
        valid_ = false;
        return;
      }
      int mic = 0;
      if (!(ss >> mic)) {
        valid_ = false;
        return;
      }
      micro = static_cast<int16_t>(mic);
    }
  }
  std::string trailing;
  if (ss >> trailing && !trailing.empty()) {
    valid_ = false;
    return;
  }
  valid_ = true;
}

bool Version::IsValid() const {
  return valid_ && (major == 0 || major == 2 || major == 3) && minor >= 0 && micro >= 0;
}

std::set<std::string> Keywords(const Version& version) {
  switch (version.major) {
    case 2:
      // https://docs.python.org/2/reference/lexical_analysis.html#keywords
      return {
          "and",   "as",     "assert", "break",  "class", "continue", "def",
          "del",   "elif",   "else",   "except", "exec",  "finally",  "for",
          "from",  "global", "if",     "import", "in",    "is",       "lambda",
          "not",   "or",     "pass",   "print",  "raise", "return",   "try",
          "while", "with",   "yield",
      };
    case 0:
    case 3:
      // https://docs.python.org/3/reference/lexical_analysis.html#keywords
      return {
          "and",      "as",       "assert",  "async", "await",  "break",
          "class",    "continue", "def",     "del",   "elif",   "else",
          "except",   "False",    "finally", "for",   "from",   "global",
          "if",       "import",   "in",      "is",    "lambda", "None",
          "nonlocal", "not",      "or",      "pass",  "raise",  "return",
          "True",     "try",      "while",   "with",  "yield",
      };
    default:
      return {};
  }
}

const python::Import& python::Imports::Import(const std::string& module) {
  python::Import import;
  import.module = module;
  imports.push_back(std::move(import));
  return imports.back();
}

const python::Import& python::Imports::Import(const std::string& module,
                                              const std::string& name) {
  python::Import import;
  import.module = module;
  import.name = name;
  imports.push_back(std::move(import));
  return imports.back();
}

const python::Import& python::Imports::Export(const std::string& module) {
  python::Import import;
  import.module = module;
  exports.push_back(std::move(import));
  return exports.back();
}

const python::Import& python::Imports::Export(const std::string& module,
                                              const std::string& name) {
  python::Import import;
  import.module = module;
  import.name = name;
  exports.push_back(std::move(import));
  return exports.back();
}
}  // namespace python
}  // namespace flatbuffers
