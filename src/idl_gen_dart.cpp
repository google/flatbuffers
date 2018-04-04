/*
 * Copyright 2018 Dan Field
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

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

static std::string GeneratedFileName(const std::string &path,
                                     const std::string &file_name) {
  return path + file_name + "_generated.dart";
}

namespace dart {

const std::string _kFb = "fb";
// see https://www.dartlang.org/guides/language/language-tour#keywords
// yeild*, async*, and sync* shouldn't be problems anyway but keeping them in
static const char *keywords[] = {
  "abstract",   "deferred", "if",       "super",   "as",       "do",
  "implements", "switch",   "assert",   "dynamic", "import",   "sync*",
  "async",      "else",     "in",       "this",    "async*",   "enum",
  "is",         "throw",    "await",    "export",  "library",  "true",
  "break",      "external", "new",      "try",     "case",     "extends",
  "null",       "typedef",  "catch",    "factory", "operator", "var",
  "class",      "false",    "part",     "void",    "const",    "final",
  "rethrow",    "while",    "continue", "finally", "return",   "with",
  "covariant",  "for",      "set",      "yield",   "default",  "get",
  "static",     "yield*"
};

// Iterate through all definitions we haven't generate code for (enums, structs,
// and tables) and output them to a single file.
class DartGenerator : public BaseGenerator {
 public:
  typedef std::unordered_map<std::string, std::string> namespace_code_map;

  DartGenerator(const Parser &parser, const std::string &path,
                const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", "."){};
  // Iterate through all definitions we haven't generate code for (enums,
  // structs, and tables) and output them to a single file.
  bool generate() {
    // std::string enum_code, struct_code, import_code, code;
    std::string code;
    namespace_code_map namespace_code;
    generateEnums(&namespace_code);
    generateStructs(&namespace_code);

    for (auto kv = namespace_code.begin(); kv != namespace_code.end(); ++kv) {
      code.clear();
      code = code + "// " + FlatBuffersGeneratedWarning() + "\n";
      code = code +
             "// ignore_for_file: unused_import, "
             "non_constant_identifier_names\n\n";

      code += "library " + kv->first + ";\n\n";

      code += "import 'dart:typed_data' show Uint8List;\n";
      // code += "import 'package:flat_buffers/flat_buffers.dart' as " + _kFb +
      // ";\n\n";
      code += "import './flat_buffers.dart' as " + _kFb + ";\n\n";

      for (auto kv2 = namespace_code.begin(); kv2 != namespace_code.end();
           ++kv2) {
        if (kv2->first != kv->first) {
          code += "import '" +
                  GeneratedFileName("./", file_name_ + "_" + kv2->first) +
                  "' as " + ImportAliasName(kv2->first) + ";\n";
        }
      }
      code += "\n";
      code += kv->second;

      if (!SaveFile(
              GeneratedFileName(path_, file_name_ + "_" + kv->first).c_str(),
              code, false)) {
        return false;
      }
    }
    return true;
  }

 private:
  static std::string ImportAliasName(const std::string &ns) {
    std::string ret;
    ret.assign(ns);
    size_t pos = ret.find(".");
    while (pos != std::string::npos) {
      ret.replace(pos, 1, "_");
      pos = ret.find(".", pos + 1);
    }

    return ret;
  }

  static std::string BuildNamespaceName(const Namespace &ns) {
    std::stringstream sstream;
    std::copy(ns.components.begin(), ns.components.end() - 1,
              std::ostream_iterator<std::string>(sstream, "."));

    std::string ret = sstream.str() + ns.components.back();
    for (int i = 0; ret[i]; i++) {
      auto lower = tolower(ret[i]);
      if (lower != ret[i]) {
        ret[i] = static_cast<char>(lower);
        if (i != 0 && ret[i - 1] != '.') {
          ret.insert(i, "_");
          i++;
        }
      }
    }
    // std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
    return ret;
  }
  static std::string EscapeKeyword(const std::string &name) {
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
      if (name == keywords[i]) { return MakeCamel(name + "_", false); }
    }

    return MakeCamel(name, false);
  }

  // Generate code for all enums.
  void generateEnums(
      namespace_code_map *namespace_code) {  // std::string *enum_code_ptr) {
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      auto &enum_def = **it;
      GenEnum(enum_def, namespace_code);  // enum_code_ptr);
    }
  }

  // Generate code for all structs.
  void generateStructs(namespace_code_map *namespace_code) {
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      auto &struct_def = **it;
      GenStruct(struct_def, namespace_code);
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

    for (auto it = dc.begin(); it != dc.end(); ++it) {
      if (indent) code += indent;
      code += "/// " + *it + "\n";
    }
    if (!extra_lines.empty()) {
      if (!dc.empty()) {
        if (indent) code += indent;
        code += "///\n";
      }
      if (indent) code += indent;
      std::string::size_type start = 0;
      for (;;) {
        auto end = extra_lines.find('\n', start);
        if (end != std::string::npos) {
          code += "/// " + extra_lines.substr(start, end - start) + "\n";
          start = end + 1;
        } else {
          code += "/// " + extra_lines.substr(start) + "\n";
          break;
        }
      }
    }
  }

  static void GenDocComment(std::string *code_ptr,
                            const std::string &extra_lines) {
    GenDocComment(std::vector<std::string>(), code_ptr, extra_lines);
  }

  // Generate an enum declaration and an enum string lookup table.
  void GenEnum(EnumDef &enum_def, namespace_code_map *namespace_code) {
    if (enum_def.generated) return;
    std::string ns = BuildNamespaceName(*enum_def.defined_namespace);
    std::string code;
    GenDocComment(enum_def.doc_comment, &code, "");

    std::string name =
        enum_def.is_union ? enum_def.name + "TypeId" : enum_def.name;

    code += "class " + name + " {\n";
    code += "  final int value;\n";
    code += "  const " + name + "._(this.value);\n\n";
    code += "  factory " + name + ".fromValue(int value) {\n";
    code += "    if (value < minValue || maxValue < value) {\n";
    code += "      throw new RangeError.range(value, minValue, maxValue);\n";
    code += "    }\n";
    code += "    return values[value - minValue];\n";
    code += "  }\n\n";

    code += "  static const int minValue = " +
            NumToString(enum_def.vals.vec.front()->value) + ";\n";
    code += "  static const int maxValue = " +
            NumToString(enum_def.vals.vec.back()->value) + ";\n";

    code +=
        "  static bool containsValue(int value) => minValue <= value && "
        "value <= maxValue;\n\n";

    for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
         ++it) {
      auto &ev = **it;

      if (!ev.doc_comment.empty()) {
        if (it != enum_def.vals.vec.begin()) { code += '\n'; }
        GenDocComment(ev.doc_comment, &code, "", "  ");
      }
      code += "  static const " + ev.name + " = ";
      code += "const " + name + "._(" + NumToString(ev.value) + ");\n";
    }

    code += "  static get values => [";
    for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
         ++it) {
      auto &ev = **it;
      code += ev.name + ",";
    }
    code += "];\n\n";
    code += "  static const " + _kFb + ".Reader<" + enum_def.name +
            "> reader = const _" + enum_def.name + "Reader();\n";
    code += "}\n\n";

    GenEnumReader(enum_def, &code);

    (*namespace_code)[ns] += code;
  }

  void GenEnumReader(EnumDef &enum_def, std::string *code_ptr) {
    std::string &code = *code_ptr;

    code += "class _" + enum_def.name + "Reader extends " + _kFb + ".Reader<" +
            enum_def.name + "> {\n";
    code += "  const _" + enum_def.name + "Reader();\n\n";
    code += "  @override\n";
    code += "  int get size => 1;\n\n";
    code += "  @override\n";
    code += "  " + enum_def.name + " read(" + _kFb +
            ".BufferContext bc, int offset) =>\n";
    code += "      new " + enum_def.name + ".fromValue(const " + _kFb + "." +
            GenType(enum_def.underlying_type) + "Reader().read(bc, offset));\n";
    code += "}\n\n";
  }

  static std::string GenType(const Type &type) {
    switch (type.base_type) {
      case BASE_TYPE_BOOL: return "Bool";
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
      case BASE_TYPE_UNION: return type.enum_def->name + "TypeId";
      default: return "Table";
    }
  }

  std::string GenReaderTypeName(const Type &type, Namespace *current_namespace,
                                const FieldDef &def,
                                bool parent_is_vector = false) {
    if (type.base_type == BASE_TYPE_BOOL) {
      return "const " + _kFb + ".BoolReader()";
    } else if (type.base_type == BASE_TYPE_VECTOR) {
      return "const " + _kFb + ".ListReader<" +
             GenDartTypeName(type.VectorType(), current_namespace, def) + ">(" +
             GenReaderTypeName(type.VectorType(), current_namespace, def,
                               true) +
             ")";
    } else if (type.base_type == BASE_TYPE_STRING) {
      return "const " + _kFb + ".StringReader()";
    }
    if (IsScalar(type.base_type)) {
      if (type.enum_def && parent_is_vector) {
        return GenDartTypeName(type, current_namespace, def) + ".reader";
      }
      return "const " + _kFb + "." + GenType(type) + "Reader()";
    } else {
      return GenDartTypeName(type, current_namespace, def) + ".reader";
    }
  }

  std::string GenDartTypeName(const Type &type, Namespace *current_namespace,
                              const FieldDef &def, bool addBuilder = false) {
    if (type.enum_def) {
      if (type.enum_def->is_union && type.base_type != BASE_TYPE_UNION) {
        return type.enum_def->name + "TypeId";
      } else if (type.enum_def->is_union) {
        return "dynamic";
      } else if (type.base_type != BASE_TYPE_VECTOR) {
        return type.enum_def->name;
      }
    }

    switch (type.base_type) {
      case BASE_TYPE_BOOL: return "bool";
      case BASE_TYPE_LONG:
      case BASE_TYPE_ULONG:
      case BASE_TYPE_INT:
      case BASE_TYPE_UINT:
      case BASE_TYPE_SHORT:
      case BASE_TYPE_USHORT:
      case BASE_TYPE_CHAR:
      case BASE_TYPE_UCHAR: return "int";
      case BASE_TYPE_FLOAT:
      case BASE_TYPE_DOUBLE: return "double";
      case BASE_TYPE_STRING: return "String";
      case BASE_TYPE_STRUCT:
        return MaybeWrapNamespace(
            type.struct_def->name + (addBuilder ? "Builder" : ""),
            current_namespace, def);
      case BASE_TYPE_VECTOR:
        return "List<" +
               GenDartTypeName(type.VectorType(), current_namespace, def,
                               addBuilder) +
               ">";
      default: assert(0); return "dynamic";
    }
  }

  static const std::string MaybeWrapNamespace(const std::string &type_name,
                                              Namespace *current_ns,
                                              const FieldDef &field) {
    std::string curr_ns_str = BuildNamespaceName(*current_ns);
    std::string field_ns_str = "";
    if (field.value.type.struct_def) {
      field_ns_str +=
          BuildNamespaceName(*field.value.type.struct_def->defined_namespace);
    } else if (field.value.type.enum_def) {
      field_ns_str +=
          BuildNamespaceName(*field.value.type.enum_def->defined_namespace);
    }

    if (field_ns_str != "" && field_ns_str != curr_ns_str) {
      return ImportAliasName(field_ns_str) + "." + type_name;
    } else {
      return type_name;
    }
  }

  // Generate an accessor struct with constructor for a flatbuffers struct.
  void GenStruct(const StructDef &struct_def,
                 namespace_code_map *namespace_code) {
    if (struct_def.generated) return;

    std::string object_namespace =
        BuildNamespaceName(*struct_def.defined_namespace);
    std::string code;

    std::string object_name;

    // Emit constructor

    object_name = struct_def.name;
    GenDocComment(struct_def.doc_comment, &code, object_name);

    std::string impl_name = "_" + struct_def.name + "Impl";
    std::string reader_name = "_" + struct_def.name + "Reader";
    std::string builder_name = struct_def.name + "Builder";

    std::string impl_code, reader_code, builder_code;

    // abstract base class
    code += "abstract class " + struct_def.name + " {\n";

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      GenDocComment(field.doc_comment, &code, "");

      code += "  " +
              GenDartTypeName(field.value.type, struct_def.defined_namespace,
                              field, false) +
              " get " + MakeCamel(field.name, false) + ";\n";
    }

    code += "\n";

    code += "  factory " + struct_def.name + "(List<int> bytes) {\n";
    code += "    " + _kFb + ".BufferContext rootRef = new " + _kFb +
            ".BufferContext.fromBytes(bytes);\n";
    code += "    return reader.read(rootRef, 0);\n";
    code += "  }\n\n";
    code += "  static const " + _kFb + ".Reader<" + struct_def.name +
            "> reader = const " + reader_name + "();\n\n";
    code += "}\n\n";

    GenImplementation(struct_def, &impl_name, &impl_code);
    GenReader(struct_def, &reader_name, &impl_name, &reader_code);
    GenBuilder(struct_def, &builder_name, &builder_code);

    code += impl_code;
    code += reader_code;
    code += builder_code;

    (*namespace_code)[object_namespace] += code;
  }

  void GenImplementation(const StructDef &struct_def,
                         std::string *impl_name_ptr, std::string *code_ptr) {
    std::string &code = *code_ptr;
    std::string &impl_name = *impl_name_ptr;

    code += "class " + impl_name + " implements " + struct_def.name + " {\n";
    code += "  final " + _kFb + ".BufferContext _bc;\n";
    code += "  final int _bcOffset;\n\n";
    code += "  " + impl_name + "(this._bc, this._bcOffset);\n\n";

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      auto offset = it - struct_def.fields.vec.begin();

      std::string field_name = MakeCamel(field.name, false);
      std::string type_name = GenDartTypeName(
          field.value.type, struct_def.defined_namespace, field, false);

      code += "  " + type_name + " _" + field_name + ";\n";
      code += "  @override\n";
      code += "  " + type_name + " get " + field_name + " {\n";
      if (field.value.type.base_type == BASE_TYPE_UNION) {
        code += "    switch (" + field_name + "?.value) {\n";
        for (auto en_it = field.value.type.enum_def->vals.vec.begin() + 1;
             en_it != field.value.type.enum_def->vals.vec.end(); ++en_it) {
          auto &ev = **en_it;
          code += "      case " + NumToString(ev.value) + ": return " +
                  ev.name + ".reader.vTableGet(_bc, _bcOffset, " +
                  NumToString(offset) + ", null);\n";
        }
        code += "      default: return null;\n";
        code += "    }\n";
      } else {
        code += "    _" + field_name + " \?\?= ";
        if (field.value.type.enum_def &&
            field.value.type.base_type != BASE_TYPE_VECTOR) {
          code += "new " +
                  GenDartTypeName(field.value.type,
                                  struct_def.defined_namespace, field) +
                  ".fromValue(";
        }
        if (!struct_def.fixed) { code += "\n        "; }

        code += GenReaderTypeName(field.value.type,
                                  struct_def.defined_namespace, field);
        if (struct_def.fixed) {
          code +=
              ".read(_bc, _bcOffset + " + NumToString(field.value.offset) + ")";
        } else {
          code +=
              ".vTableGet(_bc, _bcOffset, " + NumToString(offset) + ", null)";
        }
        if (field.value.type.enum_def &&
            field.value.type.base_type != BASE_TYPE_VECTOR) {
          code += ")";
        }
        code += ";\n";
        code += "    return _" + field_name + ";\n";
      }
      code += "  }\n\n";
    }

    code += "}\n\n";  // end impl
  }

  void GenReader(const StructDef &struct_def, std::string *reader_name_ptr,
                 std::string *impl_name_ptr, std::string *code_ptr) {
    std::string &code = *code_ptr;
    std::string &reader_name = *reader_name_ptr;
    std::string &impl_name = *impl_name_ptr;

    code += "class " + reader_name + " extends " + _kFb;
    if (struct_def.fixed) {
      code += ".StructReader<";
    } else {
      code += ".TableReader<";
    }
    code += impl_name + "> {\n";
    code += "  const " + reader_name + "();\n\n";
    code += "  @override\n";
    code += "  " + impl_name +
            " createObject(fb.BufferContext bc, int offset) => \n    new " +
            impl_name + "(bc, offset);\n";
    code += "}\n\n";
  }

  void GenBuilder(const StructDef &struct_def, std::string *builder_name_ptr,
                  std::string *code_ptr) {
    if (struct_def.fields.vec.size() == 0) { return; }
    std::string &code = *code_ptr;
    std::string &builder_name = *builder_name_ptr;

    code += "class " + builder_name + " {\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      code += "  final " +
              GenDartTypeName(field.value.type, struct_def.defined_namespace,
                              field, true) +
              " _" + field.name + ";\n";
    }
    code += "\n";
    code += "  " + builder_name + "({\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      code += "    " +
              GenDartTypeName(field.value.type, struct_def.defined_namespace,
                              field, true) +
              " " + field.name + ",\n";
    }
    code += "  })\n";
    code += "      : ";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      code += "_" + field.name + " = " + field.name;
      if (it == struct_def.fields.vec.end() - 1) {
        code += ";\n\n";
      } else {
        code += ",\n        ";
      }
    }

    code += "  /// Finish building, and store into the [fbBuilder].\n";
    code += "  int finish(" + _kFb + ".Builder fbBuilder) {\n";
    code += "    assert(fbBuilder != null);\n";

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      if (IsScalar(field.value.type.base_type)) continue;

      code += "    final int offset_" + field.name;
      if (field.value.type.base_type == BASE_TYPE_VECTOR) {
        code += " = _" + field.name + "?.isNotEmpty == true\n";
        code += "        ? fbBuilder.writeList";
        switch (field.value.type.VectorType().base_type) {
          case BASE_TYPE_STRING:
            code += "(_" + field.name +
                    ".map((b) => fbBuilder.writeString(b)).toList())";
            break;
          case BASE_TYPE_STRUCT:
            code += "(_" + field.name +
                    ".map((b) => b.finish(fbBuilder)).toList())";
            break;
          default:
            code += GenType(field.value.type.VectorType()) + "(_" + field.name;
            if (field.value.type.enum_def) { code += ".map((f) => f.value)"; }
            code += ")";
        }
        code += "\n        : null;\n";
      } else if (field.value.type.base_type == BASE_TYPE_STRING) {
        code += " = fbBuilder.writeString(_" + field.name + ");\n";
      } else {
        code += " = _" + field.name + "?.finish(fbBuilder);\n";
      }
    }

    code += "\n";
    if (struct_def.fixed) {
      StructBuilderBody(struct_def, code_ptr);
    } else {
      TableBuilderBody(struct_def, code_ptr);
    }
    code += "  }\n\n";

    code += "  Uint8List toBytes() {\n";
    code += "    " + _kFb + ".Builder fbBuilder = new ";
    code += _kFb + ".Builder();\n";
    code += "    int offset = finish(fbBuilder);\n";
    code += "    return fbBuilder.finish(offset);\n";
    code += "  }\n";
    code += "}\n";
  }

  void StructBuilderBody(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;

    code += "    fbBuilder.prep(" + NumToString(struct_def.minalign) + ", ";
    code += NumToString(struct_def.bytesize) + ");\n";

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;

      if (field.deprecated) continue;

      if (field.padding) {
        code += "    fbBuilder.pad(" + NumToString(field.padding) + ");\n";
      }

      if (IsStruct(field.value.type)) {
        code += "    _" + field.name + ".finish(fbBuilder);\n";
      } else {
        code += "    fbBuilder.put" + GenType(field.value.type) + "(";
        code += "_" + field.name + ");\n";
      }
    }

    code += "    return fbBuilder.offset;\n";
  }

  void TableBuilderBody(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "    fbBuilder.startTable();\n";

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;

      if (field.deprecated) continue;

      auto offset = it - struct_def.fields.vec.begin();

      if (IsScalar(field.value.type.base_type)) {
        code += "    fbBuilder.add" + GenType(field.value.type) + "(" +
                NumToString(offset) + ", _" + field.name;
        if (field.value.type.enum_def) { code += "?.value"; }
        code += ");\n";
      } else {
        code += "    if (offset_" + field.name + " != null) {\n";
        code += "      fbBuilder.addOffset(" + NumToString(offset) +
                ", offset_" + field.name + ");\n";
        code += "    }\n";
      }
    }
    code += "    return fbBuilder.endTable();\n";
  }
};
}  // namespace dart

bool GenerateDart(const Parser &parser, const std::string &path,
                  const std::string &file_name) {
  dart::DartGenerator generator(parser, path, file_name);
  return generator.generate();
}

std::string DartMakeRule(const Parser &parser, const std::string &path,
                         const std::string &file_name) {
  assert(parser.opts.lang <= IDLOptions::kMAX);

  std::string filebase =
      flatbuffers::StripPath(flatbuffers::StripExtension(file_name));
  std::string make_rule = GeneratedFileName(path, filebase) + ": ";

  auto included_files = parser.GetIncludedFilesRecursive(file_name);
  for (auto it = included_files.begin(); it != included_files.end(); ++it) {
    make_rule += " " + *it;
  }
  return make_rule;
}

}  // namespace flatbuffers
