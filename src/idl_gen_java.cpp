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

// independent from idl_parser, since this code is not needed for most clients

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#if defined(FLATBUFFERS_CPP98_STL)
#  include <cctype>
#endif  // defined(FLATBUFFERS_CPP98_STL)

namespace flatbuffers {
namespace java {

static TypedFloatConstantGenerator JavaFloatGen("Double.", "Float.", "NaN",
                                                "POSITIVE_INFINITY",
                                                "NEGATIVE_INFINITY");

static CommentConfig comment_config = {
  "/**",
  " *",
  " */",
};

class JavaGenerator : public BaseGenerator {
 public:
  JavaGenerator(const Parser &parser, const std::string &path,
                const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", ".", "java"),
        cur_name_space_(nullptr) {}

  JavaGenerator &operator=(const JavaGenerator &);
  bool generate() {
    std::string one_file_code;
    cur_name_space_ = parser_.current_namespace_;

    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      std::string enumcode;
      auto &enum_def = **it;
      if (!parser_.opts.one_file) cur_name_space_ = enum_def.defined_namespace;
      GenEnum(enum_def, &enumcode);
      if (parser_.opts.one_file) {
        one_file_code += enumcode;
      } else {
        if (!SaveType(enum_def.name, *enum_def.defined_namespace, enumcode,
                      false))
          return false;
      }
    }

    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      std::string declcode;
      auto &struct_def = **it;
      if (!parser_.opts.one_file)
        cur_name_space_ = struct_def.defined_namespace;
      GenStruct(struct_def, &declcode);
      if (parser_.opts.one_file) {
        one_file_code += declcode;
      } else {
        if (!SaveType(struct_def.name, *struct_def.defined_namespace, declcode,
                      true))
          return false;
      }
    }

    if (parser_.opts.one_file) {
      return SaveType(file_name_, *parser_.current_namespace_, one_file_code,
                      true);
    }
    return true;
  }

  // Save out the generated code for a single class while adding
  // declaration boilerplate.
  bool SaveType(const std::string &defname, const Namespace &ns,
                const std::string &classcode, bool needs_includes) const {
    if (!classcode.length()) return true;

    std::string code;
    code = "// " + std::string(FlatBuffersGeneratedWarning()) + "\n\n";

    std::string namespace_name = FullNamespace(".", ns);
    if (!namespace_name.empty()) {
      code += "package " + namespace_name + ";";
      code += "\n\n";
    }
    if (needs_includes) {
      code +=
          "import java.nio.*;\nimport java.lang.*;\nimport "
          "java.util.*;\nimport com.google.flatbuffers.*;\n";
      if (parser_.opts.gen_nullable) {
        code += "\nimport javax.annotation.Nullable;\n";
      }
      if (parser_.opts.java_checkerframework) {
        code += "\nimport org.checkerframework.dataflow.qual.Pure;\n";
      }
      code += "\n@SuppressWarnings(\"unused\")\n";
    }
    if (parser_.opts.gen_generated) {
      code += "\n@javax.annotation.Generated(value=\"flatc\")\n";
    }
    code += classcode;
    if (!namespace_name.empty()) code += "";
    auto filename = NamespaceDir(ns) + defname + ".java";
    return SaveFile(filename.c_str(), code, false);
  }

  const Namespace *CurrentNameSpace() const { return cur_name_space_; }

  std::string GenNullableAnnotation(const Type &t) const {
    return parser_.opts.gen_nullable &&
                   !IsScalar(DestinationType(t, true).base_type) &&
                   t.base_type != BASE_TYPE_VECTOR
               ? " @Nullable "
               : "";
  }

  std::string GenPureAnnotation(const Type &t) const {
    return parser_.opts.java_checkerframework &&
                   !IsScalar(DestinationType(t, true).base_type)
               ? " @Pure "
               : "";
  }

  std::string GenTypeBasic(const Type &type) const {
    // clang-format off
    static const char * const java_typename[] = {
      #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, ...) \
        #JTYPE,
        FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
      #undef FLATBUFFERS_TD
    };
    // clang-format on
    return java_typename[type.base_type];
  }

  std::string GenTypePointer(const Type &type) const {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "String";
      case BASE_TYPE_VECTOR: return GenTypeGet(type.VectorType());
      case BASE_TYPE_STRUCT: return WrapInNameSpace(*type.struct_def);
      case BASE_TYPE_UNION: FLATBUFFERS_FALLTHROUGH();  // else fall thru
      default: return "Table";
    }
  }

  std::string GenTypeGet(const Type &type) const {
    return IsScalar(type.base_type)
               ? GenTypeBasic(type)
               : (IsArray(type) ? GenTypeGet(type.VectorType())
                                : GenTypePointer(type));
  }

  // Find the destination type the user wants to receive the value in (e.g.
  // one size higher signed types for unsigned serialized values in Java).
  Type DestinationType(const Type &type, bool vectorelem) const {
    switch (type.base_type) {
      // We use int for both uchar/ushort, since that generally means less
      // casting than using short for uchar.
      case BASE_TYPE_UCHAR: return Type(BASE_TYPE_INT);
      case BASE_TYPE_USHORT: return Type(BASE_TYPE_INT);
      case BASE_TYPE_UINT: return Type(BASE_TYPE_LONG);
      case BASE_TYPE_ARRAY:
      case BASE_TYPE_VECTOR:
        if (vectorelem) return DestinationType(type.VectorType(), vectorelem);
        FLATBUFFERS_FALLTHROUGH();  // else fall thru
      default: return type;
    }
  }

  std::string GenOffsetType() const { return "int"; }

  std::string GenOffsetConstruct(const std::string &variable_name) const {
    return variable_name;
  }

  std::string GenVectorOffsetType() const { return "int"; }

  // Generate destination type name
  std::string GenTypeNameDest(const Type &type) const {
    return GenTypeGet(DestinationType(type, true));
  }

  // Mask to turn serialized value into destination type value.
  std::string DestinationMask(const Type &type, bool vectorelem) const {
    switch (type.base_type) {
      case BASE_TYPE_UCHAR: return " & 0xFF";
      case BASE_TYPE_USHORT: return " & 0xFFFF";
      case BASE_TYPE_UINT: return " & 0xFFFFFFFFL";
      case BASE_TYPE_VECTOR:
        if (vectorelem) return DestinationMask(type.VectorType(), vectorelem);
        FLATBUFFERS_FALLTHROUGH();  // else fall thru
      default: return "";
    }
  }

  // Casts necessary to correctly read serialized data
  std::string DestinationCast(const Type &type) const {
    if (IsSeries(type)) {
      return DestinationCast(type.VectorType());
    } else {
      // Cast necessary to correctly read serialized unsigned values.
      if (type.base_type == BASE_TYPE_UINT) return "(long)";
    }
    return "";
  }

  // Cast statements for mutator method parameters.
  // In Java, parameters representing unsigned numbers need to be cast down to
  // their respective type. For example, a long holding an unsigned int value
  // would be cast down to int before being put onto the buffer. In C#, one cast
  // directly cast an Enum to its underlying type, which is essential before
  // putting it onto the buffer.
  std::string SourceCast(const Type &type, bool castFromDest) const {
    if (IsSeries(type)) {
      return SourceCast(type.VectorType(), castFromDest);
    } else {
      if (castFromDest) {
        if (type.base_type == BASE_TYPE_UINT)
          return "(int)";
        else if (type.base_type == BASE_TYPE_USHORT)
          return "(short)";
        else if (type.base_type == BASE_TYPE_UCHAR)
          return "(byte)";
      }
    }
    return "";
  }

  std::string SourceCast(const Type &type) const {
    return SourceCast(type, true);
  }

  std::string SourceCastBasic(const Type &type, bool castFromDest) const {
    return IsScalar(type.base_type) ? SourceCast(type, castFromDest) : "";
  }

  std::string SourceCastBasic(const Type &type) const {
    return SourceCastBasic(type, true);
  }

  std::string GenEnumDefaultValue(const FieldDef &field) const {
    auto &value = field.value;
    FLATBUFFERS_ASSERT(value.type.enum_def);
    auto &enum_def = *value.type.enum_def;
    auto enum_val = enum_def.FindByValue(value.constant);
    return enum_val ? (WrapInNameSpace(enum_def) + "." + enum_val->name)
                    : value.constant;
  }

  std::string GenDefaultValue(const FieldDef &field) const {
    auto &value = field.value;
    auto longSuffix = "L";
    switch (value.type.base_type) {
      case BASE_TYPE_BOOL: return value.constant == "0" ? "false" : "true";
      case BASE_TYPE_ULONG: {
        // Converts the ulong into its bits signed equivalent
        uint64_t defaultValue = StringToUInt(value.constant.c_str());
        return NumToString(static_cast<int64_t>(defaultValue)) + longSuffix;
      }
      case BASE_TYPE_UINT:
      case BASE_TYPE_LONG: return value.constant + longSuffix;
      default:
        if (IsFloat(value.type.base_type))
          return JavaFloatGen.GenFloatConstant(field);
        else
          return value.constant;
    }
  }

  std::string GenDefaultValueBasic(const FieldDef &field) const {
    auto &value = field.value;
    if (!IsScalar(value.type.base_type)) { return "0"; }
    return GenDefaultValue(field);
  }

  void GenEnum(EnumDef &enum_def, std::string *code_ptr) const {
    std::string &code = *code_ptr;
    if (enum_def.generated) return;

    // Generate enum definitions of the form:
    // public static (final) int name = value;
    // In Java, we use ints rather than the Enum feature, because we want them
    // to map directly to how they're used in C/C++ and file formats.
    // That, and Java Enums are expensive, and not universally liked.
    GenComment(enum_def.doc_comment, code_ptr, &comment_config);

    if (enum_def.attributes.Lookup("private")) {
      // For Java, we leave the enum unmarked to indicate package-private
      // For C# we mark the enum as internal
    } else {
      code += "public ";
    }
    code += "final class " + enum_def.name;
    code += " {\n";
    code += "  private " + enum_def.name + "() { }\n";
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      GenComment(ev.doc_comment, code_ptr, &comment_config, "  ");
      code += "  public static final ";
      code += GenTypeBasic(enum_def.underlying_type);
      code += " ";
      code += ev.name + " = ";
      code += enum_def.ToString(ev);
      code += ";\n";
    }

    // Generate a generate string table for enum values.
    // We do not do that for C# where this functionality is native.
    // Problem is, if values are very sparse that could generate really big
    // tables. Ideally in that case we generate a map lookup instead, but for
    // the moment we simply don't output a table at all.
    auto range = enum_def.Distance();
    // Average distance between values above which we consider a table
    // "too sparse". Change at will.
    static const uint64_t kMaxSparseness = 5;
    if (range / static_cast<uint64_t>(enum_def.size()) < kMaxSparseness) {
      code += "\n  public static final String";
      code += "[] names = { ";
      auto val = enum_def.Vals().front();
      for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end();
           ++it) {
        auto ev = *it;
        for (auto k = enum_def.Distance(val, ev); k > 1; --k) code += "\"\", ";
        val = ev;
        code += "\"" + (*it)->name + "\", ";
      }
      code += "};\n\n";
      code += "  public static ";
      code += "String";
      code += " " + MakeCamel("name", false);
      code += "(int e) { return names[e";
      if (enum_def.MinValue()->IsNonZero())
        code += " - " + enum_def.MinValue()->name;
      code += "]; }\n";
    }

    // Close the class
    code += "}";
    // Java does not need the closing semi-colon on class definitions.
    code += "";
    code += "\n\n";
  }

  // Returns the function name that is able to read a value of the given type.
  std::string GenGetter(const Type &type) const {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "__string";
      case BASE_TYPE_STRUCT: return "__struct";
      case BASE_TYPE_UNION: return "__union";
      case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
      case BASE_TYPE_ARRAY: return GenGetter(type.VectorType());
      default: {
        std::string getter = "bb.get";
        if (type.base_type == BASE_TYPE_BOOL) {
          getter = "0!=" + getter;
        } else if (GenTypeBasic(type) != "byte") {
          getter += MakeCamel(GenTypeBasic(type));
        }
        return getter;
      }
    }
  }

  // Returns the function name that is able to read a value of the given type.
  std::string GenGetterForLookupByKey(flatbuffers::FieldDef *key_field,
                                      const std::string &data_buffer,
                                      const char *num = nullptr) const {
    auto type = key_field->value.type;
    auto dest_mask = DestinationMask(type, true);
    auto dest_cast = DestinationCast(type);
    auto getter = data_buffer + ".get";
    if (GenTypeBasic(type) != "byte") {
      getter += MakeCamel(GenTypeBasic(type));
    }
    getter = dest_cast + getter + "(" + GenOffsetGetter(key_field, num) + ")" +
             dest_mask;
    return getter;
  }

  // Direct mutation is only allowed for scalar fields.
  // Hence a setter method will only be generated for such fields.
  std::string GenSetter(const Type &type) const {
    if (IsScalar(type.base_type)) {
      std::string setter = "bb.put";
      if (GenTypeBasic(type) != "byte" && type.base_type != BASE_TYPE_BOOL) {
        setter += MakeCamel(GenTypeBasic(type));
      }
      return setter;
    } else {
      return "";
    }
  }

  // Returns the method name for use with add/put calls.
  std::string GenMethod(const Type &type) const {
    return IsScalar(type.base_type) ? MakeCamel(GenTypeBasic(type))
                                    : (IsStruct(type) ? "Struct" : "Offset");
  }

  // Recursively generate arguments for a constructor, to deal with nested
  // structs.
  void GenStructArgs(const StructDef &struct_def, std::string *code_ptr,
                     const char *nameprefix, size_t array_count = 0) const {
    std::string &code = *code_ptr;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      const auto &field_type = field.value.type;
      const auto array_field = IsArray(field_type);
      const auto &type = array_field ? field_type.VectorType()
                                     : DestinationType(field_type, false);
      const auto array_cnt = array_field ? (array_count + 1) : array_count;
      if (IsStruct(type)) {
        // Generate arguments for a struct inside a struct. To ensure names
        // don't clash, and to make it obvious these arguments are constructing
        // a nested struct, prefix the name with the field name.
        GenStructArgs(*field_type.struct_def, code_ptr,
                      (nameprefix + (field.name + "_")).c_str(), array_cnt);
      } else {
        code += ", ";
        code += GenTypeBasic(type);
        for (size_t i = 0; i < array_cnt; i++) code += "[]";
        code += " ";
        code += nameprefix;
        code += MakeCamel(field.name, false);
      }
    }
  }

  // Recusively generate struct construction statements of the form:
  // builder.putType(name);
  // and insert manual padding.
  void GenStructBody(const StructDef &struct_def, std::string *code_ptr,
                     const char *nameprefix, size_t index = 0,
                     bool in_array = false) const {
    std::string &code = *code_ptr;
    std::string indent((index + 1) * 2, ' ');
    code += indent + "  builder.prep(";
    code += NumToString(struct_def.minalign) + ", ";
    code += NumToString(struct_def.bytesize) + ");\n";
    for (auto it = struct_def.fields.vec.rbegin();
         it != struct_def.fields.vec.rend(); ++it) {
      auto &field = **it;
      const auto &field_type = field.value.type;
      if (field.padding) {
        code += indent + "  builder.pad(";
        code += NumToString(field.padding) + ");\n";
      }
      if (IsStruct(field_type)) {
        GenStructBody(*field_type.struct_def, code_ptr,
                      (nameprefix + (field.name + "_")).c_str(), index,
                      in_array);
      } else {
        const auto &type =
            IsArray(field_type) ? field_type.VectorType() : field_type;
        const auto index_var = "_idx" + NumToString(index);
        if (IsArray(field_type)) {
          code += indent + "  for (int " + index_var + " = ";
          code += NumToString(field_type.fixed_length);
          code += "; " + index_var + " > 0; " + index_var + "--) {\n";
          in_array = true;
        }
        if (IsStruct(type)) {
          GenStructBody(*field_type.struct_def, code_ptr,
                        (nameprefix + (field.name + "_")).c_str(), index + 1,
                        in_array);
        } else {
          code += IsArray(field_type) ? "  " : "";
          code += indent + "  builder.put";
          code += GenMethod(type) + "(";
          code += SourceCast(type);
          auto argname = nameprefix + MakeCamel(field.name, false);
          code += argname;
          size_t array_cnt = index + (IsArray(field_type) ? 1 : 0);
          for (size_t i = 0; in_array && i < array_cnt; i++) {
            code += "[_idx" + NumToString(i) + "-1]";
          }
          code += ");\n";
        }
        if (IsArray(field_type)) { code += indent + "  }\n"; }
      }
    }
  }

  std::string GenByteBufferLength(const char *bb_name) const {
    std::string bb_len = bb_name;
    bb_len += ".capacity()";
    return bb_len;
  }

  std::string GenOffsetGetter(flatbuffers::FieldDef *key_field,
                              const char *num = nullptr) const {
    std::string key_offset = "";
    key_offset += "__offset(" + NumToString(key_field->value.offset) + ", ";
    if (num) {
      key_offset += num;
      key_offset += ", _bb)";
    } else {
      key_offset += GenByteBufferLength("bb");
      key_offset += " - tableOffset, bb)";
    }
    return key_offset;
  }

  std::string GenLookupKeyGetter(flatbuffers::FieldDef *key_field) const {
    std::string key_getter = "      ";
    key_getter += "int tableOffset = ";
    key_getter += "__indirect(vectorLocation + 4 * (start + middle)";
    key_getter += ", bb);\n      ";
    if (key_field->value.type.base_type == BASE_TYPE_STRING) {
      key_getter += "int comp = ";
      key_getter += "compareStrings(";
      key_getter += GenOffsetGetter(key_field);
      key_getter += ", byteKey, bb);\n";
    } else {
      auto get_val = GenGetterForLookupByKey(key_field, "bb");
      key_getter += GenTypeNameDest(key_field->value.type) + " val = ";
      key_getter += get_val + ";\n";
      key_getter += "      int comp = val > key ? 1 : val < key ? -1 : 0;\n";
    }
    return key_getter;
  }

  std::string GenKeyGetter(flatbuffers::FieldDef *key_field) const {
    std::string key_getter = "";
    auto data_buffer = "_bb";
    if (key_field->value.type.base_type == BASE_TYPE_STRING) {
      key_getter += " return ";
      key_getter += "";
      key_getter += "compareStrings(";
      key_getter += GenOffsetGetter(key_field, "o1") + ", ";
      key_getter += GenOffsetGetter(key_field, "o2") + ", " + data_buffer + ")";
      key_getter += ";";
    } else {
      auto field_getter = GenGetterForLookupByKey(key_field, data_buffer, "o1");
      key_getter +=
          "\n    " + GenTypeNameDest(key_field->value.type) + " val_1 = ";
      key_getter +=
          field_getter + ";\n    " + GenTypeNameDest(key_field->value.type);
      key_getter += " val_2 = ";
      field_getter = GenGetterForLookupByKey(key_field, data_buffer, "o2");
      key_getter += field_getter + ";\n";
      key_getter += "    return val_1 > val_2 ? 1 : val_1 < val_2 ? -1 : 0;\n ";
    }
    return key_getter;
  }

  void GenStruct(StructDef &struct_def, std::string *code_ptr) const {
    if (struct_def.generated) return;
    std::string &code = *code_ptr;

    // Generate a struct accessor class, with methods of the form:
    // public type name() { return bb.getType(i + offset); }
    // or for tables of the form:
    // public type name() {
    //   int o = __offset(offset); return o != 0 ? bb.getType(o + i) : default;
    // }
    GenComment(struct_def.doc_comment, code_ptr, &comment_config);
    if (struct_def.attributes.Lookup("private")) {
      // For Java, we leave the struct unmarked to indicate package-private
      // For C# we mark the struct as internal
    } else {
      code += "public ";
    }
    code += "final ";
    code += "class " + struct_def.name;
    code += " extends ";
    code += struct_def.fixed ? "Struct" : "Table";
    code += " {\n";

    if (!struct_def.fixed) {
      // Generate verson check method.
      // Force compile time error if not using the same version runtime.
      code += "  public static void ValidateVersion() {";
      code += " Constants.";
      code += "FLATBUFFERS_1_12_0(); ";
      code += "}\n";

      // Generate a special accessor for the table that when used as the root
      // of a FlatBuffer
      std::string method_name = "getRootAs" + struct_def.name;
      std::string method_signature =
          "  public static " + struct_def.name + " " + method_name;

      // create convenience method that doesn't require an existing object
      code += method_signature + "(ByteBuffer _bb) ";
      code += "{ return " + method_name + "(_bb, new " + struct_def.name +
              "()); }\n";

      // create method that allows object reuse
      code +=
          method_signature + "(ByteBuffer _bb, " + struct_def.name + " obj) { ";
      code += "_bb.order(ByteOrder.LITTLE_ENDIAN); ";
      code += "return (obj.__assign(_bb.getInt(_bb.";
      code += "position()";
      code += ") + _bb.";
      code += "position()";
      code += ", _bb)); }\n";
      if (parser_.root_struct_def_ == &struct_def) {
        if (parser_.file_identifier_.length()) {
          // Check if a buffer has the identifier.
          code += "  public static ";
          code += "boolean " + struct_def.name;
          code += "BufferHasIdentifier(ByteBuffer _bb) { return ";
          code += "__has_identifier(_bb, \"";
          code += parser_.file_identifier_;
          code += "\"); }\n";
        }
      }
    }
    // Generate the __init method that sets the field in a pre-existing
    // accessor object. This is to allow object reuse.
    code += "  public void __init(int _i, ByteBuffer _bb) ";
    code += "{ ";
    code += "__reset(_i, _bb); ";
    code += "}\n";
    code +=
        "  public " + struct_def.name + " __assign(int _i, ByteBuffer _bb) ";
    code += "{ __init(_i, _bb); return this; }\n\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      GenComment(field.doc_comment, code_ptr, &comment_config, "  ");
      std::string type_name = GenTypeGet(field.value.type);
      std::string type_name_dest = GenTypeNameDest(field.value.type);
      std::string conditional_cast = "";
      std::string optional = "";
      std::string dest_mask = DestinationMask(field.value.type, true);
      std::string dest_cast = DestinationCast(field.value.type);
      std::string src_cast = SourceCast(field.value.type);
      std::string method_start =
          "  public " +
          (field.required ? "" : GenNullableAnnotation(field.value.type)) +
          GenPureAnnotation(field.value.type) + type_name_dest + optional +
          " " + MakeCamel(field.name, false);
      std::string obj = "obj";

      // Most field accessors need to retrieve and test the field offset first,
      // this is the prefix code for that:
      auto offset_prefix =
          IsArray(field.value.type)
              ? " { return "
              : (" { int o = __offset(" + NumToString(field.value.offset) +
                 "); return o != 0 ? ");
      // Generate the accessors that don't do object reuse.
      if (field.value.type.base_type == BASE_TYPE_STRUCT) {
        // Calls the accessor that takes an accessor object with a new object.
        code += method_start + "() { return ";
        code += MakeCamel(field.name, false);
        code += "(new ";
        code += type_name + "()); }\n";
      } else if (field.value.type.base_type == BASE_TYPE_VECTOR &&
                 field.value.type.element == BASE_TYPE_STRUCT) {
        // Accessors for vectors of structs also take accessor objects, this
        // generates a variant without that argument.
        code += method_start + "(int j) { return ";
        code += MakeCamel(field.name, false);
        code += "(new " + type_name + "(), j); }\n";
      }

      std::string getter = dest_cast + GenGetter(field.value.type);
      code += method_start;
      std::string default_cast = "";
      std::string member_suffix = "; ";
      if (IsScalar(field.value.type.base_type)) {
        code += "()";
        member_suffix += "";
        if (struct_def.fixed) {
          code += " { return " + getter;
          code += "(bb_pos + ";
          code += NumToString(field.value.offset) + ")";
          code += dest_mask;
        } else {
          code += offset_prefix + getter;
          code += "(o + bb_pos)" + dest_mask;
          code += " : " + default_cast;
          code += GenDefaultValue(field);
        }
      } else {
        switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT:
            code += "(" + type_name + " obj)";
            if (struct_def.fixed) {
              code += " { return " + obj + ".__assign(";
              code += "bb_pos + " + NumToString(field.value.offset) + ", ";
              code += "bb)";
            } else {
              code += offset_prefix + conditional_cast;
              code += obj + ".__assign(";
              code += field.value.type.struct_def->fixed
                          ? "o + bb_pos"
                          : "__indirect(o + bb_pos)";
              code += ", bb) : null";
            }
            break;
          case BASE_TYPE_STRING:
            code += "()";
            member_suffix += "";
            code += offset_prefix + getter + "(o + ";
            code += "bb_pos) : null";
            break;
          case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();  // fall thru
          case BASE_TYPE_VECTOR: {
            auto vectortype = field.value.type.VectorType();
            code += "(";
            if (vectortype.base_type == BASE_TYPE_STRUCT) {
              code += type_name + " obj, ";
              getter = obj + ".__assign";
            } else if (vectortype.base_type == BASE_TYPE_UNION) {
              code += type_name + " obj, ";
            }
            code += "int j)";
            const auto body = offset_prefix + conditional_cast + getter + "(";
            if (vectortype.base_type == BASE_TYPE_UNION) {
              code += body + "obj, ";
            } else {
              code += body;
            }
            std::string index;
            if (IsArray(field.value.type)) {
              index += "bb_pos + " + NumToString(field.value.offset) + " + ";
            } else {
              index += "__vector(o) + ";
            }
            index += "j * " + NumToString(InlineSize(vectortype));
            if (vectortype.base_type == BASE_TYPE_STRUCT) {
              code += vectortype.struct_def->fixed
                          ? index
                          : "__indirect(" + index + ")";
              code += ", bb";
            } else {
              code += index;
            }
            code += ")" + dest_mask;
            if (!IsArray(field.value.type)) {
              code += " : ";
              code +=
                  field.value.type.element == BASE_TYPE_BOOL
                      ? "false"
                      : (IsScalar(field.value.type.element) ? default_cast + "0"
                                                            : "null");
            }

            break;
          }
          case BASE_TYPE_UNION:
            code += "(" + type_name + " obj)" + offset_prefix + getter;
            code += "(obj, o + bb_pos) : null";
            break;
          default: FLATBUFFERS_ASSERT(0);
        }
      }
      code += member_suffix;
      code += "}\n";
      if (field.value.type.base_type == BASE_TYPE_VECTOR) {
        code += "  public int " + MakeCamel(field.name, false);
        code += "Length";
        code += "()";
        code += offset_prefix;
        code += "__vector_len(o) : 0; ";
        code += "";
        code += "}\n";
        // See if we should generate a by-key accessor.
        if (field.value.type.element == BASE_TYPE_STRUCT &&
            !field.value.type.struct_def->fixed) {
          auto &sd = *field.value.type.struct_def;
          auto &fields = sd.fields.vec;
          for (auto kit = fields.begin(); kit != fields.end(); ++kit) {
            auto &key_field = **kit;
            if (key_field.key) {
              auto qualified_name = WrapInNameSpace(sd);
              code += "  public " + qualified_name + " ";
              code += MakeCamel(field.name, false) + "ByKey(";
              code += GenTypeNameDest(key_field.value.type) + " key)";
              code += offset_prefix;
              code += qualified_name + ".__lookup_by_key(";
              code += "null, ";
              code += "__vector(o), key, ";
              code += "bb) : null; ";
              code += "}\n";
              code += "  public " + qualified_name + " ";
              code += MakeCamel(field.name, false) + "ByKey(";
              code += qualified_name + " obj, ";
              code += GenTypeNameDest(key_field.value.type) + " key)";
              code += offset_prefix;
              code += qualified_name + ".__lookup_by_key(obj, ";
              code += "__vector(o), key, ";
              code += "bb) : null; ";
              code += "}\n";
              break;
            }
          }
        }
      }
      // Generate the accessors for vector of structs with vector access object
      if (field.value.type.base_type == BASE_TYPE_VECTOR) {
        std::string vector_type_name;
        const auto &element_base_type = field.value.type.VectorType().base_type;
        if (IsScalar(element_base_type)) {
          vector_type_name = MakeCamel(type_name, true) + "Vector";
        } else if (element_base_type == BASE_TYPE_STRING) {
          vector_type_name = "StringVector";
        } else if (element_base_type == BASE_TYPE_UNION) {
          vector_type_name = "UnionVector";
        } else {
          vector_type_name = type_name + ".Vector";
        }
        auto vector_method_start = GenNullableAnnotation(field.value.type) +
                                   "  public " + vector_type_name + optional +
                                   " " + MakeCamel(field.name, false) +
                                   "Vector";
        code += vector_method_start + "() { return ";
        code += MakeCamel(field.name, false) + "Vector";
        code += "(new " + vector_type_name + "()); }\n";
        code += vector_method_start + "(" + vector_type_name + " obj)";
        code += offset_prefix + conditional_cast + obj + ".__assign(";
        code += "__vector(o), ";
        if (!IsScalar(element_base_type)) {
          auto vectortype = field.value.type.VectorType();
          code += NumToString(InlineSize(vectortype)) + ", ";
        }
        code += "bb) : null" + member_suffix + "}\n";
      }
      // Generate a ByteBuffer accessor for strings & vectors of scalars.
      if ((field.value.type.base_type == BASE_TYPE_VECTOR &&
           IsScalar(field.value.type.VectorType().base_type)) ||
          field.value.type.base_type == BASE_TYPE_STRING) {
        code += "  public ByteBuffer ";
        code += MakeCamel(field.name, false);
        code += "AsByteBuffer() { return ";
        code += "__vector_as_bytebuffer(";
        code += NumToString(field.value.offset) + ", ";
        code += NumToString(field.value.type.base_type == BASE_TYPE_STRING
                                ? 1
                                : InlineSize(field.value.type.VectorType()));
        code += "); }\n";
        code += "  public ByteBuffer ";
        code += MakeCamel(field.name, false);
        code += "InByteBuffer(ByteBuffer _bb) { return ";
        code += "__vector_in_bytebuffer(_bb, ";
        code += NumToString(field.value.offset) + ", ";
        code += NumToString(field.value.type.base_type == BASE_TYPE_STRING
                                ? 1
                                : InlineSize(field.value.type.VectorType()));
        code += "); }\n";
      }
      // generate object accessors if is nested_flatbuffer
      if (field.nested_flatbuffer) {
        auto nested_type_name = WrapInNameSpace(*field.nested_flatbuffer);
        auto nested_method_name =
            MakeCamel(field.name, false) + "As" + field.nested_flatbuffer->name;
        auto get_nested_method_name = nested_method_name;
        code += "  public " + nested_type_name + " ";
        code += nested_method_name + "() { return ";
        code +=
            get_nested_method_name + "(new " + nested_type_name + "()); }\n";
        code += "  public " + nested_type_name + " ";
        code += get_nested_method_name + "(";
        code += nested_type_name + " obj";
        code += ") { int o = __offset(";
        code += NumToString(field.value.offset) + "); ";
        code += "return o != 0 ? " + conditional_cast + obj + ".__assign(";
        code += "";
        code += "__indirect(__vector(o)), ";
        code += "bb) : null; }\n";
      }
      // Generate mutators for scalar fields or vectors of scalars.
      if (parser_.opts.mutable_buffer) {
        auto is_series = (IsSeries(field.value.type));
        const auto &underlying_type =
            is_series ? field.value.type.VectorType() : field.value.type;
        // Boolean parameters have to be explicitly converted to byte
        // representation.
        auto setter_parameter = underlying_type.base_type == BASE_TYPE_BOOL
                                    ? "(byte)(" + field.name + " ? 1 : 0)"
                                    : field.name;
        auto mutator_prefix = MakeCamel("mutate", false);
        // A vector mutator also needs the index of the vector element it should
        // mutate.
        auto mutator_params = (is_series ? "(int j, " : "(") +
                              GenTypeNameDest(underlying_type) + " " +
                              field.name + ") { ";
        auto setter_index =
            is_series
                ? (IsArray(field.value.type)
                       ? "bb_pos + " + NumToString(field.value.offset)
                       : "__vector(o)") +
                      +" + j * " + NumToString(InlineSize(underlying_type))
                : (struct_def.fixed
                       ? "bb_pos + " + NumToString(field.value.offset)
                       : "o + bb_pos");
        if (IsScalar(underlying_type.base_type) && !IsUnion(field.value.type)) {
          code += "  public ";
          code += struct_def.fixed ? "void " : "boolean ";
          code += mutator_prefix + MakeCamel(field.name, true);
          code += mutator_params;
          if (struct_def.fixed) {
            code += GenSetter(underlying_type) + "(" + setter_index + ", ";
            code += src_cast + setter_parameter + "); }\n";
          } else {
            code += "int o = __offset(";
            code += NumToString(field.value.offset) + ");";
            code += " if (o != 0) { " + GenSetter(underlying_type);
            code += "(" + setter_index + ", " + src_cast + setter_parameter +
                    "); return true; } else { return false; } }\n";
          }
        }
      }
      if (parser_.opts.java_primitive_has_method &&
          IsScalar(field.value.type.base_type) && !struct_def.fixed) {
        auto vt_offset_constant = "  public static final int VT_" +
                                  MakeScreamingCamel(field.name) + " = " +
                                  NumToString(field.value.offset) + ";";

        code += vt_offset_constant;
        code += "\n";
      }
    }
    code += "\n";
    flatbuffers::FieldDef *key_field = nullptr;
    if (struct_def.fixed) {
      // create a struct constructor function
      code += "  public static " + GenOffsetType() + " ";
      code += "create";
      code += struct_def.name + "(FlatBufferBuilder builder";
      GenStructArgs(struct_def, code_ptr, "");
      code += ") {\n";
      GenStructBody(struct_def, code_ptr, "");
      code += "    return ";
      code += GenOffsetConstruct("builder." + std::string("offset()"));
      code += ";\n  }\n";
    } else {
      // Generate a method that creates a table in one go. This is only possible
      // when the table has no struct fields, since those have to be created
      // inline, and there's no way to do so in Java.
      bool has_no_struct_fields = true;
      int num_fields = 0;
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        if (IsStruct(field.value.type)) {
          has_no_struct_fields = false;
        } else {
          num_fields++;
        }
      }
      // JVM specifications restrict default constructor params to be < 255.
      // Longs and doubles take up 2 units, so we set the limit to be < 127.
      if (has_no_struct_fields && num_fields && num_fields < 127) {
        // Generate a table constructor of the form:
        // public static int createName(FlatBufferBuilder builder, args...)
        code += "  public static " + GenOffsetType() + " ";
        code += "create" + struct_def.name;
        code += "(FlatBufferBuilder builder";
        for (auto it = struct_def.fields.vec.begin();
             it != struct_def.fields.vec.end(); ++it) {
          auto &field = **it;
          if (field.deprecated) continue;
          code += ",\n      ";
          code += GenTypeBasic(DestinationType(field.value.type, false));
          code += " ";
          code += field.name;
          if (!IsScalar(field.value.type.base_type)) code += "Offset";
        }
        code += ") {\n    builder.";
        code += "startTable(";
        code += NumToString(struct_def.fields.vec.size()) + ");\n";
        for (size_t size = struct_def.sortbysize ? sizeof(largest_scalar_t) : 1;
             size; size /= 2) {
          for (auto it = struct_def.fields.vec.rbegin();
               it != struct_def.fields.vec.rend(); ++it) {
            auto &field = **it;
            if (!field.deprecated &&
                (!struct_def.sortbysize ||
                 size == SizeOf(field.value.type.base_type))) {
              code += "    " + struct_def.name + ".";
              code += "add";
              code += MakeCamel(field.name) + "(builder, " + field.name;
              if (!IsScalar(field.value.type.base_type)) code += "Offset";
              code += ");\n";
            }
          }
        }
        code += "    return " + struct_def.name + ".";
        code += "end" + struct_def.name;
        code += "(builder);\n  }\n\n";
      }
      // Generate a set of static methods that allow table construction,
      // of the form:
      // public static void addName(FlatBufferBuilder builder, short name)
      // { builder.addShort(id, name, default); }
      // Unlike the Create function, these always work.
      code += "  public static void start";
      code += struct_def.name;
      code += "(FlatBufferBuilder builder) { builder.";
      code += "startTable(";
      code += NumToString(struct_def.fields.vec.size()) + "); }\n";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        if (field.key) key_field = &field;
        code += "  public static void add";
        code += MakeCamel(field.name);
        code += "(FlatBufferBuilder builder, ";
        code += GenTypeBasic(DestinationType(field.value.type, false));
        auto argname = MakeCamel(field.name, false);
        if (!IsScalar(field.value.type.base_type)) argname += "Offset";
        code += " " + argname + ") { builder.add";
        code += GenMethod(field.value.type) + "(";
        code += NumToString(it - struct_def.fields.vec.begin()) + ", ";
        code += SourceCastBasic(field.value.type);
        code += argname;
        code += ", ";
        code += SourceCastBasic(field.value.type);
        code += GenDefaultValue(field);
        code += "); }\n";
        if (field.value.type.base_type == BASE_TYPE_VECTOR) {
          auto vector_type = field.value.type.VectorType();
          auto alignment = InlineAlignment(vector_type);
          auto elem_size = InlineSize(vector_type);
          if (!IsStruct(vector_type)) {
            // generate a method to create a vector from a java array.
            if ((vector_type.base_type == BASE_TYPE_CHAR ||
                 vector_type.base_type == BASE_TYPE_UCHAR)) {
              // Handle byte[] and ByteBuffers separately for Java
              code += "  public static " + GenVectorOffsetType() + " ";
              code += "create";
              code += MakeCamel(field.name);
              code += "Vector(FlatBufferBuilder builder, byte[] data) ";
              code += "{ return builder.createByteVector(data); }\n";

              code += "  public static " + GenVectorOffsetType() + " ";
              code += "create";
              code += MakeCamel(field.name);
              code += "Vector(FlatBufferBuilder builder, ByteBuffer data) ";
              code += "{ return builder.createByteVector(data); }\n";
            } else {
              code += "  public static " + GenVectorOffsetType() + " ";
              code += "create";
              code += MakeCamel(field.name);
              code += "Vector(FlatBufferBuilder builder, ";
              code += GenTypeBasic(vector_type) + "[] data) ";
              code += "{ builder.startVector(";
              code += NumToString(elem_size);
              code += ", data.length, ";
              code += NumToString(alignment);
              code += "); for (int i = data.";
              code += "length - 1; i >= 0; i--) builder.";
              code += "add";
              code += GenMethod(vector_type);
              code += "(";
              code += SourceCastBasic(vector_type, false);
              code += "data[i]";
              code += "); return ";
              code += "builder.endVector(); }\n";
            }
          }
          // Generate a method to start a vector, data to be added manually
          // after.
          code += "  public static void start";
          code += MakeCamel(field.name);
          code += "Vector(FlatBufferBuilder builder, int numElems) ";
          code += "{ builder.startVector(";
          code += NumToString(elem_size);
          code += ", numElems, " + NumToString(alignment);
          code += "); }\n";
        }
      }
      code += "  public static " + GenOffsetType() + " ";
      code += "end" + struct_def.name;
      code += "(FlatBufferBuilder builder) {\n    int o = builder.";
      code += "endTable();\n";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (!field.deprecated && field.required) {
          code += "    builder.required(o, ";
          code += NumToString(field.value.offset);
          code += ");  // " + field.name + "\n";
        }
      }
      code += "    return " + GenOffsetConstruct("o") + ";\n  }\n";
      if (parser_.root_struct_def_ == &struct_def) {
        std::string size_prefix[] = { "", "SizePrefixed" };
        for (int i = 0; i < 2; ++i) {
          code += "  public static void ";
          code += "finish" + size_prefix[i] + struct_def.name;
          code += "Buffer(FlatBufferBuilder builder, " + GenOffsetType();
          code += " offset) {";
          code += " builder.finish" + size_prefix[i] + "(offset";

          if (parser_.file_identifier_.length())
            code += ", \"" + parser_.file_identifier_ + "\"";
          code += "); }\n";
        }
      }
    }
    // Only generate key compare function for table,
    // because `key_field` is not set for struct
    if (struct_def.has_key && !struct_def.fixed) {
      FLATBUFFERS_ASSERT(key_field);
      code += "\n  @Override\n  protected int keysCompare(";
      code += "Integer o1, Integer o2, ByteBuffer _bb) {";
      code += GenKeyGetter(key_field);
      code += " }\n";

      code += "\n  public static " + struct_def.name;
      code += " __lookup_by_key(";
      code += struct_def.name + " obj, ";
      code += "int vectorLocation, ";
      code += GenTypeNameDest(key_field->value.type);
      code += " key, ByteBuffer bb) {\n";
      if (key_field->value.type.base_type == BASE_TYPE_STRING) {
        code += "    byte[] byteKey = ";
        code += "key.getBytes(java.nio.charset.StandardCharsets.UTF_8);\n";
      }
      code += "    int span = ";
      code += "bb.getInt(vectorLocation - 4);\n";
      code += "    int start = 0;\n";
      code += "    while (span != 0) {\n";
      code += "      int middle = span / 2;\n";
      code += GenLookupKeyGetter(key_field);
      code += "      if (comp > 0) {\n";
      code += "        span = middle;\n";
      code += "      } else if (comp < 0) {\n";
      code += "        middle++;\n";
      code += "        start += middle;\n";
      code += "        span -= middle;\n";
      code += "      } else {\n";
      code += "        return ";
      code += "(obj == null ? new " + struct_def.name + "() : obj)";
      code += ".__assign(tableOffset, bb);\n";
      code += "      }\n    }\n";
      code += "    return null;\n";
      code += "  }\n";
    }
    GenVectorAccessObject(struct_def, code_ptr);
    code += "}";
    code += "\n\n";
  }

  void GenVectorAccessObject(StructDef &struct_def,
                             std::string *code_ptr) const {
    auto &code = *code_ptr;
    // Generate a vector of structs accessor class.
    code += "\n";
    code += "  ";
    if (!struct_def.attributes.Lookup("private")) code += "public ";
    code += "static ";
    code += "final ";
    code += "class Vector extends ";
    code += "BaseVector {\n";

    // Generate the __assign method that sets the field in a pre-existing
    // accessor object. This is to allow object reuse.
    std::string method_indent = "    ";
    code += method_indent + "public Vector ";
    code += "__assign(int _vector, int _element_size, ByteBuffer _bb) { ";
    code += "__reset(_vector, _element_size, _bb); return this; }\n\n";

    auto type_name = struct_def.name;
    auto method_start = method_indent + "public " + type_name + " get";
    // Generate the accessors that don't do object reuse.
    code += method_start + "(int j) { return get";
    code += "(new " + type_name + "(), j); }\n";
    code += method_start + "(" + type_name + " obj, int j) { ";
    code += " return obj.__assign(";
    std::string index = "__element(j)";
    code += struct_def.fixed ? index : "__indirect(" + index + ", bb)";
    code += ", bb); }\n";
    // See if we should generate a by-key accessor.
    if (!struct_def.fixed) {
      auto &fields = struct_def.fields.vec;
      for (auto kit = fields.begin(); kit != fields.end(); ++kit) {
        auto &key_field = **kit;
        if (key_field.key) {
          auto nullable_annotation =
              parser_.opts.gen_nullable ? "@Nullable " : "";
          code += method_indent + nullable_annotation;
          code += "public " + type_name + " ";
          code += "getByKey(";
          code += GenTypeNameDest(key_field.value.type) + " key) { ";
          code += " return __lookup_by_key(null, ";
          code += "__vector(), key, ";
          code += "bb); ";
          code += "}\n";
          code += method_indent + nullable_annotation;
          code += "public " + type_name + " ";
          code += "getByKey(";
          code += type_name + " obj, ";
          code += GenTypeNameDest(key_field.value.type) + " key) { ";
          code += " return __lookup_by_key(obj, ";
          code += "__vector(), key, ";
          code += "bb); ";
          code += "}\n";
          break;
        }
      }
    }
    code += "  }\n";
  }

  // This tracks the current namespace used to determine if a type need to be
  // prefixed by its namespace
  const Namespace *cur_name_space_;
};
}  // namespace java

bool GenerateJava(const Parser &parser, const std::string &path,
                  const std::string &file_name) {
  java::JavaGenerator generator(parser, path, file_name);
  return generator.generate();
}

}  // namespace flatbuffers
