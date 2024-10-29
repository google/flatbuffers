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

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include "src/compiler/cpp_generator.h"
#include "src/compiler/go_generator.h"
#include "src/compiler/java_generator.h"
#include "src/compiler/python_generator.h"
#include "src/compiler/swift_generator.h"
#include "src/compiler/ts_generator.h"

#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4512)  // C4512: 'class' : assignment operator could
// not be generated
#endif

namespace flatbuffers {

class FlatBufMethod : public grpc_generator::Method {
 public:
  enum Streaming { kNone, kClient, kServer, kBiDi };

  FlatBufMethod(const RPCCall *method) : method_(method) {
    streaming_ = kNone;
    auto val = method_->attributes.Lookup("streaming");
    if (val) {
      if (val->constant == "client") streaming_ = kClient;
      if (val->constant == "server") streaming_ = kServer;
      if (val->constant == "bidi") streaming_ = kBiDi;
    }
  }

  grpc::string GetLeadingComments(const grpc::string) const { return ""; }

  grpc::string GetTrailingComments(const grpc::string) const { return ""; }

  std::vector<grpc::string> GetAllComments() const {
    return method_->doc_comment;
  }

  std::string name() const { return method_->name; }

  // TODO: This method need to incorporate namespace for C++ side. Other
  // language bindings simply don't use this method.
  std::string GRPCType(const StructDef &sd) const {
    return "flatbuffers::grpc::Message<" + sd.name + ">";
  }

  std::vector<std::string> get_input_namespace_parts() const {
    return (*method_->request).defined_namespace->components;
  }

  std::string get_input_type_name() const { return (*method_->request).name; }

  std::vector<std::string> get_output_namespace_parts() const {
    return (*method_->response).defined_namespace->components;
  }

  std::string get_output_type_name() const { return (*method_->response).name; }

  bool get_module_and_message_path_input(grpc::string * /*str*/,
                                         grpc::string /*generator_file_name*/,
                                         bool /*generate_in_pb2_grpc*/,
                                         grpc::string /*import_prefix*/) const {
    return true;
  }

  bool get_module_and_message_path_output(
      grpc::string * /*str*/, grpc::string /*generator_file_name*/,
      bool /*generate_in_pb2_grpc*/, grpc::string /*import_prefix*/) const {
    return true;
  }

  std::string get_fb_builder() const { return "builder"; }

  std::string input_type_name() const { return GRPCType(*method_->request); }

  std::string output_type_name() const { return GRPCType(*method_->response); }

  bool NoStreaming() const { return streaming_ == kNone; }

  bool ClientStreaming() const { return streaming_ == kClient; }

  bool ServerStreaming() const { return streaming_ == kServer; }

  bool BidiStreaming() const { return streaming_ == kBiDi; }

 private:
  const RPCCall *method_;
  Streaming streaming_;
};

class FlatBufService : public grpc_generator::Service {
 public:
  FlatBufService(const ServiceDef *service) : service_(service) {}

  grpc::string GetLeadingComments(const grpc::string) const { return ""; }

  grpc::string GetTrailingComments(const grpc::string) const { return ""; }

  std::vector<grpc::string> GetAllComments() const {
    return service_->doc_comment;
  }

  std::vector<grpc::string> namespace_parts() const {
    return service_->defined_namespace->components;
  }

  std::string name() const { return service_->name; }
  bool is_internal() const {
    return service_->Definition::attributes.Lookup("private") ? true : false;
  }

  int method_count() const {
    return static_cast<int>(service_->calls.vec.size());
  }

  std::unique_ptr<const grpc_generator::Method> method(int i) const {
    return std::unique_ptr<const grpc_generator::Method>(
        new FlatBufMethod(service_->calls.vec[i]));
  }

 private:
  const ServiceDef *service_;
};

class FlatBufPrinter : public grpc_generator::Printer {
 public:
  FlatBufPrinter(std::string *str, const char indentation_type)
      : str_(str),
        escape_char_('$'),
        indent_(0),
        indentation_size_(2),
        indentation_type_(indentation_type) {}

  void Print(const std::map<std::string, std::string> &vars,
             const char *string_template) {
    std::string s = string_template;
    // Replace any occurrences of strings in "vars" that are surrounded
    // by the escape character by what they're mapped to.
    size_t pos;
    while ((pos = s.find(escape_char_)) != std::string::npos) {
      // Found an escape char, must also find the closing one.
      size_t pos2 = s.find(escape_char_, pos + 1);
      // If placeholder not closed, ignore.
      if (pos2 == std::string::npos) break;
      auto it = vars.find(s.substr(pos + 1, pos2 - pos - 1));
      // If unknown placeholder, ignore.
      if (it == vars.end()) break;
      // Subtitute placeholder.
      s.replace(pos, pos2 - pos + 1, it->second);
    }
    Print(s.c_str());
  }

  void Print(const char *s) {
    if (s == nullptr || *s == '\0') { return; }
    // Add this string, but for each part separated by \n, add indentation.
    for (;;) {
      // Current indentation.
      str_->insert(str_->end(), indent_ * indentation_size_, indentation_type_);
      // See if this contains more than one line.
      const char *lf = strchr(s, '\n');
      if (lf) {
        (*str_) += std::string(s, lf + 1);
        s = lf + 1;
        if (!*s) break;  // Only continue if there's more lines.
      } else {
        (*str_) += s;
        break;
      }
    }
  }

  void SetIndentationSize(const size_t size) {
    FLATBUFFERS_ASSERT(str_->empty());
    indentation_size_ = size;
  }

  void Indent() { indent_++; }

  void Outdent() {
    FLATBUFFERS_ASSERT(indent_ > 0);
    indent_--;
  }

 private:
  std::string *str_;
  char escape_char_;
  size_t indent_;
  size_t indentation_size_;
  char indentation_type_;
};

class FlatBufFile : public grpc_generator::File {
 public:
  enum Language {
    kLanguageGo,
    kLanguageCpp,
    kLanguageJava,
    kLanguagePython,
    kLanguageSwift,
    kLanguageTS
  };

  FlatBufFile(const Parser &parser, const std::string &file_name,
              Language language)
      : parser_(parser), file_name_(file_name), language_(language) {}

  FlatBufFile &operator=(const FlatBufFile &);

  grpc::string GetLeadingComments(const grpc::string) const { return ""; }

  grpc::string GetTrailingComments(const grpc::string) const { return ""; }

  std::vector<grpc::string> GetAllComments() const {
    return std::vector<grpc::string>();
  }

  std::string filename() const { return file_name_; }

  std::string filename_without_ext() const {
    return StripExtension(file_name_);
  }

  std::string message_header_ext() const {
    return parser_.opts.filename_suffix + ".h";
  }

  std::string service_header_ext() const { return ".grpc.fb.h"; }

  std::string package() const {
    return parser_.current_namespace_->GetFullyQualifiedName("");
  }

  std::vector<std::string> package_parts() const {
    return parser_.current_namespace_->components;
  }

  std::string additional_headers() const {
    switch (language_) {
      case kLanguageCpp: {
        return "#include \"flatbuffers/grpc.h\"\n";
      }
      case kLanguageGo: {
        return "import \"github.com/google/flatbuffers/go\"";
      }
      case kLanguageJava: {
        return "import com.google.flatbuffers.grpc.FlatbuffersUtils;";
      }
      case kLanguagePython: {
        return "";
      }
      case kLanguageSwift: {
        return "";
      }
      case kLanguageTS: {
        return "";
      }
    }
    return "";
  }

  int service_count() const {
    return static_cast<int>(parser_.services_.vec.size());
  }

  std::unique_ptr<const grpc_generator::Service> service(int i) const {
    return std::unique_ptr<const grpc_generator::Service>(
        new FlatBufService(parser_.services_.vec[i]));
  }

  std::unique_ptr<grpc_generator::Printer> CreatePrinter(
      std::string *str, const char indentation_type = ' ') const {
    return std::unique_ptr<grpc_generator::Printer>(
        new FlatBufPrinter(str, indentation_type));
  }

 private:
  const Parser &parser_;
  const std::string &file_name_;
  const Language language_;
};

class GoGRPCGenerator : public flatbuffers::BaseGenerator {
 public:
  GoGRPCGenerator(const Parser &parser, const std::string &path,
                  const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", "" /*Unused*/, "go"),
        parser_(parser),
        path_(path),
        file_name_(file_name) {}

  bool generate() {
    FlatBufFile file(parser_, file_name_, FlatBufFile::kLanguageGo);
    grpc_go_generator::Parameters p;
    p.custom_method_io_type = "flatbuffers.Builder";
    for (int i = 0; i < file.service_count(); i++) {
      auto service = file.service(i);
      const Definition *def = parser_.services_.vec[i];
      p.package_name = LastNamespacePart(*(def->defined_namespace));
      p.service_prefix =
          def->defined_namespace->GetFullyQualifiedName("");  // file.package();
      std::string output =
          grpc_go_generator::GenerateServiceSource(&file, service.get(), &p);
      std::string filename =
          NamespaceDir(*def->defined_namespace) + def->name + "_grpc.go";
      if (!flatbuffers::SaveFile(filename.c_str(), output, false)) return false;
    }
    return true;
  }

 protected:
  const Parser &parser_;
  const std::string &path_, &file_name_;
};

bool GenerateGoGRPC(const Parser &parser, const std::string &path,
                    const std::string &file_name) {
  int nservices = 0;
  for (auto it = parser.services_.vec.begin(); it != parser.services_.vec.end();
       ++it) {
    if (!(*it)->generated) nservices++;
  }
  if (!nservices) return true;
  return GoGRPCGenerator(parser, path, file_name).generate();
}

bool GenerateCppGRPC(const Parser &parser, const std::string &path,
                     const std::string &file_name) {
  int nservices = 0;
  for (auto it = parser.services_.vec.begin(); it != parser.services_.vec.end();
       ++it) {
    if (!(*it)->generated) nservices++;
  }
  if (!nservices) return true;

  grpc_cpp_generator::Parameters generator_parameters;
  // TODO(wvo): make the other parameters in this struct configurable.
  generator_parameters.use_system_headers = true;

  FlatBufFile fbfile(parser, file_name, FlatBufFile::kLanguageCpp);

  std::string header_code =
      grpc_cpp_generator::GetHeaderPrologue(&fbfile, generator_parameters) +
      grpc_cpp_generator::GetHeaderIncludes(&fbfile, generator_parameters) +
      grpc_cpp_generator::GetHeaderServices(&fbfile, generator_parameters) +
      grpc_cpp_generator::GetHeaderEpilogue(&fbfile, generator_parameters);

  std::string source_code =
      grpc_cpp_generator::GetSourcePrologue(&fbfile, generator_parameters) +
      grpc_cpp_generator::GetSourceIncludes(&fbfile, generator_parameters) +
      grpc_cpp_generator::GetSourceServices(&fbfile, generator_parameters) +
      grpc_cpp_generator::GetSourceEpilogue(&fbfile, generator_parameters);

  return flatbuffers::SaveFile((path + file_name + ".grpc.fb.h").c_str(),
                               header_code, false) &&
         flatbuffers::SaveFile((path + file_name + ".grpc.fb.cc").c_str(),
                               source_code, false);
}

class JavaGRPCGenerator : public flatbuffers::BaseGenerator {
 public:
  JavaGRPCGenerator(const Parser &parser, const std::string &path,
                    const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", "." /*separator*/, "java") {}

  bool generate() {
    FlatBufFile file(parser_, file_name_, FlatBufFile::kLanguageJava);
    grpc_java_generator::Parameters p;
    for (int i = 0; i < file.service_count(); i++) {
      auto service = file.service(i);
      const Definition *def = parser_.services_.vec[i];
      p.package_name =
          def->defined_namespace->GetFullyQualifiedName("");  // file.package();
      std::string output =
          grpc_java_generator::GenerateServiceSource(&file, service.get(), &p);
      std::string filename =
          NamespaceDir(*def->defined_namespace) + def->name + "Grpc.java";
      if (!flatbuffers::SaveFile(filename.c_str(), output, false)) return false;
    }
    return true;
  }
};

bool GenerateJavaGRPC(const Parser &parser, const std::string &path,
                      const std::string &file_name) {
  int nservices = 0;
  for (auto it = parser.services_.vec.begin(); it != parser.services_.vec.end();
       ++it) {
    if (!(*it)->generated) nservices++;
  }
  if (!nservices) return true;
  return JavaGRPCGenerator(parser, path, file_name).generate();
}

class PythonGRPCGenerator : public flatbuffers::BaseGenerator {
 private:
  CodeWriter code_;

 public:
  PythonGRPCGenerator(const Parser &parser, const std::string &filename)
      : BaseGenerator(parser, "", filename, "", "" /*Unused*/, "swift") {}

  bool generate() {
    code_.Clear();
    code_ +=
        "# Generated by the gRPC Python protocol compiler plugin. "
        "DO NOT EDIT!\n";
    code_ += "import grpc\n";

    FlatBufFile file(parser_, file_name_, FlatBufFile::kLanguagePython);

    for (int i = 0; i < file.service_count(); i++) {
      auto service = file.service(i);
      code_ += grpc_python_generator::Generate(&file, service.get());
    }
    const auto final_code = code_.ToString();
    const auto filename = GenerateFileName();
    return SaveFile(filename.c_str(), final_code, false);
  }

  std::string GenerateFileName() {
    std::string namespace_dir;
    auto &namespaces = parser_.namespaces_.back()->components;
    for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
      if (it != namespaces.begin()) namespace_dir += kPathSeparator;
      namespace_dir += *it;
    }
    std::string grpc_py_filename = namespace_dir;
    if (!namespace_dir.empty()) grpc_py_filename += kPathSeparator;
    return grpc_py_filename + file_name_ + "_grpc_fb.py";
  }
};

bool GeneratePythonGRPC(const Parser &parser, const std::string & /*path*/,
                        const std::string &file_name) {
  int nservices = 0;
  for (auto it = parser.services_.vec.begin(); it != parser.services_.vec.end();
       ++it) {
    if (!(*it)->generated) nservices++;
  }
  if (!nservices) return true;

  return PythonGRPCGenerator(parser, file_name).generate();
}

class SwiftGRPCGenerator : public flatbuffers::BaseGenerator {
 private:
  CodeWriter code_;

 public:
  SwiftGRPCGenerator(const Parser &parser, const std::string &path,
                     const std::string &filename)
      : BaseGenerator(parser, path, filename, "", "" /*Unused*/, "swift") {}

  bool generate() {
    code_.Clear();
    code_ += "// Generated GRPC code for FlatBuffers swift!";
    code_ += grpc_swift_generator::GenerateHeader();
    FlatBufFile file(parser_, file_name_, FlatBufFile::kLanguageSwift);
    for (int i = 0; i < file.service_count(); i++) {
      auto service = file.service(i);
      code_ += grpc_swift_generator::Generate(&file, service.get());
    }
    const auto final_code = code_.ToString();
    const auto filename = GeneratedFileName(path_, file_name_);
    return SaveFile(filename.c_str(), final_code, false);
  }

  static std::string GeneratedFileName(const std::string &path,
                                       const std::string &file_name) {
    return path + file_name + ".grpc.swift";
  }
};

bool GenerateSwiftGRPC(const Parser &parser, const std::string &path,
                       const std::string &file_name) {
  int nservices = 0;
  for (auto it = parser.services_.vec.begin(); it != parser.services_.vec.end();
       ++it) {
    if (!(*it)->generated) nservices++;
  }
  if (!nservices) return true;
  return SwiftGRPCGenerator(parser, path, file_name).generate();
}

class TSGRPCGenerator : public flatbuffers::BaseGenerator {
 private:
  CodeWriter code_;

 public:
  TSGRPCGenerator(const Parser &parser, const std::string &path,
                  const std::string &filename)
      : BaseGenerator(parser, path, filename, "", "" /*Unused*/, "ts") {}

  bool generate() {
    code_.Clear();
    FlatBufFile file(parser_, file_name_, FlatBufFile::kLanguageTS);

    for (int i = 0; i < file.service_count(); i++) {
      auto service = file.service(i);
      code_ += grpc_ts_generator::Generate(&file, service.get(), file_name_);
      const auto ts_name = GeneratedFileName(path_, file_name_);
      if (!SaveFile(ts_name.c_str(), code_.ToString(), false)) return false;

      code_.Clear();
      code_ += grpc_ts_generator::GenerateInterface(&file, service.get(),
                                                    file_name_);
      const auto ts_interface_name = GeneratedFileName(path_, file_name_, true);
      if (!SaveFile(ts_interface_name.c_str(), code_.ToString(), false))
        return false;
    }
    return true;
  }

  static std::string GeneratedFileName(const std::string &path,
                                       const std::string &file_name,
                                       const bool is_interface = false) {
    if (is_interface) return path + file_name + "_grpc.d.ts";
    return path + file_name + "_grpc.js";
  }
};

bool GenerateTSGRPC(const Parser &parser, const std::string &path,
                    const std::string &file_name) {
  int nservices = 0;
  for (auto it = parser.services_.vec.begin(); it != parser.services_.vec.end();
       ++it) {
    if (!(*it)->generated) nservices++;
  }
  if (!nservices) return true;
  return TSGRPCGenerator(parser, path, file_name).generate();
}

}  // namespace flatbuffers

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif
