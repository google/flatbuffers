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

#include <algorithm>
#include <list>

#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

const char *const kTypeNames[] = {
  #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE) \
    IDLTYPE,
    FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
  #undef FLATBUFFERS_TD
  nullptr
};

const char kTypeSizes[] = {
  #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE) \
      sizeof(CTYPE),
    FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
  #undef FLATBUFFERS_TD
};

// The enums in the reflection schema should match the ones we use internally.
// Compare the last element to check if these go out of sync.
static_assert(BASE_TYPE_UNION ==
              static_cast<BaseType>(reflection::Union),
              "enums don't match");

// Any parsing calls have to be wrapped in this macro, which automates
// handling of recursive error checking a bit. It will check the received
// CheckedError object, and return straight away on error.
#define ECHECK(call) { auto ce = (call); if (ce.Check()) return ce; }

// These two functions are called hundreds of times below, so define a short
// form:
#define NEXT() ECHECK(Next())
#define EXPECT(tok) ECHECK(Expect(tok))

CheckedError Parser::Error(const std::string &msg) {
  error_ = file_being_parsed_.length() ? AbsolutePath(file_being_parsed_) : "";
  #ifdef _WIN32
    error_ += "(" + NumToString(line_) + ")";  // MSVC alike
  #else
    if (file_being_parsed_.length()) error_ += ":";
    error_ += NumToString(line_) + ":0";  // gcc alike
  #endif
  error_ += ": error: " + msg;
  return CheckedError(true);
}

inline CheckedError NoError() { return CheckedError(false); }

// Ensure that integer values we parse fit inside the declared integer type.
CheckedError Parser::CheckBitsFit(int64_t val, size_t bits) {
  // Left-shifting a 64-bit value by 64 bits or more is undefined
  // behavior (C99 6.5.7), so check *before* we shift.
  if (bits < 64) {
    // Bits we allow to be used.
    auto mask = static_cast<int64_t>((1ull << bits) - 1);
    if ((val & ~mask) != 0 &&  // Positive or unsigned.
        (val |  mask) != -1)   // Negative.
      return Error("constant does not fit in a " + NumToString(bits) +
                   "-bit field");
  }
  return NoError();
}

// atot: templated version of atoi/atof: convert a string to an instance of T.
template<typename T> inline CheckedError atot(const char *s, Parser &parser,
                                              T *val) {
  int64_t i = StringToInt(s);
  ECHECK(parser.CheckBitsFit(i, sizeof(T) * 8));
  *val = (T)i;
  return NoError();
}
template<> inline CheckedError atot<bool>(const char *s, Parser &parser,
                                          bool *val) {
  (void)parser;
  *val = 0 != atoi(s);
  return NoError();
}
template<> inline CheckedError atot<float>(const char *s, Parser &parser,
                                           float *val) {
  (void)parser;
  *val = static_cast<float>(strtod(s, nullptr));
  return NoError();
}
template<> inline CheckedError atot<double>(const char *s, Parser &parser,
                                            double *val) {
  (void)parser;
  *val = strtod(s, nullptr);
  return NoError();
}

template<> inline CheckedError atot<Offset<void>>(const char *s, Parser &parser,
                                                  Offset<void> *val) {
  (void)parser;
  *val = Offset<void>(atoi(s));
  return NoError();
}

std::string Namespace::GetFullyQualifiedName(const std::string &name,
                                             size_t max_components) const {
  // Early exit if we don't have a defined namespace.
  if (components.size() == 0 || !max_components) {
    return name;
  }
  std::stringstream stream;
  for (size_t i = 0; i < std::min(components.size(), max_components);
       i++) {
    if (i) {
      stream << ".";
    }
    stream << components[i];
  }

  stream << "." << name;
  return stream.str();
}



// Declare tokens we'll use. Single character tokens are represented by their
// ascii character code (e.g. '{'), others above 256.
#define FLATBUFFERS_GEN_TOKENS(TD) \
  TD(Eof, 256, "end of file") \
  TD(StringConstant, 257, "string constant") \
  TD(IntegerConstant, 258, "integer constant") \
  TD(FloatConstant, 259, "float constant") \
  TD(Identifier, 260, "identifier") \
  TD(Table, 261, "table") \
  TD(Struct, 262, "struct") \
  TD(Enum, 263, "enum") \
  TD(Union, 264, "union") \
  TD(NameSpace, 265, "namespace") \
  TD(RootType, 266, "root_type") \
  TD(FileIdentifier, 267, "file_identifier") \
  TD(FileExtension, 268, "file_extension") \
  TD(Include, 269, "include") \
  TD(Attribute, 270, "attribute") \
  TD(Null, 271, "null") \
  TD(Service, 272, "rpc_service")
#ifdef __GNUC__
__extension__  // Stop GCC complaining about trailing comma with -Wpendantic.
#endif
enum {
  #define FLATBUFFERS_TOKEN(NAME, VALUE, STRING) kToken ## NAME = VALUE,
    FLATBUFFERS_GEN_TOKENS(FLATBUFFERS_TOKEN)
  #undef FLATBUFFERS_TOKEN
  #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE) \
      kToken ## ENUM,
    FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
  #undef FLATBUFFERS_TD
};

static std::string TokenToString(int t) {
  static const char *tokens[] = {
    #define FLATBUFFERS_TOKEN(NAME, VALUE, STRING) STRING,
      FLATBUFFERS_GEN_TOKENS(FLATBUFFERS_TOKEN)
    #undef FLATBUFFERS_TOKEN
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE) \
      IDLTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };
  if (t < 256) {  // A single ascii char token.
    std::string s;
    s.append(1, static_cast<char>(t));
    return s;
  } else {       // Other tokens.
    return tokens[t - 256];
  }
}

std::string Parser::TokenToStringId(int t) {
  return TokenToString(t) + (t == kTokenIdentifier ? ": " + attribute_ : "");
}

// Parses exactly nibbles worth of hex digits into a number, or error.
CheckedError Parser::ParseHexNum(int nibbles, int64_t *val) {
  for (int i = 0; i < nibbles; i++)
    if (!isxdigit(static_cast<const unsigned char>(cursor_[i])))
      return Error("escape code must be followed by " + NumToString(nibbles) +
                   " hex digits");
  std::string target(cursor_, cursor_ + nibbles);
  *val = StringToUInt(target.c_str(), 16);
  cursor_ += nibbles;
  return NoError();
}

CheckedError Parser::SkipByteOrderMark() {
  if (static_cast<unsigned char>(*cursor_) != 0xef) return NoError();
  cursor_++;
  if (static_cast<unsigned char>(*cursor_) != 0xbb) return Error("invalid utf-8 byte order mark");
  cursor_++;
  if (static_cast<unsigned char>(*cursor_) != 0xbf) return Error("invalid utf-8 byte order mark");
  cursor_++;
  return NoError();
}

bool IsIdentifierStart(char c) {
  return isalpha(static_cast<unsigned char>(c)) || c == '_';
}

CheckedError Parser::Next() {
  doc_comment_.clear();
  bool seen_newline = false;
  attribute_.clear();
  for (;;) {
    char c = *cursor_++;
    token_ = c;
    switch (c) {
      case '\0': cursor_--; token_ = kTokenEof; return NoError();
      case ' ': case '\r': case '\t': break;
      case '\n': line_++; seen_newline = true; break;
      case '{': case '}': case '(': case ')': case '[': case ']':
      case ',': case ':': case ';': case '=': return NoError();
      case '.':
        if(!isdigit(static_cast<const unsigned char>(*cursor_))) return NoError();
        return Error("floating point constant can\'t start with \".\"");
      case '\"':
      case '\'': {
        int unicode_high_surrogate = -1;

        while (*cursor_ != c) {
          if (*cursor_ < ' ' && *cursor_ >= 0)
            return Error("illegal character in string constant");
          if (*cursor_ == '\\') {
            cursor_++;
            if (unicode_high_surrogate != -1 &&
                *cursor_ != 'u') {
              return Error(
                "illegal Unicode sequence (unpaired high surrogate)");
            }
            switch (*cursor_) {
              case 'n':  attribute_ += '\n'; cursor_++; break;
              case 't':  attribute_ += '\t'; cursor_++; break;
              case 'r':  attribute_ += '\r'; cursor_++; break;
              case 'b':  attribute_ += '\b'; cursor_++; break;
              case 'f':  attribute_ += '\f'; cursor_++; break;
              case '\"': attribute_ += '\"'; cursor_++; break;
              case '\'': attribute_ += '\''; cursor_++; break;
              case '\\': attribute_ += '\\'; cursor_++; break;
              case '/':  attribute_ += '/';  cursor_++; break;
              case 'x': {  // Not in the JSON standard
                cursor_++;
                int64_t val;
                ECHECK(ParseHexNum(2, &val));
                attribute_ += static_cast<char>(val);
                break;
              }
              case 'u': {
                cursor_++;
                int64_t val;
                ECHECK(ParseHexNum(4, &val));
                if (val >= 0xD800 && val <= 0xDBFF) {
                  if (unicode_high_surrogate != -1) {
                    return Error(
                      "illegal Unicode sequence (multiple high surrogates)");
                  } else {
                    unicode_high_surrogate = static_cast<int>(val);
                  }
                } else if (val >= 0xDC00 && val <= 0xDFFF) {
                  if (unicode_high_surrogate == -1) {
                    return Error(
                      "illegal Unicode sequence (unpaired low surrogate)");
                  } else {
                    int code_point = 0x10000 +
                      ((unicode_high_surrogate & 0x03FF) << 10) +
                      (val & 0x03FF);
                    ToUTF8(code_point, &attribute_);
                    unicode_high_surrogate = -1;
                  }
                } else {
                  if (unicode_high_surrogate != -1) {
                    return Error(
                      "illegal Unicode sequence (unpaired high surrogate)");
                  }
                  ToUTF8(static_cast<int>(val), &attribute_);
                }
                break;
              }
              default: return Error("unknown escape code in string constant");
            }
          } else { // printable chars + UTF-8 bytes
            if (unicode_high_surrogate != -1) {
              return Error(
                "illegal Unicode sequence (unpaired high surrogate)");
            }
            attribute_ += *cursor_++;
          }
        }
        if (unicode_high_surrogate != -1) {
          return Error(
            "illegal Unicode sequence (unpaired high surrogate)");
        }
        cursor_++;
        token_ = kTokenStringConstant;
        return NoError();
      }
      case '/':
        if (*cursor_ == '/') {
          const char *start = ++cursor_;
          while (*cursor_ && *cursor_ != '\n' && *cursor_ != '\r') cursor_++;
          if (*start == '/') {  // documentation comment
            if (cursor_ != source_ && !seen_newline)
              return Error(
                    "a documentation comment should be on a line on its own");
            doc_comment_.push_back(std::string(start + 1, cursor_));
          }
          break;
        } else if (*cursor_ == '*') {
          cursor_++;
          // TODO: make nested.
          while (*cursor_ != '*' || cursor_[1] != '/') {
            if (!*cursor_) return Error("end of file in comment");
            cursor_++;
          }
          cursor_ += 2;
          break;
        }
        // fall thru
      default:
        if (IsIdentifierStart(c)) {
          // Collect all chars of an identifier:
          const char *start = cursor_ - 1;
          while (isalnum(static_cast<unsigned char>(*cursor_)) ||
                 *cursor_ == '_')
            cursor_++;
          attribute_.append(start, cursor_);
          // First, see if it is a type keyword from the table of types:
          #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, \
            PTYPE) \
            if (attribute_ == IDLTYPE) { \
              token_ = kToken ## ENUM; \
              return NoError(); \
            }
            FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
          #undef FLATBUFFERS_TD
          // If it's a boolean constant keyword, turn those into integers,
          // which simplifies our logic downstream.
          if (attribute_ == "true" || attribute_ == "false") {
            attribute_ = NumToString(attribute_ == "true");
            token_ = kTokenIntegerConstant;
            return NoError();
          }
          // Check for declaration keywords:
          if (attribute_ == "table") {
            token_ = kTokenTable;
            return NoError();
          }
          if (attribute_ == "struct") {
            token_ = kTokenStruct;
            return NoError();
          }
          if (attribute_ == "enum") {
            token_ = kTokenEnum;
            return NoError();
          }
          if (attribute_ == "union") {
            token_ = kTokenUnion;
            return NoError();
          }
          if (attribute_ == "namespace") {
            token_ = kTokenNameSpace;
            return NoError();
          }
          if (attribute_ == "root_type") {
            token_ = kTokenRootType;
            return NoError();
          }
          if (attribute_ == "include") {
            token_ = kTokenInclude;
            return NoError();
          }
          if (attribute_ == "attribute") {
            token_ = kTokenAttribute;
            return NoError();
          }
          if (attribute_ == "file_identifier") {
            token_ = kTokenFileIdentifier;
            return NoError();
          }
          if (attribute_ == "file_extension") {
            token_ = kTokenFileExtension;
            return NoError();
          }
          if (attribute_ == "null") {
            token_ = kTokenNull;
            return NoError();
          }
          if (attribute_ == "rpc_service") {
            token_ = kTokenService;
            return NoError();
          }
          // If not, it is a user-defined identifier:
          token_ = kTokenIdentifier;
          return NoError();
        } else if (isdigit(static_cast<unsigned char>(c)) || c == '-') {
          const char *start = cursor_ - 1;
          if (c == '0' && (*cursor_ == 'x' || *cursor_ == 'X')) {
              cursor_++;
              while (isxdigit(static_cast<unsigned char>(*cursor_))) cursor_++;
              attribute_.append(start + 2, cursor_);
              attribute_ = NumToString(StringToUInt(attribute_.c_str(), 16));
              token_ = kTokenIntegerConstant;
              return NoError();
          }
          while (isdigit(static_cast<unsigned char>(*cursor_))) cursor_++;
          if (*cursor_ == '.' || *cursor_ == 'e' || *cursor_ == 'E') {
            if (*cursor_ == '.') {
              cursor_++;
              while (isdigit(static_cast<unsigned char>(*cursor_))) cursor_++;
            }
            // See if this float has a scientific notation suffix. Both JSON
            // and C++ (through strtod() we use) have the same format:
            if (*cursor_ == 'e' || *cursor_ == 'E') {
              cursor_++;
              if (*cursor_ == '+' || *cursor_ == '-') cursor_++;
              while (isdigit(static_cast<unsigned char>(*cursor_))) cursor_++;
            }
            token_ = kTokenFloatConstant;
          } else {
            token_ = kTokenIntegerConstant;
          }
          attribute_.append(start, cursor_);
          return NoError();
        }
        std::string ch;
        ch = c;
        if (c < ' ' || c > '~') ch = "code: " + NumToString(c);
        return Error("illegal character: " + ch);
    }
  }
}

// Check if a given token is next.
bool Parser::Is(int t) {
  return t == token_;
}

// Expect a given token to be next, consume it, or error if not present.
CheckedError Parser::Expect(int t) {
  if (t != token_) {
    return Error("expecting: " + TokenToString(t) + " instead got: " +
                 TokenToStringId(token_));
  }
  NEXT();
  return NoError();
}

CheckedError Parser::ParseNamespacing(std::string *id, std::string *last) {
  while (Is('.')) {
    NEXT();
    *id += ".";
    *id += attribute_;
    if (last) *last = attribute_;
    EXPECT(kTokenIdentifier);
  }
  return NoError();
}

EnumDef *Parser::LookupEnum(const std::string &id) {
  // Search thru parent namespaces.
  for (int components = static_cast<int>(namespaces_.back()->components.size());
       components >= 0; components--) {
    auto ed = enums_.Lookup(
                namespaces_.back()->GetFullyQualifiedName(id, components));
    if (ed) return ed;
  }
  return nullptr;
}

CheckedError Parser::ParseTypeIdent(Type &type) {
  std::string id = attribute_;
  EXPECT(kTokenIdentifier);
  ECHECK(ParseNamespacing(&id, nullptr));
  auto enum_def = LookupEnum(id);
  if (enum_def) {
    type = enum_def->underlying_type;
    if (enum_def->is_union) type.base_type = BASE_TYPE_UNION;
  } else {
    type.base_type = BASE_TYPE_STRUCT;
    type.struct_def = LookupCreateStruct(id);
  }
  return NoError();
}

// Parse any IDL type.
CheckedError Parser::ParseType(Type &type) {
  if (token_ >= kTokenBOOL && token_ <= kTokenSTRING) {
    type.base_type = static_cast<BaseType>(token_ - kTokenNONE);
    NEXT();
  } else {
    if (token_ == kTokenIdentifier) {
      ECHECK(ParseTypeIdent(type));
    } else if (token_ == '[') {
      NEXT();
      Type subtype;
      ECHECK(ParseType(subtype));
      if (subtype.base_type == BASE_TYPE_VECTOR) {
        // We could support this, but it will complicate things, and it's
        // easier to work around with a struct around the inner vector.
        return Error(
              "nested vector types not supported (wrap in table first).");
      }
      if (subtype.base_type == BASE_TYPE_UNION) {
        // We could support this if we stored a struct of 2 elements per
        // union element.
        return Error(
              "vector of union types not supported (wrap in table first).");
      }
      type = Type(BASE_TYPE_VECTOR, subtype.struct_def, subtype.enum_def);
      type.element = subtype.base_type;
      EXPECT(']');
    } else {
      return Error("illegal type syntax");
    }
  }
  return NoError();
}

CheckedError Parser::AddField(StructDef &struct_def, const std::string &name,
                              const Type &type, FieldDef **dest) {
  auto &field = *new FieldDef();
  field.value.offset =
    FieldIndexToOffset(static_cast<voffset_t>(struct_def.fields.vec.size()));
  field.name = name;
  field.file = struct_def.file;
  field.value.type = type;
  if (struct_def.fixed) {  // statically compute the field offset
    auto size = InlineSize(type);
    auto alignment = InlineAlignment(type);
    // structs_ need to have a predictable format, so we need to align to
    // the largest scalar
    struct_def.minalign = std::max(struct_def.minalign, alignment);
    struct_def.PadLastField(alignment);
    field.value.offset = static_cast<voffset_t>(struct_def.bytesize);
    struct_def.bytesize += size;
  }
  if (struct_def.fields.Add(name, &field))
    return Error("field already exists: " + name);
  *dest = &field;
  return NoError();
}

CheckedError Parser::ParseField(StructDef &struct_def) {
  std::string name = attribute_;
  std::vector<std::string> dc = doc_comment_;
  EXPECT(kTokenIdentifier);
  EXPECT(':');
  Type type;
  ECHECK(ParseType(type));

  if (struct_def.fixed && !IsScalar(type.base_type) && !IsStruct(type))
    return Error("structs_ may contain only scalar or struct fields");

  FieldDef *typefield = nullptr;
  if (type.base_type == BASE_TYPE_UNION) {
    // For union fields, add a second auto-generated field to hold the type,
    // with _type appended as the name.
    ECHECK(AddField(struct_def, name + "_type", type.enum_def->underlying_type,
                    &typefield));
  }

  FieldDef *field;
  ECHECK(AddField(struct_def, name, type, &field));

  if (token_ == '=') {
    NEXT();
    if (!IsScalar(type.base_type))
      return Error("default values currently only supported for scalars");
    ECHECK(ParseSingleValue(field->value));
  }
  if (IsFloat(field->value.type.base_type)) {
    if (!strpbrk(field->value.constant.c_str(), ".eE"))
      field->value.constant += ".0";
  }

  if (type.enum_def &&
      IsScalar(type.base_type) &&
      !struct_def.fixed &&
      !type.enum_def->attributes.Lookup("bit_flags") &&
      !type.enum_def->ReverseLookup(static_cast<int>(
                         StringToInt(field->value.constant.c_str()))))
    return Error("enum " + type.enum_def->name +
          " does not have a declaration for this field\'s default of " +
          field->value.constant);

  field->doc_comment = dc;
  ECHECK(ParseMetaData(&field->attributes));
  field->deprecated = field->attributes.Lookup("deprecated") != nullptr;
  auto hash_name = field->attributes.Lookup("hash");
  if (hash_name) {
    switch (type.base_type) {
      case BASE_TYPE_INT:
      case BASE_TYPE_UINT: {
        if (FindHashFunction32(hash_name->constant.c_str()) == nullptr)
          return Error("Unknown hashing algorithm for 32 bit types: " +
                hash_name->constant);
        break;
      }
      case BASE_TYPE_LONG:
      case BASE_TYPE_ULONG: {
        if (FindHashFunction64(hash_name->constant.c_str()) == nullptr)
          return Error("Unknown hashing algorithm for 64 bit types: " +
                hash_name->constant);
        break;
      }
      default:
        return Error(
              "only int, uint, long and ulong data types support hashing.");
    }
  }
  if (field->deprecated && struct_def.fixed)
    return Error("can't deprecate fields in a struct");
  field->required = field->attributes.Lookup("required") != nullptr;
  if (field->required && (struct_def.fixed ||
                         IsScalar(field->value.type.base_type)))
    return Error("only non-scalar fields in tables may be 'required'");
  field->key = field->attributes.Lookup("key") != nullptr;
  if (field->key) {
    if (struct_def.has_key)
      return Error("only one field may be set as 'key'");
    struct_def.has_key = true;
    if (!IsScalar(field->value.type.base_type)) {
      field->required = true;
      if (field->value.type.base_type != BASE_TYPE_STRING)
        return Error("'key' field must be string or scalar type");
    }
  }
  auto nested = field->attributes.Lookup("nested_flatbuffer");
  if (nested) {
    if (nested->type.base_type != BASE_TYPE_STRING)
      return Error(
            "nested_flatbuffer attribute must be a string (the root type)");
    if (field->value.type.base_type != BASE_TYPE_VECTOR ||
        field->value.type.element != BASE_TYPE_UCHAR)
      return Error(
            "nested_flatbuffer attribute may only apply to a vector of ubyte");
    // This will cause an error if the root type of the nested flatbuffer
    // wasn't defined elsewhere.
    LookupCreateStruct(nested->constant);
  }

  if (typefield) {
    // If this field is a union, and it has a manually assigned id,
    // the automatically added type field should have an id as well (of N - 1).
    auto attr = field->attributes.Lookup("id");
    if (attr) {
      auto id = atoi(attr->constant.c_str());
      auto val = new Value();
      val->type = attr->type;
      val->constant = NumToString(id - 1);
      typefield->attributes.Add("id", val);
    }
  }

  EXPECT(';');
  return NoError();
}

CheckedError Parser::ParseAnyValue(Value &val, FieldDef *field,
                                   size_t parent_fieldn) {
  switch (val.type.base_type) {
    case BASE_TYPE_UNION: {
      assert(field);
      if (!parent_fieldn ||
          field_stack_.back().second->value.type.base_type != BASE_TYPE_UTYPE)
        return Error("missing type field before this union value: " +
                     field->name);
      uint8_t enum_idx;
      ECHECK(atot(field_stack_.back().first.constant.c_str(), *this,
                  &enum_idx));
      auto enum_val = val.type.enum_def->ReverseLookup(enum_idx);
      if (!enum_val) return Error("illegal type id for: " + field->name);
      ECHECK(ParseTable(*enum_val->struct_def, &val.constant, nullptr));
      break;
    }
    case BASE_TYPE_STRUCT:
      ECHECK(ParseTable(*val.type.struct_def, &val.constant, nullptr));
      break;
    case BASE_TYPE_STRING: {
      auto s = attribute_;
      EXPECT(kTokenStringConstant);
      val.constant = NumToString(builder_.CreateString(s).o);
      break;
    }
    case BASE_TYPE_VECTOR: {
      EXPECT('[');
      uoffset_t off;
      ECHECK(ParseVector(val.type.VectorType(), &off));
      val.constant = NumToString(off);
      break;
    }
    case BASE_TYPE_INT:
    case BASE_TYPE_UINT:
    case BASE_TYPE_LONG:
    case BASE_TYPE_ULONG: {
      if (field && field->attributes.Lookup("hash") &&
          (token_ == kTokenIdentifier || token_ == kTokenStringConstant)) {
        ECHECK(ParseHash(val, field));
      } else {
        ECHECK(ParseSingleValue(val));
      }
      break;
    }
    default:
      ECHECK(ParseSingleValue(val));
      break;
  }
  return NoError();
}

void Parser::SerializeStruct(const StructDef &struct_def, const Value &val) {
  assert(val.constant.length() == struct_def.bytesize);
  builder_.Align(struct_def.minalign);
  builder_.PushBytes(reinterpret_cast<const uint8_t *>(val.constant.c_str()),
                     struct_def.bytesize);
  builder_.AddStructOffset(val.offset, builder_.GetSize());
}

CheckedError Parser::ParseTable(const StructDef &struct_def, std::string *value,
                                uoffset_t *ovalue) {
  EXPECT('{');
  size_t fieldn = 0;
  for (;;) {
    if ((!opts.strict_json || !fieldn) && Is('}')) { NEXT(); break; }
    std::string name = attribute_;
    if (Is(kTokenStringConstant)) {
      NEXT();
    } else {
      EXPECT(opts.strict_json ? kTokenStringConstant : kTokenIdentifier);
    }
    auto field = struct_def.fields.Lookup(name);
    if (!field) {
      if (!opts.skip_unexpected_fields_in_json) {
        return Error("unknown field: " + name);
      } else {
        EXPECT(':');
        ECHECK(SkipAnyJsonValue());
      }
    } else {
      EXPECT(':');
      if (Is(kTokenNull)) {
        NEXT(); // Ignore this field.
      } else {
        Value val = field->value;
        ECHECK(ParseAnyValue(val, field, fieldn));
        size_t i = field_stack_.size();
        // Hardcoded insertion-sort with error-check.
        // If fields are specified in order, then this loop exits immediately.
        for (; i > field_stack_.size() - fieldn; i--) {
          auto existing_field = field_stack_[i - 1].second;
          if (existing_field == field)
            return Error("field set more than once: " + field->name);
          if (existing_field->value.offset < field->value.offset) break;
        }
        field_stack_.insert(field_stack_.begin() + i, std::make_pair(val, field));
        fieldn++;
      }
    }
    if (Is('}')) { NEXT(); break; }
    EXPECT(',');
  }

  if (struct_def.fixed && fieldn != struct_def.fields.vec.size())
    return Error("struct: wrong number of initializers: " + struct_def.name);

  auto start = struct_def.fixed
                 ? builder_.StartStruct(struct_def.minalign)
                 : builder_.StartTable();

  for (size_t size = struct_def.sortbysize ? sizeof(largest_scalar_t) : 1;
       size;
       size /= 2) {
    // Go through elements in reverse, since we're building the data backwards.
    for (auto it = field_stack_.rbegin();
             it != field_stack_.rbegin() + fieldn; ++it) {
      auto &field_value = it->first;
      auto field = it->second;
      if (!struct_def.sortbysize ||
          size == SizeOf(field_value.type.base_type)) {
        switch (field_value.type.base_type) {
          #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, \
            PTYPE) \
            case BASE_TYPE_ ## ENUM: \
              builder_.Pad(field->padding); \
              if (struct_def.fixed) { \
                CTYPE val; \
                ECHECK(atot(field_value.constant.c_str(), *this, &val)); \
                builder_.PushElement(val); \
              } else { \
                CTYPE val, valdef; \
                ECHECK(atot(field_value.constant.c_str(), *this, &val)); \
                ECHECK(atot(field->value.constant.c_str(), *this, &valdef)); \
                builder_.AddElement(field_value.offset, val, valdef); \
              } \
              break;
            FLATBUFFERS_GEN_TYPES_SCALAR(FLATBUFFERS_TD);
          #undef FLATBUFFERS_TD
          #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, \
            PTYPE) \
            case BASE_TYPE_ ## ENUM: \
              builder_.Pad(field->padding); \
              if (IsStruct(field->value.type)) { \
                SerializeStruct(*field->value.type.struct_def, field_value); \
              } else { \
                CTYPE val; \
                ECHECK(atot(field_value.constant.c_str(), *this, &val)); \
                builder_.AddOffset(field_value.offset, val); \
              } \
              break;
            FLATBUFFERS_GEN_TYPES_POINTER(FLATBUFFERS_TD);
          #undef FLATBUFFERS_TD
        }
      }
    }
  }
  for (size_t i = 0; i < fieldn; i++) field_stack_.pop_back();

  if (struct_def.fixed) {
    builder_.ClearOffsets();
    builder_.EndStruct();
    assert(value);
    // Temporarily store this struct in the value string, since it is to
    // be serialized in-place elsewhere.
    value->assign(
          reinterpret_cast<const char *>(builder_.GetCurrentBufferPointer()),
          struct_def.bytesize);
    builder_.PopBytes(struct_def.bytesize);
    assert(!ovalue);
  } else {
    auto val = builder_.EndTable(start,
                          static_cast<voffset_t>(struct_def.fields.vec.size()));
    if (ovalue) *ovalue = val;
    if (value) *value = NumToString(val);
  }
  return NoError();
}

CheckedError Parser::ParseVector(const Type &type, uoffset_t *ovalue) {
  int count = 0;
  for (;;) {
    if ((!opts.strict_json || !count) && Is(']')) { NEXT(); break; }
    Value val;
    val.type = type;
    ECHECK(ParseAnyValue(val, nullptr, 0));
    field_stack_.push_back(std::make_pair(val, nullptr));
    count++;
    if (Is(']')) { NEXT(); break; }
    EXPECT(',');
  }

  builder_.StartVector(count * InlineSize(type) / InlineAlignment(type),
                       InlineAlignment(type));
  for (int i = 0; i < count; i++) {
    // start at the back, since we're building the data backwards.
    auto &val = field_stack_.back().first;
    switch (val.type.base_type) {
      #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE) \
        case BASE_TYPE_ ## ENUM: \
          if (IsStruct(val.type)) SerializeStruct(*val.type.struct_def, val); \
          else { \
             CTYPE elem; \
             ECHECK(atot(val.constant.c_str(), *this, &elem)); \
             builder_.PushElement(elem); \
          } \
          break;
        FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
      #undef FLATBUFFERS_TD
    }
    field_stack_.pop_back();
  }

  builder_.ClearOffsets();
  *ovalue = builder_.EndVector(count);
  return NoError();
}

CheckedError Parser::ParseMetaData(SymbolTable<Value> *attributes) {
  if (Is('(')) {
    NEXT();
    for (;;) {
      auto name = attribute_;
      EXPECT(kTokenIdentifier);
      if (known_attributes_.find(name) == known_attributes_.end())
        return Error("user define attributes must be declared before use: " +
                     name);
      auto e = new Value();
      attributes->Add(name, e);
      if (Is(':')) {
        NEXT();
        ECHECK(ParseSingleValue(*e));
      }
      if (Is(')')) { NEXT(); break; }
      EXPECT(',');
    }
  }
  return NoError();
}

CheckedError Parser::TryTypedValue(int dtoken, bool check, Value &e,
                                   BaseType req, bool *destmatch) {
  bool match = dtoken == token_;
  if (match) {
    *destmatch = true;
    e.constant = attribute_;
    if (!check) {
      if (e.type.base_type == BASE_TYPE_NONE) {
        e.type.base_type = req;
      } else {
        return Error(std::string("type mismatch: expecting: ") +
                     kTypeNames[e.type.base_type] +
                     ", found: " +
                     kTypeNames[req]);
      }
    }
    NEXT();
  }
  return NoError();
}

CheckedError Parser::ParseEnumFromString(Type &type, int64_t *result) {
  *result = 0;
  // Parse one or more enum identifiers, separated by spaces.
  const char *next = attribute_.c_str();
  do {
    const char *divider = strchr(next, ' ');
    std::string word;
    if (divider) {
      word = std::string(next, divider);
      next = divider + strspn(divider, " ");
    } else {
      word = next;
      next += word.length();
    }
    if (type.enum_def) {  // The field has an enum type
      auto enum_val = type.enum_def->vals.Lookup(word);
      if (!enum_val)
        return Error("unknown enum value: " + word +
              ", for enum: " + type.enum_def->name);
      *result |= enum_val->value;
    } else {  // No enum type, probably integral field.
      if (!IsInteger(type.base_type))
        return Error("not a valid value for this field: " + word);
      // TODO: could check if its a valid number constant here.
      const char *dot = strrchr(word.c_str(), '.');
      if (!dot)
        return Error("enum values need to be qualified by an enum type");
      std::string enum_def_str(word.c_str(), dot);
      std::string enum_val_str(dot + 1, word.c_str() + word.length());
      auto enum_def = LookupEnum(enum_def_str);
      if (!enum_def) return Error("unknown enum: " + enum_def_str);
      auto enum_val = enum_def->vals.Lookup(enum_val_str);
      if (!enum_val) return Error("unknown enum value: " + enum_val_str);
      *result |= enum_val->value;
    }
  } while(*next);
  return NoError();
}


CheckedError Parser::ParseHash(Value &e, FieldDef* field) {
  assert(field);
  Value *hash_name = field->attributes.Lookup("hash");
  switch (e.type.base_type) {
    case BASE_TYPE_INT:
    case BASE_TYPE_UINT: {
      auto hash = FindHashFunction32(hash_name->constant.c_str());
      uint32_t hashed_value = hash(attribute_.c_str());
      e.constant = NumToString(hashed_value);
      break;
    }
    case BASE_TYPE_LONG:
    case BASE_TYPE_ULONG: {
      auto hash = FindHashFunction64(hash_name->constant.c_str());
      uint64_t hashed_value = hash(attribute_.c_str());
      e.constant = NumToString(hashed_value);
      break;
    }
    default:
      assert(0);
  }
  NEXT();
  return NoError();
}

CheckedError Parser::ParseSingleValue(Value &e) {
  // First check if this could be a string/identifier enum value:
  if (e.type.base_type != BASE_TYPE_STRING &&
      e.type.base_type != BASE_TYPE_NONE &&
      (token_ == kTokenIdentifier || token_ == kTokenStringConstant)) {
    if (IsIdentifierStart(attribute_[0])) {  // Enum value.
      int64_t val;
      ECHECK(ParseEnumFromString(e.type, &val));
      e.constant = NumToString(val);
      NEXT();
    } else {  // Numeric constant in string.
      if (IsInteger(e.type.base_type)) {
        // TODO(wvo): do we want to check for garbage after the number?
        e.constant = NumToString(StringToInt(attribute_.c_str()));
      } else if (IsFloat(e.type.base_type)) {
        e.constant = NumToString(strtod(attribute_.c_str(), nullptr));
      } else {
        assert(0);  // Shouldn't happen, we covered all types.
        e.constant = "0";
      }
      NEXT();
    }
  } else {
    bool match = false;
    ECHECK(TryTypedValue(kTokenIntegerConstant,
                         IsScalar(e.type.base_type),
                         e,
                         BASE_TYPE_INT,
                         &match));
    ECHECK(TryTypedValue(kTokenFloatConstant,
                         IsFloat(e.type.base_type),
                         e,
                         BASE_TYPE_FLOAT,
                         &match));
    ECHECK(TryTypedValue(kTokenStringConstant,
                         e.type.base_type == BASE_TYPE_STRING,
                         e,
                         BASE_TYPE_STRING,
                         &match));
    if (!match)
      return Error("cannot parse value starting with: " +
                   TokenToStringId(token_));
  }
  return NoError();
}

StructDef *Parser::LookupCreateStruct(const std::string &name,
                                      bool create_if_new, bool definition) {
  std::string qualified_name = namespaces_.back()->GetFullyQualifiedName(name);
  // See if it exists pre-declared by an unqualified use.
  auto struct_def = structs_.Lookup(name);
  if (struct_def && struct_def->predecl) {
    if (definition) {
      // Make sure it has the current namespace, and is registered under its
      // qualified name.
      struct_def->defined_namespace = namespaces_.back();
      structs_.Move(name, qualified_name);
    }
    return struct_def;
  }
  // See if it exists pre-declared by an qualified use.
  struct_def = structs_.Lookup(qualified_name);
  if (struct_def && struct_def->predecl) {
    if (definition) {
      // Make sure it has the current namespace.
      struct_def->defined_namespace = namespaces_.back();
    }
    return struct_def;
  }
  if (!definition) {
    // Search thru parent namespaces.
    for (size_t components = namespaces_.back()->components.size();
         components && !struct_def; components--) {
      struct_def = structs_.Lookup(
          namespaces_.back()->GetFullyQualifiedName(name, components - 1));
    }
  }
  if (!struct_def && create_if_new) {
    struct_def = new StructDef();
    if (definition) {
      structs_.Add(qualified_name, struct_def);
      struct_def->name = name;
      struct_def->defined_namespace = namespaces_.back();
    } else {
      // Not a definition.
      // Rather than failing, we create a "pre declared" StructDef, due to
      // circular references, and check for errors at the end of parsing.
      // It is defined in the root namespace, since we don't know what the
      // final namespace will be.
      // TODO: maybe safer to use special namespace?
      structs_.Add(name, struct_def);
      struct_def->name = name;
      struct_def->defined_namespace = new Namespace();
      namespaces_.insert(namespaces_.begin(), struct_def->defined_namespace);
    }
  }
  return struct_def;
}

CheckedError Parser::ParseEnum(bool is_union, EnumDef **dest) {
  std::vector<std::string> enum_comment = doc_comment_;
  NEXT();
  std::string enum_name = attribute_;
  EXPECT(kTokenIdentifier);
  auto &enum_def = *new EnumDef();
  enum_def.name = enum_name;
  enum_def.file = file_being_parsed_;
  enum_def.doc_comment = enum_comment;
  enum_def.is_union = is_union;
  enum_def.defined_namespace = namespaces_.back();
  if (enums_.Add(namespaces_.back()->GetFullyQualifiedName(enum_name),
                 &enum_def))
    return Error("enum already exists: " + enum_name);
  if (is_union) {
    enum_def.underlying_type.base_type = BASE_TYPE_UTYPE;
    enum_def.underlying_type.enum_def = &enum_def;
  } else {
    if (opts.proto_mode) {
      enum_def.underlying_type.base_type = BASE_TYPE_INT;
    } else {
      // Give specialized error message, since this type spec used to
      // be optional in the first FlatBuffers release.
      if (!Is(':')) {
        return Error("must specify the underlying integer type for this"
              " enum (e.g. \': short\', which was the default).");
      } else {
        NEXT();
      }
      // Specify the integer type underlying this enum.
      ECHECK(ParseType(enum_def.underlying_type));
      if (!IsInteger(enum_def.underlying_type.base_type))
        return Error("underlying enum type must be integral");
    }
    // Make this type refer back to the enum it was derived from.
    enum_def.underlying_type.enum_def = &enum_def;
  }
  ECHECK(ParseMetaData(&enum_def.attributes));
  EXPECT('{');
  if (is_union) enum_def.vals.Add("NONE", new EnumVal("NONE", 0));
  for (;;) {
    if (opts.proto_mode && attribute_ == "option") {
      ECHECK(ParseProtoOption());
    } else {
      auto value_name = attribute_;
      auto full_name = value_name;
      std::vector<std::string> value_comment = doc_comment_;
      EXPECT(kTokenIdentifier);
      if (is_union) ECHECK(ParseNamespacing(&full_name, &value_name));
      auto prevsize = enum_def.vals.vec.size();
      auto value = enum_def.vals.vec.size()
        ? enum_def.vals.vec.back()->value + 1
        : 0;
      auto &ev = *new EnumVal(value_name, value);
      if (enum_def.vals.Add(value_name, &ev))
        return Error("enum value already exists: " + value_name);
      ev.doc_comment = value_comment;
      if (is_union) {
        ev.struct_def = LookupCreateStruct(full_name);
      }
      if (Is('=')) {
        NEXT();
        ev.value = StringToInt(attribute_.c_str());
        EXPECT(kTokenIntegerConstant);
        if (!opts.proto_mode && prevsize &&
            enum_def.vals.vec[prevsize - 1]->value >= ev.value)
          return Error("enum values must be specified in ascending order");
      }
      if (opts.proto_mode && Is('[')) {
        NEXT();
        // ignore attributes on enums.
        while (token_ != ']') NEXT();
        NEXT();
      }
    }
    if (!Is(opts.proto_mode ? ';' : ',')) break;
    NEXT();
    if (Is('}')) break;
  }
  EXPECT('}');
  if (enum_def.attributes.Lookup("bit_flags")) {
    for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
         ++it) {
      if (static_cast<size_t>((*it)->value) >=
           SizeOf(enum_def.underlying_type.base_type) * 8)
        return Error("bit flag out of range of underlying integral type");
      (*it)->value = 1LL << (*it)->value;
    }
  }
  if (dest) *dest = &enum_def;
  return NoError();
}

CheckedError Parser::StartStruct(const std::string &name, StructDef **dest) {
  auto &struct_def = *LookupCreateStruct(name, true, true);
  if (!struct_def.predecl) return Error("datatype already exists: " + name);
  struct_def.predecl = false;
  struct_def.name = name;
  struct_def.file = file_being_parsed_;
  // Move this struct to the back of the vector just in case it was predeclared,
  // to preserve declaration order.
  *remove(structs_.vec.begin(), structs_.vec.end(), &struct_def) = &struct_def;
  *dest = &struct_def;
  return NoError();
}

CheckedError Parser::CheckClash(std::vector<FieldDef*> &fields,
                                StructDef *struct_def,
                                const char *suffix,
                                BaseType basetype) {
  auto len = strlen(suffix);
  for (auto it = fields.begin(); it != fields.end(); ++it) {
    auto &fname = (*it)->name;
    if (fname.length() > len &&
        fname.compare(fname.length() - len, len, suffix) == 0 &&
        (*it)->value.type.base_type != BASE_TYPE_UTYPE) {
      auto field = struct_def->fields.Lookup(
                                             fname.substr(0, fname.length() - len));
      if (field && field->value.type.base_type == basetype)
        return Error("Field " + fname +
                     " would clash with generated functions for field " +
                     field->name);
    }
  }
  return NoError();
}

static bool compareFieldDefs(const FieldDef *a, const FieldDef *b) {
  auto a_id = atoi(a->attributes.Lookup("id")->constant.c_str());
  auto b_id = atoi(b->attributes.Lookup("id")->constant.c_str());
  return a_id < b_id;
}

CheckedError Parser::ParseDecl() {
  std::vector<std::string> dc = doc_comment_;
  bool fixed = Is(kTokenStruct);
  if (fixed) NEXT() else EXPECT(kTokenTable);
  std::string name = attribute_;
  EXPECT(kTokenIdentifier);
  StructDef *struct_def;
  ECHECK(StartStruct(name, &struct_def));
  struct_def->doc_comment = dc;
  struct_def->fixed = fixed;
  ECHECK(ParseMetaData(&struct_def->attributes));
  struct_def->sortbysize =
    struct_def->attributes.Lookup("original_order") == nullptr && !fixed;
  EXPECT('{');
  while (token_ != '}') ECHECK(ParseField(*struct_def));
  auto force_align = struct_def->attributes.Lookup("force_align");
  if (fixed && force_align) {
    auto align = static_cast<size_t>(atoi(force_align->constant.c_str()));
    if (force_align->type.base_type != BASE_TYPE_INT ||
        align < struct_def->minalign ||
        align > 16 ||
        align & (align - 1))
      return Error("force_align must be a power of two integer ranging from the"
            "struct\'s natural alignment to 16");
    struct_def->minalign = align;
  }
  struct_def->PadLastField(struct_def->minalign);
  // Check if this is a table that has manual id assignments
  auto &fields = struct_def->fields.vec;
  if (!struct_def->fixed && fields.size()) {
    size_t num_id_fields = 0;
    for (auto it = fields.begin(); it != fields.end(); ++it) {
      if ((*it)->attributes.Lookup("id")) num_id_fields++;
    }
    // If any fields have ids..
    if (num_id_fields) {
      // Then all fields must have them.
      if (num_id_fields != fields.size())
        return Error(
              "either all fields or no fields must have an 'id' attribute");
      // Simply sort by id, then the fields are the same as if no ids had
      // been specified.
      std::sort(fields.begin(), fields.end(), compareFieldDefs);
      // Verify we have a contiguous set, and reassign vtable offsets.
      for (int i = 0; i < static_cast<int>(fields.size()); i++) {
        if (i != atoi(fields[i]->attributes.Lookup("id")->constant.c_str()))
          return Error("field id\'s must be consecutive from 0, id " +
                NumToString(i) + " missing or set twice");
        fields[i]->value.offset = FieldIndexToOffset(static_cast<voffset_t>(i));
      }
    }
  }

  ECHECK(CheckClash(fields, struct_def, "_type", BASE_TYPE_UNION));
  ECHECK(CheckClash(fields, struct_def, "Type", BASE_TYPE_UNION));
  ECHECK(CheckClash(fields, struct_def, "_length", BASE_TYPE_VECTOR));
  ECHECK(CheckClash(fields, struct_def, "Length", BASE_TYPE_VECTOR));
  ECHECK(CheckClash(fields, struct_def, "_byte_vector", BASE_TYPE_STRING));
  ECHECK(CheckClash(fields, struct_def, "ByteVector", BASE_TYPE_STRING));
  EXPECT('}');
  return NoError();
}

CheckedError Parser::ParseService() {
  std::vector<std::string> service_comment = doc_comment_;
  NEXT();
  auto service_name = attribute_;
  EXPECT(kTokenIdentifier);
  auto &service_def = *new ServiceDef();
  service_def.name = service_name;
  service_def.file = file_being_parsed_;
  service_def.doc_comment = service_comment;
  service_def.defined_namespace = namespaces_.back();
  if (services_.Add(namespaces_.back()->GetFullyQualifiedName(service_name),
                    &service_def))
    return Error("service already exists: " + service_name);
  ECHECK(ParseMetaData(&service_def.attributes));
  EXPECT('{');
  do {
    auto rpc_name = attribute_;
    EXPECT(kTokenIdentifier);
    EXPECT('(');
    Type reqtype, resptype;
    ECHECK(ParseTypeIdent(reqtype));
    EXPECT(')');
    EXPECT(':');
    ECHECK(ParseTypeIdent(resptype));
    if (reqtype.base_type != BASE_TYPE_STRUCT || reqtype.struct_def->fixed ||
        resptype.base_type != BASE_TYPE_STRUCT || resptype.struct_def->fixed)
        return Error("rpc request and response types must be tables");
    auto &rpc = *new RPCCall();
    rpc.name = rpc_name;
    rpc.request = reqtype.struct_def;
    rpc.response = resptype.struct_def;
    if (service_def.calls.Add(rpc_name, &rpc))
      return Error("rpc already exists: " + rpc_name);
    ECHECK(ParseMetaData(&rpc.attributes));
    EXPECT(';');
  } while (token_ != '}');
  NEXT();
  return NoError();
}

bool Parser::SetRootType(const char *name) {
  root_struct_def_ = structs_.Lookup(name);
  if (!root_struct_def_)
    root_struct_def_ = structs_.Lookup(
                         namespaces_.back()->GetFullyQualifiedName(name));
  return root_struct_def_ != nullptr;
}

void Parser::MarkGenerated() {
  // This function marks all existing definitions as having already
  // been generated, which signals no code for included files should be
  // generated.
  for (auto it = enums_.vec.begin();
           it != enums_.vec.end(); ++it) {
    (*it)->generated = true;
  }
  for (auto it = structs_.vec.begin();
           it != structs_.vec.end(); ++it) {
    (*it)->generated = true;
  }
}

CheckedError Parser::ParseNamespace() {
  NEXT();
  auto ns = new Namespace();
  namespaces_.push_back(ns);
  if (token_ != ';') {
    for (;;) {
      ns->components.push_back(attribute_);
      EXPECT(kTokenIdentifier);
      if (Is('.')) NEXT() else break;
    }
  }
  EXPECT(';');
  return NoError();
}

static bool compareEnumVals(const EnumVal *a, const EnumVal* b) {
  return a->value < b->value;
}

// Best effort parsing of .proto declarations, with the aim to turn them
// in the closest corresponding FlatBuffer equivalent.
// We parse everything as identifiers instead of keywords, since we don't
// want protobuf keywords to become invalid identifiers in FlatBuffers.
CheckedError Parser::ParseProtoDecl() {
  bool isextend = attribute_ == "extend";
  if (attribute_ == "package") {
    // These are identical in syntax to FlatBuffer's namespace decl.
    ECHECK(ParseNamespace());
  } else if (attribute_ == "message" || isextend) {
    std::vector<std::string> struct_comment = doc_comment_;
    NEXT();
    StructDef *struct_def = nullptr;
    if (isextend) {
      if (Is('.')) NEXT();  // qualified names may start with a . ?
      auto id = attribute_;
      EXPECT(kTokenIdentifier);
      ECHECK(ParseNamespacing(&id, nullptr));
      struct_def = LookupCreateStruct(id, false);
      if (!struct_def)
        return Error("cannot extend unknown message type: " + id);
    } else {
      std::string name = attribute_;
      EXPECT(kTokenIdentifier);
      ECHECK(StartStruct(name, &struct_def));
      // Since message definitions can be nested, we create a new namespace.
      auto ns = new Namespace();
      // Copy of current namespace.
      *ns = *namespaces_.back();
      // But with current message name.
      ns->components.push_back(name);
      namespaces_.push_back(ns);
    }
    struct_def->doc_comment = struct_comment;
    ECHECK(ParseProtoFields(struct_def, isextend, false));
    if (!isextend) {
      // We have to remove the nested namespace, but we can't just throw it
      // away, so put it at the beginning of the vector.
      auto ns = namespaces_.back();
      namespaces_.pop_back();
      namespaces_.insert(namespaces_.begin(), ns);
    }
    if (Is(';')) NEXT();
  } else if (attribute_ == "enum") {
    // These are almost the same, just with different terminator:
    EnumDef *enum_def;
    ECHECK(ParseEnum(false, &enum_def));
    if (Is(';')) NEXT();
    // Protobuf allows them to be specified in any order, so sort afterwards.
    auto &v = enum_def->vals.vec;
    std::sort(v.begin(), v.end(), compareEnumVals);

    // Temp: remove any duplicates, as .fbs files can't handle them.
    for (auto it = v.begin(); it != v.end(); ) {
      if (it != v.begin() && it[0]->value == it[-1]->value) it = v.erase(it);
      else ++it;
    }
  } else if (attribute_ == "syntax") {  // Skip these.
    NEXT();
    EXPECT('=');
    EXPECT(kTokenStringConstant);
    EXPECT(';');
  } else if (attribute_ == "option") {  // Skip these.
    ECHECK(ParseProtoOption());
    EXPECT(';');
  } else if (attribute_ == "service") {  // Skip these.
    NEXT();
    EXPECT(kTokenIdentifier);
    ECHECK(ParseProtoCurliesOrIdent());
  } else {
    return Error("don\'t know how to parse .proto declaration starting with " +
          TokenToStringId(token_));
  }
  return NoError();
}

CheckedError Parser::ParseProtoFields(StructDef *struct_def, bool isextend,
                                      bool inside_oneof) {
  EXPECT('{');
  while (token_ != '}') {
    if (attribute_ == "message" || attribute_ == "extend" ||
        attribute_ == "enum") {
      // Nested declarations.
      ECHECK(ParseProtoDecl());
    } else if (attribute_ == "extensions") {  // Skip these.
      NEXT();
      EXPECT(kTokenIntegerConstant);
      if (Is(kTokenIdentifier)) {
        NEXT();  // to
        NEXT();  // num
      }
      EXPECT(';');
    } else if (attribute_ == "option") {  // Skip these.
      ECHECK(ParseProtoOption());
      EXPECT(';');
    } else if (attribute_ == "reserved") {  // Skip these.
      NEXT();
      EXPECT(kTokenIntegerConstant);
      while (Is(',')) { NEXT(); EXPECT(kTokenIntegerConstant); }
      EXPECT(';');
    } else {
      std::vector<std::string> field_comment = doc_comment_;
      // Parse the qualifier.
      bool required = false;
      bool repeated = false;
      bool oneof = false;
      if (!inside_oneof) {
        if (attribute_ == "optional") {
          // This is the default.
          EXPECT(kTokenIdentifier);
        } else if (attribute_ == "required") {
          required = true;
          EXPECT(kTokenIdentifier);
        } else if (attribute_ == "repeated") {
          repeated = true;
          EXPECT(kTokenIdentifier);
        } else if (attribute_ == "oneof") {
          oneof = true;
          EXPECT(kTokenIdentifier);
        } else {
          // can't error, proto3 allows decls without any of the above.
        }
      }
      StructDef *anonymous_struct = nullptr;
      Type type;
      if (attribute_ == "group" || oneof) {
        if (!oneof) EXPECT(kTokenIdentifier);
        auto name = "Anonymous" + NumToString(anonymous_counter++);
        ECHECK(StartStruct(name, &anonymous_struct));
        type = Type(BASE_TYPE_STRUCT, anonymous_struct);
      } else {
        ECHECK(ParseTypeFromProtoType(&type));
      }
      // Repeated elements get mapped to a vector.
      if (repeated) {
        type.element = type.base_type;
        type.base_type = BASE_TYPE_VECTOR;
      }
      std::string name = attribute_;
      // Protos may use our keywords "attribute" & "namespace" as an identifier.
      if (Is(kTokenAttribute) || Is(kTokenNameSpace)) {
        NEXT();
        // TODO: simpler to just not make these keywords?
        name += "_";  // Have to make it not a keyword.
      } else {
        EXPECT(kTokenIdentifier);
      }
      if (!oneof) {
        // Parse the field id. Since we're just translating schemas, not
        // any kind of binary compatibility, we can safely ignore these, and
        // assign our own.
        EXPECT('=');
        EXPECT(kTokenIntegerConstant);
      }
      FieldDef *field = nullptr;
      if (isextend) {
        // We allow a field to be re-defined when extending.
        // TODO: are there situations where that is problematic?
        field = struct_def->fields.Lookup(name);
      }
      if (!field) ECHECK(AddField(*struct_def, name, type, &field));
      field->doc_comment = field_comment;
      if (!IsScalar(type.base_type)) field->required = required;
      // See if there's a default specified.
      if (Is('[')) {
        NEXT();
        for (;;) {
          auto key = attribute_;
          ECHECK(ParseProtoKey());
          EXPECT('=');
          auto val = attribute_;
          ECHECK(ParseProtoCurliesOrIdent());
          if (key == "default") {
            // Temp: skip non-numeric defaults (enums).
            auto numeric = strpbrk(val.c_str(), "0123456789-+.");
            if (IsScalar(type.base_type) && numeric == val.c_str())
              field->value.constant = val;
          } else if (key == "deprecated") {
            field->deprecated = val == "true";
          }
          if (!Is(',')) break;
          NEXT();
        }
        EXPECT(']');
      }
      if (anonymous_struct) {
        ECHECK(ParseProtoFields(anonymous_struct, false, oneof));
        if (Is(';')) NEXT();
      } else {
        EXPECT(';');
      }
    }
  }
  NEXT();
  return NoError();
}

CheckedError Parser::ParseProtoKey() {
  if (token_ == '(') {
    NEXT();
    // Skip "(a.b)" style custom attributes.
    while (token_ == '.' || token_ == kTokenIdentifier) NEXT();
    EXPECT(')');
    while (Is('.')) { NEXT(); EXPECT(kTokenIdentifier); }
  } else {
    EXPECT(kTokenIdentifier);
  }
  return NoError();
}

CheckedError Parser::ParseProtoCurliesOrIdent() {
  if (Is('{')) {
    NEXT();
    for (int nesting = 1; nesting; ) {
      if (token_ == '{') nesting++;
      else if (token_ == '}') nesting--;
      NEXT();
    }
  } else {
    NEXT();  // Any single token.
  }
  return NoError();
}

CheckedError Parser::ParseProtoOption() {
  NEXT();
  ECHECK(ParseProtoKey());
  EXPECT('=');
  ECHECK(ParseProtoCurliesOrIdent());
  return NoError();
}

// Parse a protobuf type, and map it to the corresponding FlatBuffer one.
CheckedError Parser::ParseTypeFromProtoType(Type *type) {
  struct type_lookup { const char *proto_type; BaseType fb_type; };
  static type_lookup lookup[] = {
    { "float", BASE_TYPE_FLOAT },  { "double", BASE_TYPE_DOUBLE },
    { "int32", BASE_TYPE_INT },    { "int64", BASE_TYPE_LONG },
    { "uint32", BASE_TYPE_UINT },  { "uint64", BASE_TYPE_ULONG },
    { "sint32", BASE_TYPE_INT },   { "sint64", BASE_TYPE_LONG },
    { "fixed32", BASE_TYPE_UINT }, { "fixed64", BASE_TYPE_ULONG },
    { "sfixed32", BASE_TYPE_INT }, { "sfixed64", BASE_TYPE_LONG },
    { "bool", BASE_TYPE_BOOL },
    { "string", BASE_TYPE_STRING },
    { "bytes", BASE_TYPE_STRING },
    { nullptr, BASE_TYPE_NONE }
  };
  for (auto tl = lookup; tl->proto_type; tl++) {
    if (attribute_ == tl->proto_type) {
      type->base_type = tl->fb_type;
      NEXT();
      return NoError();
    }
  }
  if (Is('.')) NEXT();  // qualified names may start with a . ?
  ECHECK(ParseTypeIdent(*type));
  return NoError();
}

CheckedError Parser::SkipAnyJsonValue() {
  switch (token_) {
    case '{':
      ECHECK(SkipJsonObject());
      break;
    case kTokenStringConstant:
      ECHECK(SkipJsonString());
      break;
    case '[':
      ECHECK(SkipJsonArray());
      break;
    case kTokenIntegerConstant:
      EXPECT(kTokenIntegerConstant);
      break;
    case kTokenFloatConstant:
      EXPECT(kTokenFloatConstant);
      break;
    default:
      return Error(std::string("Unexpected token:") + std::string(1, static_cast<char>(token_)));
  }
  return NoError();
}

CheckedError Parser::SkipJsonObject() {
  EXPECT('{');
  size_t fieldn = 0;

  for (;;) {
    if ((!opts.strict_json || !fieldn) && Is('}')) break;

    if (!Is(kTokenStringConstant)) {
      EXPECT(opts.strict_json ? kTokenStringConstant : kTokenIdentifier);
    }
    else {
      NEXT();
    }

    EXPECT(':');
    ECHECK(SkipAnyJsonValue());
    fieldn++;

    if (Is('}')) break;
    EXPECT(',');
  }

  NEXT();
  return NoError();
}

CheckedError Parser::SkipJsonArray() {
  EXPECT('[');

  for (;;) {
    if (Is(']')) break;

    ECHECK(SkipAnyJsonValue());

    if (Is(']')) break;
    EXPECT(',');
  }

  NEXT();
  return NoError();
}

CheckedError Parser::SkipJsonString() {
  EXPECT(kTokenStringConstant);
  return NoError();
}

bool Parser::Parse(const char *source, const char **include_paths,
                   const char *source_filename) {
  return !DoParse(source, include_paths, source_filename).Check();
}

CheckedError Parser::DoParse(const char *source, const char **include_paths,
                             const char *source_filename) {
  file_being_parsed_ = source_filename ? source_filename : "";
  if (source_filename &&
      included_files_.find(source_filename) == included_files_.end()) {
    included_files_[source_filename] = true;
    files_included_per_file_[source_filename] = std::set<std::string>();
  }
  if (!include_paths) {
    static const char *current_directory[] = { "", nullptr };
    include_paths = current_directory;
  }
  source_ = cursor_ = source;
  line_ = 1;
  error_.clear();
  builder_.Clear();
  // Start with a blank namespace just in case this file doesn't have one.
  namespaces_.push_back(new Namespace());
  ECHECK(SkipByteOrderMark());
  NEXT();
  // Includes must come before type declarations:
  for (;;) {
    // Parse pre-include proto statements if any:
    if (opts.proto_mode &&
        (attribute_ == "option" || attribute_ == "syntax" ||
         attribute_ == "package")) {
        ECHECK(ParseProtoDecl());
    } else if (Is(kTokenInclude) ||
               (opts.proto_mode &&
                attribute_ == "import" &&
                Is(kTokenIdentifier))) {
      NEXT();
      if (opts.proto_mode && attribute_ == "public") NEXT();
      auto name = attribute_;
      EXPECT(kTokenStringConstant);
      // Look for the file in include_paths.
      std::string filepath;
      for (auto paths = include_paths; paths && *paths; paths++) {
        filepath = flatbuffers::ConCatPathFileName(*paths, name);
        if(FileExists(filepath.c_str())) break;
      }
      if (filepath.empty())
        return Error("unable to locate include file: " + name);
      if (source_filename)
        files_included_per_file_[source_filename].insert(filepath);
      if (included_files_.find(filepath) == included_files_.end()) {
        // We found an include file that we have not parsed yet.
        // Load it and parse it.
        std::string contents;
        if (!LoadFile(filepath.c_str(), true, &contents))
          return Error("unable to load include file: " + name);
        ECHECK(DoParse(contents.c_str(), include_paths, filepath.c_str()));
        // We generally do not want to output code for any included files:
        if (!opts.generate_all) MarkGenerated();
        // This is the easiest way to continue this file after an include:
        // instead of saving and restoring all the state, we simply start the
        // file anew. This will cause it to encounter the same include
        // statement again, but this time it will skip it, because it was
        // entered into included_files_.
        // This is recursive, but only go as deep as the number of include
        // statements.
        return DoParse(source, include_paths, source_filename);
      }
      EXPECT(';');
    } else {
      break;
    }
  }
  // Now parse all other kinds of declarations:
  while (token_ != kTokenEof) {
    if (opts.proto_mode) {
      ECHECK(ParseProtoDecl());
    } else if (token_ == kTokenNameSpace) {
      ECHECK(ParseNamespace());
    } else if (token_ == '{') {
      if (!root_struct_def_)
        return Error("no root type set to parse json with");
      if (builder_.GetSize()) {
        return Error("cannot have more than one json object in a file");
      }
      uoffset_t toff;
      ECHECK(ParseTable(*root_struct_def_, nullptr, &toff));
      builder_.Finish(Offset<Table>(toff),
                file_identifier_.length() ? file_identifier_.c_str() : nullptr);
    } else if (token_ == kTokenEnum) {
      ECHECK(ParseEnum(false, nullptr));
    } else if (token_ == kTokenUnion) {
      ECHECK(ParseEnum(true, nullptr));
    } else if (token_ == kTokenRootType) {
      NEXT();
      auto root_type = attribute_;
      EXPECT(kTokenIdentifier);
      ECHECK(ParseNamespacing(&root_type, nullptr));
      if (!SetRootType(root_type.c_str()))
        return Error("unknown root type: " + root_type);
      if (root_struct_def_->fixed)
        return Error("root type must be a table");
      EXPECT(';');
    } else if (token_ == kTokenFileIdentifier) {
      NEXT();
      file_identifier_ = attribute_;
      EXPECT(kTokenStringConstant);
      if (file_identifier_.length() !=
          FlatBufferBuilder::kFileIdentifierLength)
        return Error("file_identifier must be exactly " +
              NumToString(FlatBufferBuilder::kFileIdentifierLength) +
              " characters");
      EXPECT(';');
    } else if (token_ == kTokenFileExtension) {
      NEXT();
      file_extension_ = attribute_;
      EXPECT(kTokenStringConstant);
      EXPECT(';');
    } else if(token_ == kTokenInclude) {
      return Error("includes must come before declarations");
    } else if(token_ == kTokenAttribute) {
      NEXT();
      auto name = attribute_;
      EXPECT(kTokenStringConstant);
      EXPECT(';');
      known_attributes_[name] = false;
    } else if (token_ == kTokenService) {
      ECHECK(ParseService());
    } else {
      ECHECK(ParseDecl());
    }
  }
  for (auto it = structs_.vec.begin(); it != structs_.vec.end(); ++it) {
    if ((*it)->predecl) {
      return Error("type referenced but not defined: " + (*it)->name);
    }
  }
  for (auto it = enums_.vec.begin(); it != enums_.vec.end(); ++it) {
    auto &enum_def = **it;
    if (enum_def.is_union) {
      for (auto val_it = enum_def.vals.vec.begin();
           val_it != enum_def.vals.vec.end();
           ++val_it) {
        auto &val = **val_it;
        if (val.struct_def && val.struct_def->fixed)
          return Error("only tables can be union elements: " + val.name);
      }
    }
  }
  return NoError();
}

std::set<std::string> Parser::GetIncludedFilesRecursive(
    const std::string &file_name) const {
  std::set<std::string> included_files;
  std::list<std::string> to_process;

  if (file_name.empty()) return included_files;
  to_process.push_back(file_name);

  while (!to_process.empty()) {
    std::string current = to_process.front();
    to_process.pop_front();
    included_files.insert(current);

    auto new_files = files_included_per_file_.at(current);
    for (auto it = new_files.begin(); it != new_files.end(); ++it) {
      if (included_files.find(*it) == included_files.end())
        to_process.push_back(*it);
    }
  }

  return included_files;
}

// Schema serialization functionality:

template<typename T> bool compareName(const T* a, const T* b) {
    return a->name < b->name;
}

template<typename T> void AssignIndices(const std::vector<T *> &defvec) {
  // Pre-sort these vectors, such that we can set the correct indices for them.
  auto vec = defvec;
  std::sort(vec.begin(), vec.end(), compareName<T>);
  for (int i = 0; i < static_cast<int>(vec.size()); i++) vec[i]->index = i;
}

void Parser::Serialize() {
  builder_.Clear();
  AssignIndices(structs_.vec);
  AssignIndices(enums_.vec);
  std::vector<Offset<reflection::Object>> object_offsets;
  for (auto it = structs_.vec.begin(); it != structs_.vec.end(); ++it) {
    auto offset = (*it)->Serialize(&builder_, *this);
    object_offsets.push_back(offset);
    (*it)->serialized_location = offset.o;
  }
  std::vector<Offset<reflection::Enum>> enum_offsets;
  for (auto it = enums_.vec.begin(); it != enums_.vec.end(); ++it) {
    auto offset = (*it)->Serialize(&builder_, *this);
    enum_offsets.push_back(offset);
    (*it)->serialized_location = offset.o;
  }
  auto schema_offset = reflection::CreateSchema(
                         builder_,
                         builder_.CreateVectorOfSortedTables(&object_offsets),
                         builder_.CreateVectorOfSortedTables(&enum_offsets),
                         builder_.CreateString(file_identifier_),
                         builder_.CreateString(file_extension_),
                         root_struct_def_
                           ? root_struct_def_->serialized_location
                           : 0);
  builder_.Finish(schema_offset, reflection::SchemaIdentifier());
}

Offset<reflection::Object> StructDef::Serialize(FlatBufferBuilder *builder,
                                                const Parser &parser) const {
  std::vector<Offset<reflection::Field>> field_offsets;
  for (auto it = fields.vec.begin(); it != fields.vec.end(); ++it) {
    field_offsets.push_back(
      (*it)->Serialize(builder,
                       static_cast<uint16_t>(it - fields.vec.begin()), parser));
  }
  return reflection::CreateObject(*builder,
                                  builder->CreateString(name),
                                  builder->CreateVectorOfSortedTables(
                                    &field_offsets),
                                  fixed,
                                  static_cast<int>(minalign),
                                  static_cast<int>(bytesize),
                                  SerializeAttributes(builder, parser));
}

Offset<reflection::Field> FieldDef::Serialize(FlatBufferBuilder *builder,
                                              uint16_t id,
                                              const Parser &parser) const {
  return reflection::CreateField(*builder,
                                 builder->CreateString(name),
                                 value.type.Serialize(builder),
                                 id,
                                 value.offset,
                                 IsInteger(value.type.base_type)
                                   ? StringToInt(value.constant.c_str())
                                   : 0,
                                 IsFloat(value.type.base_type)
                                   ? strtod(value.constant.c_str(), nullptr)
                                   : 0.0,
                                 deprecated,
                                 required,
                                 key,
                                 SerializeAttributes(builder, parser));
  // TODO: value.constant is almost always "0", we could save quite a bit of
  // space by sharing it. Same for common values of value.type.
}

Offset<reflection::Enum> EnumDef::Serialize(FlatBufferBuilder *builder,
                                            const Parser &parser) const {
  std::vector<Offset<reflection::EnumVal>> enumval_offsets;
  for (auto it = vals.vec.begin(); it != vals.vec.end(); ++it) {
    enumval_offsets.push_back((*it)->Serialize(builder));
  }
  return reflection::CreateEnum(*builder,
                                builder->CreateString(name),
                                builder->CreateVector(enumval_offsets),
                                is_union,
                                underlying_type.Serialize(builder),
                                SerializeAttributes(builder, parser));
}

Offset<reflection::EnumVal> EnumVal::Serialize(FlatBufferBuilder *builder) const
                                                                               {
  return reflection::CreateEnumVal(*builder,
                                   builder->CreateString(name),
                                   value,
                                   struct_def
                                     ? struct_def->serialized_location
                                     : 0);
}

Offset<reflection::Type> Type::Serialize(FlatBufferBuilder *builder) const {
  return reflection::CreateType(*builder,
                                static_cast<reflection::BaseType>(base_type),
                                static_cast<reflection::BaseType>(element),
                                struct_def ? struct_def->index :
                                             (enum_def ? enum_def->index : -1));
}

flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<
  reflection::KeyValue>>>
    Definition::SerializeAttributes(FlatBufferBuilder *builder,
                                    const Parser &parser) const {
  std::vector<flatbuffers::Offset<reflection::KeyValue>> attrs;
  for (auto kv = attributes.dict.begin(); kv != attributes.dict.end(); ++kv) {
    auto it = parser.known_attributes_.find(kv->first);
    assert(it != parser.known_attributes_.end());
    if (!it->second) {  // Custom attribute.
      attrs.push_back(
          reflection::CreateKeyValue(*builder, builder->CreateString(kv->first),
                                     builder->CreateString(
                                         kv->second->constant)));
    }
  }
  if (attrs.size()) {
    return builder->CreateVectorOfSortedTables(&attrs);
  } else {
    return 0;
  }
}

}  // namespace flatbuffers
