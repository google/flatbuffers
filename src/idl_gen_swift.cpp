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
  return "\tpublic func {{VALUENAME}}(at index: Int32) -> {{VALUETYPE}}" +
         optional + " { ";
}

inline char LowerCase(char c) {
  return static_cast<char>(::tolower(static_cast<unsigned char>(c)));
}

class SwiftGenerator : public BaseGenerator {
 private:
  const Namespace *cur_name_space_;
  CodeWriter code_;
  std::unordered_set<std::string> keywords_;
  std::set<std::string> namespaces_;
  int namespace_depth;

 public:
  SwiftGenerator(const Parser &parser, const std::string &path,
                 const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", ".", "swift"),
        cur_name_space_(nullptr) {
    namespace_depth = 0;
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
      nullptr,
    };
    for (auto kw = keywords; *kw; kw++) keywords_.insert(*kw);
  }

  bool generate() {
    code_.Clear();
    code_.SetValue("ACCESS", "_accessor");
    code_ += "// " + std::string(FlatBuffersGeneratedWarning()) + "\n";
    code_ += "import FlatBuffers\n";
    // Generate code for all the enum declarations.

    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      const auto &enum_def = **it;
      if (!enum_def.generated) {
        SetNameSpace(enum_def.defined_namespace);
        GenEnum(enum_def);
      }
    }

    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      const auto &struct_def = **it;
      if (struct_def.fixed && !struct_def.generated) {
        SetNameSpace(struct_def.defined_namespace);
        GenStructReader(struct_def);
      }
    }

    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      const auto &struct_def = **it;
      if (struct_def.fixed && !struct_def.generated) {
        SetNameSpace(struct_def.defined_namespace);
        GenStructWriter(struct_def);
      }
    }

    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      const auto &struct_def = **it;
      if (!struct_def.fixed && !struct_def.generated) {
        SetNameSpace(struct_def.defined_namespace);
        GenTable(struct_def);
      }
    }

    if (cur_name_space_) SetNameSpace(nullptr);

    const auto filename = GeneratedFileName(path_, file_name_, parser_.opts);
    const auto final_code = code_.ToString();
    return SaveFile(filename.c_str(), final_code, false);
  }

  void mark(const std::string &str) {
    code_.SetValue("MARKVALUE", str);
    code_ += "\n// MARK: - {{MARKVALUE}}\n";
  }

  // Generates the create function for swift
  void GenStructWriter(const StructDef &struct_def) {
    code_.SetValue("STRUCTNAME", Name(struct_def));
    std::string static_type = this->namespace_depth == 0 ? "" : "static ";
    code_ += "public " + static_type + "func create{{STRUCTNAME}}(\\";
    std::string func_header = "";
    GenerateStructArgs(struct_def, &func_header, "");
    code_ += func_header.substr(0, func_header.size() - 2) + "\\";
    code_ += ") -> UnsafeMutableRawPointer {";
    code_ +=
        "\tlet memory = UnsafeMutableRawPointer.allocate(byteCount: "
        "{{STRUCTNAME}}.size, alignment: {{STRUCTNAME}}.alignment)";
    code_ +=
        "\tmemory.initializeMemory(as: UInt8.self, repeating: 0, count: "
        "{{STRUCTNAME}}.size)";
    GenerateStructBody(struct_def, "");
    code_ += "\treturn memory";
    code_ += "}\n";
  }

  void GenerateStructBody(const StructDef &struct_def,
                          const std::string &nameprefix, int offset = 0) {
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      auto name = nameprefix + Name(field);
      const auto &field_type = field.value.type;
      auto type = GenTypeBasic(field_type, false);
      if (IsStruct(field.value.type)) {
        GenerateStructBody(*field_type.struct_def, (nameprefix + field.name),
                           static_cast<int>(field.value.offset));
      } else {
        auto off = NumToString(offset + field.value.offset);
        code_ += "\tmemory.storeBytes(of: " + name +
                 (field_type.enum_def ? ".rawValue" : "") +
                 ", toByteOffset: " + off + ", as: " + type + ".self)";
      }
    }
  }

  void GenerateStructArgs(const StructDef &struct_def, std::string *code_ptr,
                          const std::string &nameprefix) {
    auto &code = *code_ptr;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      const auto &field_type = field.value.type;
      if (IsStruct(field.value.type)) {
        GenerateStructArgs(*field_type.struct_def, code_ptr,
                           (nameprefix + field.name));
      } else {
        auto name = Name(field);
        auto type = GenType(field.value.type);
        code += nameprefix + name + ": " + type;
        code += ", ";
      }
    }
  }

  void GenObjectHeader(const StructDef &struct_def) {
    GenComment(struct_def.doc_comment);
    code_.SetValue("STRUCTNAME", Name(struct_def));
    code_.SetValue("PROTOCOL",
                   struct_def.fixed ? "Readable" : "FlatBufferObject");
    code_.SetValue("OBJECTTYPE", struct_def.fixed ? "Struct" : "Table");
    code_ += "public struct {{STRUCTNAME}}: {{PROTOCOL}} {\n";
    code_ += ValidateFunc();
    code_ += "\tpublic var __buffer: ByteBuffer! { return {{ACCESS}}.bb }";
    code_ += "\n\tprivate var {{ACCESS}}: {{OBJECTTYPE}}";
    if (struct_def.fixed) {
      code_.SetValue("BYTESIZE", NumToString(struct_def.bytesize));
      code_.SetValue("MINALIGN", NumToString(struct_def.minalign));
      code_ += "\tpublic static var size = {{BYTESIZE}}";
      code_ += "\tpublic static var alignment = {{MINALIGN}}\t";
    } else {
      if (parser_.file_identifier_.length()) {
        code_.SetValue("FILENAME", parser_.file_identifier_);
        code_ +=
            "\tpublic static func finish(_ fbb: FlatBufferBuilder, end: "
            "Offset<UOffset>, prefix: Bool = false) { fbb.finish(offset: end, "
            "fileId: "
            "\"{{FILENAME}}\", addPrefix: prefix) }";
      }
      code_ +=
          "\tpublic static func getRootAs{{STRUCTNAME}}(bb: ByteBuffer) -> "
          "{{STRUCTNAME}} { return {{STRUCTNAME}}(Table(bb: bb, position: "
          "Int32(bb.read(def: UOffset.self, position: bb.reader)) + "
          "Int32(bb.reader))) }\n";
      code_ += "\tprivate init(_ t: Table) { {{ACCESS}} = t }";
    }
    code_ +=
        "\tpublic init(_ bb: ByteBuffer, o: Int32) { {{ACCESS}} = "
        "{{OBJECTTYPE}}(bb: "
        "bb, position: o) }";
    code_ += "";
  }

  // Generates the reader for swift
  void GenTable(const StructDef &struct_def) {
    GenObjectHeader(struct_def);
    GenTableReader(struct_def);
    GenTableWriter(struct_def);
    code_ += "}\n";
  }

  void GenTableReader(const StructDef &struct_def) {
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      GenTableReaderFields(field);
    }
  }

  void GenTableWriter(const StructDef &struct_def) {
    flatbuffers::FieldDef *key_field = nullptr;
    std::vector<std::string> require_fields;
    std::string create_func_body;
    std::string create_func_header;
    auto should_generate_create = struct_def.fields.vec.size() != 0;

    code_.SetValue("NUMBEROFFIELDS", NumToString(struct_def.fields.vec.size()));
    code_ +=
        "\tpublic static func start{{STRUCTNAME}}(_ fbb: FlatBufferBuilder) -> "
        "UOffset { fbb.startTable(with: {{NUMBEROFFIELDS}}) }";

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      if (field.key) key_field = &field;
      if (field.required)
        require_fields.push_back(NumToString(field.value.offset));

      GenTableWriterFields(
          field, &create_func_body, &create_func_header,
          static_cast<int>(it - struct_def.fields.vec.begin()));
    }
    code_ +=
        "\tpublic static func end{{STRUCTNAME}}(_ fbb: FlatBufferBuilder, "
        "start: "
        "UOffset) -> Offset<UOffset> { let end = Offset<UOffset>(offset: "
        "fbb.endTable(at: start))\\";
    if (require_fields.capacity() != 0) {
      std::string fields = "";
      for (auto it = require_fields.begin(); it != require_fields.end(); ++it)
        fields += *it + ", ";
      code_.SetValue("FIELDS", fields.substr(0, fields.size() - 2));
      code_ += "; fbb.require(table: end, fields: [{{FIELDS}}])\\";
    }
    code_ += "; return end }";

    code_ +=
        "\tpublic static func create{{STRUCTNAME}}(_ fbb: FlatBufferBuilder\\";
    if (should_generate_create)
      code_ += ",\n" +
               create_func_header.substr(0, create_func_header.size() - 2) +
               "\\";
    code_ += ") -> Offset<UOffset> {";
    code_ += "\t\tlet __start = {{STRUCTNAME}}.start{{STRUCTNAME}}(fbb)";
    if (should_generate_create)
      code_ += create_func_body.substr(0, create_func_body.size() - 1);
    code_ += "\t\treturn {{STRUCTNAME}}.end{{STRUCTNAME}}(fbb, start: __start)";
    code_ += "\t}";

    std::string spacing = "\t\t";

    if (key_field != nullptr && !struct_def.fixed && struct_def.has_key) {
      code_.SetValue("VALUENAME", struct_def.name);
      code_.SetValue("VOFFSET", NumToString(key_field->value.offset));

      code_ +=
          "\tpublic static func "
          "sortVectorOf{{VALUENAME}}(offsets:[Offset<UOffset>], "
          "_ fbb: FlatBufferBuilder) -> Offset<UOffset> {";
      code_ += spacing + "var off = offsets";
      code_ +=
          spacing +
          "off.sort { Table.compare(Table.offset(Int32($1.o), vOffset: "
          "{{VOFFSET}}, fbb: fbb.buffer), Table.offset(Int32($0.o), vOffset: "
          "{{VOFFSET}}, fbb: fbb.buffer), fbb: fbb.buffer) < 0 } ";
      code_ += spacing + "return fbb.createVector(ofOffsets: off)";
      code_ += "\t}";
      GenLookup(*key_field);
    }
  }

  void GenTableWriterFields(const FieldDef &field, std::string *create_body,
                            std::string *create_header, const int position) {
    std::string builder_string = ", _ fbb: FlatBufferBuilder) { fbb.add(";
    auto &create_func_body = *create_body;
    auto &create_func_header = *create_header;
    auto name = Name(field);
    auto type = GenType(field.value.type);
    code_.SetValue("VALUENAME", name);
    code_.SetValue("VALUETYPE", type);
    code_.SetValue("OFFSET", NumToString(position));
    code_.SetValue("CONSTANT", field.value.constant);
    std::string check_if_vector =
        (field.value.type.base_type == BASE_TYPE_VECTOR ||
         field.value.type.base_type == BASE_TYPE_ARRAY)
            ? "VectorOf("
            : "(";
    std::string body = "add" + check_if_vector + name + ": ";
    code_ += "\tpublic static func " + body + "\\";

    create_func_body += "\t\t{{STRUCTNAME}}." + body + name + ", fbb)\n";

    if (IsScalar(field.value.type.base_type) &&
        !IsBool(field.value.type.base_type)) {
      auto default_value = IsEnum(field.value.type) ? GenEnumDefaultValue(field)
                                                    : field.value.constant;
      auto is_enum = IsEnum(field.value.type) ? ".rawValue" : "";
      code_ += "{{VALUETYPE}}" + builder_string + "element: {{VALUENAME}}" +
               is_enum + ", def: {{CONSTANT}}, at: {{OFFSET}}) }";
      create_func_header +=
          "\t\t" + name + ": " + type + " = " + default_value + ",\n";
      return;
    }

    if (IsBool(field.value.type.base_type)) {
      std::string default_value =
          "0" == field.value.constant ? "false" : "true";
      code_.SetValue("VALUETYPE", "Bool");
      code_.SetValue("CONSTANT", default_value);
      code_ += "{{VALUETYPE}}" + builder_string +
               "condition: {{VALUENAME}}, def: {{CONSTANT}}, at: {{OFFSET}}) }";
      create_func_header +=
          "\t\t" + name + ": " + type + " = " + default_value + ",\n";
      return;
    }

    auto offset_type = field.value.type.base_type == BASE_TYPE_STRING
                           ? "Offset<String>"
                           : "Offset<UOffset>";
    auto camel_case_name =
        (field.value.type.base_type == BASE_TYPE_VECTOR ||
                 field.value.type.base_type == BASE_TYPE_ARRAY
             ? "vectorOf"
             : "offsetOf") +
        MakeCamel(name, true);
    create_func_header += "\t\t" + camel_case_name + " " + name + ": " +
                          offset_type + " = Offset(),\n";
    auto reader_type =
        IsStruct(field.value.type) && field.value.type.struct_def->fixed
            ? "structOffset: {{OFFSET}}) }"
            : "offset: {{VALUENAME}}, at: {{OFFSET}})  }";
    code_ += offset_type + builder_string + reader_type;
  }

  void GenTableReaderFields(const FieldDef &field) {
    auto offset = NumToString(field.value.offset);
    auto name = Name(field);
    auto type = GenType(field.value.type);
    code_.SetValue("VALUENAME", name);
    code_.SetValue("VALUETYPE", type);
    code_.SetValue("OFFSET", offset);
    code_.SetValue("CONSTANT", field.value.constant);
    std::string const_string = "return o == 0 ? {{CONSTANT}} : ";
    GenComment(field.doc_comment, "\t");
    if (IsScalar(field.value.type.base_type) && !IsEnum(field.value.type) &&
        !IsBool(field.value.type.base_type)) {
      code_ += GenReaderMainBody() + GenOffset() + const_string +
               GenReader("VALUETYPE", "o") + " }";
      if (parser_.opts.mutable_buffer) code_ += GenMutate("o", GenOffset());
      return;
    }

    if (IsBool(field.value.type.base_type)) {
      code_.SetValue("VALUETYPE", "Bool");
      code_ += GenReaderMainBody() + "\\";
      code_.SetValue("VALUETYPE", "Byte");
      code_ += GenOffset() +
               "return o == 0 ? false : 0 != " + GenReader("VALUETYPE", "o") +
               " }";
      if (parser_.opts.mutable_buffer) code_ += GenMutate("o", GenOffset());
      return;
    }

    if (IsEnum(field.value.type)) {
      auto default_value = GenEnumDefaultValue(field);
      code_.SetValue("BASEVALUE", GenTypeBasic(field.value.type, false));
      code_ += GenReaderMainBody() + "\\";
      code_ += GenOffset() + "return o == 0 ? " + default_value + " : " +
               GenEnumConstructor("o") + "?? " + default_value + " }";
      if (parser_.opts.mutable_buffer && !IsUnion(field.value.type))
        code_ += GenMutate("o", GenOffset(), true);
      return;
    }

    if (IsStruct(field.value.type) && field.value.type.struct_def->fixed) {
      code_.SetValue("VALUETYPE", GenType(field.value.type));
      code_.SetValue("CONSTANT", "nil");
      code_ += GenReaderMainBody("?") + GenOffset() + const_string +
               GenConstructor("o + {{ACCESS}}.postion");
      return;
    }
    switch (field.value.type.base_type) {
      case BASE_TYPE_STRUCT:
        code_.SetValue("VALUETYPE", GenType(field.value.type));
        code_.SetValue("CONSTANT", "nil");
        code_ += GenReaderMainBody("?") + GenOffset() + const_string +
                 GenConstructor(GenIndirect("o + {{ACCESS}}.postion"));
        break;

      case BASE_TYPE_STRING:
        code_.SetValue("VALUETYPE", GenType(field.value.type));
        code_.SetValue("CONSTANT", "nil");
        code_ += GenReaderMainBody("?") + GenOffset() + const_string +
                 "{{ACCESS}}.string(at: o) }";
        code_ +=
            "\tpublic var {{VALUENAME}}SegmentArray: [UInt8]? { return "
            "{{ACCESS}}.getVector(at: {{OFFSET}}) }";
        break;

      case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();  // fall thru
      case BASE_TYPE_VECTOR:
        GenTableReaderVectorFields(field, const_string);
        break;
      case BASE_TYPE_UNION:
        code_.SetValue("CONSTANT", "nil");
        code_ +=
            "\tpublic func {{VALUENAME}}<T: FlatBufferObject>(type: "
            "T.Type) -> T? { " +
            GenOffset() + const_string + "{{ACCESS}}.union(o) }";
        break;
      default: FLATBUFFERS_ASSERT(0);
    }
  }

  void GenTableReaderVectorFields(const FieldDef &field,
                                  const std::string &const_string) {
    auto vectortype = field.value.type.VectorType();
    code_.SetValue("SIZE", NumToString(InlineSize(vectortype)));
    code_ += "\tpublic var {{VALUENAME}}Count: Int32 { " + GenOffset() +
             const_string + "{{ACCESS}}.vector(count: o) }";
    code_.SetValue("CONSTANT", IsScalar(vectortype.base_type) == true
                                   ? field.value.constant
                                   : "nil");
    auto nullable = IsScalar(vectortype.base_type) == true ? "" : "?";
    nullable = IsEnum(vectortype) == true ? "?" : nullable;
    if (vectortype.base_type != BASE_TYPE_UNION) {
      code_ += GenArrayMainBody(nullable) + GenOffset() + "\\";
    } else {
      code_ +=
          "\tpublic func {{VALUENAME}}<T: FlatBufferObject>(at index: "
          "Int32, type: T.Type) -> T? { " +
          GenOffset() + "\\";
    }

    if (IsBool(vectortype.base_type)) {
      code_.SetValue("CONSTANT", field.value.offset == 0 ? "false" : "true");
      code_.SetValue("VALUETYPE", "Byte");
    }
    if (!IsEnum(vectortype))
      code_ +=
          const_string + (IsBool(vectortype.base_type) ? "0 != " : "") + "\\";

    if (IsScalar(vectortype.base_type) && !IsEnum(vectortype) &&
        !IsBool(field.value.type.base_type)) {
      code_ +=
          "{{ACCESS}}.directRead(of: {{VALUETYPE}}.self, offset: "
          "{{ACCESS}}.vector(at: o) + index * {{SIZE}}) }";
      code_ +=
          "\tpublic var {{VALUENAME}}: [{{VALUETYPE}}] { return "
          "{{ACCESS}}.getVector(at: {{OFFSET}}) ?? [] }";
      if (parser_.opts.mutable_buffer) code_ += GenMutateArray();
      return;
    }
    if (vectortype.base_type == BASE_TYPE_STRUCT &&
        field.value.type.struct_def->fixed) {
      code_ += GenConstructor("{{ACCESS}}.vector(at: o) + index * {{SIZE}}");
      return;
    }

    if (vectortype.base_type == BASE_TYPE_STRING) {
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
        "\tpublic func {{VALUENAME}}By(key: {{TYPE}}) -> {{VALUETYPE}}? { \\";
    code_ += GenOffset() +
             "return o == 0 ? nil : {{VALUETYPE}}.lookupByKey(vector: "
             "{{ACCESS}}.vector(at: o), key: key, fbb: {{ACCESS}}.bb) }";
  }

  // Generates the reader for swift
  void GenStructReader(const StructDef &struct_def) {
    GenObjectHeader(struct_def);
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      auto offset = NumToString(field.value.offset);
      auto name = Name(field);
      auto type = GenType(field.value.type);
      code_.SetValue("VALUENAME", name);
      code_.SetValue("VALUETYPE", type);
      code_.SetValue("OFFSET", offset);
      GenComment(field.doc_comment, "\t");
      if (IsScalar(field.value.type.base_type) && !IsEnum(field.value.type)) {
        code_ +=
            GenReaderMainBody() + "return " + GenReader("VALUETYPE") + " }";
        if (parser_.opts.mutable_buffer) code_ += GenMutate("{{OFFSET}}", "");
      } else if (IsEnum(field.value.type)) {
        code_.SetValue("BASEVALUE", GenTypeBasic(field.value.type, false));
        code_ += GenReaderMainBody() + "return " +
                 GenEnumConstructor("{{OFFSET}}") + "?? " +
                 GenEnumDefaultValue(field) + " }";
      } else if (IsStruct(field.value.type)) {
        code_.SetValue("VALUETYPE", GenType(field.value.type));
        code_ += GenReaderMainBody() + "return " +
                 GenConstructor("{{ACCESS}}.postion + {{OFFSET}}");
      }
    }

    code_ += "}\n";
  }

  void GenEnum(const EnumDef &enum_def) {
    if (enum_def.generated) return;
    code_.SetValue("ENUM_NAME", Name(enum_def));
    code_.SetValue("BASE_TYPE", GenTypeBasic(enum_def.underlying_type, false));
    GenComment(enum_def.doc_comment);
    code_ += "public enum {{ENUM_NAME}}: {{BASE_TYPE}}, Enum { ";
    code_ += "\tpublic typealias T = {{BASE_TYPE}}";
    code_ +=
        "\tpublic static var byteSize: Int { return "
        "MemoryLayout<{{BASE_TYPE}}>.size "
        "}";
    code_ += "\tpublic var value: {{BASE_TYPE}} { return self.rawValue }";

    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      const auto &ev = **it;
      auto name = Name(ev);
      std::transform(name.begin(), name.end(), name.begin(), LowerCase);
      code_.SetValue("KEY", name);
      code_.SetValue("VALUE", enum_def.ToString(ev));
      GenComment(ev.doc_comment, "\t");
      code_ += "\tcase {{KEY}} = {{VALUE}}";
    }
    code_ += "\n";
    AddMinOrMaxEnumValue(enum_def.MaxValue()->name, "max");
    AddMinOrMaxEnumValue(enum_def.MinValue()->name, "min");
    code_ += "}\n";
  }

  void AddMinOrMaxEnumValue(const std::string &str, const std::string &type) {
    auto current_value = str;
    std::transform(current_value.begin(), current_value.end(),
                   current_value.begin(), LowerCase);
    code_.SetValue(type, current_value);
    code_ += "\tpublic static var " + type + ": {{ENUM_NAME}} { return .{{" +
             type + "}} }";
  }

  void GenLookup(const FieldDef &key_field) {
    code_.SetValue("OFFSET", NumToString(key_field.value.offset));
    auto offset_reader =
        "Table.offset(Int32(fbb.capacity) - tableOffset, vOffset: {{OFFSET}}, "
        "fbb: fbb)";
    std::string spacing = "\t\t";
    std::string double_spacing = spacing + "\t";

    code_.SetValue("TYPE", GenType(key_field.value.type));
    code_ +=
        "\tfileprivate static func lookupByKey(vector: Int32, key: {{TYPE}}, "
        "fbb: "
        "ByteBuffer) -> {{VALUENAME}}? {";

    if (key_field.value.type.base_type == BASE_TYPE_STRING)
      code_ += spacing + "let key = key.utf8.map { $0 }";
    code_ += spacing +
             "var span = fbb.read(def: Int32.self, position: Int(vector - 4))";
    code_ += spacing + "var start: Int32 = 0";
    code_ += spacing + "while span != 0 {";
    code_ += double_spacing + "var middle = span / 2";
    code_ +=
        double_spacing +
        "let tableOffset = Table.indirect(vector + 4 * (start + middle), fbb)";
    if (key_field.value.type.base_type == BASE_TYPE_STRING) {
      code_ += double_spacing + "let comp = Table.compare(" + offset_reader +
               ", key, fbb: fbb)";
    } else {
      code_ += double_spacing +
               "let comp = fbb.read(def: {{TYPE}}.self, position: Int(" +
               offset_reader + "))";
    }

    code_ += double_spacing + "if comp > 0 {";
    code_ += double_spacing + "\tspan = middle";
    code_ += double_spacing + "} else if comp < 0 {";
    code_ += double_spacing + "\tmiddle += 1";
    code_ += double_spacing + "\tstart += middle";
    code_ += double_spacing + "\tspan -= middle";
    code_ += double_spacing + "} else {";
    code_ += double_spacing + "\treturn {{VALUENAME}}(fbb, o: tableOffset)";
    code_ += double_spacing + "}";
    code_ += spacing + "}";
    code_ += spacing + "return nil";
    code_ += "\t}";
  }

  void GenComment(const std::vector<std::string> &dc, const char *prefix = "") {
    std::string text;
    ::flatbuffers::GenComment(dc, &text, nullptr, prefix);
    code_ += text + "\\";
  }

  std::string GenOffset() { return "let o = {{ACCESS}}.offset({{OFFSET}}); "; }

  std::string GenReaderMainBody(const std::string &optional = "") {
    return "\tpublic var {{VALUENAME}}: {{VALUETYPE}}" + optional + " { ";
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
    return "\tpublic func mutate({{VALUENAME}}: {{VALUETYPE}}) -> Bool {" +
           get_offset + " return {{ACCESS}}.mutate({{VALUENAME}}" +
           (isRaw ? ".rawValue" : "") + ", index: " + offset + ") }";
  }

  std::string GenMutateArray() {
    return "\tpublic func mutate({{VALUENAME}}: {{VALUETYPE}}, at index: "
           "Int32) -> Bool { " +
           GenOffset() +
           "return {{ACCESS}}.directMutate({{VALUENAME}}, index: "
           "{{ACCESS}}.vector(at: o) + index * {{SIZE}}) }";
  }

  std::string GenEnumDefaultValue(const FieldDef &field) {
    auto &value = field.value;
    FLATBUFFERS_ASSERT(value.type.enum_def);
    auto &enum_def = *value.type.enum_def;
    auto enum_val = enum_def.FindByValue(value.constant);
    std::string name;
    if (enum_val) {
      name = enum_val->name;
    } else {
      const auto &ev = **enum_def.Vals().begin();
      name = ev.name;
    }
    std::transform(name.begin(), name.end(), name.begin(), LowerCase);
    return "." + name;
  }

  std::string GenEnumConstructor(const std::string &at) {
    return "{{VALUETYPE}}(rawValue: " + GenReader("BASEVALUE", at) + ") ";
  }

  std::string ValidateFunc() {
    return "\tstatic func validateVersion() { FlatBuffersVersion_1_11_1() }";
  }

  std::string GenType(const Type &type) const {
    return IsScalar(type.base_type)
               ? GenTypeBasic(type)
               : (IsArray(type) ? GenType(type.VectorType())
                                : GenTypePointer(type));
  }

  std::string GenTypePointer(const Type &type) const {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "String";
      case BASE_TYPE_VECTOR: return GenType(type.VectorType());
      case BASE_TYPE_STRUCT: return WrapInNameSpace(*type.struct_def);
      case BASE_TYPE_UNION:
      default: return "FlatBufferObject";
    }
  }

  std::string GenTypeBasic(const Type &type) const {
    return GenTypeBasic(type, true);
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
      if (type.enum_def)
        return WrapInNameSpace(type.enum_def->defined_namespace,
                               Name(*type.enum_def));
      if (type.base_type == BASE_TYPE_BOOL) return "Bool";
    }
    return swift_type[static_cast<int>(type.base_type)];
  }

  std::string EscapeKeyword(const std::string &name) const {
    return keywords_.find(name) == keywords_.end() ? name : name + "_";
  }

  std::string Name(const EnumVal &ev) const { return EscapeKeyword(ev.name); }

  std::string Name(const Definition &def) const {
    return EscapeKeyword(MakeCamel(def.name, false));
  }

  // MARK: - Copied from the cpp implementation, needs revisiting
  void SetNameSpace(const Namespace *ns) {
    if (cur_name_space_ == ns) { return; }
    // Compute the size of the longest common namespace prefix.
    // If cur_name_space is A::B::C::D and ns is A::B::E::F::G,
    // the common prefix is A::B:: and we have old_size = 4, new_size = 5
    // and common_prefix_size = 2
    size_t old_size = cur_name_space_ ? cur_name_space_->components.size() : 0;
    size_t new_size = ns ? ns->components.size() : 0;

    size_t common_prefix_size = 0;
    while (common_prefix_size < old_size && common_prefix_size < new_size &&
           ns->components[common_prefix_size] ==
               cur_name_space_->components[common_prefix_size]) {
      common_prefix_size++;
    }

    // Close cur_name_space in reverse order to reach the common prefix.
    // In the previous example, D then C are closed.
    for (size_t j = old_size; j > common_prefix_size; --j) {
      if (namespace_depth != 0) {
        code_ += "}";
        namespace_depth -= 1;
      }
      mark(cur_name_space_->components[j - 1]);
    }
    if (old_size != common_prefix_size) { code_ += ""; }

    // open namespace parts to reach the ns namespace
    // in the previous example, E, then F, then G are opened
    bool is_extension = false;
    for (auto j = common_prefix_size; j < new_size; ++j) {
      std::string name = ns->components[j];
      if (namespaces_.find(name) == namespaces_.end()) {
        code_ += "public enum " + name + " {";
        namespace_depth += 1;
        namespaces_.insert(name);
      } else {
        code_ += "}";
        is_extension = true;
      }
    }
    if (is_extension) {
      code_.SetValue("EXTENSION", FullNamespace(".", *ns));
      code_ += "extension {{EXTENSION}} {";
    }
    if (new_size != common_prefix_size) { code_ += ""; }
    cur_name_space_ = ns;
  }
};
}  // namespace swift
bool GenerateSwift(const Parser &parser, const std::string &path,
                   const std::string &file_name) {
  swift::SwiftGenerator generator(parser, path, file_name);
  return generator.generate();
}
}  // namespace flatbuffers
