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

static std::string GenGetter(const Type &type);
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
static void BeginFile(const bool needs_imports,
                      std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "//! Automatically generated, do not modify.\n\n";
  if (needs_imports) {
    // the flatbuffers runtime lib
    code += "use flatbuffers;\n";
    // definitions in the same namepsace
    code += "use super::*;\n\n";
  }
}

// Begin a table struct declaration.
static void TableStructDefinition(const StructDef &struct_def,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "#[derive(Debug)]\n";
  code += "pub struct " + struct_def.name + "<'a> {\n";
  code += Indent + "table: flatbuffers::Table<'a>,\n";
  code += "}\n\n";
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

// Vtable Enum defintion
static void VtableDefinition(const StructDef &struct_def,
                             std::string *code_ptr) {
    std::string &code = *code_ptr;
    if (struct_def.fields.vec.size() > 0) {
        code += "enum VT {\n";
        for (auto it = struct_def.fields.vec.begin();
             it != struct_def.fields.vec.end();
             ++it) {
            auto &field = **it;
            if (!field.deprecated) {  // Deprecated fields won't be accessible.
                code += Indent + GenFieldOffsetName(field, false) + " = ";
                code += NumToString(field.value.offset);
                code += ",";
                if (field.padding) {
                    code += "// Padding" + NumToString(field.padding) + "\n";
                } else {
                    code += "\n";
                }
            }
        }
        code += "}\n\n";
    }
}

// Start Table Struct implimentation functions
static void BeginTableImpl(const StructDef &struct_def,
                           std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "impl<'a> " + struct_def.name + "<'a> {\n";
}

// End Struct implimentation functions
static void EndImpl(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "}\n\n";
}

// A single enum member definition
static void EnumUnionMember(const EnumVal ev, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent;
  if (ev.name.compare("NONE") == 0) {
    code += "None,\n";
  } else {
    // TODO dont assume table
    code += ev.name + "(" +ev.name + "<'a>),\n";
  }
}

// A wrapper aroudn a union Enum to use as a return type in accessor
// functions
static void EnumUnionImpl(const EnumDef &enum_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string uname =  enum_def.name + "Union";
  code += "#[derive(Debug)]\n";
  code += "pub enum " + uname + "<'a> {\n";
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    EnumUnionMember(ev, code_ptr);
  }
  code += "}\n\n";
  code += "impl<'a> " + enum_def.name + "Union<'a> {\n";
  code += Indent + "pub fn from_type(";
  code += "table: &'a flatbuffers::Table, t: ";
  code += enum_def.name + ", offset: usize) -> ";
  code += enum_def.name + "Union<'a> {\n";
  code += Indent + Indent + "match t {\n";
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    code += Indent + Indent + Indent;
    if (ev.name.compare("NONE") == 0) {
      code += enum_def.name + "::NONE => {\n";
      code += Indent + Indent + Indent + Indent;
      code += uname + "::None\n";
      code += Indent + Indent + Indent;
      code +="}\n";
    } else {
      code += enum_def.name +"::" + ev.name;
      code += " => {\n";
      code += Indent + Indent + Indent +Indent;
      code += uname + "::" + ev.name +"(";
      code += "table.get_root(offset as u32).into()";
      code += ")\n";
      code += Indent + Indent + Indent;
      code += "}\n";
    }
  }
  code += Indent + Indent + "}\n";
  code += Indent + "}\n";
  code += "}\n\n";
}

 
// Begin enum code with a class declaration.
static void BeginEnum(const EnumDef &enum_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "#[derive(PartialEq, Eq, Clone, Debug, Hash)]\n";
  code += "#[repr(" + GenTypeGet(enum_def.underlying_type) + ")]\n";
  code += "pub enum " + enum_def.name + " {\n";
}

// A single enum member definition
static void EnumMember(const EnumVal ev, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent;
  code += ev.name;
  code += " = " + NumToString(ev.value) + ",\n";
}

// A single enum member match on value return type
static void EnumValueMatch(const std::string class_name,
                           const EnumVal ev, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent + Indent + Indent;
  code += NumToString(ev.value) + " => " ;
  code += class_name + "::" + ev.name+ ",\n";
}

  // A single enum member match on type return name
static void EnumNameMatch(const std::string class_name,
                           const EnumVal ev, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent + Indent + Indent;
  code += class_name + "::" + ev.name;
  code += " => \"" + ev.name + "\",\n";
}

// Const to list all Enum variants
static void EnumConst(const EnumDef &enum_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string uname = enum_def.name;
  std::transform(uname.begin(), uname.end(), uname.begin(), ::toupper);
  code += "/// A List of all `"+ enum_def.name + "` enum variants.\n";
  code += "pub const " + uname + "_LIST: [";
  code += enum_def.name + ";" + NumToString(enum_def.vals.vec.size());
  code += "] = [";
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    code += enum_def.name + "::"  +  ev.name + ",";
  }
  code += "];\n\n";
}

// Impl for Enum
static void EnumImpl(const EnumDef &enum_def, std::string *code_ptr) {
  EnumConst(enum_def, code_ptr);
  std::string &code = *code_ptr;
  code += "impl " + enum_def.name + " {\n";
  code += Indent;
  code += "/// Returns a `str` representation of a `"+ enum_def.name;
  code += "` enum.\n";
  code += Indent + "pub fn name(&self) -> &'static str {\n";
  code += Indent + Indent;
  code += "match *self {\n";
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    EnumNameMatch(enum_def.name, ev, code_ptr);
  }
  code += Indent + Indent + "}\n";
  code += Indent + "}\n";
  code += "}\n\n";
}

// Enum Impl for From trait 
static void EnumFromImpl(const EnumDef &enum_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "impl From<";
  code += GenTypeGet(enum_def.underlying_type);
  code += "> for " + enum_def.name + " {\n";
  code += Indent + "fn from(value: ";
  code += GenTypeGet(enum_def.underlying_type);
  code += ") -> " + enum_def.name;
  code += " {\n";
  code += Indent + Indent;
  code += "match value {\n";
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    EnumValueMatch(enum_def.name, ev, code_ptr);
  }
  code += Indent + Indent + Indent;
  code += "_ => unreachable!(\"Unable to create a `" + enum_def.name;
  code += "` from value {} \", value),\n";
  code += Indent + Indent + "}\n";
  code += Indent + "}\n";
  code += "}\n\n";
}

// Enum Impl for From trait 
// static void EnumFromRefImpl(const EnumDef &enum_def, std::string *code_ptr) {
//   std::string &code = *code_ptr;
//   code += "impl<'a> From<&'a ";
//   code += GenTypeGet(enum_def.underlying_type);
//   code += "> for &'a " + enum_def.name + " {\n";
//   code += Indent + "fn from(value: &";
//   code += GenTypeGet(enum_def.underlying_type);
//   code += ") -> &" + enum_def.name;
//   code += " {\n";
//   code += Indent + Indent;
//   code += "unsafe {\n";
//   code += Indent + Indent + Indent;
//   code += "use std::mem;\n";
//   code += Indent + Indent + Indent;
//   code += "mem::transmute(value)\n";
//   code += Indent + Indent + "}\n";
//   code += Indent + "}\n";
//   code += "}\n\n";
// }
  

// End enum code.
static void EndEnum(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "}\n\n";
}

// Initialize a new struct or table from existing data.
static void NewRootTypeFromBuffer(const StructDef &struct_def,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent + "pub fn new";
  code += "(buf: &[u8], offset: flatbuffers::UOffsetT) -> ";
  code += struct_def.name + " {\n";
  code += Indent + Indent;
  code += struct_def.name +  " {\n";
  code += Indent + Indent + Indent;
  if (struct_def.fixed) {
    code += "table: flatbuffers::Table::with_pos(buf, offset),\n"; 
  } else {
    code += "table: flatbuffers::Table::from_offset(buf, offset),\n"; 
  }
  code += Indent + Indent + "}\n";
  code += Indent + "}\n\n";
}

// Most field accessors need to retrieve and test the field offset first,
// this is the prefix code for that.
std::string OffsetPrefix(const FieldDef &field) {
  return Indent + Indent +
    "let offset = self.table.field_offset(" +
    GenFieldOffsetName(field, true) +
    " as u16);\n" + Indent + Indent + "if offset != 0 {\n";
}

//Get the value of a struct's scalar.
static void GetScalarFieldOfStruct(const FieldDef &field,
                                   std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string getter = GenGetter(field.value.type);
  std::string unsigned_type = TypeName(field);
  bool is_enum = false;
  if (field.value.type.enum_def) {
    is_enum = true;
  }
  code += Indent + "pub fn ";
  code += field.name + "(&self) -> ";
  if (is_enum) {
    code += field.value.type.enum_def->name;
  } else {
    code += TypeName(field);
  }
  code += " {\n";
  code += Indent + Indent;
  code += "let offset = ";
  code +=  GenFieldOffsetName(field, true) + " as u32;\n";
  code += Indent + Indent;
  code += "return " + getter + "offset)"; 
  if (is_enum) {
    code += ".into()";
  }
  code += "\n";
  code += Indent + "}\n\n";
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

// Get the value of a table's scalar.
static void GetScalarFieldOfTable(const FieldDef &field,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string getter = GenGetter(field.value.type);
  bool is_enum = false;
  if (field.value.type.enum_def) {
    is_enum = true;
  }
  code += Indent + "pub fn ";
  code += field.name + "(&self) -> ";
  if (is_enum) {
    code += field.value.type.enum_def->name;
  } else {
    code += TypeName(field);
  }
  code += " {\n";
  code += OffsetPrefix(field);
  code += Indent + Indent + Indent + "return " + getter;
  code += "offset)";
  if (is_enum) {
    code += ".into()";
  }
  code += "\n";
  code += Indent + Indent + "}\n";
  code += Indent + Indent + MapConstant(field);
  if (is_enum) {
    code += ".into()";
  }
  code += "\n" +Indent + "}\n\n";
}

// Get a struct by initializing an existing struct.
// Specific to Struct.
static void GetStructFieldOfStruct(const FieldDef &field,
                                   std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent + "pub fn " + field.name;
  code += "(&self) -> "+ TypeName(field) +" {\n";
  code += Indent + Indent;
  code += "let offset = " + GenFieldOffsetName(field, true);
  code += " as u32;\n";
  code += Indent + Indent + " return self.table.get_struct";
  code += "::<" + TypeName(field) + ">";
  code += "(offset)\n";
  code += Indent + "}\n\n";
}

// Get a struct by initializing an existing struct.
// Specific to Table.
static void GetStructFieldOfTable(const FieldDef &field,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent + "pub fn " + field.name;
  code += "(&self) -> Option<"+ TypeName(field) + "> {\n";
  code += OffsetPrefix(field);
  if (field.value.type.struct_def->fixed) {
    /// struct stored inli
    code += Indent + Indent + Indent + " return Some(self.table.get_struct";
    code += "::<" + TypeName(field) + ">";
    code += "(offset))\n";
  } else {
    code += Indent + Indent + Indent;
    code += "return Some(self.table.get_indirect_root(offset).into())\n";
  }
  code += Indent + Indent + "};\n";
  code += Indent + Indent + "None\n";
  code += Indent + "}\n\n";
}

// Get the value of a string.
static void GetStringField(const FieldDef &field,
                           std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent + "pub fn " + field.name;
  code += "(&self) -> &str {\n";
  code += OffsetPrefix(field);
  code += Indent + Indent + Indent + "return " + GenGetter(field.value.type);
  code += "offset)\n";
  code += Indent + Indent + "}\n";
  code += Indent + Indent + "\"\"\n";
  code += Indent + "}\n\n";
}

// Get the value of a union from an object.
static void GetUnionField(const FieldDef &field,
                          std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent + "pub fn " + field.name;
  code += "(&self) -> ";
  code += TypeName(field) + " {\n";
  code += OffsetPrefix(field);
  code += Indent + Indent + Indent + "let t = self.";
  code += field.name +"_type();\n";
  code += Indent + Indent + Indent;
  code += Indent + Indent + Indent + "return ";
  code += TypeName(field) + "::from_type(&self.table";
  code += ", t, offset as usize);\n";
  code += Indent + Indent + "}\n";
  code += Indent +Indent + TypeName(field) + "::None\n";
  code += Indent + "}\n\n";
}

// Get the value of a vector's struct member.
static void GetMemberOfVectorOfStruct(const FieldDef &field,
                                      std::string *code_ptr) {
  std::string &code = *code_ptr;
  auto vectortype = field.value.type.VectorType();
  code += Indent + "pub fn " + field.name;
  code += "(&self) -> flatbuffers::Iterator<";
  code += TypeName(field) + "> {\n";
  code += OffsetPrefix(field);
  code += Indent + Indent + Indent;
  if (!(vectortype.struct_def->fixed)) {
    code += "return self.table";
    code += ".table_vector(offset)\n";
  } else {
    code += "return self.table";
    code += ".struct_vector(offset)\n";
  }
  code += Indent + Indent + "}\n";
  code += Indent +Indent + "flatbuffers::empty_iterator(&self.table)\n";
  code += Indent + "}\n\n";
}

// Get the value of a vector's non-struct member. 
static void GetMemberOfVectorOfNonStruct(const FieldDef &field,
                                         std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent + "pub fn " + field.name;
  if (field.value.type.VectorType().base_type == BASE_TYPE_STRING) {
    code += "(&self) -> flatbuffers::Iterator<";
    code += TypeName(field) + "> {\n";
  } else {
    code += "(&self) -> &[";
    code += TypeName(field) + "] {\n";
  }
  
  code += OffsetPrefix(field);
  code += Indent + Indent + Indent;
  code += "return " + GenGetter(field.value.type);
  code += "offset);\n";
  code += Indent + Indent + "}\n";
  if (field.value.type.VectorType().base_type == BASE_TYPE_STRING) {
    code += Indent + Indent + "flatbuffers::empty_iterator(&self.table)";
  } else {
    code += Indent +Indent + "&[]\n";
  }
  code += Indent + "}\n\n";
}

// Begin the creator function signature.
static void BeginBuilderArgs(const StructDef &struct_def,
                             std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string lname = struct_def.name;
  std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
  code += "pub fn build_" + lname;
  code += "(builder: &mut flatbuffers::Builder";
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

// Recursively generate struct construction statements and instert manual
// padding.
static void StructBuilderBody(const StructDef &struct_def,
                              const char *nameprefix,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "    builder.prep(" + NumToString(struct_def.minalign) + ", ";
  code += NumToString(struct_def.bytesize) + ");\n";
  for (auto it = struct_def.fields.vec.rbegin();
       it != struct_def.fields.vec.rend();
       ++it) {
    auto &field = **it;
    if (field.padding)
      code += "    builder.pad(" + NumToString(field.padding) + ");\n";
    if (IsStruct(field.value.type)) {
      StructBuilderBody(*field.value.type.struct_def,
                        (nameprefix + (field.name + "_")).c_str(),
                        code_ptr);
    } else {
      code += "    builder.add_" + GenMethod(field) + "(";
      code += nameprefix + field.name + ");\n";
    }
  }
}

// End the builder function
static void EndBuilderBody(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "    builder.offset() as flatbuffers::UOffsetT \n";
  code += "}\n";
}

// Init functions for the table Builder object
static void GenTableBuilderInitFn(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent + "/// Create a new builder.\n";
  code += Indent + "pub fn with_capacity(size: usize) -> Self {\n";
  code += Indent + Indent + "Builder {\n";
  code += Indent + Indent + Indent + "inner: flatbuffers";
  code += "::Builder::with_capacity(size),\n";
  code += Indent + Indent + "}\n";
  code += Indent + "}\n\n";
}

  // Add Default, From, Into and ObjectBuilder traits.
static void GenTableBuilderBoilerPlate(std::string *code_ptr) {
  std::string &code = *code_ptr;
  // Default
  code += "impl Default for Builder {\n";
  code += Indent + "fn default() -> Builder {\n";
  code += Indent + Indent +"Builder::with_capacity(1024)\n";
  code += Indent +"}\n";
  code += "}\n\n";
  // From
  code += "impl From<flatbuffers::Builder> for Builder {\n";
  code += Indent + "fn from(b: flatbuffers::Builder) -> Builder {\n";
  code += Indent + Indent +"Builder {\n";
  code += Indent + Indent + Indent + "inner: b,\n";
  code += Indent + Indent + "}\n";
  code += Indent +"}\n";
  code += "}\n\n";
  // Into
  code += "impl Into<flatbuffers::Builder> for Builder {\n";
  code += Indent + "fn into(self) -> flatbuffers::Builder {\n";
  code += Indent + Indent +"self.inner\n";
  code += Indent +"}\n";
  code += "}\n\n";
  // ObjectBuilder
  code += "impl flatbuffers::ObjectBuilder for Builder {}\n\n";
}

// Get the value of a table's starting offset.
static void GetStartOfTable(const StructDef &struct_def,
                            std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent;
  code += "/// Initialize a new `Builder` for a `";
  code += struct_def.name + "` table.\n";
  code += Indent;
  code += "pub fn start(&mut self) {\n";
  code += Indent + Indent;
  code += "self.inner.start_object(";
  code += NumToString(struct_def.fields.vec.size());
  code += ");\n";
  code += Indent + "}\n\n";
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
  code += "/// Set the value for field `" + field.name + "`.\n";
  code += Indent;
  code += "pub fn add_" + field.name;
  code += "(&mut self, ";
  code += field.name + ": ";
  if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
    code += "flatbuffers::UOffsetT";
  } else {
    code += GenTypeBasic(field.value.type);
  }
  code += ") {\n";
  code += Indent + Indent;
  code += "self.inner.add_slot_";
  code += GenMethod(field) + "(";
  code += NumToString(offset) + ", ";
  code += field.name;
  code += ", " + MapConstant(field);
  code += ")\n";
  code += Indent + "}\n\n";
}

// Set the value of one of the members of a table's vector.
static void BuildVectorOfTable(const FieldDef &field,
                               std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent;
  code += "/// Initializes bookkeeping for writing a new `";
  code += field.name + "` vector.\n";
  code += Indent;
  code += "pub fn start_";
  code += field.name;
  code += "_vector(&mut self, numElems: usize) {\n";
  code += Indent + Indent;
  code += "self.inner.start_vector(";
  auto vector_type = field.value.type.VectorType();
  auto alignment = InlineAlignment(vector_type);
  auto elem_size = InlineSize(vector_type);
  code += NumToString(elem_size);
  code += ", numElems, " + NumToString(alignment);
  code += ")\n";
  code += Indent + "}\n\n";
}

// Get the offset of the end of a table.
static void GenEndOfTable(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent;
  code += "/// Finalize the current object and return the offset.\n";
  code += Indent;
  code += "pub fn end";
  code += "(&mut self) -> flatbuffers::UOffsetT {\n";
  code += Indent + Indent + "return self.inner.end_object()\n";
  code += Indent + "}\n\n";
}

// Get the offset of the end of a table.
static void GenFinishOnTable(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent;
  code += "/// Finish the buffer.\n";
  code += Indent;
  code += "pub fn finish";
  code += "(&mut self, r: flatbuffers::UOffsetT) {\n";
  code += Indent + Indent + "return self.inner.finish(r)\n";
  code += Indent + "}\n\n";
}

// Generic builder functions.
// This table builder wraps an inner flatbuffers:Builder
// so these are conveniance functions that delegate to the
// inner builder
static void GenBuilderFns(std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string inner = "self.inner.";
  code += Indent;
  code += "pub fn create_string(&mut self, v: &str) -> flatbuffers::UOffsetT { ";
  code += inner +"create_string(v) }\n";
  code += Indent;
  code += "pub fn get_bytes(&self) -> &[u8] { ";
  code += inner +"get_bytes() }\n";
  code += Indent;
  code += "pub fn len(&self) -> usize { ";
  code += inner +"len() }\n";
  code += Indent;
  code += "pub fn offset(&self) -> usize { ";
  code += inner +"offset() }\n";
  code += Indent;
  code += "pub fn reset(&mut self) { ";
  code += inner +"reset() }\n";
  code += Indent;
  code += "pub fn prep(&mut self, s:usize, a: usize) { ";
  code += inner +"prep(s,a) }\n";
  code += Indent;
  code += "pub fn pad(&mut self, n:usize) { ";
  code += inner +"pad(n) }\n";
  code += Indent;
  code += "pub fn end_vector(&mut self) -> flatbuffers::UOffsetT { ";
  code += inner +"end_vector() }\n";

  code += Indent;
  code += "pub fn add_bool(&mut self, v:bool) { ";
  code += inner +"add_bool(v) }\n";
  code += Indent;
  code += "pub fn add_u8(&mut self, v:u8) { ";
  code += inner +"add_u8(v) }\n";
  code += Indent;
  code += "pub fn add_i8(&mut self, v:i8) { ";
  code += inner +"add_i8(v) }\n";
  code += Indent;
  code += "pub fn add_u16(&mut self, v:u16) { ";
  code += inner +"add_u16(v) }\n";
  code += Indent;
  code += "pub fn add_i16(&mut self, v:i16) { ";
  code += inner +"add_i16(v) }\n";
  code += Indent;
  code += "pub fn add_u32(&mut self, v:u32) { ";
  code += inner +"add_u32(v) }\n";
  code += Indent;
  code += "pub fn add_i32(&mut self, v:i32) { ";
  code += inner +"add_i32(v) }\n";
  code += Indent;
  code += "pub fn add_u64(&mut self, v:u64) { ";
  code += inner +"add_u64(v) }\n";
  code += Indent;
  code += "pub fn add_i64(&mut self, v:i64) { ";
  code += inner +"add_i64(v) }\n";
  code += Indent;
  code += "pub fn add_f32(&mut self, v:f32) { ";
  code += inner +"add_f32(v) }\n";
  code += Indent;
  code += "pub fn add_f64(&mut self, v:f64) { ";
  code += inner +"add_f64(v) }\n";
  code += Indent;
  code += "pub fn add_uoffset(&mut self, v: flatbuffers::UOffsetT) { ";
  code += inner +"add_uoffset(v) }\n";

  code += Indent;
  code += "pub fn add_slot_bool(&mut self, o: usize, v: bool, d: bool) { ";
  code += inner +"add_slot_bool(o,v,d) }\n";
  code += Indent;
  code += "pub fn add_slot_u8(&mut self, o: usize, v: u8, d: u8) { ";
  code += inner +"add_slot_u8(o,v,d) }\n";
  code += Indent;
  code += "pub fn add_slot_i8(&mut self, o: usize, v: i8, d: i8) { ";
  code += inner +"add_slot_i8(o,v,d) }\n";
  code += Indent;
  code += "pub fn add_slot_u16(&mut self, o: usize, v: u16, d: u16) { ";
  code += inner +"add_slot_u16(o,v,d) }\n";
  code += Indent;
  code += "pub fn add_slot_i16(&mut self, o: usize, v: i16, d: i16) { ";
  code += inner +"add_slot_i16(o,v,d) }\n";
  code += Indent;
  code += "pub fn add_slot_i32(&mut self, o: usize, v: i32, d: i32) { ";
  code += inner +"add_slot_i32(o,v,d) }\n";
  code += Indent;
  code += "pub fn add_slot_u32(&mut self, o: usize, v: u32, d: u32) { ";
  code += inner +"add_slot_u32(o,v,d) }\n";
  code += Indent;
  code += "pub fn add_slot_u64(&mut self, o: usize, v: u64, d: u64) { ";
  code += inner +"add_slot_u64(o,v,d) }\n";
  code += Indent;
  code += "pub fn add_slot_i64(&mut self, o: usize, v: i64, d: i64) { ";
  code += inner +"add_slot_i64(o,v,d) }\n";
  code += Indent;
  code += "pub fn add_slot_f32(&mut self, o: usize, v: f32, d: f32) { ";
  code += inner +"add_slot_f32(o,v,d) }\n";
  code += Indent;
  code += "pub fn add_slot_f64(&mut self, o: usize, v: f64, d: f64) { ";
  code += inner +"add_slot_f64(o,v,d) }\n";
  code += Indent;
  code += "pub fn add_slot_uoffset(&mut self, o: usize, v: ";
  code += "flatbuffers::UOffsetT, d: flatbuffers::UOffsetT) { ";
  code += inner +"add_slot_uoffset(o,v,d) }\n";
  code += Indent;
  code += "pub fn add_slot_struct(&mut self, o: usize, v: ";
  code += "flatbuffers::UOffsetT, d: flatbuffers::UOffsetT) { ";
  code += inner +"add_slot_struct(o,v,d) }\n";
}

// Generate a struct field, conditioned on its child type(s).
static void GenStructAccessor(const StructDef &struct_def,
                              const FieldDef &field,
                              std::string *code_ptr) {
  GenComment(field.doc_comment, code_ptr, nullptr, Indent.c_str());
  if (IsScalar(field.value.type.base_type)) {
      if (struct_def.fixed) {
        GetScalarFieldOfStruct(field, code_ptr);
      } else {
        GetScalarFieldOfTable(field, code_ptr);
      }
  } else {
    switch (field.value.type.base_type) {
    case BASE_TYPE_STRUCT:
        if (struct_def.fixed) {
         GetStructFieldOfStruct(field, code_ptr);
       } else {
          GetStructFieldOfTable(field, code_ptr);
        }
        break;
    case BASE_TYPE_STRING:
      GetStringField(field, code_ptr);
      break;
    case BASE_TYPE_VECTOR: {
      auto vectortype = field.value.type.VectorType();
      if (vectortype.base_type == BASE_TYPE_STRUCT) {
        GetMemberOfVectorOfStruct(field, code_ptr);
      } else {
        GetMemberOfVectorOfNonStruct(field, code_ptr);
      }
      break;
      }
    case BASE_TYPE_UNION:
      GetUnionField(field, code_ptr);
      break;
    default:
      assert(0);
    }
  }
}

// Generate table constructors, conditioned on its members' types.
static void GenTableBuilderStructImpl(const StructDef &struct_def,
                             std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "/// Builder Object for `"+ struct_def.name + "` tables.\n";
  code += "pub struct Builder {\n";
  code += Indent + "inner: flatbuffers::Builder,\n";
  code += "}\n\n";
  code += "impl Builder {\n";
}

// Generate table constructors, conditioned on its members' types.
static void GenEndTableBuilderStructImpl(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "}\n\n";
}

// Build helper functions for simple fixed structs
static void GenTableBuilderFxedFnImpl(const StructDef &struct_def,
                                         std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string oname = struct_def.name;
  std::string lname = struct_def.name;
  std::transform(lname.begin(), lname.end(),
                 lname.begin(), ::tolower);
  code += Indent + "pub fn build_" + lname + "(&mut self ";
  StructBuilderArgs(struct_def, "", code_ptr);
  code += ") -> flatbuffers::UOffsetT {\n";
  code += Indent + Indent + ModName(oname) + "::build_";
  code += lname + "(&mut self.inner";
  StructBuilderArgs(struct_def, "", code_ptr, true);
  code += ")\n";
  code += Indent + "}\n\n";
}

  

// Build helper functions for simple fixed structs
static void GenTableBuilderFxedFns(const StructDef &struct_def,
                                   std::string *code_ptr) {
  std::set<std::string> generated;
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (field.deprecated) continue;
    if (field.value.type.base_type == BASE_TYPE_VECTOR) {
      if (field.value.type.element == BASE_TYPE_STRUCT) {
        if (field.value.type.struct_def->fixed) {
          std::string id = field.value.type.struct_def->name;
          auto result_1 = generated.insert(id);
          if (result_1.second) {
            GenTableBuilderFxedFnImpl(*field.value.type.struct_def, code_ptr);
          }
        }
      }
    }
    if (field.value.type.base_type == BASE_TYPE_STRUCT) {
      if (field.value.type.struct_def->fixed) {
        std::string id = field.value.type.struct_def->name;
        auto result_1 = generated.insert(id);
        if (result_1.second) {
          GenTableBuilderFxedFnImpl(*field.value.type.struct_def, code_ptr);
        }
      } 
    }
  }
}

// Generate table constructors, conditioned on its members' types.
static void GenTableBuilders(const StructDef &struct_def,
                             std::string *code_ptr) {
  GenTableBuilderStructImpl(struct_def, code_ptr);
  GenTableBuilderInitFn(code_ptr);
  GetStartOfTable(struct_def, code_ptr);
  GenEndOfTable(code_ptr);
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
  GenTableBuilderFxedFns(struct_def, code_ptr);
  GenFinishOnTable(code_ptr);
  GenBuilderFns(code_ptr);
  GenEndTableBuilderStructImpl(code_ptr);
  GenTableBuilderBoilerPlate(code_ptr);
}

// Generate a From<&framebuffer:Table> impl
static void GenTableFromImpl(const StructDef &struct_def,
                             std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "impl<'a> From<flatbuffers::Table<'a>> for ";
  code +=  struct_def.name + "<'a> {\n";
  code += Indent + "fn  from(table: flatbuffers::Table) -> ";
  code += struct_def.name + " {\n";
  code += Indent + Indent + struct_def.name + "{\n";
  code += Indent + Indent + Indent + "table: table,\n";
  code += Indent + Indent + "}\n";
  code += Indent + "}\n";
  code += "}\n\n";
  }

// Generate struct or table methods.
static void GenStruct(const StructDef &struct_def,
                      std::string *code_ptr) {
  if (struct_def.generated) return;
  GenComment(struct_def.doc_comment, code_ptr, nullptr);
  
  TableStructDefinition(struct_def, code_ptr);
  VtableDefinition(struct_def, code_ptr);
  BeginTableImpl(struct_def, code_ptr);
  NewRootTypeFromBuffer(struct_def, code_ptr);
  // Generate the Init method that sets the field in a pre-existing
  // accessor object. This is to allow object reuse.
  //InitializeExisting(struct_def, code_ptr);

  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (field.deprecated) continue;
    GenStructAccessor(struct_def, field, code_ptr);
  }
  EndImpl(code_ptr);
  GenTableFromImpl(struct_def, code_ptr);

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

  GenComment(enum_def.doc_comment, code_ptr, nullptr);
  BeginEnum(enum_def, code_ptr);
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    GenComment(ev.doc_comment, code_ptr, nullptr, Indent.c_str());
    EnumMember(ev, code_ptr);
  }
  EndEnum(code_ptr);
  EnumImpl(enum_def, code_ptr);
  EnumFromImpl(enum_def, code_ptr);
  if (enum_def.is_union) {
    EnumUnionImpl(enum_def, code_ptr); 
  }
}

// Returns the function name that is able to read a value of the given type.
static std::string GenGetter(const Type &type) {
  switch (type.base_type) {
  case BASE_TYPE_STRING: return "self.table.get_str(";
  case BASE_TYPE_UNION: return "let obj = self.table  \
     .get_union::<"+  type.enum_def->name  + ">(";
  case BASE_TYPE_VECTOR:
    if (type.VectorType().base_type == BASE_TYPE_STRING) {
      return "self.table.str_vector(";
    } else if (type.VectorType().base_type==BASE_TYPE_UCHAR) { 
      return "self.table.byte_vector(";
    } else if (type.VectorType().base_type==BASE_TYPE_BOOL) { 
      return "self.table.bool_vector(";
    } else if (type.VectorType().base_type==BASE_TYPE_CHAR) { 
      return "self.table.ibyte_vector(";
    }
  default:
    return "self.table.get_" +  \
      GenTypeGet(type) + "(";
  }
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
      if (struct_def.fixed) {
        std::string lname = struct_def.name;
        std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
        re_exports += "pub use self::" + mod_name + "::{";
        re_exports += struct_def.name + ", build_" + lname +"};\n"; 
      } else {
        re_exports += "pub use self::" + mod_name + "::";
        re_exports += struct_def.name + ";\n"; 
      }
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
                     const std::string &classcode, const std::string &path,
                     bool needs_imports) {
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
  BeginFile(needs_imports, &code);
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
      return type.enum_def->name + "Union";
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
      bool import = enum_def.is_union;
      if (!SaveType(parser_, enum_def, enumcode, path_, import)) return false;
    }
    return true;
  }

  bool generateStructs() {
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      auto &struct_def = **it;
      std::string declcode;
      GenStruct(struct_def, &declcode);
      if (!SaveType(parser_, struct_def, declcode, path_, true)) return false;
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
