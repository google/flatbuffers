#include "codegen/python.h"

#include <set>
#include <sstream>
#include <string>
#include <utility>

namespace flatbuffers {
namespace python {
Version::Version(const std::string &version) {
  std::stringstream ss(version);
  char dot;
  ss >> major >> dot >> minor >> dot >> micro;
}

bool Version::IsValid() const {
  return (major == 0 || major == 2 || major == 3) && minor >= 0 && micro >= 0;
}

std::set<std::string> Keywords(const Version &version) {
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

const python::Import &python::Imports::Import(const std::string &module) {
  python::Import import;
  import.module = module;
  imports.push_back(std::move(import));
  return imports.back();
}

const python::Import &python::Imports::Import(const std::string &module,
                                              const std::string &name) {
  python::Import import;
  import.module = module;
  import.name = name;
  imports.push_back(std::move(import));
  return imports.back();
}
}  // namespace python
}  // namespace flatbuffers
