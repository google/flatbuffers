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

#include <list>
#include <algorithm>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include "flatbuffers/code_generators.h"

namespace flatbuffers {

namespace dlang {

class DlangGenerator : public BaseGenerator {
public:
 DlangGenerator(const Parser &parser, const std::string &path,
              const std::string &file_name)
     : BaseGenerator(parser, path, file_name, "", "."),
       cur_name_space_(nullptr){}

   bool generate(){
       std::string one_file_code;
       cur_name_space_ = parser_.namespaces_.back();
       for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
               ++it) {
            std::string enumcode;
            auto &enum_def = **it;
            if (!parser_.opts.one_file)
                   cur_name_space_ = enum_def.defined_namespace;
            GenEnum(enum_def, &enumcode);
            if (parser_.opts.one_file) {
                one_file_code += enumcode;
              } else {
                if (!SaveType(enum_def.name, *enum_def.defined_namespace,
                              enumcode, false)) return false;
              }
       }

       for (auto it = parser_.structs_.vec.begin();
               it != parser_.structs_.vec.end(); ++it) {
            std::string declcode;
            auto &struct_def = **it;
            if (!parser_.opts.one_file)
              cur_name_space_ = struct_def.defined_namespace;
            GenStruct(struct_def, &declcode);
            if (parser_.opts.one_file) {
              one_file_code += declcode;
            } else {
              if (!SaveType(struct_def.name, *struct_def.defined_namespace,
                            declcode, true)) return false;
            }
          }
       if (parser_.opts.one_file) {
             return SaveType(file_name_, *parser_.namespaces_.back(),
                             one_file_code, true);
        } else  if (parser_.namespaces_.back()->components.size()) {
            return SavePackage();
       }

       return true;
   }


   void GenEnum( EnumDef &enum_def,
                        std::string *code_ptr) {
       std::string &code = *code_ptr;
       if (enum_def.generated) return;
       ////printf("!generated %s\n", enum_def.name.c_str());

       // Generate enum definitions of the form:
       // public static (final) int name = value;
       // In Java, we use ints rather than the Enum feature, because we want them
       // to map directly to how they're used in C/C++ and file formats.
       // That, and Java Enums are expensive, and not universally liked.
       GenComment(enum_def.doc_comment, code_ptr);

       code += std::string("")  + "enum " + enum_def.name;

       code += " : " + GenTypeBasic( enum_def.underlying_type);

       code += "\n{\n";

       for (auto it = enum_def.vals.vec.begin();
            it != enum_def.vals.vec.end();
            ++it) {
           auto &ev = **it;
           GenComment(ev.doc_comment, code_ptr, "  ");

           code += "  ";
           code += " " + ev.name + " = ";
           code += NumToString(ev.value);
           code += ",\n";
       }

       // Close the class
       code += std::string("") + "}" + "\n\n";
   }

   void GenComment(const std::vector<std::string> &dc, std::string *code_ptr, const char *prefix = "") {
       if (dc.begin() == dc.end()) {
           // Don't output empty comment blocks with 0 lines of comment content.
           return;
       }

       std::string &code = *code_ptr;

       std::string line_prefix = std::string(prefix) + "///";
       for (auto it = dc.begin();
            it != dc.end();
            ++it) {
           code += line_prefix + *it + "\n";
       }
   }


   bool SaveType(const std::string &defname, const Namespace &ns
                 , const std::string &classcode, bool needs_includes) {

       if (!classcode.length()) return true;

       std::string code;
       code = code + "// " + FlatBuffersGeneratedWarning();
       code += "module ";
       std::string namespace_name = FullNamespace(".", ns);
        if (!namespace_name.empty()) {
            code += namespace_name + ".";
            code += defname + ";\n\n";
        }
        if (needs_includes)
            code += "import std.typecons;\nimport flatbuffers;\n\n";
       code += classcode;
       auto filename = NamespaceDir(ns) + defname + ".d";
       return SaveFile(filename.c_str(), code, false);
   }


   // Returns the function name that is able to read a value of the given type.
   std::string GenGetter( const Type &type) {
       switch (type.base_type) {
       case BASE_TYPE_STRING: return "__string";
       case BASE_TYPE_STRUCT: return "__struct";
       case BASE_TYPE_UNION:  return "__union!";
       case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
       default: {
           std::string getter = std::string("_buffer.") + "get!";
           if (type.base_type == BASE_TYPE_BOOL) {
               getter += "ubyte";
           } else {
               getter += GenTypeGet(type);
           }
           return getter;
       }
       }
   }

   // Returns the method name for use with add/put calls.
   std::string GenMethod( const Type &type) {
       return IsScalar(type.base_type)
               ? (std::string("!") + GenTypeBasic( type))
               : (IsStruct(type) ? "Struct" : "Offset");
   }

   // Recursively generate arguments for a constructor, to deal with nested
   // structs.
   void GenStructArgs(const StructDef &struct_def,
                             std::string *code_ptr, const char *nameprefix) {
       std::string &code = *code_ptr;
       for (auto it = struct_def.fields.vec.begin();
            it != struct_def.fields.vec.end();
            ++it) {
           auto &field = **it;
           if (IsStruct(field.value.type)) {
               // Generate arguments for a struct inside a struct. To ensure names
               // don't clash, and to make it obvious these arguments are constructing
               // a nested struct, prefix the name with the struct name.
               GenStructArgs( *field.value.type.struct_def, code_ptr,
                              (field.value.type.struct_def->name + "_").c_str());
           } else {
               code += ", ";
               code += GenTypeBasic(field.value.type);

               code += " ";
               code += nameprefix;
               code += MakeCamel(field.name, false);
           }
       }
   }

   // Recusively generate struct construction statements of the form:
   // builder.put!type(name);
   // and insert manual padding.
   void GenStructBody(const StructDef &struct_def,
                             std::string *code_ptr, const char *nameprefix) {
       std::string &code = *code_ptr;
       code += std::string("\t\tbuilder.") +  "prep(";
       code += NumToString(struct_def.minalign) + ", ";
       code += NumToString(struct_def.bytesize) + ");\n";
       for (auto it = struct_def.fields.vec.rbegin();
            it != struct_def.fields.vec.rend(); ++it) {
           auto &field = **it;
           if (field.padding) {
               code += std::string("\t\tbuilder.") + "pad(";
               code += NumToString(field.padding) + ");\n";
           }
           if (IsStruct(field.value.type)) {
               GenStructBody( *field.value.type.struct_def, code_ptr,
                              (field.value.type.struct_def->name + "_").c_str());
           } else {
               code += std::string("\t\tbuilder.") +  "put!";
               code += GenTypeBasic(field.value.type) + "(";
               code +=  nameprefix + MakeCamel(field.name, false);
               code += ");\n";
           }
       }
   }

   void GenStruct(StructDef &struct_def, std::string *code_ptr) {
       if (struct_def.generated) return;
       std::string &code = *code_ptr;

       if(!parser_.opts.one_file){
           // Create imports for modules that this struct depends on.
               std::string namespace_general;
               auto &namespaces = parser_.namespaces_.back()->components;
               for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
                   namespace_general += *it;
                   if (namespace_general.length()) {
                       namespace_general += ".";
                   }
               }
               std::list<std::string> imports;
               for (auto it = struct_def.fields.vec.begin();
                    it != struct_def.fields.vec.end();
                    ++it) {
                   auto &field = **it;
                   if (field.deprecated) continue;

                   if (field.value.type.struct_def)
                       imports.push_back(field.value.type.struct_def->name);
                   if (field.value.type.enum_def)
                       imports.push_back(field.value.type.enum_def->name);
               }
               imports.unique();
               for (auto it = imports.begin(); it != imports.end(); ++it) {
                   auto import = "import " + namespace_general + (*it) + ";\n";
                   code += import;
               }
       }

       GenComment(struct_def.doc_comment, code_ptr);

       code += "struct " + struct_def.name;

       code += " {\n";
       //Dlang mixin Struct
       code += std::string("\tmixin ") + (struct_def.fixed? "Struct" : "Table") + "!" + struct_def.name + ";\n\n";

       if (!struct_def.fixed) {
           // Generate a special accessor for the table that when used as the root
           // of a FlatBuffer
           std::string method_name =  "getRootAs" + struct_def.name;
           // create convenience method that doesn't require an existing object
           code += std::string("  ")  + "static " + struct_def.name + " " + method_name + "(ByteBuffer _bb) ";
           code += "{  return " + struct_def.name + ".init_(_bb.get!int(_bb.position()) + _bb.position(), _bb); }\n";

           if (parser_.root_struct_def_ == &struct_def) {
               if (parser_.file_identifier_.length()) {
                   // Check if a buffer has the identifier.
                   code += std::string("  ")  + "static ";
                   code += "bool" + struct_def.name;
                   code += "BufferHasIdentifier(ByteBuffer _bb) { return ";
                   code += "__has_identifier(_bb, \"" + parser_.file_identifier_;
                   code += "\"); }\n";
               }
           }
       }

       for (auto it = struct_def.fields.vec.begin();
            it != struct_def.fields.vec.end();
            ++it) {
           auto &field = **it;
           if (field.deprecated) continue;
           GenComment(field.doc_comment, code_ptr, "  ");
           std::string type_name = GenTypeGet(field.value.type);
           std::string type_name_dest = GenTypeGet(field.value.type);
           std::string method_start = std::string("") + type_name_dest + " ";


           if (field.value.type.base_type == BASE_TYPE_VECTOR) {
                 method_start = std::string("\t") + "auto " + MakeCamel(field.name, false);
                 code += method_start + "() { ";
                 code += "return Iterator!(";
                 code += struct_def.name + ", " + type_name +", \"" + field.name;
                 code += "\")(this); }\n";
           }

           std::string nullValue = "null";
           bool typeNeedsExtraHandling = false;

           typeNeedsExtraHandling = !IsScalar(field.value.type.base_type) && !IsScalar(field.value.type.element);
           method_start = std::string("") + (typeNeedsExtraHandling? "Nullable!" : "") +
                   (field.value.type.base_type==BASE_TYPE_UNION? "T" : type_name) + " " +
                   MakeCamel(field.name, false);
           nullValue = "Nullable!" + (field.value.type.base_type==BASE_TYPE_UNION? "T" : type_name) + ".init";

           // Most field accessors need to retrieve and test the field offset first,
           // this is the prefix code for that:
           auto offset_prefix = " { uint o = __offset(" +
                   NumToString(field.value.offset) +
                   "); return o != 0 ? " +
                   (typeNeedsExtraHandling
                    ? "Nullable!" + (field.value.type.base_type==BASE_TYPE_UNION? "T" : type_name) + "("
                    : "");

           std::string getter =  GenGetter(field.value.type);

           // Most field accessors need to retrieve and test the field offset first,
           // this is the prefix code for that:
           if (IsScalar(field.value.type.base_type)) {
               method_start = std::string("\t@property ") + method_start;
               code += method_start;
               code += "()";
               if (struct_def.fixed) {
                   code += " { return " + getter;
                   code += "(_pos + " + NumToString(field.value.offset) + ")";
               } else {
                   code += offset_prefix + getter;
                   code += "(o + _pos)";
                   if(field.value.type.base_type == BASE_TYPE_BOOL)
                       code += " != 0 ";
                   code +=  " : ";
                   code += GenDefaultValue(field.value);
               }
           } else {
               switch (field.value.type.base_type) {
               case BASE_TYPE_STRUCT:
                    method_start = std::string("\t@property ") + method_start;
                    code += method_start;
                   code += "(";
                   if (struct_def.fixed) {
                       code += ") { return Nullable!" + type_name + "(" + type_name + ".init_(_pos + ";

                       code += NumToString(field.value.offset) + ", _buffer)";
                       code += ")";
                   } else {
                       code += ")";
                       code += offset_prefix;
                       code += type_name;
                       code += ".init_(";
                       code += field.value.type.struct_def->fixed
                               ? "o + _pos"
                               : "__indirect(o + _pos)";
                       code += ", _buffer)) : " + nullValue;
                   }
                   break;
               case BASE_TYPE_STRING:
                   method_start = std::string("\t@property ") + method_start;
                   code += method_start;
                   code += "()";
                   code += offset_prefix + getter + "(o + _pos)";
                   code += ") : Nullable!" + type_name + ".init";
                   break;
               case BASE_TYPE_VECTOR: {
                   method_start = std::string("\t") + method_start;
                   code += method_start;
                   auto vectortype = field.value.type.VectorType();
                   code += "(uint j)";
                   code += offset_prefix ;
                   if(IsScalar(field.value.type.element)){ //基本类型
                       code += getter;
                       code += "(__dvector(o) + j * " + NumToString(InlineSize(vectortype));
                       code +=  ") ";
                       if(field.value.type.element == BASE_TYPE_BOOL)
                           code += " != 0 ";
                       code += " : ";
                       code += GenDefaultValue(field.value);
                   } else if(field.value.type.element == BASE_TYPE_STRING ){ //string 类型处理
                       code += getter;
                       code += "(__dvector(o) + j * " + NumToString(InlineSize(vectortype));
                       code += ")) : Nullable!" + type_name + ".init";
                   } else { //结构体和table处理，现在不支持union 和 vector的vector
                       code += type_name ;
                       code +=".init_(";
                       auto index = "__dvector(o) + j * " + NumToString(InlineSize(vectortype));
                       if (vectortype.base_type == BASE_TYPE_STRUCT) {
                           code += vectortype.struct_def->fixed ? index : std::string("__indirect(") + index + ")";
                           code += ", _buffer";
                       } else {
                           code += index;
                       }
                       code += std::string(")") + (typeNeedsExtraHandling? ")" : "")   + " : ";
                       code += IsScalar(field.value.type.element) ?  "0" : nullValue;
                   }
                   break;
               }
               case BASE_TYPE_UNION:
                   method_start = std::string("\t") + method_start;
                   code += method_start;
                   code += "(T)()" + offset_prefix + getter;
                   code += "(T)(o)";
                   code += ")";
                   code += " : " + nullValue;
                   break;
               default:
                   assert(0);
               }
           }
           code += "; ";
           code += "}\n";
           if (field.value.type.base_type == BASE_TYPE_VECTOR) {
               code += std::string("\t@property ") + "uint " + MakeCamel(field.name, false);
               code += "Length";
               code += "()";
               auto offset_prefix1 = " { uint o = __offset(" +
                       NumToString(field.value.offset) +
                       "); return o != 0 ? ";
               code += offset_prefix1;
               code += "__vector_len(o)";
               code += " : 0; ";
               code += "}\n";
           }
       }
       code += "\n";

       if (struct_def.fixed) {
           // create a struct constructor function
           code += std::string("\t")  + "static uint " +  "create";
           code += struct_def.name + "(FlatBufferBuilder builder";
           GenStructArgs( struct_def, code_ptr, "");
           code += ") {\n";
           GenStructBody( struct_def, code_ptr, "");
           code += "\t\treturn builder.";
           code += "offset()";
           code += ";\n\t}\n";
       } else {
           // Generate a method that creates a table in one go. This is only possible
           // when the table has no struct fields, since those have to be created inline
           bool has_no_struct_fields = true;
           int num_fields = 0;
           for (auto it = struct_def.fields.vec.begin();
                it != struct_def.fields.vec.end(); ++it) {
               auto &field = **it;
               if (field.deprecated) continue;
               if (IsStruct(field.value.type)) {
                   has_no_struct_fields = false;
               } else {
                   num_fields++;
               }
           }
           if (has_no_struct_fields && num_fields) {
               // Generate a table constructor of the form:
               // public static void createName(FlatBufferBuilder builder, args...)
               code += std::string("\t")  + "static uint " +  "create";
               code += struct_def.name;
               code += "(FlatBufferBuilder builder";
               for (auto it = struct_def.fields.vec.begin();
                    it != struct_def.fields.vec.end(); ++it) {
                   auto &field = **it;
                   if (field.deprecated) continue;
                   code += ",";
                   code += GenTypeBasic(field.value.type);
                   code += " ";
                   code += field.name;
               }
               code += ") {\n\t\tbuilder.";
               code += "startObject(";
               code += NumToString(struct_def.fields.vec.size()) + ");\n";
               for (size_t size = struct_def.sortbysize ? sizeof(largest_scalar_t) : 1;
                    size;
                    size /= 2) {
                   for (auto it = struct_def.fields.vec.rbegin();
                        it != struct_def.fields.vec.rend(); ++it) {
                       auto &field = **it;
                       if (!field.deprecated &&
                               (!struct_def.sortbysize ||
                                size == SizeOf(field.value.type.base_type))) {
                           code += "\t\t" + struct_def.name + ".";
                           code += "add";
                           code += MakeCamel(field.name) + "(builder, " + field.name + ");\n";
                       }
                   }
               }
               code += "\t\treturn " + struct_def.name + ".";
               code += "end" + struct_def.name;
               code += "(builder);\n\t}\n\n";
           }
           // Generate a set of static methods that allow table construction,
           // of the form:
           // public static void addName(FlatBufferBuilder builder, short name)
           // { builder.addShort(id, name, default); }
           // Unlike the Create function, these always work.
           code += std::string("\t")  + "static void " + "start";
           code += struct_def.name;
           code += "(FlatBufferBuilder builder) { builder.";
           code += "startObject(";
           code += NumToString(struct_def.fields.vec.size()) + "); }\n";
           for (auto it = struct_def.fields.vec.begin();
                it != struct_def.fields.vec.end(); ++it) {
               auto &field = **it;
               if (field.deprecated) continue;
               code += std::string("\t")  + "static void " +  "add";
               code += MakeCamel(field.name);
               code += "(FlatBufferBuilder builder, ";
               code += GenTypeBasic(field.value.type);
               auto argname = MakeCamel(field.name, false);
               if (!IsScalar(field.value.type.base_type)) argname += "Offset";
               code += std::string(" ") + argname + ") { builder." + "add";
               code += GenMethod( field.value.type) + "(";
               code += NumToString(it - struct_def.fields.vec.begin()) + ", ";
               code += argname;
               code += ", " + GenDefaultValue(field.value);
               code += "); }\n";
               if (field.value.type.base_type == BASE_TYPE_VECTOR) {
                   auto vector_type = field.value.type.VectorType();
                   auto alignment = InlineAlignment(vector_type);
                   auto elem_size = InlineSize(vector_type);
                   if (!IsStruct(vector_type)) {
                       // Generate a method to create a vector from a Java array.
                       code += std::string("\t")  + "static uint " + "create";
                       code += MakeCamel(field.name);
                       code += "Vector(FlatBufferBuilder builder, ";
                       code += GenTypeBasic(vector_type) + "[] data) ";
                       code += std::string("{ builder.") + "startVector(";
                       code += NumToString(elem_size) + ", ";
                       code += std::string("") + "cast(uint)data.length, ";
                       code += NumToString(alignment);
                       code += "); for (size_t i = ";
                       code += std::string("") + "data.length; i > 0; i--) builder.";
                       code += "add";
                       code += GenMethod( vector_type);
                       code += "(data[i - 1]); return builder.";
                       code += "endVector(); }\n";
                   }
                   // Generate a method to start a vector, data to be added manually after.
                   code += std::string("\t")  + "static void " + "start";
                   code += MakeCamel(field.name);
                   code += "Vector(FlatBufferBuilder builder, uint numElems) ";
                   code += std::string("{ builder.") + "startVector(";
                   code += NumToString(elem_size);
                   code += ", numElems, " + NumToString(alignment);
                   code += "); }\n";
               }
           }
           code += std::string("\t")  + "static uint ";
           code += "end" + struct_def.name;
           code += "(FlatBufferBuilder builder) {\n\t\tuint o = builder.";
           code += "endObject();\n";
           for (auto it = struct_def.fields.vec.begin();
                it != struct_def.fields.vec.end();
                ++it) {
               auto &field = **it;
               if (!field.deprecated && field.required) {
                   code += std::string("\t\tbuilder.") + "required(o, ";
                   code += NumToString(field.value.offset);
                   code += ");  // " + field.name + "\n";
               }
           }
           code += "\t\treturn o;\n\t}\n";
           if (parser_.root_struct_def_ == &struct_def) {
               code += std::string("\t")  + "static void ";
               code += "finish" + struct_def.name;
               code += "Buffer(FlatBufferBuilder builder, uint offset) { ";
               code += std::string("builder.") + "finish(offset";
               if (parser_.file_identifier_.length())
                   code += ", \"" + parser_.file_identifier_ + "\"";
               code += ");\t}\n";
           }
       }
       code += "}\n\n";
   }

   std::string GenDefaultValue(const Value &value) {
       switch (value.type.base_type) {
       case BASE_TYPE_BOOL:
           return (value.constant == "0" ? "false" : "true");
       case BASE_TYPE_VECTOR:
           return  value.type.element == BASE_TYPE_BOOL
                   ? (value.constant == "0" ? "false" : "true")
                   : value.constant;
       default:
           return value.constant;
       }
   }

   bool SavePackage() {
       std::string unit_name = "package";

       std::string namespace_general;
       std::string namespace_dir = path_;  // Either empty or ends in separator.
       std::string tmp_general = "";
       auto &namespaces = parser_.namespaces_.back()->components;
       for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
           if (it != namespaces.end()-1) {
               namespace_general += (*it);
           }
           namespace_dir += (*it) + kPathSeparator;
           if (namespace_general.length() && it != namespaces.end()-1) {
               namespace_general += ".";
           }
       }
       EnsureDirExists(namespace_dir);
       namespace_general += (*(namespaces.end()-1));
       tmp_general = namespace_general;
       std::string code = "// automatically generated, do not modify\n\n";
       code += "module " + namespace_general + ";";
       code += "\n\n";
       code += std::string("public ") + "import std.typecons;\npublic import flatbuffers;\n\n",
       tmp_general += ".";
       //printf("tmp_general : %s\n", tmp_general.c_str());
       std::list<std::string> modules;
       for (auto it = parser_.enums_.vec.begin();
            it != parser_.enums_.vec.end(); ++it) {
            auto str  = (**it).defined_namespace->GetFullyQualifiedName("");
            //printf("full name : %s\n",str.c_str());
            if(str == tmp_general)
                modules.push_back((**it).name);
       }
       for (auto it = parser_.structs_.vec.begin();
            it != parser_.structs_.vec.end(); ++it) {
           auto str  = (**it).defined_namespace->GetFullyQualifiedName("");
           //printf("full name : %s\n",str.c_str());
           if(str == tmp_general)
               modules.push_back((**it).name);
       }
       modules.unique();
       for (auto it = modules.begin();
            it != modules.end(); ++it) {
           auto import = "public import " + namespace_general + "." +  (*it) + ";\n";
           code += import;
       }
       auto filename = namespace_dir + unit_name + ".d";

       //printf("SaveFile: %s\n", filename.c_str());

       return SaveFile(filename.c_str(), code, false);
   }

private:
  // This tracks the current namespace so we can insert namespace declarations.
  const Namespace *cur_name_space_;

  const Namespace *CurrentNameSpace() { return cur_name_space_; }

  // Return a Dlang type from the table in idl.h
   std::string GenTypeBasic(const Type &type) {
     static const char *ctypename[] = {
     #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, DTYPE) \
             #DTYPE,
         FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
     #undef FLATBUFFERS_TD
     };
     return ctypename[type.base_type];
   }

   std::string GenTypePointer(const Type &type) {
     switch (type.base_type) {
       case BASE_TYPE_STRING:
         return "string";
       case BASE_TYPE_VECTOR:
         return GenTypeGet(type.VectorType());
       case BASE_TYPE_STRUCT:
         return type.struct_def->name;
       case BASE_TYPE_UNION:
         return "T";
         // fall through
       default:
         return "T";
     }
   }

   std::string GenTypeGet(const Type &type) {
       return IsScalar(type.base_type)
               ? GenTypeBasic(type)
               : GenTypePointer(type);
   }

};

}

bool GenerateDlang(const Parser &parser,
                   const std::string &path,
                   const std::string & file_name) {
    dlang::DlangGenerator generator(parser,path,file_name);
    return generator.generate();
}


}  // namespace flatbuffers
