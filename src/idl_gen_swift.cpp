/*
 * Copyright 2020 Google Inc. All rights reserved.
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

#include <cctype>
#include <unordered_set>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

namespace swift {

inline std::string GenIndirect(const std::string &reading) {
  return "{{ACCESS}}.indirect(" + reading + ")";
}

inline std::string GenArrayMainBody(const std::string &optional) {
  return "{{ACCESS_TYPE}} func {{VALUENAME}}(at index: Int32) -> "
         "{{VALUETYPE}}" +
         optional + " { ";
}

class SwiftGenerator : public BaseGenerator {
 private:
  CodeWriter code_;
  std::unordered_set<std::string> keywords_;
  int namespace_depth;

 public:
  SwiftGenerator(const Parser &parser, const std::string &path,
                 const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", "_", "swift") {
    namespace_depth = 0;
    code_.SetPadding("  ");
    static const char *const keywords[] = {
      "associatedtype",
      "class",
      "deinit",
      "enum",
      "extension",
      "fileprivate",
      "func",
      "import",
      "init",
      "inout",
      "internal",
      "let",
      "open",
      "operator",
      "private",
      "protocol",
      "public",
      "rethrows",
      "static",
      "struct",
      "subscript",
      "typealias",
      "var",
      "break",
      "case",
      "continue",
      "default",
      "defer",
      "do",
      "else",
      "fallthrough",
      "for",
      "guard",
      "if",
      "in",
      "repeat",
      "return",
      "switch",
      "where",
      "while",
      "Any",
      "catch",
      "false",
      "is",
      "nil",
      "super",
      "self",
      "Self",
      "throw",
      "throws",
      "true",
      "try",
      "associativity",
      "convenience",
      "dynamic",
      "didSet",
      "final",
      "get",
      "infix",
      "indirect",
      "lazy",
      "left",
      "mutating",
      "none",
      "nonmutating",
      "optional",
      "override",
      "postfix",
      "precedence",
      "prefix",
      "Protocol",
      "required",
      "right",
      "set",
      "Type",
      "unowned",
      "weak",
      "willSet",
      "Void",
      nullptr,
    };
    for (auto kw = keywords; *kw; kw++) keywords_.insert(*kw);
  }

  bool generate() {
    code_.Clear();
    code_.SetValue("ACCESS", "_accessor");
    code_.SetValue("TABLEOFFSET", "VTOFFSET");
    code_ += "// " + std::string(FlatBuffersGeneratedWarning());
    code_ += "// swiftlint:disable all";
    code_ += "// swiftformat:disable all\n";
    code_ += "import FlatBuffers\n";
    // Generate code for all the enum declarations.

    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      const auto &enum_def = **it;
      if (!enum_def.generated) { GenEnum(enum_def); }
    }

    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      const auto &struct_def = **it;
      if (struct_def.fixed && !struct_def.generated) {
        GenStructReader(struct_def);
        GenMutableStructReader(struct_def);
      }
    }

    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      const auto &struct_def = **it;
      if (!struct_def.fixed && !struct_def.generated) {
        GenTable(struct_def);
        if (parser_.opts.generate_object_based_api) {
          GenObjectAPI(struct_def);
        }
      }
    }

    const auto filename = GeneratedFileName(path_, file_name_, parser_.opts);
    const auto final_code = code_.ToString();
    return SaveFile(filename.c_str(), final_code, false);
  }

  void mark(const std::string &str) {
    code_.SetValue("MARKVALUE", str);
    code_ += "\n// MARK: - {{MARKVALUE}}\n";
  }

  // MARK: - Generating structs

  // Generates the reader for swift
  void GenStructReader(const StructDef &struct_def) {
    auto is_private_access = struct_def.attributes.Lookup("private");
    code_.SetValue("ACCESS_TYPE", is_private_access ? "internal" : "public");
    GenComment(struct_def.doc_comment);
    code_.SetValue("STRUCTNAME", NameWrappedInNameSpace(struct_def));
    code_ += "{{ACCESS_TYPE}} struct {{STRUCTNAME}}: NativeStruct\\";
    if (parser_.opts.generate_object_based_api) code_ += ", NativeObject\\";
    code_ += " {";
    code_ += "";
    Indent();
    code_ += ValidateFunc();
    code_ += "";
    int padding_id = 0;
    std::string constructor = "";
    std::vector<std::string> base_constructor;
    std::vector<std::string> main_constructor;

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      if (!constructor.empty()) constructor += ", ";

      auto name = Name(field);
      auto type = GenType(field.value.type);
      code_.SetValue("VALUENAME", name);
      if (IsEnum(field.value.type)) {
        code_.SetValue("BASEVALUE", GenTypeBasic(field.value.type, false));
      }
      code_.SetValue("VALUETYPE", type);
      GenComment(field.doc_comment);
      std::string valueType =
          IsEnum(field.value.type) ? "{{BASEVALUE}}" : "{{VALUETYPE}}";
      code_ += "private var _{{VALUENAME}}: " + valueType;
      auto accessing_value = IsEnum(field.value.type) ? ".value" : "";
      auto base_value =
          IsStruct(field.value.type) ? (type + "()") : field.value.constant;

      main_constructor.push_back("_" + name + " = " + name + accessing_value);
      base_constructor.push_back("_" + name + " = " + base_value);

      if (field.padding) { GenPadding(field, &padding_id); }
      constructor += name + ": " + type;
    }
    code_ += "";
    BuildObjectConstructor(main_constructor, constructor);
    BuildObjectConstructor(base_constructor, "");

    if (parser_.opts.generate_object_based_api)
      GenerateObjectAPIStructConstructor(struct_def);

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      auto name = Name(field);
      auto type = GenType(field.value.type);
      code_.SetValue("VALUENAME", name);
      code_.SetValue("VALUETYPE", type);
      GenComment(field.doc_comment);
      if (!IsEnum(field.value.type)) {
        code_ += GenReaderMainBody() + "_{{VALUENAME}} }";
      } else if (IsEnum(field.value.type)) {
        code_ +=
            GenReaderMainBody() + "{{VALUETYPE}}(rawValue: _{{VALUENAME}})! }";
      }
    }
    Outdent();
    code_ += "}\n";
  }

  void GenMutableStructReader(const StructDef &struct_def) {
    GenObjectHeader(struct_def);

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      auto offset = NumToString(field.value.offset);
      auto name = Name(field);
      auto type = GenType(field.value.type);
      code_.SetValue("VALUENAME", name);
      if (IsEnum(field.value.type)) {
        code_.SetValue("BASEVALUE", GenTypeBasic(field.value.type, false));
      }
      code_.SetValue("VALUETYPE", type);
      code_.SetValue("OFFSET", offset);
      if (IsScalar(field.value.type.base_type) && !IsEnum(field.value.type)) {
        code_ +=
            GenReaderMainBody() + "return " + GenReader("VALUETYPE") + " }";
      } else if (IsEnum(field.value.type)) {
        code_.SetValue("BASEVALUE", GenTypeBasic(field.value.type, false));
        code_ += GenReaderMainBody() + "return " +
                 GenEnumConstructor("{{OFFSET}}") + "?? " +
                 GenEnumDefaultValue(field) + " }";
      } else if (IsStruct(field.value.type)) {
        code_.SetValue("VALUETYPE", GenType(field.value.type) + Mutable());
        code_ += GenReaderMainBody() + "return " +
                 GenConstructor("{{ACCESS}}.postion + {{OFFSET}}");
      }
      if (parser_.opts.mutable_buffer && !IsStruct(field.value.type))
        code_ += GenMutate("{{OFFSET}}", "", IsEnum(field.value.type));
    }

    if (parser_.opts.generate_object_based_api) {
      GenerateObjectAPIExtensionHeader(NameWrappedInNameSpace(struct_def));
      code_ += "return builder.create(struct: obj)";
      Outdent();
      code_ += "}";
    }
    Outdent();
    code_ += "}\n";
  }

  // Generates the create function for swift
  void GenStructWriter(const StructDef &struct_def) {
    auto is_private_access = struct_def.attributes.Lookup("private");
    code_.SetValue("ACCESS_TYPE", is_private_access ? "internal" : "public");
    code_.SetValue("STRUCTNAME", NameWrappedInNameSpace(struct_def));
    code_.SetValue("SHORT_STRUCTNAME", Name(struct_def));
    code_ += "extension {{STRUCTNAME}} {";
    Indent();
    code_ += "@discardableResult";
    code_ +=
        "{{ACCESS_TYPE}} static func create{{SHORT_STRUCTNAME}}(builder: inout "
        "FlatBufferBuilder, \\";
    std::string func_header = "";
    GenerateStructArgs(struct_def, &func_header, "", "");
    code_ += func_header.substr(0, func_header.size() - 2) + "\\";
    code_ += ") -> Offset {";
    Indent();
    code_ +=
        "builder.createStructOf(size: {{STRUCTNAME}}.size, alignment: "
        "{{STRUCTNAME}}.alignment)";
    code_ += "return builder.endStruct()";
    Outdent();
    code_ += "}\n";
    Outdent();
    code_ += "}\n";
  }

  void GenerateStructArgs(const StructDef &struct_def, std::string *code_ptr,
                          const std::string &nameprefix,
                          const std::string &object_name,
                          const std::string &obj_api_named = "",
                          bool is_obj_api = false) {
    auto &code = *code_ptr;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      const auto &field_type = field.value.type;
      if (IsStruct(field.value.type)) {
        GenerateStructArgs(
            *field_type.struct_def, code_ptr, (nameprefix + field.name),
            (object_name + "." + field.name), obj_api_named, is_obj_api);
      } else {
        auto name = Name(field);
        auto type = GenType(field.value.type);
        if (!is_obj_api) {
          code += nameprefix + name + ": " + type;
          if (!IsEnum(field.value.type)) {
            code += " = ";
            auto is_bool = IsBool(field.value.type.base_type);
            auto constant =
                is_bool ? ("0" == field.value.constant ? "false" : "true")
                        : field.value.constant;
            code += constant;
          }
          code += ", ";
          continue;
        }
        code +=
            nameprefix + name + ": " + obj_api_named + object_name + "." + name;
        code += ", ";
      }
    }
  }

  // MARK: - Table Generator

  // Generates the reader for swift
  void GenTable(const StructDef &struct_def) {
    auto is_private_access = struct_def.attributes.Lookup("private");
    code_.SetValue("ACCESS_TYPE", is_private_access ? "internal" : "public");

    GenObjectHeader(struct_def);
    GenTableAccessors(struct_def);
    GenTableReader(struct_def);
    GenTableWriter(struct_def);
    if (parser_.opts.generate_object_based_api)
      GenerateObjectAPITableExtension(struct_def);
    Outdent();
    code_ += "}\n";
  }

  // Generates the reader for swift
  void GenTableAccessors(const StructDef &struct_def) {
    // Generate field id constants.
    if (struct_def.fields.vec.size() > 0) {
      code_ += "private enum {{TABLEOFFSET}}: VOffset {";
      Indent();
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        const auto &field = **it;
        if (field.deprecated) { continue; }
        code_.SetValue("OFFSET_NAME", Name(field));
        code_.SetValue("OFFSET_VALUE", NumToString(field.value.offset));
        code_ += "case {{OFFSET_NAME}} = {{OFFSET_VALUE}}";
      }
      code_ += "var v: Int32 { Int32(self.rawValue) }";
      code_ += "var p: VOffset { self.rawValue }";
      Outdent();
      code_ += "}";
      code_ += "";
    }
  }

  void GenObjectHeader(const StructDef &struct_def) {
    GenComment(struct_def.doc_comment);

    code_.SetValue("SHORT_STRUCTNAME", Name(struct_def));
    code_.SetValue("STRUCTNAME", NameWrappedInNameSpace(struct_def));
    code_.SetValue("OBJECTTYPE", struct_def.fixed ? "Struct" : "Table");
    code_.SetValue("MUTABLE", struct_def.fixed ? Mutable() : "");
    code_ +=
        "{{ACCESS_TYPE}} struct {{STRUCTNAME}}{{MUTABLE}}: FlatBufferObject\\";
    if (!struct_def.fixed && parser_.opts.generate_object_based_api)
      code_ += ", ObjectAPIPacker\\";
    code_ += " {\n";
    Indent();
    code_ += ValidateFunc();
    code_ +=
        "{{ACCESS_TYPE}} var __buffer: ByteBuffer! { return {{ACCESS}}.bb }";
    code_ += "private var {{ACCESS}}: {{OBJECTTYPE}}\n";
    if (!struct_def.fixed) {
      if (parser_.file_identifier_.length()) {
        code_.SetValue("FILENAME", parser_.file_identifier_);
        code_ +=
            "{{ACCESS_TYPE}} static func finish(_ fbb: inout "
            "FlatBufferBuilder, end: "
            "Offset, prefix: Bool = false) { fbb.finish(offset: end, "
            "fileId: "
            "\"{{FILENAME}}\", addPrefix: prefix) }";
      }
      code_ +=
          "{{ACCESS_TYPE}} static func getRootAs{{SHORT_STRUCTNAME}}(bb: "
          "ByteBuffer) -> "
          "{{STRUCTNAME}} { return {{STRUCTNAME}}(Table(bb: bb, position: "
          "Int32(bb.read(def: UOffset.self, position: bb.reader)) + "
          "Int32(bb.reader))) }\n";
      code_ += "private init(_ t: Table) { {{ACCESS}} = t }";
    }
    code_ +=
        "{{ACCESS_TYPE}} init(_ bb: ByteBuffer, o: Int32) { {{ACCESS}} = "
        "{{OBJECTTYPE}}(bb: "
        "bb, position: o) }";
    code_ += "";
  }

  void GenTableWriter(const StructDef &struct_def) {
    flatbuffers::FieldDef *key_field = nullptr;
    std::vector<std::string> require_fields;
    std::vector<std::string> create_func_body;
    std::vector<std::string> create_func_header;
    auto should_generate_create = struct_def.fields.vec.size() != 0;

    code_.SetValue("NUMBEROFFIELDS", NumToString(struct_def.fields.vec.size()));
    code_ +=
        "{{ACCESS_TYPE}} static func start{{SHORT_STRUCTNAME}}(_ fbb: inout "
        "FlatBufferBuilder) -> "
        "UOffset { fbb.startTable(with: {{NUMBEROFFIELDS}}) }";

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      if (field.key) key_field = &field;
      if (field.IsRequired())
        require_fields.push_back(NumToString(field.value.offset));

      GenTableWriterFields(field, &create_func_body, &create_func_header);
    }
    code_ +=
        "{{ACCESS_TYPE}} static func end{{SHORT_STRUCTNAME}}(_ fbb: inout "
        "FlatBufferBuilder, "
        "start: "
        "UOffset) -> Offset { let end = Offset(offset: "
        "fbb.endTable(at: start))\\";
    if (require_fields.capacity() != 0) {
      std::string fields = "";
      for (auto it = require_fields.begin(); it != require_fields.end(); ++it)
        fields += *it + ", ";
      code_.SetValue("FIELDS", fields.substr(0, fields.size() - 2));
      code_ += "; fbb.require(table: end, fields: [{{FIELDS}}])\\";
    }
    code_ += "; return end }";

    if (should_generate_create) {
      code_ += "{{ACCESS_TYPE}} static func create{{SHORT_STRUCTNAME}}(";
      Indent();
      code_ += "_ fbb: inout FlatBufferBuilder,";
      for (auto it = create_func_header.begin(); it < create_func_header.end();
           ++it) {
        code_ += *it + "\\";
        if (it < create_func_header.end() - 1) code_ += ",";
      }
      code_ += "";
      Outdent();
      code_ += ") -> Offset {";
      Indent();
      code_ += "let __start = {{STRUCTNAME}}.start{{SHORT_STRUCTNAME}}(&fbb)";
      for (auto it = create_func_body.begin(); it < create_func_body.end();
           ++it) {
        code_ += *it;
      }
      code_ +=
          "return {{STRUCTNAME}}.end{{SHORT_STRUCTNAME}}(&fbb, start: __start)";
      Outdent();
      code_ += "}";
    }

    std::string spacing = "";

    if (key_field != nullptr && !struct_def.fixed && struct_def.has_key) {
      code_.SetValue("VALUENAME", NameWrappedInNameSpace(struct_def));
      code_.SetValue("SHORT_VALUENAME", Name(struct_def));
      code_.SetValue("VOFFSET", NumToString(key_field->value.offset));

      code_ +=
          "{{ACCESS_TYPE}} static func "
          "sortVectorOf{{SHORT_VALUENAME}}(offsets:[Offset], "
          "_ fbb: inout FlatBufferBuilder) -> Offset {";
      Indent();
      code_ += spacing + "var off = offsets";
      code_ +=
          spacing +
          "off.sort { Table.compare(Table.offset(Int32($1.o), vOffset: "
          "{{VOFFSET}}, fbb: fbb.buffer), Table.offset(Int32($0.o), vOffset: "
          "{{VOFFSET}}, fbb: fbb.buffer), fbb: fbb.buffer) < 0 } ";
      code_ += spacing + "return fbb.createVector(ofOffsets: off)";
      Outdent();
      code_ += "}";
      GenLookup(*key_field);
    }
  }

  void GenTableWriterFields(const FieldDef &field,
                            std::vector<std::string> *create_body,
                            std::vector<std::string> *create_header) {
    std::string builder_string = ", _ fbb: inout FlatBufferBuilder) { ";
    auto &create_func_body = *create_body;
    auto &create_func_header = *create_header;
    auto name = Name(field);
    auto type = GenType(field.value.type);
    auto opt_scalar =
        field.IsOptional() && IsScalar(field.value.type.base_type);
    auto nullable_type = opt_scalar ? type + "?" : type;
    code_.SetValue("VALUENAME", name);
    code_.SetValue("VALUETYPE", nullable_type);
    code_.SetValue("OFFSET", name);
    code_.SetValue("CONSTANT", field.value.constant);
    std::string check_if_vector =
        (IsVector(field.value.type) || IsArray(field.value.type)) ? "VectorOf("
                                                                  : "(";
    auto body = "add" + check_if_vector + name + ": ";
    code_ += "{{ACCESS_TYPE}} static func " + body + "\\";

    create_func_body.push_back("{{STRUCTNAME}}." + body + name + ", &fbb)");

    if (IsScalar(field.value.type.base_type) &&
        !IsBool(field.value.type.base_type)) {
      std::string is_enum = IsEnum(field.value.type) ? ".rawValue" : "";
      std::string optional_enum =
          IsEnum(field.value.type) ? ("?" + is_enum) : "";
      code_ +=
          "{{VALUETYPE}}" + builder_string + "fbb.add(element: {{VALUENAME}}\\";

      code_ += field.IsOptional() ? (optional_enum + "\\")
                                  : (is_enum + ", def: {{CONSTANT}}\\");

      code_ += ", at: {{TABLEOFFSET}}.{{OFFSET}}.p) }";

      auto default_value =
          IsEnum(field.value.type)
              ? (field.IsOptional() ? "nil" : GenEnumDefaultValue(field))
              : field.value.constant;
      create_func_header.push_back(
          "" + name + ": " + nullable_type + " = " +
          (field.IsOptional() ? "nil" : default_value));
      return;
    }

    if (IsBool(field.value.type.base_type)) {
      std::string default_value =
          "0" == field.value.constant ? "false" : "true";

      code_.SetValue("CONSTANT", default_value);
      code_.SetValue("VALUETYPE", field.IsOptional() ? "Bool?" : "Bool");
      code_ += "{{VALUETYPE}}" + builder_string +
               "fbb.add(element: {{VALUENAME}},\\";
      code_ += field.IsOptional() ? "\\" : " def: {{CONSTANT}},";
      code_ += " at: {{TABLEOFFSET}}.{{OFFSET}}.p) }";
      create_func_header.push_back(
          name + ": " + nullable_type + " = " +
          (field.IsOptional() ? "nil" : default_value));
      return;
    }

    if (IsStruct(field.value.type)) {
      auto create_struct =
          "guard let {{VALUENAME}} = {{VALUENAME}} else { return };"
          " fbb.create(struct: {{VALUENAME}}, position: "
          "{{TABLEOFFSET}}.{{OFFSET}}.p) }";
      code_ += type + "?" + builder_string + create_struct;
      /// Optional hard coded since structs are always optional
      create_func_header.push_back(name + ": " + type + "? = nil");
      return;
    }

    auto camel_case_name =
        MakeCamel(name, false) +
        (IsVector(field.value.type) || IsArray(field.value.type)
             ? "VectorOffset"
             : "Offset");
    create_func_header.push_back(camel_case_name + " " + name + ": " +
                                 "Offset = Offset()");
    auto reader_type =
        IsStruct(field.value.type) && field.value.type.struct_def->fixed
            ? "structOffset: {{TABLEOFFSET}}.{{OFFSET}}.p) }"
            : "offset: {{VALUENAME}}, at: {{TABLEOFFSET}}.{{OFFSET}}.p) }";
    code_ += "Offset" + builder_string + "fbb.add(" + reader_type;

    auto vectortype = field.value.type.VectorType();

    if ((vectortype.base_type == BASE_TYPE_STRUCT &&
         field.value.type.struct_def->fixed) &&
        (IsVector(field.value.type) || IsArray(field.value.type))) {
      auto field_name = NameWrappedInNameSpace(*vectortype.struct_def);
      code_ += "public static func startVectorOf" + MakeCamel(name, true) +
               "(_ size: Int, in builder: inout "
               "FlatBufferBuilder) {";
      Indent();
      code_ += "builder.startVector(size * MemoryLayout<" + field_name +
               ">.size, elementSize: MemoryLayout<" + field_name +
               ">.alignment)";
      Outdent();
      code_ += "}";
    }
  }

  void GenTableReader(const StructDef &struct_def) {
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      GenTableReaderFields(field);
    }
  }

  void GenTableReaderFields(const FieldDef &field) {
    auto offset = NumToString(field.value.offset);
    auto name = Name(field);
    auto type = GenType(field.value.type);
    code_.SetValue("VALUENAME", name);
    code_.SetValue("VALUETYPE", type);
    code_.SetValue("OFFSET", name);
    code_.SetValue("CONSTANT", field.value.constant);
    std::string def_Val = field.IsDefault() ? "{{CONSTANT}}" : "nil";
    std::string optional = field.IsOptional() ? "?" : "";
    auto const_string = "return o == 0 ? " + def_Val + " : ";
    GenComment(field.doc_comment);
    if (IsScalar(field.value.type.base_type) && !IsEnum(field.value.type) &&
        !IsBool(field.value.type.base_type)) {
      code_ += GenReaderMainBody(optional) + GenOffset() + const_string +
               GenReader("VALUETYPE", "o") + " }";
      if (parser_.opts.mutable_buffer) code_ += GenMutate("o", GenOffset());
      return;
    }

    if (IsBool(field.value.type.base_type)) {
      std::string default_value =
          "0" == field.value.constant ? "false" : "true";
      code_.SetValue("CONSTANT", default_value);
      code_.SetValue("VALUETYPE", "Bool");
      code_ += GenReaderMainBody(optional) + "\\";
      code_.SetValue("VALUETYPE", "Byte");
      code_ += GenOffset() + "return o == 0 ? {{CONSTANT}} : 0 != " +
               GenReader("VALUETYPE", "o") + " }";
      if (parser_.opts.mutable_buffer) code_ += GenMutate("o", GenOffset());
      return;
    }

    if (IsEnum(field.value.type)) {
      auto default_value =
          field.IsOptional() ? "nil" : GenEnumDefaultValue(field);
      code_.SetValue("BASEVALUE", GenTypeBasic(field.value.type, false));
      code_ += GenReaderMainBody(optional) + "\\";
      code_ += GenOffset() + "return o == 0 ? " + default_value + " : " +
               GenEnumConstructor("o") + "?? " + default_value + " }";
      if (parser_.opts.mutable_buffer && !IsUnion(field.value.type))
        code_ += GenMutate("o", GenOffset(), true);
      return;
    }

    std::string is_required = field.IsRequired() ? "!" : "?";
    auto required_reader = field.IsRequired() ? "return " : const_string;

    if (IsStruct(field.value.type) && field.value.type.struct_def->fixed) {
      code_.SetValue("VALUETYPE", GenType(field.value.type));
      code_.SetValue("CONSTANT", "nil");
      code_ += GenReaderMainBody(is_required) + GenOffset() + required_reader +
               "{{ACCESS}}.readBuffer(of: {{VALUETYPE}}.self, at: o) }";
      code_.SetValue("VALUENAME", "mutable" + MakeCamel(name));
      code_.SetValue("VALUETYPE", GenType(field.value.type) + Mutable());
      code_.SetValue("CONSTANT", "nil");
      code_ += GenReaderMainBody(is_required) + GenOffset() + required_reader +
               GenConstructor("o + {{ACCESS}}.postion");
      return;
    }
    switch (field.value.type.base_type) {
      case BASE_TYPE_STRUCT:
        code_.SetValue("VALUETYPE", GenType(field.value.type));
        code_.SetValue("CONSTANT", "nil");
        code_ += GenReaderMainBody(is_required) + GenOffset() +
                 required_reader +
                 GenConstructor(GenIndirect("o + {{ACCESS}}.postion"));
        break;

      case BASE_TYPE_STRING: {
        auto default_string = "\"" + field.value.constant + "\"";
        code_.SetValue("VALUETYPE", GenType(field.value.type));
        code_.SetValue("CONSTANT", field.IsDefault() ? default_string : "nil");
        code_ += GenReaderMainBody(is_required) + GenOffset() +
                 required_reader + "{{ACCESS}}.string(at: o) }";
        code_ += "{{ACCESS_TYPE}} var {{VALUENAME}}SegmentArray: [UInt8]" +
                 is_required +
                 " { return "
                 "{{ACCESS}}.getVector(at: {{TABLEOFFSET}}.{{OFFSET}}.v) }";
        break;
      }
      case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();  // fall thru
      case BASE_TYPE_VECTOR: GenTableReaderVectorFields(field); break;
      case BASE_TYPE_UNION:
        code_.SetValue("CONSTANT", "nil");
        code_ +=
            "{{ACCESS_TYPE}} func {{VALUENAME}}<T: "
            "FlatbuffersInitializable>(type: "
            "T.Type) -> T" +
            is_required + " { " + GenOffset() + required_reader +
            "{{ACCESS}}.union(o) }";
        break;
      default: FLATBUFFERS_ASSERT(0);
    }
  }

  void GenTableReaderVectorFields(const FieldDef &field) {
    std::string const_string = "return o == 0 ? {{CONSTANT}} : ";
    auto vectortype = field.value.type.VectorType();
    code_.SetValue("SIZE", NumToString(InlineSize(vectortype)));
    code_ += "{{ACCESS_TYPE}} var {{VALUENAME}}Count: Int32 { " + GenOffset() +
             "return o == 0 ? 0 : {{ACCESS}}.vector(count: o) }";
    code_.SetValue("CONSTANT",
                   IsScalar(vectortype.base_type) == true ? "0" : "nil");
    auto nullable = IsScalar(vectortype.base_type) == true ? "" : "?";
    nullable = IsEnum(vectortype) == true ? "?" : nullable;

    if (vectortype.base_type != BASE_TYPE_UNION) {
      code_ += GenArrayMainBody(nullable) + GenOffset() + "\\";
    } else {
      code_ +=
          "{{ACCESS_TYPE}} func {{VALUENAME}}<T: FlatbuffersInitializable>(at "
          "index: "
          "Int32, type: T.Type) -> T? { " +
          GenOffset() + "\\";
    }

    if (IsBool(vectortype.base_type)) {
      code_.SetValue("CONSTANT", field.value.offset == 0 ? "false" : "true");
      code_.SetValue("VALUETYPE", "Bool");
    }

    if (!IsEnum(vectortype)) code_ += const_string + "\\";

    if (IsScalar(vectortype.base_type) && !IsEnum(vectortype) &&
        !IsBool(field.value.type.base_type)) {
      code_ +=
          "{{ACCESS}}.directRead(of: {{VALUETYPE}}.self, offset: "
          "{{ACCESS}}.vector(at: o) + index * {{SIZE}}) }";
      code_ +=
          "{{ACCESS_TYPE}} var {{VALUENAME}}: [{{VALUETYPE}}] { return "
          "{{ACCESS}}.getVector(at: {{TABLEOFFSET}}.{{OFFSET}}.v) ?? [] }";
      if (parser_.opts.mutable_buffer) code_ += GenMutateArray();
      return;
    }

    if (vectortype.base_type == BASE_TYPE_STRUCT &&
        field.value.type.struct_def->fixed) {
      code_ +=
          "{{ACCESS}}.directRead(of: {{VALUETYPE}}.self, offset: "
          "{{ACCESS}}.vector(at: o) + index * {{SIZE}}) }";
      code_.SetValue("VALUENAME", "mutable" + MakeCamel(Name(field)));
      code_.SetValue("VALUETYPE", GenType(field.value.type) + Mutable());
      code_ += GenArrayMainBody(nullable) + GenOffset() + const_string +
               GenConstructor("{{ACCESS}}.vector(at: o) + index * {{SIZE}}");

      return;
    }

    if (IsString(vectortype)) {
      code_ +=
          "{{ACCESS}}.directString(at: {{ACCESS}}.vector(at: o) + "
          "index * {{SIZE}}) }";
      return;
    }

    if (IsEnum(vectortype)) {
      code_.SetValue("BASEVALUE", GenTypeBasic(vectortype, false));
      code_ += "return o == 0 ? {{VALUETYPE}}" + GenEnumDefaultValue(field) +
               " : {{VALUETYPE}}(rawValue: {{ACCESS}}.directRead(of: "
               "{{BASEVALUE}}.self, offset: {{ACCESS}}.vector(at: o) + "
               "index * {{SIZE}})) }";
      return;
    }
    if (vectortype.base_type == BASE_TYPE_UNION) {
      code_ +=
          "{{ACCESS}}.directUnion({{ACCESS}}.vector(at: o) + "
          "index * {{SIZE}}) }";
      return;
    }

    if (vectortype.base_type == BASE_TYPE_STRUCT &&
        !field.value.type.struct_def->fixed) {
      code_ += GenConstructor(
          "{{ACCESS}}.indirect({{ACCESS}}.vector(at: o) + index * "
          "{{SIZE}})");
      auto &sd = *field.value.type.struct_def;
      auto &fields = sd.fields.vec;
      for (auto kit = fields.begin(); kit != fields.end(); ++kit) {
        auto &key_field = **kit;
        if (key_field.key) {
          GenByKeyFunctions(key_field);
          break;
        }
      }
    }
  }

  void GenByKeyFunctions(const FieldDef &key_field) {
    code_.SetValue("TYPE", GenType(key_field.value.type));
    code_ +=
        "{{ACCESS_TYPE}} func {{VALUENAME}}By(key: {{TYPE}}) -> {{VALUETYPE}}? "
        "{ \\";
    code_ += GenOffset() +
             "return o == 0 ? nil : {{VALUETYPE}}.lookupByKey(vector: "
             "{{ACCESS}}.vector(at: o), key: key, fbb: {{ACCESS}}.bb) }";
  }

  void GenEnum(const EnumDef &enum_def) {
    if (enum_def.generated) return;
    auto is_private_access = enum_def.attributes.Lookup("private");
    code_.SetValue("ACCESS_TYPE", is_private_access ? "internal" : "public");
    code_.SetValue("ENUM_NAME", NameWrappedInNameSpace(enum_def));
    code_.SetValue("BASE_TYPE", GenTypeBasic(enum_def.underlying_type, false));
    GenComment(enum_def.doc_comment);
    code_ += "{{ACCESS_TYPE}} enum {{ENUM_NAME}}: {{BASE_TYPE}}, Enum {";
    Indent();
    code_ += "{{ACCESS_TYPE}} typealias T = {{BASE_TYPE}}";
    code_ +=
        "{{ACCESS_TYPE}} static var byteSize: Int { return "
        "MemoryLayout<{{BASE_TYPE}}>.size "
        "}";
    code_ +=
        "{{ACCESS_TYPE}} var value: {{BASE_TYPE}} { return self.rawValue }";
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      const auto &ev = **it;
      auto name = Name(ev);
      code_.SetValue("KEY", name);
      code_.SetValue("VALUE", enum_def.ToString(ev));
      GenComment(ev.doc_comment);
      code_ += "case {{KEY}} = {{VALUE}}";
    }
    code_ += "\n";
    AddMinOrMaxEnumValue(Name(*enum_def.MaxValue()), "max");
    AddMinOrMaxEnumValue(Name(*enum_def.MinValue()), "min");
    Outdent();
    code_ += "}\n";
    if (parser_.opts.generate_object_based_api && enum_def.is_union) {
      code_ += "{{ACCESS_TYPE}} struct {{ENUM_NAME}}Union {";
      Indent();
      code_ += "{{ACCESS_TYPE}} var type: {{ENUM_NAME}}";
      code_ += "{{ACCESS_TYPE}} var value: NativeObject?";
      code_ +=
          "{{ACCESS_TYPE}} init(_ v: NativeObject?, type: {{ENUM_NAME}}) {";
      Indent();
      code_ += "self.type = type";
      code_ += "self.value = v";
      Outdent();
      code_ += "}";
      code_ +=
          "{{ACCESS_TYPE}} func pack(builder: inout FlatBufferBuilder) -> "
          "Offset {";
      Indent();
      BuildUnionEnumSwitchCaseWritter(enum_def);
      Outdent();
      code_ += "}";
      Outdent();
      code_ += "}";
    }
  }

  // MARK: - Object API

  void GenerateObjectAPIExtensionHeader(std::string name) {
    code_ += "\n";
    code_ += "{{ACCESS_TYPE}} mutating func unpack() -> " + name + " {";
    Indent();
    code_ += "return " + name + "(&self)";
    Outdent();
    code_ += "}";
    code_ +=
        "{{ACCESS_TYPE}} static func pack(_ builder: inout FlatBufferBuilder, "
        "obj: "
        "inout " +
        name + "?) -> Offset {";
    Indent();
    code_ += "guard var obj = obj else { return Offset() }";
    code_ += "return pack(&builder, obj: &obj)";
    Outdent();
    code_ += "}";
    code_ += "";
    code_ +=
        "{{ACCESS_TYPE}} static func pack(_ builder: inout FlatBufferBuilder, "
        "obj: "
        "inout " +
        name + ") -> Offset {";
    Indent();
  }

  void GenerateObjectAPIStructConstructor(const StructDef &struct_def) {
    code_ +=
        "{{ACCESS_TYPE}} init(_ _t: inout {{STRUCTNAME}}" + Mutable() + ") {";
    Indent();
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      auto name = Name(field);
      auto type = GenType(field.value.type);
      code_.SetValue("VALUENAME", name);
      if (IsStruct(field.value.type)) {
        code_ += "var _v{{VALUENAME}} = _t.{{VALUENAME}}";
        code_ += "_{{VALUENAME}} = _v{{VALUENAME}}.unpack()";
        continue;
      }
      std::string is_enum = IsEnum(field.value.type) ? ".value" : "";
      code_ += "_{{VALUENAME}} = _t.{{VALUENAME}}" + is_enum;
    }
    Outdent();
    code_ += "}\n";
  }

  void GenObjectAPI(const StructDef &struct_def) {
    code_ += "{{ACCESS_TYPE}} class " + ObjectAPIName("{{STRUCTNAME}}") +
             ": NativeObject {\n";
    std::vector<std::string> buffer_constructor;
    std::vector<std::string> base_constructor;
    Indent();
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      BuildObjectAPIConstructorBody(field, struct_def.fixed, buffer_constructor,
                                    base_constructor);
    }
    code_ += "";
    BuildObjectConstructor(buffer_constructor,
                           "_ _t: inout " + NameWrappedInNameSpace(struct_def));
    BuildObjectConstructor(base_constructor);
    if (!struct_def.fixed)
      code_ +=
          "{{ACCESS_TYPE}} func serialize() -> ByteBuffer { return "
          "serialize(type: "
          "{{STRUCTNAME}}.self) }\n";
    Outdent();
    code_ += "}";
  }

  void GenerateObjectAPITableExtension(const StructDef &struct_def) {
    GenerateObjectAPIExtensionHeader(ObjectAPIName("{{STRUCTNAME}}"));
    std::vector<std::string> unpack_body;
    std::string builder = ", &builder)";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      auto name = Name(field);
      auto type = GenType(field.value.type);
      std::string check_if_vector =
          (IsVector(field.value.type) || IsArray(field.value.type))
              ? "VectorOf("
              : "(";
      std::string body = "add" + check_if_vector + name + ": ";
      switch (field.value.type.base_type) {
        case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();
        case BASE_TYPE_VECTOR: {
          GenerateVectorObjectAPITableExtension(field, name, type);
          unpack_body.push_back("{{STRUCTNAME}}." + body + "__" + name +
                                builder);
          break;
        }
        case BASE_TYPE_UNION: {
          code_ += "let __" + name + " = obj." + name +
                   "?.pack(builder: &builder) ?? Offset()";
          unpack_body.push_back("if let o = obj." + name + "?.type {");
          unpack_body.push_back("  {{STRUCTNAME}}.add(" + name + "Type: o" +
                                builder);
          unpack_body.push_back("  {{STRUCTNAME}}." + body + "__" + name +
                                builder);
          unpack_body.push_back("}\n");
          break;
        }
        case BASE_TYPE_STRUCT: {
          if (field.value.type.struct_def &&
              field.value.type.struct_def->fixed) {
            // This is a Struct (IsStruct), not a table. We create
            // a native swift object in this case.
            std::string code;
            GenerateStructArgs(*field.value.type.struct_def, &code, "", "",
                               "$0", true);
            code = code.substr(0, code.size() - 2);
            unpack_body.push_back("{{STRUCTNAME}}." + body + "obj." + name +
                                  builder);
          } else {
            code_ += "let __" + name + " = " + type +
                     ".pack(&builder, obj: &obj." + name + ")";
            unpack_body.push_back("{{STRUCTNAME}}." + body + "__" + name +
                                  builder);
          }
          break;
        }
        case BASE_TYPE_STRING: {
          unpack_body.push_back("{{STRUCTNAME}}." + body + "__" + name +
                                builder);
          if (field.IsRequired()) {
            code_ +=
                "let __" + name + " = builder.create(string: obj." + name + ")";
          } else {
            BuildingOptionalObjects(name, "builder.create(string: s)");
          }
          break;
        }
        case BASE_TYPE_UTYPE: break;
        default:
          unpack_body.push_back("{{STRUCTNAME}}." + body + "obj." + name +
                                builder);
      }
    }
    code_ += "let __root = {{STRUCTNAME}}.start{{SHORT_STRUCTNAME}}(&builder)";
    for (auto it = unpack_body.begin(); it < unpack_body.end(); it++)
      code_ += *it;
    code_ +=
        "return {{STRUCTNAME}}.end{{SHORT_STRUCTNAME}}(&builder, start: "
        "__root)";
    Outdent();
    code_ += "}";
  }

  void GenerateVectorObjectAPITableExtension(const FieldDef &field,
                                             const std::string &name,
                                             const std::string &type) {
    auto vectortype = field.value.type.VectorType();
    switch (vectortype.base_type) {
      case BASE_TYPE_UNION: {
        code_ += "var __" + name + "__: [Offset] = []";
        code_ += "for i in obj." + name + " {";
        Indent();
        code_ += "guard let off = i?.pack(builder: &builder) else { continue }";
        code_ += "__" + name + "__.append(off)";
        Outdent();
        code_ += "}";
        code_ += "let __" + name + " = builder.createVector(ofOffsets: __" +
                 name + "__)";
        code_ += "let __" + name + "Type = builder.createVector(obj." + name +
                 ".compactMap { $0?.type })";
        break;
      }
      case BASE_TYPE_UTYPE: break;
      case BASE_TYPE_STRUCT: {
        if (field.value.type.struct_def &&
            !field.value.type.struct_def->fixed) {
          code_ += "var __" + name + "__: [Offset] = []";
          code_ += "for var i in obj." + name + " {";
          Indent();
          code_ +=
              "__" + name + "__.append(" + type + ".pack(&builder, obj: &i))";
          Outdent();
          code_ += "}";
          code_ += "let __" + name + " = builder.createVector(ofOffsets: __" +
                   name + "__)";
        } else {
          code_ += "{{STRUCTNAME}}.startVectorOf" + MakeCamel(name, true) +
                   "(obj." + name + ".count, in: &builder)";
          std::string code;
          GenerateStructArgs(*field.value.type.struct_def, &code, "", "", "_o",
                             true);
          code = code.substr(0, code.size() - 2);
          code_ += "for i in obj." + name + " {";
          Indent();
          code_ += "guard let _o = i else { continue }";
          code_ += "builder.create(struct: _o)";
          Outdent();
          code_ += "}";
          code_ += "let __" + name + " = builder.endVector(len: obj." + name +
                   ".count)";
        }
        break;
      }
      case BASE_TYPE_STRING: {
        code_ += "let __" + name + " = builder.createVector(ofStrings: obj." +
                 name + ".compactMap({ $0 }) )";
        break;
      }
      default: {
        code_ += "let __" + name + " = builder.createVector(obj." + name + ")";
        break;
      }
    }
  }

  void BuildingOptionalObjects(const std::string &name,
                               const std::string &body_front) {
    code_ += "let __" + name + ": Offset";
    code_ += "if let s = obj." + name + " {";
    Indent();
    code_ += "__" + name + " = " + body_front;
    Outdent();
    code_ += "} else {";
    Indent();
    code_ += "__" + name + " = Offset()";
    Outdent();
    code_ += "}";
    code_ += "";
  }

  void BuildObjectConstructor(const std::vector<std::string> &body,
                              const std::string &header = "") {
    code_.SetValue("HEADER", header);
    code_ += "{{ACCESS_TYPE}} init({{HEADER}}) {";
    Indent();
    for (auto it = body.begin(); it < body.end(); ++it) code_ += *it;
    Outdent();
    code_ += "}\n";
  }

  void BuildObjectAPIConstructorBody(
      const FieldDef &field, bool is_fixed,
      std::vector<std::string> &buffer_constructor,
      std::vector<std::string> &base_constructor) {
    auto name = Name(field);
    auto type = GenType(field.value.type);
    code_.SetValue("VALUENAME", name);
    code_.SetValue("VALUETYPE", type);
    std::string is_required = field.IsRequired() ? "" : "?";

    switch (field.value.type.base_type) {
      case BASE_TYPE_STRUCT: {
        type = GenType(field.value.type, true);
        code_.SetValue("VALUETYPE", type);
        auto optional =
            (field.value.type.struct_def && field.value.type.struct_def->fixed);
        std::string question_mark =
            (field.IsRequired() || (optional && is_fixed) ? "" : "?");

        code_ +=
            "{{ACCESS_TYPE}} var {{VALUENAME}}: {{VALUETYPE}}" + question_mark;
        base_constructor.push_back("" + name + " = " + type + "()");

        if (field.value.type.struct_def->fixed) {
          buffer_constructor.push_back("" + name + " = _t." + name);
        } else {
          buffer_constructor.push_back("var __" + name + " = _t." + name);
          buffer_constructor.push_back(
              "" + name + " = __" + name +
              (field.IsRequired() ? "!" : question_mark) + ".unpack()");
        }
        break;
      }
      case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();
      case BASE_TYPE_VECTOR: {
        BuildObjectAPIConstructorBodyVectors(field, name, buffer_constructor,
                                             base_constructor, "    ");
        break;
      }
      case BASE_TYPE_STRING: {
        code_ += "{{ACCESS_TYPE}} var {{VALUENAME}}: String" + is_required;
        buffer_constructor.push_back(name + " = _t." + name);

        if (field.IsRequired()) {
          std::string default_value =
              field.IsDefault() ? field.value.constant : "";
          base_constructor.push_back(name + " = \"" + default_value + "\"");
          break;
        }
        if (field.IsDefault() && !field.IsRequired()) {
          std::string value = field.IsDefault() ? field.value.constant : "nil";
          base_constructor.push_back(name + " = \"" + value + "\"");
        }
        break;
      }
      case BASE_TYPE_UTYPE: break;
      case BASE_TYPE_UNION: {
        BuildUnionEnumSwitchCase(*field.value.type.enum_def, name,
                                 buffer_constructor);
        break;
      }
      default: {
        buffer_constructor.push_back(name + " = _t." + name);
        std::string nullable = field.IsOptional() ? "?" : "";
        if (IsScalar(field.value.type.base_type) &&
            !IsBool(field.value.type.base_type) && !IsEnum(field.value.type)) {
          code_ +=
              "{{ACCESS_TYPE}} var {{VALUENAME}}: {{VALUETYPE}}" + nullable;
          if (!field.IsOptional())
            base_constructor.push_back(name + " = " + field.value.constant);
          break;
        }

        if (IsEnum(field.value.type)) {
          auto default_value = IsEnum(field.value.type)
                                   ? GenEnumDefaultValue(field)
                                   : field.value.constant;
          code_ += "{{ACCESS_TYPE}} var {{VALUENAME}}: {{VALUETYPE}}";
          base_constructor.push_back(name + " = " + default_value);
          break;
        }

        if (IsBool(field.value.type.base_type)) {
          code_ += "{{ACCESS_TYPE}} var {{VALUENAME}}: Bool" + nullable;
          std::string default_value =
              "0" == field.value.constant ? "false" : "true";
          if (!field.IsOptional())
            base_constructor.push_back(name + " = " + default_value);
        }
      }
    }
  }

  void BuildObjectAPIConstructorBodyVectors(
      const FieldDef &field, const std::string &name,
      std::vector<std::string> &buffer_constructor,
      std::vector<std::string> &base_constructor,
      const std::string &indentation) {
    auto vectortype = field.value.type.VectorType();

    if (vectortype.base_type != BASE_TYPE_UTYPE) {
      buffer_constructor.push_back(name + " = []");
      buffer_constructor.push_back("for index in 0..<_t." + name + "Count {");
      base_constructor.push_back(name + " = []");
    }

    switch (vectortype.base_type) {
      case BASE_TYPE_STRUCT: {
        code_.SetValue("VALUETYPE", GenType(vectortype, true));
        code_ += "{{ACCESS_TYPE}} var {{VALUENAME}}: [{{VALUETYPE}}?]";
        if (!vectortype.struct_def->fixed) {
          buffer_constructor.push_back(indentation + "var __v_ = _t." + name +
                                       "(at: index)");
          buffer_constructor.push_back(indentation + name +
                                       ".append(__v_?.unpack())");
        } else {
          buffer_constructor.push_back(indentation + name + ".append(_t." +
                                       name + "(at: index))");
        }
        break;
      }
      case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();
      case BASE_TYPE_VECTOR: {
        break;
      }
      case BASE_TYPE_UNION: {
        BuildUnionEnumSwitchCase(*field.value.type.enum_def, name,
                                 buffer_constructor, indentation, true);
        break;
      }
      case BASE_TYPE_UTYPE: break;
      default: {
        code_.SetValue(
            "VALUETYPE",
            (IsString(vectortype) ? "String?" : GenType(vectortype)));
        code_ += "{{ACCESS_TYPE}} var {{VALUENAME}}: [{{VALUETYPE}}]";

        if (IsEnum(vectortype) && vectortype.base_type != BASE_TYPE_UNION) {
          auto default_value = IsEnum(field.value.type)
                                   ? GenEnumDefaultValue(field)
                                   : field.value.constant;
          buffer_constructor.push_back(indentation + name + ".append(_t." +
                                       name + "(at: index)!)");
          break;
        }
        buffer_constructor.push_back(indentation + name + ".append(_t." + name +
                                     "(at: index))");
        break;
      }
    }
    if (vectortype.base_type != BASE_TYPE_UTYPE)
      buffer_constructor.push_back("}");
  }

  void BuildUnionEnumSwitchCaseWritter(const EnumDef &ev) {
    auto field_name = Name(ev);
    code_.SetValue("VALUETYPE", field_name);
    code_ += "switch type {";
    for (auto it = ev.Vals().begin(); it < ev.Vals().end(); ++it) {
      auto field = **it;
      auto ev_name = Name(field);
      auto type = GenType(field.union_type);
      auto is_struct = IsStruct(field.union_type) ? type + Mutable() : type;
      if (field.union_type.base_type == BASE_TYPE_NONE) { continue; }
      code_ += "case ." + ev_name + ":";
      Indent();
      code_ += "var __obj = value as? " + GenType(field.union_type, true);
      code_ += "return " + is_struct + ".pack(&builder, obj: &__obj)";
      Outdent();
    }
    code_ += "default: return Offset()";
    code_ += "}";
  }

  void BuildUnionEnumSwitchCase(const EnumDef &ev, const std::string &name,
                                std::vector<std::string> &buffer_constructor,
                                const std::string &indentation = "",
                                const bool is_vector = false) {
    auto field_name = NameWrappedInNameSpace(ev);
    code_.SetValue("VALUETYPE", field_name);
    code_ += "{{ACCESS_TYPE}} var {{VALUENAME}}: \\";
    code_ += is_vector ? "[{{VALUETYPE}}Union?]" : "{{VALUETYPE}}Union?";

    auto vector_reader = is_vector ? "(at: index" : "";
    buffer_constructor.push_back(indentation + "switch _t." + name + "Type" +
                                 vector_reader + (is_vector ? ")" : "") + " {");

    for (auto it = ev.Vals().begin(); it < ev.Vals().end(); ++it) {
      auto field = **it;
      auto ev_name = Name(field);
      if (field.union_type.base_type == BASE_TYPE_NONE) { continue; }
      auto type = IsStruct(field.union_type)
                      ? GenType(field.union_type) + Mutable()
                      : GenType(field.union_type);
      buffer_constructor.push_back(indentation + "case ." + ev_name + ":");
      buffer_constructor.push_back(
          indentation + "  var _v = _t." + name + (is_vector ? "" : "(") +
          vector_reader + (is_vector ? ", " : "") + "type: " + type + ".self)");
      auto constructor =
          field_name + "Union(_v?.unpack(), type: ." + ev_name + ")";
      buffer_constructor.push_back(
          indentation + "  " + name +
          (is_vector ? ".append(" + constructor + ")" : " = " + constructor));
    }
    buffer_constructor.push_back(indentation + "default: break");
    buffer_constructor.push_back(indentation + "}");
  }

  void AddMinOrMaxEnumValue(const std::string &str, const std::string &type) {
    auto current_value = str;
    code_.SetValue(type, current_value);
    code_ += "{{ACCESS_TYPE}} static var " + type +
             ": {{ENUM_NAME}} { return .{{" + type + "}} }";
  }

  void GenLookup(const FieldDef &key_field) {
    code_.SetValue("OFFSET", NumToString(key_field.value.offset));
    std::string offset_reader =
        "Table.offset(Int32(fbb.capacity) - tableOffset, vOffset: {{OFFSET}}, "
        "fbb: fbb)";

    code_.SetValue("TYPE", GenType(key_field.value.type));
    code_ +=
        "fileprivate static func lookupByKey(vector: Int32, key: {{TYPE}}, "
        "fbb: "
        "ByteBuffer) -> {{VALUENAME}}? {";
    Indent();
    if (IsString(key_field.value.type))
      code_ += "let key = key.utf8.map { $0 }";
    code_ += "var span = fbb.read(def: Int32.self, position: Int(vector - 4))";
    code_ += "var start: Int32 = 0";
    code_ += "while span != 0 {";
    Indent();
    code_ += "var middle = span / 2";
    code_ +=
        "let tableOffset = Table.indirect(vector + 4 * (start + middle), fbb)";
    if (IsString(key_field.value.type)) {
      code_ += "let comp = Table.compare(" + offset_reader + ", key, fbb: fbb)";
    } else {
      code_ += "let comp = fbb.read(def: {{TYPE}}.self, position: Int(" +
               offset_reader + "))";
    }

    code_ += "if comp > 0 {";
    Indent();
    code_ += "span = middle";
    Outdent();
    code_ += "} else if comp < 0 {";
    Indent();
    code_ += "middle += 1";
    code_ += "start += middle";
    code_ += "span -= middle";
    Outdent();
    code_ += "} else {";
    Indent();
    code_ += "return {{VALUENAME}}(fbb, o: tableOffset)";
    Outdent();
    code_ += "}";
    Outdent();
    code_ += "}";
    code_ += "return nil";
    Outdent();
    code_ += "}";
  }

  inline void GenPadding(const FieldDef &field, int *id) {
    if (field.padding) {
      for (int i = 0; i < 4; i++) {
        if (static_cast<int>(field.padding) & (1 << i)) {
          auto bits = (1 << i) * 8;
          code_ += "private let padding" + NumToString((*id)++) + "__: UInt" +
                   NumToString(bits) + " = 0";
        }
      }
      FLATBUFFERS_ASSERT(!(field.padding & ~0xF));
    }
  }

  void GenComment(const std::vector<std::string> &dc) {
    if (dc.begin() == dc.end()) {
      // Don't output empty comment blocks with 0 lines of comment content.
      return;
    }
    for (auto it = dc.begin(); it != dc.end(); ++it) { code_ += "/// " + *it; }
  }

  std::string GenOffset() {
    return "let o = {{ACCESS}}.offset({{TABLEOFFSET}}.{{OFFSET}}.v); ";
  }

  std::string GenReaderMainBody(const std::string &optional = "") {
    return "{{ACCESS_TYPE}} var {{VALUENAME}}: {{VALUETYPE}}" + optional +
           " { ";
  }

  std::string GenReader(const std::string &type,
                        const std::string &at = "{{OFFSET}}") {
    return "{{ACCESS}}.readBuffer(of: {{" + type + "}}.self, at: " + at + ")";
  }

  std::string GenConstructor(const std::string &offset) {
    return "{{VALUETYPE}}({{ACCESS}}.bb, o: " + offset + ") }";
  }

  std::string GenMutate(const std::string &offset,
                        const std::string &get_offset, bool isRaw = false) {
    return "@discardableResult {{ACCESS_TYPE}} func mutate({{VALUENAME}}: "
           "{{VALUETYPE}}) -> Bool {" +
           get_offset + " return {{ACCESS}}.mutate({{VALUENAME}}" +
           (isRaw ? ".rawValue" : "") + ", index: " + offset + ") }";
  }

  std::string GenMutateArray() {
    return "{{ACCESS_TYPE}} func mutate({{VALUENAME}}: {{VALUETYPE}}, at "
           "index: "
           "Int32) -> Bool { " +
           GenOffset() +
           "return {{ACCESS}}.directMutate({{VALUENAME}}, index: "
           "{{ACCESS}}.vector(at: o) + index * {{SIZE}}) }";
  }

  std::string GenEnumDefaultValue(const FieldDef &field) {
    auto &value = field.value;
    FLATBUFFERS_ASSERT(value.type.enum_def);
    auto &enum_def = *value.type.enum_def;
    // Vector of enum defaults are always "[]" which never works.
    const std::string constant = IsVector(value.type) ? "0" : value.constant;
    auto enum_val = enum_def.FindByValue(constant);
    std::string name;
    if (enum_val) {
      name = Name(*enum_val);
    } else {
      const auto &ev = **enum_def.Vals().begin();
      name = Name(ev);
    }
    return "." + name;
  }

  std::string GenEnumConstructor(const std::string &at) {
    return "{{VALUETYPE}}(rawValue: " + GenReader("BASEVALUE", at) + ") ";
  }

  std::string ValidateFunc() {
    return "static func validateVersion() { FlatBuffersVersion_2_0_0() }";
  }

  std::string GenType(const Type &type,
                      const bool should_consider_suffix = false) const {
    return IsScalar(type.base_type)
               ? GenTypeBasic(type)
               : (IsArray(type) ? GenType(type.VectorType())
                                : GenTypePointer(type, should_consider_suffix));
  }

  std::string GenTypePointer(const Type &type,
                             const bool should_consider_suffix) const {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "String";
      case BASE_TYPE_VECTOR: return GenType(type.VectorType());
      case BASE_TYPE_STRUCT: {
        auto &struct_ = *type.struct_def;
        if (should_consider_suffix && !struct_.fixed) {
          return WrapInNameSpace(struct_.defined_namespace,
                                 ObjectAPIName(Name(struct_)));
        }
        return WrapInNameSpace(struct_.defined_namespace, Name(struct_));
      }
      case BASE_TYPE_UNION:
      default: return "FlatbuffersInitializable";
    }
  }

  std::string GenTypeBasic(const Type &type) const {
    return GenTypeBasic(type, true);
  }

  std::string ObjectAPIName(const std::string &name) const {
    return parser_.opts.object_prefix + name + parser_.opts.object_suffix;
  }

  void Indent() { code_.IncrementIdentLevel(); }

  void Outdent() { code_.DecrementIdentLevel(); }

  std::string NameWrappedInNameSpace(const EnumDef &enum_def) const {
    return WrapInNameSpace(enum_def.defined_namespace, Name(enum_def));
  }

  std::string NameWrappedInNameSpace(const StructDef &struct_def) const {
    return WrapInNameSpace(struct_def.defined_namespace, Name(struct_def));
  }

  std::string GenTypeBasic(const Type &type, bool can_override) const {
    // clang-format off
    static const char * const swift_type[] = {
      #define FLATBUFFERS_TD(ENUM, IDLTYPE, \
              CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, RTYPE, KTYPE, STYPE) \
        #STYPE,
        FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
      #undef FLATBUFFERS_TD
    };
    // clang-format on
    if (can_override) {
      if (type.enum_def) return NameWrappedInNameSpace(*type.enum_def);
      if (type.base_type == BASE_TYPE_BOOL) return "Bool";
    }
    return swift_type[static_cast<int>(type.base_type)];
  }

  std::string EscapeKeyword(const std::string &name) const {
    return keywords_.find(name) == keywords_.end() ? name : name + "_";
  }

  std::string Mutable() const { return "_Mutable"; }

  std::string Name(const EnumVal &ev) const {
    auto name = ev.name;
    if (isupper(name.front())) {
      std::transform(name.begin(), name.end(), name.begin(), CharToLower);
    }
    return EscapeKeyword(MakeCamel(name, false));
  }

  std::string Name(const Definition &def) const {
    return EscapeKeyword(MakeCamel(def.name, false));
  }
};
}  // namespace swift
bool GenerateSwift(const Parser &parser, const std::string &path,
                   const std::string &file_name) {
  swift::SwiftGenerator generator(parser, path, file_name);
  return generator.generate();
}
}  // namespace flatbuffers
