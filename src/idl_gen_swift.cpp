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
  return "public func {{VALUENAME}}(at index: Int32) -> {{VALUETYPE}}" +
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
    code_.SetPadding("    ");
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
        if (parser_.opts.generate_object_based_api) {
          GenObjectAPI(struct_def);
        }
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
        if (parser_.opts.generate_object_based_api) {
          GenObjectAPI(struct_def);
        }
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
    GenerateStructArgs(struct_def, &func_header, "", "");
    code_ += func_header.substr(0, func_header.size() - 2) + "\\";
    code_ += ") -> UnsafeMutableRawPointer {";
    Indent();
    code_ +=
        "let memory = UnsafeMutableRawPointer.allocate(byteCount: "
        "{{STRUCTNAME}}.size, alignment: {{STRUCTNAME}}.alignment)";
    code_ +=
        "memory.initializeMemory(as: UInt8.self, repeating: 0, count: "
        "{{STRUCTNAME}}.size)";
    GenerateStructBody(struct_def, "");
    code_ += "return memory";
    Outdent();
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
        code_ += "memory.storeBytes(of: " + name +
                 (field_type.enum_def ? ".rawValue" : "") +
                 ", toByteOffset: " + off + ", as: " + type + ".self)";
      }
    }
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
          code += ", ";
          continue;
        }
        code +=
            nameprefix + name + ": " + obj_api_named + object_name + "." + name;
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
    Indent();
    code_ += ValidateFunc();
    code_ += "public var __buffer: ByteBuffer! { return {{ACCESS}}.bb }";
    code_ += "private var {{ACCESS}}: {{OBJECTTYPE}}\n";
    if (struct_def.fixed) {
      code_.SetValue("BYTESIZE", NumToString(struct_def.bytesize));
      code_.SetValue("MINALIGN", NumToString(struct_def.minalign));
      code_ += "public static var size = {{BYTESIZE}}";
      code_ += "public static var alignment = {{MINALIGN}}";
    } else {
      if (parser_.file_identifier_.length()) {
        code_.SetValue("FILENAME", parser_.file_identifier_);
        code_ +=
            "public static func finish(_ fbb: inout FlatBufferBuilder, end: "
            "Offset<UOffset>, prefix: Bool = false) { fbb.finish(offset: end, "
            "fileId: "
            "\"{{FILENAME}}\", addPrefix: prefix) }";
      }
      code_ +=
          "public static func getRootAs{{STRUCTNAME}}(bb: ByteBuffer) -> "
          "{{STRUCTNAME}} { return {{STRUCTNAME}}(Table(bb: bb, position: "
          "Int32(bb.read(def: UOffset.self, position: bb.reader)) + "
          "Int32(bb.reader))) }\n";
      code_ += "private init(_ t: Table) { {{ACCESS}} = t }";
    }
    code_ +=
        "public init(_ bb: ByteBuffer, o: Int32) { {{ACCESS}} = "
        "{{OBJECTTYPE}}(bb: "
        "bb, position: o) }";
    code_ += "";
  }

  // Generates the reader for swift
  void GenTable(const StructDef &struct_def) {
    GenObjectHeader(struct_def);
    GenTableReader(struct_def);
    GenTableWriter(struct_def);
    if (parser_.opts.generate_object_based_api)
      GenerateObjectAPITableExtension(struct_def);
    Outdent();
    code_ += "}\n";
  }

  void GenerateObjectAPIExtensionHeader() {
    code_ += "\n";
    code_ += "public mutating func unpack() -> {{STRUCTNAME}}T {";
    Indent();
    code_ += "return {{STRUCTNAME}}T(&self)";
    Outdent();
    code_ += "}";
    code_ +=
        "public static func pack(_ builder: inout FlatBufferBuilder, obj: "
        "inout {{STRUCTNAME}}T?) -> Offset<UOffset> {";
    Indent();
    code_ += "guard let obj = obj else { return Offset<UOffset>() }";
    code_ += "";
  }

  void GenerateObjectAPIStructExtension(const StructDef &struct_def) {
    GenerateObjectAPIExtensionHeader();
    std::string code;
    GenerateStructArgs(struct_def, &code, "", "", "obj", true);
    code_ += "return builder.create(struct: create{{STRUCTNAME}}(\\";
    code_ += code.substr(0, code.size() - 2) + "\\";
    code_ += "), type: {{STRUCTNAME}}.self)";
    Outdent();
    code_ += "}";
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
    std::vector<std::string> create_func_body;
    std::vector<std::string> create_func_header;
    auto should_generate_create = struct_def.fields.vec.size() != 0;

    code_.SetValue("NUMBEROFFIELDS", NumToString(struct_def.fields.vec.size()));
    code_ +=
        "public static func start{{STRUCTNAME}}(_ fbb: inout "
        "FlatBufferBuilder) -> "
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
        "public static func end{{STRUCTNAME}}(_ fbb: inout "
        "FlatBufferBuilder, "
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

    if (should_generate_create) {
      code_ +=
          "public static func create{{STRUCTNAME}}(_ fbb: inout "
          "FlatBufferBuilder,";
      for (auto it = create_func_header.begin(); it < create_func_header.end();
           ++it) {
        code_ += *it + "\\";
        if (it < create_func_header.end() - 1) code_ += ",";
      }
      code_ += ") -> Offset<UOffset> {";
      Indent();
      code_ += "let __start = {{STRUCTNAME}}.start{{STRUCTNAME}}(&fbb)";
      for (auto it = create_func_body.begin(); it < create_func_body.end();
           ++it) {
        code_ += *it;
      }
      code_ += "return {{STRUCTNAME}}.end{{STRUCTNAME}}(&fbb, start: __start)";
      Outdent();
      code_ += "}";
    }

    std::string spacing = "";

    if (key_field != nullptr && !struct_def.fixed && struct_def.has_key) {
      code_.SetValue("VALUENAME", struct_def.name);
      code_.SetValue("VOFFSET", NumToString(key_field->value.offset));

      code_ +=
          "public static func "
          "sortVectorOf{{VALUENAME}}(offsets:[Offset<UOffset>], "
          "_ fbb: inout FlatBufferBuilder) -> Offset<UOffset> {";
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
                            std::vector<std::string> *create_header,
                            const int position) {
    std::string builder_string = ", _ fbb: inout FlatBufferBuilder) { fbb.add(";
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
    auto body = "add" + check_if_vector + name + ": ";
    code_ += "public static func " + body + "\\";

    create_func_body.push_back("{{STRUCTNAME}}." + body + name + ", &fbb)");

    if (IsScalar(field.value.type.base_type) &&
        !IsBool(field.value.type.base_type)) {
      auto default_value = IsEnum(field.value.type) ? GenEnumDefaultValue(field)
                                                    : field.value.constant;
      auto is_enum = IsEnum(field.value.type) ? ".rawValue" : "";
      code_ += "{{VALUETYPE}}" + builder_string + "element: {{VALUENAME}}" +
               is_enum + ", def: {{CONSTANT}}, at: {{OFFSET}}) }";
      create_func_header.push_back("" + name + ": " + type + " = " +
                                   default_value);
      return;
    }

    if (IsBool(field.value.type.base_type)) {
      std::string default_value =
          "0" == field.value.constant ? "false" : "true";
      code_.SetValue("VALUETYPE", "Bool");
      code_.SetValue("CONSTANT", default_value);
      code_ += "{{VALUETYPE}}" + builder_string +
               "condition: {{VALUENAME}}, def: {{CONSTANT}}, at: {{OFFSET}}) }";
      create_func_header.push_back(name + ": " + type + " = " + default_value);
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
    create_func_header.push_back(camel_case_name + " " + name + ": " +
                                 offset_type + " = Offset()");
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
    GenComment(field.doc_comment);
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
            "public var {{VALUENAME}}SegmentArray: [UInt8]? { return "
            "{{ACCESS}}.getVector(at: {{OFFSET}}) }";
        break;

      case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();  // fall thru
      case BASE_TYPE_VECTOR:
        GenTableReaderVectorFields(field, const_string);
        break;
      case BASE_TYPE_UNION:
        code_.SetValue("CONSTANT", "nil");
        code_ +=
            "public func {{VALUENAME}}<T: FlatBufferObject>(type: "
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
    code_ += "public var {{VALUENAME}}Count: Int32 { " + GenOffset() +
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
          "public func {{VALUENAME}}<T: FlatBufferObject>(at index: "
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
          "public var {{VALUENAME}}: [{{VALUETYPE}}] { return "
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
        "public func {{VALUENAME}}By(key: {{TYPE}}) -> {{VALUETYPE}}? { \\";
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
      GenComment(field.doc_comment);
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
    if (parser_.opts.generate_object_based_api)
      GenerateObjectAPIStructExtension(struct_def);
    Outdent();
    code_ += "}\n";
  }

  void GenEnum(const EnumDef &enum_def) {
    if (enum_def.generated) return;
    code_.SetValue("ENUM_NAME", Name(enum_def));
    code_.SetValue("BASE_TYPE", GenTypeBasic(enum_def.underlying_type, false));
    GenComment(enum_def.doc_comment);
    code_ += "public enum {{ENUM_NAME}}: {{BASE_TYPE}}, Enum { ";
    Indent();
    code_ += "public typealias T = {{BASE_TYPE}}";
    code_ +=
        "public static var byteSize: Int { return "
        "MemoryLayout<{{BASE_TYPE}}>.size "
        "}";
    code_ += "public var value: {{BASE_TYPE}} { return self.rawValue }";
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      const auto &ev = **it;
      auto name = Name(ev);
      std::transform(name.begin(), name.end(), name.begin(), LowerCase);
      code_.SetValue("KEY", name);
      code_.SetValue("VALUE", enum_def.ToString(ev));
      GenComment(ev.doc_comment);
      code_ += "case {{KEY}} = {{VALUE}}";
    }
    code_ += "\n";
    AddMinOrMaxEnumValue(enum_def.MaxValue()->name, "max");
    AddMinOrMaxEnumValue(enum_def.MinValue()->name, "min");
    Outdent();
    code_ += "}\n";
    if (parser_.opts.generate_object_based_api && enum_def.is_union) {
      code_ += "struct {{ENUM_NAME}}Union {";
      Indent();
      code_ += "var type: {{ENUM_NAME}}";
      code_ += "var value: NativeTable?";
      code_ += "init(_ v: NativeTable?, type: {{ENUM_NAME}}) {";
      Indent();
      code_ += "self.type = type";
      code_ += "self.value = v";
      Outdent();
      code_ += "}";
      code_ +=
          "func pack(builder: inout FlatBufferBuilder) -> Offset<UOffset> {";
      Indent();
      BuildUnionEnumSwitchCaseWritter(enum_def);
      Outdent();
      code_ += "}";
      Outdent();
      code_ += "}";
    }
  }

  void GenObjectAPI(const StructDef &struct_def) {
    code_ += "public class {{STRUCTNAME}}T: NativeTable {\n";
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
    BuildObjectAPIConstructor(buffer_constructor,
                              "_ _t: inout " + struct_def.name);
    BuildObjectAPIConstructor(base_constructor);
    Outdent();
    code_ += "}";
  }

  void GenerateObjectAPITableExtension(const StructDef &struct_def) {
    GenerateObjectAPIExtensionHeader();
    std::vector<std::string> unpack_body;
    std::string builder = ", &builder)";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      auto name = Name(field);
      auto type = GenType(field.value.type);
      std::string check_if_vector =
          (field.value.type.base_type == BASE_TYPE_VECTOR ||
           field.value.type.base_type == BASE_TYPE_ARRAY)
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
            std::string struct_builder =
                type + ".pack(&builder, obj: &obj." + name + ")";
            unpack_body.push_back("{{STRUCTNAME}}." + body + struct_builder +
                                  builder);
          } else {
            unpack_body.push_back("{{STRUCTNAME}}." + body + "__" + name +
                                  builder);
            code_ += "let __" + name + " = " + type +
                     ".pack(&builder, obj: &obj." + name + ")";
          }
          break;
        }
        case BASE_TYPE_STRING: {
          unpack_body.push_back("{{STRUCTNAME}}." + body + "__" + name +
                                builder);
          BuildingOptionalObjects(name, "String", "builder.create(string: s)");
          break;
        }
        case BASE_TYPE_UTYPE: break;
        default:
          unpack_body.push_back("{{STRUCTNAME}}." + body + "obj." + name +
                                builder);
      }
    }
    code_ += "let __root = {{STRUCTNAME}}.start{{STRUCTNAME}}(&builder)";
    for (auto it = unpack_body.begin(); it < unpack_body.end(); it++)
      code_ += *it;
    code_ +=
        "return {{STRUCTNAME}}.end{{STRUCTNAME}}(&builder, start: "
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
        code_ += "var __" + name + "__: [Offset<UOffset>] = []";
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
          code_ += "var __" + name + "__: [Offset<UOffset>] = []";
          code_ += "for var i in obj." + name + " {";
          Indent();
          code_ +=
              "__" + name + "__.append(" + type + ".pack(&builder, obj: &i))";
          Outdent();
          code_ += "}";
          code_ += "let __" + name + " = builder.createVector(ofOffsets: __" +
                   name + "__)";
        } else {
          code_ += "var __" + name + "__: [UnsafeMutableRawPointer] = []";
          std::string code;
          GenerateStructArgs(*field.value.type.struct_def, &code, "", "", "_o",
                             true);
          code = code.substr(0, code.size() - 2);
          code_ += "for i in obj." + name + " {";
          Indent();
          code_ += "guard let _o = i else { continue }";
          code_ += "__" + name + "__.append(create" +
                   field.value.type.struct_def->name + "(" + code + "))";
          Outdent();
          code_ += "}";
          code_ += "let __" + name + " = builder.createVector(structs: __" +
                   name + "__, type: " + type + ".self)";
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
                               const std::string &object_type,
                               const std::string &body_front) {
    code_ += "let __" + name + ": Offset<" + object_type + ">";
    code_ += "if let s = obj." + name + " {";
    Indent();
    code_ += "__" + name + " = " + body_front;
    Outdent();
    code_ += "} else {";
    Indent();
    code_ += "__" + name + " = Offset<" + object_type + ">()";
    Outdent();
    code_ += "}";
    code_ += "";
  }

  void BuildObjectAPIConstructor(const std::vector<std::string> &body,
                                 const std::string &header = "") {
    code_.SetValue("HEADER", header);
    code_ += "init({{HEADER}}) {";
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

    switch (field.value.type.base_type) {
      case BASE_TYPE_STRUCT: {
        code_.SetValue("VALUETYPE", type + "T");
        buffer_constructor.push_back("var __" + name + " = _t." + name);
        auto optional =
            (field.value.type.struct_def && field.value.type.struct_def->fixed);
        std::string question_mark = (optional && is_fixed ? "" : "?");
        code_ += "var {{VALUENAME}}: {{VALUETYPE}}" + question_mark;
        buffer_constructor.push_back("" + name + " = __" + name +
                                     question_mark + ".unpack()");
        base_constructor.push_back("" + name + " = " + type + "T()");
        break;
      }
      case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();
      case BASE_TYPE_VECTOR: {
        BuildObjectAPIConstructorBodyVectors(field, name, buffer_constructor,
                                             base_constructor, "    ");
        break;
      }
      case BASE_TYPE_STRING: {
        code_ += "var {{VALUENAME}}: String?";
        buffer_constructor.push_back(name + " = _t." + name);
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

        if (IsScalar(field.value.type.base_type) &&
            !IsBool(field.value.type.base_type) && !IsEnum(field.value.type)) {
          code_ += "var {{VALUENAME}}: {{VALUETYPE}}";
          base_constructor.push_back(name + " = " + field.value.constant);
          break;
        }

        if (IsEnum(field.value.type)) {
          auto default_value = IsEnum(field.value.type)
                                   ? GenEnumDefaultValue(field)
                                   : field.value.constant;
          code_ += "var {{VALUENAME}}: {{VALUETYPE}}";
          base_constructor.push_back(name + " = " + default_value);
          break;
        }

        if (IsBool(field.value.type.base_type)) {
          code_ += "var {{VALUENAME}}: Bool";
          std::string default_value =
              "0" == field.value.constant ? "false" : "true";
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
        code_.SetValue("VALUETYPE", GenType(vectortype) + "T");
        code_ += "var {{VALUENAME}}: [{{VALUETYPE}}?]";
        buffer_constructor.push_back(indentation + "var __v_ = _t." + name +
                                     "(at: index)");
        buffer_constructor.push_back(indentation + name +
                                     ".append(__v_?.unpack())");
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
        code_.SetValue("VALUETYPE", (vectortype.base_type == BASE_TYPE_STRING
                                         ? "String?"
                                         : GenType(vectortype)));
        code_ += "var {{VALUENAME}}: [{{VALUETYPE}}]";

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
    auto field_name = EscapeKeyword(ev.name);
    code_.SetValue("VALUETYPE", field_name);
    code_ += "switch type {";
    for (auto it = ev.Vals().begin(); it < ev.Vals().end(); ++it) {
      auto field = **it;
      auto ev_name = Name(field);
      auto type = GenType(field.union_type);
      std::transform(ev_name.begin(), ev_name.end(), ev_name.begin(),
                     LowerCase);
      if (field.union_type.base_type == BASE_TYPE_NONE ||
          field.union_type.base_type == BASE_TYPE_STRING) {
        continue;
      }
      code_ += "case ." + ev_name + ":";
      Indent();
      code_ += "var __obj = value as? " + type + "T";
      code_ += "return " + type + ".pack(&builder, obj: &__obj)";
      Outdent();
    }
    code_ += "default: return Offset()";
    code_ += "}";
  }

  void BuildUnionEnumSwitchCase(const EnumDef &ev, const std::string &name,
                                std::vector<std::string> &buffer_constructor,
                                const std::string &indentation = "",
                                const bool is_vector = false) {
    auto field_name = EscapeKeyword(ev.name);
    code_.SetValue("VALUETYPE", field_name);
    code_ += "var {{VALUENAME}}: \\";
    code_ += is_vector ? "[{{VALUETYPE}}Union?]" : "{{VALUETYPE}}Union?";

    auto vector_reader = is_vector ? "(at: index" : "";
    buffer_constructor.push_back(indentation + "switch _t." + name + "Type" +
                                 vector_reader + (is_vector ? ")" : "") + " {");

    for (auto it = ev.Vals().begin(); it < ev.Vals().end(); ++it) {
      auto field = **it;
      auto ev_name = Name(field);
      std::transform(ev_name.begin(), ev_name.end(), ev_name.begin(),
                     LowerCase);
      if (field.union_type.base_type == BASE_TYPE_NONE ||
          field.union_type.base_type == BASE_TYPE_STRING) {
        continue;
      }
      buffer_constructor.push_back(indentation + "case ." + ev_name + ":");
      buffer_constructor.push_back(
          indentation + "    var _v = _t." + name + (is_vector ? "" : "(") +
          vector_reader + (is_vector ? ", " : "") +
          "type: " + GenType(field.union_type) + ".self)");
      auto constructor =
          field_name + "Union(_v?.unpack(), type: ." + ev_name + ")";
      buffer_constructor.push_back(
          indentation + "    " + name +
          (is_vector ? ".append(" + constructor + ")" : " = " + constructor));
    }
    buffer_constructor.push_back(indentation + "default: break");
    buffer_constructor.push_back(indentation + "}");
  }

  void AddMinOrMaxEnumValue(const std::string &str, const std::string &type) {
    auto current_value = str;
    std::transform(current_value.begin(), current_value.end(),
                   current_value.begin(), LowerCase);
    code_.SetValue(type, current_value);
    code_ += "public static var " + type + ": {{ENUM_NAME}} { return .{{" +
             type + "}} }";
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
    if (key_field.value.type.base_type == BASE_TYPE_STRING)
      code_ += "let key = key.utf8.map { $0 }";
    code_ += "var span = fbb.read(def: Int32.self, position: Int(vector - 4))";
    code_ += "var start: Int32 = 0";
    code_ += "while span != 0 {";
    Indent();
    code_ += "var middle = span / 2";
    code_ +=
        "let tableOffset = Table.indirect(vector + 4 * (start + middle), fbb)";
    if (key_field.value.type.base_type == BASE_TYPE_STRING) {
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

  void GenComment(const std::vector<std::string> &dc) {
    if (dc.begin() == dc.end()) {
      // Don't output empty comment blocks with 0 lines of comment content.
      return;
    }
    for (auto it = dc.begin(); it != dc.end(); ++it) { code_ += "/// " + *it; }
  }

  std::string GenOffset() { return "let o = {{ACCESS}}.offset({{OFFSET}}); "; }

  std::string GenReaderMainBody(const std::string &optional = "") {
    return "public var {{VALUENAME}}: {{VALUETYPE}}" + optional + " { ";
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
    return "public func mutate({{VALUENAME}}: {{VALUETYPE}}) -> Bool {" +
           get_offset + " return {{ACCESS}}.mutate({{VALUENAME}}" +
           (isRaw ? ".rawValue" : "") + ", index: " + offset + ") }";
  }

  std::string GenMutateArray() {
    return "public func mutate({{VALUENAME}}: {{VALUETYPE}}, at index: "
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
    return "static func validateVersion() { FlatBuffersVersion_1_12_0() }";
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

  void Indent() { code_.IncrementIdentLevel(); }

  void Outdent() { code_.DecrementIdentLevel(); }

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
      if (namespace_depth >= 0) {
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
        if (namespace_depth != 0) {
          code_ += "}";
          namespace_depth = 0;
        }
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
