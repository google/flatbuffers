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

#include <string>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include "flatbuffers/code_generators.h"

namespace flatbuffers {
namespace rust {

  //static std::string GenGetter(const Type &type);
static std::string GenMethod(const FieldDef &field);
static void GenStructBuilder(const StructDef &struct_def,
                             std::string *code_ptr);
static std::string GenTypeBasic(const Type &type);
static std::string GenTypeGet(const Type &type);
static std::string TypeName(const FieldDef &field);

// Hardcode spaces per indentation.
const std::string Indent = "    ";

// Format a module name from struct/enum definitions.
static std::string ModName(std::string &def_name) {
  std::string name = def_name;
  std::transform(name.begin(), name.end(),
                 name.begin(), ::tolower);
  return name;
}

// Begin by declaring namespace and imports.
static void BeginFile(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "//! Automatically generated, do not modify.\n\n";
    // the flatbuffers runtime lib
    code += "use flatbuffers;\n";
    // definitions in the same namepsace
    code += "use super::*;\n\n";
}

  static std::string MapConstant(const FieldDef &field) {
    if ( (IsScalar(field.value.type.base_type))
         && (TypeName(field).compare("bool") == 0) ) {
      if (field.value.constant == "0") {
        return "false";
      }
      return "true";
    }
    return field.value.constant;
  }

// Begin a table struct declaration.
static void TableStructDefinition(const StructDef &struct_def,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  if (!struct_def.fixed) {
    code += "table_object!{" + struct_def.name;
    code += ", 4"; //mmmm assuming type.base_type is UOFFSET
  } else {
    code += "struct_object!{" + struct_def.name;
    code += ", " + NumToString(struct_def.bytesize);
  }
  code += ", [";
  bool first = true;
  for (auto it = struct_def.fields.vec.begin();
        it != struct_def.fields.vec.end();
        ++it) {
     auto &field = **it;
     if (field.deprecated) continue;
     if (first) {
       code += "\n";
       first = false;
     } else {
       code += ", \n";
     }
     //GenComment(field.doc_comment, code_ptr, nullptr, "");
    if ( (IsScalar(field.value.type.base_type))
         && !(field.value.type.enum_def)) {
        code += Indent + "(";
        code += field.name + ",";
        code += "get_" + TypeName(field);
        code += ", " + TypeName(field);
        code += ", " + NumToString(field.value.offset);
        code += ", " + MapConstant(field);
          code += ")";
        continue;
    }
    if ( (IsScalar(field.value.type.base_type))
         && (field.value.type.enum_def) ) {
        code += Indent +  "(";
        code += field.name + ",";
        code += "simple_enum,";
        code += "get_" + TypeName(field);
        code += ", " + TypeName(field);
        if (field.value.type.enum_def->is_union) {
          code += ", " + field.value.type.enum_def->name;
          code += "Type";
        } else {
          code += ", " + field.value.type.enum_def->name;
        }
        code += ", " + NumToString(field.value.offset);
        code += ", " + MapConstant(field);
        code += ")";
        continue;
    }
    switch (field.value.type.base_type) {
    case BASE_TYPE_STRUCT: {
      code += Indent +  "(";
      code += field.name + ",";
      code += "get_struct";
      code += ", " + TypeName(field);
      code += ", " + NumToString(field.value.offset);
      code += ")";
      break;
    }
    case BASE_TYPE_STRING: {
      code += Indent +  "(";
      code += field.name + ",";
      code += "get_str";
      code += ", " + TypeName(field);
      code += ", " + NumToString(field.value.offset);
      code += ", " + MapConstant(field);
      code += ")";
      break;
    }
    case BASE_TYPE_VECTOR: {
      code += Indent +  "(";
      code += field.name + ",";
      code += "vector";
      code += ", " + TypeName(field);
      code += ", " + NumToString(field.value.offset);
      code += ")";
      break;
      break;
    }
    case BASE_TYPE_UNION:
      code += Indent +  "(";
      code += field.name + ",";
      code += "union,";
      code += field.name + "_type";
      code += ", " + TypeName(field);
      code += ", " + field.value.type.enum_def->name;
      code += ", " + NumToString(field.value.offset);
      code += ", " + MapConstant(field);
      code += ")";
      break;
    default:
      assert(0);
    }
  }
  code += "]}\n\n";

}

std::string GenFieldOffsetName(const FieldDef &field,
                               bool qualified) {
    std::string uname = field.name;
    std::transform(uname.begin(), uname.end(), uname.begin(), ::toupper);
    if (qualified) {
        return "VT::" + uname;
    } else {
        return uname;
    }
}

// Most field accessors need to retrieve and test the field offset first,
// this is the prefix code for that.
std::string OffsetPrefix(const FieldDef &field) {
  return Indent + Indent +
    "let offset = self.table.field_offset(" +
    GenFieldOffsetName(field, true) +
    " as u16);\n" + Indent + Indent + "if offset != 0 {\n";
}

// Begin the creator function signature.
static void BeginBuilderArgs(const StructDef &struct_def,
                             std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string lname = struct_def.name;
  std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
  code += Indent + "fn build_" + lname;
  code += "(&mut self";
}

static void BeginBuilderTraitArgs(const StructDef &struct_def,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string lname = struct_def.name;
  std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
  code += "pub trait " + struct_def.name + "Builder {\n";
  code += Indent + "fn build_" + lname;
  code += "(&mut self";
}

// Recursively generate arguments for a constructor, to deal with nested
// structs.
static void StructBuilderArgs(const StructDef &struct_def,
                              const char *nameprefix,
                              std::string *code_ptr,
                              bool name_only = false) {
  std::string &code = *code_ptr;
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (IsStruct(field.value.type)) {
      // Generate arguments for a struct inside a struct. To ensure names
      // don't clash, and to make it obvious these arguments are constructing
      // a nested struct, prefix the name with the field name.
      StructBuilderArgs(*field.value.type.struct_def,
                        (nameprefix + (field.name + "_")).c_str(),
                        code_ptr, name_only);
    } else {
      code += (std::string)", " + nameprefix;
      code += field.name;
      if (!name_only) {
        code += ": " + GenTypeBasic(field.value.type);
      }
    }
  }
}

// End the creator function signature.
static void EndBuilderArgs(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += ") -> flatbuffers::UOffsetT {\n";
}

// End the creator function signature.
static void EndBuilderTraitArgs(const StructDef &struct_def,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += ") -> flatbuffers::UOffsetT;\n";
  code += "}\n\n";
  code += "impl " + struct_def.name + "Builder for flatbuffers::Builder {\n";
}

// Recursively generate struct construction statements and instert manual
// padding.
static void StructBuilderBody(const StructDef &struct_def,
                              const char *nameprefix,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent + Indent + "self.prep(" + NumToString(struct_def.minalign) + ", ";
  code += NumToString(struct_def.bytesize) + ");\n";
  for (auto it = struct_def.fields.vec.rbegin();
       it != struct_def.fields.vec.rend();
       ++it) {
    auto &field = **it;
    if (field.padding)
      code += Indent + Indent + "self.pad(" + NumToString(field.padding) + ");\n";
    if (IsStruct(field.value.type)) {
      StructBuilderBody(*field.value.type.struct_def,
                        (nameprefix + (field.name + "_")).c_str(),
                        code_ptr);
    } else {
      code += Indent + Indent + "self.add_" + GenMethod(field) + "(";
      code += nameprefix + field.name + ");\n";
    }
  }
}

// End the builder function
static void EndBuilderBody(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent + Indent +"self.offset() as flatbuffers::UOffsetT \n";
  code += Indent + "}\n";
  code += "}\n\n";
}

// Get the value of a table's starting offset.
static void GetStartOfTable(const StructDef &struct_def,
                            std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string lname = struct_def.name;
  std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
  code += Indent;
  code += "fn start_" + lname + "(&mut self) {\n";
  code += Indent + Indent;
  code += "self.start_object(";
  code += NumToString(struct_def.fields.vec.size());
  code += ");\n";
  code += Indent + "}\n\n";
}

static void BuildFieldOfTableDef(const StructDef &struct_def,
                                 const FieldDef &field,
                                 std::string *code_ptr) {
    std::string &code = *code_ptr;
    std::string lname = struct_def.name;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
    code += Indent;
    code += "/// Set the value for field `" + field.name + "`.\n";
    code += Indent;
    code += "fn add_" + field.name;
    code += "(&mut self, ";
    code += field.name + ": ";
    if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
      code += "flatbuffers::UOffsetT";
    } else {
      code += GenTypeBasic(field.value.type);
    }
    code += ");\n";
}

// Set the value of a table's field.
static void BuildFieldOfTable(const StructDef &struct_def,
                              const FieldDef &field,
                              const size_t offset,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string lname = struct_def.name;
  std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
  code += Indent;
  code += "fn add_" + field.name;
  code += "(&mut self, ";
  code += field.name + ": ";
  if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
    code += "flatbuffers::UOffsetT";
  } else {
    code += GenTypeBasic(field.value.type);
  }
  code += ") {\n";
  code += Indent + Indent;
  code += "self.add_slot_";
  code += GenMethod(field) + "(";
  code += NumToString(offset) + ", ";
  code += field.name;
  code += ", " + MapConstant(field);
  code += ")\n";
  code += Indent + "}\n\n";
}

// Set the value of one of the members of a table's vector.
static void BuildVectorOfTableDef(const FieldDef &field,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent;
  code += "/// Initializes bookkeeping for writing a new `";
  code += field.name + "` vector.\n";
  code += Indent;
  code += "fn start_";
  code += field.name;
  code += "_vector(&mut self, numElems: usize);\n";
}



// Set the value of one of the members of a table's vector.
static void BuildVectorOfTable(const FieldDef &field,
                               std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent;
  code += "/// Initializes bookkeeping for writing a new `";
  code += field.name + "` vector.\n";
  code += Indent;
  code += "fn start_";
  code += field.name;
  code += "_vector(&mut self, numElems: usize) {\n";
  code += Indent + Indent;
  code += "self.start_vector(";
  auto vector_type = field.value.type.VectorType();
  auto alignment = InlineAlignment(vector_type);
  auto elem_size = InlineSize(vector_type);
  code += NumToString(elem_size);
  code += ", numElems, " + NumToString(alignment);
  code += ")\n";
  code += Indent + "}\n\n";
}

// Generate table constructors, conditioned on its members' types.
static void GenTableBuilderStructImpl(const StructDef &struct_def,
                             std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string lname = struct_def.name;
  std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
  code += "/// Builder Trait for `"+ struct_def.name + "` tables.\n";
  code += "pub trait " + struct_def.name  + "Builder {\n";
  code += Indent + "fn start_"+lname+ "(&mut self);\n";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (field.deprecated) continue;
    BuildFieldOfTableDef(struct_def, field, code_ptr);
    if (field.value.type.base_type == BASE_TYPE_VECTOR) {
      BuildVectorOfTableDef(field, code_ptr);
    }
  }
  code += "}\n\n";

  code += "impl " + struct_def.name + "Builder for flatbuffers::Builder {\n";
}

// Generate table constructors, conditioned on its members' types.
static void GenEndTableBuilderStructImpl(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "}\n\n";}

// Generate table constructors, conditioned on its members' types.
static void GenTableBuilders(const StructDef &struct_def,
                             std::string *code_ptr) {
  GenTableBuilderStructImpl(struct_def, code_ptr);
  GetStartOfTable(struct_def, code_ptr);
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (field.deprecated) continue;

    auto offset = it - struct_def.fields.vec.begin();
    BuildFieldOfTable(struct_def, field, offset, code_ptr);
    if (field.value.type.base_type == BASE_TYPE_VECTOR) {
      BuildVectorOfTable(field, code_ptr);
    }
  }
  //GenTableBuilderFxedFns(struct_def, code_ptr);
  GenEndTableBuilderStructImpl(code_ptr);
}

// Generate struct or table methods.
static void GenStruct(const StructDef &struct_def,
                      std::string *code_ptr) {
  if (struct_def.generated) return;
  GenComment(struct_def.doc_comment, code_ptr, nullptr);
   
  TableStructDefinition(struct_def, code_ptr);
 
  if (struct_def.fixed) {
    // create a struct constructor function
    GenStructBuilder(struct_def, code_ptr);
  } else {
    // Create a set of functions that allow table construction.
    GenTableBuilders(struct_def, code_ptr);
  }
}

// Generate enum declarations.
static void GenEnum(const EnumDef &enum_def, std::string *code_ptr) {
  if (enum_def.generated) return;
  std::string &code = *code_ptr;
  GenComment(enum_def.doc_comment, code_ptr, nullptr);
  if (enum_def.is_union) {
    code += "union!{" + enum_def.name +",";
    code += enum_def.name + "Type,";
  } else {
    code += "simple_enum!{" + enum_def.name +",";
  }
  code += GenTypeGet(enum_def.underlying_type);
  code += ", [";
  bool first = true;
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    //GenComment(ev.doc_comment, code_ptr, nullptr, Indent.c_str());
    if  (ev.name.compare("NONE") == 0) {
      continue;
    }
    if (first) {
      code += "\n";
      first = false;
    } else {
      code += ", \n";
    }
    if (enum_def.is_union) {
      code += Indent + "(" + ev.name +", "+NumToString(ev.value);
      code += ", " + ev.name;
      code += ")";
    }
    else {
      code += Indent + "(" + ev.name +", "+NumToString(ev.value) +")";
    }
  }
  code += "]}\n\n";
  
}

// Returns the method name for use with add/put calls.
static std::string GenMethod(const FieldDef &field) {
  return IsScalar(field.value.type.base_type)
    ? GenTypeBasic(field.value.type)
    : (IsStruct(field.value.type) ? "struct" : "uoffset");
}


// Generate the mod.rs export.
static std::string GenNameSpaceExports(const Parser &parser_,
                                       std::string &namespace_name) {
  std::string code = "";
  std::string re_exports = "";
  code += "//! Automatically generated, do not modify\n";
  code += "//!\n";
  code += "//! Flatbuffer definitions for the " + namespace_name;
  code += " namespace.\n";
  for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
       ++it) {
    auto &enum_def = **it;
    std::string mod_name = ModName(enum_def.name);
    std::string qname = enum_def.defined_namespace->components.back();
    if (namespace_name.compare(qname) == 0) {
      code += "mod " + mod_name + ";\n";
      re_exports += "pub use self::" + mod_name + "::*;\n";
    }
  }

  for (auto it = parser_.structs_.vec.begin();
       it != parser_.structs_.vec.end(); ++it) {
    auto &struct_def = **it;
    std::string mod_name = ModName(struct_def.name);
    std::string qname = struct_def.defined_namespace->components.back();
    if (namespace_name.compare(qname) == 0) {
      code += "pub mod " + mod_name + ";\n";
      re_exports += "pub use self::" + mod_name + "::{";
      re_exports += struct_def.name +", "+ struct_def.name +"Builder};\n"; 
    }
  }
  std::vector<std::string> components = parser_.namespaces_.back()->components;
  int pos = find(components.begin(),
                 components.end(),
                 namespace_name)
    - components.begin();
  unsigned long next_pos = pos + 1;
  if(next_pos < components.size()) {
    std::string mod_name = ModName(components[next_pos]);
    re_exports += "pub mod " + mod_name;
    re_exports += ";\n";
  }
  code += "\n" + re_exports;
  return code;
}

// Save out the generated code for a Rust Table type.
static bool SaveType(const Parser &parser, const Definition &def,
                     const std::string &classcode, const std::string &path
                     ) {
  if (!classcode.length()) return true;

  std::string namespace_name;
  std::string namespace_dir = path;
  auto &namespaces = parser.namespaces_.back()->components;
  for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
    if (namespace_name.length()) {
      namespace_name += ".";
      namespace_dir += kPathSeparator;
    }
    namespace_name = *it;
    namespace_dir += *it;
    if (parser.opts.strict_rust) {
      std::transform(namespace_dir.begin(), namespace_dir.end(),
                     namespace_dir.begin(), ::tolower);
    }
    EnsureDirExists(namespace_dir.c_str());

    std::string mod_filename = namespace_dir + "/mod.rs";
    std::string mod_exports = GenNameSpaceExports(parser, namespace_name);
    SaveFile(mod_filename.c_str(), mod_exports, false);
  }


  std::string code = "";
  BeginFile(&code);
  code += classcode;
  std::string filename = namespace_dir + kPathSeparator + def.name + ".rs";
  if (parser.opts.strict_rust) {
    std::transform(filename.begin(), filename.end(),
                   filename.begin(), ::tolower);
  }
  return SaveFile(filename.c_str(), code, false);
}

static std::string GenTypeBasic(const Type &type) {
  static const char *ctypename[] = {
   #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, RTYPE) \
      #RTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };
  return ctypename[type.base_type];
}

static std::string GenTypePointer(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING:
      return "&str";
    case BASE_TYPE_VECTOR:
      return GenTypeGet(type.VectorType());
    case BASE_TYPE_STRUCT:
      return type.struct_def->name;
    case BASE_TYPE_UNION:
      // fall through
  default:
      return type.enum_def->name;
  }
}

static std::string GenTypeGet(const Type &type) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(type)
    : GenTypePointer(type);
}

static std::string TypeName(const FieldDef &field) {
  return GenTypeGet(field.value.type);
}

// Create a struct with a builder and the struct's arguments.
static void GenStructBuilder(const StructDef &struct_def,
                             std::string *code_ptr) {

  BeginBuilderTraitArgs(struct_def, code_ptr);
  StructBuilderArgs(struct_def, "", code_ptr);
  EndBuilderTraitArgs(struct_def, code_ptr);


  BeginBuilderArgs(struct_def, code_ptr);
  StructBuilderArgs(struct_def, "", code_ptr);
  EndBuilderArgs(code_ptr);

  StructBuilderBody(struct_def, "", code_ptr);
  EndBuilderBody(code_ptr);
}

class RustGenerator : public BaseGenerator {
 public:
  RustGenerator(const Parser &parser, const std::string &path,
                  const std::string &file_name)
      : BaseGenerator(parser, path, file_name){};
  bool generate() {
    if (!generateEnums()) return false;
    if (!generateStructs()) return false;
    return true;
  }

 private:
  bool generateEnums() {
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      auto &enum_def = **it;
      std::string enumcode;
      GenEnum(enum_def, &enumcode);
      if (!SaveType(parser_, enum_def, enumcode, path_)) return false;
    }
    return true;
  }

  bool generateStructs() {
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      auto &struct_def = **it;
      std::string declcode;
      GenStruct(struct_def, &declcode);
      if (!SaveType(parser_, struct_def, declcode, path_)) return false;
    }
    return true;
  }
};

} // namespace rust

bool GenerateRust(const Parser &parser, const std::string &path,
                const std::string &file_name) {
  rust::RustGenerator generator(parser, path, file_name);
  return generator.generate();
}

} // namespace flatbuffers
