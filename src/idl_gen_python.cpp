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

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#include <iterator>

#ifdef _WIN32
#include <direct.h>
#define mkdir(n, m) _mkdir(n)
#else
#include <sys/stat.h>
#endif

namespace flatbuffers {
namespace python {

// Return a IDL type from the table in idl.h
static std::string GenTypeBasic(const Type &type) {
  static const char *pytypename[] = {
#define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE) IDLTYPE,
    FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
#undef FLATBUFFERS_TD
  };
  return pytypename[type.base_type];
}

void GenPyComment(const std::vector<std::string> &dc, 
                  std::string *code_ptr, 
                  std::string prefix = "") {
  std::string &code = *code_ptr;
  if (!dc.empty()) {
      for(const auto &line: dc) {
          code += prefix + "# " + line + "\n";
    }
  }
}

void GenDocString(const std::vector<std::string> &dc, 
                  std::string *code_ptr, 
                  std::string prefix = "") {
  std::string &code = *code_ptr;
  if (!dc.empty()) {
    code += prefix + "\"\"\"\n";
    for(const std::string &line: dc) {
        code += prefix + line + "\n";
    }
    code += prefix + "\"\"\"\n\n";
  }
}

static char GenPyFmtChar(const Type& type) {
  switch(type.base_type) {
    case BASE_TYPE_BOOL: return '?';
    case BASE_TYPE_CHAR: return 'b';
    case BASE_TYPE_UCHAR: return 'B';
    case BASE_TYPE_SHORT: return 'h';
    case BASE_TYPE_USHORT: return 'H';
    case BASE_TYPE_INT: return 'i';
    case BASE_TYPE_UINT: return 'I';
    case BASE_TYPE_FLOAT: return 'f';
    case BASE_TYPE_DOUBLE: return 'd';
    case BASE_TYPE_LONG: return 'q';
    case BASE_TYPE_ULONG: return 'Q';
    default: assert("Unexpected scalar type"); return 0;
  }
}

// Generate an enum declaration and an enum string lookup table.
static void GenEnum(EnumDef &enum_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "class " + enum_def.name + "(_Enum):\n";
  GenDocString(enum_def.doc_comment, code_ptr, "    ");

  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    GenPyComment(ev.doc_comment, code_ptr, "  ");
    code += "    " + ev.name + " = " + NumToString(ev.value) + "\n";
  }

  std::string type;
  if (enum_def.underlying_type.base_type == BASE_TYPE_UTYPE) {
    type = "ubyte";
  } else {
    type = GenTypeBasic(enum_def.underlying_type);
  }

  code += "\n\n"
      "def read_" + enum_def.name + "(view, offset):\n"
      "    return " + enum_def.name + "(flatbuffers.encode.read_" + type +
      "(view, offset))\n"
      "\n\n";
}

static void GenFieldOffset(const std::string& offset, std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "        offset = self.get_offset(" + offset + ")\n"
          "        if offset == 0:\n"
          "            return None\n"
          "        data_offset = self._offset + offset\n";
}

static void GenProperty(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;
  const Type &type = field.value.type;
  const auto offset = NumToString((field.value.offset / 2) - 2);

  code += "    def get_" + field.name + "(self):\n";
  GenDocString(field.doc_comment, code_ptr, "        ");

  if(IsScalar(type.base_type) && type.enum_def == nullptr) {
    code += "        return self.read_" + GenTypeBasic(type) + "_field(" +
            offset + ", " + field.value.constant + ")\n\n";
  } else if(type.base_type != BASE_TYPE_UNION && type.enum_def != nullptr) {
    code += "        return self.read_field(" + offset + ", read_" + 
            type.enum_def->name + ", " + field.value.constant + ")\n\n";
  } else if(type.base_type == BASE_TYPE_STRING) {
    GenFieldOffset(offset, code_ptr);
    code += "        data_offset += flatbuffers.encode.read_uoffset"
            "(self._buf, data_offset)\n"
            "        return flatbuffers.encode.read_string(self._buf, "
            "data_offset)\n\n";
  } else if(type.base_type == BASE_TYPE_VECTOR) {
    GenFieldOffset(offset, code_ptr);

    if(IsScalar(type.element)) {
      code += "        return flatbuffers.encode.read_scalar_vector(\"";
      code += GenPyFmtChar(type.VectorType());
      code += "\", self._buf, data_offset)\n\n";
    } else if (IsStruct(type.VectorType())) {
      code += "        return flatbuffers.StructVector(" +
              type.struct_def->name + ", " + 
              "self._buf, data_offset)\n\n";
    } else if (type.element == BASE_TYPE_STRING) {
      code += "        return flatbuffers.IndirectVector(" 
              "flatbuffers.encode.read_string, self._buf, data_offset)\n\n";
    } else {
      code += "        return flatbuffers.IndirectVector(" + 
              type.struct_def->name + ", self._buf, data_offset)\n\n";
    }
  } else if (type.base_type == BASE_TYPE_UNION) {
    const auto& enum_def = *type.enum_def;

    code += "        tpe = self.get_" + field.name + "_type()\n"
        "        if tpe is None or tpe == " + enum_def.name + ".NONE:\n"
        "            return None\n";

    for(std::size_t i=1; i<enum_def.vals.vec.size(); i++) {
      const auto& val = enum_def.vals.vec[i]->name;
      code += "        if tpe == " + enum_def.name + "." + val + ":\n"
              "            target = " + val + "\n";
    }
    GenFieldOffset(offset, code_ptr);
    code += "        data_offset += flatbuffers.encode.read_uoffset"
            "(self._buf, data_offset)\n"
            "        return target(self._buf, data_offset)\n\n";
  } else {
    GenFieldOffset(offset, code_ptr);
    if (!IsStruct(type)) {
        code += "        data_offset += flatbuffers.encode.read_uoffset"
                "(self._buf, data_offset)\n";
    }
    code += "        return " + type.struct_def->name + "("
            "self._buf, data_offset)\n\n";
  }
}

static void GenTable(StructDef &struct_def, std::string *code_ptr) {
  if (struct_def.generated) return;

  std::string &code = *code_ptr;

  code += "class " + struct_def.name + "(flatbuffers.Table):\n";
  GenDocString(struct_def.doc_comment, code_ptr, "    ");

  for (auto field_ptr: struct_def.fields.vec) {
    auto &field = *field_ptr;

    if(!field.deprecated) {
      GenProperty(field, code_ptr);
    }


  }
  code += "\n";

  code += "class " + struct_def.name + "Builder(object):\n"
          "    def __init__(self, fbb, start):\n"
          "        self._fbb = fbb\n"
          "        self._start = start\n\n";

  for (auto field_ptr: struct_def.fields.vec) {
    auto &field = *field_ptr;
    const Type &type = field.value.type;

    if(field.deprecated || type.base_type == BASE_TYPE_UTYPE) {
      continue;
    }

    code += "    def add_" + field.name + "(self, value):\n";

    const auto offset = NumToString((field.value.offset / 2) - 2);

      if(IsScalar(type.base_type) && type.enum_def == nullptr) {
        code += "        self.add_" + GenTypeBasic(type) + "(" +
                offset + ", value, " + field.value.constant + ")\n\n";
      } else if (type.enum_def != nullptr) {
        code += "        assert type(value) is " + type.enum_def->name + "\n";
                "        self.add_" + GenTypeBasic(type) + "(" +
                offset + ", value.value, " + field.value.constant + ")\n\n";
      } else if (type.base_type == BASE_TYPE_UNION) {
        code += "        tpe = type(value)\n";
        for (auto val: type.enum_def->vals.vec) {
          code += "        if tpe is " + val->name + ":\n";
        }
      } else {
        code += "        assert type(value) is Offset\n"
                "        self.add_offset(" + offset + ", value, " +
                field.value.constant + ")\n\n";
      }
  }
}

static int TotalValues(Type &type) {
  if (IsScalar(type.base_type)) {
    return 1;
  }; 

  int count = 0;
  for (auto field: type.struct_def->fields.vec) {
    auto &field_type = field->value.type;

    if (IsScalar(field_type.base_type)) {
      count++;
    } else {
      count += TotalValues(field_type);
    }
  }

  return count;
}

static const std::string GenStructFormat(StructDef &struct_def) {
  std::string format;

  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    auto &type = field.value.type;

    if(IsScalar(type.base_type)) {
      format += GenPyFmtChar(type);
    } else if(IsStruct(type)) {
      format += GenStructFormat(*(type.struct_def));
    }

    // add padding
    if(field.padding == 1) {
      format += "x";
    } else if(field.padding > 1) {
      format += NumToString(field.padding) + "x";
    };
  }
  return format;
}

// Generate an accessor struct with constructor for a flatbuffers struct.
static void GenStruct(StructDef &struct_def,
                      std::string *code_ptr) {
  if (struct_def.generated) return;

  std::string &code = *code_ptr;
  auto &fields = struct_def.fields.vec;

  code += "class " + struct_def.name + "(flatbuffers.Struct):\n";
  GenDocString(struct_def.doc_comment, code_ptr, "    ");

  code += "    _FORMAT = struct.Struct(\"" + GenStructFormat(struct_def) + 
          "\")\n\n"
          "    @classmethod\n"
          "    def format(cls):\n"
          "        return cls._FORMAT\n\n";

  int i = 0;
  for (auto &field: fields) {
    auto &type = field->value.type;

      code += "    def get_" + field->name + "(self):\n";
      GenDocString(field->doc_comment, code_ptr, "        ");
    if(type.enum_def != nullptr) {
      code += "        return " + type.enum_def->name + "(_getitem(self, " + 
              NumToString(i) + "))\n\n";
      i++;
    } else if (IsScalar(type.base_type)) {
      code += "        return _getitem(self, " + NumToString(i) + ")\n\n";
      i++;
    } else if(IsStruct(type)) {
      int size = TotalValues(type);
      code += "        return _tuple.__new__(" + type.struct_def->name + ", "
              "_getitem(self, slice(" + NumToString(i) + ", " + 
              NumToString(i + size) + ")))\n\n";
      i += size;
    }
  }

  code += "\n";

}

}  // namespace python

// Iterate through all definitions we haven't generate code for (enums, structs,
// and tables) and output them to a single file.
std::string GeneratePython(const Parser &parser,
                           const std::string &,
                           const GeneratorOptions &) {
  using namespace python;

  // Generate code for all the enum declarations.
  std::string enum_code;
  for (auto it = parser.enums_.vec.begin();
       it != parser.enums_.vec.end(); ++it) {
    GenEnum(**it, &enum_code);
  }

  // Generate code for all structs, then all tables.
  std::string decl_code;
  for (auto it = parser.structs_.vec.begin();
       it != parser.structs_.vec.end(); ++it) {
    if ((**it).fixed) GenStruct(**it, &decl_code);
  }

  for (auto it = parser.structs_.vec.begin();
       it != parser.structs_.vec.end(); ++it) {
    if (!(**it).fixed) GenTable(**it, &decl_code);
  }


  // Only output file-level code if there were any declarations.
  if (enum_code.length() || decl_code.length()) {
    std::string code;
    code = "# automatically generated by the FlatBuffers compiler,"
        " do not modify\n\n";

    code += "import flatbuffers\n"
        "import enum\n"
        "import operator\n"
        "import struct\n\n";

    code += "# localize performance sensitive globals\n"
        "_getitem = operator.getitem\n"
        "_tuple = tuple\n"
        "_Enum = enum.Enum\n"
        "\n\n";

    // Output the main declaration code from above.
    code += enum_code;
    code += decl_code;

    if (parser.root_struct_def) {
      code += "def get_root_as_" + parser.root_struct_def->name +
              "(source):\n"
              "    buf = source if type(source) is memoryview "
              "else memoryview(source)\n"
              "    offset = flatbuffers.encode.read_uoffset(buf, 0)\n"
              "    return " + parser.root_struct_def->name + 
              "(buf, offset)\n";
    }

    return code;
  }

  return std::string();
}

static bool SavePackage(const Parser &parser, const std::string &path, 
                        const std::string &code) {

  if (!code.length()) return true;

  std::string namespace_dir = path;
  std::string init_name;
  auto &namespaces = parser.namespaces_.back()->components;
  for (const auto &ns: namespaces) {
    if(namespace_dir != path) {
      namespace_dir += kPathSeparator;
    }
    namespace_dir += ns;
    mkdir(namespace_dir.c_str(), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);

    init_name = namespace_dir + kPathSeparator + "__init__.py";

    // create empty __init__.py if it doesn't exist
    if(!FileExists(init_name.c_str()) && 
       !SaveFile(init_name.c_str(), "", 0, false)) return false;
  }

  return SaveFile(init_name.c_str(), code, false);
}

bool GeneratePython(const Parser &parser,
                    const std::string &path,
                    const std::string &file_name,
                    const GeneratorOptions &opts) {
  auto code = GeneratePython(parser, file_name, opts);
  return !code.length() ||
      SavePackage(parser, path, code);
}

}  // namespace flatbuffers

