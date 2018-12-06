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

#include <sstream>
#include <string>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include <unordered_set>

#ifdef _WIN32
#  include <direct.h>
#  define PATH_SEPARATOR "\\"
#  define mkdir(n, m) _mkdir(n)
#else
#  include <sys/stat.h>
#  define PATH_SEPARATOR "/"
#endif

namespace flatbuffers {
namespace go {

struct GoGenerator : public BaseGenerator {
	// see https://golang.org/ref/spec#Keywords
	std::unordered_set<std::string> golang_keywords_ = {
		"break",  "default", "func",        "interface", "select", "case", "defer",
		"go",     "map",     "struct",      "chan",      "else",   "goto", "package",
		"switch", "const",   "fallthrough", "if",        "range",  "type", "continue",
		"for",    "import",  "return",      "var"
	};

	std::string NativeName(StructDef const &sd) {
		return parser_.opts.object_prefix + sd.name + parser_.opts.object_suffix;
	}

	std::string GoIdentity(
		const std::string &name, bool exportable = false
	) {
		if (golang_keywords_.count(name) != 0)
			return MakeCamel(name + "_", exportable);
		else
			return MakeCamel(name, exportable);
	}

	std::string ToBasicType(const Type &type) {
		const char *ctypename[] = {
// clang-format off
#define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, RTYPE) #GTYPE,

FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
#undef FLATBUFFERS_TD
// clang-format on
		};
		return ctypename[type.base_type];
	}

	// Most field accessors need to retrieve and test the field offset first,
	// this is the prefix code for that.
	void GenOffsetPrefixTest(FieldDef const *fld) {
		code_ += "\to := flatbuffers.UOffsetT(rcv._tab.Offset("
			+ NumToString(fld->value.offset) + "))";
		code_ += "\tif o != 0 {";
	}

	void GenComment(
		const std::vector<std::string> &dc, 
		const CommentConfig *config,
		const char *prefix = ""
	) {
		std::string s;
		flatbuffers::GenComment(dc, &s, config, prefix);
		if (!s.empty())
			code_ += s;
	}

	void GenNativeMarshal(StructDef const &def) {
		code_ += "{{NATIVE_STRUCT_RECEIVER}} Marshal(builder *flatbuffers.Builder) flatbuffers.UOffsetT {";
		int comp_count_in = 0;
		int comp_count_out = 0;

		for (auto fld: def.fields.vec) {
			if (fld->deprecated)
				continue;

			if (!IsScalar(fld->value.type.base_type))
				comp_count_in++;
		}

		if (comp_count_in > 0) {
			code_ += "\tvar objs [" + NumToString(comp_count_in) + "]flatbuffers.UOffsetT";
		}

		code_ += "\tb := Build{{STRUCT_NAME}}(builder)";
		for (auto fld: def.fields.vec) {
			if (fld->deprecated)
				continue;

			code_ += "\tb.Add" + MakeCamel(fld->name) + "(\\";
			if (!IsScalar(fld->value.type.base_type)) {
				code_ += "objs[" + NumToString(comp_count_out++) + "])";
			} else {
				code_ += "rcv." + GoIdentity(fld->name) + ")";
			}
		}
		code_ += "\treturn b.End()";
		code_ += "}\n";
	}

	void GenNativeUnmarshal(StructDef const &def) {
		code_ += "{{STRUCT_RECEIVER}} Unmarshal(obj *{{NATIVE_STRUCT_NAME}}) *{{NATIVE_STRUCT_NAME}} {";
		(void)def;
		code_ += "}\n";
	}

	// Get the length of a vector.
	void GenVectorLenGetter(FieldDef const *fld) {
		code_ += "{{STRUCT_RECEIVER}} " + MakeCamel(fld->name)
			+ "Length() int {";
		GenOffsetPrefixTest(fld);
		code_ += "\t\treturn rcv._tab.VectorLen(o)";
		code_ += "\t}";
		code_ += "\treturn 0";
		code_ += "}\n";
	}

	// Get a [ubyte] vector as a byte slice.
	void GenUByteSliceGetter(FieldDef const *fld) {
		code_ += "{{STRUCT_RECEIVER}} " + MakeCamel(fld->name)
			+ "Bytes() []byte {";
		GenOffsetPrefixTest(fld);
		code_ += "\t\treturn rcv._tab.ByteVector(o + rcv._tab.Pos)";
		code_ += "\t}";
		code_ += "\treturn nil";
		code_ += "}\n";
	}

	void GenScalarFieldGetter(FieldDef const *fld, bool is_fixed) {
		std::string getter = GetGetterForType(fld->value.type);
		std::string return_type;
		std::string return_value;

		if (fld->value.type.enum_def) {
			return_type = GetEnumType(fld->value.type.enum_def);
			return_value = return_type + "(rv)";
		} else {
			return_type = GetTypeName(fld->value.type);
			return_value = "rv";
		}

		code_ += "{{STRUCT_RECEIVER}} " + MakeCamel(fld->name)
			+ "() " + return_type + " {";

		if (is_fixed) {
			code_ += "\trv := " + getter
			+ "(rcv._tab.Pos + flatbuffers.UOffsetT("
			+ NumToString(fld->value.offset) + "))";
			code_ += "\treturn " + return_value;
		} else {
			GenOffsetPrefixTest(fld);
			code_ += "\t\trv := " + getter + "(o + rcv._tab.Pos)";
			code_ += "\t\treturn " + return_value;
			code_ += "\t}";
			code_ += "\treturn " + GenConstant(*fld);
		}

		code_ += "}\n";
	}

	// Get a struct by initializing an existing struct.
	// Specific to Struct.
	void GenStructFieldOfStructGetter(FieldDef const *fld) {
		code_ += "{{STRUCT_RECEIVER}} " + MakeCamel(fld->name)
			+ "(obj *" + TypeName(*fld)
			+ ") *" + TypeName(*fld) + " {";
		code_ += "\tif obj == nil {";
		code_ += "\t\tobj = new(" + TypeName(*fld) + ")";
		code_ += "\t}";
		code_ += "\tobj.Init(rcv._tab.Bytes, rcv._tab.Pos + "
			+ NumToString(fld->value.offset) + ")";
		code_ += "\treturn obj";
		code_ += "}\n";
	}

	// Get a struct by initializing an existing struct.
	// Specific to Table.
	void GenStructFieldOfTableGetter(FieldDef const *fld) {
		code_ += "{{STRUCT_RECEIVER}} " + MakeCamel(fld->name)
			+ "(obj *" + TypeName(*fld)
			+ ") *" + TypeName(*fld) + " {";

		GenOffsetPrefixTest(fld);

		if (fld->value.type.struct_def->fixed) {
			code_ += "\t\tx := o + rcv._tab.Pos";
		} else {
			code_ += "\t\tx := rcv._tab.Indirect(o + rcv._tab.Pos)";
		}
		code_ += "\t\tif obj == nil {";
		code_ += "\t\t\tobj = new(" + TypeName(*fld) + ")";
		code_ += "\t\t}";
		code_ += "\t\tobj.Init(rcv._tab.Bytes, x)";
		code_ += "\t\treturn obj";
		code_ += "\t}";
		code_ += "\treturn nil";
		code_ += "}\n";
	}

	// Get the value of a string.
	void GenStringFieldGetter(FieldDef const *fld) {
		code_ += "{{STRUCT_RECEIVER}} " + MakeCamel(fld->name)
			+ "() " + TypeName(*fld) + " {";

		GenOffsetPrefixTest(fld);
		code_ += "\t\treturn " + GetGetterForType(fld->value.type)
			+ "(o + rcv._tab.Pos)";
		code_ += "\t}";
		code_ += "\treturn \"\"";
		code_ += "}\n";
	}

	// Get the value of a union from an object.
	void GenUnionFieldGetter(FieldDef const *fld) {
		code_ += "{{STRUCT_RECEIVER}} " + MakeCamel(fld->name)
			+ "(obj " + TypeName(*fld) + ") bool {";

		GenOffsetPrefixTest(fld);
		code_ += "\t\t" + GetGetterForType(fld->value.type)
			+ "(obj, o)";
		code_ += "\t\treturn true";
		code_ += "\t}";
		code_ += "\treturn false";
		code_ += "}\n";
	}

	// Get the value of a vector's struct member.
	void GenMemberOfVectorOfStructGetter(FieldDef const *fld) {
		auto vectortype(fld->value.type.VectorType());

		code_ += "{{STRUCT_RECEIVER}} " + MakeCamel(fld->name)
			+ "(obj *" + TypeName(*fld) + ", j int) bool {";

		GenOffsetPrefixTest(fld);

		code_ += "\t\tx := rcv._tab.Vector(o)";
		code_ += "\t\tx += flatbuffers.UOffsetT(j) * "
			+ NumToString(InlineSize(vectortype));
		if (!(vectortype.struct_def->fixed)) {
			code_ += "\t\tx = rcv._tab.Indirect(x)";
		}
		code_ += "\t\tobj.Init(rcv._tab.Bytes, x)";
		code_ += "\t\treturn true";
		code_ += "\t}";
		code_ += "\treturn false";
		code_ += "}\n";
	}

	// Get the value of a vector's non-struct member. Uses a named return
	// argument to conveniently set the zero value for the result.
	void GenMemberOfVectorOfNonStructGetter(FieldDef const *fld) {
		auto vectortype(fld->value.type.VectorType());

		code_ += "{{STRUCT_RECEIVER}} " + MakeCamel(fld->name)
			+ "(j int) " + TypeName(*fld) + " {";
		GenOffsetPrefixTest(fld);

		code_ += "\t\ta := rcv._tab.Vector(o)";
		code_ += "\t\treturn " + GetGetterForType(fld->value.type) + "("
			+ "a + flatbuffers.UOffsetT(j*"
			+ NumToString(InlineSize(vectortype)) + "))";
		code_ += "\t}";
		if (vectortype.base_type == BASE_TYPE_STRING) {
			code_ += "\treturn \"\"";
		} else if (vectortype.base_type == BASE_TYPE_BOOL) {
			code_ += "\treturn false";
		} else {
			code_ += "\treturn 0";
		}
		code_ += "}\n";
	}

	// Set the value of a table's field.
	void BuildFieldOfTable(
		StructDef const &def, FieldDef const *fld, const size_t offset
	) {
		std::string basic_type(ToBasicType(fld->value.type));
		std::string actual_type;

		if (fld->value.type.enum_def) {
			actual_type = GetEnumType(fld->value.type.enum_def);
		} else {
			actual_type = GetTypeName(fld->value.type);
		}

		std::string line = "{{STRUCT_BUILDER_RECEIVER}} Add";
		line += MakeCamel(fld->name) + "(" + GoIdentity(fld->name) + " ";

		if (!IsScalar(fld->value.type.base_type) && (!def.fixed)) {
			line += "flatbuffers.UOffsetT";
		} else if (fld->value.type.enum_def) {
			line += actual_type;
		} else {
			line += basic_type;
		}

		line += ") {";
		code_ += line;

		line = "\tb.Builder.Prepend";
		line += GetMethodName(fld) + "Slot(";
		line += NumToString(offset) + ", ";
		if (!IsScalar(fld->value.type.base_type) && (!def.fixed)) {
			line += GoIdentity(fld->name);
		} else if (fld->value.type.enum_def) {
			line += basic_type;
			line += "(" + GoIdentity(fld->name) + ")";
		} else{
			line += GoIdentity(fld->name);
		}
		line += ", " + GenConstant(*fld) + ")";

		code_ += line;
		code_ += "}\n";
	}

	// Set the value of one of the members of a table's vector.
	void BuildVectorOfTable(FieldDef const *fld) {
		code_ += "{{STRUCT_BUILDER_RECEIVER}} Start" + MakeCamel(fld->name)
			+ "Vector(numElems int) flatbuffers.UOffsetT {";

		auto vector_type = fld->value.type.VectorType();
		auto alignment = InlineAlignment(vector_type);
		auto elem_size = InlineSize(vector_type);

		code_ += "\treturn b.Builder.StartVector("
			+ NumToString(elem_size)
			+ ", numElems, " + NumToString(alignment) + ")";
		code_ += "}\n";
	}

	// Generate a struct field getter, conditioned on its child type(s).
	void GenStructAccessor(StructDef const &def, FieldDef const *fld) {
		GenComment(fld->doc_comment, nullptr, "");
		if (IsScalar(fld->value.type.base_type)) {
			GenScalarFieldGetter(fld, def.fixed);
		} else {
			switch (fld->value.type.base_type) {
			case BASE_TYPE_STRUCT:
				if (def.fixed) {
					GenStructFieldOfStructGetter(fld);
				} else {
					GenStructFieldOfTableGetter(fld);
				}
				break;
			case BASE_TYPE_STRING:
				GenStringFieldGetter(fld);
				break;
			case BASE_TYPE_VECTOR: {
				auto vectortype(fld->value.type.VectorType());
				if (vectortype.base_type == BASE_TYPE_STRUCT) {
					GenMemberOfVectorOfStructGetter(fld);
				} else {
					GenMemberOfVectorOfNonStructGetter(fld);
				}
				break;
			}
			case BASE_TYPE_UNION:
				GenUnionFieldGetter(fld);
				break;
			default:
				FLATBUFFERS_ASSERT(0);
			}
		}

		if (fld->value.type.base_type == BASE_TYPE_VECTOR) {
			GenVectorLenGetter(fld);
			if (fld->value.type.element == BASE_TYPE_UCHAR) {
				GenUByteSliceGetter(fld);
			}
		}
	}

	// Mutate the value of a struct's scalar.
	void MutateScalarFieldOfStruct(FieldDef const *fld) {
		std::string type = MakeCamel(ToBasicType(fld->value.type));
		std::string setter = "rcv._tab.Mutate" + type;

		code_ += "{{STRUCT_RECEIVER}} Mutate" + MakeCamel(fld->name)
			+ "(n " + TypeName(*fld) + ") bool {";
		code_ += "\treturn " + setter
			+ "(rcv._tab.Pos+flatbuffers.UOffsetT("
			+ NumToString(fld->value.offset) + "), n)";
		code_ += "}\n";
	}

	// Mutate the value of a table's scalar.
	void MutateScalarFieldOfTable(FieldDef const *fld) {
		std::string basic_type = ToBasicType(fld->value.type);
		std::string setter = "rcv._tab.Mutate" + MakeCamel(basic_type) + "Slot";
		std::string actual_type;
		std::string arg;

		if (fld->value.type.enum_def) {
			actual_type = GetEnumType(fld->value.type.enum_def);
			arg = basic_type + "(n)";
		} else {
			actual_type = GetTypeName(fld->value.type);
			arg = "n";
		}

		code_ += "{{STRUCT_RECEIVER}} Mutate" + MakeCamel(fld->name)
			+ "(n " + actual_type + ") bool {";

		code_ += "\treturn " + setter + "("
			+ NumToString(fld->value.offset) + ", " + arg +")";
		code_ += "}\n";
	}

	// Generate a struct field setter, conditioned on its child type(s).
	void GenStructMutator(const StructDef &def, FieldDef const *fld) {
		GenComment(fld->doc_comment, nullptr, "");
		if (IsScalar(fld->value.type.base_type)) {
			if (def.fixed) {
				MutateScalarFieldOfStruct(fld);
			} else {
				MutateScalarFieldOfTable(fld);
			}
		}
	}

	// Generate table constructors, conditioned on its members' types.
	void GenTableBuilders(StructDef const &def) {
		code_ += "type {{STRUCT_NAME}}Builder struct {";
		code_ += "\tBuilder *flatbuffers.Builder";
		code_ += "}\n";
		code_ += "func Build{{STRUCT_NAME}}(builder *flatbuffers.Builder) {{STRUCT_NAME}}Builder {";
		code_ += "\tbuilder.StartObject(" + NumToString(def.fields.vec.size()) + ")";
		code_ += "\treturn {{STRUCT_NAME}}Builder {";
		code_ += "\t\tBuilder: builder,";
		code_ += "\t}";
		code_ += "}\n";

		code_.SetValue(
			"STRUCT_BUILDER_RECEIVER",
			"func (b " + def.name + "Builder)"
		);

		int offset(0);
		for (auto fld: def.fields.vec) {
			if (fld->deprecated)
				continue;

			BuildFieldOfTable(def, fld, offset++);
			if (fld->value.type.base_type == BASE_TYPE_VECTOR) {
				BuildVectorOfTable(fld);
			}
		}

		code_ += "{{STRUCT_BUILDER_RECEIVER}} End() flatbuffers.UOffsetT {";
		code_ += "\treturn b.Builder.EndObject()";
		code_ += "}\n";
	}

	// Generate struct or table methods.
	void GenStruct(StructDef const &def) {
		if (def.generated)
			return;

		code_.SetValue("STRUCT_NAME", def.name);
		code_.SetValue(
			"STRUCT_RECEIVER",
			std::string("func (rcv *") + def.name + ')'
		);

		if (parser_.opts.generate_object_based_api) {
			GenComment(def.doc_comment, nullptr);
			GenNativeStruct(def);
		}

		GenComment(def.doc_comment, nullptr);
		code_ += "type {{STRUCT_NAME}} struct {";
		// _ is reserved in flatbuffers field names, so no chance of name conflict:
		code_ += "\t_tab \\";
		code_ += def.fixed ? "flatbuffers.Struct" : "flatbuffers.Table";
		code_ += "}\n";

		if (!def.fixed) {
			// Generate a special accessor for the table that has been declared as
			// the root type.

			code_ += "func GetRootAs{{STRUCT_NAME}}(buf []byte, offset flatbuffers.UOffsetT) *{{STRUCT_NAME}} {";
			code_ += "\tn := flatbuffers.GetUOffsetT(buf[offset:])";
			code_ += "\tx := &{{STRUCT_NAME}}{}";
			code_ += "\tx.Init(buf, n + offset)";
			code_ += "\treturn x";
			code_ += "}\n";

			code_ += "{{STRUCT_RECEIVER}} Table() flatbuffers.Table {";
			code_ += "\treturn rcv._tab";
			code_ += "}\n";
		} else {
			code_ += "{{STRUCT_RECEIVER}} Table() flatbuffers.Table {";
			code_ += "\treturn rcv._tab.Table";
			code_ += "}\n";
		}

		// Generate the Init method that sets the field in a pre-existing
		// accessor object. This is to allow object reuse.
		code_ += "{{STRUCT_RECEIVER}} Init(buf []byte, i flatbuffers.UOffsetT) {";
		code_ += "\trcv._tab.Bytes = buf";
		code_ += "\trcv._tab.Pos = i";
		code_ += "}\n";


		// Generate struct fields accessors
		for (auto fld: def.fields.vec) {
			if (fld->deprecated)
				continue;

			GenStructAccessor(def, fld);
			GenStructMutator(def, fld);
		}

		// Generate builders
		if (def.fixed) {
			// create a struct constructor function
			GenStructBuilder(def);
		} else {
			// Create a set of functions that allow table construction.
			GenTableBuilders(def);
			GenTableCreate(def);
		}

		if (parser_.opts.generate_object_based_api) {
			GenNativeMarshal(def);
			GenNativeUnmarshal(def);
		}
	}

	// Generate enum declarations.
	void GenEnum(EnumDef const *def) {
		if (def->generated)
			return;

		code_.SetValue("ENUM_NAME", def->name);
		code_.SetValue("ENUM_TYPE", GetEnumType(def));

		GenComment(def->doc_comment, nullptr);

		code_ += "type {{ENUM_TYPE}} = " + ToBasicType(
			def->underlying_type
		) + "\n";

		code_ += "const (";

		for (auto it: def->vals.vec) {
			GenComment(it->doc_comment, nullptr, "\t");

			code_ += "\t{{ENUM_NAME}}" + it->name + " {{ENUM_TYPE}}"
				+ " = " + NumToString(it->value);
		}

		code_ += ")\n";

		code_ += "var EnumNames{{ENUM_NAME}} = map[{{ENUM_TYPE}}]string {";

		for (auto it: def->vals.vec) {
			code_ += "\t{{ENUM_NAME}}" + it->name + ": \""
				+ it->name + "\",";
		}

		code_ += "}\n";

		if (parser_.opts.generate_object_based_api && def->is_union)
			GenEnumInterface(def);
	}

	void GenEnumInterface(EnumDef const *def) {
		code_.SetValue("ENUM_UNION_TYPE", GetEnumType(def, true));
		code_.SetValue(
			"ENUM_VALUE_TAG", "is" + GetEnumType(def) + "_Value"
		);
		code_.SetValue(
			"ENUM_UNION_RECEIVER",
			"func (u *" + GetEnumType(def, true) + ")"
		);

		code_ += "type {{ENUM_UNION_TYPE}} struct {";
		code_ += "\t{{ENUM_VALUE_TAG}}";
		code_ += "}\n";

		code_ += "type {{ENUM_VALUE_TAG}} interface {";
		code_ += "\t{{ENUM_VALUE_TAG}}()";
		code_ += "}\n";

		for (auto it: def->vals.vec) {
			std::string ct_name = "{{ENUM_TYPE}}_" + it->name;
			auto ct_value_type(GetRefType(it->union_type, true));

			code_ += "type " + ct_name + " struct {";
			code_ += "\tValue " + ct_value_type;
			code_ += "}\n";

			code_ += "func (" + ct_name + ") {{ENUM_VALUE_TAG}}() {}\n";

			code_ += "{{ENUM_UNION_RECEIVER}} Get" + it->name
				+ "() " + ct_value_type + " {";
			code_ += "\treturn u.{{ENUM_VALUE_TAG}}.(" + ct_name + ").Value";
			code_ += "}\n";
		}
	}

	// Returns the function name that is able to read a value of the given type.
	std::string GetGetterForType(const Type &type) {
		switch (type.base_type) {
		case BASE_TYPE_STRING:
			return "rcv._tab.String";
		case BASE_TYPE_UNION:
			return "rcv._tab.Union";
		case BASE_TYPE_VECTOR:
			return GetGetterForType(type.VectorType());
		default:
			return "rcv._tab.Get" + MakeCamel(ToBasicType(type));
		}
	}

	// Returns the method name for use with add/put calls.
	std::string GetMethodName(FieldDef const *fld) {
		return IsScalar(fld->value.type.base_type)
			? MakeCamel(ToBasicType(fld->value.type))
			: (IsStruct(fld->value.type) ? "Struct" : "UOffsetT");
	}

	std::string GetPointerTypeName(const Type &type) {
		switch (type.base_type) {
			case BASE_TYPE_STRING:
				return "string";
			case BASE_TYPE_VECTOR:
				return GetTypeName(type.VectorType());
			case BASE_TYPE_STRUCT:
				return GetStructRefType(
					type.struct_def, false, false
				);
			case BASE_TYPE_UNION:
				// fall through
			default:
				return "*flatbuffers.Table";
		}
	}

	std::string GetTypeName(const Type &type) {
		if (type.enum_def != nullptr && !type.enum_def->is_union) {
			return GetEnumType(type.enum_def);
		}

		return IsScalar(type.base_type)
			? ToBasicType(type) : GetPointerTypeName(type);
	}

	std::string TypeName(const FieldDef &field) {
		return GetTypeName(field.value.type);
	}

	std::string GenConstant(const FieldDef &field) {
		switch (field.value.type.base_type) {
			case BASE_TYPE_BOOL:
				return field.value.constant == "0" ? "false" : "true";
			default:
				return field.value.constant;
		}
	}

	// Recursively generate arguments for a constructor, to deal with nested
	// structs.
	void GenStructBuilderArgs(StructDef const &def, std::string nameprefix) {
		for (auto fld: def.fields.vec) {
			if (IsStruct(fld->value.type)) {
				// Generate arguments for a struct inside a struct. To ensure names
				// don't clash, and to make it obvious these arguments are constructing
				// a nested struct, prefix the name with the field name.
				GenStructBuilderArgs(
					*(fld->value.type.struct_def),
					(nameprefix + (fld->name + "_"))
				);
			} else {
				code_ += "\t" + nameprefix + GoIdentity(fld->name)
					+ " " + ToBasicType(fld->value.type) + ",";
			}
		}
	}

	// Recursively generate struct construction statements and instert manual
	// padding.
	void GenStructBuilderBody(
		StructDef const &def, std::string nameprefix
	) {
		code_ += "\tbuilder.Prep(" + NumToString(def.minalign)
			+ ", " + NumToString(def.bytesize) + ")";

		for (
			auto fld = def.fields.vec.rbegin();
			fld != def.fields.vec.rend();
			++fld
		) {
			if ((*fld)->padding)
				code_ += "\tbuilder.Pad(" + NumToString((*fld)->padding) + ")";

			if (IsStruct((*fld)->value.type)) {
				GenStructBuilderBody(
					*((*fld)->value.type.struct_def),
					nameprefix + ((*fld)->name + "_")
				);
			} else {
				code_ += "\tbuilder.Prepend"
					+ GetMethodName(*fld) + "("
					+ nameprefix + GoIdentity((*fld)->name) + ")";
			}
		}
	}

	// Create a struct with a builder and the struct's arguments.
	void GenStructBuilder(StructDef const &def) {
		code_ += "func Create{{STRUCT_NAME}}(";
		code_ += "\tbuilder *flatbuffers.Builder,";

		GenStructBuilderArgs(def, "");

		code_ += ") flatbuffers.UOffsetT {";

		GenStructBuilderBody(def, "");

		code_ += "\treturn builder.Offset()";
		code_ += "}\n";
	}

	void GenTableCreate(StructDef const &def) {
		code_ += "func Create{{STRUCT_NAME}}(";
		code_ += "\tbuilder *flatbuffers.Builder,";
		std::string line;

		for (auto fld: def.fields.vec) {
			if (fld->deprecated)
				continue;

			auto &type = fld->value.type;
			line = "\t" + GoIdentity(fld->name) + " ";
			if (!IsScalar(type.base_type)) {
				line += "flatbuffers.UOffsetT";
			} else if (type.enum_def) {
				line += GetEnumType(type.enum_def);
			} else {
				line += ToBasicType(type);
			}
			code_ += line + ",";
			line.clear();
		}

		code_ += ") flatbuffers.UOffsetT {";
		code_ += "\tb := Build{{STRUCT_NAME}}(builder)";

		for (auto fld: def.fields.vec) {
			if (fld->deprecated)
				continue;

			code_ += "\tb.Add" + MakeCamel(fld->name)
				+ "(" + GoIdentity(fld->name) + ")";
		}
		code_ += "\treturn b.End()";
		code_ += "}\n";
	}

	void AddToImports(Namespace const *ns) {
		auto is = go_import_prefix_;
		for (auto &c: ns->components) {
			is += "/";
			is += c;
		}
		imports_.insert(is);
	}

	std::string GetEnumType(EnumDef const *def, bool is_union = false) {
		std::string ns;
		if (def->defined_namespace != current_namespace_) {
			AddToImports(def->defined_namespace);
			if (!parser_.opts.one_file)
				ns = LastNamespacePart(
					*(def->defined_namespace)
				) + ".";
		}

		return ns + GoIdentity(def->name)
			+ (is_union ? "Union" : "");
	}

	std::string GetStructRefType(
		StructDef const *def, bool native, bool allow_ptr
	) {
		std::string ns;
		if (def->defined_namespace != current_namespace_) {
			AddToImports(def->defined_namespace);
			if (!parser_.opts.one_file)
				ns = LastNamespacePart(
					*(def->defined_namespace)
				) + ".";
		}

		std::string ptr;

		if (allow_ptr && !def->fixed)
			ptr = "*";

		return ptr + ns + (native ? NativeName(*def) : def->name);
	}

	std::string GetRefTypeComposite(Type const &type, bool native) {
		switch (type.base_type) {
		case BASE_TYPE_STRING:
			return "string";
		case BASE_TYPE_VECTOR:
			return "[]" + GetRefType(type.VectorType(), native);
		case BASE_TYPE_STRUCT:
			return GetStructRefType(type.struct_def, native, true);
		case BASE_TYPE_UNION:
			// fall through
		default:
			needs_common_imports_ = true;
			return "*flatbuffers.Table";
		}
	}

	std::string GetRefType(Type const &type, bool native) {
		if (type.enum_def != nullptr && !type.enum_def->is_union) {
			return GetEnumType(type.enum_def);
		}

		return IsScalar(type.base_type)
			? ToBasicType(type)
			: GetRefTypeComposite(type, native);
	}

	std::string NativeFieldType(FieldDef const *field) {
		auto& type(field->value.type);

		if (type.enum_def) {
			return GetEnumType(
				type.enum_def,
				type.base_type == BASE_TYPE_UNION
			);
		}

		if (IsScalar(type.base_type))
			return ToBasicType(type);

		switch (type.base_type) {
			case BASE_TYPE_STRING:
				return "string";
			case BASE_TYPE_VECTOR:
				return "[]" + GetRefType(type.VectorType(), true);
			case BASE_TYPE_STRUCT:
				return GetStructRefType(
					type.struct_def, true, true
				);
			case BASE_TYPE_UNION:
				// fall through
			default:
				return "*flatbuffers.Table";
		}
	}

	void GenNativeStruct(const StructDef &def) {
		code_.SetValue("NATIVE_STRUCT_NAME", NativeName(def));
		code_.SetValue(
			"NATIVE_STRUCT_RECEIVER",
			std::string("func (rcv *") + NativeName(def) + ')'
		);

		code_ += "type {{NATIVE_STRUCT_NAME}} struct {";
		// Generate struct fields
		for (auto fld: def.fields.vec) {
			if (fld->deprecated)
				continue;

			code_ += "\t" + GoIdentity(fld->name, true)
				+ " " + NativeFieldType(fld);
		}
		code_ += "}\n";
	}

	GoGenerator(
		const Parser &parser,
		const std::string &path,
		const std::string &file_name,
		const std::string &go_import_prefix
	) : BaseGenerator(
		parser, path, file_name, "", ""
	), go_import_prefix_(go_import_prefix) {}

	bool generate() {
		for (auto it: parser_.enums_.vec) {
			current_namespace_ = it->defined_namespace;
			GenEnum(it);
			if (!parser_.opts.one_file) {
				if (!SaveType(*it))
					return false;
			}
		}

		for (auto it: parser_.structs_.vec) {
			needs_common_imports_ = true;
			current_namespace_ = it->defined_namespace;
			GenStruct(*it);
			if (!parser_.opts.one_file) {
				if (!SaveType(*it))
					return false;
			}
		}

		return true;
	}

	void clear() {
		imports_.clear();
		current_namespace_ = nullptr;
		code_.Clear();
		needs_common_imports_ = false;
	}

	// Begin by declaring namespace and imports.
	void BeginFile(const std::string name_space_name, std::string &code) {
		code = code + "// Code generated by the FlatBuffers compiler. DO NOT EDIT.\n\n";
		code += "package " + name_space_name + "\n\n";

		if (needs_common_imports_ || !imports_.empty()) {
			code += "import (\n";
		}

		if (needs_common_imports_) {
			if (!parser_.opts.go_import.empty()) {
				code += "\tflatbuffers \"" + parser_.opts.go_import + "\"\n";
			} else {
				code += "\tflatbuffers \"github.com/google/flatbuffers/go\"\n";
			}
		}

		for (auto& is: imports_) {
			code += "\t\"" + is + "\"\n";
		}

		if (needs_common_imports_ || !imports_.empty()) {
			code += ")\n\n";
		}
	}

	// Save out the generated code for a Go Table type.
	bool SaveType(Definition const &def) {
		Cleaner cl(this);

		auto typecode(code_.ToString());

		if (typecode.empty()) {
			return true;
		}

		Namespace &ns = *def.defined_namespace;

		std::string code;
		BeginFile(LastNamespacePart(ns), code);
		code += typecode;
		std::string filename = NamespaceDir(ns) + def.name + ".go";

		return SaveFile(filename.c_str(), code, false);
	}

	struct Cleaner {
		Cleaner(GoGenerator *self)
		: self_(self) {}

		~Cleaner() {
			self_->clear();
		}

		GoGenerator *self_;
	};

	const std::string go_import_prefix_;
	std::set<std::string> imports_;
	Namespace *current_namespace_;
	CodeWriter code_;
	bool needs_common_imports_;
};
} // namespace go

bool GenerateGo(
	const Parser &parser,
	const std::string &path,
	const std::string &file_name
) {
	go::GoGenerator generator(
		parser, path, file_name, parser.opts.go_namespace
	);
	return generator.generate();
}

} // namespace flatbuffers
