#ifndef FLATBUFFERS_INCLUDE_CODEGEN_IDL_NAMER_H_
#define FLATBUFFERS_INCLUDE_CODEGEN_IDL_NAMER_H_

#include <iostream>

#include "codegen/namer.h"
#include "flatbuffers/base.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/options.h"

inline flatbuffers::Case EffectiveCase(flatbuffers::Case c) {
  const bool preserve = flatbuffers::global_options.preserve_case;
  return preserve ? flatbuffers::Case::kKeep : c;
}

namespace flatbuffers {

// Provides Namer capabilities to types defined in the flatbuffers IDL.
class IdlNamer : public Namer {
 public:
  explicit IdlNamer(Config config, std::set<std::string> keywords)
      : Namer(config, std::move(keywords)) {}

  using Namer::Constant;
  using Namer::Directories;
  using Namer::Field;
  using Namer::File;
  using Namer::Function;
  using Namer::Method;
  using Namer::Namespace;
  using Namer::NamespacedType;
  using Namer::ObjectType;
  using Namer::Type;
  using Namer::Variable;
  using Namer::Variant;

  std::string Constant(const FieldDef &d) const {
    return Format(d.name, EffectiveCase(config_.constants));
  }

  // Types are always structs or enums so we can only expose these two
  // overloads.
  std::string Type(const StructDef &d) const {
    return Format(d.name, EffectiveCase(config_.types));
  }
  std::string Type(const EnumDef &d) const {
    return Format(d.name, EffectiveCase(config_.types));
  }

  std::string Function(const Definition &d) const {
    return Format(d.name, EffectiveCase(config_.functions));
  }
  std::string Function(const std::string &name) const override {
    return Format(name, EffectiveCase(config_.functions));
  }
  std::string Function(const std::string &prefix, const Definition &d) const {
    return Format(prefix + d.name, EffectiveCase(config_.functions));
  }

  std::string Field(const FieldDef &d) const {
    return Format(d.name, EffectiveCase(config_.fields));
  }
  std::string Field(const FieldDef &d, const std::string &suffix) const {
    return Format(d.name + "_" + suffix, EffectiveCase(config_.fields));
  }

  std::string Variable(const FieldDef &s) const { return Variable(s.name); }

  std::string Variable(const StructDef &s) const { return Variable(s.name); }

  std::string Variant(const EnumVal &v) const {
    return Format(v.name, EffectiveCase(config_.variants));
  }
  std::string EnumVariant(const EnumDef &e, const EnumVal &v) const {
    return Format(e.name, EffectiveCase(config_.types)) +
           config_.enum_variant_seperator +
           Format(v.name, EffectiveCase(config_.variants));
  }

  std::string ObjectType(const StructDef &d) const {
    return config_.object_prefix +
           Format(d.name, EffectiveCase(config_.types)) + config_.object_suffix;
  }
  std::string ObjectType(const EnumDef &d) const {
    return config_.object_prefix +
           Format(d.name, EffectiveCase(config_.types)) + config_.object_suffix;
  }

  std::string Method(const FieldDef &d) const {
    return Format(d.name, EffectiveCase(config_.methods));
  }
  std::string Method(const FieldDef &d, const std::string &suffix) const {
    return Format(d.name + "_" + suffix, EffectiveCase(config_.methods));
  }
  std::string Method(const std::string &prefix, const StructDef &d) const {
    return Format(prefix + "_" + d.name, EffectiveCase(config_.methods));
  }
  std::string Method(const std::string &prefix, const FieldDef &d) const {
    return Format(prefix + "_" + d.name, EffectiveCase(config_.methods));
  }
  std::string Method(const std::string &prefix, const FieldDef &d,
                     const std::string &suffix) const {
    return Format(prefix + "_" + d.name + "_" + suffix,
                  EffectiveCase(config_.methods));
  }
  std::string Method(const std::string &pre, const std::string &mid,
                     const std::string &suf) const override {
    return Format(pre + "_" + mid + "_" + suf, EffectiveCase(config_.methods));
  }

  std::string Method(const std::string &pre,
                     const std::string &suf) const override {
    return Format(pre + "_" + suf, EffectiveCase(config_.methods));
  }
  std::string Method(const std::string &pre, const char *suf) const {
    return Format(pre + "_" + suf, EffectiveCase(config_.methods));
  }

  std::string Namespace(const struct Namespace &ns) const {
    std::string result;
    for (auto &comp : ns.components) {
      if (!result.empty()) result += config_.namespace_seperator;
      result += Format(comp, EffectiveCase(config_.namespaces));
    }
    return result;
  }

  std::string NamespacedEnumVariant(const EnumDef &e, const EnumVal &v) const {
    return NamespacedString(e.defined_namespace, EnumVariant(e, v));
  }

  std::string NamespacedType(const Definition &d) const {
    return NamespacedString(d.defined_namespace,
                            Format(d.name, EffectiveCase(config_.types)));
  }

  std::string NamespacedObjectType(const Definition &d) const {
    return NamespacedString(d.defined_namespace,
                            config_.object_prefix +
                                Format(d.name, EffectiveCase(config_.types)) +
                                config_.object_suffix);
  }

  std::string Directories(const struct Namespace &ns,
                          SkipDir skips = SkipDir::None,
                          Case input_case = Case::kUpperCamel) const {
    return Directories(ns.components, skips, input_case);
  }

  // Legacy fields do not really follow the usual config and should be
  // considered for deprecation.

  std::string LegacyRustNativeVariant(const EnumVal &v) const {
    return ConvertCase(EscapeKeyword(v.name), Case::kUpperCamel);
  }

  std::string LegacyRustFieldOffsetName(const FieldDef &field) const {
    return "VT_" + ConvertCase(EscapeKeyword(field.name), Case::kAllUpper);
  }
  std::string LegacyRustUnionTypeOffsetName(const FieldDef &field) const {
    return "VT_" +
           ConvertCase(EscapeKeyword(field.name + "_type"), Case::kAllUpper);
  }

  std::string LegacySwiftVariant(const EnumVal &ev) const {
    auto name = ev.name;
    if (isupper(name.front())) {
      std::transform(name.begin(), name.end(), name.begin(), CharToLower);
    }
    return EscapeKeyword(ConvertCase(name, Case::kLowerCamel));
  }

  // Also used by Kotlin, lol.
  std::string LegacyJavaMethod2(const std::string &prefix, const StructDef &sd,
                                const std::string &suffix) const {
    return prefix + sd.name + suffix;
  }

  std::string LegacyKotlinVariant(EnumVal &ev) const {
    // Namer assumes the input case is snake case which is wrong...
    return ConvertCase(EscapeKeyword(ev.name), Case::kLowerCamel);
  }
  // Kotlin methods escapes keywords after case conversion but before
  // prefixing and suffixing.
  std::string LegacyKotlinMethod(const std::string &prefix, const FieldDef &d,
                                 const std::string &suffix) const {
    return prefix + ConvertCase(EscapeKeyword(d.name), Case::kUpperCamel) +
           suffix;
  }
  std::string LegacyKotlinMethod(const std::string &prefix, const StructDef &d,
                                 const std::string &suffix) const {
    return prefix + ConvertCase(EscapeKeyword(d.name), Case::kUpperCamel) +
           suffix;
  }

  // This is a mix of snake case and keep casing, when Ts should be using
  // lower camel case.
  std::string LegacyTsMutateMethod(const FieldDef &d) {
    return "mutate_" + d.name;
  }

  std::string LegacyRustUnionTypeMethod(const FieldDef &d) {
    // assert d is a union
    // d should convert case but not escape keywords due to historical reasons
    return ConvertCase(d.name, config_.fields, Case::kLowerCamel) + "_type";
  }

 private:
  std::string NamespacedString(const struct Namespace *ns,
                               const std::string &str) const {
    std::string ret;
    if (ns != nullptr) { ret += Namespace(ns->components); }
    if (!ret.empty()) ret += config_.namespace_seperator;
    return ret + str;
  }
};

// This is a temporary helper function for code generators to call until all
// flag-overriding logic into flatc.cpp
inline Namer::Config WithFlagOptions(const Namer::Config &input,
                                     const IDLOptions &opts,
                                     const std::string &path) {
  Namer::Config result = input;
  result.object_prefix = opts.object_prefix;
  result.object_suffix = opts.object_suffix;
  result.output_path = path;
  result.filename_suffix = opts.filename_suffix;
  return result;
}

}  // namespace flatbuffers

#endif  // FLATBUFFERS_INCLUDE_CODEGEN_IDL_NAMER_H_
