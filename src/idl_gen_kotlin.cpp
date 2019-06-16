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

#include <functional>
#include <unordered_set>
#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#if defined(FLATBUFFERS_CPP98_STL)
#  include <cctype>
#endif  // defined(FLATBUFFERS_CPP98_STL)

namespace flatbuffers {

namespace kotlin {

typedef std::map<std::string, std::pair<std::string, std::string> > FbbParamMap;
static TypedFloatConstantGenerator KotlinFloatGen("Double.", "Float.", "NaN",
                                                  "POSITIVE_INFINITY",
                                                  "NEGATIVE_INFINITY");

static const CommentConfig comment_config = { "/**", " *", " */" };
static const std::string ident_pad = "    ";
static const char *keywords[] = {
  "package",  "as",     "typealias", "class",  "this",   "super",
  "val",      "var",    "fun",       "for",    "null",   "true",
  "false",    "is",     "in",        "throw",  "return", "break",
  "continue", "object", "if",        "try",    "else",   "while",
  "do",       "when",   "interface", "typeof", "Any",    "Character"
};

// Escape Keywords
static std::string Esc(const std::string &name) {
  for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
    if (name == keywords[i]) { return MakeCamel(name + "_", false); }
  }

  return MakeCamel(name, false);
}

class KotlinGenerator : public BaseGenerator {
 public:
  KotlinGenerator(const Parser &parser, const std::string &path,
                  const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", "."),
        cur_name_space_(nullptr) {}

  KotlinGenerator &operator=(const KotlinGenerator &);
  bool generate() FLATBUFFERS_OVERRIDE {
    std::string one_file_code;

    cur_name_space_ = parser_.current_namespace_;
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      CodeWriter enumWriter(ident_pad);
      auto &enum_def = **it;
      if (!parser_.opts.one_file) cur_name_space_ = enum_def.defined_namespace;
      GenEnum(enum_def, enumWriter);
      if (parser_.opts.one_file) {
        one_file_code += enumWriter.ToString();
      } else {
        if (!SaveType(enum_def.name, *enum_def.defined_namespace,
                      enumWriter.ToString(), false))
          return false;
      }
    }

    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      CodeWriter structWriter(ident_pad);
      auto &struct_def = **it;
      if (!parser_.opts.one_file)
        cur_name_space_ = struct_def.defined_namespace;
      GenStruct(struct_def, structWriter);
      if (parser_.opts.one_file) {
        one_file_code += structWriter.ToString();
      } else {
        if (!SaveType(struct_def.name, *struct_def.defined_namespace,
                      structWriter.ToString(), true))
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

    std::string code =
        "// " + std::string(FlatBuffersGeneratedWarning()) + "\n\n";

    std::string namespace_name = FullNamespace(".", ns);
    if (!namespace_name.empty()) {
      code += "package " + namespace_name;
      code += "\n\n";
    }
    if (needs_includes) {
      code += "import java.nio.*\n";
      code += "import kotlin.math.sign\n";
      code += "import com.google.flatbuffers.*\n\n";
    }
    code += classcode;
    auto filename = NamespaceDir(ns) + defname + ".kt";
    return SaveFile(filename.c_str(), code, false);
  }

  const Namespace *CurrentNameSpace() const FLATBUFFERS_OVERRIDE {
    return cur_name_space_;
  }

  static bool IsEnum(const Type &type) {
    return type.enum_def != nullptr && IsInteger(type.base_type);
  }

  static std::string GenTypeBasic(const Type &type) {
    // clang-format off
        static const char * const kotlin_typename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, \
        CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, RTYPE, KTYPE) \
    #KTYPE,
        FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
        };
        return kotlin_typename[type.base_type];

    }

    std::string GenTypePointer(const Type &type) const {
        switch (type.base_type) {
        case BASE_TYPE_STRING:
            return "String";
        case BASE_TYPE_VECTOR:
            return GenTypeGet(type.VectorType());
        case BASE_TYPE_STRUCT:
            return WrapInNameSpace(*type.struct_def);
        default:
            return "Table";
        }
    }

    std::string GenTypeGet(const Type &type) const {
        return IsScalar(type.base_type) ? GenTypeBasic(type)
                                        : GenTypePointer(type);
    }

    // Find the destination type the user wants to receive the value in (e.g.
    // one size higher signed types for unsigned serialized values in Java).
    static Type DestinationType(const Type &type, bool vec_elem) {
        switch (type.base_type) {
        // We use int for both uchar/ushort, since that generally means less
        // casting than using short for uchar.
        case BASE_TYPE_UCHAR:
            return Type(BASE_TYPE_INT);
        case BASE_TYPE_USHORT:
            return Type(BASE_TYPE_INT);
        case BASE_TYPE_UINT:
            return Type(BASE_TYPE_LONG);
        case BASE_TYPE_VECTOR:
            if (vec_elem) return DestinationType(type.VectorType(), vec_elem);
            FLATBUFFERS_FALLTHROUGH();  // else fall thru
        default:
            return type;
        }
    }

    // Generate destination type name
    std::string GenTypeNameDest(const Type &type) const {
        return GenTypeGet(DestinationType(type, true));
    }

    // Mask to turn serialized value into destination type value.
    std::string DestinationMask(const Type &type, bool vec_elem) const {
        switch (type.base_type) {
        case BASE_TYPE_UCHAR:
            return ".and(0xFF)";
        case BASE_TYPE_USHORT:
            return ".and(0xFFFF)";
        case BASE_TYPE_UINT:
            return ".and(0xFFFFFFFFL)";
        case BASE_TYPE_VECTOR:
            if (vec_elem) return DestinationMask(type.VectorType(), vec_elem);
            FLATBUFFERS_FALLTHROUGH();  // else fall thru
        default:
            return "";
        }
    }

    // Casts necessary to correctly read serialized data
    std::string DestinationCast(const Type &type) const {
        if (type.base_type == BASE_TYPE_VECTOR) {
            return DestinationCast(type.VectorType());
        }
        if (type.base_type == BASE_TYPE_UINT) {
            return ".toLong()";
        }
        return "";
    }

    // Cast statements for mutator method parameters.
    // In Java, parameters representing unsigned numbers need to be cast down to
    // their respective type. For example, a long holding an unsigned int value
    // would be cast down to int before being put onto the buffer.
    static std::string SourceCast(const Type &type, bool castFromDest) {
        if (type.base_type == BASE_TYPE_VECTOR) {
            return SourceCast(type.VectorType(), castFromDest);
        }
        if (castFromDest) {
            if (type.base_type == BASE_TYPE_UINT)
                return ".toInt()";
            if (type.base_type == BASE_TYPE_USHORT)
                return ".toShort()";
            if (type.base_type == BASE_TYPE_UCHAR)
                return ".toByte()";
        }
        return "";
    }

    static std::string SourceCast(const Type &type) {
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
        case BASE_TYPE_BOOL:
            return value.constant == "0" ? "false" : "true";
        case BASE_TYPE_ULONG: {
            // Converts the ulong into its bits signed equivalent
            uint64_t defaultValue = StringToUInt(value.constant.c_str());
            return NumToString(static_cast<int64_t>(defaultValue)) + longSuffix;
        }
        case BASE_TYPE_UINT:
        case BASE_TYPE_LONG:
            return value.constant + longSuffix;
        default:
            if (IsFloat(value.type.base_type)) {
                auto val = KotlinFloatGen.GenFloatConstant(field);
                if (value.type.base_type == BASE_TYPE_DOUBLE &&
                        val.back() == 'f') {
                    val.pop_back();
                }
                return val;
            } else {
                return value.constant;
            }
        }
    }

    std::string GenDefaultValueBasic(const FieldDef &field) const {
        auto &value = field.value;
        if (!IsScalar(value.type.base_type)) {
            return "0";
        }
        return GenDefaultValue(field);
    }

    void GenEnum(EnumDef &enum_def, CodeWriter &writer) const {
        if (enum_def.generated) return;

        GenerateComment(enum_def.doc_comment, writer, &comment_config);

        writer += "@Suppress(\"unused\")";
        writer += "class " + Esc(enum_def.name) + " private constructor() {";
        writer.IncrementIdentLevel();

        GenerateCompanionObject(writer, [&](){
            // Write all properties
            auto vals = enum_def.Vals();
            for (auto it = vals.begin(); it != vals.end(); ++it) {
                auto &ev = **it;
                writer.SetValue("name", Esc(ev.name));
                writer.SetValue("type", GenTypeBasic(enum_def.underlying_type));
                writer.SetValue("val", enum_def.ToString(ev));
                GenerateComment(ev.doc_comment, writer, &comment_config);
                writer += "const val {{name}}: {{type}} = {{val}}";
            }

            // Generate a generate string table for enum values.
            // Problem is, if values are very sparse that could generate really
            // big tables. Ideally in that case we generate a map lookup
            // instead, but for the moment we simply don't output a table at all.
            auto range = enum_def.Distance();
            // Average distance between values above which we consider a table
            // "too sparse". Change at will.
            static const uint64_t kMaxSparseness = 5;
            if (range / static_cast<uint64_t>(enum_def.size()) < kMaxSparseness) {
                GeneratePropertyOneLine(writer, "names", "Array<String>",
                               [&](){
                    writer += "arrayOf(\\";
                    auto val = enum_def.Vals().front();
                    for (auto it = vals.begin(); it != vals.end(); ++it) {
                        auto ev = *it;
                        for (auto k = enum_def.Distance(val, ev); k > 1; --k)
                            writer += "\"\", \\";
                        val = ev;
                        writer += "\"" + (*it)->name + "\"\\";
                        if (it+1 != vals.end()) {
                            writer += ", \\";
                        }
                    }
                    writer += ")";
                });
                GenerateFunOneLine(writer, "name", "e: Int", "String", [&](){
                    writer += "names[e\\";
                    if (enum_def.MinValue()->IsNonZero())
                        writer += " - " + enum_def.MinValue()->name + "\\";
                    writer += "]";
                });
            }
        });
        writer.DecrementIdentLevel();
        writer += "}";
    }

    // Returns the function name that is able to read a value of the given type.
    std::string GenGetter(const Type &type) const {
        switch (type.base_type) {
        case BASE_TYPE_STRING:
            return "__string";
        case BASE_TYPE_STRUCT:
            return "__struct";
        case BASE_TYPE_UNION:
            return "__union";
        case BASE_TYPE_VECTOR:
            return GenGetter(type.VectorType());
        default: {
            std::string getter = "bb.get";
            if (GenTypeBasic(type) != "Byte" &&
                    GenTypeBasic(type) != "Boolean") {
                getter += MakeCamel(GenTypeBasic(type));
            }
            return getter;
        }
        }
    }

    // Returns the function name that is able to read a value of the given type.
    std::string GenLookupByKey(flatbuffers::FieldDef *key_field,
                                        const std::string &data_buffer,
                                        const char *num = nullptr) const {
        auto type = key_field->value.type;
        auto dest_mask = DestinationMask(type, true);
        auto dest_cast = DestinationCast(type);
        auto getter = data_buffer + ".get";
        if (GenTypeBasic(type) != "Byte") {
            getter += MakeCamel(GenTypeBasic(type));
        }
        getter = getter + "(" + GenOffsetGetter(key_field, num) + ")" +
                dest_cast + dest_mask;
        return getter;
    }

    // Direct mutation is only allowed for scalar fields.
    // Hence a setter method will only be generated for such fields.
    std::string GenSetter(const Type &type) const {
        if (IsScalar(type.base_type)) {
            std::string setter = "bb.put";
            if (GenTypeBasic(type) != "Byte" &&
                    type.base_type != BASE_TYPE_BOOL) {
                setter += MakeCamel(GenTypeBasic(type));
            }
            return setter;
        }
        return "";
    }

    // Returns the method name for use with add/put calls.
    static std::string GenMethod(const Type &type) {
        return IsScalar(type.base_type) ? MakeCamel(GenTypeBasic(type))
                                        : (IsStruct(type) ? "Struct" : "Offset");
    }

    // Recursively generate arguments for a constructor, to deal with nested
    // structs.
    static void GenStructArgs(const StructDef &struct_def, CodeWriter &writer,
                              const char *nameprefix) {
        for (auto it = struct_def.fields.vec.begin();
             it != struct_def.fields.vec.end(); ++it) {
            auto &field = **it;
            if (IsStruct(field.value.type)) {
                // Generate arguments for a struct inside a struct. To ensure
                // names don't clash, and to make it obvious these arguments are
                // constructing a nested struct, prefix the name with the field
                // name.
                GenStructArgs(*field.value.type.struct_def, writer,
                              (nameprefix + (field.name + "_")).c_str());
            } else {
                writer += std::string(", ") + nameprefix + "\\";
                writer += MakeCamel(field.name) + ": \\";
                writer += GenTypeBasic(DestinationType(field.value.type, false))
                        + "\\";
            }
        }
    }

    // Recusively generate struct construction statements of the form:
    // builder.putType(name);
    // and insert manual padding.
    static void GenStructBody(const StructDef &struct_def, CodeWriter &writer,
                              const char *nameprefix) {
        writer.SetValue("align", NumToString(struct_def.minalign));
        writer.SetValue("size", NumToString(struct_def.bytesize));
        writer += "builder.prep({{align}}, {{size}})";
        auto fields_vec = struct_def.fields.vec;
        for (auto it = fields_vec.rbegin(); it != fields_vec.rend(); ++it) {
            auto &field = **it;

            if (field.padding) {
                writer.SetValue("pad", NumToString(field.padding));
                writer += "builder.pad({{pad}})";
            }
            if (IsStruct(field.value.type)) {
                GenStructBody(*field.value.type.struct_def, writer,
                              (nameprefix + (field.name + "_")).c_str());
            } else {
                writer.SetValue("type", GenMethod(field.value.type));
                writer.SetValue("cast", SourceCast(field.value.type));
                writer.SetValue("argname", nameprefix +
                              MakeCamel(field.name, false));
                writer += "builder.put{{type}}({{argname}}{{cast}})";
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
        std::string key_offset = "__offset(" +
                NumToString(key_field->value.offset) + ", ";
        if (num) {
            key_offset += num;
            key_offset += ", _bb)";
        } else {
            key_offset += GenByteBufferLength("bb");
            key_offset += " - tableOffset, bb)";
        }
        return key_offset;
    }

    void GenStruct(StructDef &struct_def, CodeWriter &writer) const {
        if (struct_def.generated) return;

        GenerateComment(struct_def.doc_comment, writer, &comment_config);
        auto fixed = struct_def.fixed;

        writer.SetValue("struct_name", Esc(struct_def.name));
        writer.SetValue("superclass", fixed ? "Struct" : "Table");

        writer += "@Suppress(\"unused\")";
        writer += "class {{struct_name}} : {{superclass}}() {\n";

        writer.IncrementIdentLevel();

        {
            // Generate the __init() method that sets the field in a pre-existing
            // accessor object. This is to allow object reuse.
            GenerateFun(writer, "__init", "_i: Int, _bb: ByteBuffer", "", [&]() {
                writer += "__reset(_i, _bb)";
            });

            // Generate assign method
            GenerateFun(writer, "__assign", "_i: Int, _bb: ByteBuffer",
                        Esc(struct_def.name), [&]() {
                writer += "__init(_i, _bb)";
                writer += "return this";
            });

            // Generate all getters
            GenerateStructGetters(struct_def, writer);

            // Generate Static Fields
            GenerateCompanionObject(writer, [&](){

                if (!struct_def.fixed) {
                    FieldDef *key_field = nullptr;

                    // Generate verson check method.
                    // Force compile time error if not using the same version
                    // runtime.
                    GenerateFunOneLine(writer, "validateVersion", "", "", [&](){
                        writer += "Constants.FLATBUFFERS_1_11_1()";
                    });

                    GenerateGetRootAsAccessors(Esc(struct_def.name), writer);
                    GenerateBufferHasIdentifier(struct_def, writer);
                    GenerateTableCreator(struct_def, writer);

                    GenerateStartStructMethod(struct_def, writer);

                    // Static Add for fields
                    auto fields = struct_def.fields.vec;
                    int field_pos = -1;
                    for (auto it = fields.begin(); it != fields.end(); ++it) {
                        auto &field = **it;
                        field_pos++;
                        if (field.deprecated) continue;
                        if (field.key) key_field = &field;
                        GenerateAddField(NumToString(field_pos), field, writer);

                        if (field.value.type.base_type == BASE_TYPE_VECTOR) {
                            auto vector_type = field.value.type.VectorType();
                            if (!IsStruct(vector_type)) {
                                GenerateCreateVectorField(field, writer);
                            }
                            GenerateStartVectorField(field, writer);
                        }
                    }

                    GenerateEndStructMethod(struct_def, writer);
                    auto file_identifier = parser_.file_identifier_;
                    if (parser_.root_struct_def_ == &struct_def) {
                        GenerateFinishStructBuffer(struct_def,
                                                   file_identifier,
                                                   writer);
                        GenerateFinishSizePrefixed(struct_def,
                                                   file_identifier,
                                                   writer);
                    }

                    if (struct_def.has_key) {
                        GenerateLookupByKey(key_field, struct_def, writer);
                    }
                } else {
                    GenerateStaticConstructor(struct_def, writer);
                }
            });
        }

        // class closing
        writer.DecrementIdentLevel();
        writer += "}";
    }

    // TODO: move key_field to reference instead of pointer
    void GenerateLookupByKey(FieldDef *key_field, StructDef &struct_def,
                             CodeWriter &writer) const {
        std::stringstream params;
        params << "obj: " << Esc(struct_def.name) << "?" << ", ";
        params << "vectorLocation: Int, ";
        params << "key: "
               <<  GenTypeNameDest(key_field->value.type)
                << ", ";
        params << "bb: ByteBuffer";

        auto statements = [&]() {
            auto base_type = key_field->value.type.base_type;
            writer.SetValue("struct_name", Esc(struct_def.name));
            if (base_type == BASE_TYPE_STRING) {
                writer += "val byteKey = key."
                        "toByteArray(Table.UTF8_CHARSET.get()!!)";
            }
            writer += "var span = bb.getInt(vectorLocation - 4)";
            writer += "var start = 0";
            writer += "while (span != 0) {";
            writer.IncrementIdentLevel();
            writer += "var middle = span / 2";
            writer += "val tableOffset = __indirect(vector"
                    "Location + 4 * (start + middle), bb)";
            if (key_field->value.type.base_type == BASE_TYPE_STRING) {
                writer += "val comp = compareStrings(\\";
                writer += GenOffsetGetter(key_field) + "\\";
                writer += ", byteKey, bb)";
            } else {
                auto get_val = GenLookupByKey(key_field, "bb");
                writer += "val value = " + get_val;
                writer += "val comp = (value - key).sign";
            }
            writer += "when {";
            writer.IncrementIdentLevel();
            writer += "comp > 0 -> span = middle";
            writer += "comp < 0 -> {";
            writer.IncrementIdentLevel();
            writer += "middle++";
            writer += "start += middle";
            writer += "span -= middle";
            writer.DecrementIdentLevel();
            writer += "}"; // end comp < 0
            writer += "else -> {";
            writer.IncrementIdentLevel();
            writer += "return (obj ?: {{struct_name}}()).__assign(tableOffset, bb)";
            writer.DecrementIdentLevel();
            writer += "}"; // end else
            writer.DecrementIdentLevel();
            writer += "}"; // end when
            writer.DecrementIdentLevel();
            writer += "}"; // end while
            writer += "return null";
        };
        GenerateFun(writer, "__lookup_by_key",
                    params.str(),
                    Esc(struct_def.name) + "?",
                    statements);
    }

    void GenerateFinishSizePrefixed(StructDef &struct_def,
                                                const std::string &identifier,
                                                CodeWriter &writer) const {
        auto id = identifier.length() > 0  ? ", \"" + identifier + "\"" : "";
        auto params = "builder: FlatBufferBuilder, offset: Int";
        auto method_name = "finishSizePrefixed" + Esc(struct_def.name) + "Buffer";
        GenerateFunOneLine(writer, method_name, params, "", [&]() {
            writer += "builder.finishSizePrefixed(offset" + id  + ")";
        });
    }
    void GenerateFinishStructBuffer(StructDef &struct_def,
                                    const std::string &identifier,
                                    CodeWriter &writer) const {
        auto id = identifier.length() > 0  ? ", \"" + identifier + "\"" : "";
        auto params = "builder: FlatBufferBuilder, offset: Int";
        auto method_name = "finish" + Esc(struct_def.name) + "Buffer";
        GenerateFunOneLine(writer, method_name, params, "", [&]() {
            writer += "builder.finish(offset" + id + ")";
        });
    }

    void GenerateEndStructMethod(StructDef &struct_def, CodeWriter &writer) const {
        // Generate end{{TableName}}(builder: FlatBufferBuilder) method
        auto name = "end" + Esc(struct_def.name);
        auto params = "builder: FlatBufferBuilder";
        auto returns = "Int";
        auto field_vec = struct_def.fields.vec;

        GenerateFun(writer, name, params, returns, [&](){
            writer += "val o = builder.endTable()";
            writer.IncrementIdentLevel();
            for (auto it = field_vec.begin(); it != field_vec.end(); ++it) {
                auto &field = **it;
                if (field.deprecated || !field.required) {
                    continue;
                }
                writer.SetValue("offset", NumToString(field.value.offset));
                writer += "builder.required(o, {{offset}})";
            }
            writer.DecrementIdentLevel();
            writer += "return o";
        });
    }

    // Generate a method to create a vector from a Kotlin array.
    void GenerateCreateVectorField(FieldDef &field, CodeWriter &writer) const {
        auto vector_type = field.value.type.VectorType();
        auto method_name = "create" + MakeCamel(Esc(field.name)) + "Vector";
        auto params = "builder: FlatBufferBuilder, data: " +
                GenTypeBasic(vector_type) + "Array";
        writer.SetValue("size", NumToString(InlineSize(vector_type)));
        writer.SetValue("align", NumToString(InlineAlignment(vector_type)));
        writer.SetValue("root", GenMethod(vector_type));
        writer.SetValue("cast", SourceCastBasic(vector_type, false));

        GenerateFun(writer, method_name, params, "Int", [&](){
            writer += "builder.startVector({{size}}, data.size, {{align}})";
            writer += "for (i in data.size - 1 downTo 0) {";
            writer.IncrementIdentLevel();
            writer += "builder.add{{root}}(data[i]{{cast}})";
            writer.DecrementIdentLevel();
            writer += "}";
            writer += "return builder.endVector()";
        });
    }

    void GenerateStartVectorField(FieldDef &field, CodeWriter &writer) const {
        // Generate a method to start a vector, data to be added manually
        // after.
        auto vector_type = field.value.type.VectorType();
        auto params = "builder: FlatBufferBuilder, numElems: Int";
        writer.SetValue("size", NumToString(InlineSize(vector_type)));
        writer.SetValue("align", NumToString(InlineAlignment(vector_type)));

        GenerateFunOneLine(writer,
                           "start" + MakeCamel(Esc(field.name) + "Vector", true),
                           params,
                           "",
                           [&]() {
            writer += "builder.startVector({{size}}, numElems, {{align}})";
        });
    }

    void GenerateAddField(std::string field_pos, FieldDef &field,
                          CodeWriter &writer) const {
        auto field_type = GenTypeBasic(DestinationType(field.value.type, false));
        auto secondArg = MakeCamel(Esc(field.name), false) + ": " + field_type;
        GenerateFunOneLine(writer, "add" + MakeCamel(Esc(field.name), true),
                           "builder: FlatBufferBuilder, " + secondArg, "", [&](){
            auto method = GenMethod(field.value.type);
            writer.SetValue("field_name", MakeCamel(Esc(field.name), false));
            writer.SetValue("method_name", method);
            writer.SetValue("pos", field_pos);
            writer.SetValue("default", GenDefaultValue(field));
            writer.SetValue("cast_1", FlexBufferBuilderCast(method, field, true));
            writer.SetValue("cast_2", FlexBufferBuilderCast(method, field, false));

            writer += "builder.add{{method_name}}({{pos}}, \\";
            writer += "{{field_name}}{{cast_1}}, {{default}}{{cast_2}})";
        });
    }

    static std::string FlexBufferBuilderCast(const std::string &method,
                                      FieldDef &field,
                                      bool isFirst) {
        auto field_type = GenTypeBasic(DestinationType(field.value.type, false));
        std::string to_type;
        if (method == "Boolean")
            to_type = "Boolean";
        else if (method == "Long")
            to_type = "Long";
        else if (method == "Int" || method == "Offset" || method == "Struct")
            to_type = "Int";
        else if (method == "Byte" || method.empty())
            to_type =  isFirst ? "Byte" : "Int";
        else if (method == "Short")
            to_type =  isFirst ? "Short" : "Int";
        else if (method == "Double")
            to_type =  "Double";
        else if (method == "Float")
            to_type =  isFirst ? "Float" : "Double";

        if (field_type != to_type)
            return ".to" + to_type + "()";
        return "";
    }

    // fun startMonster(builder: FlatBufferBuilder) = builder.startTable(11)
    void GenerateStartStructMethod(StructDef &struct_def, CodeWriter &code) const {
        GenerateFunOneLine(code, "start" + Esc(struct_def.name),
                           "builder: FlatBufferBuilder", "", [&] () {
            code += "builder.startTable("+ NumToString(struct_def.fields.vec.size()) + ")";
        });
    }

    void GenerateTableCreator(StructDef &struct_def, CodeWriter &writer) const {
        // Generate a method that creates a table in one go. This is only possible
        // when the table has no struct fields, since those have to be created
        // inline, and there's no way to do so in Java.
        bool has_no_struct_fields = true;
        int num_fields = 0;
        auto fields_vec = struct_def.fields.vec;

        for (auto it = fields_vec.begin(); it != fields_vec.end(); ++it) {
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

            auto name = "create" + Esc(struct_def.name);
            std::stringstream params;
            params << "builder: FlatBufferBuilder";
            for (auto it = fields_vec.begin(); it != fields_vec.end(); ++it) {
                auto &field = **it;
                if (field.deprecated) continue;
                params << ", " << MakeCamel(Esc(field.name), false);
                if (!IsScalar(field.value.type.base_type)){
                    params << "Offset: ";
                } else {
                    params << ": ";
                }
                params << GenTypeBasic(DestinationType(field.value.type, false));
            }

            GenerateFun(writer, name, params.str(), "Int", [&]() {
                writer.SetValue("vec_size", NumToString(fields_vec.size()));

                writer += "builder.startTable({{vec_size}})";

                auto sortbysize = struct_def.sortbysize;
                auto largest = sortbysize ? sizeof(largest_scalar_t) : 1;
                for (size_t size = largest; size; size /= 2) {
                    for (auto it = fields_vec.rbegin(); it != fields_vec.rend();
                         ++it) {
                        auto &field = **it;
                        auto base_type_size = SizeOf(field.value.type.base_type);
                        if (!field.deprecated &&
                                (!sortbysize || size == base_type_size)) {
                            writer.SetValue("camel_field_name",
                                          MakeCamel(Esc(field.name), true));
                            writer.SetValue("field_name",
                                          MakeCamel(Esc(field.name), false));

                            writer += "add{{camel_field_name}}(builder, {{field_name}}\\";
                            if (!IsScalar(field.value.type.base_type)){
                                writer += "Offset\\";
                            }
                            writer += ")";
                        }
                    }
                }
              writer += "return end{{struct_name}}(builder)";
            });
        }

    }
    void GenerateBufferHasIdentifier(StructDef &struct_def,
                                     CodeWriter &writer) const {
        auto file_identifier = parser_.file_identifier_;
        // Check if a buffer has the identifier.
        if (parser_.root_struct_def_ != &struct_def || !file_identifier.length())
            return;
        auto name = MakeCamel(Esc(struct_def.name), false);
        GenerateFunOneLine(writer, name + "BufferHasIdentifier",
                           "_bb: ByteBuffer",
                           "Boolean",
                           [&]() {
            writer += "__has_identifier(_bb, \"" + file_identifier + "\")";
        });
    }

    void GenerateStructGetters(StructDef &struct_def, CodeWriter &writer) const {
        auto fields_vec = struct_def.fields.vec;
        FieldDef *key_field = nullptr;
        for (auto it = fields_vec.begin(); it != fields_vec.end(); ++it) {
            auto &field = **it;
            if (field.deprecated) continue;
            if (field.key) key_field = &field;

            GenerateComment(field.doc_comment, writer, &comment_config);

            auto field_name = MakeCamel(Esc(field.name), false);
            auto field_type = GenTypeGet(field.value.type);
            auto field_default_value = GenDefaultValue(field);
            auto return_type = GenTypeNameDest(field.value.type);
            auto field_mask = DestinationMask(field.value.type, true);
            auto src_cast = SourceCast(field.value.type);
            auto dst_cast = DestinationCast(field.value.type);
            auto getter = GenGetter(field.value.type);
            auto getter_cast = TypeConversor(field_type, return_type);
            auto offset_val = NumToString(field.value.offset);
            auto offset_prefix = "val o = __offset(" + offset_val
                                 + "); return o != 0 ? ";
            auto value_base_type = field.value.type.base_type;
            // Most field accessors need to retrieve and test the field offset
            // first, this is the offset value for that:
            writer.SetValue("offset", NumToString(field.value.offset));
            writer.SetValue("return_type", return_type);
            writer.SetValue("field_type", field_type);
            writer.SetValue("field_name", field_name);
            writer.SetValue("field_default", field_default_value);
            writer.SetValue("field_mask", field_mask);
            writer.SetValue("field_read_func", getter);
            writer.SetValue("cast", getter_cast);

            auto opt_ret_type = return_type + "?";
            // Generate the accessors that don't do object reuse.
            if (value_base_type == BASE_TYPE_STRUCT) {
                // Calls the accessor that takes an accessor object with a
                // new object.
                // val pos
                //     get() = pos(Vec3())
                GenerateGetterOneLine(writer, field_name, opt_ret_type, [&](){
                    writer += "{{field_name}}({{field_type}}())";
                });
            } else if (value_base_type == BASE_TYPE_VECTOR &&
                       field.value.type.element == BASE_TYPE_STRUCT) {
                // Accessors for vectors of structs also take accessor objects,
                // this generates a variant without that argument.
                // ex: fun weapons(j: Int) = weapons(Weapon(), j)
                GenerateFunOneLine(writer, field_name, "j: Int", opt_ret_type, [&](){
                    writer += "{{field_name}}({{return_type}}(), j)";
                });
            }

            if (IsScalar(value_base_type)) {
                if (struct_def.fixed) {
                    GenerateGetterOneLine(writer, field_name, return_type, [&](){
                        writer += "{{field_read_func}}(bb_pos + {{offset}})"
                                  "{{cast}}{{field_mask}}";
                    });
                } else {
                    GenerateGetter(writer, field_name, return_type, [&](){
                        writer += "val o = __offset({{offset}})";
                        writer += "return if(o != 0) {{field_read_func}}"
                                  "(o + bb_pos){{cast}}{{field_mask}} else "
                                  "{{field_default}}";
                    });
                }
            } else {
                switch (value_base_type) {
                case BASE_TYPE_STRUCT:
                    if (struct_def.fixed) {
                        // create getter with object reuse
                        // ex:
                        // fun pos(obj: Vec3) : Vec3? = obj.__assign(bb_pos + 4, bb)
                        // ? adds nullability annotation
                        GenerateFunOneLine(writer,
                                           field_name, "obj: " + field_type ,
                                           return_type + "?", [&](){
                            writer += "obj.__assign(bb_pos + {{offset}}, bb)";
                        });
                    } else {
                        // create getter with object reuse
                        // ex:
                        //  fun pos(obj: Vec3) : Vec3? {
                        //      val o = __offset(4)
                        //      return if(o != 0) {
                        //          obj.__assign(o + bb_pos, bb)
                        //      else {
                        //          null
                        //      }
                        //  }
                        // ? adds nullability annotation
                        GenerateFun(writer, field_name, "obj: " + field_type,
                                    return_type + "?", [&](){
                            auto fixed = field.value.type.struct_def->fixed;

                            writer.SetValue("seek", Indirect("o + bb_pos", fixed));
                            OffsetWrapper(writer,
                                          offset_val,
                                          [&]() { writer += "obj.__assign({{seek}}, bb)"; },
                                          [&]() { writer += "null"; });
                        });
                    }
                    break;
                case BASE_TYPE_STRING:
                    // create string getter
                    // e.g.
                    // val Name : String?
                    //     get() = {
                    //         val o = __offset(10)
                    //         return if (o != 0) __string(o + bb_pos) else null
                    //     }
                    // ? adds nullability annotation
                    GenerateGetter(writer, field_name, return_type + "?", [&](){

                        writer += "val o = __offset({{offset}})";
                        writer += "return if (o != 0) __string(o + bb_pos) else null";
                    });
                    break;
                case BASE_TYPE_VECTOR: {
                    // e.g.
                    // fun inventory(j: Int) : Int {
                    //     val o = __offset(14)
                    //     return if (o != 0) {
                    //         bb.get(__vector(o) + j * 1).toInt().and(0xFF)
                    //     } else {
                    //        0
                    //     }
                    // }

                    auto vectortype = field.value.type.VectorType();
                    std::string params = "j: Int";
                    std::string nullable = IsScalar(vectortype.base_type) ? ""
                                                                          : "?";

                    if (vectortype.base_type == BASE_TYPE_STRUCT ||
                            vectortype.base_type == BASE_TYPE_UNION) {
                        params = "obj: " + field_type + ", j: Int";
                    }


                    writer.SetValue("toType", TypeConversor(field_type,
                                                            return_type));

                    auto ret_type = return_type + nullable;
                    GenerateFun(writer, field_name, params, ret_type, [&](){
                        auto inline_size = NumToString(InlineSize(vectortype));
                        auto index = "__vector(o) + j * " + inline_size;
                        auto not_found = NotFoundReturn(field.value.type.element);
                        auto found = "";
                        writer.SetValue("index", index);
                        switch(vectortype.base_type) {
                        case BASE_TYPE_STRUCT: {
                            bool fixed = vectortype.struct_def->fixed;
                            writer.SetValue("index", Indirect(index, fixed));
                            found = "obj.__assign({{index}}, bb){{cast}}{{field_mask}}";
                            break;
                        }
                        case BASE_TYPE_UNION:
                            found = "{{field_read_func}}(obj, {{index}} - bb_pos){{cast}}{{field_mask}}";
                            break;
                        default:
                            found = "{{field_read_func}}({{index}}){{cast}}{{field_mask}}";
                        }
                        OffsetWrapper(writer, offset_val,
                                      [&]() { writer += found; } ,
                                      [&]() { writer += not_found; });
                    });
                    break;
                }
                case BASE_TYPE_UNION:
                    GenerateFun(writer, field_name, "obj: " + field_type,
                                return_type + "?", [&](){
                        writer += OffsetWrapperOneLine(offset_val,
                                                       getter + "(obj, o)",
                                                       "null");
                    });
                    break;
                default:
                    FLATBUFFERS_ASSERT(0);
                }
            }

            if (value_base_type == BASE_TYPE_VECTOR) {
                // Generate Lenght functions for vectors
                GenerateGetter(writer, field_name + "Length", "Int", [&](){
                    writer += OffsetWrapperOneLine(offset_val,
                                                   "__vector_len(o)", "0");
                });

                // See if we should generate a by-key accessor.
                if (field.value.type.element == BASE_TYPE_STRUCT &&
                        !field.value.type.struct_def->fixed) {
                    auto &sd = *field.value.type.struct_def;
                    auto &fields = sd.fields.vec;
                    for (auto kit = fields.begin(); kit != fields.end(); ++kit) {
                        auto &kfield = **kit;
                        if (kfield.key) {
                            auto qualified_name = WrapInNameSpace(sd);
                            auto name = MakeCamel(Esc(field.name), false) + "ByKey";
                            auto params = "key: " + GenTypeNameDest(kfield.value.type);
                            auto rtype = qualified_name + "?";
                            GenerateFun(writer, name, params, rtype, [&] () {
                                OffsetWrapper(writer, offset_val,
                                [&] () {
                                    writer += qualified_name +
                                    ".__lookup_by_key(null, __vector(o), key, bb)";
                                },
                                [&] () {
                                    writer += "null";
                                });
                            });

                            auto param2 = "obj: " + qualified_name +
                                    ", key: " +
                                    GenTypeNameDest(kfield.value.type);
                            GenerateFun(writer, name, param2, rtype, [&](){
                                OffsetWrapper(writer, offset_val,
                                [&] () {
                                    writer += qualified_name +
                                    ".__lookup_by_key(obj, __vector(o), key, bb)";
                                },
                                [&]() { writer += "null"; });
                            });

                            break;
                        }
                    }
                }
            }

            if ((value_base_type == BASE_TYPE_VECTOR &&
                 IsScalar(field.value.type.VectorType().base_type)) ||
                    value_base_type == BASE_TYPE_STRING) {

                auto end_idx = NumToString(value_base_type == BASE_TYPE_STRING
                                           ? 1
                                           : InlineSize(field.value.type.VectorType()));
                // Generate a ByteBuffer accessor for strings & vectors of scalars.
                // e.g.
                // val inventoryByteBuffer: ByteBuffer
                //     get =  __vector_as_bytebuffer(14, 1)

                GenerateGetterOneLine(writer, field_name + "AsByteBuffer",
                                      "ByteBuffer", [&](){
                    writer.SetValue("end", end_idx);
                    writer += "__vector_as_bytebuffer({{offset}}, {{end}})";
                });

                // Generate a ByteBuffer accessor for strings & vectors of scalars.
                // e.g.
                // fun inventoryInByteBuffer(_bb: Bytebuffer):
                //     ByteBuffer = __vector_as_bytebuffer(_bb, 14, 1)
                GenerateFunOneLine(writer, field_name + "InByteBuffer",
                                   "_bb: ByteBuffer", "ByteBuffer", [&](){
                    writer.SetValue("end", end_idx);
                    writer += "__vector_in_bytebuffer(_bb, {{offset}}, {{end}})";
                });
            }

            // generate object accessors if is nested_flatbuffer
            //fun testnestedflatbufferAsMonster() : Monster?
            //{ return testnestedflatbufferAsMonster(new Monster()); }

            if (field.nested_flatbuffer) {
                auto nested_type_name = WrapInNameSpace(*field.nested_flatbuffer);
                auto nested_method_name =
                        field_name + "As" +
                        field.nested_flatbuffer->name;

                GenerateGetterOneLine(writer,
                                      nested_method_name,
                                      nested_type_name + "?", [&](){
                    writer += nested_method_name + "(" + nested_type_name + "())";
                });

                GenerateFun(writer,
                            nested_method_name,
                            "obj: " + nested_type_name,
                            nested_type_name + "?", [&](){
                    OffsetWrapper(writer, offset_val,
                                  [&]() { writer += "obj.__assign(__indirect(__vector(o)), bb)"; },
                                  [&]() { writer += "null";});
                });
            }

            // Generate mutators for scalar fields or vectors of scalars.
            if (parser_.opts.mutable_buffer) {
                auto value_type = field.value.type;
                auto underlying_type = value_base_type == BASE_TYPE_VECTOR
                        ? value_type.VectorType()
                        : value_type;
                auto name = "mutate" + MakeCamel(Esc(field.name), true);
                auto size = NumToString(InlineSize(underlying_type));
                auto params = Esc(field.name) + ": " + GenTypeNameDest(underlying_type);
                // A vector mutator also needs the index of the vector element it should
                // mutate.
                if (value_base_type == BASE_TYPE_VECTOR)
                    params.insert(0, "j: Int, ");

                // Boolean parameters have to be explicitly converted to byte
                // representation.
                auto setter_parameter = underlying_type.base_type == BASE_TYPE_BOOL
                        ? "(if(" + Esc(field.name) + ") 1 else 0).toByte()"
                        : Esc(field.name);

                auto setter_index = value_base_type == BASE_TYPE_VECTOR
                        ? "__vector(o) + j * " + size
                        : (struct_def.fixed
                           ? "bb_pos + " + offset_val
                           : "o + bb_pos");
                if (IsScalar(value_base_type) || (value_base_type == BASE_TYPE_VECTOR &&
                         IsScalar(value_type.VectorType().base_type))) {

                    auto statements = [&] () {
                        writer.SetValue("setter", GenSetter(underlying_type));
                        writer.SetValue("index", setter_index);
                        writer.SetValue("params", setter_parameter);
                        writer.SetValue("cast", src_cast);
                        if (struct_def.fixed) {
                            writer += "{{setter}}({{index}}, {{params}}{{cast}})";
                        } else {
                            OffsetWrapper(writer, offset_val, [&](){
                                writer += "{{setter}}({{index}}, {{params}}{{cast}})";
                                writer += "true";
                            }, [&](){ writer += "false";});
                        }
                    };

                    if (struct_def.fixed) {
                        GenerateFunOneLine(writer, name, params, "ByteBuffer",
                                    statements);
                    } else {
                        GenerateFun(writer, name, params, "Boolean",
                                    statements);
                    }
                }
            }
        }
        if (struct_def.has_key && !struct_def.fixed) {
            // Key Comparison method
            GenerateOverrideFun(
                        writer,
                        "keysCompare",
                        "o1: Int, o2: Int, _bb: ByteBuffer", "Int", [&]() {
                auto data_buffer = "_bb";
                if (key_field->value.type.base_type == BASE_TYPE_STRING) {
                    writer.SetValue("offset", NumToString(key_field->value.offset));
                    writer += " return compareStrings(__offset({{offset}}, o1, "
                            "_bb), __offset({{offset}}, o2, _bb), _bb)";

                } else {
                    auto getter1 = GenLookupByKey(key_field, data_buffer, "o1");
                    auto getter2 = GenLookupByKey(key_field, data_buffer, "o2");
                    writer += "val val_1 = " + getter1;
                    writer += "val val_2 = " + getter2;
                    writer += "return (val_1 - val_2).sign";
                }
            });
        }
    }

    static std::string TypeConversor(const std::string &from_type,
                                     const std::string &to_type) {
        if (to_type == "Boolean")
            return ".toInt() != 0";

        if (from_type == to_type)
            return "";

        if (to_type == "Int")
            return ".toInt()";

        if (to_type == "Short")
            return ".toShort()";

        if (to_type == "Long")
            return ".toLong()";

        if (to_type == "Byte")
           return ".toByte()";

        if (to_type == "Boolean")
            return ".toInt() != 0";

        return "";
    }

    void GenerateCompanionObject(CodeWriter &code,
                                 const std::function<void()> &callback) const {
        code += "companion object {";
        code.IncrementIdentLevel();
        callback();
        code.DecrementIdentLevel();
        code += "}";
    }

    // Generate a documentation comment, if available.
    void GenerateComment(const std::vector<std::string> &dc, CodeWriter &writer,
                    const CommentConfig *config) const {
      if (dc.begin() == dc.end()) {
        // Don't output empty comment blocks with 0 lines of comment content.
        return;
      }

      if (config != nullptr && config->first_line != nullptr) {
        writer += std::string(config->first_line);
      }
      std::string line_prefix =
          ((config != nullptr && config->content_line_prefix != nullptr)
               ? config->content_line_prefix
               : "///");
      for (auto it = dc.begin(); it != dc.end(); ++it) {
        writer += line_prefix + *it;
      }
      if (config != nullptr && config->last_line != nullptr) {
        writer += std::string(config->last_line);
      }
    }

    static void GenerateGetRootAsAccessors(const std::string &struct_name,
                                           CodeWriter &writer) {
        // Generate a special accessor for the table that when used as the root
        // ex: fun getRootAsMonster(_bb: ByteBuffer): Monster {...}
        writer.SetValue("gr_name", struct_name);
        writer.SetValue("gr_method", "getRootAs" + struct_name);

        // create convenience method that doesn't require an existing object
        writer += "fun {{gr_method}}(_bb: ByteBuffer): {{gr_name}} = \\";
        writer += "{{gr_method}}(_bb, {{gr_name}}())";

        // create method that allows object reuse
        // ex: fun Monster getRootAsMonster(_bb: ByteBuffer, obj: Monster) {...}
        writer += "fun {{gr_method}}"
                 "(_bb: ByteBuffer, obj: {{gr_name}}): {{gr_name}} {";
        writer.IncrementIdentLevel();
        writer += "_bb.order(ByteOrder.LITTLE_ENDIAN)";
        writer += "return (obj.__assign(_bb.getInt(_bb.position())"
                 " + _bb.position(), _bb))";
        writer.DecrementIdentLevel();
        writer += "}";
    }

    static void GenerateStaticConstructor(const StructDef &struct_def,
                                          CodeWriter &code) {
        // create a struct constructor function
        auto params = StructConstructorParams(struct_def);
        GenerateFun(code, "create" + Esc(struct_def.name), params, "Int", [&](){
            GenStructBody(struct_def, code, "");
            code += "return builder.offset()";
        });
    }

    static std::string StructConstructorParams(const StructDef &struct_def,
                                               const std::string &prefix = "") {
        //builder: FlatBufferBuilder
        std::stringstream out;
        auto field_vec = struct_def.fields.vec;
        if (prefix.empty()) {
            out << "builder: FlatBufferBuilder";
        }
        for (auto it = field_vec.begin(); it != field_vec.end(); ++it) {
            auto &field = **it;
            if (IsStruct(field.value.type)) {
                // Generate arguments for a struct inside a struct. To ensure
                // names don't clash, and to make it obvious these arguments are
                // constructing a nested struct, prefix the name with the field
                // name.
                out << StructConstructorParams(*field.value.type.struct_def,
                                              prefix + (Esc(field.name) + "_"));
            } else {
                out << ", " << prefix << MakeCamel(Esc(field.name), false)
                    << ": "
                    << GenTypeBasic(DestinationType(field.value.type, false));
            }
        }
        return out.str();
    }

    static void GeneratePropertyOneLine(CodeWriter &writer,
                               const std::string &name,
                               const std::string &type,
                               const std::function<void()> &body) {
        // Generates Kotlin getter for properties
        // e.g.:
        // val prop: Mytype = x
        writer.SetValue("_name", name);
        writer.SetValue("_type", type);
        writer += "val {{_name}} : {{_type}} = \\";
        body();
    }
    static void GenerateGetterOneLine(CodeWriter &writer,
                               const std::string &name,
                               const std::string &type,
                               const std::function<void()> &body) {
        // Generates Kotlin getter for properties
        // e.g.:
        // val prop: Mytype get() = x
        writer.SetValue("_name", name);
        writer.SetValue("_type", type);
        writer += "val {{_name}} : {{_type}} get() = \\";
        body();
    }

    static void GenerateGetter(CodeWriter &writer,
                               const std::string &name,
                               const std::string &type,
                               const std::function<void()> &body) {
        // Generates Kotlin getter for properties
        // e.g.:
        // val prop: Mytype
        //     get() = {
        //       return x
        //     }
        writer.SetValue("name", name);
        writer.SetValue("type", type);
        writer += "val {{name}} : {{type}}";
        writer.IncrementIdentLevel();
        writer += "get() {";
        writer.IncrementIdentLevel();
        body();
        writer.DecrementIdentLevel();
        writer += "}";
        writer.DecrementIdentLevel();
    }

    static void GenerateFun(CodeWriter &writer,
                            const std::string &name,
                            const std::string &params,
                            const std::string &returnType,
                            const std::function<void()> &body) {
        // Generates Kotlin function
        // e.g.:
        // fun path(j: Int): Vec3 {
        //     return path(Vec3(), j)
        // }
        auto noreturn = returnType.empty();
        writer.SetValue("name", name);
        writer.SetValue("params", params);
        writer.SetValue("return_type", noreturn ? "" : ": " + returnType);
        writer += "fun {{name}}({{params}}) {{return_type}} {";
        writer.IncrementIdentLevel();
        body();
        writer.DecrementIdentLevel();
        writer += "}";
    }

    static void GenerateFunOneLine(CodeWriter &writer,
                                   const std::string &name,
                                   const std::string &params,
                                   const std::string &returnType,
                                   const std::function<void()> &body) {
        // Generates Kotlin function
        // e.g.:
        // fun path(j: Int): Vec3 = return path(Vec3(), j)
        writer.SetValue("name", name);
        writer.SetValue("params", params);
        writer.SetValue("return_type_p", returnType.empty() ? "" :
                                                          " : " + returnType);
        writer += "fun {{name}}({{params}}){{return_type_p}} = \\";
        body();
    }

    static void GenerateOverrideFun(CodeWriter &writer,
                                   const std::string &name,
                                   const std::string &params,
                                   const std::string &returnType,
                                   const std::function<void()> &body) {
        // Generates Kotlin function
        // e.g.:
        // override fun path(j: Int): Vec3 = return path(Vec3(), j)
        writer += "override \\";
        GenerateFun(writer, name, params, returnType, body);
    }

    static void GenerateOverrideFunOneLine(CodeWriter &writer,
                                   const std::string &name,
                                   const std::string &params,
                                   const std::string &returnType,
                                   const std::string &statement) {
        // Generates Kotlin function
        // e.g.:
        // override fun path(j: Int): Vec3 = return path(Vec3(), j)
        writer.SetValue("name", name);
        writer.SetValue("params", params);
        writer.SetValue("return_type", returnType.empty() ? "" :
                                                          " : " + returnType);
        writer += "override fun {{name}}({{params}}){{return_type}} = \\";
        writer += statement;
    }

    static std::string OffsetWrapperOneLine(const std::string &offset,
                                            const std::string &found,
                                            const std::string &not_found) {
        return "val o = __offset(" + offset + "); return if (o != 0) " + found +
                " else " + not_found;
    }

    static void OffsetWrapper(CodeWriter &code,
                       const std::string &offset,
                       const std::function<void()> &found,
                       const std::function<void()> &not_found) {
        code += "val o = __offset(" + offset + ")";
        code +="return if (o != 0) {";
        code.IncrementIdentLevel();
        found();
        code.DecrementIdentLevel();
        code += "} else {";
        code.IncrementIdentLevel();
        not_found();
        code.DecrementIdentLevel();
        code += "}";
    }

    static std::string Indirect(const std::string &index, bool fixed) {
        // We apply __indirect() and struct is not fixed.
        if (!fixed)
            return "__indirect(" + index + ")";
        return index;
    }

    static std::string NotFoundReturn(BaseType el) {
        if (el == BASE_TYPE_DOUBLE)
            return "0.0";
        if (el == BASE_TYPE_FLOAT)
            return "0.0f";
        if (el == BASE_TYPE_BOOL)
            return "false";
        if (IsScalar(el))
            return "0";
        return "null";
    }

    // This tracks the current namespace used to determine if a type need to be
    // prefixed by its namespace
    const Namespace *cur_name_space_;
};
}  // namespace kotlin

bool GenerateKotlin(const Parser &parser, const std::string &path,
                    const std::string &file_name) {
    kotlin::KotlinGenerator generator(parser, path, file_name);
    return generator.generate();
}
}  // namespace flatbuffers
