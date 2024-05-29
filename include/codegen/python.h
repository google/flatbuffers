#ifndef FLATBUFFERS_INCLUDE_CODEGEN_PYTHON_H_
#define FLATBUFFERS_INCLUDE_CODEGEN_PYTHON_H_

#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace flatbuffers {
namespace python {
// `Version` represent a Python version.
//
// The zero value (i.e. `Version{}`) represents both Python2 and Python3.
//
// https://docs.python.org/3/faq/general.html#how-does-the-python-version-numbering-scheme-work
struct Version {
  explicit Version(const std::string &version);

  bool IsValid() const;

  int16_t major = 0;
  int16_t minor = 0;
  int16_t micro = 0;
};

std::set<std::string> Keywords(const Version &version);

struct Import {
  bool IsLocal() const { return module == "."; }

  std::string module;
  std::string name;
};

struct Imports {
  const python::Import &Import(const std::string &module);
  const python::Import &Import(const std::string &module,
                               const std::string &name);

  std::vector<python::Import> imports;
};
}  // namespace python
}  // namespace flatbuffers

#endif  // FLATBUFFERS_INCLUDE_CODEGEN_PYTHON_H_
