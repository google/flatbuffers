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
#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <unordered_set>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

struct ImportDefinition {
  std::string name;
  std::string import_statement;
  std::string export_statement;
  std::string bare_file_path;
  std::string rel_file_path;
  const Definition *dependent;
  const Definition *dependency;
};

enum AnnotationType { kParam = 0, kType = 1, kReturns = 2 };

namespace ts {
// Iterate through all definitions we haven't generate code for (enums, structs,
// and tables) and output them to a single file.
class TsGenerator : public BaseGenerator {
 public:
  typedef std::map<std::string, ImportDefinition> import_set;

  TsGenerator(const Parser &parser, const std::string &path,
              const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", ".", "ts") {}
  bool generate() {
    generateEnums();
    generateStructs();
    generateEntry();
    return true;
  }

  // Save out the generated code for a single class while adding
  // declaration boilerplate.
  bool SaveType(const Definition &definition, const std::string &classcode,
                import_set &imports, import_set &bare_imports) const {
    if (!classcode.length()) return true;

    std::string code =
        "// " + std::string(FlatBuffersGeneratedWarning()) + "\n\n";

    for (auto it = bare_imports.begin(); it != bare_imports.end(); it++)
      code += it->second.import_statement + "\n";
    if (!bare_imports.empty()) code += "\n";

    for (auto it = imports.begin(); it != imports.end(); it++)
      if (it->second.dependency != &definition)  // do not import itself
        code += it->second.import_statement + "\n";
    if (!imports.empty()) code += "\n\n";

    code += classcode;
    auto filename = NamespaceDir(*definition.defined_namespace, true) +
                    ToDasherizedCase(definition.name) + ".ts";
    return SaveFile(filename.c_str(), code, false);
  }

 private:
  import_set imports_all_;

  // Generate code for all enums.
  void generateEnums() {
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      import_set bare_imports;
      import_set imports;
      std::string enumcode;
      auto &enum_def = **it;
      GenEnum(enum_def, &enumcode, imports, false);
      GenEnum(enum_def, &enumcode, imports, true);
      SaveType(enum_def, enumcode, imports, bare_imports);
      imports_all_.insert(imports.begin(), imports.end());
    }
  }

  // Generate code for all structs.
  void generateStructs() {
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      import_set bare_imports;
      import_set imports;
      AddImport(bare_imports, "* as flatbuffers", "flatbuffers");
      auto &struct_def = **it;
      std::string declcode;
      GenStruct(parser_, struct_def, &declcode, imports);
      SaveType(struct_def, declcode, imports, bare_imports);
      imports_all_.insert(imports.begin(), imports.end());
    }
  }

  // Generate code for a single entry point module.
  void generateEntry() {
    std::string code;
    for (auto it = imports_all_.begin(); it != imports_all_.end(); it++)
      code += it->second.export_statement + "\n";
    std::string path = "./" + path_ + file_name_ + ".ts";
    SaveFile(path.c_str(), code, false);
  }

  // Generate a documentation comment, if available.
  static void GenDocComment(const std::vector<std::string> &dc,
                            std::string *code_ptr,
                            const char *indent = nullptr) {
    if (dc.empty()) {
      // Don't output empty comment blocks with 0 lines of comment content.
      return;
    }

    std::string &code = *code_ptr;
    if (indent) code += indent;
    code += "/**\n";
    for (auto it = dc.begin(); it != dc.end(); ++it) {
      if (indent) code += indent;
      code += " *" + *it + "\n";
    }
    if (indent) code += indent;
    code += " */\n";
  }

  static void GenDocComment(std::string *code_ptr) {
    GenDocComment(std::vector<std::string>(), code_ptr);
  }

  // Generate an enum declaration and an enum string lookup table.
  void GenEnum(EnumDef &enum_def, std::string *code_ptr, import_set &imports,
               bool reverse) {
    if (enum_def.generated) return;
    if (reverse) return;  // FIXME.
    std::string &code = *code_ptr;
    GenDocComment(enum_def.doc_comment, code_ptr);
    std::string ns = GetNameSpace(enum_def);
    std::string enum_def_name = enum_def.name + (reverse ? "Name" : "");
    code += "export enum " + enum_def.name + "{\n";
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      if (!ev.doc_comment.empty()) {
        if (it != enum_def.Vals().begin()) { code += '\n'; }
        GenDocComment(ev.doc_comment, code_ptr, "  ");
      }

      // Generate mapping between EnumName: EnumValue(int)
      if (reverse) {
        code += "  '" + enum_def.ToString(ev) + "'";
        code += " = ";
        code += "'" + ev.name + "'";
      } else {
        code += "  " + ev.name;
        code += " = ";
        code += enum_def.ToString(ev);
      }

      code += (it + 1) != enum_def.Vals().end() ? ",\n" : "\n";
    }
    code += "}";

    if (enum_def.is_union) {
      code += GenUnionConvFunc(enum_def.underlying_type, imports);
    }

    code += "\n\n";
  }

  static std::string GenType(const Type &type) {
    switch (type.base_type) {
      case BASE_TYPE_BOOL:
      case BASE_TYPE_CHAR: return "Int8";
      case BASE_TYPE_UTYPE:
      case BASE_TYPE_UCHAR: return "Uint8";
      case BASE_TYPE_SHORT: return "Int16";
      case BASE_TYPE_USHORT: return "Uint16";
      case BASE_TYPE_INT: return "Int32";
      case BASE_TYPE_UINT: return "Uint32";
      case BASE_TYPE_LONG: return "Int64";
      case BASE_TYPE_ULONG: return "Uint64";
      case BASE_TYPE_FLOAT: return "Float32";
      case BASE_TYPE_DOUBLE: return "Float64";
      case BASE_TYPE_STRING: return "String";
      case BASE_TYPE_VECTOR: return GenType(type.VectorType());
      case BASE_TYPE_STRUCT: return type.struct_def->name;
      default: return "flatbuffers.Table";
    }
  }

  std::string GenGetter(const Type &type, const std::string &arguments) {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return GenBBAccess() + ".__string" + arguments;
      case BASE_TYPE_STRUCT: return GenBBAccess() + ".__struct" + arguments;
      case BASE_TYPE_UNION:
        if (!UnionHasStringType(*type.enum_def)) {
          return GenBBAccess() + ".__union" + arguments;
        }
        return GenBBAccess() + ".__union_with_string" + arguments;
      case BASE_TYPE_VECTOR: return GenGetter(type.VectorType(), arguments);
      default: {
        auto getter =
            GenBBAccess() + ".read" + MakeCamel(GenType(type)) + arguments;
        if (type.base_type == BASE_TYPE_BOOL) { getter = "!!" + getter; }
        return getter;
      }
    }
  }

  std::string GenBBAccess() const { return "this.bb!"; }

  std::string GenDefaultValue(const FieldDef &field, const std::string &context,
                              import_set &imports) {
    if (field.IsScalarOptional()) { return "null"; }

    const auto &value = field.value;
    if (value.type.enum_def && value.type.base_type != BASE_TYPE_UNION &&
        value.type.base_type != BASE_TYPE_VECTOR) {
      if (auto val = value.type.enum_def->FindByValue(value.constant)) {
        return AddImport(imports, *value.type.enum_def, *value.type.enum_def) +
               "." + val->name;
      } else {
        return value.constant;
      }
    }

    switch (value.type.base_type) {
      case BASE_TYPE_BOOL: return value.constant == "0" ? "false" : "true";

      case BASE_TYPE_STRING:
      case BASE_TYPE_UNION:
      case BASE_TYPE_STRUCT: {
        return "null";
      }

      case BASE_TYPE_VECTOR: return "[]";

      case BASE_TYPE_LONG:
      case BASE_TYPE_ULONG: {
        int64_t constant = StringToInt(value.constant.c_str());
        std::string createLong = context + ".createLong";
        return createLong + "(" + NumToString(static_cast<int32_t>(constant)) +
               ", " + NumToString(static_cast<int32_t>(constant >> 32)) + ")";
      }

      default: return value.constant;
    }
  }

  std::string GenTypeName(import_set &imports, const Definition &owner,
                          const Type &type, bool input,
                          bool allowNull = false) {
    if (!input) {
      if (IsString(type) || type.base_type == BASE_TYPE_STRUCT) {
        std::string name;
        if (IsString(type)) {
          name = "string|Uint8Array";
        } else {
          name = AddImport(imports, owner, *type.struct_def);
        }
        return allowNull ? (name + "|null") : name;
      }
    }

    switch (type.base_type) {
      case BASE_TYPE_BOOL: return allowNull ? "boolean|null" : "boolean";
      case BASE_TYPE_LONG:
      case BASE_TYPE_ULONG:
        return allowNull ? "flatbuffers.Long|null" : "flatbuffers.Long";
      default:
        if (IsScalar(type.base_type)) {
          if (type.enum_def) {
            const auto enum_name = AddImport(imports, owner, *type.enum_def);
            return allowNull ? (enum_name + "|null") : enum_name;
          }
          return allowNull ? "number|null" : "number";
        }
        return "flatbuffers.Offset";
    }
  }

  // Returns the method name for use with add/put calls.
  static std::string GenWriteMethod(const Type &type) {
    // Forward to signed versions since unsigned versions don't exist
    switch (type.base_type) {
      case BASE_TYPE_UTYPE:
      case BASE_TYPE_UCHAR: return GenWriteMethod(Type(BASE_TYPE_CHAR));
      case BASE_TYPE_USHORT: return GenWriteMethod(Type(BASE_TYPE_SHORT));
      case BASE_TYPE_UINT: return GenWriteMethod(Type(BASE_TYPE_INT));
      case BASE_TYPE_ULONG: return GenWriteMethod(Type(BASE_TYPE_LONG));
      default: break;
    }

    return IsScalar(type.base_type) ? MakeCamel(GenType(type))
                                    : (IsStruct(type) ? "Struct" : "Offset");
  }

  template<typename T> static std::string MaybeAdd(T value) {
    return value != 0 ? " + " + NumToString(value) : "";
  }

  template<typename T> static std::string MaybeScale(T value) {
    return value != 1 ? " * " + NumToString(value) : "";
  }

  void GenStructArgs(import_set &imports, const StructDef &struct_def,
                     std::string *arguments, const std::string &nameprefix) {
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (IsStruct(field.value.type)) {
        // Generate arguments for a struct inside a struct. To ensure names
        // don't clash, and to make it obvious these arguments are constructing
        // a nested struct, prefix the name with the field name.
        GenStructArgs(imports, *field.value.type.struct_def, arguments,
                      nameprefix + field.name + "_");
      } else {
        *arguments += ", " + nameprefix + field.name + ": " +
                      GenTypeName(imports, field, field.value.type, true,
                                  field.IsOptional());
      }
    }
  }

  static void GenStructBody(const StructDef &struct_def, std::string *body,
                            const std::string &nameprefix) {
    *body += "  builder.prep(";
    *body += NumToString(struct_def.minalign) + ", ";
    *body += NumToString(struct_def.bytesize) + ");\n";

    for (auto it = struct_def.fields.vec.rbegin();
         it != struct_def.fields.vec.rend(); ++it) {
      auto &field = **it;
      if (field.padding) {
        *body += "  builder.pad(" + NumToString(field.padding) + ");\n";
      }
      if (IsStruct(field.value.type)) {
        // Generate arguments for a struct inside a struct. To ensure names
        // don't clash, and to make it obvious these arguments are constructing
        // a nested struct, prefix the name with the field name.
        GenStructBody(*field.value.type.struct_def, body,
                      nameprefix + field.name + "_");
      } else {
        *body += "  builder.write" + GenWriteMethod(field.value.type) + "(";
        if (field.value.type.base_type == BASE_TYPE_BOOL) { *body += "+"; }
        *body += nameprefix + field.name + ");\n";
      }
    }
  }

  std::string GenerateNewExpression(const std::string &object_name) {
    return "new " + object_name + "()";
  }

  void GenerateRootAccessor(StructDef &struct_def, std::string *code_ptr,
                            std::string &code, const std::string &object_name,
                            bool size_prefixed) {
    if (!struct_def.fixed) {
      GenDocComment(code_ptr);
      std::string sizePrefixed("SizePrefixed");
      code += "static get" + (size_prefixed ? sizePrefixed : "") + "Root" +
              GetPrefixedName(struct_def, "As");
      code += "(bb:flatbuffers.ByteBuffer, obj?:" + object_name +
              "):" + object_name + " {\n";
      if (size_prefixed) {
        code +=
            "  bb.setPosition(bb.position() + "
            "flatbuffers.SIZE_PREFIX_LENGTH);\n";
      }
      code += "  return (obj || " + GenerateNewExpression(object_name);
      code += ").__init(bb.readInt32(bb.position()) + bb.position(), bb);\n";
      code += "}\n\n";
    }
  }

  void GenerateFinisher(StructDef &struct_def, std::string *code_ptr,
                        std::string &code, bool size_prefixed) {
    if (parser_.root_struct_def_ == &struct_def) {
      std::string sizePrefixed("SizePrefixed");
      GenDocComment(code_ptr);

      code += "static finish" + (size_prefixed ? sizePrefixed : "") +
              GetPrefixedName(struct_def) + "Buffer";
      code += "(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {\n";
      code += "  builder.finish(offset";
      if (!parser_.file_identifier_.empty()) {
        code += ", '" + parser_.file_identifier_ + "'";
      }
      if (size_prefixed) {
        if (parser_.file_identifier_.empty()) { code += ", undefined"; }
        code += ", true";
      }
      code += ");\n";
      code += "}\n\n";
    }
  }

  static std::string GetObjApiClassName(const StructDef &sd,
                                        const IDLOptions &opts) {
    return GetObjApiClassName(sd.name, opts);
  }

  static std::string GetObjApiClassName(const std::string &name,
                                        const IDLOptions &opts) {
    return opts.object_prefix + name + opts.object_suffix;
  }

  bool UnionHasStringType(const EnumDef &union_enum) {
    return std::any_of(union_enum.Vals().begin(), union_enum.Vals().end(),
                       [](const EnumVal *ev) {
                         return !ev->IsZero() && IsString(ev->union_type);
                       });
  }

  std::string GenUnionGenericTypeTS(const EnumDef &union_enum) {
    // TODO: make it work without any
    // return std::string("T") + (UnionHasStringType(union_enum) ? "|string" :
    // "");
    return std::string("any") +
           (UnionHasStringType(union_enum) ? "|string" : "");
  }

  std::string GenUnionTypeTS(const EnumDef &union_enum, import_set &imports) {
    std::string ret;
    std::set<std::string> type_list;

    for (auto it = union_enum.Vals().begin(); it != union_enum.Vals().end();
         ++it) {
      const auto &ev = **it;
      if (ev.IsZero()) { continue; }

      std::string type = "";
      if (IsString(ev.union_type)) {
        type = "string";  // no need to wrap string type in namespace
      } else if (ev.union_type.base_type == BASE_TYPE_STRUCT) {
        type = AddImport(imports, union_enum, *ev.union_type.struct_def);
      } else {
        FLATBUFFERS_ASSERT(false);
      }
      type_list.insert(type);
    }

    for (auto it = type_list.begin(); it != type_list.end(); ++it) {
      ret += *it + ((std::next(it) == type_list.end()) ? "" : "|");
    }

    return ret;
  }

  std::string AddImport(import_set &imports, const Definition &dependent,
                        const StructDef &dependency) {
    std::string ns;
    const auto &depc_comps = dependency.defined_namespace->components;
    for (auto it = depc_comps.begin(); it != depc_comps.end(); it++) ns += *it;
    std::string unique_name = ns + dependency.name;
    std::string import_name = dependency.name;
    std::string long_import_name;
    if (imports.find(unique_name) != imports.end())
      return imports.find(unique_name)->second.name;
    for (auto it = imports.begin(); it != imports.end(); it++) {
      if (it->second.name == import_name) {
        long_import_name = ns + import_name;
        break;
      }
    }
    std::string import_statement;
    std::string export_statement;
    import_statement += "import { ";
    export_statement += "export { ";
    std::string symbols_expression;
    if (long_import_name.empty()) {
      symbols_expression += import_name;
      if (parser_.opts.generate_object_based_api)
        symbols_expression += ", " + import_name + "T";
    } else {
      symbols_expression += dependency.name + " as " + long_import_name;
      if (parser_.opts.generate_object_based_api)
        symbols_expression +=
            ", " + dependency.name + "T as " + long_import_name + "T";
    }
    import_statement += symbols_expression + " } from '";
    export_statement += symbols_expression + " } from '";
    std::string bare_file_path;
    std::string rel_file_path;
    const auto &dep_comps = dependent.defined_namespace->components;
    for (size_t i = 0; i < dep_comps.size(); i++)
      rel_file_path += i == 0 ? ".." : (kPathSeparator + std::string(".."));
    if (dep_comps.size() == 0) rel_file_path += ".";
    for (auto it = depc_comps.begin(); it != depc_comps.end(); it++)
      bare_file_path += kPathSeparator + ToDasherizedCase(*it);
    bare_file_path += kPathSeparator + ToDasherizedCase(dependency.name);
    rel_file_path += bare_file_path;
    import_statement += rel_file_path + "';";
    export_statement += "." + bare_file_path + "';";
    ImportDefinition import;
    import.name = long_import_name.empty() ? import_name : long_import_name;
    import.bare_file_path = bare_file_path;
    import.rel_file_path = rel_file_path;
    import.import_statement = import_statement;
    import.export_statement = export_statement;
    import.dependency = &dependency;
    import.dependent = &dependent;
    imports.insert(std::make_pair(unique_name, import));
    return import.name;
  }

  // TODO: largely (but not identical) duplicated code from above couln't find a
  // good way to refactor
  std::string AddImport(import_set &imports, const Definition &dependent,
                        const EnumDef &dependency) {
    std::string ns;
    const auto &depc_comps = dependency.defined_namespace->components;
    for (auto it = depc_comps.begin(); it != depc_comps.end(); it++) ns += *it;
    std::string unique_name = ns + dependency.name;
    std::string import_name = dependency.name;
    std::string long_import_name;
    if (imports.find(unique_name) != imports.end())
      return imports.find(unique_name)->second.name;
    for (auto it = imports.begin(); it != imports.end(); it++) {
      if (it->second.name == import_name) {
        long_import_name = ns + import_name;
        break;
      }
    }
    std::string import_statement;
    std::string export_statement;
    import_statement += "import { ";
    export_statement += "export { ";
    std::string symbols_expression;
    if (long_import_name.empty())
      symbols_expression += import_name;
    else
      symbols_expression += dependency.name + " as " + long_import_name;
    if (dependency.is_union) {
      symbols_expression += ", unionTo" + import_name;
      symbols_expression += ", unionListTo" + import_name;
    }
    import_statement += symbols_expression + " } from '";
    export_statement += symbols_expression + " } from '";
    std::string bare_file_path;
    std::string rel_file_path;
    const auto &dep_comps = dependent.defined_namespace->components;
    for (size_t i = 0; i < dep_comps.size(); i++)
      rel_file_path += i == 0 ? ".." : (kPathSeparator + std::string(".."));
    if (dep_comps.size() == 0) rel_file_path += ".";
    for (auto it = depc_comps.begin(); it != depc_comps.end(); it++)
      bare_file_path += kPathSeparator + ToDasherizedCase(*it);
    bare_file_path += kPathSeparator + ToDasherizedCase(dependency.name);
    rel_file_path += bare_file_path;
    import_statement += rel_file_path + "';";
    export_statement += "." + bare_file_path + "';";
    ImportDefinition import;
    import.name = long_import_name.empty() ? import_name : long_import_name;
    import.bare_file_path = bare_file_path;
    import.rel_file_path = rel_file_path;
    import.import_statement = import_statement;
    import.export_statement = export_statement;
    import.dependency = &dependency;
    import.dependent = &dependent;
    imports.insert(std::make_pair(unique_name, import));
    return import.name;
  }

  void AddImport(import_set &imports, std::string import_name,
                 std::string fileName) {
    ImportDefinition import;
    import.name = import_name;
    import.import_statement = "import " + import_name + " from '" + fileName + "';";
    imports.insert(std::make_pair(import_name, import));
  }

  // Generate a TS union type based on a union's enum
  std::string GenObjApiUnionTypeTS(import_set &imports, const IDLOptions &opts,
                                   const EnumDef &union_enum) {
    std::string ret = "";
    std::set<std::string> type_list;

    for (auto it = union_enum.Vals().begin(); it != union_enum.Vals().end();
         ++it) {
      const auto &ev = **it;
      if (ev.IsZero()) { continue; }

      std::string type = "";
      if (IsString(ev.union_type)) {
        type = "string";  // no need to wrap string type in namespace
      } else if (ev.union_type.base_type == BASE_TYPE_STRUCT) {
        type = GetObjApiClassName(
            AddImport(imports, union_enum, *ev.union_type.struct_def), opts);
      } else {
        FLATBUFFERS_ASSERT(false);
      }
      type_list.insert(type);
    }

    size_t totalPrinted = 0;
    for (auto it = type_list.begin(); it != type_list.end(); ++it) {
      ++totalPrinted;
      ret += *it + ((totalPrinted == type_list.size()) ? "" : "|");
    }

    return ret;
  }

  std::string GenUnionConvFuncName(const EnumDef &enum_def) {
    return "unionTo" + enum_def.name;
  }

  std::string GenUnionListConvFuncName(const EnumDef &enum_def) {
    return "unionListTo" + enum_def.name;
  }

  std::string GenUnionConvFunc(const Type &union_type, import_set &imports) {
    if (union_type.enum_def) {
      const auto &enum_def = *union_type.enum_def;

      const auto valid_union_type = GenUnionTypeTS(enum_def, imports);
      const auto valid_union_type_with_null = valid_union_type + "|null";

      auto ret = "\n\nexport function " + GenUnionConvFuncName(enum_def) +
                 "(\n  type: " + enum_def.name +
                 ",\n  accessor: (obj:" + valid_union_type + ") => " +
                 valid_union_type_with_null +
                 "\n): " + valid_union_type_with_null + " {\n";

      const auto enum_type = AddImport(imports, enum_def, enum_def);

      const auto union_enum_loop = [&](const std::string &accessor_str) {
        ret += "  switch(" + enum_type + "[type]) {\n";
        ret += "    case 'NONE': return null; \n";

        for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end();
             ++it) {
          const auto &ev = **it;
          if (ev.IsZero()) { continue; }

          ret += "    case '" + ev.name + "': ";

          if (IsString(ev.union_type)) {
            ret += "return " + accessor_str + "'') as string;";
          } else if (ev.union_type.base_type == BASE_TYPE_STRUCT) {
            const auto type =
                AddImport(imports, enum_def, *ev.union_type.struct_def);
            ret += "return " + accessor_str + "new " + type + "())! as " +
                   type + ";";
          } else {
            FLATBUFFERS_ASSERT(false);
          }
          ret += "\n";
        }

        ret += "    default: return null;\n";
        ret += "  }\n";
      };

      union_enum_loop("accessor(");
      ret += "}";

      ret += "\n\nexport function " + GenUnionListConvFuncName(enum_def) +
             "(\n  type: " + enum_def.name +
             ", \n  accessor: (index: number, obj:" + valid_union_type +
             ") => " + valid_union_type_with_null +
             ", \n  index: number\n): " + valid_union_type_with_null + " {\n";
      union_enum_loop("accessor(index, ");
      ret += "}";

      return ret;
    }
    FLATBUFFERS_ASSERT(0);
    return "";
  }

  // Used for generating a short function that returns the correct class
  // based on union enum type. Assume the context is inside the non object api
  // type
  std::string GenUnionValTS(import_set &imports, const std::string &field_name,
                            const Type &union_type,
                            const bool is_array = false) {
    if (union_type.enum_def) {
      const auto &enum_def = *union_type.enum_def;
      const auto enum_type = AddImport(imports, enum_def, enum_def);
      const std::string union_accessor = "this." + field_name;

      const auto union_has_string = UnionHasStringType(enum_def);
      const auto field_binded_method = "this." + field_name + ".bind(this)";

      std::string ret;

      if (!is_array) {
        const auto conversion_function = GenUnionConvFuncName(enum_def);
        const auto target_enum = "this." + field_name + "Type()";

        ret = "(() => {\n";
        ret += "      let temp = " + conversion_function + "(" + target_enum +
               ", " + field_binded_method + ");\n";
        ret += "      if(temp === null) { return null; }\n";
        ret += union_has_string
                   ? "      if(typeof temp === 'string') { return temp; }\n"
                   : "";
        ret += "      return temp.unpack()\n";
        ret += "  })()";
      } else {
        const auto conversion_function = GenUnionListConvFuncName(enum_def);
        const auto target_enum_accesor = "this." + field_name + "Type";
        const auto target_enum_length = target_enum_accesor + "Length()";

        ret = "(() => {\n";
        ret += "    let ret = [];\n";
        ret += "    for(let targetEnumIndex = 0; targetEnumIndex < " +
               target_enum_length +
               "; "
               "++targetEnumIndex) {\n";
        ret += "      let targetEnum = " + target_enum_accesor +
               "(targetEnumIndex);\n";
        ret += "      if(targetEnum === null || " + enum_type +
               "[targetEnum!] === 'NONE') { "
               "continue; }\n\n";
        ret += "      let temp = " + conversion_function + "(targetEnum, " +
               field_binded_method + ", targetEnumIndex);\n";
        ret += "      if(temp === null) { continue; }\n";
        ret += union_has_string ? "      if(typeof temp === 'string') { "
                                  "ret.push(temp); continue; }\n"
                                : "";
        ret += "      ret.push(temp.unpack());\n";
        ret += "    }\n";
        ret += "    return ret;\n";
        ret += "  })()";
      }

      return ret;
    }

    FLATBUFFERS_ASSERT(0);
    return "";
  }

  static std::string GenNullCheckConditional(
      const std::string &nullCheckVar, const std::string &trueVal,
      const std::string &falseVal = "null") {
    return "(" + nullCheckVar + " !== null ? " + trueVal + " : " + falseVal +
           ")";
  }

  std::string GenStructMemberValueTS(const StructDef &struct_def,
                                     const std::string &prefix,
                                     const std::string &delimiter,
                                     const bool nullCheck = true) {
    std::string ret;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;

      const auto curr_member_accessor =
          prefix + "." + MakeCamel(field.name, false);
      if (IsStruct(field.value.type)) {
        ret += GenStructMemberValueTS(*field.value.type.struct_def,
                                      curr_member_accessor, delimiter);
      } else {
        if (nullCheck) {
          ret +=
              "(" + prefix + " === null ? 0 : " + curr_member_accessor + "!)";
        } else {
          ret += curr_member_accessor;
        }
      }

      if (std::next(it) != struct_def.fields.vec.end()) { ret += delimiter; }
    }

    return ret;
  }

  void GenObjApi(const Parser &parser, StructDef &struct_def,
                 std::string &obj_api_unpack_func, std::string &obj_api_class,
                 import_set &imports) {
    const auto class_name = GetObjApiClassName(struct_def, parser.opts);

    std::string unpack_func = "\nunpack(): " + class_name +
                              " {\n  return new " + class_name + "(" +
                              (struct_def.fields.vec.empty() ? "" : "\n");
    std::string unpack_to_func = "\nunpackTo(_o: " + class_name + "): void {" +
                                 +(struct_def.fields.vec.empty() ? "" : "\n");

    std::string constructor_func = "constructor(";
    constructor_func += (struct_def.fields.vec.empty() ? "" : "\n");

    const auto has_create =
        struct_def.fixed || CanCreateFactoryMethod(struct_def);

    std::string pack_func_prototype =
        "\npack(builder:flatbuffers.Builder): flatbuffers.Offset {\n";

    std::string pack_func_offset_decl;
    std::string pack_func_create_call;

    const auto struct_name = AddImport(imports, struct_def, struct_def);

    if (has_create) {
      pack_func_create_call = "  return " + struct_name + ".create" +
                              GetPrefixedName(struct_def) + "(builder" +
                              (struct_def.fields.vec.empty() ? "" : ",\n    ");
    } else {
      pack_func_create_call = "  " + struct_name + ".start" +
                              GetPrefixedName(struct_def) + "(builder);\n";
    }

    if (struct_def.fixed) {
      // when packing struct, nested struct's members instead of the struct's
      // offset are used
      pack_func_create_call +=
          GenStructMemberValueTS(struct_def, "this", ",\n    ", false) + "\n  ";
    }

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      const auto field_name = MakeCamel(field.name, false);
      const std::string field_binded_method =
          "this." + field_name + ".bind(this)";

      std::string field_val;
      std::string field_type;
      // a string that declares a variable containing the
      // offset for things that can't be generated inline
      // empty otw
      std::string field_offset_decl;
      // a string that contains values for things that can be created inline or
      // the variable name from field_offset_decl
      std::string field_offset_val;
      const auto field_default_val =
          GenDefaultValue(field, "flatbuffers", imports);

      // Emit a scalar field
      const auto is_string = IsString(field.value.type);
      if (IsScalar(field.value.type.base_type) || is_string) {
        const auto has_null_default = is_string || HasNullDefault(field);

        field_type += GenTypeName(imports, field, field.value.type, false,
                                  has_null_default);
        field_val = "this." + field_name + "()";

        if (field.value.type.base_type != BASE_TYPE_STRING) {
          field_offset_val = "this." + field_name;
        } else {
          field_offset_decl = GenNullCheckConditional(
              "this." + field_name,
              "builder.createString(this." + field_name + "!)", "0");
        }
      }

      // Emit an object field
      else {
        auto is_vector = false;
        switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT: {
            const auto &sd = *field.value.type.struct_def;
            field_type += GetObjApiClassName(sd, parser.opts);

            const std::string field_accessor = "this." + field_name + "()";
            field_val = GenNullCheckConditional(field_accessor,
                                                field_accessor + "!.unpack()");
            auto packing = GenNullCheckConditional(
                "this." + field_name, "this." + field_name + "!.pack(builder)",
                "0");

            if (sd.fixed) {
              field_offset_val = std::move(packing);
            } else {
              field_offset_decl = std::move(packing);
            }

            break;
          }

          case BASE_TYPE_VECTOR: {
            auto vectortype = field.value.type.VectorType();
            auto vectortypename =
                GenTypeName(imports, struct_def, vectortype, false);
            is_vector = true;

            field_type = "(";

            switch (vectortype.base_type) {
              case BASE_TYPE_STRUCT: {
                const auto &sd = *field.value.type.struct_def;
                field_type += GetObjApiClassName(sd, parser.opts);
                field_type += ")[]";

                field_val = GenBBAccess() + ".createObjList(" +
                            field_binded_method + ", this." + field_name +
                            "Length())";

                if (sd.fixed) {
                  field_offset_decl =
                      "builder.createStructOffsetList(this." + field_name +
                      ", " + AddImport(imports, struct_def, struct_def) +
                      ".start" + MakeCamel(field_name) + "Vector)";
                } else {
                  field_offset_decl =
                      AddImport(imports, struct_def, struct_def) + ".create" +
                      MakeCamel(field_name) +
                      "Vector(builder, builder.createObjectOffsetList(" +
                      "this." + field_name + "))";
                }

                break;
              }

              case BASE_TYPE_STRING: {
                field_type += "string)[]";
                field_val = GenBBAccess() + ".createScalarList(" +
                            field_binded_method + ", this." + field_name +
                            "Length())";
                field_offset_decl =
                    AddImport(imports, struct_def, struct_def) + ".create" +
                    MakeCamel(field_name) +
                    "Vector(builder, builder.createObjectOffsetList(" +
                    "this." + field_name + "))";
                break;
              }

              case BASE_TYPE_UNION: {
                field_type += GenObjApiUnionTypeTS(imports, parser.opts,
                                                   *(vectortype.enum_def));
                field_type += ")[]";
                field_val =
                    GenUnionValTS(imports, field_name, vectortype, true);

                field_offset_decl =
                    AddImport(imports, struct_def, struct_def) + ".create" +
                    MakeCamel(field_name) +
                    "Vector(builder, builder.createObjectOffsetList(" +
                    "this." + field_name + "))";

                break;
              }
              default: {
                if (vectortype.enum_def) {
                  field_type += GenTypeName(imports, struct_def, vectortype,
                                            false, HasNullDefault(field));
                } else {
                  field_type += vectortypename;
                }
                field_type += ")[]";
                field_val = GenBBAccess() + ".createScalarList(" +
                            field_binded_method + ", this." + field_name +
                            "Length())";

                field_offset_decl = AddImport(imports, struct_def, struct_def) +
                                    ".create" + MakeCamel(field_name) +
                                    "Vector(builder, this." + field_name + ")";

                break;
              }
            }

            break;
          }

          case BASE_TYPE_UNION: {
            field_type += GenObjApiUnionTypeTS(imports, parser.opts,
                                               *(field.value.type.enum_def));

            field_val = GenUnionValTS(imports, field_name, field.value.type);
            field_offset_decl =
                "builder.createObjectOffset(this." + field_name + ")";
            break;
          }

          default: FLATBUFFERS_ASSERT(0); break;
        }

        // length 0 vector is simply empty instead of null
        field_type += is_vector ? "" : "|null";
      }

      if (!field_offset_decl.empty()) {
        field_offset_decl =
            "  const " + field_name + " = " + field_offset_decl + ";";
      }
      if (field_offset_val.empty()) { field_offset_val = field_name; }

      unpack_func += "    " + field_val;
      unpack_to_func += "  _o." + field_name + " = " + field_val + ";";

      constructor_func += "  public " + field_name + ": " + field_type + " = " +
                          field_default_val;

      if (!struct_def.fixed) {
        if (!field_offset_decl.empty()) {
          pack_func_offset_decl += field_offset_decl + "\n";
        }

        if (has_create) {
          pack_func_create_call += field_offset_val;
        } else {
          pack_func_create_call += "  " + struct_name + ".add" +
                                   MakeCamel(field.name) + "(builder, " +
                                   field_offset_val + ");\n";
        }
      }

      if (std::next(it) != struct_def.fields.vec.end()) {
        constructor_func += ",\n";

        if (!struct_def.fixed && has_create) {
          pack_func_create_call += ",\n    ";
        }

        unpack_func += ",\n";
        unpack_to_func += "\n";
      } else {
        constructor_func += "\n";
        if (!struct_def.fixed) {
          pack_func_offset_decl += (pack_func_offset_decl.empty() ? "" : "\n");
          pack_func_create_call += "\n  ";
        }

        unpack_func += "\n  ";
        unpack_to_func += "\n";
      }
    }

    constructor_func += "){}\n\n";

    if (has_create) {
      pack_func_create_call += ");";
    } else {
      pack_func_create_call += "return " + struct_name + ".end" +
                               GetPrefixedName(struct_def) + "(builder);";
    }

    obj_api_class = "\nexport class " +
                    GetObjApiClassName(struct_def, parser.opts) + " {\n";

    obj_api_class += constructor_func;
    obj_api_class += pack_func_prototype + pack_func_offset_decl +
                     pack_func_create_call + "\n}";

    obj_api_class += "\n}\n";

    unpack_func += ");\n}";
    unpack_to_func += "}\n";

    obj_api_unpack_func = unpack_func + "\n\n" + unpack_to_func;
  }

  static bool CanCreateFactoryMethod(const StructDef &struct_def) {
    // to preserve backwards compatibility, we allow the first field to be a
    // struct
    return struct_def.fields.vec.size() < 2 ||
           std::all_of(std::begin(struct_def.fields.vec) + 1,
                       std::end(struct_def.fields.vec),
                       [](const FieldDef *f) -> bool {
                         FLATBUFFERS_ASSERT(f != nullptr);
                         return f->value.type.base_type != BASE_TYPE_STRUCT;
                       });
  }

  // Generate an accessor struct with constructor for a flatbuffers struct.
  void GenStruct(const Parser &parser, StructDef &struct_def,
                 std::string *code_ptr, import_set &imports) {
    if (struct_def.generated) return;
    std::string &code = *code_ptr;

    std::string object_name;
    std::string object_namespace = GetNameSpace(struct_def);

    // Emit constructor
    object_name = struct_def.name;
    GenDocComment(struct_def.doc_comment, code_ptr);
    code += "export class " + struct_def.name;
    code += " {\n";
    code += "  bb: flatbuffers.ByteBuffer|null = null;\n";
    code += "  bb_pos = 0;\n";

    // Generate the __init method that sets the field in a pre-existing
    // accessor object. This is to allow object reuse.
    code +=
        "__init(i:number, bb:flatbuffers.ByteBuffer):" + object_name + " {\n";
    code += "  this.bb_pos = i;\n";
    code += "  this.bb = bb;\n";
    code += "  return this;\n";
    code += "}\n\n";

    // Generate special accessors for the table that when used as the root of a
    // FlatBuffer
    GenerateRootAccessor(struct_def, code_ptr, code, object_name, false);
    GenerateRootAccessor(struct_def, code_ptr, code, object_name, true);

    // Generate the identifier check method
    if (!struct_def.fixed && parser_.root_struct_def_ == &struct_def &&
        !parser_.file_identifier_.empty()) {
      GenDocComment(code_ptr);
      code +=
          "static bufferHasIdentifier(bb:flatbuffers.ByteBuffer):boolean "
          "{\n";
      code += "  return bb.__has_identifier('" + parser_.file_identifier_;
      code += "');\n}\n\n";
    }

    // Emit field accessors
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      auto offset_prefix =
          "  const offset = " + GenBBAccess() + ".__offset(this.bb_pos, " +
          NumToString(field.value.offset) + ");\n  return offset ? ";

      // Emit a scalar field
      const auto is_string = IsString(field.value.type);
      if (IsScalar(field.value.type.base_type) || is_string) {
        const auto has_null_default = is_string || HasNullDefault(field);

        GenDocComment(field.doc_comment, code_ptr);
        std::string prefix = MakeCamel(field.name, false) + "(";
        if (is_string) {
          code += prefix + "):string|null\n";
          code +=
              prefix + "optionalEncoding:flatbuffers.Encoding" + "):" +
              GenTypeName(imports, struct_def, field.value.type, false, true) +
              "\n";
          code += prefix + "optionalEncoding?:any";
        } else {
          code += prefix;
        }
        if (field.value.type.enum_def) {
          code += "):" +
                  GenTypeName(imports, struct_def, field.value.type, false,
                              field.IsOptional()) +
                  " {\n";
        } else {
          code += "):" +
                  GenTypeName(imports, struct_def, field.value.type, false,
                              has_null_default) +
                  " {\n";
        }

        if (struct_def.fixed) {
          code +=
              "  return " +
              GenGetter(field.value.type,
                        "(this.bb_pos" + MaybeAdd(field.value.offset) + ")") +
              ";\n";
        } else {
          std::string index = "this.bb_pos + offset";
          if (is_string) { index += ", optionalEncoding"; }
          code += offset_prefix +
                  GenGetter(field.value.type, "(" + index + ")") + " : " +
                  GenDefaultValue(field, GenBBAccess(), imports);
          code += ";\n";
        }
      }

      // Emit an object field
      else {
        switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT: {
            const auto type =
                AddImport(imports, struct_def, *field.value.type.struct_def);
            GenDocComment(field.doc_comment, code_ptr);
            code += MakeCamel(field.name, false);
            code += "(obj?:" + type + "):" + type + "|null {\n";

            if (struct_def.fixed) {
              code += "  return (obj || " + GenerateNewExpression(type);
              code += ").__init(this.bb_pos";
              code +=
                  MaybeAdd(field.value.offset) + ", " + GenBBAccess() + ");\n";
            } else {
              code += offset_prefix + "(obj || " + GenerateNewExpression(type) +
                      ").__init(";
              code += field.value.type.struct_def->fixed
                          ? "this.bb_pos + offset"
                          : GenBBAccess() + ".__indirect(this.bb_pos + offset)";
              code += ", " + GenBBAccess() + ") : null;\n";
            }

            break;
          }

          case BASE_TYPE_VECTOR: {
            auto vectortype = field.value.type.VectorType();
            auto vectortypename =
                GenTypeName(imports, struct_def, vectortype, false);
            auto inline_size = InlineSize(vectortype);
            auto index = GenBBAccess() +
                         ".__vector(this.bb_pos + offset) + index" +
                         MaybeScale(inline_size);
            std::string ret_type;
            bool is_union = false;
            switch (vectortype.base_type) {
              case BASE_TYPE_STRUCT: ret_type = vectortypename; break;
              case BASE_TYPE_STRING: ret_type = vectortypename; break;
              case BASE_TYPE_UNION:
                ret_type = "?flatbuffers.Table";
                is_union = true;
                break;
              default: ret_type = vectortypename;
            }
            GenDocComment(field.doc_comment, code_ptr);
            std::string prefix = MakeCamel(field.name, false);
            // TODO: make it work without any
            // if (is_union) { prefix += "<T extends flatbuffers.Table>"; }
            if (is_union) { prefix += ""; }
            prefix += "(index: number";
            if (is_union) {
              const auto union_type =
                  GenUnionGenericTypeTS(*(field.value.type.enum_def));

              vectortypename = union_type;
              code += prefix + ", obj:" + union_type;
            } else if (vectortype.base_type == BASE_TYPE_STRUCT) {
              code += prefix + ", obj?:" + vectortypename;
            } else if (IsString(vectortype)) {
              code += prefix + "):string\n";
              code += prefix + ",optionalEncoding:flatbuffers.Encoding" +
                      "):" + vectortypename + "\n";
              code += prefix + ",optionalEncoding?:any";
            } else {
              code += prefix;
            }
            code += "):" + vectortypename + "|null {\n";

            if (vectortype.base_type == BASE_TYPE_STRUCT) {
              code += offset_prefix + "(obj || " +
                      GenerateNewExpression(vectortypename);
              code += ").__init(";
              code += vectortype.struct_def->fixed
                          ? index
                          : GenBBAccess() + ".__indirect(" + index + ")";
              code += ", " + GenBBAccess() + ")";
            } else {
              if (is_union) {
                index = "obj, " + index;
              } else if (IsString(vectortype)) {
                index += ", optionalEncoding";
              }
              code += offset_prefix + GenGetter(vectortype, "(" + index + ")");
            }
            code += " : ";
            if (field.value.type.element == BASE_TYPE_BOOL) {
              code += "false";
            } else if (field.value.type.element == BASE_TYPE_LONG ||
                       field.value.type.element == BASE_TYPE_ULONG) {
              code += GenBBAccess() + ".createLong(0, 0)";
            } else if (IsScalar(field.value.type.element)) {
              if (field.value.type.enum_def) {
                code += field.value.constant;
              } else {
                code += "0";
              }
            } else {
              code += "null";
            }
            code += ";\n";
            break;
          }

          case BASE_TYPE_UNION: {
            GenDocComment(field.doc_comment, code_ptr);
            code += MakeCamel(field.name, false);

            const auto &union_enum = *(field.value.type.enum_def);
            const auto union_type = GenUnionGenericTypeTS(union_enum);
            code += "<T extends flatbuffers.Table>(obj:" + union_type +
                    "):" + union_type +
                    "|null "
                    "{\n";

            code += offset_prefix +
                    GenGetter(field.value.type, "(obj, this.bb_pos + offset)") +
                    " : null;\n";
            break;
          }
          default: FLATBUFFERS_ASSERT(0);
        }
      }
      code += "}\n\n";

      // Adds the mutable scalar value to the output
      if (IsScalar(field.value.type.base_type) && parser.opts.mutable_buffer &&
          !IsUnion(field.value.type)) {
        std::string type =
            GenTypeName(imports, struct_def, field.value.type, true);

        code += "mutate_" + field.name + "(value:" + type + "):boolean {\n";

        if (struct_def.fixed) {
          code += "  " + GenBBAccess() + ".write" +
                  MakeCamel(GenType(field.value.type)) + "(this.bb_pos + " +
                  NumToString(field.value.offset) + ", ";
        } else {
          code += "  const offset = " + GenBBAccess() +
                  ".__offset(this.bb_pos, " + NumToString(field.value.offset) +
                  ");\n\n";
          code += "  if (offset === 0) {\n";
          code += "    return false;\n";
          code += "  }\n\n";

          // special case for bools, which are treated as uint8
          code += "  " + GenBBAccess() + ".write" +
                  MakeCamel(GenType(field.value.type)) +
                  "(this.bb_pos + offset, ";
          if (field.value.type.base_type == BASE_TYPE_BOOL) { code += "+"; }
        }

        code += "value);\n";
        code += "  return true;\n";
        code += "}\n\n";
      }

      // Emit vector helpers
      if (IsVector(field.value.type)) {
        // Emit a length helper
        GenDocComment(code_ptr);
        code += MakeCamel(field.name, false);
        code += "Length():number {\n" + offset_prefix;

        code +=
            GenBBAccess() + ".__vector_len(this.bb_pos + offset) : 0;\n}\n\n";

        // For scalar types, emit a typed array helper
        auto vectorType = field.value.type.VectorType();
        if (IsScalar(vectorType.base_type) && !IsLong(vectorType.base_type)) {
          GenDocComment(code_ptr);

          code += MakeCamel(field.name, false);
          code += "Array():" + GenType(vectorType) + "Array|null {\n" +
                  offset_prefix;

          code += "new " + GenType(vectorType) + "Array(" + GenBBAccess() +
                  ".bytes().buffer, " + GenBBAccess() +
                  ".bytes().byteOffset + " + GenBBAccess() +
                  ".__vector(this.bb_pos + offset), " + GenBBAccess() +
                  ".__vector_len(this.bb_pos + offset)) : null;\n}\n\n";
        }
      }
    }

    // Emit the fully qualified name
    if (parser_.opts.generate_name_strings) {
      GenDocComment(code_ptr);
      code += "static getFullyQualifiedName():string {\n";
      code += "  return '" + WrapInNameSpace(struct_def) + "';\n";
      code += "}\n\n";
    }

    // Emit the size of the struct.
    if (struct_def.fixed) {
      GenDocComment(code_ptr);
      code += "static sizeOf():number {\n";
      code += "  return " + NumToString(struct_def.bytesize) + ";\n";
      code += "}\n\n";
    }

    // Emit a factory constructor
    if (struct_def.fixed) {
      std::string arguments;
      GenStructArgs(imports, struct_def, &arguments, "");
      GenDocComment(code_ptr);

      code += "static create" + GetPrefixedName(struct_def) +
              "(builder:flatbuffers.Builder";
      code += arguments + "):flatbuffers.Offset {\n";

      GenStructBody(struct_def, &code, "");
      code += "  return builder.offset();\n}\n\n";
    } else {
      // Generate a method to start building a new object
      GenDocComment(code_ptr);

      code += "static start" + GetPrefixedName(struct_def) +
              "(builder:flatbuffers.Builder) {\n";

      code += "  builder.startObject(" +
              NumToString(struct_def.fields.vec.size()) + ");\n";
      code += "}\n\n";

      // Generate a set of static methods that allow table construction
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        const auto argname = GetArgName(field);

        // Generate the field insertion method
        GenDocComment(code_ptr);
        code += "static add" + MakeCamel(field.name);
        code += "(builder:flatbuffers.Builder, " + argname + ":" +
                GetArgType(imports, struct_def, field, false) + ") {\n";
        code += "  builder.addField" + GenWriteMethod(field.value.type) + "(";
        code += NumToString(it - struct_def.fields.vec.begin()) + ", ";
        if (field.value.type.base_type == BASE_TYPE_BOOL) { code += "+"; }
        code += argname + ", ";
        if (!IsScalar(field.value.type.base_type)) {
          code += "0";
        } else if (HasNullDefault(field)) {
          if (IsLong(field.value.type.base_type)) {
            code += "builder.createLong(0, 0)";
          } else {
            code += "0";
          }
        } else {
          if (field.value.type.base_type == BASE_TYPE_BOOL) { code += "+"; }
          code += GenDefaultValue(field, "builder", imports);
        }
        code += ");\n}\n\n";

        if (IsVector(field.value.type)) {
          auto vector_type = field.value.type.VectorType();
          auto alignment = InlineAlignment(vector_type);
          auto elem_size = InlineSize(vector_type);

          // Generate a method to create a vector from a JavaScript array
          if (!IsStruct(vector_type)) {
            GenDocComment(code_ptr);

            const std::string sig_begin =
                "static create" + MakeCamel(field.name) +
                "Vector(builder:flatbuffers.Builder, data:";
            const std::string sig_end = "):flatbuffers.Offset";
            std::string type =
                GenTypeName(imports, struct_def, vector_type, true) + "[]";
            if (type == "number[]") {
              const auto &array_type = GenType(vector_type);
              // the old type should be deprecated in the future
              std::string type_old = "number[]|Uint8Array";
              std::string type_new = "number[]|" + array_type + "Array";
              if (type_old == type_new) {
                type = type_new;
              } else {
                // add function overloads
                code += sig_begin + type_new + sig_end + ";\n";
                code +=
                    "/**\n * @deprecated This Uint8Array overload will "
                    "be removed in the future.\n */\n";
                code += sig_begin + type_old + sig_end + ";\n";
                type = type_new + "|Uint8Array";
              }
            }
            code += sig_begin + type + sig_end + " {\n";
            code += "  builder.startVector(" + NumToString(elem_size);
            code += ", data.length, " + NumToString(alignment) + ");\n";
            code += "  for (let i = data.length - 1; i >= 0; i--) {\n";
            code += "    builder.add" + GenWriteMethod(vector_type) + "(";
            if (vector_type.base_type == BASE_TYPE_BOOL) { code += "+"; }
            code += "data[i]!);\n";
            code += "  }\n";
            code += "  return builder.endVector();\n";
            code += "}\n\n";
          }

          // Generate a method to start a vector, data to be added manually
          // after
          GenDocComment(code_ptr);

          code += "static start" + MakeCamel(field.name);
          code += "Vector(builder:flatbuffers.Builder, numElems:number) {\n";
          code += "  builder.startVector(" + NumToString(elem_size);
          code += ", numElems, " + NumToString(alignment) + ");\n";
          code += "}\n\n";
        }
      }

      // Generate a method to stop building a new object
      GenDocComment(code_ptr);

      code += "static end" + GetPrefixedName(struct_def);
      code += "(builder:flatbuffers.Builder):flatbuffers.Offset {\n";

      code += "  const offset = builder.endObject();\n";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (!field.deprecated && field.IsRequired()) {
          code += "  builder.requiredField(offset, ";
          code += NumToString(field.value.offset);
          code += ") // " + field.name + "\n";
        }
      }
      code += "  return offset;\n";
      code += "}\n\n";

      // Generate the methods to complete buffer construction
      GenerateFinisher(struct_def, code_ptr, code, false);
      GenerateFinisher(struct_def, code_ptr, code, true);

      // Generate a convenient CreateX function
      if (CanCreateFactoryMethod(struct_def)) {
        code += "static create" + GetPrefixedName(struct_def);
        code += "(builder:flatbuffers.Builder";
        for (auto it = struct_def.fields.vec.begin();
             it != struct_def.fields.vec.end(); ++it) {
          const auto &field = **it;
          if (field.deprecated) continue;
          code += ", " + GetArgName(field) + ":" +
                  GetArgType(imports, struct_def, field, true);
        }

        code += "):flatbuffers.Offset {\n";
        code += "  " + struct_def.name + ".start" +
                GetPrefixedName(struct_def) + "(builder);\n";

        std::string methodPrefix = struct_def.name;
        for (auto it = struct_def.fields.vec.begin();
             it != struct_def.fields.vec.end(); ++it) {
          const auto &field = **it;
          if (field.deprecated) continue;

          const auto arg_name = GetArgName(field);

          if (field.IsScalarOptional()) {
            code += "  if (" + arg_name + " !== null)\n  ";
          }

          code += "  " + methodPrefix + ".add" + MakeCamel(field.name) + "(";
          code += "builder, " + arg_name + ");\n";
        }

        code += "  return " + methodPrefix + ".end" +
                GetPrefixedName(struct_def) + "(builder);\n";
        code += "}\n";
      }
    }

    if (!struct_def.fixed && parser_.services_.vec.size() != 0) {
      auto name = GetPrefixedName(struct_def, "");
      code += "\n";
      code += "serialize():Uint8Array {\n";
      code += "  return this.bb!.bytes();\n";
      code += "}\n";

      code += "\n";
      code += "static deserialize(buffer: Uint8Array):" + name + " {\n";
      code += "  return " + AddImport(imports, struct_def, struct_def) +
              ".getRootAs" + name + "(new flatbuffers.ByteBuffer(buffer))\n";
      code += "}\n";
    }

    if (parser_.opts.generate_object_based_api) {
      std::string obj_api_class;
      std::string obj_api_unpack_func;
      GenObjApi(parser_, struct_def, obj_api_unpack_func, obj_api_class,
                imports);

      code += obj_api_unpack_func + "}\n" + obj_api_class;
    } else {
      code += "}\n";
    }
  }

  static bool HasNullDefault(const FieldDef &field) {
    return field.IsOptional() && field.value.constant == "null";
  }

  std::string GetArgType(import_set &imports, const Definition &owner,
                         const FieldDef &field, bool allowNull) {
    return GenTypeName(imports, owner, field.value.type, true,
                       allowNull && field.IsOptional());
  }

  static std::string GetArgName(const FieldDef &field) {
    auto argname = MakeCamel(field.name, false);
    if (!IsScalar(field.value.type.base_type)) { argname += "Offset"; }

    return argname;
  }

  std::string GetPrefixedName(const StructDef &struct_def,
                              const char *prefix = "") {
    return prefix + struct_def.name;
  }
};  // namespace ts
}  // namespace ts

bool GenerateTS(const Parser &parser, const std::string &path,
                const std::string &file_name) {
  ts::TsGenerator generator(parser, path, file_name);
  return generator.generate();
}

std::string TSMakeRule(const Parser &parser, const std::string &path,
                       const std::string &file_name) {
  std::string filebase =
      flatbuffers::StripPath(flatbuffers::StripExtension(file_name));
  ts::TsGenerator generator(parser, path, file_name);
  std::string make_rule =
      generator.GeneratedFileName(path, filebase, parser.opts) + ": ";

  auto included_files = parser.GetIncludedFilesRecursive(file_name);
  for (auto it = included_files.begin(); it != included_files.end(); ++it) {
    make_rule += " " + *it;
  }
  return make_rule;
}

}  // namespace flatbuffers
