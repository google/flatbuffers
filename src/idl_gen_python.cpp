/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
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

#include <unordered_set>

namespace flatbuffers {

// Pedantic warning free version of toupper().
inline char ToUpper(char c) { return static_cast<char>(::toupper(c)); }

static std::string GeneratedFileName(const std::string &path,
                                     const std::string &file_name) {
  return path + file_name + "_generated.py";
}

namespace python {

static const auto DOCSTRING_TOKEN = "\"\"\"";  // Three double-quotes.

class PythonGenerator : public BaseGenerator {
 public:
  PythonGenerator(const Parser &parser, const std::string &path,
                  const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", "") {
    static const char *const keywords[] = {
      "False",   "None",     "True",     "and",    "as",   "assert", "break",
      "class",   "continue", "def",      "del",    "elif", "else",   "except",
      "finally", "for",      "from",     "global", "if",   "import", "in",
      "is",      "lambda",   "nonlocal", "not",    "or",   "pass",   "raise",
      "return",  "try",      "while",    "with",   "yield"
    };
    keywords_.insert(std::begin(keywords), std::end(keywords));
  }

  std::string GetQualifiedName(const Definition &def) {
    auto noext = flatbuffers::StripExtension(def.file);
    auto basename = flatbuffers::StripPath(noext);

    if (basename == file_name_) { return Name(def); }

    return basename + "." + Name(def);
  }

  void GenImports() {
    int num_includes = 0;
    for (auto it = parser_.included_files_.begin();
         it != parser_.included_files_.end(); ++it) {
      if (it->second.empty()) continue;
      auto noext = flatbuffers::StripExtension(it->second);
      auto basename = flatbuffers::StripPath(noext);

      std::string include_prefix = parser_.opts.include_prefix;
      if (!include_prefix.empty() && include_prefix[0] == '.') {
        include_prefix = "from " + include_prefix + " ";
      }

      code_ += "import " + parser_.opts.include_prefix +
               (parser_.opts.keep_include_path ? noext : basename) +
               "_generated";
      num_includes++;
    }
    if (num_includes) code_ += "";
  }

  std::string EscapeKeyword(const std::string &name) const {
    return keywords_.find(name) == keywords_.end() ? name : name + "_";
  }

  std::string Name(const Definition &def) const {
    return EscapeKeyword(def.name);
  }

  std::string TypeName(const FieldDef &field) const {
    return GenTypeGet(field.value.type);
  }

  void GenOffsetPrefix(const FieldDef &field,
                       const std::string &default_value) {
    code_ += "        offset = self._tab.Offset(self." +
             GenFieldOffsetName(field) + ")";
    code_ += "        if not offset:";
    code_ += "            return " + default_value;
    code_ += "";
  }

  void GenScalarFieldOfStruct(const FieldDef &field) {
    code_ += "        number_type = flatbuffers.number_types." +
             MakeCamel(GenTypeGet(field.value.type)) + "Flags";
    code_ += "        result = flatbuffers.encode.Get\\";
    code_ += "(number_type.packer_type, self._buf, " +
             NumToString(field.value.offset) + ")";
    code_ += "        return number_type.py_type(result)";
  }

  std::string GetDefaultValue(const flatbuffers::FieldDef &field) {
    if (field.value.type.base_type == BASE_TYPE_BOOL) {
      return field.value.constant == "0" ? "False" : "True";
    } else {
      return field.value.constant;
    }
  }

  void GenScalarFieldOfTable(const FieldDef &field) {
    std::string default_value = GetDefaultValue(field);

    GenOffsetPrefix(field, default_value);
    code_ += "        return \\";

    auto is_union_type_field = field.value.type.base_type == BASE_TYPE_UTYPE;
    if (is_union_type_field) {
      FLATBUFFERS_ASSERT(field.value.type.enum_def);

      code_ += Name(*field.value.type.enum_def) + "(\\";
    }

    code_ += GenGetter(field.value.type) + "offset + self._tab.Pos)\\";

    if (is_union_type_field) {
      code_ += ")";
    } else {
      code_ += "";
    }
  }

  // Get a struct by initializing a struct of the expected type.
  // Specific to Struct.
  void GenStructFieldOfStruct(const FieldDef &field) {
    code_.SetValue("FIELD_TYPE", TypeName(field));

    code_ += "        return {{FIELD_TYPE}}(self._buf, " +
             NumToString(field.value.offset) + ")";
  }

  // Get a struct by initializing a struct of the expected type.
  // Specific to Table.
  void GenStructFieldOfTable(const FieldDef &field) {
    GenOffsetPrefix(field, "None");

    if (field.value.type.struct_def->fixed) {
      code_ += "        offset_in_buf = offset + self._tab.Pos";
    } else {
      code_ +=
          "        offset_in_buf = self._tab.Indirect(offset + self._tab.Pos)";
    }

    code_.SetValue("FIELD_TYPE", TypeName(field));
    code_ += "        return {{FIELD_TYPE}}(self._tab.Bytes, offset_in_buf)";
  }

  // Get the value of a string.
  void GenStringField(const FieldDef &field) {
    GenOffsetPrefix(field, "None");

    code_ += "        return \\";
    code_ += GenGetter(field.value.type) + "offset + self._tab.Pos)";
  }

  // Get the value of a union from an object.
  void GenUnionField(const FieldDef &field) {
    code_.SetValue("FIELD_NAME", Name(field));

    code_ += "        union_table = self.{{FIELD_NAME}}_union";
    code_ += "";
    code_ += "        if union_table is None:";
    code_ += "            return None";
    code_ += "";
    code_ += "        value_type = self.{{FIELD_NAME}}_type.table_type";
    code_ += "        return value_type(union_table.Bytes, union_table.Pos)";
    code_ += "";

    code_ += "    @property";
    code_ += "    def {{FIELD_NAME}}_union(self):";

    GenOffsetPrefix(field, "None");

    code_ += "        union_table = flatbuffers.table.Table(bytearray(), 0)";
    code_ += "        " + GenGetter(field.value.type) + "union_table, offset)";
    code_ += "        return union_table";
  }

  // Get the value of a vector's struct member.
  void GenMemberOfVectorOfStruct(const FieldDef &field) {
    auto vectortype = field.value.type.VectorType();

    GenOffsetPrefix(field, "None");

    code_ += "        item_offset_in_vector = index * " +
             NumToString(InlineSize(vectortype));
    code_ += "        vector_offset = self._tab.Vector(offset)";
    if (vectortype.struct_def->fixed) {
      code_ +=
          "        item_offset_in_buf = vector_offset + item_offset_in_vector";
    } else {
      code_ +=
          "        item_offset_in_buf = self._tab.Indirect(vector_offset + "
          "item_offset_in_vector)";
    }

    code_ += "        return " + TypeName(field) +
             "(self._tab.Bytes, item_offset_in_buf)";
  }

  // Get the value of a vector's non-struct member. Uses a named return
  // argument to conveniently set the zero value for the result.
  void GetMemberOfVectorOfNonStruct(const FieldDef &field) {
    auto vectortype = field.value.type.VectorType();

    GenOffsetPrefix(field,
                    vectortype.base_type == BASE_TYPE_STRING ? "\"\"" : "0");

    code_ += "        item_offset_in_vector = index * " +
             NumToString(InlineSize(vectortype));
    code_ += "        vector_offset = self._tab.Vector(offset)";
    code_ +=
        "        item_offset_in_buf = vector_offset + item_offset_in_vector";
    code_ +=
        "        return " + GenGetter(field.value.type) + "item_offset_in_buf)";
  }

  // Returns a non-struct vector as a numpy array. Much faster
  // than iterating over the vector element by element.
  void GetVectorOfNonStructAsNumpy(const FieldDef &field) {
    auto vectortype = field.value.type.VectorType();

    // Currently, we only support accessing as numpy array if
    // the vector type is a scalar.
    if (!(IsScalar(vectortype.base_type))) { return; }

    GenOffsetPrefix(field,
                    vectortype.base_type == BASE_TYPE_STRING ? "\"\"" : "0");

    code_ += "        return self._tab.GetVectorAsNumpy(\\";
    code_ += "flatbuffers.number_types." +
             MakeCamel(GenTypeGet(field.value.type)) + "Flags, offset)";
  }

  void GenField(const FieldDef &field, bool owner_is_struct) {
    code_.SetValue("FIELD_NAME", Name(field));

    if (field.value.type.base_type == BASE_TYPE_VECTOR) {
      GenVectorField(field);
    } else {
      code_ += "    @property";
      code_ += "    def {{FIELD_NAME}}(self):";
      GenComment(field.doc_comment, "        ");

      if (IsScalar(field.value.type.base_type)) {
        if (owner_is_struct) {
          GenScalarFieldOfStruct(field);
        } else {
          GenScalarFieldOfTable(field);
        }
      } else {
        switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT:
            if (owner_is_struct) {
              GenStructFieldOfStruct(field);
            } else {
              GenStructFieldOfTable(field);
            }
            break;
          case BASE_TYPE_STRING: GenStringField(field); break;
          case BASE_TYPE_UNION: GenUnionField(field); break;
          default: FLATBUFFERS_ASSERT(0);
        }
      }
    }

    code_ += "";
  }

  void GenScalarFieldSetter(const FieldDef &field) {
    code_.SetValue("FIELD_NAME", Name(field));
    code_.SetValue("FIELD_TYPE", MakeCamel(GenTypeGet(field.value.type)));

    code_ += "    @{{FIELD_NAME}}.setter";
    code_ += "    def {{FIELD_NAME}}(self, value):";
    code_ +=
        "        number_type = flatbuffers.number_types.{{FIELD_TYPE}}Flags";
    code_ +=
        "        flatbuffers.number_types.enforce_number(value, number_type)";
    code_ +=
        "        flatbuffers.encode.Write(number_type.packer_type, "
        "self._buf, " +
        NumToString(field.value.offset) + ", value)";
    code_ += "";
  }

  // Get the length of a vector.
  void GetVectorLen(const FieldDef &field) {
    code_.SetValue("FIELD_NAME", Name(field));

    code_ += "    @property";
    code_ += "    def {{FIELD_NAME}}_length(self):";

    GenOffsetPrefix(field, "0");
    code_ += "        return self._tab.VectorLen(offset)";
  }

  void GenVectorField(const flatbuffers::FieldDef &field) {
    code_.SetValue("FIELD_NAME", Name(field));

    // TODO: Add overload which returns a complex vector object
    //       with __getattr__ and __len__ instead of these accessors.
    // Hence the _item suffix for the item accessor.
    code_ += "    def {{FIELD_NAME}}_item(self, index):";
    GenComment(field.doc_comment, "        ");

    auto vectortype = field.value.type.VectorType();
    if (vectortype.base_type == BASE_TYPE_STRUCT) {
      GenMemberOfVectorOfStruct(field);
    } else {
      GetMemberOfVectorOfNonStruct(field);

      // Currently, we only support accessing as numpy array if
      // the vector type is a scalar.
      if (IsScalar(vectortype.base_type)) {
        code_ += "";
        code_ += "    def {{FIELD_NAME}}_as_numpy(self):";
        GetVectorOfNonStructAsNumpy(field);
      }
    }

    code_ += "";
    GetVectorLen(field);
  }

  std::string Name(const EnumVal &ev) const { return EscapeKeyword(ev.name); }

  std::string TrimBlankLinesAtEndOfFile(std::string code) {
    auto end_of_code = std::find_if(code.rbegin(), code.rend(),
                                    [](char value) { return value != '\n'; });
    FLATBUFFERS_ASSERT(end_of_code != code.rend());

    code.erase(end_of_code.base(), code.end());
    code.append("\n");
    return code;
  }

  // Iterate through all definitions we haven't generate code for (enums,
  // structs, and tables) and output them to a single file.
  bool generate() {
    code_.Clear();
    code_ += "# " + std::string(FlatBuffersGeneratedWarning());
    code_ += "";

    code_ += "import flatbuffers";
    if (!parser_.enums_.vec.empty()) { code_ += "import enum"; }
    code_ += "";

    if (parser_.opts.include_dependence_headers) { GenImports(); }

    code_ += "";

    // Generate code for all the enum declarations.
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      const auto &enum_def = **it;
      if (!enum_def.generated) { GenEnum(enum_def); }
    }

    // Generate code for all structs, then all tables.
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      const auto &struct_def = **it;
      if (struct_def.fixed && !struct_def.generated) { GenStruct(struct_def); }
    }
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      const auto &struct_def = **it;
      if (!struct_def.fixed && !struct_def.generated) { GenTable(struct_def); }
    }

    const auto file_path = GeneratedFileName(path_, file_name_);
    const auto final_code = TrimBlankLinesAtEndOfFile(code_.ToString());
    return SaveFile(file_path.c_str(), final_code, false);
  }

 private:
  CodeWriter code_;

  std::unordered_set<std::string> keywords_;

  void GenComment(const std::vector<std::string> &dc, const char *prefix = "") {
    if (dc.size() == 0) { return; }

    CommentConfig config;
    config.first_line = DOCSTRING_TOKEN;
    config.content_line_prefix = "";
    config.last_line = DOCSTRING_TOKEN;

    std::string text;

    ::flatbuffers::GenComment(dc, &text, &config, prefix);
    code_ += text;
  }

  std::string GenTypeBasic(const Type &type) const {
    static const char *ctypename[] = {
    // clang-format off
            #define FLATBUFFERS_TD(ENUM, IDLTYPE, \
                CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, RTYPE) \
                #PTYPE,
                FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
            #undef FLATBUFFERS_TD
      // clang-format on
    };
    return ctypename[type.base_type];
  }

  std::string GenTypePointer(const Type &type) const {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "string";
      case BASE_TYPE_VECTOR: return GenTypeGet(type.VectorType());
      case BASE_TYPE_STRUCT: return type.struct_def->name;
      case BASE_TYPE_UNION:
        // fall through
      default: return "*flatbuffers.Table";
    }
  }

  // Return a number type for any type (scalar/pointer) specifically for
  // using a flatbuffer.
  std::string GenTypeGet(const Type &type) const {
    if (IsScalar(type.base_type)) {
      return GenTypeBasic(type);
    } else {
      return GenTypePointer(type);
    }
  }

  std::string GenEnumDecl(const EnumDef &enum_def) const {
    return "class " + Name(enum_def) + "(enum.IntEnum):";
  }

  std::string StripUnionType(const std::string &name) {
    return name.substr(0, name.size() - strlen(UnionTypeFieldSuffix()));
  }

  // Generate an enum declaration,
  // an enum string lookup table,
  // and an enum array of values
  void GenEnum(const EnumDef &enum_def) {
    code_.SetValue("ENUM_NAME", Name(enum_def));
    code_.SetValue("SEP", "");

    GenComment(enum_def.doc_comment, "    ");
    code_ += GenEnumDecl(enum_def);

    int64_t anyv = 0;
    const EnumVal *minv = nullptr, *maxv = nullptr;
    for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
         ++it) {
      const auto &ev = **it;

      GenComment(ev.doc_comment, "    ");
      code_.SetValue("KEY", Name(ev));
      code_.SetValue("VALUE", NumToString(ev.value));
      code_ += "    {{KEY}} = {{VALUE}}";

      minv = !minv || minv->value > ev.value ? &ev : minv;
      maxv = !maxv || maxv->value < ev.value ? &ev : maxv;
      anyv |= ev.value;
    }

    FLATBUFFERS_ASSERT(minv && maxv);

    if (enum_def.attributes.Lookup("bit_flags")) {
      code_.SetValue("KEY", "NONE");
      code_.SetValue("VALUE", "0");
      code_ += "    {{KEY}} = {{VALUE}}";

      code_.SetValue("KEY", "ANY");
      code_.SetValue("VALUE", NumToString(anyv));
      code_ += "    {{KEY}} = {{VALUE}}";
    } else {  // MIN & MAX are useless for bit_flags
      code_.SetValue("KEY", "MIN");
      code_.SetValue("VALUE", minv->name);
      code_ += "    {{KEY}} = {{VALUE}}";

      code_.SetValue("KEY", "MAX");
      code_.SetValue("VALUE", maxv->name);
      code_ += "    {{KEY}} = {{VALUE}}";
    }

    code_ += "";

    // Generate type traits for unions to map from a type to union enum value.
    if (enum_def.is_union && !enum_def.uses_type_aliases) {
      code_ += "    @property";
      code_ += "    def table_type(self):";
      code_ += "        return self.table_types()[self]";
      code_ += "";

      code_ += "    @classmethod";
      code_ += "    def table_types(cls):";
      code_ += "        return {";

      code_.SetValue("SEP", "");

      for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
           ++it) {
        const auto &ev = **it;

        if (ev.union_type.base_type != BASE_TYPE_STRUCT) { continue; }
        FLATBUFFERS_ASSERT(ev.union_type.struct_def);

        auto name = Name(ev);
        code_.SetValue("VALUE_NAME", name);
        code_.SetValue("VALUE_TYPE",
                       GetQualifiedName(*ev.union_type.struct_def));

        code_ += "{{SEP}}            cls.{{VALUE_NAME}}: {{VALUE_TYPE}}\\";
        code_.SetValue("SEP", ",\n");
      }

      code_ += "";
      code_ += "        }";
      code_ += "";
    }

    code_ += "";
  }

  std::string GenFieldOffsetName(const FieldDef &field) {
    std::string uname = Name(field);
    std::transform(uname.begin(), uname.end(), uname.begin(), ToUpper);
    return "VT_" + uname;
  }

  void GenFullyQualifiedNameGetter(const StructDef &struct_def,
                                   const std::string &name) {
    if (!parser_.opts.generate_name_strings) { return; }
    auto fullname = struct_def.defined_namespace->GetFullyQualifiedName(name);
    code_.SetValue("NAME", fullname);
    code_ += "    @staticmethod";
    code_ += "    def fully_qualified_name():";
    code_ += "        return \"{{NAME}}\"";
    code_ += "";
  }

  // Generate an accessor struct, builder structs & function for a table.
  void GenTable(const StructDef &struct_def) {
    // Generate an accessor struct, with methods of the form:
    // type name() const { return GetField<type>(offset, defaultval); }
    code_.SetValue("STRUCT_NAME", Name(struct_def));
    code_ += "class {{STRUCT_NAME}}(object):";

    GenComment(struct_def.doc_comment, "    ");

    // Generate field id constants.
    if (struct_def.fields.vec.size() > 0) {
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        const auto &field = **it;
        if (field.deprecated) {
          // Deprecated fields won't be accessible.
          continue;
        }

        code_.SetValue("OFFSET_NAME", GenFieldOffsetName(field));
        code_.SetValue("OFFSET_VALUE", NumToString(field.value.offset));
        code_ += "    {{OFFSET_NAME}} = {{OFFSET_VALUE}}";
      }
      code_ += "";
    }

    code_ += "    def __init__(self, buf, offset):";
    code_ += "        self._tab = flatbuffers.table.Table(buf, offset)";
    code_ += "";

    code_ += "    @classmethod";
    code_ += "    def from_root(cls, buf, offset=0):";
    code_ +=
        "        relative_table_offset = "
        "flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)";
    code_ +=
        "        return {{STRUCT_NAME}}(buf, offset + relative_table_offset)";
    code_ += "";

    GenFullyQualifiedNameGetter(struct_def, Name(struct_def));

    // Generate the accessors.
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const auto &field = **it;
      if (field.deprecated) {
        // Deprecated fields won't be accessible.
        continue;
      }

      code_.SetValue("FIELD_NAME", Name(field));
      GenField(field, false);
    }

    GenCreateFunction(struct_def);

    code_ += "";

    GenBuilders(struct_def);
  }

  void GenCreateFunction(const StructDef &struct_def) {
    // Generate a convenient create_x function that uses the above builder
    // to create a table in one go.

    code_ += "    @staticmethod";
    code_ += "    def create(fbb\\";

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const auto &field = **it;
      if (!field.deprecated && !IsStruct(field.value.type)) {
        code_.SetValue("PARAM_NAME", Name(field));

        code_ += ", {{PARAM_NAME}}\\";
      }
    }
    code_ += "):";

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const auto &field = **it;

      if (field.value.type.base_type != BASE_TYPE_STRING &&
          field.value.type.base_type != BASE_TYPE_VECTOR) {
        continue;
      }

      GenDirectBuilder(field);
    }

    code_ += "        builder = {{STRUCT_NAME}}Builder(fbb)";

    for (size_t size = struct_def.sortbysize ? sizeof(largest_scalar_t) : 1;
         size; size /= 2) {
      for (auto it = struct_def.fields.vec.rbegin();
           it != struct_def.fields.vec.rend(); ++it) {
        const auto &field = **it;
        if (!field.deprecated && !IsStruct(field.value.type) &&
            (!struct_def.sortbysize ||
             size == SizeOf(field.value.type.base_type))) {
          code_.SetValue("FIELD_NAME", Name(field));
          code_ += "        builder.add_{{FIELD_NAME}}({{FIELD_NAME}})";
        }
      }
    }
    code_ += "        return builder.finish()";
    code_ += "";
  }

  void GenBuilders(const StructDef &struct_def) {
    code_.SetValue("STRUCT_NAME", Name(struct_def));

    // Generate a builder struct:
    code_ += "class {{STRUCT_NAME}}Builder(object):";
    code_ += "    def __init__(self, builder):";
    code_ += "        self._builder = builder";
    code_ += "        builder.StartObject(" +
             NumToString(struct_def.fields.vec.size()) + ")";
    code_ += "";

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const auto &field = **it;
      if (field.deprecated) { continue; }

      const bool is_scalar = IsScalar(field.value.type.base_type);

      std::string offset = GenFieldOffsetName(field);
      std::string name = Name(field);
      std::string value = is_scalar ? field.value.constant : "";

      code_.SetValue("FIELD_NAME", Name(field));
      code_.SetValue("FIELD_TYPE", TypeName(field));
      code_.SetValue("SLOT", NumToString(it - struct_def.fields.vec.begin()));

      code_ += "    def add_{{FIELD_NAME}}(self, {{FIELD_NAME}}):";
      if (IsStruct(field.value.type)) {
        code_ +=
            "        self._builder.Prep({{FIELD_TYPE}}.MIN_ALIGN, "
            "{{FIELD_TYPE}}.BYTE_SIZE)";
        code_ += "        self._builder.Pad({{FIELD_TYPE}}.BYTE_SIZE)";
        code_ +=
            "        {{FIELD_NAME}}.copy_into(self._builder.Bytes, "
            "self._builder.Head())";
        code_ +=
            "        self._builder.PrependStructSlot({{SLOT}}, "
            "self._builder.Offset(), 0)";
      } else {
        code_ += "        self._builder.Prepend\\";
        if (is_scalar) {
          code_ += MakeCamel(GenTypeBasic(field.value.type)) + "\\";
        } else {
          code_ += "UOffsetTRelative\\";
        }
        code_ += "Slot({{SLOT}}, {{FIELD_NAME}}, " + field.value.constant + ")";
      }

      code_ += "";

      if (field.value.type.base_type == BASE_TYPE_VECTOR) {
        auto vectortype = field.value.type.VectorType();

        code_ += "    @staticmethod";
        code_ += "    def start_{{FIELD_NAME}}(builder, number_of_elements):";
        code_ += "        return builder.StartVector(\\";
        code_ += NumToString(InlineSize(vectortype)) + ", \\";
        code_ += "number_of_elements, \\";
        code_ += NumToString(InlineAlignment(vectortype)) + ")";
        code_ += "";

        // End method is defined for symmetry.
        code_ += "    @staticmethod";
        code_ += "    def end_{{FIELD_NAME}}(builder, number_of_elements):";
        code_ += "        return builder.EndVector(number_of_elements)";
        code_ += "";
      }
    }

    // finish() function.
    code_ += "    def finish(self):";
    code_ += "        return self._builder.EndObject()";
    code_ += "";
    code_ += "";
  }

  void GenDirectBuilder(const flatbuffers::FieldDef &field) {
    code_.SetValue("FIELD_NAME", Name(field));

    code_ +=
        "        if {{FIELD_NAME}} and not isinstance({{FIELD_NAME}}, "
        "flatbuffers.number_types.UOffsetTFlags.py_type):";

    if (field.value.type.base_type == BASE_TYPE_STRING) {
      code_ += "            {{FIELD_NAME}} = fbb.CreateString({{FIELD_NAME}})";
    } else if (field.value.type.base_type == BASE_TYPE_VECTOR) {
      auto vectortype = field.value.type.VectorType();

      switch (vectortype.base_type) {
        case BASE_TYPE_CHAR:
        case BASE_TYPE_UCHAR:
          code_ +=
              "            {{FIELD_NAME}} = "
              "fbb.CreateByteVector({{FIELD_NAME}})";
          break;
        default:
          code_ += "            fbb.StartVector(\\";
          code_ += NumToString(InlineSize(vectortype)) + ", \\";
          code_ += "len({{FIELD_NAME}}), \\";
          code_ += NumToString(InlineAlignment(vectortype)) + ")";

          code_ += "            for elem in {{FIELD_NAME}}:";
          code_ += "                fbb.Prepend\\";
          if (IsScalar(vectortype.base_type)) {
            code_ += MakeCamel(GenTypeBasic(field.value.type)) + "\\";
          } else {
            code_ += "UOffsetTRelative\\";
          }
          code_ += "(elem)";
          code_ +=
              "            {{FIELD_NAME}} = fbb.EndVector(len({{FIELD_NAME}}))";
      }
    }
  }

  // Generate an accessor struct with constructor for a flatbuffers struct.
  void GenStruct(const StructDef &struct_def) {
    // Generate an accessor struct, with private variables of the form:
    // type name_;
    // Generates manual padding and alignment.
    // Variables are private because they contain little endian data on all
    // platforms.

    code_.SetValue("ALIGN", NumToString(struct_def.minalign));
    code_.SetValue("STRUCT_NAME", Name(struct_def));

    code_ += "class {{STRUCT_NAME}}(object):";
    GenComment(struct_def.doc_comment, "    ");

    GenFullyQualifiedNameGetter(struct_def, Name(struct_def));

    code_ += "    BYTE_SIZE = " + NumToString(struct_def.bytesize);
    code_ += "    MIN_ALIGN = " + NumToString(struct_def.minalign);
    code_ += "";

    // Generate constructor.
    code_ += "    def __init__(self, buf, offset):";
    code_ += "        self._buf = buf[offset:offset + self.BYTE_SIZE]";
    code_ += "";

    // Generate a creator that takes all fields as arguments.
    code_ += "    @classmethod";
    code_ += "    def create(cls\\";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const auto &field = **it;

      std::string default_value;
      if (IsScalar(field.value.type.base_type)) {
        default_value = GetDefaultValue(field);
      } else {
        default_value = "None";
      }

      code_ += ", " + Name(field) + "=" + default_value + "\\";
    }
    code_ += "):";

    code_ += "        value = cls(bytearray(cls.BYTE_SIZE), 0)";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const auto &field = **it;
      if (IsScalar(field.value.type.base_type)) {
        code_ += "        value." + Name(field) + " = " + Name(field);
      } else {
        code_ += "        if " + Name(field) + " is not None:";
        code_ += "            " + Name(field) +
                 ".copy_into(value._buf, value._offset + " +
                 NumToString(field.value.offset) + ")";
      }
    }
    code_ += "        return value";
    code_ += "";

    // Generate accessor properties.
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const auto &field = **it;

      GenField(field, true);
      if (IsScalar(field.value.type.base_type)) { GenScalarFieldSetter(field); }
    }

    // Generate copy_into function.
    code_ += "    def copy_into(self, buf, offset):";
    code_ += "        buf[offset:offset + len(self._buf)] = self._buf";
    code_ += "";
    code_ += "";
  }

  // Returns the function name that is able to read a value of the given type.
  std::string GenGetter(const Type &type) {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "self._tab.String(";
      case BASE_TYPE_UNION: return "self._tab.Union(";
      case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
      default:
        return "self._tab.Get(flatbuffers.number_types." +
               MakeCamel(GenTypeGet(type)) + "Flags, ";
    }
  }
};  // namespace python

}  // namespace python

bool GeneratePython(const Parser &parser, const std::string &path,
                    const std::string &file_name) {
  python::PythonGenerator generator(parser, path, file_name);
  return generator.generate();
}

std::string PythonMakeRule(const Parser &parser, const std::string &path,
                           const std::string &file_name) {
  const auto filebase =
      flatbuffers::StripPath(flatbuffers::StripExtension(file_name));
  const auto included_files = parser.GetIncludedFilesRecursive(file_name);
  std::string make_rule = GeneratedFileName(path, filebase) + ": ";
  for (auto it = included_files.begin(); it != included_files.end(); ++it) {
    make_rule += " " + *it;
  }
  return make_rule;
}

}  // namespace flatbuffers
