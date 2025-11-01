#ifndef FLATBUFFERS_INCLUDE_CODEGEN_IDLNAMES_H_
#define FLATBUFFERS_INCLUDE_CODEGEN_IDLNAMES_H_

#include <string>
#include <unordered_set>

namespace flatbuffers {

// Hash + equality on pointer identity, not string content
struct PtrHash {
  size_t operator()(const std::string* s) const noexcept {
    return std::hash<const void*>()(static_cast<const void*>(s));
  }
};

struct PtrEqual {
  bool operator()(const std::string* a, const std::string* b) const noexcept {
    return a == b;
  }
};

inline std::unordered_set<const std::string*, PtrHash, PtrEqual>&
IdlNamePointers() {
  static std::unordered_set<const std::string*, PtrHash, PtrEqual> pointers;
  return pointers;
}

// Record the address of an IDL name (struct name, field name, enum value,
// etc.).
inline void RecordIdlName(const std::string* name) {
  IdlNamePointers().insert(name);
}

// Test whether a given string object comes from the IDL.
inline bool IsIdlName(const std::string& s) {
  return IdlNamePointers().count(&s) != 0;
}

}  // namespace flatbuffers

#endif  // FLATBUFFERS_INCLUDE_CODEGEN_IDLNAMES_H_
