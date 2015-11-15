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

#ifdef _WIN32
#include <direct.h>
#define PATH_SEPARATOR "\\"
#define mkdir(n, m) _mkdir(n)
#else
#include <sys/stat.h>
#define PATH_SEPARATOR "/"
#endif

namespace flatbuffers {
namespace kotlin {

static std::string GenGetter(const Type &type);
static std::string GenSetter(const Type &type);
static std::string GenMethod(const FieldDef &field);
static void GenStructBuilder(const StructDef &struct_def,
                             std::string *code_ptr);
static void GenReceiver(const StructDef &struct_def, std::string *code_ptr);
static std::string GenTypeBasic(const Type &type);
static std::string GenTypeGet(const Type &type);

static std::string TypeName(const FieldDef &field);
static std::string Zero(const FieldDef &field);

static std::string DestinationCast(const Type &type) ;
static std::string DestinationMask(const Type &type, bool vectorelem);

// Most field accessors need to retrieve and test the field offset first,
// this is the prefix code for that.
std::string OffsetPrefix(const FieldDef &field) {
  return "{\n\to = _offset(" +NumToString(field.value.offset) +")\n if (o != 0) {\n";
}

// Begin by declaring namespace and imports.
static void BeginFile(const std::string name_space_name,
                      const bool needs_imports,
                      std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "// automatically generated, do not modify\n\n";
  code += "package " + name_space_name + "\n\n";
  if (needs_imports) code += "import java.nio.*;\nimport com.google.flatbuffers.kotlin.*;\n\n";
 
}



// Begin enum code with a class declaration.
static void BeginEnum(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "const (\n";
}

// A single enum member.
static void EnumMember(const EnumDef &enum_def, const EnumVal ev,
                       std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\t";
  code += enum_def.name;
  code += ev.name;
  code += " = ";
  code += NumToString(ev.value) + "\n";
}

// End enum code.
static void EndEnum(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += ")\n";
}

// Initialize a new struct or table from existing data.
static void NewRootTypeFromBuffer(const StructDef &struct_def,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
// DONE
  code += "fun rootAs";
  code += struct_def.name;
  code += "(byteBuffer : ByteBuffer, reuse : " + struct_def.name + "? = null) ";
  code += " : " + struct_def.name;
  code += " {\n";
  code += "\tbyteBuffer.order(ByteOrder.LITTLE_ENDIAN);\n";
  code += "\t return (reuse ?: " + struct_def.name + "()).wrap(byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position(), byteBuffer)\n";
  code +=  "}\n\n";
}

// Initialize an existing object with other data, to avoid an allocation.
static void InitializeExisting(const StructDef &struct_def,
                               std::string *code_ptr) {
  std::string &code = *code_ptr;
// DONE
  code += "fun wrap(position : Int, byteBuffer : ByteBuffer) : " + struct_def.name + " = apply {\n";
  code +=  "\t\t_position = position\n";
  code +=  "\t\t_byteBuffer = byteBuffer\n";
  code +=  "\t}\n\n";
}

// Begin a class declaration.
static void BeginClass(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "public open class " + struct_def.name + " : Table() {\n\t";
}

// Begin a class companion object (for static methods)
static void BeginCompanion(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "companion object {\n\t";
}


// End a class companion object (for static methods)
static void EndCompanion(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\n}\n\n";
}


// End a class declaration.
static void EndClass(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\n}\n\n";
}


// Get the length of a vector.
static void GetVectorLen(
                         const FieldDef &field,
                         std::string *code_ptr) {
  std::string &code = *code_ptr;
// DONE
  code += "public val " + field.name + "Size : Int \n";
  code += "\tget() {\n";
  code += "\t\tval o = _offset(" + NumToString(field.value.offset) + ")\n";
  code += "\t\treturn if (o == 0) 0 else _arraySize(o)\n";
  code +="\t}\n\n";
}

// Get a [ubyte] vector as a byte slice.
static void GetUByteSlice(const StructDef &struct_def,
                          const FieldDef &field,
                          std::string *code_ptr) {
  std::string &code = *code_ptr;

  GenReceiver(struct_def, code_ptr); 

  code += " " + MakeCamel(field.name) + "Bytes(";
  code += ") []byte " + OffsetPrefix(field);
  code += "\t\treturn rcv._tab.ByteVector(o + rcv._tab.Pos)\n\t}\n";
  code += "\treturn nil\n}\n\n";
}

// Get the value of a struct's scalar.
static void GetScalarFieldOfStruct(
                                   const FieldDef &field,
                                   std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string getter = GenGetter(field.value.type);
  std::string setter = GenSetter(field.value.type);
// DONE

  if (field.value.type.base_type == BASE_TYPE_STRING) code += "public val "; else  code += "public var ";
  code+= field.name + " : " + TypeName(field) + "\n";
  code += "\tget() = " + getter + "(position + _offset(" + NumToString(field.value.offset) + ")) }\n\n";

// we could use a var here for stuff different of strings
  if (field.value.type.base_type != BASE_TYPE_STRING) {
     code += "\tset(value) { " + setter + "(position + _offset(" + NumToString(field.value.offset) + "), value) }\n\n";
  }
}

// Get the value of a table's scalar.
static void GetScalarFieldOfTable(
                                  const FieldDef &field,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string getter = GenGetter(field.value.type);
// DONE
  code += "public val " + field.name + " : " + TypeName(field) + "\n";
  code += "\t get() {\n";
  code += "\t\tval o = _offset(" +NumToString(field.value.offset) + ")\n";
  code += "\t\treturn if (o == 0) " + field.value.constant + " else " + getter + "(o + _position)\n";
  code += "\t}\n\n"; 
}

// Get a struct by initializing an existing struct.
// Specific to Struct.
static void GetStructFieldOfStruct(
                                   const FieldDef &field,
                                   std::string *code_ptr) {
  std::string &code = *code_ptr;
//DONE
  code += "public fun " + field.name + "(reuse : " + TypeName(field) + "? = null) : " + TypeName(field) + " = ";
  code += "(reuse?: " + TypeName(field) + "()).wrap(_byteBuffer, _position + " + NumToString(field.value.offset) + ")\n\n";
}

// Get a struct by initializing an existing struct.
// Specific to Table.
static void GetStructFieldOfTable(
                                  const FieldDef &field,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;

  // DONE
  code += "fun " + field.name + "(reuse : " + TypeName(field) + " = null) {\n";
  code += "\tval o = _offset(" +NumToString(field.value.offset) + ")\n";
  code += "\tif (o==0) return null\n";
  if (field.value.type.struct_def->fixed) code += "\tval x = o + _position\n"; else code += "\tval x = _indirect(o + _posistion)\n";
  code += "\t return (reuse ?:" + TypeName(field) + "()).wrap(_byteBuffer, x)\n";
  code += "\t}\n\n";
}

// Get the value of a string.
static void GetStringField(
                           const FieldDef &field,
                           std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "public val " +field.name + " : String?\n";
  code += "\tget() {\n";
  code += "\t\tval o = _offset(" +NumToString(field.value.offset) + ")\n";
  code += "\t\treturn if (o == 0) null else " + GenGetter(field.value.type) + "(o + _position)\n";
  code += "\t}\n\n";
}

// Get the value of a union from an object.
static void GetUnionField(
                          const FieldDef &field,
                          std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "fun " + field.name + "(reuse : " + TypeName(field) + "?=null) :Boolean {\n";
  code += "\tval o = _offset(" +NumToString(field.value.offset) + ")\n";
  code += "\tif (o==0) return false\n";
  code += "\t" + GenGetter(field.value.type) + "(reuse ?:" + TypeName(field) + "(), o)\n";
  code += "\treturn true\n";
  code += "\t}\n\n";
}

// Get the value of a vector's struct member.
static void GetMemberOfVectorOfStruct(
                                      const FieldDef &field,
                                      std::string *code_ptr) {
  std::string &code = *code_ptr;
  auto vectortype = field.value.type.VectorType();

// DONE
  code += field.name + "(j :Int, reuse : " + TypeName(field) + "?=null) : Boolean {";
  code += "\to = _offset(" +NumToString(field.value.offset) + ")\n";
  code += "\tif (o==0) return false";
  code += "\tval x = _array(o) + j * " + NumToString(InlineSize(vectortype)) + "\n";
  code += "\t(reuse ?: " + TypeName(field) +"() ).wrap(_byteBuffer, x";
  if (vectortype.struct_def->fixed) code += ")\n"; else code += " + _indirect(x))\n";
  code += "\treturn true\n\t}\n";
  code += "\t}\n\n";
}

// Get the value of a vector's non-struct member. Uses a named return
// argument to conveniently set the zero value for the result.
static void GetMemberOfVectorOfNonStruct(
                                         const FieldDef &field,
                                         std::string *code_ptr) {
  std::string &code = *code_ptr;
  auto vectortype = field.value.type.VectorType();

  // DONE
  code += "public fun " + field.name + "(j : Int) : " + TypeName(field) + " {\n";
  code += "\tval o = _offset(" + NumToString(field.value.offset) + ")\n";
  if (vectortype.base_type == BASE_TYPE_STRING) code += "\treturn if (o==0) null"; else code += "\treturn if (o==0) " + Zero(field);
  code += " else " + GenGetter(field.value.type) + "(_array(o) + j * " + NumToString(InlineSize(vectortype)) + ")\n";
  code += "\t}\n\n";

  if (vectortype.base_type != BASE_TYPE_STRING) {
    code += "public fun mutate" + MakeCamel(field.name) + "(j : Int, value : " + TypeName(field) + ") :Boolean {\n";
    code += "\tval o = _offset(" + NumToString(field.value.offset) + ")\n";
    code += "\tif (o==0) return  false\n";
    code += "\t" + GenSetter(field.value.type) + "(_array(o) + j * " + NumToString(InlineSize(vectortype)) + ", value ";
    code += DestinationCast(field.value.type) + DestinationMask(field.value.type, true) + " )\n";
    code += "\treturn true\n";
    code += "\t}\n\n";
  }
}

// Begin the creator function signature.
static void BeginBuilderArgs(const StructDef &struct_def,
                             std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "\n";
  code += "fun create" + struct_def.name;
  code += "(builder : FlatBufferBuilder";
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
      code += " : " + GenTypeBasic(field.value.type);
    }
  }
}

// End the creator function signature.
static void EndBuilderArgs(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += ") :Int {\n";
}

// Recursively generate struct construction statements and instert manual
// padding.
static void StructBuilderBody(const StructDef &struct_def,
                              const char *nameprefix,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "    builder.Prep(" + NumToString(struct_def.minalign) + ", ";
  code += NumToString(struct_def.bytesize) + ")\n";
  for (auto it = struct_def.fields.vec.rbegin();
       it != struct_def.fields.vec.rend();
       ++it) {
    auto &field = **it;
    if (field.padding)
      code += "    builder.Pad(" + NumToString(field.padding) + ")\n";
    if (IsStruct(field.value.type)) {
      StructBuilderBody(*field.value.type.struct_def,
                        (nameprefix + (field.name + "_")).c_str(),
                        code_ptr);
    } else {
      code += "    builder.add" + GenMethod(field) + "(";
      code += nameprefix + MakeCamel(field.name, false) + ")\n";
    }
  }
}

static void EndBuilderBody(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "    return builder.offset()\n";
  code += "}\n";
}

// Get the value of a table's starting offset.
static void GetStartOfTable(const StructDef &struct_def,
                            std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "public fun start" + struct_def.name;
  code += "(builder:FlatBufferBuilder) :Unit = ";
  code += "builder.startObject(";
  code += NumToString(struct_def.fields.vec.size());
  code += ")\n";
}

// Set the value of a table's field.
static void BuildFieldOfTable(const StructDef &struct_def,
                              const FieldDef &field,
                              const size_t offset,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "public fun add" + MakeCamel(field.name);
  code += "(builder : FlatBufferBuilder, ";
  code += MakeCamel(field.name, false);
  if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
    code += "Offset : Int ";
  } else {
    code += " : " + GenTypeBasic(field.value.type);
  }
  code += ") { builder.add";
  if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
    code += "Offset(" + NumToString(offset) + ", " + MakeCamel(field.name, false) + "Offset";
  } else {
    code +=  GenMethod(field) + "(" + NumToString(offset) + ", " + MakeCamel(field.name, false);
  }
  code += ", " + field.value.constant + ") }\n";
}

// Set the value of one of the members of a table's vector.
static void BuildVectorOfTable(
                               const FieldDef &field,
                               std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "fun start" + MakeCamel(field.name);
  code += "Array(builder :FlatBufferBuilder, numElems : Int) ";
  code += " : Unit = builder.startArray(";
  auto vector_type = field.value.type.VectorType();
  auto alignment = InlineAlignment(vector_type);
  auto elem_size = InlineSize(vector_type);
  code += NumToString(elem_size);
  code += ", numElems, " + NumToString(alignment);
  code += ")\n\n";
}

// Get the offset of the end of a table.
static void GetEndOffsetOnTable(const StructDef &struct_def,
                                std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "fun end" + struct_def.name + "(builder :FlatBufferBuilder) :Int {\n\t";
  code += "\tval o = builder.endObject()\n\t";
  
  // TODO required fields go there
  // like builder.required(o, 6);  // blueprint

code += "\treturn o\n";
  code += "}";
}

// Generate the receiver for function signatures.
static void GenReceiver(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "fun (rcv " + struct_def.name + ")";
}

// Generate a struct field, conditioned on its child type(s).
static void GenStructAccessor(const StructDef &struct_def,
                              const FieldDef &field,
                              std::string *code_ptr) {
  GenComment(field.doc_comment, code_ptr, nullptr, "");
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
  if (field.value.type.base_type == BASE_TYPE_VECTOR) {
    GetVectorLen(field, code_ptr);
    if (field.value.type.element == BASE_TYPE_UCHAR) {
      GetUByteSlice(struct_def, field, code_ptr);
    }
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
      BuildVectorOfTable(field, code_ptr);
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
  BeginClass(struct_def, code_ptr);
  if (&struct_def == root_struct_def) {
    // Generate a special accessor for the table that has been declared as
    // the root type.
    NewRootTypeFromBuffer(struct_def, code_ptr);
  }
  // Generate the Init method that sets the field in a pre-existing
  // accessor object. This is to allow object reuse.
  InitializeExisting(struct_def, code_ptr);

  for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
    auto &field = **it;
    if (field.deprecated) continue;
    GenStructAccessor(struct_def, field, code_ptr);
  }


  BeginCompanion(code_ptr);
  //if (struct_def.fixed) {
    // create a struct constructor function
    GenStructBuilder(struct_def, code_ptr);
  //} else {
    // Create a set of functions that allow table construction.
    GenTableBuilders(struct_def, code_ptr);
  //}
  EndCompanion(code_ptr);
  EndClass(code_ptr);
}

// Generate enum declarations.
static void GenEnum(const EnumDef &enum_def, std::string *code_ptr) {
  if (enum_def.generated) return;

  GenComment(enum_def.doc_comment, code_ptr, nullptr);
  BeginEnum(code_ptr);
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    GenComment(ev.doc_comment, code_ptr, nullptr, "\t");
    EnumMember(enum_def, ev, code_ptr);
  }
  EndEnum(code_ptr);
}

// Returns the function name that is able to read a value of the given type.
static std::string GenGetter(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING: return "_string";
    case BASE_TYPE_UNION: return "_union";
    case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
    default:
      return "_byteBuffer.get" + GenTypeGet(type);
  }
}



// Returns the function name that is able to read a value of the given type.
static std::string GenSetter(const Type &type) {
  switch (type.base_type) {
    //case BASE_TYPE_STRING: return "_string";
    //case BASE_TYPE_UNION: return "_union";
    // case BASE_TYPE_VECTOR: return "_byteBuffer.put" + GenTypeGet(type.VectorType());
    default:
      return "_byteBuffer.put" + GenTypeGet(type);
  }
}



// Casts necessary to correctly read serialized data
static std::string DestinationCast(const Type &type) {
      // Cast necessary to correctly read serialized unsigned values.
  switch (type.base_type) {
  case BASE_TYPE_UINT:  return " as Long";
  case BASE_TYPE_USHORT:  return " as Int";
  case BASE_TYPE_UCHAR:  return " as Short";
default: return "";
}
    /*  if (
type.base_type == BASE_TYPE_LONG ||
type.base_type == BASE_TYPE_UINT ||
          (type.base_type == BASE_TYPE_VECTOR &&
           type.element == BASE_TYPE_UINT)) return "as Long";

  return "";*/
}


// Mask to turn serialized value into destination type value.
static std::string DestinationMask(
                                   const Type &type, bool vectorelem) {

  switch (type.base_type) {
    case BASE_TYPE_UCHAR:  return " and 0xFF";
    case BASE_TYPE_USHORT: return " and 0xFFFF";
    case BASE_TYPE_UINT:   return " and 0xFFFFFFFFL";
    case BASE_TYPE_VECTOR:if (vectorelem) return DestinationMask(type.VectorType(), vectorelem);
      // else fall thru:
    default: return "";
  }
}




// Returns the method name for use with add/put calls.
static std::string GenMethod(const FieldDef &field) {
  return IsScalar(field.value.type.base_type)
    ? /*MakeCamel(*/GenTypeBasic(field.value.type)/*)*/
    : (IsStruct(field.value.type) ? "Struct" : "Int");
}


// Save out the generated code for a Go Table type.
static bool SaveType(const Parser &parser, const Definition &def,
                     const std::string &classcode, const std::string &path,
                     bool needs_imports) {
  if (!classcode.length()) return true;

  std::string namespace_name;
  std::string namespace_dir = path;  // Either empty or ends in separator.
  auto &namespaces = parser.namespaces_.back()->components;
  for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
    if (namespace_name.length()) {
      namespace_name += ".";
    }
    namespace_name = *it;
    namespace_dir += *it + kPathSeparator;
  }
  EnsureDirExists(namespace_dir);

  std::string code = "";
  BeginFile(namespace_name, needs_imports, &code);
  code += classcode;
  std::string filename = namespace_dir + def.name + ".kt";
  return SaveFile(filename.c_str(), code, false);
}

static std::string GenTypeBasic(const Type &type) {
  static const char *ctypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, KTYPE, GTYPE, NTYPE, PTYPE) \
      #KTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };
  return ctypename[type.base_type];
}

/*
static std::string Zero(const FieldDef &field) {
switch (field.value.type.base_type) {
case BASE_TYPE_NONE:
case BASE_TYPE_UTYPE:
case BASE_TYPE_CHAR:
case BASE_TYPE_UCHAR:return "0.toByte()";
case BASE_TYPE_BOOL:return "false";
case BASE_TYPE_SHORT:
case BASE_TYPE_USHORT:return "0.toShort()";
case BASE_TYPE_INT:
case BASE_TYPE_UINT:return "0";
case BASE_TYPE_LONG:
case BASE_TYPE_ULONG:return "0L";
case BASE_TYPE_FLOAT:return "0f";
case BASE_TYPE_DOUBLE:return "0.0";
case BASE_TYPE_STRING:
case BASE_TYPE_VECTOR:
case BASE_TYPE_STRUCT:
case BASE_TYPE_UNION:return "null";
    default:return "0";
  }
}*/


// I didn't succeed in making that work with a switch :/ see above, there is someting wrong I think with the types
static std::string Zero(const FieldDef &field) {
std::string typeName = TypeName(field);
if (typeName == "Int") return "0";
if (typeName == "Long") return "0L";
if (typeName == "Float") return "0f";
if (typeName == "Double") return "0.0";
if (typeName == "Short") return "ZERO_SHORT";
if (typeName == "Byte") return "ZERO_BYTE";
// there are chars in kotlin (16 bits wide unsigned) but maybee it's better to serialize them as shorts
return "null";
}

static std::string GenTypePointer(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING:
      return "_string";
    case BASE_TYPE_VECTOR:
      return GenTypeGet(type.VectorType());
    case BASE_TYPE_STRUCT:
      return type.struct_def->name;
    case BASE_TYPE_UNION:
      // fall through
    default:
      return "Table";
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

}  // namespace kotlin

bool GenerateKotlin(const Parser &parser,
                const std::string &path,
                const std::string & /*file_name*/,
                const GeneratorOptions & /*opts*/) {
  for (auto it = parser.enums_.vec.begin();
       it != parser.enums_.vec.end(); ++it) {
    std::string enumcode;
    kotlin::GenEnum(**it, &enumcode);
    if (!kotlin::SaveType(parser, **it, enumcode, path, false))
      return false;
  }

  for (auto it = parser.structs_.vec.begin(); it != parser.structs_.vec.end(); ++it) {
    std::string declcode;
    kotlin::GenStruct(**it, &declcode, parser.root_struct_def_);
    if (!kotlin::SaveType(parser, **it, declcode, path, true)) return false;
  }

  return true;
}

}  // namespace flatbuffers

