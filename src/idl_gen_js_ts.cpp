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
#include <cassert>
#include <unordered_map>
#include <unordered_set>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

struct JsTsLanguageParameters {
  IDLOptions::Language language;
  std::string file_extension;
};

struct ReexportDescription {
  std::string symbol;
  std::string source_namespace;
  std::string target_namespace;
};

enum AnnotationType { kParam = 0, kType = 1, kReturns = 2 };

const JsTsLanguageParameters &GetJsLangParams(IDLOptions::Language lang) {
  static JsTsLanguageParameters js_language_parameters[] = {
    {
        IDLOptions::kJs,
        ".js",
    },
    {
        IDLOptions::kTs,
        ".ts",
    },
  };

  if (lang == IDLOptions::kJs) {
    return js_language_parameters[0];
  } else {
    FLATBUFFERS_ASSERT(lang == IDLOptions::kTs);
    return js_language_parameters[1];
  }
}

namespace jsts {
// Iterate through all definitions we haven't generate code for (enums, structs,
// and tables) and output them to a single file.
class JsTsGenerator : public BaseGenerator {
 public:
  typedef std::unordered_set<std::string> imported_fileset;
  typedef std::unordered_multimap<std::string, ReexportDescription>
      reexport_map;

  JsTsGenerator(const Parser &parser, const std::string &path,
                const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", ".",
                      parser.opts.lang == IDLOptions::kJs ? "js" : "ts"),
        lang_(GetJsLangParams(parser_.opts.lang)) {}
  // Iterate through all definitions we haven't generate code for (enums,
  // structs, and tables) and output them to a single file.
  bool generate() {
    imported_fileset imported_files;
    reexport_map reexports;

    std::string enum_code, struct_code, import_code, exports_code, code;
    generateEnums(&enum_code, &exports_code, reexports);
    generateStructs(&struct_code, &exports_code, imported_files);
    generateImportDependencies(&import_code, imported_files);
    generateReexports(&import_code, reexports, imported_files);

    code = code + "// " + FlatBuffersGeneratedWarning() + "\n\n";

    // Generate code for all the namespace declarations.
    GenNamespaces(&code, &exports_code);

    // Output the main declaration code from above.
    code += import_code;

    code += enum_code;
    code += struct_code;

    if (lang_.language == IDLOptions::kJs && !exports_code.empty() &&
        !parser_.opts.skip_js_exports) {
      if (parser_.opts.use_ES6_js_export_format)
        code += "// Exports for ECMAScript6 Modules\n";
      else
        code += "// Exports for Node.js and RequireJS\n";
      code += exports_code;
    }

    return SaveFile(GeneratedFileName(path_, file_name_, parser_.opts).c_str(),
                    code, false);
  }

 private:
  JsTsLanguageParameters lang_;

  // Generate code for imports
  void generateImportDependencies(std::string *code_ptr,
                                  const imported_fileset &imported_files) {
    std::string &code = *code_ptr;
    for (auto it = imported_files.begin(); it != imported_files.end(); ++it) {
      const auto &file = *it;
      const auto basename =
          flatbuffers::StripPath(flatbuffers::StripExtension(file));
      if (basename != file_name_) { code += GenPrefixedImport(file, basename); }
    }
  }

  // Generate reexports, which might not have been explicitly imported using the
  // "export import" trick
  void generateReexports(std::string *code_ptr, const reexport_map &reexports,
                         imported_fileset imported_files) {
    if (!parser_.opts.reexport_ts_modules ||
        lang_.language != IDLOptions::kTs) {
      return;
    }

    std::string &code = *code_ptr;
    for (auto it = reexports.begin(); it != reexports.end(); ++it) {
      const auto &file = *it;
      const auto basename =
          flatbuffers::StripPath(flatbuffers::StripExtension(file.first));
      if (basename != file_name_) {
        if (imported_files.find(file.first) == imported_files.end()) {
          code += GenPrefixedImport(file.first, basename);
          imported_files.emplace(file.first);
        }

        if (!file.second.target_namespace.empty()) {
          code += "export namespace " + file.second.target_namespace + " { \n";
        }
        code += "export import " + file.second.symbol + " = ";
        code += GenFileNamespacePrefix(file.first) + ".";
        if (!file.second.source_namespace.empty()) {
          code += file.second.source_namespace + ".";
        }
        code += file.second.symbol + ";\n";
        if (!file.second.target_namespace.empty()) {
          code += "}\n";
        }
      }
    }
  }

  // Generate code for all enums.
  void generateEnums(std::string *enum_code_ptr, std::string *exports_code_ptr,
                     reexport_map &reexports) {
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      auto &enum_def = **it;
      GenEnum(enum_def, enum_code_ptr, exports_code_ptr, reexports, false);
      GenEnum(enum_def, enum_code_ptr, exports_code_ptr, reexports, true);
    }
  }

  // Generate code for all structs.
  void generateStructs(std::string *decl_code_ptr,
                       std::string *exports_code_ptr,
                       imported_fileset &imported_files) {
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      auto &struct_def = **it;
      GenStruct(parser_, struct_def, decl_code_ptr, exports_code_ptr,
                imported_files);
    }
  }
  void GenNamespaces(std::string *code_ptr, std::string *exports_ptr) {
    if (lang_.language == IDLOptions::kTs &&
        parser_.opts.skip_flatbuffers_import) {
      return;
    }

    std::set<std::string> namespaces;

    for (auto it = parser_.namespaces_.begin(); it != parser_.namespaces_.end();
         ++it) {
      std::string namespace_so_far;

      // Gather all parent namespaces for this namespace
      for (auto component = (*it)->components.begin();
           component != (*it)->components.end(); ++component) {
        if (!namespace_so_far.empty()) { namespace_so_far += '.'; }
        namespace_so_far += *component;
        namespaces.insert(namespace_so_far);
      }
    }

    // Make sure parent namespaces come before child namespaces
    std::vector<std::string> sorted_namespaces(namespaces.begin(),
                                               namespaces.end());
    std::sort(sorted_namespaces.begin(), sorted_namespaces.end());

    // Emit namespaces in a form that Closure Compiler can optimize
    std::string &code = *code_ptr;
    std::string &exports = *exports_ptr;
    for (auto it = sorted_namespaces.begin(); it != sorted_namespaces.end();
         ++it) {
      if (lang_.language == IDLOptions::kTs) {
        if (it->find('.') == std::string::npos) {
          code += "import { flatbuffers } from \"./flatbuffers\"\n";
          break;
        }
      } else {
        code += "/**\n * @const\n * @namespace\n */\n";
        if (it->find('.') == std::string::npos) {
          code += "var ";
          if (parser_.opts.use_goog_js_export_format) {
            exports += "goog.exportSymbol('" + *it + "', " + *it + ");\n";
          } else if (parser_.opts.use_ES6_js_export_format) {
            exports += "export {" + *it + "};\n";
          } else {
            exports += "this." + *it + " = " + *it + ";\n";
          }
        }
        code += *it + " = " + *it + " || {};\n\n";
      }
    }
  }

  // Generate a documentation comment, if available.
  static void GenDocComment(const std::vector<std::string> &dc,
                            std::string *code_ptr,
                            const std::string &extra_lines,
                            const char *indent = nullptr) {
    if (dc.empty() && extra_lines.empty()) {
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
    if (!extra_lines.empty()) {
      if (!dc.empty()) {
        if (indent) code += indent;
        code += " *\n";
      }
      if (indent) code += indent;
      std::string::size_type start = 0;
      for (;;) {
        auto end = extra_lines.find('\n', start);
        if (end != std::string::npos) {
          code += " * " + extra_lines.substr(start, end - start) + "\n";
          start = end + 1;
        } else {
          code += " * " + extra_lines.substr(start) + "\n";
          break;
        }
      }
    }
    if (indent) code += indent;
    code += " */\n";
  }

  static void GenDocComment(std::string *code_ptr,
                            const std::string &extra_lines) {
    GenDocComment(std::vector<std::string>(), code_ptr, extra_lines);
  }

  std::string GenTypeAnnotation(AnnotationType annotation_type,
                                const std::string &type_name,
                                const std::string &arg_name,
                                bool include_newline = true) {
    std::string result = "";
    switch (annotation_type) {
      case kParam: {
        result += "@param";
        break;
      }
      case kType: {
        if (lang_.language != IDLOptions::kTs) {
          result += "@type";
        } else {
          return "";
        }
        break;
      }
      case kReturns: {
        result += "@returns";
        break;
      }
    }
    switch (lang_.language) {
      case IDLOptions::kTs: {
        result += " " + type_name;
        break;
      }
      default: {
        result += " {" + type_name + "}";
      }
    }
    if (!arg_name.empty()) { result += " " + arg_name; }
    if (include_newline) { result += "\n"; }

    return result;
  }

  // Generate an enum declaration and an enum string lookup table.
  void GenEnum(EnumDef &enum_def, std::string *code_ptr,
               std::string *exports_ptr, reexport_map &reexports,
               bool reverse) {
    if (enum_def.generated) return;
    if (reverse && lang_.language == IDLOptions::kTs) return;  // FIXME.
    std::string &code = *code_ptr;
    std::string &exports = *exports_ptr;
    GenDocComment(enum_def.doc_comment, code_ptr,
                  reverse ? "@enum {string}" : "@enum {number}");
    std::string ns = GetNameSpace(enum_def);
    std::string enum_def_name = enum_def.name + (reverse ? "Name" : "");
    if (lang_.language == IDLOptions::kTs) {
      if (!ns.empty()) { code += "export namespace " + ns + "{\n"; }
      code += "export enum " + enum_def.name + "{\n";
    } else {
      if (enum_def.defined_namespace->components.empty()) {
        code += "var ";
        if (parser_.opts.use_goog_js_export_format) {
          exports += "goog.exportSymbol('" + enum_def_name + "', " +
                     enum_def.name + ");\n";
        } else if (parser_.opts.use_ES6_js_export_format) {
          exports += "export {" + enum_def_name + "};\n";
        } else {
          exports += "this." + enum_def_name + " = " + enum_def_name + ";\n";
        }
      }
      code += WrapInNameSpace(enum_def) + (reverse ? "Name" : "") + " = {\n";
    }
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      if (!ev.doc_comment.empty()) {
        if (it != enum_def.Vals().begin()) { code += '\n'; }
        GenDocComment(ev.doc_comment, code_ptr, "", "  ");
      }

      // Generate mapping between EnumName: EnumValue(int)
      if (reverse) {
        code += "  '" + enum_def.ToString(ev) + "'";
        code += lang_.language == IDLOptions::kTs ? "= " : ": ";
        code += "'" + ev.name + "'";
      } else {
        code += "  " + ev.name;
        code += lang_.language == IDLOptions::kTs ? "= " : ": ";
        code += enum_def.ToString(ev);
      }

      code += (it + 1) != enum_def.Vals().end() ? ",\n" : "\n";

      if (ev.union_type.struct_def) {
        ReexportDescription desc = { ev.name,
                                     GetNameSpace(*ev.union_type.struct_def),
                                     GetNameSpace(enum_def) };
        reexports.insert(
            std::make_pair(ev.union_type.struct_def->file, std::move(desc)));
      }
    }
    code += "};";

    if (lang_.language == IDLOptions::kTs) {
      if (enum_def.is_union) {
        code += GenUnionConvFunc(enum_def.underlying_type);
      }
      if (!ns.empty()) { code += "\n}"; }
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
      default: return "Table";
    }
  }

  std::string GenGetter(const Type &type, const std::string &arguments) {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return GenBBAccess() + ".__string" + arguments;
      case BASE_TYPE_STRUCT: return GenBBAccess() + ".__struct" + arguments;
      case BASE_TYPE_UNION:
        if (!UnionHasStringType(*type.enum_def) ||
            lang_.language == IDLOptions::kJs) {
          return GenBBAccess() + ".__union" + arguments;
        }
        return GenBBAccess() + ".__union_with_string" + arguments;
      case BASE_TYPE_VECTOR: return GenGetter(type.VectorType(), arguments);
      default: {
        auto getter =
            GenBBAccess() + ".read" + MakeCamel(GenType(type)) + arguments;
        if (type.base_type == BASE_TYPE_BOOL) { getter = "!!" + getter; }
        if (type.enum_def) {
          getter = "/** " +
                   GenTypeAnnotation(kType, WrapInNameSpace(*type.enum_def), "",
                                     false) +
                   " */ (" + getter + ")";
        }
        return getter;
      }
    }
  }

  std::string GenBBAccess() {
    return lang_.language == IDLOptions::kTs ? "this.bb!" : "this.bb";
  }

  std::string GenDefaultValue(const Value &value, const std::string &context) {
    if (value.type.enum_def && value.type.base_type != BASE_TYPE_UNION &&
        value.type.base_type != BASE_TYPE_VECTOR) {
      if (auto val = value.type.enum_def->FindByValue(value.constant)) {
        if (lang_.language == IDLOptions::kTs) {
          return GenPrefixedTypeName(WrapInNameSpace(*value.type.enum_def),
                                     value.type.enum_def->file) +
                 "." + val->name;
        } else {
          return WrapInNameSpace(*value.type.enum_def) + "." + val->name;
        }
      } else {
        return "/** " +
               GenTypeAnnotation(kType, WrapInNameSpace(*value.type.enum_def),
                                 "", false) +
               "} */ (" + value.constant + ")";
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
        return context + ".createLong(" +
               NumToString(static_cast<int32_t>(constant)) + ", " +
               NumToString(static_cast<int32_t>(constant >> 32)) + ")";
      }

      default: return value.constant;
    }
  }

  std::string GenTypeName(const Type &type, bool input,
                          bool allowNull = false) {
    if (!input) {
      if (type.base_type == BASE_TYPE_STRING ||
          type.base_type == BASE_TYPE_STRUCT) {
        std::string name;
        if (type.base_type == BASE_TYPE_STRING) {
          name = "string|Uint8Array";
        } else {
          name = WrapInNameSpace(*type.struct_def);
        }
        return (allowNull) ? (name + "|null") : (name);
      }
    }

    switch (type.base_type) {
      case BASE_TYPE_BOOL: return "boolean";
      case BASE_TYPE_LONG:
      case BASE_TYPE_ULONG: return "flatbuffers.Long";
      default:
        if (IsScalar(type.base_type)) {
          if (type.enum_def) { return WrapInNameSpace(*type.enum_def); }
          return "number";
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

  static std::string GenFileNamespacePrefix(const std::string &file) {
    return "NS" + std::to_string(HashFnv1a<uint64_t>(file.c_str()));
  }

  std::string GenPrefixedImport(const std::string &full_file_name,
                                const std::string &base_name) {
    // Either keep the include path as it was
    // or use only the base_name + kGeneratedFileNamePostfix
    std::string path;
    if (parser_.opts.keep_include_path) {
      auto it = parser_.included_files_.find(full_file_name);
      FLATBUFFERS_ASSERT(it != parser_.included_files_.end());
      path = flatbuffers::StripExtension(it->second) +
             parser_.opts.filename_suffix;
    } else {
      path = base_name + parser_.opts.filename_suffix;
    }

    // Add the include prefix and make the path always relative
    path = flatbuffers::ConCatPathFileName(parser_.opts.include_prefix, path);
    path = std::string(".") + kPathSeparator + path;

    return "import * as " + GenFileNamespacePrefix(full_file_name) +
           " from \"" + path + "\";\n";
  }

  // Adds a source-dependent prefix, for of import * statements.
  std::string GenPrefixedTypeName(const std::string &typeName,
                                  const std::string &file) {
    const auto basename =
        flatbuffers::StripPath(flatbuffers::StripExtension(file));
    if (basename == file_name_ || parser_.opts.generate_all) {
      return typeName;
    }
    return GenFileNamespacePrefix(file) + "." + typeName;
  }

  std::string GenFullNameSpace(const Definition &def, const std::string &file) {
    return GenPrefixedTypeName(GetNameSpace(def), file);
  }

  void GenStructArgs(const StructDef &struct_def, std::string *annotations,
                     std::string *arguments, const std::string &nameprefix) {
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (IsStruct(field.value.type)) {
        // Generate arguments for a struct inside a struct. To ensure names
        // don't clash, and to make it obvious these arguments are constructing
        // a nested struct, prefix the name with the field name.
        GenStructArgs(*field.value.type.struct_def, annotations, arguments,
                      nameprefix + field.name + "_");
      } else {
        *annotations +=
            GenTypeAnnotation(kParam, GenTypeName(field.value.type, true),
                              nameprefix + field.name);
        if (lang_.language == IDLOptions::kTs) {
          *arguments += ", " + nameprefix + field.name + ": " +
                        GenTypeName(field.value.type, true);
        } else {
          *arguments += ", " + nameprefix + field.name;
        }
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
    return "new " + object_name +
           (lang_.language == IDLOptions::kTs ? "()" : "");
  }

  void GenerateRootAccessor(StructDef &struct_def, std::string *code_ptr,
                            std::string &code, std::string &object_name,
                            bool size_prefixed) {
    if (!struct_def.fixed) {
      GenDocComment(code_ptr,
                    GenTypeAnnotation(kParam, "flatbuffers.ByteBuffer", "bb") +
                        GenTypeAnnotation(kParam, object_name + "=", "obj") +
                        GenTypeAnnotation(kReturns, object_name, "", false));
      std::string sizePrefixed("SizePrefixed");
      if (lang_.language == IDLOptions::kTs) {
        code += "static get" + (size_prefixed ? sizePrefixed : "") + "Root" +
                Verbose(struct_def, "As");
        code += "(bb:flatbuffers.ByteBuffer, obj?:" + object_name +
                "):" + object_name + " {\n";
      } else {
        code += object_name + ".get" + (size_prefixed ? sizePrefixed : "") +
                "Root" + Verbose(struct_def, "As");
        code += " = function(bb, obj) {\n";
      }
      if (size_prefixed) {
        code +=
            "  bb.setPosition(bb.position() + "
            "flatbuffers.SIZE_PREFIX_LENGTH);\n";
      }
      code += "  return (obj || " + GenerateNewExpression(object_name);
      code += ").__init(bb.readInt32(bb.position()) + bb.position(), bb);\n";
      code += "};\n\n";
    }
  }

  void GenerateFinisher(StructDef &struct_def, std::string *code_ptr,
                        std::string &code, std::string &object_name,
                        bool size_prefixed) {
    if (parser_.root_struct_def_ == &struct_def) {
      std::string sizePrefixed("SizePrefixed");
      GenDocComment(
          code_ptr,
          GenTypeAnnotation(kParam, "flatbuffers.Builder", "builder") +
              GenTypeAnnotation(kParam, "flatbuffers.Offset", "offset", false));

      if (lang_.language == IDLOptions::kTs) {
        code += "static finish" + (size_prefixed ? sizePrefixed : "") +
                Verbose(struct_def) + "Buffer";
        code += "(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {\n";
      } else {
        code += object_name + ".finish" + (size_prefixed ? sizePrefixed : "") +
                Verbose(struct_def) + "Buffer";
        code += " = function(builder, offset) {\n";
      }

      code += "  builder.finish(offset";
      if (!parser_.file_identifier_.empty()) {
        code += ", '" + parser_.file_identifier_ + "'";
      }
      if (size_prefixed) {
        if (parser_.file_identifier_.empty()) { code += ", undefined"; }
        code += ", true";
      }
      code += ");\n";
      code += "};\n\n";
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
                         return !(ev->IsZero()) &&
                                (ev->union_type.base_type == BASE_TYPE_STRING);
                       });
  }

  std::string GenUnionGenericTypeTS(const EnumDef &union_enum) {
    return std::string("T") + (UnionHasStringType(union_enum) ? "|string" : "");
  }

  std::string GenUnionTypeTS(const EnumDef &union_enum) {
    std::string ret;
    std::set<std::string> type_list;

    for (auto it = union_enum.Vals().begin(); it != union_enum.Vals().end();
         ++it) {
      const auto &ev = **it;
      if (ev.IsZero()) { continue; }

      std::string type = "";
      if (ev.union_type.base_type == BASE_TYPE_STRING) {
        type = "string";  // no need to wrap string type in namespace
      } else if (ev.union_type.base_type == BASE_TYPE_STRUCT) {
        type = GenPrefixedTypeName(WrapInNameSpace(*(ev.union_type.struct_def)),
                                   union_enum.file);
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

  // Generate a TS union type based on a union's enum
  std::string GenObjApiUnionTypeTS(const IDLOptions &opts,
                                   const EnumDef &union_enum) {
    std::string ret = "";
    std::set<std::string> type_list;

    for (auto it = union_enum.Vals().begin(); it != union_enum.Vals().end();
         ++it) {
      const auto &ev = **it;
      if (ev.IsZero()) { continue; }

      std::string type = "";
      if (ev.union_type.base_type == BASE_TYPE_STRING) {
        type = "string";  // no need to wrap string type in namespace
      } else if (ev.union_type.base_type == BASE_TYPE_STRUCT) {
        type = GenPrefixedTypeName(
            GetObjApiClassName(WrapInNameSpace(*(ev.union_type.struct_def)),
                               opts),
            union_enum.file);
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

  std::string GenUnionConvFunc(const Type &union_type) {
    if (union_type.enum_def) {
      const auto &enum_def = *union_type.enum_def;

      const auto valid_union_type = GenUnionTypeTS(enum_def);
      const auto valid_union_type_with_null = valid_union_type + "|null";

      auto ret = "\n\nexport function " + GenUnionConvFuncName(enum_def) +
                 "(\n  type: " + enum_def.name +
                 ",\n  accessor: (obj:" + valid_union_type + ") => " +
                 valid_union_type_with_null +
                 "\n): " + valid_union_type_with_null + " {\n";

      const auto enum_type = GenPrefixedTypeName(
          WrapInNameSpace(*(union_type.enum_def)), union_type.enum_def->file);
      const auto &union_enum = *(union_type.enum_def);

      const auto union_enum_loop = [&](const std::string &accessor_str) {
        ret += "  switch(" + enum_type + "[type]) {\n";
        ret += "    case 'NONE': return null; \n";

        for (auto it = union_enum.Vals().begin(); it != union_enum.Vals().end();
             ++it) {
          const auto &ev = **it;
          if (ev.IsZero()) { continue; }

          ret += "    case '" + ev.name + "': ";

          if (ev.union_type.base_type == BASE_TYPE_STRING) {
            ret += "return " + accessor_str + "'') as string;";
          } else if (ev.union_type.base_type == BASE_TYPE_STRUCT) {
            const auto type = GenPrefixedTypeName(
                WrapInNameSpace(*(ev.union_type.struct_def)), union_enum.file);
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
  std::string GenUnionValTS(const std::string &field_name,
                            const Type &union_type,
                            const bool is_array = false) {
    if (union_type.enum_def) {
      const auto &enum_def = *union_type.enum_def;
      const auto enum_type =
          GenPrefixedTypeName(WrapInNameSpace(enum_def), enum_def.file);
      const std::string union_accessor = "this." + field_name;

      const auto union_has_string = UnionHasStringType(enum_def);
      const auto field_binded_method = "this." + field_name + ".bind(this)";

      std::string ret = "";

      if (!is_array) {
        const auto conversion_function =
            GenPrefixedTypeName(WrapInNameSpace(enum_def.defined_namespace,
                                                GenUnionConvFuncName(enum_def)),
                                enum_def.file);
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
        const auto conversion_function = GenPrefixedTypeName(
            WrapInNameSpace(enum_def.defined_namespace,
                            GenUnionListConvFuncName(enum_def)),
            enum_def.file);
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

  std::string GenNullCheckConditional(const std::string &nullCheckVar,
                                      const std::string &trueVal,
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
                 std::string &obj_api_unpack_func, std::string &obj_api_class) {
    const auto class_name = GetObjApiClassName(struct_def, parser.opts);

    std::string unpack_func =
        "\n/**\n * " + GenTypeAnnotation(kReturns, class_name, "") +
        " */\nunpack(): " + class_name + " {\n  return new " + class_name +
        "(" + (struct_def.fields.vec.empty() ? "" : "\n");
    std::string unpack_to_func =
        "/**\n * " + GenTypeAnnotation(kParam, class_name, "_o") +
        " */\nunpackTo(_o: " + class_name + "): void {" +
        +(struct_def.fields.vec.empty() ? "" : "\n");

    std::string constructor_annotation = "/**\n * @constructor";
    constructor_annotation += (struct_def.fields.vec.empty() ? "" : "\n");
    std::string constructor_func = "constructor(";
    constructor_func += (struct_def.fields.vec.empty() ? "" : "\n");

    std::string pack_func_prototype =
        "/**\n * " +
        GenTypeAnnotation(kParam, "flatbuffers.Builder", "builder") + " * " +
        GenTypeAnnotation(kReturns, "flatbuffers.Offset", "") +
        " */\npack(builder:flatbuffers.Builder): flatbuffers.Offset {\n";
    std::string pack_func_offset_decl;
    std::string pack_func_create_call =
        "  return " + Verbose(struct_def) + ".create" + Verbose(struct_def) +
        "(builder" + (struct_def.fields.vec.empty() ? "" : ",\n    ");
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
          GenDefaultValue(field.value, "flatbuffers");

      // Emit a scalar field
      if (IsScalar(field.value.type.base_type) ||
          field.value.type.base_type == BASE_TYPE_STRING) {
        if (field.value.type.enum_def) {
          field_type +=
              GenPrefixedTypeName(GenTypeName(field.value.type, false, true),
                                  field.value.type.enum_def->file);
        } else {
          field_type += GenTypeName(field.value.type, false, true);
        }
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
            field_type += GenPrefixedTypeName(
                WrapInNameSpace(sd.defined_namespace,
                                GetObjApiClassName(sd, parser.opts)),
                field.value.type.struct_def->file);

            const std::string field_accessor = "this." + field_name + "()";
            field_val = GenNullCheckConditional(field_accessor,
                                                field_accessor + "!.unpack()");
            field_offset_val = GenNullCheckConditional(
                "this." + field_name, "this." + field_name + "!.pack(builder)",
                "0");

            break;
          }

          case BASE_TYPE_VECTOR: {
            auto vectortype = field.value.type.VectorType();
            auto vectortypename = GenTypeName(vectortype, false);
            is_vector = true;

            field_type = "(";

            switch (vectortype.base_type) {
              case BASE_TYPE_STRUCT: {
                const auto &sd = *field.value.type.struct_def;
                field_type += GenPrefixedTypeName(
                    WrapInNameSpace(sd.defined_namespace,
                                    GetObjApiClassName(sd, parser.opts)),
                    field.value.type.struct_def->file);
                field_type += ")[]";

                field_val = GenBBAccess() + ".createObjList(" +
                            field_binded_method + ", this." + field_name +
                            "Length())";

                if (sd.fixed) {
                  field_offset_decl = "builder.createStructOffsetList(this." +
                                      field_name + ", " + Verbose(struct_def) +
                                      ".start" + MakeCamel(field_name) +
                                      "Vector)";
                } else {
                  field_offset_decl =
                      Verbose(struct_def) + ".create" + MakeCamel(field_name) +
                      "Vector(builder, builder.createObjectOffsetList(" +
                      "this." + field_name + "))";
                }

                break;
              }

              case BASE_TYPE_STRING: {
                field_type += "string)[]";
                field_val = GenBBAccess() + ".createStringList(" +
                            field_binded_method + ", this." + field_name +
                            "Length())";
                field_offset_decl =
                    Verbose(struct_def) + ".create" + MakeCamel(field_name) +
                    "Vector(builder, builder.createObjectOffsetList(" +
                    "this." + field_name + "))";
                break;
              }

              case BASE_TYPE_UNION: {
                field_type +=
                    GenObjApiUnionTypeTS(parser.opts, *(vectortype.enum_def));
                field_type += ")[]";
                field_val = GenUnionValTS(field_name, vectortype, true);

                field_offset_decl =
                    Verbose(struct_def) + ".create" + MakeCamel(field_name) +
                    "Vector(builder, builder.createObjectOffsetList(" +
                    "this." + field_name + "))";

                break;
              }
              default: {
                if (vectortype.enum_def) {
                  field_type +=
                      GenPrefixedTypeName(GenTypeName(vectortype, false, true),
                                          vectortype.enum_def->file);
                } else {
                  field_type += vectortypename;
                }
                field_type += ")[]";
                field_val = GenBBAccess() + ".createScalarList(" +
                            field_binded_method + ", this." + field_name +
                            "Length())";

                field_offset_decl = Verbose(struct_def) + ".create" +
                                    MakeCamel(field_name) +
                                    "Vector(builder, this." + field_name + ")";

                break;
              }
            }

            break;
          }

          case BASE_TYPE_UNION: {
            field_type +=
                GenObjApiUnionTypeTS(parser.opts, *(field.value.type.enum_def));

            field_val = GenUnionValTS(field_name, field.value.type);
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

      constructor_annotation +=
          " * " + GenTypeAnnotation(kParam, field_type, field_name, false);
      constructor_func += "  public " + field_name + ": " + field_type + " = " +
                          field_default_val;

      if (!struct_def.fixed) {
        if (!field_offset_decl.empty()) {
          pack_func_offset_decl += field_offset_decl + "\n";
        }
        pack_func_create_call += field_offset_val;
      }

      if (std::next(it) != struct_def.fields.vec.end()) {
        constructor_annotation += "\n";
        constructor_func += ",\n";

        if (!struct_def.fixed) { pack_func_create_call += ",\n    "; }

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

    constructor_annotation += "\n */\n";
    constructor_func += "){};\n\n";

    pack_func_create_call += ");";

    obj_api_class = "\nexport class " +
                    GetObjApiClassName(struct_def, parser.opts) + " {\n";

    obj_api_class += constructor_annotation + constructor_func;

    obj_api_class += pack_func_prototype + pack_func_offset_decl +
                     pack_func_create_call + "\n};";

    obj_api_class += "\n}\n";

    unpack_func += ");\n};";
    unpack_to_func += "};\n";

    obj_api_unpack_func = unpack_func + "\n\n" + unpack_to_func;
  }

  // Generate an accessor struct with constructor for a flatbuffers struct.
  void GenStruct(const Parser &parser, StructDef &struct_def,
                 std::string *code_ptr, std::string *exports_ptr,
                 imported_fileset &imported_files) {
    if (struct_def.generated) return;
    std::string &code = *code_ptr;
    std::string &exports = *exports_ptr;

    std::string object_name;
    std::string object_namespace = GetNameSpace(struct_def);

    // Emit constructor
    if (lang_.language == IDLOptions::kTs) {
      object_name = struct_def.name;
      GenDocComment(struct_def.doc_comment, code_ptr, "@constructor");
      if (!object_namespace.empty()) {
        code += "export namespace " + object_namespace + "{\n";
      }
      code += "export class " + struct_def.name;
      code += " {\n";
      if (lang_.language != IDLOptions::kTs) {
        code += "  /**\n";
        code +=
            "   * " + GenTypeAnnotation(kType, "flatbuffers.ByteBuffer", "");
        code += "   */\n";
      }
      code += "  bb: flatbuffers.ByteBuffer|null = null;\n";
      code += "\n";
      if (lang_.language != IDLOptions::kTs) {
        code += "  /**\n";
        code += "   * " + GenTypeAnnotation(kType, "number", "");
        code += "   */\n";
      }
      code += "  bb_pos:number = 0;\n";
    } else {
      bool isStatement = struct_def.defined_namespace->components.empty();
      object_name = WrapInNameSpace(struct_def);
      GenDocComment(struct_def.doc_comment, code_ptr, "@constructor");
      if (isStatement) {
        if (parser_.opts.use_goog_js_export_format) {
          exports += "goog.exportSymbol('" + struct_def.name + "', " +
                     struct_def.name + ");\n";
        } else if (parser_.opts.use_ES6_js_export_format) {
          exports += "export {" + struct_def.name + "};\n";
        } else {
          exports +=
              "this." + struct_def.name + " = " + struct_def.name + ";\n";
        }
        code += "function " + object_name;
      } else {
        code += object_name + " = function";
      }
      code += "() {\n";
      code += "  /**\n";
      code += "   * " + GenTypeAnnotation(kType, "flatbuffers.ByteBuffer", "");
      code += "   */\n";
      code += "  this.bb = null;\n";
      code += "\n";
      code += "  /**\n";
      code += "   * " + GenTypeAnnotation(kType, "number", "");
      code += "   */\n";
      code += "  this.bb_pos = 0;\n";
      code += isStatement ? "}\n\n" : "};\n\n";
    }

    // Generate the __init method that sets the field in a pre-existing
    // accessor object. This is to allow object reuse.
    code += "/**\n";
    code += " * " + GenTypeAnnotation(kParam, "number", "i");
    code += " * " + GenTypeAnnotation(kParam, "flatbuffers.ByteBuffer", "bb");
    code += " * " + GenTypeAnnotation(kReturns, object_name, "");
    code += " */\n";

    if (lang_.language == IDLOptions::kTs) {
      code +=
          "__init(i:number, bb:flatbuffers.ByteBuffer):" + object_name + " {\n";
    } else {
      code += object_name + ".prototype.__init = function(i, bb) {\n";
    }

    code += "  this.bb_pos = i;\n";
    code += "  this.bb = bb;\n";
    code += "  return this;\n";
    code += "};\n\n";

    // Generate special accessors for the table that when used as the root of a
    // FlatBuffer
    GenerateRootAccessor(struct_def, code_ptr, code, object_name, false);
    GenerateRootAccessor(struct_def, code_ptr, code, object_name, true);

    // Generate the identifier check method
    if (!struct_def.fixed && parser_.root_struct_def_ == &struct_def &&
        !parser_.file_identifier_.empty()) {
      GenDocComment(code_ptr,
                    GenTypeAnnotation(kParam, "flatbuffers.ByteBuffer", "bb") +
                        GenTypeAnnotation(kReturns, "boolean", "", false));
      if (lang_.language == IDLOptions::kTs) {
        code +=
            "static bufferHasIdentifier(bb:flatbuffers.ByteBuffer):boolean "
            "{\n";
      } else {
        code += object_name + ".bufferHasIdentifier = function(bb) {\n";
      }

      code += "  return bb.__has_identifier('" + parser_.file_identifier_;
      code += "');\n};\n\n";
    }

    // Emit field accessors
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      auto offset_prefix =
          "  var offset = " + GenBBAccess() + ".__offset(this.bb_pos, " +
          NumToString(field.value.offset) + ");\n  return offset ? ";

      // Emit a scalar field
      if (IsScalar(field.value.type.base_type) ||
          field.value.type.base_type == BASE_TYPE_STRING) {
        GenDocComment(
            field.doc_comment, code_ptr,
            std::string(field.value.type.base_type == BASE_TYPE_STRING
                            ? GenTypeAnnotation(kParam, "flatbuffers.Encoding=",
                                                "optionalEncoding")
                            : "") +
                GenTypeAnnotation(kReturns,
                                  GenTypeName(field.value.type, false, true),
                                  "", false));
        if (lang_.language == IDLOptions::kTs) {
          std::string prefix = MakeCamel(field.name, false) + "(";
          if (field.value.type.base_type == BASE_TYPE_STRING) {
            code += prefix + "):string|null\n";
            code += prefix + "optionalEncoding:flatbuffers.Encoding" +
                    "):" + GenTypeName(field.value.type, false, true) + "\n";
            code += prefix + "optionalEncoding?:any";
          } else {
            code += prefix;
          }
          if (field.value.type.enum_def) {
            code +=
                "):" +
                GenPrefixedTypeName(GenTypeName(field.value.type, false, true),
                                    field.value.type.enum_def->file) +
                " {\n";

            if (!parser_.opts.generate_all) {
              imported_files.insert(field.value.type.enum_def->file);
            }
          } else {
            code += "):" + GenTypeName(field.value.type, false, true) + " {\n";
          }
        } else {
          code += object_name + ".prototype." + MakeCamel(field.name, false);
          code += " = function(";
          if (field.value.type.base_type == BASE_TYPE_STRING) {
            code += "optionalEncoding";
          }
          code += ") {\n";
        }

        if (struct_def.fixed) {
          code +=
              "  return " +
              GenGetter(field.value.type,
                        "(this.bb_pos" + MaybeAdd(field.value.offset) + ")") +
              ";\n";
        } else {
          std::string index = "this.bb_pos + offset";
          if (field.value.type.base_type == BASE_TYPE_STRING) {
            index += ", optionalEncoding";
          }
          code += offset_prefix +
                  GenGetter(field.value.type, "(" + index + ")") + " : " +
                  GenDefaultValue(field.value, GenBBAccess());
          code += ";\n";
        }
      }

      // Emit an object field
      else {
        switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT: {
            auto type = WrapInNameSpace(*field.value.type.struct_def);
            GenDocComment(
                field.doc_comment, code_ptr,
                GenTypeAnnotation(kParam, type + "=", "obj") +
                    GenTypeAnnotation(kReturns, type + "|null", "", false));
            if (lang_.language == IDLOptions::kTs) {
              type =
                  GenPrefixedTypeName(type, field.value.type.struct_def->file);
              code += MakeCamel(field.name, false);
              code += "(obj?:" + type + "):" + type + "|null {\n";
            } else {
              code +=
                  object_name + ".prototype." + MakeCamel(field.name, false);
              code += " = function(obj) {\n";
            }

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

            if (lang_.language == IDLOptions::kTs &&
                !parser_.opts.generate_all) {
              imported_files.insert(field.value.type.struct_def->file);
            }

            break;
          }

          case BASE_TYPE_VECTOR: {
            auto vectortype = field.value.type.VectorType();
            auto vectortypename = GenTypeName(vectortype, false);
            auto inline_size = InlineSize(vectortype);
            auto index = GenBBAccess() +
                         ".__vector(this.bb_pos + offset) + index" +
                         MaybeScale(inline_size);
            std::string args = GenTypeAnnotation(kParam, "number", "index");
            std::string ret_type;
            bool is_union = false;
            switch (vectortype.base_type) {
              case BASE_TYPE_STRUCT:
                args += GenTypeAnnotation(kParam, vectortypename + "=", "obj");
                ret_type = vectortypename;
                break;
              case BASE_TYPE_STRING:
                args += GenTypeAnnotation(
                    kParam, "flatbuffers.Encoding=", "optionalEncoding");
                ret_type = vectortypename;
                break;
              case BASE_TYPE_UNION:
                args += GenTypeAnnotation(kParam, "flatbuffers.Table=", "obj");
                ret_type = "?flatbuffers.Table";
                is_union = true;
                break;
              default: ret_type = vectortypename;
            }
            GenDocComment(
                field.doc_comment, code_ptr,
                args + GenTypeAnnotation(kReturns, ret_type, "", false));
            if (lang_.language == IDLOptions::kTs) {
              std::string prefix = MakeCamel(field.name, false);
              if (is_union) { prefix += "<T extends flatbuffers.Table>"; }
              prefix += "(index: number";
              if (is_union) {
                const auto union_type =
                    GenUnionGenericTypeTS(*(field.value.type.enum_def));

                vectortypename = union_type;
                code += prefix + ", obj:" + union_type;
              } else if (vectortype.base_type == BASE_TYPE_STRUCT) {
                vectortypename = GenPrefixedTypeName(
                    vectortypename, vectortype.struct_def->file);
                code += prefix + ", obj?:" + vectortypename;

                if (!parser_.opts.generate_all) {
                  imported_files.insert(vectortype.struct_def->file);
                }
              } else if (vectortype.base_type == BASE_TYPE_STRING) {
                code += prefix + "):string\n";
                code += prefix + ",optionalEncoding:flatbuffers.Encoding" +
                        "):" + vectortypename + "\n";
                code += prefix + ",optionalEncoding?:any";
              } else {
                code += prefix;
              }
              code += "):" + vectortypename + "|null {\n";
            } else {
              code +=
                  object_name + ".prototype." + MakeCamel(field.name, false);
              code += " = function(index";
              if (vectortype.base_type == BASE_TYPE_STRUCT || is_union) {
                code += ", obj";
              } else if (vectortype.base_type == BASE_TYPE_STRING) {
                code += ", optionalEncoding";
              }
              code += ") {\n";
            }

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
              } else if (vectortype.base_type == BASE_TYPE_STRING) {
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
                code += "/** " +
                        GenTypeAnnotation(
                            kType, WrapInNameSpace(*field.value.type.enum_def),
                            "", false) +
                        " */ (" + field.value.constant + ")";
              } else {
                code += "0";
              }
            } else {
              code += "null";
            }
            code += ";\n";
            break;
          }

          case BASE_TYPE_UNION:
            GenDocComment(
                field.doc_comment, code_ptr,
                GenTypeAnnotation(kParam, "flatbuffers.Table", "obj") +
                    GenTypeAnnotation(kReturns, "?flatbuffers.Table", "",
                                      false));
            if (lang_.language == IDLOptions::kTs) {
              code += MakeCamel(field.name, false);

              const auto &union_enum = *(field.value.type.enum_def);
              const auto union_type = GenUnionGenericTypeTS(union_enum);
              code += "<T extends flatbuffers.Table>(obj:" + union_type +
                      "):" + union_type +
                      "|null "
                      "{\n";
            } else {
              code +=
                  object_name + ".prototype." + MakeCamel(field.name, false);
              code += " = function(obj) {\n";
            }

            code += offset_prefix +
                    GenGetter(field.value.type, "(obj, this.bb_pos + offset)") +
                    " : null;\n";
            break;

          default: FLATBUFFERS_ASSERT(0);
        }
      }
      code += "};\n\n";

      if (parser_.opts.use_goog_js_export_format) {
        exports += "goog.exportProperty(" + object_name + ".prototype, '" +
                   MakeCamel(field.name, false) + "', " + object_name +
                   ".prototype." + MakeCamel(field.name, false) + ");\n";
      }

      // Adds the mutable scalar value to the output
      if (IsScalar(field.value.type.base_type) && parser.opts.mutable_buffer &&
          !IsUnion(field.value.type)) {
        std::string annotations = GenTypeAnnotation(
            kParam, GenTypeName(field.value.type, true), "value");
        GenDocComment(
            code_ptr,
            annotations + GenTypeAnnotation(kReturns, "boolean", "", false));

        if (lang_.language == IDLOptions::kTs) {
          std::string type;
          if (field.value.type.enum_def) {
            type = GenPrefixedTypeName(GenTypeName(field.value.type, true),
                                       field.value.type.enum_def->file);
          } else {
            type = GenTypeName(field.value.type, true);
          }

          code += "mutate_" + field.name + "(value:" + type + "):boolean {\n";
        } else {
          code += object_name + ".prototype.mutate_" + field.name +
                  " = function(value) {\n";
        }

        code += "  var offset = " + GenBBAccess() + ".__offset(this.bb_pos, " +
                NumToString(field.value.offset) + ");\n\n";
        code += "  if (offset === 0) {\n";
        code += "    return false;\n";
        code += "  }\n\n";

        // special case for bools, which are treated as uint8
        code += "  " + GenBBAccess() + ".write" +
                MakeCamel(GenType(field.value.type)) +
                "(this.bb_pos + offset, ";
        if (field.value.type.base_type == BASE_TYPE_BOOL &&
            lang_.language == IDLOptions::kTs) {
          code += "+";
        }

        code += "value);\n";
        code += "  return true;\n";
        code += "};\n\n";

        if (parser_.opts.use_goog_js_export_format) {
          exports += "goog.exportProperty(" + object_name +
                     ".prototype, 'mutate_" + field.name + "', " + object_name +
                     ".prototype.mutate_" + field.name + ");\n";
        }
      }

      // Emit vector helpers
      if (field.value.type.base_type == BASE_TYPE_VECTOR) {
        // Emit a length helper
        GenDocComment(code_ptr,
                      GenTypeAnnotation(kReturns, "number", "", false));
        if (lang_.language == IDLOptions::kTs) {
          code += MakeCamel(field.name, false);
          code += "Length():number {\n" + offset_prefix;
        } else {
          code += object_name + ".prototype." + MakeCamel(field.name, false);
          code += "Length = function() {\n" + offset_prefix;
        }

        code +=
            GenBBAccess() + ".__vector_len(this.bb_pos + offset) : 0;\n};\n\n";

        if (parser_.opts.use_goog_js_export_format) {
          exports += "goog.exportProperty(" + object_name + ".prototype, '" +
                     MakeCamel(field.name, false) + "Length', " + object_name +
                     ".prototype." + MakeCamel(field.name, false) +
                     "Length);\n";
        }

        // For scalar types, emit a typed array helper
        auto vectorType = field.value.type.VectorType();
        if (IsScalar(vectorType.base_type) && !IsLong(vectorType.base_type)) {
          GenDocComment(code_ptr, GenTypeAnnotation(
                                      kReturns, GenType(vectorType) + "Array",
                                      "", false));

          if (lang_.language == IDLOptions::kTs) {
            code += MakeCamel(field.name, false);
            code += "Array():" + GenType(vectorType) + "Array|null {\n" +
                    offset_prefix;
          } else {
            code += object_name + ".prototype." + MakeCamel(field.name, false);
            code += "Array = function() {\n" + offset_prefix;
          }

          code += "new " + GenType(vectorType) + "Array(" + GenBBAccess() +
                  ".bytes().buffer, " + GenBBAccess() +
                  ".bytes().byteOffset + " + GenBBAccess() +
                  ".__vector(this.bb_pos + offset), " + GenBBAccess() +
                  ".__vector_len(this.bb_pos + offset)) : null;\n};\n\n";

          if (parser_.opts.use_goog_js_export_format) {
            exports += "goog.exportProperty(" + object_name + ".prototype, '" +
                       MakeCamel(field.name, false) + "Array', " + object_name +
                       ".prototype." + MakeCamel(field.name, false) +
                       "Array);\n";
          }
        }
      }
    }

    // Emit a factory constructor
    if (struct_def.fixed) {
      std::string annotations =
          GenTypeAnnotation(kParam, "flatbuffers.Builder", "builder");
      std::string arguments;
      GenStructArgs(struct_def, &annotations, &arguments, "");
      GenDocComment(code_ptr, annotations + GenTypeAnnotation(
                                                kReturns, "flatbuffers.Offset",
                                                "", false));

      if (lang_.language == IDLOptions::kTs) {
        code += "static create" + Verbose(struct_def) +
                "(builder:flatbuffers.Builder";
        code += arguments + "):flatbuffers.Offset {\n";
      } else {
        code += object_name + ".create" + Verbose(struct_def);
        code += " = function(builder";
        code += arguments + ") {\n";
      }

      GenStructBody(struct_def, &code, "");
      code += "  return builder.offset();\n};\n\n";
    } else {
      // Generate a method to start building a new object
      GenDocComment(code_ptr, GenTypeAnnotation(kParam, "flatbuffers.Builder",
                                                "builder", false));

      if (lang_.language == IDLOptions::kTs) {
        code += "static start" + Verbose(struct_def) +
                "(builder:flatbuffers.Builder) {\n";
      } else {
        code += object_name + ".start" + Verbose(struct_def);
        code += " = function(builder) {\n";
      }

      code += "  builder.startObject(" +
              NumToString(struct_def.fields.vec.size()) + ");\n";
      code += "};\n\n";

      // Generate a set of static methods that allow table construction
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        const auto argname = GetArgName(field);

        // Generate the field insertion method
        GenDocComment(
            code_ptr,
            GenTypeAnnotation(kParam, "flatbuffers.Builder", "builder") +
                GenTypeAnnotation(kParam, GenTypeName(field.value.type, true),
                                  argname, false));

        if (lang_.language == IDLOptions::kTs) {
          code += "static add" + MakeCamel(field.name);
          code += "(builder:flatbuffers.Builder, " + argname + ":" +
                  GetArgType(field) + ") {\n";
        } else {
          code += object_name + ".add" + MakeCamel(field.name);
          code += " = function(builder, " + argname + ") {\n";
        }

        code += "  builder.addField" + GenWriteMethod(field.value.type) + "(";
        code += NumToString(it - struct_def.fields.vec.begin()) + ", ";
        if (field.value.type.base_type == BASE_TYPE_BOOL) { code += "+"; }
        code += argname + ", ";
        if (!IsScalar(field.value.type.base_type)) {
          code += "0";
        } else {
          if (field.value.type.base_type == BASE_TYPE_BOOL) { code += "+"; }
          code += GenDefaultValue(field.value, "builder");
        }
        code += ");\n};\n\n";

        if (field.value.type.base_type == BASE_TYPE_VECTOR) {
          auto vector_type = field.value.type.VectorType();
          auto alignment = InlineAlignment(vector_type);
          auto elem_size = InlineSize(vector_type);

          // Generate a method to create a vector from a JavaScript array
          if (!IsStruct(vector_type)) {
            GenDocComment(
                code_ptr,
                GenTypeAnnotation(kParam, "flatbuffers.Builder", "builder") +
                    GenTypeAnnotation(
                        kParam,
                        "Array.<" + GenTypeName(vector_type, true) + ">",
                        "data") +
                    GenTypeAnnotation(kReturns, "flatbuffers.Offset", "",
                                      false));

            if (lang_.language == IDLOptions::kTs) {
              code += "static create" + MakeCamel(field.name);
              std::string type = GenTypeName(vector_type, true) + "[]";
              if (type == "number[]") { type += " | Uint8Array"; }
              code += "Vector(builder:flatbuffers.Builder, data:" + type +
                      "):flatbuffers.Offset {\n";
            } else {
              code += object_name + ".create" + MakeCamel(field.name);
              code += "Vector = function(builder, data) {\n";
            }

            code += "  builder.startVector(" + NumToString(elem_size);
            code += ", data.length, " + NumToString(alignment) + ");\n";
            code += "  for (var i = data.length - 1; i >= 0; i--) {\n";
            code += "    builder.add" + GenWriteMethod(vector_type) + "(";
            if (vector_type.base_type == BASE_TYPE_BOOL) { code += "+"; }
            code += "data[i]);\n";
            code += "  }\n";
            code += "  return builder.endVector();\n";
            code += "};\n\n";
          }

          // Generate a method to start a vector, data to be added manually
          // after
          GenDocComment(
              code_ptr,
              GenTypeAnnotation(kParam, "flatbuffers.Builder", "builder") +
                  GenTypeAnnotation(kParam, "number", "numElems", false));

          if (lang_.language == IDLOptions::kTs) {
            code += "static start" + MakeCamel(field.name);
            code += "Vector(builder:flatbuffers.Builder, numElems:number) {\n";
          } else {
            code += object_name + ".start" + MakeCamel(field.name);
            code += "Vector = function(builder, numElems) {\n";
          }

          code += "  builder.startVector(" + NumToString(elem_size);
          code += ", numElems, " + NumToString(alignment) + ");\n";
          code += "};\n\n";
        }
      }

      // Generate a method to stop building a new object
      GenDocComment(
          code_ptr,
          GenTypeAnnotation(kParam, "flatbuffers.Builder", "builder") +
              GenTypeAnnotation(kReturns, "flatbuffers.Offset", "", false));

      if (lang_.language == IDLOptions::kTs) {
        code += "static end" + Verbose(struct_def);
        code += "(builder:flatbuffers.Builder):flatbuffers.Offset {\n";
      } else {
        code += object_name + ".end" + Verbose(struct_def);
        code += " = function(builder) {\n";
      }

      code += "  var offset = builder.endObject();\n";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (!field.deprecated && field.required) {
          code += "  builder.requiredField(offset, ";
          code += NumToString(field.value.offset);
          code += "); // " + field.name + "\n";
        }
      }
      code += "  return offset;\n";
      code += "};\n\n";

      // Generate the methods to complete buffer construction
      GenerateFinisher(struct_def, code_ptr, code, object_name, false);
      GenerateFinisher(struct_def, code_ptr, code, object_name, true);

      // Generate a convenient CreateX function
      if (lang_.language == IDLOptions::kJs) {
        std::string paramDoc =
            GenTypeAnnotation(kParam, "flatbuffers.Builder", "builder");
        for (auto it = struct_def.fields.vec.begin();
             it != struct_def.fields.vec.end(); ++it) {
          const auto &field = **it;
          if (field.deprecated) continue;
          paramDoc +=
              GenTypeAnnotation(kParam, GetArgType(field), GetArgName(field));
        }
        paramDoc +=
            GenTypeAnnotation(kReturns, "flatbuffers.Offset", "", false);

        GenDocComment(code_ptr, paramDoc);
      }

      if (lang_.language == IDLOptions::kTs) {
        code += "static create" + Verbose(struct_def);
        code += "(builder:flatbuffers.Builder";
      } else {
        code += object_name + ".create" + Verbose(struct_def);
        code += " = function(builder";
      }
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        const auto &field = **it;
        if (field.deprecated) continue;

        if (lang_.language == IDLOptions::kTs) {
          code += ", " + GetArgName(field) + ":" + GetArgType(field);
        } else {
          code += ", " + GetArgName(field);
        }
      }

      if (lang_.language == IDLOptions::kTs) {
        code += "):flatbuffers.Offset {\n";
        code += "  " + struct_def.name + ".start" + Verbose(struct_def) +
                "(builder);\n";
      } else {
        code += ") {\n";
        code += "  " + object_name + ".start" + Verbose(struct_def) +
                "(builder);\n";
      }

      std::string methodPrefix =
          lang_.language == IDLOptions::kTs ? struct_def.name : object_name;
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        const auto &field = **it;
        if (field.deprecated) continue;

        code += "  " + methodPrefix + ".add" + MakeCamel(field.name) + "(";
        code += "builder, " + GetArgName(field) + ");\n";
      }

      code += "  return " + methodPrefix + ".end" + Verbose(struct_def) +
              "(builder);\n";
      code += "}\n";
      if (lang_.language == IDLOptions::kJs) code += "\n";
    }

    if (lang_.language == IDLOptions::kTs) {
      if (parser_.opts.generate_object_based_api) {
        std::string obj_api_class;
        std::string obj_api_unpack_func;
        GenObjApi(parser_, struct_def, obj_api_unpack_func, obj_api_class);

        code += obj_api_unpack_func + "}\n" + obj_api_class;
      } else {
        code += "}\n";
      }
      if (!object_namespace.empty()) { code += "}\n"; }
    }
  }

  std::string GetArgType(const FieldDef &field) {
    if (field.value.type.enum_def)
      return GenPrefixedTypeName(GenTypeName(field.value.type, true),
                                 field.value.type.enum_def->file);
    return GenTypeName(field.value.type, true);
  }

  static std::string GetArgName(const FieldDef &field) {
    auto argname = MakeCamel(field.name, false);
    if (!IsScalar(field.value.type.base_type)) { argname += "Offset"; }

    return argname;
  }

  std::string Verbose(const StructDef &struct_def, const char *prefix = "") {
    return parser_.opts.js_ts_short_names ? "" : prefix + struct_def.name;
  }
};  // namespace jsts
}  // namespace jsts

bool GenerateJSTS(const Parser &parser, const std::string &path,
                  const std::string &file_name) {
  jsts::JsTsGenerator generator(parser, path, file_name);
  return generator.generate();
}

std::string JSTSMakeRule(const Parser &parser, const std::string &path,
                         const std::string &file_name) {
  FLATBUFFERS_ASSERT(parser.opts.lang <= IDLOptions::kMAX);

  std::string filebase =
      flatbuffers::StripPath(flatbuffers::StripExtension(file_name));
  jsts::JsTsGenerator generator(parser, path, file_name);
  std::string make_rule =
      generator.GeneratedFileName(path, filebase, parser.opts) + ": ";

  auto included_files = parser.GetIncludedFilesRecursive(file_name);
  for (auto it = included_files.begin(); it != included_files.end(); ++it) {
    make_rule += " " + *it;
  }
  return make_rule;
}

}  // namespace flatbuffers
