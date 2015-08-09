/*
 * Copyright 2015 Google Inc. All rights reserved.
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
#include <vector>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {
namespace js {

static std::string GenGetter(const Type &type);
static std::string GenMethod(const FieldDef &field);
static void GenStructBuilder(const StructDef &struct_def,
                             std::string *code_ptr);
static void GenReceiver(const StructDef &struct_def, std::string *code_ptr);
static std::string GenTypeBasic(const Type &type);
static std::string GenTypeGet(const Type &type);
static std::string ImportName(const FieldDef &field);

std::string MakeObjectName(const std::string &in) {
	return MakeCamel(in, false);
}

// Hardcode spaces per indentation.
const std::string Indent = "    ";

// Most field accessors need to retrieve and test the field offset first,
// this is the prefix code for that.
std::string OffsetPrefix(const FieldDef &field) {
  return
	  Indent + "var o = this._table.getOffset(" + NumToString(field.value.offset) + ");\n" +
	  Indent + "if (o != 0) {\n";
}

// Begin by declaring namespace and imports.
static void BeginFile(const std::string name_space_name,
                      const bool needs_imports,
                      std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "// automatically generated, do not modify\n\n";
  code += "// namespace: " + name_space_name + "\n\n";
  if (needs_imports) {
    code += "var flatBuffers = require('flatbuffers');\n\n";
  }
}

static void Imports(const StructDef &struct_def, std::vector<const StructDef *> visitedTypes, std::vector<std::string>& out_types) {
	// grab the list of struct types
	for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
		auto &field = **it;
		if (field.value.type.base_type != BASE_TYPE_STRUCT) {
			continue;
		}
		if (std::find(visitedTypes.begin(), visitedTypes.end(), field.value.type.struct_def) != visitedTypes.end())	{
			continue;
		}
		auto type = field.value.type.struct_def->name;
		if (std::find(out_types.begin(), out_types.end(), type) == out_types.end()) {
			out_types.push_back(type);
		}
		// Recurse...
		visitedTypes.push_back(field.value.type.struct_def);
		Imports(*field.value.type.struct_def, visitedTypes, out_types);		
	}
}

static void Imports(const StructDef &struct_def, std::string *code_ptr) {
	std::string &code = *code_ptr;
	code += "var imports = {};\n";
	// recursively find the referenced types to build the import map
	std::vector<const StructDef *> visitedTypes;
	std::vector<std::string> types;
	Imports(struct_def, visitedTypes, types);

	for(auto it = types.begin();
		it != types.end();
		++it) {
		auto &type = *it;
		auto lowerType = type;
		std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(),::tolower);
		code += "imports." + lowerType + " = require('./" + lowerType + "');\n";
	}
	code += "\n";
	if (!types.empty()) {
		code += "\n";
	}
}

// Begin a class declaration.
static void BeginClass(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "exports." + struct_def.name + " = function(fb, pos) {\n";
  code += Indent + "this._fb = fb;\n";
  code += Indent + "this._pos = pos;\n";
  if (!struct_def.fixed) {
	code += Indent + "this._table = new flatBuffers.Table(fb, pos);\n";
  }
  code += "};\n\n";
  code += "exports." + struct_def.name + "Builder = {};\n\n";
}

// Begin enum code with a class declaration.
static void BeginEnum(const std::string class_name, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "exports." + MakeObjectName(class_name) + " = {\n";
}

// A single enum member.
static void EnumMember(const EnumVal ev, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent;
  code += ev.name;
  code += ": ";
  code += NumToString(ev.value) + ",\n";
}

// End enum code.
static void EndEnum(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "};\n";
}

// Initialize a new struct or table from existing data.
static void NewRootTypeFromBuffer(const StructDef &struct_def,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "exports.getRootAs" + struct_def.name + " = function(buf, offset) {\n";
  code += Indent + "return new exports."+ struct_def.name + "(buf, buf.getInt32(offset) + offset);\n";
  code += "}\n\n";
}

// Initialize an existing object with other data, to avoid an allocation.
static void InitializeExisting(const StructDef &struct_def,
                               std::string *code_ptr) {
  std::string &code = *code_ptr;

  GenReceiver(struct_def, code_ptr);
  code += "init = function(fb, pos) {\n";
  code += Indent + "this._fb = fb;\n";
  code += Indent + "this._pos = pos;\n";
  if (!struct_def.fixed) {
	code += Indent + "this._table = new flatBuffers.Table(fb, pos);\n";
  }
  code += Indent + "return this;\n";
  code += "};\n\n";
  GenReceiver(struct_def, code_ptr);
  code += "constructor = exports." + struct_def.name + ";\n\n";
}

// Get the length of a vector.
static void GetVectorLen(const StructDef &struct_def,
                         const FieldDef &field,
                         std::string *code_ptr) {
  std::string &code = *code_ptr;

  GenReceiver(struct_def, code_ptr);
  code += "get" + MakeCamel(field.name) + "Length = function() {\n";
  code += OffsetPrefix(field);
  code += Indent + Indent + "return this._table.getVectorLen(o);\n";
  code += Indent + "}\n";
  code += Indent + "return 0;\n";
  code += "}\n\n";
}

// Get the value of a struct's scalar.
static void GetScalarFieldOfStruct(const StructDef &struct_def,
                                   const FieldDef &field,
                                   std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string getter = GenGetter(field.value.type);
  GenReceiver(struct_def, code_ptr);
  code += "get" + MakeCamel(field.name);
  code += " = function() {\n";
  code += Indent + "return " + getter;
  code += "this._pos + ";
  code += NumToString(field.value.offset) + ");\n";
  code += "};\n\n";
}

// Get the value of a table's scalar.
static void GetScalarFieldOfTable(const StructDef &struct_def,
                                  const FieldDef &field,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string getter = GenGetter(field.value.type);
  GenReceiver(struct_def, code_ptr);
  code += "get" + MakeCamel(field.name) + " = function() {\n";
  code += OffsetPrefix(field);
  code += Indent + Indent + "return " + getter;
  code += "o + this._pos);\n";
  code += Indent + "}\n";
  code += Indent + "return " + field.value.constant + ";\n";
  code += "};\n\n";
}

// Get a struct by initializing an existing struct.
// Specific to Struct.
static void GetStructFieldOfStruct(const StructDef &struct_def,
                                   const FieldDef &field,
                                   std::string *code_ptr) {
  std::string &code = *code_ptr;
  GenReceiver(struct_def, code_ptr);
  code += "get" + MakeCamel(field.name) + " = function(obj) {\n";
  code += Indent + "if (typeof obj === 'undefined') {\n";
  code += Indent + Indent + "obj = new " + ImportName(field) + "();\n";
  code += Indent + "};\n";
  code += Indent + "obj.init(this._fb, this._pos + ";
  code += NumToString(field.value.offset) + ")";
  code += "\n" + Indent + "return obj;\n";
  code += "};\n";
}

// Get a struct by initializing an existing struct.
// Specific to Table.
static void GetStructFieldOfTable(const StructDef &struct_def,
                                  const FieldDef &field,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  GenReceiver(struct_def, code_ptr);
  code += "get" + MakeCamel(field.name);
  code += " = function(obj) {\n";
  code += OffsetPrefix(field);
  code += Indent + Indent + "if (typeof obj === 'undefined') {\n";
  code += Indent + Indent + Indent + "obj = new " + ImportName(field) + "();\n";
  code += Indent + Indent + "};\n";
  if (field.value.type.struct_def->fixed) {
	code += Indent + Indent + "return obj.init(this._fb, o + this._pos);\n";
  } else {
    code += Indent + Indent + "return obj.init(this._fb, this._table.getIndirect(o + this._pos));\n";
  }
  code += Indent + "}\n";
  code += Indent + "return null;\n";
  
  code += "};\n";
}

// Get the value of a string.
static void GetStringField(const StructDef &struct_def,
                           const FieldDef &field,
                           std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string getter = GenGetter(field.value.type);
  GenReceiver(struct_def, code_ptr);
  code += "get" + MakeCamel(field.name) + " = function() {\n";
  code += OffsetPrefix(field);
  code += Indent + Indent + "return " + getter;
  code += "o + this._pos);\n";
  code += Indent + "}\n";
  code += Indent + "return " + field.value.constant + ";\n";
  code += "};\n\n";
}

// Get the value of a union from an object.
static void GetUnionField(const StructDef &struct_def,
                          const FieldDef &field,
                          std::string *code_ptr) {
  std::string &code = *code_ptr;
  GenReceiver(struct_def, code_ptr);
  code += "get" + MakeCamel(field.name) + " = function(obj) {\n";
  code += OffsetPrefix(field);

  code += Indent + Indent + "if (typeof obj === 'undefined') {\n";
  code += Indent + Indent + Indent + "obj = new flatBuffers.Table();\n";
  code += Indent + Indent + "};\n";
  code += Indent + Indent + "return this._table.getUnion(obj, o);\n";
  code += Indent + "}\n";
  code += Indent + "return null;\n";
  code += "};\n\n";
}

// Get the value of a vector's struct member.
static void GetMemberOfVectorOfStruct(const StructDef &struct_def,
                                      const FieldDef &field,
                                      std::string *code_ptr) {
  std::string &code = *code_ptr;
  auto vectortype = field.value.type.VectorType();

  GenReceiver(struct_def, code_ptr);
  code += "get" + MakeCamel(field.name) + " = function(i, obj) {\n";
  code += OffsetPrefix(field);
  code += Indent + Indent + "if (typeof obj === 'undefined') {\n";
  code += Indent + Indent + Indent + "obj = new " + ImportName(field) + "();\n";
  code += Indent + Indent + "};\n";
  code += Indent + Indent + "return obj.init(this._fb, this._table.getVector(o) + i * ";
  code += NumToString(InlineSize(vectortype)) + ");\n";
  code += Indent + "}\n";
  code += Indent + "return null;\n";
  code += "}\n\n";
}

// Get the value of a vector's non-struct member. Uses a named return
// argument to conveniently set the zero value for the result.
static void GetMemberOfVectorOfNonStruct(const StructDef &struct_def,
                                         const FieldDef &field,
                                         std::string *code_ptr) {
  std::string &code = *code_ptr;
  auto vectortype = field.value.type.VectorType();

  GenReceiver(struct_def, code_ptr);
  code += "get" + MakeCamel(field.name) + " = function(i) {\n";
  code += OffsetPrefix(field);
  code += Indent + Indent + "return " + GenGetter(field.value.type);
  code += "this._table.getVector(o) + i * " + NumToString(InlineSize(vectortype)) + ");\n";
  code += Indent + "}\n";
  if (vectortype.base_type == BASE_TYPE_STRING) {
    code += Indent + "return \'\';\n";
  } else {
    code += Indent + "return 0;\n";
  }
  code += "};\n";
}

// Begin the creator function signature.
static void BeginBuilderArgs(const StructDef &struct_def,
                             std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "\n";
  code += "exports.create" + struct_def.name;
  code += " = function(builder";
}

// Recursively generate arguments for a constructor, to deal with nested
// structs.
static void StructBuilderArgs(const StructDef &struct_def,
                              const char *nameprefix,
                              std::string *code_ptr) {
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
                        code_ptr);
    } else {
      std::string &code = *code_ptr;
      code += (std::string)", " + nameprefix;
      code += MakeCamel(field.name, false);
    }
  }
}

// End the creator function signature.
static void EndBuilderArgs(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += ") {\n";
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
      code += "    builder.prepend" + GenMethod(field) + "(";
      code += nameprefix + MakeCamel(field.name, false) + ");\n";
    }
  }
}

static void EndBuilderBody(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "    return builder.getOffset();\n";
  code += "};\n";
}

// Get the value of a table's starting offset.
static void GetStartOfTable(const StructDef &struct_def,
                            std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "exports." + struct_def.name + "Builder.start = function(builder) {\n";
  code += Indent + "builder.startObject(";
  code += NumToString(struct_def.fields.vec.size());
  code += ");\n";
  code += "};\n";
}

// Set the value of a table's field.
static void BuildFieldOfTable(const StructDef &struct_def,
                              const FieldDef &field,
                              const size_t offset,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "exports." + struct_def.name + "Builder.add" + MakeCamel(field.name) + " = function(builder, ";
  code += MakeCamel(field.name, false) + ") {\n";
  code += Indent + "builder.add" + GenMethod(field) + "(";
  code += NumToString(offset) + ", ";
  code += MakeCamel(field.name, false);
  code += ", " + field.value.constant;
  code += ");\n";
  code += "};\n";
}

// Set the value of one of the members of a table's vector.
static void BuildVectorOfTable(const StructDef &struct_def,
                               const FieldDef &field,
                               std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "exports." + struct_def.name + "Builder.start"+ MakeCamel(field.name)  + "Vector = function(builder, numElems) {\n";
  code += Indent + "return builder.startVector(";
  auto vector_type = field.value.type.VectorType();
  auto alignment = InlineAlignment(vector_type);
  auto elem_size = InlineSize(vector_type);
  code += NumToString(elem_size);
  code += ", numElems, " + NumToString(alignment);
  code += ");\n";
  code += "};\n";
}

// Get the offset of the end of a table.
static void GetEndOffsetOnTable(const StructDef &struct_def,
                                std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "exports." + struct_def.name + "Builder.end = function(builder) {\n";
  code += Indent + "return builder.endObject();\n";
  code += "};";
}

// Generate the receiver for function signatures.
static void GenReceiver(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "exports." + struct_def.name + ".prototype.";
}

// Generate a struct field, conditioned on its child type(s).
static void GenStructAccessor(const StructDef &struct_def,
                              const FieldDef &field,
                              std::string *code_ptr) {
  GenComment(field.doc_comment, code_ptr, nullptr);
  if (IsScalar(field.value.type.base_type)) {
    if (struct_def.fixed) {
      GetScalarFieldOfStruct(struct_def, field, code_ptr);
    } else {
      GetScalarFieldOfTable(struct_def, field, code_ptr);
    }
  } else {
    switch (field.value.type.base_type) {
      case BASE_TYPE_STRUCT:
        if (struct_def.fixed) {
          GetStructFieldOfStruct(struct_def, field, code_ptr);
        } else {
          GetStructFieldOfTable(struct_def, field, code_ptr);
        }
        break;
      case BASE_TYPE_STRING:
        GetStringField(struct_def, field, code_ptr);
        break;
      case BASE_TYPE_VECTOR: {
        auto vectortype = field.value.type.VectorType();
        if (vectortype.base_type == BASE_TYPE_STRUCT) {
          GetMemberOfVectorOfStruct(struct_def, field, code_ptr);
        } else {
          GetMemberOfVectorOfNonStruct(struct_def, field, code_ptr);
        }
        break;
      }
      case BASE_TYPE_UNION:
        GetUnionField(struct_def, field, code_ptr);
        break;
      default:
        assert(0);
    }
  }
  if (field.value.type.base_type == BASE_TYPE_VECTOR) {
    GetVectorLen(struct_def, field, code_ptr);
  }
}

// Generate table constructors, conditioned on its members' types.
static void GenTableBuilders(const StructDef &struct_def,
                             std::string *code_ptr) {
  GetStartOfTable(struct_def, code_ptr);

  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (field.deprecated) continue;

    auto offset = it - struct_def.fields.vec.begin();
    BuildFieldOfTable(struct_def, field, offset, code_ptr);
    if (field.value.type.base_type == BASE_TYPE_VECTOR) {
      BuildVectorOfTable(struct_def, field, code_ptr);
    }
  }

  GetEndOffsetOnTable(struct_def, code_ptr);
}

// Generate struct or table methods.
static void GenStruct(const StructDef &struct_def,
                      std::string *code_ptr,
                      StructDef *root_struct_def) {
  if (struct_def.generated) return;

  GenComment(struct_def.doc_comment, code_ptr, nullptr);
  Imports(struct_def, code_ptr);
  BeginClass(struct_def, code_ptr);

  // Generate the Init method that sets the field in a pre-existing
  // accessor object. This is to allow object reuse.
  InitializeExisting(struct_def, code_ptr);
  if (&struct_def == root_struct_def) {
    // Generate a special accessor for the table that has been declared as
    // the root type.
    NewRootTypeFromBuffer(struct_def, code_ptr);
  }  
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (field.deprecated) continue;

    GenStructAccessor(struct_def, field, code_ptr);
  }

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

  GenComment(enum_def.doc_comment, code_ptr, nullptr, "# ");
  BeginEnum(enum_def.name, code_ptr);
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    GenComment(ev.doc_comment, code_ptr, nullptr, "# ");
    EnumMember(ev, code_ptr);
  }
  EndEnum(code_ptr);
}

// Returns the function name that is able to read a value of the given type.
static std::string GenGetter(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING: return "this._fb.getString(";
    case BASE_TYPE_UNION: return "this._table.getUnion(";
    case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
    default:
      return "this._fb.get" + MakeCamel(GenTypeGet(type)) + "(";
  }
}

// Returns the method name for use with add/put calls.
static std::string GenMethod(const FieldDef &field) {
  return IsScalar(field.value.type.base_type)
    ? MakeCamel(GenTypeBasic(field.value.type))
    : (IsStruct(field.value.type) ? "Struct" : "Offset");
}


// Save out the generated code for a Python Table type.
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
    EnsureDirExists(namespace_dir.c_str());
  }


  std::string code = "";
  BeginFile(namespace_name, needs_imports, &code);
  code += classcode;
  std::string filename = def.name + ".js";
  std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
  std::string filepath = namespace_dir + kPathSeparator + filename;
  return SaveFile(filepath.c_str(), code, false);
}

static std::string GenTypeBasic(const Type &type) {
  static const char *ctypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, JSTYPE) \
      #JSTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };
  return ctypename[type.base_type];
}

static std::string GenTypePointer(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING:
      return "string";
    case BASE_TYPE_VECTOR:
      return GenTypeGet(type.VectorType());
    case BASE_TYPE_STRUCT:
      return type.struct_def->name;
    case BASE_TYPE_UNION:
      // fall through
    default:
      return "*flatbuffers.Table";
  }
}

static std::string GenTypeGet(const Type &type) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(type)
    : GenTypePointer(type);
}

static std::string ImportName(const FieldDef &field) {
  auto type = field.value.type.struct_def->name;
  auto lowerType = type;
  std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);
  return "imports." + lowerType + "." + type;
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

}  // namespace js

bool GenerateJavascript(const Parser &parser,
                    const std::string &path,
                    const std::string & /*file_name*/,
                    const GeneratorOptions & /*opts*/) {
  for (auto it = parser.enums_.vec.begin();
       it != parser.enums_.vec.end(); ++it) {
    std::string enumcode;
    js::GenEnum(**it, &enumcode);
    if (!js::SaveType(parser, **it, enumcode, path, false))
      return false;
  }

  for (auto it = parser.structs_.vec.begin();
       it != parser.structs_.vec.end(); ++it) {
    std::string declcode;
    js::GenStruct(**it, &declcode, parser.root_struct_def_);
    if (!js::SaveType(parser, **it, declcode, path, true))
      return false;
  }

  return true;
}

}  // namespace flatbuffers


