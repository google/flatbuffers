/*
 * Copyright 2021 Google Inc. All rights reserved.
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

#include "bfbs_gen_nim.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

// Ensure no includes to flatc internals. bfbs_gen.h and generator.h are OK.
#include "bfbs_gen.h"
#include "bfbs_namer.h"
#include "flatbuffers/bfbs_generator.h"

// The intermediate representation schema.
#include "flatbuffers/reflection.h"
#include "flatbuffers/reflection_generated.h"

namespace flatbuffers {
namespace {

// To reduce typing
namespace r = ::reflection;

std::set<std::string> NimKeywords() {
  return {
    "addr",      "and",     "as",        "asm",      "bind",   "block",
    "break",     "case",    "cast",      "concept",  "const",  "continue",
    "converter", "defer",   "discard",   "distinct", "div",    "do",
    "elif",      "else",    "end",       "enum",     "except", "export",
    "finally",   "for",     "from",      "func",     "if",     "import",
    "in",        "include", "interface", "is",       "isnot",  "iterator",
    "let",       "macro",   "method",    "mixin",    "mod",    "nil",
    "not",       "notin",   "object",    "of",       "or",     "out",
    "proc",      "ptr",     "raise",     "ref",      "return", "shl",
    "shr",       "static",  "template",  "try",      "tuple",  "type",
    "using",     "var",     "when",      "while",    "xor",    "yield",
  };
}

Namer::Config NimDefaultConfig() {
  return { /*types=*/Case::kKeep,
           /*constants=*/Case::kScreamingSnake,
           /*methods=*/Case::kUpperCamel,
           /*functions=*/Case::kUpperCamel,
           /*fields=*/Case::kLowerCamel,
           /*variable=*/Case::kLowerCamel,
           /*variants=*/Case::kKeep,
           /*enum_variant_seperator=*/".",
           /*escape_keywords=*/Namer::Config::Escape::AfterConvertingCase,
           /*namespaces=*/Case::kKeep,
           /*namespace_seperator=*/"/",
           /*object_prefix=*/"",
           /*object_suffix=*/"T",
           /*keyword_prefix=*/"",
           /*keyword_suffix=*/"_",
           /*filenames=*/Case::kKeep,
           /*directories=*/Case::kKeep,
           /*output_path=*/"",
           /*filename_suffix=*/"",
           /*filename_extension=*/".nim" };
}

const std::string Indent = "  ";
const std::string Export = "*";
struct NimTypeField {
  std::string name_;
  std::string type_;
  std::string doc_;
  NimTypeField(const std::string &name, const std::string &type,
               const std::string &doc = std::string())
      : name_(name), type_(type), doc_(doc) {}
};
struct NimType {
  std::string name_;
  std::string pragma_;
  bool isEnum_;
  bool isRef_;
  std::string superClass_;
  std::vector<NimTypeField> fields_;
  std::string doc_;
  NimType(const std::string &name, const std::string &pragma = std::string(),
          bool isEnum = false, bool isRef = false,
          const std::string &superClass = std::string(),
          const std::vector<NimTypeField> &fields = std::vector<NimTypeField>(),
          const std::string &doc = std::string())
      : name_(name),
        pragma_(pragma),
        isEnum_(isEnum),
        isRef_(isRef),
        superClass_(superClass),
        fields_(fields),
        doc_(doc) {}

  std::string Implementation(bool with_type) const {
    std::string ref = isRef_ ? "ref " : "";
    std::string superclass = superClass_ == "" ? "" : " of " + superClass_;
    std::string pragma = pragma_ == "" ? "" : " " + pragma_;
    std::string supertype = isEnum_ ? "enum" : "object";
    std::string field_indent = with_type ? "  " : "    ";
    std::string field_sep = isEnum_ ? " = " : Export + ": ";

    std::string code;
    code += doc_;
    code += with_type ? "type " : "  ";
    code +=
        name_ + Export + pragma + " = " + ref + supertype + superclass + "\n";
    for (auto const &x : fields_) {
      code += x.doc_;
      code += field_indent + x.name_ + field_sep + x.type_ + ",\n";
    }
    code += "\n";
    return code;
  }
};

struct NimFunction {
  std::string name_;
  bool hasSideEffects_;
  std::vector<std::pair<std::string, std::string>> args_;
  std::string returnType_;
  std::string code_;
  std::string doc_;
  NimFunction(const std::string &name, bool hasSideEffects = false,
              const std::vector<std::pair<std::string, std::string>> &args =
                  std::vector<std::pair<std::string, std::string>>(),
              const std::string &returnType = std::string(),
              const std::string &code = std::string(),
              const std::string &doc = std::string())
      : name_(name),
        hasSideEffects_(hasSideEffects),
        args_(args),
        returnType_(returnType),
        code_(code),
        doc_(doc) {}

  std::string Signature(bool with_newline = true) const {
    std::string code;
    std::string def = hasSideEffects_ ? "proc" : "func";
    std::string ret = returnType_ == "" ? "" : ": " + returnType_;
    code += def + " " + name_ + Export + "(";
    bool first = true;
    for (auto const &x : args_) {
      if (first) {
        first = false;
      } else {
        code += ", ";
      }
      code += x.first + ": " + x.second;
    }
    code += ")" + ret;
    if (with_newline) { code += "\n"; }
    return code;
  }
  std::string Implementation() const {
    std::string code;
    code += Signature(false) + " =\n";
    code += code_;
    code += "\n";
    return code;
  }
};

class NimBfbsGenerator : public BaseBfbsGenerator {
 public:
  explicit NimBfbsGenerator(const std::string &flatc_version)
      : BaseBfbsGenerator(),
        keywords_(),
        imports_(),
        current_obj_(nullptr),
        current_enum_(nullptr),
        flatc_version_(flatc_version),
        namer_(NimDefaultConfig(), NimKeywords()) {}

  GeneratorStatus GenerateFromSchema(const r::Schema *schema)
      FLATBUFFERS_OVERRIDE {
    if (!GenerateEnums(schema->enums())) { return FAILED; }
    if (!GenerateObjects(schema->objects(), schema->root_table())) {
      return FAILED;
    }
    return OK;
  }

  uint64_t SupportedAdvancedFeatures() const FLATBUFFERS_OVERRIDE {
    return 0xF;
  }

 protected:
  bool GenerateEnums(
      const flatbuffers::Vector<flatbuffers::Offset<r::Enum>> *enums) {
    ForAllEnums(enums, [&](const r::Enum *enum_def) {
      StartCodeBlock(enum_def);

      std::string ns;
      const std::string enum_name =
          namer_.Type(namer_.Denamespace(enum_def, ns));
      const std::string enum_type =
          GenerateTypeBasic(enum_def->underlying_type());

      NimType enumobj(enum_name, "{.pure.}", true);
      GenerateDocumentation(enum_def->documentation(), "", enumobj.doc_);

      ForAllEnumValues(enum_def, [&](const reflection::EnumVal *enum_val) {
        NimTypeField field(namer_.Variant(enum_val->name()->str()),
                           NumToString(enum_val->value()) + "." + enum_type);
        GenerateDocumentation(enum_val->documentation(), "  ", field.doc_);
        enumobj.fields_.push_back(field);
      });
      types_.push_back(enumobj);

      EmitCodeBlock(enum_name, ns, enum_def->declaration_file()->str());
    });
    return true;
  }

  bool GenerateObjects(
      const flatbuffers::Vector<flatbuffers::Offset<r::Object>> *objects,
      const r::Object *root_object) {
    ForAllObjects(objects, [&](const r::Object *object) {
      StartCodeBlock(object);

      // Register the main flatbuffers module.
      RegisterImports("flatbuffers", "");

      std::string ns;
      const std::string object_name =
          namer_.Type(namer_.Denamespace(object, ns));

      NimType obj(object_name);
      obj.superClass_ = "FlatObj";
      GenerateDocumentation(object->documentation(), "", obj.doc_);
      types_.push_back(obj);

      // Create all the field accessors.
      ForAllFields(object, /*reverse=*/false, [&](const r::Field *field) {
        // Skip writing deprecated fields altogether.
        if (field->deprecated()) { return; }

        const std::string field_name = namer_.Field(field->name()->str());
        const r::BaseType base_type = field->type()->base_type();

        std::string field_type = GenerateType(field->type());
        if (field->optional() && !object->is_struct()) {
          RegisterImports("std/options", "");
          field_type = "Option[" + field_type + "]";
        }

        const std::string offset_prefix =
            "let o = self.tab.Offset(" + NumToString(field->offset()) + ")\n";
        const std::string offset_prefix_2 = "if o != 0:\n";

        if (IsScalar(base_type) || base_type == r::String ||
            base_type == r::Obj || base_type == r::Union) {
          NimFunction getter(field_name, false, { { "self", object_name } },
                             field_type);
          NimFunction setter(
              "`" + field_name + "=`", false,
              { { "self", "var " + object_name }, { "n", field_type } },
              "bool");

          GenerateDocumentation(field->documentation(), "", getter.doc_);
          if (base_type == r::Obj || base_type == r::Union ||
              field->type()->index() >= 0) {
            RegisterImports(object, field);
          }

          if (object->is_struct()) {
            std::string field_getter =
                ReplaceString(GenerateGetter(field->type()), "{offset}",
                              NumToString(field->offset()));
            getter.code_ += "  return " + field_getter + "\n";

            // TODO: parser opts:
            // if (parser_.opts.mutable_buffer) {
            if (IsScalar(base_type)) {
              setter.code_ += "  return self.tab.Mutate(self.tab.Pos + " +
                              NumToString(field->offset()) + ", n)\n";
            }
            //}
          } else {
            // Table accessors
            getter.code_ += "  " + offset_prefix;
            getter.code_ += "  " + offset_prefix_2;
            std::string field_getter =
                ReplaceString(GenerateGetter(field->type()), "{offset}", "o");
            if (field->optional()) {
              field_getter = "some(" + field_getter + ")";
            }
            getter.code_ += "    return " + field_getter + "\n";
            if (!field->optional()) {
              getter.code_ += "  return " + DefaultValue(object, field) + "\n";
            }

            // TODO: parser opts:
            // if (parser_.opts.mutable_buffer) {
            if (IsScalar(base_type)) {
              setter.code_ += "  return self.tab.MutateSlot(" +
                              NumToString(field->offset()) + ", n)\n";
            }
            //}
          }
          functions_.push_back(getter);
          // if (parser_.opts.mutable_buffer) {
          if (IsScalar(base_type)) { functions_.push_back(setter); }
          //}
        } else if (base_type == r::Array || base_type == r::Vector) {
          const r::BaseType vector_base_type = field->type()->element();
          int32_t element_size = field->type()->element_size();

          if (vector_base_type == r::Obj || vector_base_type == r::Union ||
              field->type()->index() >= 0) {
            RegisterImports(object, field, true);
          }

          // Get vector length:
          NimFunction vectorLength(field_name + "Length", false,
                                   { { "self", object_name } }, "int");
          vectorLength.code_ += "  " + offset_prefix;
          vectorLength.code_ += "  " + offset_prefix_2;
          vectorLength.code_ += "    return self.tab.VectorLen(o)\n";

          // Get single vector field:
          NimFunction vectorElement(field_name, false,
                                    { { "self", object_name }, { "j", "int" } },
                                    GenerateType(field->type(), true));
          vectorElement.code_ += "  " + offset_prefix;
          vectorElement.code_ += "  " + offset_prefix_2;
          vectorElement.code_ += "    var x = self.tab.Vector(o)\n";
          vectorElement.code_ += "    x += j.uoffset * " +
                                 NumToString(element_size) + ".uoffset\n";
          vectorElement.code_ +=
              "    return " +
              ReplaceString(GenerateGetter(field->type()), "{offset}", "x") +
              "\n";

          // Get entire vector:
          NimFunction vectorList(field_name, false, { { "self", object_name } },
                                 GenerateType(field->type()));

          vectorList.code_ += "  let len = self." + field_name + "Length\n";
          vectorList.code_ += "  for i in countup(0, len - 1):\n";
          vectorList.code_ += "    result.add(self." + field_name + "(i))\n";

          functions_.push_back(vectorLength);
          functions_.push_back(vectorElement);
          functions_.push_back(vectorList);
          (void)IsSingleByte(vector_base_type);  // Unused exception
        }
      });

      // Create all the builders
      if (object->is_struct()) {
        NimFunction structCreate("Create" + object_name, true,
                                 { { "self", "var Builder" } }, "uoffset");
        std::vector<std::pair<std::string, std::string>> fields_vec =
            GenerateStructBuilderArgs(object);
        structCreate.args_.insert(structCreate.args_.end(), fields_vec.begin(),
                                  fields_vec.end());
        structCreate.code_ += AppendStructBuilderBody(object);
        structCreate.code_ += "  return self.Offset()\n";
        functions_.push_back(structCreate);
      } else {
        // Table builders
        NimFunction tableStart(object_name + "Start", true,
                               { { "builder", "var Builder" } });
        tableStart.code_ += "  builder.StartObject(" +
                            NumToString(object->fields()->size()) + ")\n";
        functions_.push_back(tableStart);

        ForAllFields(object, /*reverse=*/false, [&](const r::Field *field) {
          if (field->deprecated()) { return; }

          const std::string field_name = namer_.Field(field->name()->str());
          const std::string variable_name =
              namer_.Variable(field->name()->str());
          const std::string variable_type = GenerateTypeBasic(field->type());

          NimFunction fieldAdd(object_name + "Add" + field_name, true,
                               { { "builder", "var Builder" },
                                 { variable_name, variable_type } });
          fieldAdd.code_ += "  builder.Prepend" + GenerateMethod(field) +
                            "Slot(" + NumToString(field->id()) + ", " +
                            variable_name + ", default(" + variable_type +
                            "))\n";
          functions_.push_back(fieldAdd);

          if (IsVector(field->type()->base_type())) {
            NimFunction fieldVectorStart(
                object_name + "Start" + field_name + "Vector", true,
                { { "builder", "var Builder" }, { "numElems", "uoffset" } });

            const int32_t element_size = field->type()->element_size();
            int32_t alignment = element_size;
            if (IsStruct(field->type(), /*use_element=*/true)) {
              alignment = GetObjectByIndex(field->type()->index())->minalign();
            }

            fieldVectorStart.code_ +=
                "  builder.StartVector(" + NumToString(element_size) +
                ", numElems, " + NumToString(alignment) + ")\n";
            functions_.push_back(fieldVectorStart);
          }
        });

        NimFunction tableEnd(object_name + "End", true,
                             { { "builder", "var Builder" } }, "uoffset");
        tableEnd.code_ += "  return builder.EndObject()\n";
        functions_.push_back(tableEnd);
      }

      EmitCodeBlock(object_name, ns, object->declaration_file()->str());
    });
    return true;
  }

 private:
  void GenerateDocumentation(
      const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>
          *documentation,
      std::string indent, std::string &code) const {
    flatbuffers::ForAllDocumentation(
        documentation, [&](const flatbuffers::String *str) {
          code += indent + "# " + str->str() + "\n";
        });
  }

  std::vector<std::pair<std::string, std::string>> GenerateStructBuilderArgs(
      const r::Object *object, std::string prefix = "") const {
    std::vector<std::pair<std::string, std::string>> fields_vec;
    ForAllFields(object, /*reverse=*/false, [&](const r::Field *field) {
      if (IsStructOrTable(field->type()->base_type())) {
        const r::Object *field_object = GetObject(field->type());
        std::vector<std::pair<std::string, std::string>> inner_vec =
            GenerateStructBuilderArgs(
                field_object,
                prefix + namer_.Variable(field->name()->str()) + "_");
        fields_vec.insert(fields_vec.end(), inner_vec.begin(), inner_vec.end());
      } else {
        fields_vec.push_back(
            std::make_pair(prefix + namer_.Variable(field->name()->str()),
                           GenerateType(field->type())));
      }
    });
    return fields_vec;
  }

  std::string AppendStructBuilderBody(const r::Object *object,
                                      std::string prefix = "") const {
    std::string code;
    code += "  self.Prep(" + NumToString(object->minalign()) + ", " +
            NumToString(object->bytesize()) + ")\n";

    // We need to reverse the order we iterate over, since we build the
    // buffer backwards.
    ForAllFields(object, /*reverse=*/true, [&](const r::Field *field) {
      const int32_t num_padding_bytes = field->padding();
      if (num_padding_bytes) {
        code += "  self.Pad(" + NumToString(num_padding_bytes) + ")\n";
      }
      if (IsStructOrTable(field->type()->base_type())) {
        const r::Object *field_object = GetObject(field->type());
        code += AppendStructBuilderBody(
            field_object, prefix + namer_.Variable(field->name()->str()) + "_");
      } else {
        code += "  self.Prepend(" + prefix +
                namer_.Variable(field->name()->str()) + ")\n";
      }
    });

    return code;
  }

  std::string GenerateMethod(const r::Field *field) const {
    const r::BaseType base_type = field->type()->base_type();
    if (IsStructOrTable(base_type)) { return "Struct"; }
    return "";
  }

  std::string GenerateGetter(const r::Type *type, bool element_type = false,
                             bool enum_inner = false) const {
    const r::BaseType base_type =
        element_type ? type->element() : type->base_type();
    if (IsScalar(base_type) && !enum_inner) {
      const r::Enum *type_enum = GetEnum(type, element_type);
      if (type_enum != nullptr) {
        return GenerateType(type, element_type, false) + "(" +
               GenerateGetter(type, element_type, true) + ")";
      }
    }
    std::string offset = "{offset}";
    if (!element_type) { offset = "self.tab.Pos + " + offset; }
    switch (base_type) {
      case r::String: return "self.tab.String(" + offset + ")";
      case r::Union: return "self.tab.Union({offset})";
      case r::Obj: {
        return GenerateType(type, element_type) +
               "(tab: Vtable(Bytes: self.tab.Bytes, Pos: " + offset + "))";
      }
      case r::Vector: return GenerateGetter(type, true);
      default:
        return "Get[" + GenerateType(type, element_type, enum_inner) +
               "](self.tab, " + offset + ")";
    }
  }

  std::string GenerateType(const r::Type *type, bool element_type = false,
                           bool enum_inner = false) const {
    const r::BaseType base_type =
        element_type ? type->element() : type->base_type();
    if (IsScalar(base_type)) {
      const r::Enum *type_enum = GetEnum(type, element_type);
      if (type_enum != nullptr && !enum_inner) {
        std::string type_name = type_enum->name()->str();
        std::replace(type_name.begin(), type_name.end(), '.', '_');
        type_name += "." + namer_.Denamespace(type_enum);
        return namer_.Type(type_name);
      }
      return GenerateType(base_type);
    }
    switch (base_type) {
      case r::String: return "string";
      case r::Vector: return "seq[" + GenerateType(type, true) + "]";
      case r::Union: return "Vtable";
      case r::Obj: {
        const r::Object *type_obj = GetObject(type, element_type);
        std::string type_name = type_obj->name()->str();
        if (type_obj == current_obj_) {
          return namer_.Denamespace(type_obj);
        } else {
          std::replace(type_name.begin(), type_name.end(), '.', '_');
          type_name += "." + namer_.Denamespace(type_obj);
          return namer_.Type(type_name);
        }
      };

      default: return "uoffset";
    }
  }

  std::string GenerateTypeBasic(const r::Type *type,
                                bool element_type = false) const {
    const r::BaseType base_type =
        element_type ? type->element() : type->base_type();
    if (IsScalar(base_type)) {
      return GenerateType(base_type);
    } else {
      return "uoffset";
    }
  }

  std::string ReplaceString(std::string subject, const std::string &search,
                            const std::string &replace) {
    size_t pos = subject.find(search, 0);
    if (pos != std::string::npos) {
      subject.replace(pos, search.length(), replace);
    }
    return subject;
  }

  std::string GenerateType(const r::BaseType base_type) const {
    switch (base_type) {
      case r::None: return "uint8";
      case r::UType: return "uint8";
      case r::Bool: return "bool";
      case r::Byte: return "int8";
      case r::UByte: return "uint8";
      case r::Short: return "int16";
      case r::UShort: return "uint16";
      case r::Int: return "int32";
      case r::UInt: return "uint32";
      case r::Long: return "int64";
      case r::ULong: return "uint64";
      case r::Float: return "float32";
      case r::Double: return "float64";
      case r::String: return "string";
      default: return r::EnumNameBaseType(base_type);
    }
  }

  std::string DefaultValue(const r::Object *object,
                           const r::Field *field) const {
    const r::BaseType base_type = field->type()->base_type();
    if (IsFloatingPoint(base_type)) {
      return NumToString(field->default_real());
    }
    if (IsBool(base_type)) {
      return field->default_integer() ? "true" : "false";
    }
    if (IsScalar(base_type)) {
      const r::Enum *type_enum = GetEnum(field->type());
      if (type_enum != nullptr) {
        return GenerateType(field->type()) + "(" +
               NumToString((field->default_integer())) + ")";
      }
      return NumToString((field->default_integer()));
    }
    if (base_type == r::String) {
      return "\"" + field->default_string()->str() + "\"";
    }
    // represents offsets
    return "0";
  }

  void StartCodeBlock(const reflection::Enum *enum_def) {
    current_enum_ = enum_def;
    current_obj_ = nullptr;
    imports_.clear();
  }

  void StartCodeBlock(const reflection::Object *object) {
    current_obj_ = object;
    current_enum_ = nullptr;
    imports_.clear();
  }

  std::vector<std::string> StringSplit(const std::string orig_str,
                                       const std::string token) {
    std::vector<std::string> result;
    std::string str = orig_str;
    while (str.size()) {
      size_t index = str.find(token);
      if (index != std::string::npos) {
        result.push_back(str.substr(0, index));
        str = str.substr(index + token.size());
        if (str.size() == 0) result.push_back(str);
      } else {
        result.push_back(str);
        str = "";
      }
    }
    return result;
  }

  std::string GetRelativePathFromNamespace(const std::string &relative_to,
                                           const std::string &str2) {
    std::vector<std::string> relative_to_vec = StringSplit(relative_to, ".");
    std::vector<std::string> str2_vec = StringSplit(str2, ".");
    while (relative_to_vec.size() > 0 && str2_vec.size() > 0) {
      if (relative_to_vec[0] == str2_vec[0]) {
        relative_to_vec.erase(relative_to_vec.begin());
        str2_vec.erase(str2_vec.begin());
      } else {
        break;
      }
    }
    relative_to_vec.pop_back();
    for (size_t i = 0; i < relative_to_vec.size(); ++i) {
      str2_vec.insert(str2_vec.begin(), std::string(".."));
    }

    std::string new_path;
    for (size_t i = 0; i < str2_vec.size(); ++i) {
      new_path += str2_vec[i];
      if (i != str2_vec.size() - 1) { new_path += "/"; }
    }
    return new_path;
  }

  std::string RegisterImports(const r::Object *object, const r::Field *field,
                              bool use_element = false) {
    std::string type_name;

    const r::BaseType type =
        use_element ? field->type()->element() : field->type()->base_type();

    if (IsStructOrTable(type)) {
      const r::Object *object = GetObjectByIndex(field->type()->index());
      if (object == current_obj_) { return namer_.Denamespace(object); }
      type_name = object->name()->str();
    } else {
      const r::Enum *enum_def = GetEnumByIndex(field->type()->index());
      if (enum_def == current_enum_) { return namer_.Denamespace(enum_def); }
      type_name = enum_def->name()->str();
    }

    std::string name = type_name;
    std::replace(name.begin(), name.end(), '.', '_');
    std::string import_path =
        GetRelativePathFromNamespace(object->name()->str(), type_name);
    return RegisterImports(import_path, name);
  }

  std::string RegisterImports(const std::string &local_name,
                              const std::string &imports_name) {
    imports_[local_name] = imports_name;
    return local_name;
  }

  void EmitCodeBlock(const std::string &name, const std::string &ns,
                     const std::string &declaring_file) {
    const std::string full_qualified_name = ns.empty() ? name : ns + "." + name;

    std::string code = "#[ " + full_qualified_name + "\n";
    code +=
        "  Automatically generated by the FlatBuffers compiler, do not "
        "modify.\n";
    code += "  Or modify. I'm a message, not a cop.\n";
    code += "\n";
    code += "  flatc version: " + flatc_version_ + "\n";
    code += "\n";
    code += "  Declared by  : " + declaring_file + "\n";
    if (schema_->root_table() != nullptr) {
      const std::string root_type = schema_->root_table()->name()->str();
      const std::string root_file =
          schema_->root_table()->declaration_file()->str();
      code += "  Rooting type : " + root_type + " (" + root_file + ")\n";
    }
    code += "]#\n\n";

    if (!imports_.empty()) {
      for (auto it = imports_.cbegin(); it != imports_.cend(); ++it) {
        if (it->second == "") {
          code += "import " + it->first + "\n";

        } else {
          code += "import " + it->first + " as " + it->second + "\n";
        }
      }
      code += "\n";
    }

    // code += code_block;
    if (types_.size() > 0) { code += "type\n"; }
    for (const auto &type : types_) { code += type.Implementation(false); }
    for (const auto &func : functions_) {
      // Forward declaration
      code += func.Signature();
    }
    for (const auto &func : functions_) { code += func.Implementation(); }
    types_.clear();
    functions_.clear();
    // Namespaces are '.' deliminted, so replace it with the path separator.
    std::string path = ns;

    if (ns.empty()) {
      path = ".";
    } else {
      std::replace(path.begin(), path.end(), '.', '/');
    }

    // TODO(derekbailey): figure out a save file without depending on util.h
    EnsureDirExists(path);
    const std::string file_name = path + "/" + namer_.File(name);
    SaveFile(file_name.c_str(), code, false);
  }

  std::unordered_set<std::string> keywords_;
  std::map<std::string, std::string> imports_;
  std::vector<NimType> types_;
  std::vector<NimFunction> functions_;
  const r::Object *current_obj_;
  const r::Enum *current_enum_;
  const std::string flatc_version_;
  const BfbsNamer namer_;
};
}  // namespace

std::unique_ptr<BfbsGenerator> NewNimBfbsGenerator(
    const std::string &flatc_version) {
  return std::unique_ptr<NimBfbsGenerator>(new NimBfbsGenerator(flatc_version));
}

}  // namespace flatbuffers
