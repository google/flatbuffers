/*
 *
 * Copyright 2015 gRPC authors.
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
 *
 */

#include "src/compiler/python_generator.h"

#include <algorithm>
#include <cstddef>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include "codegen/idl_namer.h"
#include "codegen/namer.h"
#include "codegen/python.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {
namespace python {
namespace grpc {
namespace {
bool ClientStreaming(const RPCCall *method) {
  const Value *val = method->attributes.Lookup("streaming");
  return val != nullptr &&
         (val->constant == "client" || val->constant == "bidi");
}

bool ServerStreaming(const RPCCall *method) {
  const Value *val = method->attributes.Lookup("streaming");
  return val != nullptr &&
         (val->constant == "server" || val->constant == "bidi");
}

void FormatImports(std::stringstream &ss, const Imports &imports) {
  std::set<std::string> modules;
  std::map<std::string, std::set<std::string>> names_by_module;
  for (const Import &import : imports.imports) {
    if (import.IsLocal()) continue;  // skip all local imports
    if (import.name == "") {
      modules.insert(import.module);
    } else {
      names_by_module[import.module].insert(import.name);
    }
  }

  for (const std::string &module : modules) {
    ss << "import " << module << '\n';
  }
  ss << '\n';
  for (const auto &import : names_by_module) {
    ss << "from " << import.first << " import ";
    size_t i = 0;
    for (const std::string &name : import.second) {
      if (i > 0) ss << ", ";
      ss << name;
      ++i;
    }
    ss << '\n';
  }
  ss << "\n\n";
}

bool SaveStub(const std::string &filename, const Imports &imports,
              const std::string &content) {
  std::stringstream ss;
  ss << "# Generated by the gRPC FlatBuffers compiler. DO NOT EDIT!\n"
     << '\n'
     << "from __future__ import annotations\n"
     << '\n';
  FormatImports(ss, imports);
  ss << content << '\n';

  EnsureDirExists(StripFileName(filename));
  return flatbuffers::SaveFile(filename.c_str(), ss.str(), false);
}

bool SaveService(const std::string &filename, const Imports &imports,
                 const std::string &content) {
  std::stringstream ss;
  ss << "# Generated by the gRPC FlatBuffers compiler. DO NOT EDIT!\n" << '\n';
  FormatImports(ss, imports);
  ss << content << '\n';

  EnsureDirExists(StripFileName(filename));
  return flatbuffers::SaveFile(filename.c_str(), ss.str(), false);
}

class BaseGenerator {
 protected:
  BaseGenerator(const Parser &parser, const Namer::Config &config,
                const std::string &path, const Version &version)
      : parser_{ parser },
        namer_{ WithFlagOptions(config, parser.opts, path), Keywords(version) },
        version_{ version },
        path_(path) {}

 protected:
  std::string ModuleForFile(const std::string &file) const {
    std::string module = parser_.opts.include_prefix + StripExtension(file) +
                         parser_.opts.filename_suffix;
    std::replace(module.begin(), module.end(), '/', '.');
    return module;
  }

  template<typename T> std::string ModuleFor(const T *def) const {
    if (parser_.opts.one_file) return ModuleForFile(def->file);
    return namer_.NamespacedType(*def);
  }

  std::string NamespaceDir(const Parser &parser, const std::string &path,
                           const Namespace &ns, const bool dasherize) {
    EnsureDirExists(path);
    if (parser.opts.one_file) return path;
    std::string namespace_dir = path;  // Either empty or ends in separator.
    auto &namespaces = ns.components;
    for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
      namespace_dir +=
          !dasherize ? *it : ConvertCase(*it, Case::kDasher, Case::kUpperCamel);
      namespace_dir += kPathSeparator;
      EnsureDirExists(namespace_dir);
    }
    return namespace_dir;
  }

  std::string NamespaceDir(const Namespace &ns, const bool dasherize) {
    return NamespaceDir(parser_, path_, ns, dasherize);
  }

  const Parser &parser_;
  const IdlNamer namer_;
  const Version version_;
  const std::string &path_;
};

class StubGenerator : public BaseGenerator {
 public:
  StubGenerator(const Parser &parser, const std::string &path,
                const Version &version)
      : BaseGenerator(parser, kStubConfig, path, version) {}

  bool Generate() {
    Imports imports;
    std::stringstream stub;
    std::string ns_name{};
    for (const ServiceDef *service : parser_.services_.vec) {
      Generate(stub, service, &imports);
      ns_name = NamespaceDir(*service->defined_namespace, false);
    }

    std::string sanitized_suffix{ parser_.opts.grpc_filename_suffix };
    std::replace(sanitized_suffix.begin(), sanitized_suffix.end(), '.', '_');
    std::string filename =
        ns_name + kPathSeparator +
        StripPath(StripExtension(parser_.file_being_parsed_)) + "_grpc" +
        sanitized_suffix + namer_.config_.filename_extension;

    return SaveStub(filename, imports, stub.str());
  }

 private:
  void Generate(std::stringstream &ss, const ServiceDef *service,
                Imports *imports) {
    imports->Import("grpc");

    ss << "class " << service->name << "Stub(object):\n"
       << "  def __init__(self, channel: grpc.Channel) -> None: ...\n";

    for (const RPCCall *method : service->calls.vec) {
      std::string request = "bytes";
      std::string response = "bytes";

      if (parser_.opts.grpc_python_typed_handlers) {
        request = namer_.Type(*method->request);
        response = namer_.Type(*method->response);

        imports->Import(ModuleFor(method->request), request);
        imports->Import(ModuleFor(method->response), response);
      }

      ss << "  def " << method->name << "(self, ";
      if (ClientStreaming(method)) {
        imports->Import("typing");
        ss << "request_iterator: typing.Iterator[" << request << "]";
      } else {
        ss << "request: " << request;
      }
      ss << ") -> ";
      if (ServerStreaming(method)) {
        imports->Import("typing");
        ss << "typing.Iterator[" << response << "]";
      } else {
        ss << response;
      }
      ss << ": ...\n";
    }

    ss << "\n\n";
    ss << "class " << service->name << "Servicer(object):\n";

    for (const RPCCall *method : service->calls.vec) {
      std::string request = "bytes";
      std::string response = "bytes";

      if (parser_.opts.grpc_python_typed_handlers) {
        request = namer_.Type(*method->request);
        response = namer_.Type(*method->response);

        imports->Import(ModuleFor(method->request), request);
        imports->Import(ModuleFor(method->response), response);
      }

      ss << "  def " << method->name << "(self, ";
      if (ClientStreaming(method)) {
        imports->Import("typing");
        ss << "request_iterator: typing.Iterator[" << request << "]";
      } else {
        ss << "request: " << request;
      }
      ss << ", context: grpc.ServicerContext) -> ";
      if (ServerStreaming(method)) {
        imports->Import("typing");
        ss << "typing.Iterator[" << response << "]";
      } else {
        ss << response;
      }
      ss << ": ...\n";
    }

    ss << '\n'
       << '\n'
       << "def add_" << service->name
       << "Servicer_to_server(servicer: " << service->name
       << "Servicer, server: grpc.Server) -> None: ...\n";
  }
};

class ServiceGenerator : public BaseGenerator {
 public:
  ServiceGenerator(const Parser &parser, const std::string &path,
                   const Version &version)
      : BaseGenerator(parser, kConfig, path, version) {}

  bool Generate() {
    Imports imports;
    std::stringstream ss;

    imports.Import("flatbuffers");

    if (parser_.opts.grpc_python_typed_handlers) {
      ss << "def _serialize_to_bytes(table):\n"
         << "  buf = table._tab.Bytes\n"
         << "  n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, 0)\n"
         << "  if table._tab.Pos != n:\n"
         << "    raise ValueError('must be a top-level table')\n"
         << "  return bytes(buf)\n"
         << '\n'
         << '\n';
    }

    std::string ns_name{};
    for (const ServiceDef *service : parser_.services_.vec) {
      GenerateStub(ss, service, &imports);
      GenerateServicer(ss, service, &imports);
      GenerateRegister(ss, service, &imports);
      ns_name = NamespaceDir(*service->defined_namespace, false);
    }

    std::string sanitized_suffix{ parser_.opts.grpc_filename_suffix };
    std::replace(sanitized_suffix.begin(), sanitized_suffix.end(), '.', '_');
    std::string filename =
        ns_name + kPathSeparator +
        StripPath(StripExtension(parser_.file_being_parsed_)) + "_grpc" +
        sanitized_suffix + namer_.config_.filename_extension;

    return SaveService(filename, imports, ss.str());
  }

 private:
  void GenerateStub(std::stringstream &ss, const ServiceDef *service,
                    Imports *imports) {
    ss << "class " << service->name << "Stub";
    if (version_.major != 3) ss << "(object)";
    ss << ":\n"
       << "  '''Interface exported by the server.'''\n"
       << '\n'
       << "  def __init__(self, channel):\n"
       << "    '''Constructor.\n"
       << '\n'
       << "    Args:\n"
       << "      channel: A grpc.Channel.\n"
       << "    '''\n"
       << '\n';

    for (const RPCCall *method : service->calls.vec) {
      std::string response = namer_.Type(*method->response);

      imports->Import(ModuleFor(method->response), response);

      ss << "    self." << method->name << " = channel."
         << (ClientStreaming(method) ? "stream" : "unary") << "_"
         << (ServerStreaming(method) ? "stream" : "unary") << "(\n"
         << "      method='/"
         << service->defined_namespace->GetFullyQualifiedName(service->name)
         << "/" << method->name << "'";

      if (parser_.opts.grpc_python_typed_handlers) {
        ss << ",\n"
           << "      request_serializer=_serialize_to_bytes,\n"
           << "      response_deserializer=" << response << ".GetRootAs";
      }
      ss << ")\n\n";
    }

    ss << '\n';
  }

  void GenerateServicer(std::stringstream &ss, const ServiceDef *service,
                        Imports *imports) {
    imports->Import("grpc");

    ss << "class " << service->name << "Servicer";
    if (version_.major != 3) ss << "(object)";
    ss << ":\n"
       << "  '''Interface exported by the server.'''\n"
       << '\n';

    for (const RPCCall *method : service->calls.vec) {
      const std::string request_param =
          ClientStreaming(method) ? "request_iterator" : "request";
      ss << "  def " << method->name << "(self, " << request_param
         << ", context):\n"
         << "    context.set_code(grpc.StatusCode.UNIMPLEMENTED)\n"
         << "    context.set_details('Method not implemented!')\n"
         << "    raise NotImplementedError('Method not implemented!')\n"
         << '\n';
    }

    ss << '\n';
  }

  void GenerateRegister(std::stringstream &ss, const ServiceDef *service,
                        Imports *imports) {
    imports->Import("grpc");

    ss << "def add_" << service->name
       << "Servicer_to_server(servicer, server):\n"
       << "  rpc_method_handlers = {\n";

    for (const RPCCall *method : service->calls.vec) {
      std::string request = namer_.Type(*method->request);

      imports->Import(ModuleFor(method->request), request);

      ss << "    '" << method->name << "': grpc."
         << (ClientStreaming(method) ? "stream" : "unary") << "_"
         << (ServerStreaming(method) ? "stream" : "unary")
         << "_rpc_method_handler(\n"
         << "      servicer." << method->name;

      if (parser_.opts.grpc_python_typed_handlers) {
        ss << ",\n"
           << "      request_deserializer=" << request << ".GetRootAs,\n"
           << "      response_serializer=_serialize_to_bytes";
      }
      ss << "),\n";
    }
    ss << "  }\n"
       << '\n'
       << "  generic_handler = grpc.method_handlers_generic_handler(\n"
       << "    '"
       << service->defined_namespace->GetFullyQualifiedName(service->name)
       << "', rpc_method_handlers)\n"
       << '\n'
       << "  server.add_generic_rpc_handlers((generic_handler,))\n"
       << '\n';
  }
};
}  // namespace

bool Generate(const Parser &parser, const std::string &path,
              const Version &version) {
  ServiceGenerator generator{ parser, path, version };
  return generator.Generate();
}

bool GenerateStub(const Parser &parser, const std::string &path,
                  const Version &version) {
  StubGenerator generator{ parser, path, version };
  return generator.Generate();
}

}  // namespace grpc
}  // namespace python
}  // namespace flatbuffers
