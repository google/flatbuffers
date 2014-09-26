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
#include <memory>
#include <functional>

#include "flatbuffers/flatbuffers.h"

// This file defines the data types representing a parsed IDL (Interface
// Definition Language) / schema file.

namespace flatbuffers {

// The order of these matters for Is*() functions below.
// Additionally, Parser::ParseType assumes bool..string is a contiguous range
// of type tokens.
#define FLATBUFFERS_GEN_TYPES_SCALAR(TD) \
  TD(NONE,   "",       uint8_t,  byte,   byte,    byte) \
  TD(UTYPE,  "",       uint8_t,  byte,   byte,    byte) /* begin scalar/int */ \
  TD(BOOL,   "bool",   uint8_t,  byte,   byte,    byte) \
  TD(CHAR,   "byte",   int8_t,   byte,   int8,    sbyte) \
  TD(UCHAR,  "ubyte",  uint8_t,  byte,   byte,    byte) \
  TD(SHORT,  "short",  int16_t,  short,  int16,   short) \
  TD(USHORT, "ushort", uint16_t, short,  uint16,  ushort) \
  TD(INT,    "int",    int32_t,  int,    int32,   int) \
  TD(UINT,   "uint",   uint32_t, int,    uint32,  uint) \
  TD(LONG,   "long",   int64_t,  long,   int64,   long) \
  TD(ULONG,  "ulong",  uint64_t, long,   uint64,  ulong)  /* end int */ \
  TD(FLOAT,  "float",  float,    float,  float32, float)  /* begin float */ \
  TD(DOUBLE, "double", double,   double, float64, double) /* end float/scalar */
#define FLATBUFFERS_GEN_TYPES_POINTER(TD) \
  TD(STRING, "string", Offset<void>, int, int, int) \
  TD(VECTOR, "",       Offset<void>, int, int, int) \
  TD(STRUCT, "",       Offset<void>, int, int, int) \
  TD(UNION,  "",       Offset<void>, int, int, int)

// The fields are:
// - enum
// - FlatBuffers schema type.
// - C++ type.
// - Java type.
// - Go type.
// - C# / .Net type.

// using these macros, we can now write code dealing with types just once, e.g.

/*
switch (type) {
  #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE) \
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
  #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE) \
      BASE_TYPE_ ## ENUM,
    FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
  #undef FLATBUFFERS_TD
};

#define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE) \
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

  Type VectorType() const { return Type(element, struct_def, enum_def); }

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

  T *Lookup(const std::string &name) const {
    auto it = dict.find(name);
    return it == dict.end() ? nullptr : it->second;
  }

 private:
  std::map<std::string, T *> dict;      // quick lookup

 public:
  std::vector<T *> vec;  // Used to iterate in order of insertion
};

// A name space, as set in the schema.
struct Namespace {
  std::vector<std::string> components;
};

// Base class for all definition types (fields, structs_, enums_).
struct Definition {
  Definition() : generated(false), defined_namespace(nullptr) {}

  std::string name;
  std::vector<std::string> doc_comment;
  SymbolTable<Value> attributes;
  bool generated;  // did we already output code for this definition?
  Namespace *defined_namespace;  // Where it was defined.
};

struct FieldDef : public Definition {
  FieldDef() : deprecated(false), required(false), padding(0), used(false) {}

  Value value;
  bool deprecated;
  bool required;
  size_t padding;  // Bytes to always pad after this field.
  bool used;       // Used during JSON parsing to check for repeated fields.
};

struct StructDef : public Definition {
  StructDef()
    : fixed(false),
      predecl(true),
      sortbysize(true),
      minalign(1),
      bytesize(0)
    {}

  void PadLastField(size_t minalign) {
    auto padding = PaddingBytes(bytesize, minalign);
    bytesize += padding;
    if (fields.vec.size()) fields.vec.back()->padding = padding;
  }

  SymbolTable<FieldDef> fields;
  bool fixed;       // If it's struct, not a table.
  bool predecl;     // If it's used before it was defined.
  bool sortbysize;  // Whether fields come in the declaration or size order.
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

  SymbolTable<EnumVal> vals;
  bool is_union;
  Type underlying_type;
};

class Parser {
 public:
  Parser(bool proto_mode = false) :
    root_struct_def(nullptr),
    source_(nullptr),
    cursor_(nullptr),
    line_(1),
    proto_mode_(proto_mode) {
      // Just in case none are declared:
      namespaces_.push_back(new Namespace());
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

 private:
  int64_t ParseHexNum(int nibbles);
  void Next();
  bool IsNext(int t);
  void Expect(int t);
  void ParseTypeIdent(Type &type);
  void ParseType(Type &type);
  FieldDef &AddField(StructDef &struct_def,
                     const std::string &name,
                     const Type &type);
  void ParseField(StructDef &struct_def);
  void ParseAnyValue(Value &val, FieldDef *field);
  uoffset_t ParseTable(const StructDef &struct_def);
  void SerializeStruct(const StructDef &struct_def, const Value &val);
  void AddVector(bool sortbysize, int count);
  uoffset_t ParseVector(const Type &type);
  void ParseMetaData(Definition &def);
  bool TryTypedValue(int dtoken, bool check, Value &e, BaseType req);
  void ParseSingleValue(Value &e);
  int64_t ParseIntegerFromString(Type &type);
  StructDef *LookupCreateStruct(const std::string &name);
  void ParseEnum(bool is_union);
  void ParseNamespace();
  StructDef &StartStruct();
  void ParseDecl();
  void ParseProtoDecl();
  Type ParseTypeFromProtoType();

 public:
  SymbolTable<StructDef> structs_;
  SymbolTable<EnumDef> enums_;
  std::vector<Namespace *> namespaces_;
  std::string error_;         // User readable error_ if Parse() == false

  FlatBufferBuilder builder_;  // any data contained in the file
  StructDef *root_struct_def;
  std::string file_identifier_;
  std::string file_extension_;

  std::map<std::string, bool> included_files_;

 private:
  const char *source_, *cursor_;
  int line_;  // the current line being parsed
  int token_;
  bool proto_mode_;
  std::string attribute_;
  std::vector<std::string> doc_comment_;

  std::vector<std::pair<Value, FieldDef *>> field_stack_;
  std::vector<uint8_t> struct_stack_;
};

// Utility functions for multiple generators:

extern std::string MakeCamel(const std::string &in, bool first = true);
extern void GenComment(const std::vector<std::string> &dc,
                       std::string *code_ptr,
                       const char *prefix = "");

// Container of options that may apply to any of the source/text generators.
struct GeneratorOptions {
  bool strict_json;
  int indent_step;
  bool output_enum_identifiers;
  bool prefixed_enums;
  bool include_dependence_headers;

  // Possible options for the more general generator below.
  enum Language { kJava, kCSharp, kMAX };

  Language lang;

  GeneratorOptions() : strict_json(false), indent_step(2),
                       output_enum_identifiers(true), prefixed_enums(true),
                       include_dependence_headers(false),
                       lang(GeneratorOptions::kJava) {}
};

// Generate text (JSON) from a given FlatBuffer, and a given Parser
// object that has been populated with the corresponding schema.
// If ident_step is 0, no indentation will be generated. Additionally,
// if it is less than 0, no linefeeds will be generated either.
// See idl_gen_text.cpp.
// strict_json adds "quotes" around field names if true.
extern void GenerateText(const Parser &parser,
                         const void *flatbuffer,
                         const GeneratorOptions &opts,
                         std::string *text);

// Generate a C++ header from the definitions in the Parser object.
// See idl_gen_cpp.
extern std::string GenerateCPP(const Parser &parser,
                               const std::string &include_guard_ident,
                               const GeneratorOptions &opts);
extern bool GenerateCPP(const Parser &parser,
                        const std::string &path,
                        const std::string &file_name,
                        const GeneratorOptions &opts);

// Generate Go files from the definitions in the Parser object.
// See idl_gen_go.cpp.
extern bool GenerateGo(const Parser &parser,
                       const std::string &path,
                       const std::string &file_name,
                       const GeneratorOptions &opts);

// Generate Java files from the definitions in the Parser object.
// See idl_gen_java.cpp.
extern bool GenerateJava(const Parser &parser,
                         const std::string &path,
                         const std::string &file_name,
                         const GeneratorOptions &opts);

// Generate C# files from the definitions in the Parser object.
// See idl_gen_csharp.cpp.
extern bool GenerateCSharp(const Parser &parser,
                           const std::string &path,
                           const std::string &file_name,
                           const GeneratorOptions &opts);

// Generate Java/C#/.. files from the definitions in the Parser object.
// See idl_gen_general.cpp.
extern bool GenerateGeneral(const Parser &parser,
                            const std::string &path,
                            const std::string &file_name,
                            const GeneratorOptions &opts);

// Generate a schema file from the internal representation, useful after
// parsing a .proto schema.
extern std::string GenerateFBS(const Parser &parser,
                               const std::string &file_name,
                               const GeneratorOptions &opts);
extern bool GenerateFBS(const Parser &parser,
                        const std::string &path,
                        const std::string &file_name,
                        const GeneratorOptions &opts);

}  // namespace flatbuffers

#endif  // FLATBUFFERS_IDL_H_

