/*
 * Copyright 2018 Dolby Laboratories. All rights reserved.
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

// loosely based on idl_gen_python.cpp

#include <iostream>
#include <list>
#include <string>
#include <unordered_set>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {
namespace julia {

const std::string Indent = "    ";
const std::string JuliaFileExtension = ".jl";
const std::string JuliaPackageName = "FlatBuffers";

// Documentation comments
const CommentConfig JuliaCommentConfig = { "#=", "# ", "=#" };

// Class to represent a dependency graph
// with a topological sort operation
class DepGraph {
 public:
  // adjacency list
  std::vector<std::vector<unsigned int> *> adj;
  // node names
  std::vector<std::string> nodes;
  unsigned int GetOrCreateNodeID(std::string name) {
    auto it = std::find(nodes.begin(), nodes.end(), name);
    if (it == nodes.end()) {
      nodes.push_back(name);
      return nodes.size() - 1;
    }
    return std::distance(nodes.begin(), it);
  }
  DepGraph() {}
  ~DepGraph() {
    for (unsigned int i = 0; i < adj.size(); i++) delete adj[i];
  }
  void AddDep(std::string parent, std::string child) {
    unsigned int p = GetOrCreateNodeID(parent);
    unsigned int c = GetOrCreateNodeID(child);
    // n = max(p, c)
    unsigned int n = p > c ? p : c;
    // if we just created new node ids,
    // grow the adjacency list to the new size
    if (n >= adj.size()) {
      for (unsigned int i = adj.size(); i <= n; i++)
        adj.push_back(new std::vector<unsigned int>());
    }
    adj[p]->push_back(c);
  }
  // topological sort
  bool TopSort(unsigned int v, bool visited[], bool visiting[],
               std::list<unsigned int> &stack) {
    bool has_cycle = false;
    visited[v] = true;
    visiting[v] = true;
    for (auto it = adj[v]->begin(); it != adj[v]->end(); ++it) {
      if (!visited[*it]) has_cycle = TopSort(*it, visited, visiting, stack);
      if (visiting[*it] && nodes[v] != nodes[*it]) {
        printf(
            "warning: %s\n",
            ("Circular reference detected (" + nodes[v] + " -> " + nodes[*it] +
             "). " + "Julia does not currently suport such definitions.")
                .c_str());
        has_cycle = true;
      }
    }
    visiting[v] = false;
    stack.push_back(v);
    return has_cycle;
  }
  // Topological sort of dependency graph, so we can include
  // julia files in the right order
  std::vector<std::string> TopSort() {
    std::vector<std::string> sorted_nodes;
    std::list<unsigned int> stack;
    unsigned int n = adj.size();
    bool *visited = new bool[n];
    bool *visiting = new bool[n];
    for (unsigned int i = 0; i < n; i++) {
      visited[i] = false;
      visiting[i] = false;
    }

    bool has_cycle = false;
    for (unsigned int i = 0; i < n; i++)
      if (!visited[i]) has_cycle |= TopSort(i, visited, visiting, stack);

    while (!stack.empty()) {
      unsigned int id = stack.back();
      // don't return nodes on which nothing depends
      if (!adj[id]->empty()) sorted_nodes.push_back(nodes[id]);
      stack.pop_back();
    }
    return sorted_nodes;
  }
};

// Singleton class which keeps track of generated modules
// and their dependencies. A singleton class is necessary
// since our generator is run multiple times with
// multiple different .fbs files, and we want to preserve
// the set of module dependencies across these runs.
// We need to keep track of dependencies since Julia doesn't
// support forward declarations of types.
class ModuleTable {
 public:
  static ModuleTable &GetInstance() {
    static ModuleTable instance;
    return instance;
  }
  bool IsModule(const std::string &m) {
    return modules_.find(m) != modules_.end();
  }
  void AddFile(const std::string &f) { files_.insert(f); }
  bool IsFile(const std::string &f) { return files_.find(f) != files_.end(); }
  void AddDependency(std::string mod, std::string parent, std::string child) {
    if (!IsModule(mod)) modules_[mod] = new DepGraph();
    modules_[mod]->AddDep(parent, child);
  }
  std::vector<std::string> SortedModuleNames() {
    std::set<std::string> module_names;
    for (auto it = modules_.begin(); it != modules_.end(); ++it)
      module_names.insert(it->first);
    // Make sure parent modules come before child modules
    std::vector<std::string> sorted_modules(module_names.begin(),
                                            module_names.end());
    std::sort(sorted_modules.begin(), sorted_modules.end());
    return sorted_modules;
  }
  DepGraph *GetDependencies(std::string module) { return modules_[module]; }

 private:
  std::map<std::string, DepGraph *> modules_;
  std::set<std::string> files_;
  ModuleTable() {}
  ~ModuleTable() {
    for (auto it = modules_.begin(); it != modules_.end(); ++it)
      delete it->second;
  }

  ModuleTable(ModuleTable const &);
  void operator=(ModuleTable const &);
};

class JuliaGenerator : public BaseGenerator {
 public:
  JuliaGenerator(const Parser &parser, const std::string &path,
                 const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "" /* not used */,
                      "" /* not used */),
        root_module_(MakeCamel(file_name_)),
        module_table_(ModuleTable::GetInstance()) {
  }

  ~JuliaGenerator() {}
  bool generate() {
    if (!GenEnums()) return false;
    if (!GenStructs()) return false;
    if (!GenModules()) return false;
    return true;
  }

 private:
  // the root module is the name of the .fbs file which
  // we are compiling, in camel case
  const std::string root_module_;
  ModuleTable &module_table_;
  static const std::unordered_set<std::string> keywords_;

  bool GenEnums(void) {
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      auto &enum_def = **it;
      std::string enumcode;
      if (enum_def.is_union)
        GenUnion(enum_def, &enumcode);
      else
        GenEnum(enum_def, &enumcode);
      if (!SaveType(enum_def, enumcode)) return false;
    }
    return true;
  }

  bool GenStructs(void) {
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      auto &struct_def = **it;
      std::string declcode;
      GenObject(struct_def, &declcode);
      if (!SaveType(struct_def, declcode)) return false;
    }
    return true;
  }

  bool GenModules(void) {
    for (auto it = parser_.namespaces_.begin(); it != parser_.namespaces_.end();
         ++it) {
      std::string parent;
      std::string child;
      // Gather all parent namespaces for this namespace
      // TODO: this is similar to idl_gen_js.cpp, maybe
      // move into common place?
      for (auto component = (*it)->components.begin();
           component != (*it)->components.end(); ++component) {
        if (parent.empty()) {
          parent = *component;
          child = *component;
          module_table_.AddDependency(root_module_, parent, child);
        } else {
          child = parent + kPathSeparator + *component;
          // Add component to parent's list of children
          module_table_.AddDependency(parent, child, *component);
        }
        parent = child;
      }
    }
    auto sorted_modules = module_table_.SortedModuleNames();
    bool save_root = false;
    // iterate through child modules first, then parents
    for (auto m = sorted_modules.rbegin(); m != sorted_modules.rend(); ++m) {
      if (*m == root_module_) {
        save_root = true;
        continue;
      }
      SaveModule(false, *m, module_table_.GetDependencies(*m));
    }
    // save the root module last
    if (save_root)
      SaveModule(true, root_module_,
                 module_table_.GetDependencies(root_module_));
    return true;
  }

  // Begin an object declaration.
  static void BeginObject(const StructDef &struct_def, std::string *code_ptr,
                          bool has_defaults) {
    std::string &code = *code_ptr;
    if (has_defaults) code += JuliaPackageName + ".@with_kw ";
    if (!struct_def.fixed)
      code += "mutable struct ";
    else
      code += JuliaPackageName + ".@STRUCT struct ";
    code += NormalizedName(struct_def) + "\n";
  }

  void EndObject(const StructDef &struct_def,
                 const std::vector<std::string> &offsets,
                 std::string *code_ptr) const {
    std::string &code = *code_ptr;
    std::string name = NormalizedName(struct_def);
    code += "end\n";
    code += JuliaPackageName + ".@ALIGN(" + name + +", " +
            NumToString(struct_def.minalign) + ")\n";
    std::string method_signature = "(::Type{T}) where {T<:" + name + "}";
    if (!offsets.empty() && !struct_def.fixed) {
      bool first = true;
      int i = 0;
      code += JuliaPackageName + ".slot_offsets" + method_signature;
      code += " = [";
      for (auto it = offsets.begin(); it != offsets.end(); ++it) {
        if (!first) code += ", ";
        if (i == 0) code += "\n" + Indent;
        code += *it;
        i++;
        i %= 4;  // print four offsets per line
        first = false;
      }
      code += "\n]\n";
    }

    // emit file identifier and extension for the root type
    if (parser_.root_struct_def_ == &struct_def) {
      code += JuliaPackageName + ".root_type" + method_signature + " = true\n";
      if (!parser_.file_identifier_.empty()) {
        code += JuliaPackageName + ".file_identifier" + method_signature;
        code += " = \"" + parser_.file_identifier_ + "\"\n";
      }
      if (!parser_.file_extension_.empty()) {
        code += JuliaPackageName + ".file_extension" + method_signature;
        code += " = \"" + parser_.file_extension_ + "\"\n";
      }
    }
    code += "\n";
  }

  static std::string EscapeKeyword(const std::string &name) {
    return keywords_.find(name) == keywords_.end() ? name : name + "_";
  }

  static std::string const NormalizedName(const Definition &child,
                                          const Definition *parent = NULL) {
    std::string prefix = "";
    if (parent != NULL) {
      std::string relname = GetRelativeName(*parent, &child, false);
      if (!relname.empty()) prefix = relname + ".";
    }
    return prefix + EscapeKeyword(child.name);
  }

  static std::string NormalizedName(const EnumVal &ev) {
    return EscapeKeyword(ev.name);
  }

  static void BeginEnum(const std::string enum_name,
                        const std::string enum_type, std::string *code_ptr) {
    *code_ptr += "@enum " + enum_name + "::" + enum_type + " begin\n";
  }

  static void EnumMember(const std::string enum_name, const EnumVal ev,
                         std::string *code_ptr) {
    *code_ptr += Indent + enum_name + NormalizedName(ev);
    *code_ptr += " = " + NumToString(ev.value) + "\n";
  }

  static void EndEnum(std::string *code_ptr) { *code_ptr += "end\n\n"; }

  static void BeginUnion(const std::string union_name, std::string *code_ptr) {
    *code_ptr += JuliaPackageName + ".@UNION(" + union_name + ", (\n";
  }

  static void UnionMember(const std::string type_name, std::string *code_ptr) {
    *code_ptr += Indent + type_name + ",\n";
  }

  static void EndUnion(std::string *code_ptr) { *code_ptr += "))\n\n"; }

  static void NewObjectFromBuffer(const StructDef &struct_def,
                                  std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += NormalizedName(struct_def) + "(buf::AbstractVector{UInt8})";
    code += " = " + JuliaPackageName + ".read(" + NormalizedName(struct_def) +
            ", buf)\n";
    code += NormalizedName(struct_def) + "(io::IO) = " + JuliaPackageName;
    code += ".deserialize(io, " + NormalizedName(struct_def) + ")\n";
  }

  void GenScalarField(const StructDef &struct_def, const FieldDef &field,
                      std::string *code_ptr, bool *has_defaults,
                      std::set<std::string> *imports_ptr) {
    std::string &code = *code_ptr;
    std::string field_name = NormalizedName(field);
    code += Indent + field_name;
    code += "::";
    code += GenTypeGet(field.value.type);
    if (!field.value.constant.empty() && !struct_def.fixed) {
      *has_defaults = true;
      std::string c = field.value.constant;
      if (field.value.type.base_type == BASE_TYPE_BOOL) {
        c = c == "0" ? "false" : "true";
      }
      code += " = " + c;
    }
    if (IsScalarEnum(field.value.type))
      AddDependency(struct_def, field.value.type, imports_ptr);
    code += "\n";
  }

  // whether two namespaces have the same prefix up to "prefix_size"
  // used for figuring out relative import paths
  static bool SameNamespacePrefix(Namespace n1, Namespace n2,
                                  size_t prefix_size) {
    if (n1.components.size() < prefix_size) return false;
    if (n2.components.size() < prefix_size) return false;
    size_t i;
    for (i = 0; i < prefix_size; i++) {
      if (n1.components[i] != n2.components[i]) return false;
    }
    return true;
  }

  static void GetRelativeNamespaces(const Definition &parent_def,
                                    const Definition *child_def,
                                    Namespace *parent, Namespace *child) {
    Namespace p, c;
    if (parent_def.defined_namespace != NULL) p = *parent_def.defined_namespace;
    if (child_def != NULL && child_def->defined_namespace != NULL)
      c = *child_def->defined_namespace;
    *parent = p;
    *child = c;
  }

  static void GetRelativeNamespaces(const Definition &def, const Type &type,
                                    Namespace *parent, Namespace *child) {
    Namespace p, c;
    Definition *child_def = GetBaseDefinition(type);
    GetRelativeNamespaces(def, child_def, parent, child);
  }

  static std::string GetRelativeName(const Namespace &parent,
                                     const Namespace &child,
                                     bool dotprefix = true) {
    // here we are accounting for modules which are at
    // different levels of the tree.
    std::string relname = "";
    // go up to common level (don't add dots to path if we are
    // annotating the type of a field - julia doesn't like that)
    unsigned int i = 0;
    unsigned int n = parent.components.size();
    while (i <= n && !SameNamespacePrefix(parent, child, n - i)) {
      if (dotprefix) relname += ".";
      i++;
    }
    // traverse down to the place we need to be
    unsigned int m = parent.components.size() - i;
    unsigned int j;
    for (j = m; j < child.components.size(); j++) {
      if (dotprefix || !relname.empty()) relname += ".";
      relname += child.components[j];
    }
    return relname;
  }

  static std::string GetRelativeName(const Definition &parent_def,
                                     const Definition *child_def,
                                     bool dotprefix = false) {
    Namespace parent, child;
    GetRelativeNamespaces(parent_def, child_def, &parent, &child);
    return GetRelativeName(parent, child, dotprefix);
  }

  static std::string GetRelativeName(const Definition &def, const Type &type,
                                     bool dotprefix = false) {
    Namespace parent, child;
    GetRelativeNamespaces(def, type, &parent, &child);
    return GetRelativeName(parent, child, dotprefix);
  }

  // This definition depends on this type. Add the necessary relative imports.
  void AddDependency(const Definition &def, const Type &type,
                     std::set<std::string> *imports_ptr) {
    Namespace parent, child;
    Definition *child_def = GetBaseDefinition(type);
    if (child_def == NULL) return;

    GetRelativeNamespaces(def, child_def, &parent, &child);
    std::string relname = GetRelativeName(parent, child);

    // add relative import
    if (!relname.empty()) {
      // special case: we are importing something from a direct parent
      if (relname.find_first_not_of('.') == std::string::npos)
        imports_ptr->insert(relname + "." + NormalizedName(*child_def));
      else
        imports_ptr->insert(relname);
    }

    std::string module = GetCanonicalName(child);
    std::string child_name =
        GetCanonicalName(child) + kPathSeparator + NormalizedName(*child_def);
    std::string parent_name =
        GetCanonicalName(parent) + kPathSeparator + NormalizedName(def);
    // self-reference - this is fine, but don't add it to the dependency table
    if (child_name == parent_name) return;
    module_table_.AddDependency(module, parent_name, child_name);
  }

  // generate a field which depends upon generated types
  void GenDependentField(const StructDef &struct_def, const FieldDef &field,
                         std::string *code_ptr,
                         bool *has_defaults,
                         std::set<std::string> *imports_ptr) {
    std::string type_name = GenTypeGet(field.value.type, &struct_def);

    BaseType bt = field.value.type.base_type;
    if ((bt == BASE_TYPE_STRUCT || bt == BASE_TYPE_VECTOR) &&
        !struct_def.fixed) {
      type_name = "Union{" + type_name + ", Nothing}";
    }
    // initialise nullable fields to nothing by default
    if ((bt == BASE_TYPE_STRUCT || bt == BASE_TYPE_UNION ||
         bt == BASE_TYPE_VECTOR || bt == BASE_TYPE_STRING) &&
        !struct_def.fixed) {
      type_name += " = nothing";
      *has_defaults = true;
    }
    *code_ptr += Indent + NormalizedName(field) + "::" + type_name + "\n";
    AddDependency(struct_def, field.value.type, imports_ptr);
  }

  static void GenStringField(const StructDef &struct_def, const FieldDef &field,
                             std::string *code_ptr, bool *has_defaults) {
    *code_ptr += Indent + NormalizedName(field) + "::";
    // initialise strings to be empty by default
    *code_ptr += "Union{" + GenTypeGet(field.value.type) + ", Nothing}";
    if (!struct_def.fixed) {
      *code_ptr += " = nothing";
      *has_defaults = true;
    }
    *code_ptr += "\n";
  }

  // Generate a field, conditioned on its child type(s).
  void GenField(const StructDef &struct_def, const FieldDef &field,
                std::string *code_ptr, std::set<std::string> *imports_ptr,
                bool *has_defaults) {
    GenComment(field.doc_comment, code_ptr, &JuliaCommentConfig);
    if (IsScalar(field.value.type.base_type)) {
      GenScalarField(struct_def, field, code_ptr, has_defaults, imports_ptr);
    } else {
      switch (field.value.type.base_type) {
        case BASE_TYPE_STRING:
          GenStringField(struct_def, field, code_ptr, has_defaults);
          break;
        case BASE_TYPE_STRUCT:
        case BASE_TYPE_VECTOR:
        case BASE_TYPE_UNION:
          GenDependentField(struct_def, field, code_ptr, has_defaults, imports_ptr);
          break;
        default: FLATBUFFERS_ASSERT(0);
      }
    }
  }

  static std::string GenImports(const std::set<std::string> &imports) {
    std::string impstr = "";
    for (auto it = imports.begin(); it != imports.end(); ++it)
      impstr += "import " + *it + "\n";
    return impstr;
  }

  // Generate structs/tables
  void GenObject(const StructDef &struct_def, std::string *code_ptr) {
    if (struct_def.generated) return;

    // always need FlatBuffers package for structs
    std::set<std::string> imports;
    imports.insert(JuliaPackageName);

    bool has_defaults = false;

    // generate all the fields
    std::vector<std::string> offsets;

    GenComment(struct_def.doc_comment, code_ptr, &JuliaCommentConfig);
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      GenField(struct_def, field, code_ptr, &imports, &has_defaults);
      offsets.push_back("0x" + IntToStringHex(field.value.offset, 8));
    }
    EndObject(struct_def, offsets, code_ptr);

    // need to call BeginObject after EndObject because we don't know
    // the defaults until we've looked at all the fields.
    std::string struct_begin;
    BeginObject(struct_def, &struct_begin, has_defaults);

    *code_ptr = GenImports(imports) + "\n" + struct_begin + *code_ptr;

    // Generate a functions for constructing the object from a buffer
    NewObjectFromBuffer(struct_def, code_ptr);
  }

  void GenUnion(const EnumDef &enum_def, std::string *code_ptr) {
    if (enum_def.generated) return;

    // always need FlatBuffers package for unions
    std::set<std::string> imports = { JuliaPackageName };
    std::string union_name = NormalizedName(enum_def);
    BeginUnion(union_name, code_ptr);
    for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
         ++it) {
      auto &ev = **it;
      std::string type_name = GenTypeGet(ev.union_type, &enum_def);
      GenComment(ev.doc_comment, code_ptr, &JuliaCommentConfig);
      UnionMember(type_name, code_ptr);
      // special case, instead make every Union a union with Nothing
      if (ev.name == "NONE") continue;
      AddDependency(enum_def, ev.union_type, &imports);
    }
    EndUnion(code_ptr);

    *code_ptr = GenImports(imports) + "\n" + *code_ptr;
  }

  void GenEnum(const EnumDef &enum_def, std::string *code_ptr) {
    if (enum_def.generated) return;
    GenComment(enum_def.doc_comment, code_ptr, &JuliaCommentConfig);
    std::string enum_name = NormalizedName(enum_def);
    BeginEnum(enum_name, GenTypeBasic(enum_def.underlying_type), code_ptr);
    for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
         ++it) {
      auto &ev = **it;
      GenComment(ev.doc_comment, code_ptr, &JuliaCommentConfig);
      EnumMember(enum_name, ev, code_ptr);
    }
    EndEnum(code_ptr);
  }

  static std::string GenTypeBasic(const Type &type) {
    static const char *ctypename[] = {
// clang-format off
      #define FLATBUFFERS_TD(ENUM, IDLTYPE, \
        CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, RTYPE, JLTYPE) \
        #JLTYPE,
        FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
      #undef FLATBUFFERS_TD
      // clang-format on
    };
    return ctypename[type.base_type];
  }

  static std::string GenTypePointer(const Type &type,
                                    const Definition *parent) {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "String";
      case BASE_TYPE_VECTOR:
        return "Vector{" + GenTypeGet(type.VectorType(), parent) + "}";
      case BASE_TYPE_STRUCT: return NormalizedName(*type.struct_def, parent);
      case BASE_TYPE_UNION: return NormalizedName(*type.enum_def, parent);
      case BASE_TYPE_NONE: return "Nothing";
      default: return "Any";
    }
  }

  static Definition *GetBaseDefinition(const Type &type) {
    switch (type.base_type) {
      case BASE_TYPE_VECTOR: return GetBaseDefinition(type.VectorType());
      case BASE_TYPE_STRUCT: return type.struct_def;
      case BASE_TYPE_UNION: return type.enum_def;
      default: return NULL;
    }
  }

  static bool IsScalarEnum(const Type &type) {
    return (type.enum_def != nullptr && !type.enum_def->is_union &&
            IsInteger(type.enum_def->underlying_type.base_type));
  }

  static std::string GenTypeGet(const Type &type,
                                const Definition *parent = NULL) {
    if (IsScalar(type.base_type))
      return IsScalarEnum(type) ? NormalizedName(*type.enum_def, parent)
                                : GenTypeBasic(type);
    return GenTypePointer(type, parent);
  }

  void BeginFile(const std::string submodule_name,
                 std::string *code_ptr) const {
    std::string &code = *code_ptr;
    code = code + "# " + FlatBuffersGeneratedWarning() + "\n\n";
    std::string module = submodule_name;
    if (module.empty()) module = root_module_;
    code += "# module: " + module + "\n\n";
  }

  std::string GetDirname(const Definition &def) const {
    std::string d = path_;
    if (def.defined_namespace != NULL)
      d += GetCanonicalName(*def.defined_namespace);
    return d;
  }

  std::string GetFilename(const Definition &def) const {
    return ConCatPathFileName(GetDirname(def), NormalizedName(def)) +
           JuliaFileExtension;
  }

  std::string GetModule(const Definition &def) const {
    if (def.defined_namespace != NULL)
      return GetCanonicalName(*def.defined_namespace);
    return root_module_;
  }

  static std::string GetSubModule(const Definition &def) {
    if (def.defined_namespace != NULL)
      return LastNamespacePart(*def.defined_namespace);
    return "";
  }

  // Save out the generated code for a Julia module
  bool SaveModule(bool is_root, std::string full_module_name,
                  DepGraph *children) {
    std::string module_name = full_module_name;
    auto start = full_module_name.rfind(kPathSeparator);
    if (start != std::string::npos)
      module_name = full_module_name.substr(start + 1);
    std::string module_dir = ConCatPathFileName(path_, full_module_name);
    if (is_root) {
      module_dir = path_;
      module_name = root_module_;
    }
    std::string module_jl =
        ConCatPathFileName(module_dir, module_name) + JuliaFileExtension;
    std::string code = "";
    bool need_module_file = false;
    BeginFile(module_name, &code);
    code += "module " + module_name + "\n";

    // Include all the dependencies of this module in the right order
    std::vector<std::string> sorted_children = children->TopSort();
    for (auto it = sorted_children.rbegin(); it != sorted_children.rend();
         ++it) {
      std::string child = *it;
      bool is_module = false;
      std::string submodule_name =
          child.substr(child.rfind(kPathSeparator) + 1);

      // Don't include the root module from the root
      if (is_root && submodule_name == root_module_) continue;

      // Modules live in a subdirectory named after themselves
      if (module_table_.IsModule(child) &&
          !(is_root && submodule_name == root_module_)) {
        is_module = true;
        child = ConCatPathFileName(child, submodule_name);
      }
      std::string dir = is_root ? path_ : module_dir;
      std::string relname;
      if (is_root)
        relname = child;
      else if (is_module)
        relname = child.substr(child.find(kPathSeparator) + 1);
      else
        relname = child.substr(child.rfind(kPathSeparator) + 1);

      // If the file doesn't exist, don't include it
      // TODO: this doesn't allow types which reference each other,
      // but Julia doesn't support this yet anyway
      std::string toinclude = relname + JuliaFileExtension;
      std::string fullpath = ConCatPathFileName(dir, toinclude);
      if (!module_table_.IsFile(fullpath.c_str())) continue;
      code += Indent + "include(\"" + toinclude + "\")\n";
      need_module_file = true;
    }
    code += "end\n";

    // only write the module if it has some things to include
    if (!need_module_file) return true;

    EnsureDirExists(module_dir);
    if (!SaveFile(module_jl.c_str(), code, false)) return false;

    module_table_.AddFile(module_jl);
    return true;
  }

  // Canonical julia name of a namespace (Foo.Bar.Baz)
  std::string GetCanonicalName(const Namespace &ns) const {
    std::string name;
    for (size_t i = 0; i < ns.components.size(); i++) {
      if (i) name += kPathSeparator;
      name += std::string(ns.components[i]);
    }
    if (name.empty()) name = root_module_;
    return name;
  }

  // Add a dependency between two definitions
  void AddDependency(const Definition *parent, const Definition *child) {
    FLATBUFFERS_ASSERT(parent != NULL && child != NULL);
    std::string parent_name = NormalizedName(*parent);
    std::string module = parent_name;
    if (parent->defined_namespace != NULL)
      module = GetCanonicalName(*parent->defined_namespace);
    module_table_.AddDependency(module, parent_name, NormalizedName(*child));
  }

  // Add a definition as a dependency to its own module
  void AddToOwnModule(const Definition &def) {
    std::string m = GetModule(def);
    module_table_.AddDependency(m, m + kPathSeparator + NormalizedName(def), m);
  }

  // Save out the generated code for a Julia struct
  bool SaveType(const Definition &def, const std::string &declcode) {
    if (!declcode.length()) return true;
    std::string code = "";
    BeginFile(GetSubModule(def), &code);
    code += declcode;
    std::string filename = GetFilename(def);
    EnsureDirExists(GetDirname(def));
    if (!SaveFile(filename.c_str(), code, false)) return false;
    module_table_.AddFile(filename);
    AddToOwnModule(def);
    return true;
  }
};

const std::unordered_set<std::string> JuliaGenerator::keywords_{
  { "begin",      "while",    "if",       "for",    "try",    "return",
    "break",      "continue", "function", "macro",  "quote",  "let",
    "local",      "global",   "const",    "do",     "struct", "module",
    "baremodule", "using",    "import",   "export", "end",    "else",
    "catch",      "finally",  "true",     "false",  "Any" }
};

}  // namespace julia

bool GenerateJulia(const Parser &parser, const std::string &path,
                   const std::string &file_name) {
  julia::JuliaGenerator generator(parser, path, file_name);
  return generator.generate();
}

}  // namespace flatbuffers
