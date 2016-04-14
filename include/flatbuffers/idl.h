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

#ifndef FLATBUFFERS_IDL_H_
#define FLATBUFFERS_IDL_H_

#include <map>
#include <stack>
#include <memory>
#include <functional>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/hash.h"
#include "flatbuffers/reflection.h"

// This file defines the data types representing a parsed IDL (Interface
// Definition Language) / schema file.

namespace flatbuffers {

// The order of these matters for Is*() functions below.
// Additionally, Parser::ParseType assumes bool..string is a contiguous range
// of type tokens.
#define FLATBUFFERS_GEN_TYPES_SCALAR(TD) \
  TD(NONE,   "",       uint8_t,  byte,   byte,    byte,   uint8) \
  TD(UTYPE,  "",       uint8_t,  byte,   byte,    byte,   uint8) /* begin scalar/int */ \
  TD(BOOL,   "bool",   uint8_t,  boolean,byte,    bool,   bool) \
  TD(CHAR,   "byte",   int8_t,   byte,   int8,    sbyte,  int8) \
  TD(UCHAR,  "ubyte",  uint8_t,  byte,   byte,    byte,   uint8) \
  TD(SHORT,  "short",  int16_t,  short,  int16,   short,  int16) \
  TD(USHORT, "ushort", uint16_t, short,  uint16,  ushort, uint16) \
  TD(INT,    "int",    int32_t,  int,    int32,   int,    int32) \
  TD(UINT,   "uint",   uint32_t, int,    uint32,  uint,   uint32) \
  TD(LONG,   "long",   int64_t,  long,   int64,   long,   int64) \
  TD(ULONG,  "ulong",  uint64_t, long,   uint64,  ulong,  uint64) /* end int */ \
  TD(FLOAT,  "float",  float,    float,  float32, float,  float32) /* begin float */ \
  TD(DOUBLE, "double", double,   double, float64, double, float64) /* end float/scalar */
#define FLATBUFFERS_GEN_TYPES_POINTER(TD) \
  TD(STRING, "string", Offset<void>, int, int, StringOffset, int) \
  TD(VECTOR, "",       Offset<void>, int, int, VectorOffset, int) \
  TD(STRUCT, "",       Offset<void>, int, int, int, int) \
  TD(UNION,  "",       Offset<void>, int, int, int, int)

// The fields are:
// - enum
// - FlatBuffers schema type.
// - C++ type.
// - Java type.
// - Go type.
// - C# / .Net type.
// - Python type.

// using these macros, we can now write code dealing with types just once, e.g.

/*
switch (type) {
  #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE) \
    case BASE_TYPE_ ## ENUM: \
      // do something specific to CTYPE here
    FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
  #undef FLATBUFFERS_TD
}
*/

#define FLATBUFFERS_GEN_TYPES(TD) \
        FLATBUFFERS_GEN_TYPES_SCALAR(TD) \
        FLATBUFFERS_GEN_TYPES_POINTER(TD)

// Create an enum for all the types above.
#ifdef __GNUC__
__extension__  // Stop GCC complaining about trailing comma with -Wpendantic.
#endif
enum BaseType {
  #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE) \
      BASE_TYPE_ ## ENUM,
    FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
  #undef FLATBUFFERS_TD
};

#define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE) \
    static_assert(sizeof(CTYPE) <= sizeof(largest_scalar_t), \
                  "define largest_scalar_t as " #CTYPE);
  FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
#undef FLATBUFFERS_TD

inline bool IsScalar (BaseType t) { return t >= BASE_TYPE_UTYPE &&
                                           t <= BASE_TYPE_DOUBLE; }
inline bool IsInteger(BaseType t) { return t >= BASE_TYPE_UTYPE &&
                                           t <= BASE_TYPE_ULONG; }
inline bool IsFloat  (BaseType t) { return t == BASE_TYPE_FLOAT ||
                                           t == BASE_TYPE_DOUBLE; }

extern const char *const kTypeNames[];
extern const char kTypeSizes[];

inline size_t SizeOf(BaseType t) {
  return kTypeSizes[t];
}

struct StructDef;
struct EnumDef;
class Parser;

// Represents any type in the IDL, which is a combination of the BaseType
// and additional information for vectors/structs_.
struct Type {
  explicit Type(BaseType _base_type = BASE_TYPE_NONE,
                StructDef *_sd = nullptr, EnumDef *_ed = nullptr)
    : base_type(_base_type),
      element(BASE_TYPE_NONE),
      struct_def(_sd),
      enum_def(_ed)
  {}

  bool operator==(const Type &o) {
    return base_type == o.base_type && element == o.element &&
           struct_def == o.struct_def && enum_def == o.enum_def;
  }

  Type VectorType() const { return Type(element, struct_def, enum_def); }

  Offset<reflection::Type> Serialize(FlatBufferBuilder *builder) const;

  BaseType base_type;
  BaseType element;       // only set if t == BASE_TYPE_VECTOR
  StructDef *struct_def;  // only set if t or element == BASE_TYPE_STRUCT
  EnumDef *enum_def;      // set if t == BASE_TYPE_UNION / BASE_TYPE_UTYPE,
                          // or for an integral type derived from an enum.
};

// Represents a parsed scalar value, it's type, and field offset.
struct Value {
  Value() : constant("0"), offset(static_cast<voffset_t>(
                                ~(static_cast<voffset_t>(0U)))) {}
  Type type;
  std::string constant;
  voffset_t offset;
};

// Helper class that retains the original order of a set of identifiers and
// also provides quick lookup.
template<typename T> class SymbolTable {
 public:
  ~SymbolTable() {
    for (auto it = vec.begin(); it != vec.end(); ++it) {
      delete *it;
    }
  }

  bool Add(const std::string &name, T *e) {
    vec.emplace_back(e);
    auto it = dict.find(name);
    if (it != dict.end()) return true;
    dict[name] = e;
    return false;
  }

  void Move(const std::string &oldname, const std::string &newname) {
    auto it = dict.find(oldname);
    if (it != dict.end()) {
      auto obj = it->second;
      dict.erase(it);
      dict[newname] = obj;
    } else {
      assert(false);
    }
  }

  T *Lookup(const std::string &name) const {
    auto it = dict.find(name);
    return it == dict.end() ? nullptr : it->second;
  }

 public:
  std::map<std::string, T *> dict;      // quick lookup
  std::vector<T *> vec;  // Used to iterate in order of insertion
};

// A name space, as set in the schema.
struct Namespace {
  std::vector<std::string> components;

  // Given a (potentally unqualified) name, return the "fully qualified" name
  // which has a full namespaced descriptor.
  // With max_components you can request less than the number of components
  // the current namespace has.
  std::string GetFullyQualifiedName(const std::string &name,
                                    size_t max_components = 1000) const;
};

// Base class for all definition types (fields, structs_, enums_).
struct Definition {
  Definition() : generated(false), defined_namespace(nullptr),
                 serialized_location(0), index(-1) {}

  flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<
    reflection::KeyValue>>>
      SerializeAttributes(FlatBufferBuilder *builder,
                          const Parser &parser) const;

  std::string name;
  std::string file;
  std::vector<std::string> doc_comment;
  SymbolTable<Value> attributes;
  bool generated;  // did we already output code for this definition?
  Namespace *defined_namespace;  // Where it was defined.

  // For use with Serialize()
  uoffset_t serialized_location;
  int index;  // Inside the vector it is stored.
};

struct FieldDef : public Definition {
  FieldDef() : deprecated(false), required(false), key(false), padding(0) {}

  Offset<reflection::Field> Serialize(FlatBufferBuilder *builder, uint16_t id,
                                      const Parser &parser) const;

  Value value;
  bool deprecated; // Field is allowed to be present in old data, but can't be
                   // written in new data nor accessed in new code.
  bool required;   // Field must always be present.
  bool key;        // Field functions as a key for creating sorted vectors.
  size_t padding;  // Bytes to always pad after this field.
};

struct StructDef : public Definition {
  StructDef()
    : fixed(false),
      predecl(true),
      sortbysize(true),
      has_key(false),
      minalign(1),
      bytesize(0)
    {}

  void PadLastField(size_t min_align) {
    auto padding = PaddingBytes(bytesize, min_align);
    bytesize += padding;
    if (fields.vec.size()) fields.vec.back()->padding = padding;
  }

  Offset<reflection::Object> Serialize(FlatBufferBuilder *builder,
                                       const Parser &parser) const;

  SymbolTable<FieldDef> fields;
  bool fixed;       // If it's struct, not a table.
  bool predecl;     // If it's used before it was defined.
  bool sortbysize;  // Whether fields come in the declaration or size order.
  bool has_key;     // It has a key field.
  size_t minalign;  // What the whole object needs to be aligned to.
  size_t bytesize;  // Size if fixed.
};

inline bool IsStruct(const Type &type) {
  return type.base_type == BASE_TYPE_STRUCT && type.struct_def->fixed;
}

inline size_t InlineSize(const Type &type) {
  return IsStruct(type) ? type.struct_def->bytesize : SizeOf(type.base_type);
}

inline size_t InlineAlignment(const Type &type) {
  return IsStruct(type) ? type.struct_def->minalign : SizeOf(type.base_type);
}

struct EnumVal {
  EnumVal(const std::string &_name, int64_t _val)
    : name(_name), value(_val), struct_def(nullptr) {}

  Offset<reflection::EnumVal> Serialize(FlatBufferBuilder *builder) const;

  std::string name;
  std::vector<std::string> doc_comment;
  int64_t value;
  StructDef *struct_def;  // only set if this is a union
};

struct EnumDef : public Definition {
  EnumDef() : is_union(false) {}

  EnumVal *ReverseLookup(int enum_idx, bool skip_union_default = true) {
    for (auto it = vals.vec.begin() + static_cast<int>(is_union &&
                                                       skip_union_default);
             it != vals.vec.end(); ++it) {
      if ((*it)->value == enum_idx) {
        return *it;
      }
    }
    return nullptr;
  }

  Offset<reflection::Enum> Serialize(FlatBufferBuilder *builder,
                                     const Parser &parser) const;

  SymbolTable<EnumVal> vals;
  bool is_union;
  Type underlying_type;
};

struct RPCCall {
  std::string name;
  SymbolTable<Value> attributes;
  StructDef *request, *response;
};

struct ServiceDef : public Definition {
  SymbolTable<RPCCall> calls;
};

// Container of options that may apply to any of the source/text generators.
struct IDLOptions {
  bool strict_json;
  bool skip_js_exports;
  bool output_default_scalars_in_json;
  int indent_step;
  bool output_enum_identifiers;
  bool prefixed_enums;
  bool scoped_enums;
  bool include_dependence_headers;
  bool mutable_buffer;
  bool one_file;
  bool proto_mode;
  bool generate_all;
  bool skip_unexpected_fields_in_json;
  bool generate_name_strings;

  // Possible options for the more general generator below.
  enum Language { kJava, kCSharp, kGo, kMAX };

  Language lang;

  IDLOptions()
    : strict_json(false),
      skip_js_exports(false),
      output_default_scalars_in_json(false),
      indent_step(2),
      output_enum_identifiers(true), prefixed_enums(true), scoped_enums(false),
      include_dependence_headers(true),
      mutable_buffer(false),
      one_file(false),
      proto_mode(false),
      generate_all(false),
      skip_unexpected_fields_in_json(false),
      generate_name_strings(false),
      lang(IDLOptions::kJava) {}
};

// A way to make error propagation less error prone by requiring values to be
// checked.
// Once you create a value of this type you must either:
// - Call Check() on it.
// - Copy or assign it to another value.
// Failure to do so leads to an assert.
// This guarantees that this as return value cannot be ignored.
class CheckedError {
 public:
  explicit CheckedError(bool error)
    : is_error_(error), has_been_checked_(false) {}

  CheckedError &operator=(const CheckedError &other) {
    is_error_ = other.is_error_;
    has_been_checked_ = false;
    other.has_been_checked_ = true;
    return *this;
  }

  CheckedError(const CheckedError &other) {
    *this = other;  // Use assignment operator.
  }

  ~CheckedError() { assert(has_been_checked_); }

  bool Check() { has_been_checked_ = true; return is_error_; }

 private:
  bool is_error_;
  mutable bool has_been_checked_;
};

// Additionally, in GCC we can get these errors statically, for additional
// assurance:
#ifdef __GNUC__
#define FLATBUFFERS_CHECKED_ERROR CheckedError \
          __attribute__((warn_unused_result))
#else
#define FLATBUFFERS_CHECKED_ERROR CheckedError
#endif

class Parser {
 public:
  explicit Parser(const IDLOptions &options = IDLOptions())
    : root_struct_def_(nullptr),
      opts(options),
      source_(nullptr),
      cursor_(nullptr),
      line_(1),
      anonymous_counter(0) {
    // Just in case none are declared:
    namespaces_.push_back(new Namespace());
    known_attributes_["deprecated"] = true;
    known_attributes_["required"] = true;
    known_attributes_["key"] = true;
    known_attributes_["hash"] = true;
    known_attributes_["id"] = true;
    known_attributes_["force_align"] = true;
    known_attributes_["bit_flags"] = true;
    known_attributes_["original_order"] = true;
    known_attributes_["nested_flatbuffer"] = true;
    known_attributes_["csharp_partial"] = true;
    known_attributes_["stream"] = true;
    known_attributes_["idempotent"] = true;
  }

  ~Parser() {
    for (auto it = namespaces_.begin(); it != namespaces_.end(); ++it) {
      delete *it;
    }
  }

  // Parse the string containing either schema or JSON data, which will
  // populate the SymbolTable's or the FlatBufferBuilder above.
  // include_paths is used to resolve any include statements, and typically
  // should at least include the project path (where you loaded source_ from).
  // include_paths must be nullptr terminated if specified.
  // If include_paths is nullptr, it will attempt to load from the current
  // directory.
  // If the source was loaded from a file and isn't an include file,
  // supply its name in source_filename.
  bool Parse(const char *_source, const char **include_paths = nullptr,
             const char *source_filename = nullptr);

  // Set the root type. May override the one set in the schema.
  bool SetRootType(const char *name);

  // Mark all definitions as already having code generated.
  void MarkGenerated();

  // Get the files recursively included by the given file. The returned
  // container will have at least the given file.
  std::set<std::string> GetIncludedFilesRecursive(
      const std::string &file_name) const;

  // Fills builder_ with a binary version of the schema parsed.
  // See reflection/reflection.fbs
  void Serialize();

  FLATBUFFERS_CHECKED_ERROR CheckBitsFit(int64_t val, size_t bits);

private:
  FLATBUFFERS_CHECKED_ERROR Error(const std::string &msg);
  FLATBUFFERS_CHECKED_ERROR ParseHexNum(int nibbles, int64_t *val);
  FLATBUFFERS_CHECKED_ERROR Next();
  FLATBUFFERS_CHECKED_ERROR SkipByteOrderMark();
  bool Is(int t);
  FLATBUFFERS_CHECKED_ERROR Expect(int t);
  std::string TokenToStringId(int t);
  EnumDef *LookupEnum(const std::string &id);
  FLATBUFFERS_CHECKED_ERROR ParseNamespacing(std::string *id,
                                             std::string *last);
  FLATBUFFERS_CHECKED_ERROR ParseTypeIdent(Type &type);
  FLATBUFFERS_CHECKED_ERROR ParseType(Type &type);
  FLATBUFFERS_CHECKED_ERROR AddField(StructDef &struct_def,
                                     const std::string &name, const Type &type,
                                     FieldDef **dest);
  FLATBUFFERS_CHECKED_ERROR ParseField(StructDef &struct_def);
  FLATBUFFERS_CHECKED_ERROR ParseAnyValue(Value &val, FieldDef *field,
                                          size_t parent_fieldn);
  FLATBUFFERS_CHECKED_ERROR ParseTable(const StructDef &struct_def,
                                       std::string *value, uoffset_t *ovalue);
  void SerializeStruct(const StructDef &struct_def, const Value &val);
  void AddVector(bool sortbysize, int count);
  FLATBUFFERS_CHECKED_ERROR ParseVector(const Type &type, uoffset_t *ovalue);
  FLATBUFFERS_CHECKED_ERROR ParseMetaData(SymbolTable<Value> *attributes);
  FLATBUFFERS_CHECKED_ERROR TryTypedValue(int dtoken, bool check, Value &e,
                                          BaseType req, bool *destmatch);
  FLATBUFFERS_CHECKED_ERROR ParseHash(Value &e, FieldDef* field);
  FLATBUFFERS_CHECKED_ERROR ParseSingleValue(Value &e);
  FLATBUFFERS_CHECKED_ERROR ParseEnumFromString(Type &type, int64_t *result);
  StructDef *LookupCreateStruct(const std::string &name,
                                bool create_if_new = true,
                                bool definition = false);
  FLATBUFFERS_CHECKED_ERROR ParseEnum(bool is_union, EnumDef **dest);
  FLATBUFFERS_CHECKED_ERROR ParseNamespace();
  FLATBUFFERS_CHECKED_ERROR StartStruct(const std::string &name,
                                        StructDef **dest);
  FLATBUFFERS_CHECKED_ERROR ParseDecl();
  FLATBUFFERS_CHECKED_ERROR ParseService();
  FLATBUFFERS_CHECKED_ERROR ParseProtoFields(StructDef *struct_def,
                                             bool isextend, bool inside_oneof);
  FLATBUFFERS_CHECKED_ERROR ParseProtoOption();
  FLATBUFFERS_CHECKED_ERROR ParseProtoKey();
  FLATBUFFERS_CHECKED_ERROR ParseProtoDecl();
  FLATBUFFERS_CHECKED_ERROR ParseProtoCurliesOrIdent();
  FLATBUFFERS_CHECKED_ERROR ParseTypeFromProtoType(Type *type);
  FLATBUFFERS_CHECKED_ERROR SkipAnyJsonValue();
  FLATBUFFERS_CHECKED_ERROR SkipJsonObject();
  FLATBUFFERS_CHECKED_ERROR SkipJsonArray();
  FLATBUFFERS_CHECKED_ERROR SkipJsonString();
  FLATBUFFERS_CHECKED_ERROR DoParse(const char *_source,
                                    const char **include_paths,
                                    const char *source_filename);
  FLATBUFFERS_CHECKED_ERROR CheckClash(std::vector<FieldDef*> &fields,
                                       StructDef *struct_def,
                                       const char *suffix,
                                       BaseType baseType);

 public:
  SymbolTable<StructDef> structs_;
  SymbolTable<EnumDef> enums_;
  SymbolTable<ServiceDef> services_;
  std::vector<Namespace *> namespaces_;
  std::string error_;         // User readable error_ if Parse() == false

  FlatBufferBuilder builder_;  // any data contained in the file
  StructDef *root_struct_def_;
  std::string file_identifier_;
  std::string file_extension_;

  std::map<std::string, bool> included_files_;
  std::map<std::string, std::set<std::string>> files_included_per_file_;

  std::map<std::string, bool> known_attributes_;

  IDLOptions opts;

 private:
  const char *source_, *cursor_;
  int line_;  // the current line being parsed
  int token_;
  std::string file_being_parsed_;

  std::string attribute_;
  std::vector<std::string> doc_comment_;

  std::vector<std::pair<Value, FieldDef *>> field_stack_;

  int anonymous_counter;
};

// Utility functions for multiple generators:

extern std::string MakeCamel(const std::string &in, bool first = true);

struct CommentConfig;

extern void GenComment(const std::vector<std::string> &dc,
                       std::string *code_ptr,
                       const CommentConfig *config,
                       const char *prefix = "");

// Generate text (JSON) from a given FlatBuffer, and a given Parser
// object that has been populated with the corresponding schema.
// If ident_step is 0, no indentation will be generated. Additionally,
// if it is less than 0, no linefeeds will be generated either.
// See idl_gen_text.cpp.
// strict_json adds "quotes" around field names if true.
extern void GenerateText(const Parser &parser,
                         const void *flatbuffer,
                         std::string *text);
extern bool GenerateTextFile(const Parser &parser,
                             const std::string &path,
                             const std::string &file_name);

// Generate binary files from a given FlatBuffer, and a given Parser
// object that has been populated with the corresponding schema.
// See idl_gen_general.cpp.
extern bool GenerateBinary(const Parser &parser,
                           const std::string &path,
                           const std::string &file_name);

// Generate a C++ header from the definitions in the Parser object.
// See idl_gen_cpp.
extern std::string GenerateCPP(const Parser &parser,
                               const std::string &include_guard_ident);
extern bool GenerateCPP(const Parser &parser,
                        const std::string &path,
                        const std::string &file_name);

// Generate JavaScript code from the definitions in the Parser object.
// See idl_gen_js.
extern std::string GenerateJS(const Parser &parser);
extern bool GenerateJS(const Parser &parser,
                       const std::string &path,
                       const std::string &file_name);

// Generate Go files from the definitions in the Parser object.
// See idl_gen_go.cpp.
extern bool GenerateGo(const Parser &parser,
                       const std::string &path,
                       const std::string &file_name);

// Generate Java files from the definitions in the Parser object.
// See idl_gen_java.cpp.
extern bool GenerateJava(const Parser &parser,
                         const std::string &path,
                         const std::string &file_name);

// Generate Php code from the definitions in the Parser object.
// See idl_gen_php.
extern bool GeneratePhp(const Parser &parser,
       const std::string &path,
       const std::string &file_name);

// Generate Python files from the definitions in the Parser object.
// See idl_gen_python.cpp.
extern bool GeneratePython(const Parser &parser,
                           const std::string &path,
                           const std::string &file_name);

// Generate C# files from the definitions in the Parser object.
// See idl_gen_csharp.cpp.
extern bool GenerateCSharp(const Parser &parser,
                           const std::string &path,
                           const std::string &file_name);

// Generate Java/C#/.. files from the definitions in the Parser object.
// See idl_gen_general.cpp.
extern bool GenerateGeneral(const Parser &parser,
                            const std::string &path,
                            const std::string &file_name);

// Generate a schema file from the internal representation, useful after
// parsing a .proto schema.
extern std::string GenerateFBS(const Parser &parser,
                               const std::string &file_name);
extern bool GenerateFBS(const Parser &parser,
                        const std::string &path,
                        const std::string &file_name);

// Generate a make rule for the generated JavaScript code.
// See idl_gen_js.cpp.
extern std::string JSMakeRule(const Parser &parser,
                              const std::string &path,
                              const std::string &file_name);

// Generate a make rule for the generated C++ header.
// See idl_gen_cpp.cpp.
extern std::string CPPMakeRule(const Parser &parser,
                               const std::string &path,
                               const std::string &file_name);

// Generate a make rule for the generated Java/C#/... files.
// See idl_gen_general.cpp.
extern std::string GeneralMakeRule(const Parser &parser,
                                   const std::string &path,
                                   const std::string &file_name);

// Generate a make rule for the generated text (JSON) files.
// See idl_gen_text.cpp.
extern std::string TextMakeRule(const Parser &parser,
                                const std::string &path,
                                const std::string &file_names);

// Generate a make rule for the generated binary files.
// See idl_gen_general.cpp.
extern std::string BinaryMakeRule(const Parser &parser,
                                  const std::string &path,
                                  const std::string &file_name);

}  // namespace flatbuffers

#endif  // FLATBUFFERS_IDL_H_

