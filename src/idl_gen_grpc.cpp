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

#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4512)  // C4512: 'class' : assignment operator could
// not be generated
#endif

namespace flatbuffers {

namespace {

class GRPCGenerator : public flatbuffers::BaseGenerator {
 public:
  enum Language { kLanguageGo, kLanguageCpp, kLanguageJava };

  GRPCGenerator(const Parser &parser, const std::string &path,
                const std::string &file_name, Language language)
      : flatbuffers::BaseGenerator(parser, path, file_name, "",
                                   ((language == kLanguageCpp) ? "::" : ".")),
        language_(language) {}

  virtual ~GRPCGenerator() {}

  const Namespace *CurrentNameSpace() const {
    return parser_.current_namespace_;
  }

  using flatbuffers::BaseGenerator::WrapInNameSpace;

  const Parser &GetParser() const { return parser_; }

  const std::string &GetPath() const { return path_; }

  const std::string &GetFileName() const { return file_name_; }

  Language GetLanguage() const { return language_; }

 private:
  Language language_;
};
}  // namespace

class FlatBufMethod : public grpc_generator::Method {
 public:
  enum Streaming {
    kNone, kClient, kServer, kBiDi
  };

  FlatBufMethod(const GRPCGenerator &generator, const RPCCall *method)
      : generator_(generator), method_(method) {
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

  std::string GRPCType(const StructDef &sd) const {
    return "flatbuffers::grpc::Message<" + generator_.WrapInNameSpace(sd) + ">";
  }

  std::string get_input_type_name() const { return (*method_->request).name; }

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

  std::string input_type_name() const { return GRPCType(*method_->request); }

  std::string output_type_name() const { return GRPCType(*method_->response); }

  bool NoStreaming() const { return streaming_ == kNone; }

  bool ClientStreaming() const { return streaming_ == kClient; }

  bool ServerStreaming() const { return streaming_ == kServer; }

  bool BidiStreaming() const { return streaming_ == kBiDi; }

 private:
  const GRPCGenerator &generator_;
  const RPCCall *method_;
  Streaming streaming_;
};

class FlatBufService : public grpc_generator::Service {
 public:
  FlatBufService(const GRPCGenerator &generator, const ServiceDef *service)
      : generator_(generator), service_(service) {}

  grpc::string GetLeadingComments(const grpc::string) const { return ""; }

  grpc::string GetTrailingComments(const grpc::string) const { return ""; }

  std::vector<grpc::string> GetAllComments() const {
    return service_->doc_comment;
  }

  std::string name() const { return service_->name; }

  int method_count() const {
    return static_cast<int>(service_->calls.vec.size());
  };

  std::unique_ptr<const grpc_generator::Method> method(int i) const {
    return std::unique_ptr<const grpc_generator::Method>(
        new FlatBufMethod(generator_, service_->calls.vec[i]));
  };

 private:
  const GRPCGenerator &generator_;
  const ServiceDef *service_;
};

class FlatBufPrinter : public grpc_generator::Printer {
 public:
  FlatBufPrinter(std::string *str) : str_(str), escape_char_('$'), indent_(0) {}

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
    if (s == nullptr || std::strlen(s) == 0) { return; }
    // Add this string, but for each part separated by \n, add indentation.
    for (;;) {
      // Current indentation.
      str_->insert(str_->end(), indent_ * 2, ' ');
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

  void Indent() { indent_++; }

  void Outdent() {
    indent_--;
        FLATBUFFERS_ASSERT(indent_ >= 0);
  }

 private:
  std::string *str_;
  char escape_char_;
  int indent_;
};

class FlatBufFile : public grpc_generator::File {
 public:
  FlatBufFile(const GRPCGenerator &generator) : generator_(generator) {}

  FlatBufFile &operator=(const FlatBufFile &);

  grpc::string GetLeadingComments(const grpc::string) const { return ""; }

  grpc::string GetTrailingComments(const grpc::string) const { return ""; }

  std::vector<grpc::string> GetAllComments() const {
    return std::vector<grpc::string>();
  }

  std::string filename() const { return generator_.GetFileName(); }

  std::string filename_without_ext() const {
    return StripExtension(generator_.GetFileName());
  }

  std::string message_header_ext() const { return "_generated.h"; }

  std::string service_header_ext() const { return ".grpc.fb.h"; }

  std::string package() const {
    return generator_.GetParser().current_namespace_->GetFullyQualifiedName("");
  }

  std::vector<std::string> package_parts() const {
    return generator_.GetParser().current_namespace_->components;
  }

  std::string additional_headers() const {
    switch (generator_.GetLanguage()) {
      case GRPCGenerator::kLanguageCpp: {
        return "#include \"flatbuffers/grpc.h\"\n";
      }
      case GRPCGenerator::kLanguageGo: {
        return "import \"github.com/google/flatbuffers/go\"";
      }
      case GRPCGenerator::kLanguageJava: {
        return "import com.google.flatbuffers.grpc.FlatbuffersUtils;";
      }
    }
    return "";
  }

  int service_count() const {
    return static_cast<int>(generator_.GetParser().services_.vec.size());
  };

  std::unique_ptr<const grpc_generator::Service> service(int i) const {
    return std::unique_ptr<const grpc_generator::Service>(
        new FlatBufService(generator_, generator_.GetParser().services_.vec[i]));
  }

  std::unique_ptr<grpc_generator::Printer> CreatePrinter(
      std::string *str) const {
    return std::unique_ptr<grpc_generator::Printer>(new FlatBufPrinter(str));
  }

 private:
  const GRPCGenerator &generator_;
};

class GoGRPCGenerator : public GRPCGenerator {
 public:
  GoGRPCGenerator(const Parser &parser, const std::string &path,
                  const std::string &file_name)
      : GRPCGenerator(parser, path, file_name, kLanguageGo) {}

  bool generate() {
    FlatBufFile file(*this);
    grpc_go_generator::Parameters p;
    p.custom_method_io_type = "flatbuffers.Builder";
    for (int i = 0; i < file.service_count(); i++) {
      auto service = file.service(i);
      const Definition *def = GetParser().services_.vec[i];
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

class CppGRPCGenerator : public GRPCGenerator {
 public:
  CppGRPCGenerator(const Parser &parser, const std::string &path,
                   const std::string &file_name)
      : GRPCGenerator(parser, path, file_name, kLanguageCpp) {}

  bool generate() {
    grpc_cpp_generator::Parameters generator_parameters;
    // TODO(wvo): make the other parameters in this struct configurable.
    generator_parameters.use_system_headers = true;

    FlatBufFile fbfile(*this);

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

    return flatbuffers::SaveFile((GetPath() + GetFileName() + ".grpc.fb.h").c_str(),
                                 header_code, false) &&
           flatbuffers::SaveFile((GetPath() + GetFileName() + ".grpc.fb.cc").c_str(),
                                 source_code, false);
  }
};

bool GenerateCppGRPC(const Parser &parser, const std::string &path,
                     const std::string &file_name) {
  int nservices = 0;
  for (auto it = parser.services_.vec.begin(); it != parser.services_.vec.end();
       ++it) {
    if (!(*it)->generated) nservices++;
  }
  if (!nservices) return true;

  return CppGRPCGenerator(parser, path, file_name).generate();
}

class JavaGRPCGenerator : public GRPCGenerator {
 public:
  JavaGRPCGenerator(const Parser &parser, const std::string &path,
                    const std::string &file_name)
      : GRPCGenerator(parser, path, file_name, kLanguageJava) {}

  bool generate() {
    FlatBufFile file(*this);
    grpc_java_generator::Parameters p;
    for (int i = 0; i < file.service_count(); i++) {
      auto service = file.service(i);
      const Definition *def = GetParser().services_.vec[i];
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

}  // namespace flatbuffers

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif
