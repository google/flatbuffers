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

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <tuple>
#include <vector>

#include "flatbuffers/util.h"
#include "src/compiler/python_generator.h"
#include "src/compiler/python_private_generator.h"

using std::make_pair;
using std::map;
using std::pair;
using std::replace;
using std::tuple;
using std::vector;
using std::set;

namespace grpc_python_generator {

grpc::string generator_file_name;

typedef map<grpc::string, grpc::string> StringMap;
typedef vector<grpc::string> StringVector;
typedef tuple<grpc::string, grpc::string> StringPair;
typedef set<StringPair> StringPairSet;

// Provides RAII indentation handling. Use as:
// {
//   IndentScope raii_my_indent_var_name_here(my_py_printer);
//   // constructor indented my_py_printer
//   ...
//   // destructor called at end of scope, un-indenting my_py_printer
// }
class IndentScope {
 public:
  explicit IndentScope(grpc_generator::Printer* printer) : printer_(printer) {
    printer_->Indent();
  }

  ~IndentScope() { printer_->Outdent(); }

 private:
  grpc_generator::Printer* printer_;
};

inline grpc::string StringReplace(grpc::string str, const grpc::string& from,
                                  const grpc::string& to, bool replace_all) {
  size_t pos = 0;

  do {
    pos = str.find(from, pos);
    if (pos == grpc::string::npos) {
      break;
    }
    str.replace(pos, from.length(), to);
    pos += to.length();
  } while (replace_all);

  return str;
}

inline grpc::string StringReplace(grpc::string str, const grpc::string& from,
                                  const grpc::string& to) {
  return StringReplace(str, from, to, true);
}

grpc::string ModuleName(const grpc::string& filename,
                        const grpc::string& import_prefix) {
  grpc::string basename = flatbuffers::StripExtension(filename);
  basename = StringReplace(basename, "-", "_");
  basename = StringReplace(basename, "/", ".");
  return import_prefix + basename + "_fb";
}

grpc::string ModuleAlias(const grpc::string& filename,
                         const grpc::string& import_prefix) {
  grpc::string module_name = ModuleName(filename, import_prefix);
  // We can't have dots in the module name, so we replace each with _dot_.
  // But that could lead to a collision between a.b and a_dot_b, so we also
  // duplicate each underscore.
  module_name = StringReplace(module_name, "_", "__");
  module_name = StringReplace(module_name, ".", "_dot_");
  return module_name;
}

PrivateGenerator::PrivateGenerator(const GeneratorConfiguration& config_,
                                   const grpc_generator::File* file_)
    : config(config_), file(file_) {}

void PrivateGenerator::PrintBetaServicer(const grpc_generator::Service* service,
                                         grpc_generator::Printer* out) {
  StringMap service_dict;
  service_dict["Service"] = service->name();
  out->Print("\n\n");
  out->Print(service_dict, "class Beta$Service$Servicer(object):\n");
  {
    IndentScope raii_class_indent(out);
    out->Print(
        "\"\"\"The Beta API is deprecated for 0.15.0 and later.\n"
        "\nIt is recommended to use the GA API (classes and functions in this\n"
        "file not marked beta) for all further purposes. This class was "
        "generated\n"
        "only to ease transition from grpcio<0.15.0 to "
        "grpcio>=0.15.0.\"\"\"\n");
    for (int i = 0; i < service->method_count(); ++i) {
      auto method = service->method(i);
      grpc::string arg_name =
          method->ClientStreaming() ? "request_iterator" : "request";
      StringMap method_dict;
      method_dict["Method"] = method->name();
      method_dict["ArgName"] = arg_name;
      out->Print(method_dict, "def $Method$(self, $ArgName$, context):\n");
      {
        IndentScope raii_method_indent(out);
        out->Print("context.code(beta_interfaces.StatusCode.UNIMPLEMENTED)\n");
      }
    }
  }
}

void PrivateGenerator::PrintBetaStub(const grpc_generator::Service* service,
                                     grpc_generator::Printer* out) {
  StringMap service_dict;
  service_dict["Service"] = service->name();
  out->Print("\n\n");
  out->Print(service_dict, "class Beta$Service$Stub(object):\n");
  {
    IndentScope raii_class_indent(out);
    out->Print(
        "\"\"\"The Beta API is deprecated for 0.15.0 and later.\n"
        "\nIt is recommended to use the GA API (classes and functions in this\n"
        "file not marked beta) for all further purposes. This class was "
        "generated\n"
        "only to ease transition from grpcio<0.15.0 to "
        "grpcio>=0.15.0.\"\"\"\n");
    for (int i = 0; i < service->method_count(); ++i) {
      auto method = service->method(i);
      grpc::string arg_name =
          method->ClientStreaming() ? "request_iterator" : "request";
      StringMap method_dict;
      method_dict["Method"] = method->name();
      method_dict["ArgName"] = arg_name;
      out->Print(method_dict,
                 "def $Method$(self, $ArgName$, timeout, metadata=None, "
                 "with_call=False, protocol_options=None):\n");
      {
        IndentScope raii_method_indent(out);
        out->Print("raise NotImplementedError()\n");
      }
      if (!method->ServerStreaming()) {
        out->Print(method_dict, "$Method$.future = None\n");
      }
    }
  }
}

void PrivateGenerator::PrintBetaServerFactory(
    const grpc::string& package_qualified_service_name,
    const grpc_generator::Service* service, grpc_generator::Printer* out) {
  StringMap service_dict;
  service_dict["Service"] = service->name();
  out->Print("\n\n");
  out->Print(service_dict,
             "def beta_create_$Service$_server(servicer, pool=None, "
             "pool_size=None, default_timeout=None, maximum_timeout=None):\n");
  {
    IndentScope raii_create_server_indent(out);
    out->Print(
        "\"\"\"The Beta API is deprecated for 0.15.0 and later.\n"
        "\nIt is recommended to use the GA API (classes and functions in this\n"
        "file not marked beta) for all further purposes. This function was\n"
        "generated only to ease transition from grpcio<0.15.0 to grpcio>=0.15.0"
        "\"\"\"\n");
    StringMap method_implementation_constructors;
    StringMap input_message_modules_and_classes;
    StringMap output_message_modules_and_classes;
    for (int i = 0; i < service->method_count(); ++i) {
      auto method = service->method(i);
      const grpc::string method_implementation_constructor =
          grpc::string(method->ClientStreaming() ? "stream_" : "unary_") +
          grpc::string(method->ServerStreaming() ? "stream_" : "unary_") +
          "inline";
      grpc::string input_message_module_and_class = method->get_fb_builder();
      grpc::string output_message_module_and_class = method->get_fb_builder();
      method_implementation_constructors.insert(
          make_pair(method->name(), method_implementation_constructor));
      input_message_modules_and_classes.insert(
          make_pair(method->name(), input_message_module_and_class));
      output_message_modules_and_classes.insert(
          make_pair(method->name(), output_message_module_and_class));
    }
    StringMap method_dict;
    method_dict["PackageQualifiedServiceName"] = package_qualified_service_name;
//    out->Print("request_deserializers = {\n");
//    for (StringMap::iterator name_and_input_module_class_pair =
//             input_message_modules_and_classes.begin();
//         name_and_input_module_class_pair !=
//         input_message_modules_and_classes.end();
//         name_and_input_module_class_pair++) {
//      method_dict["MethodName"] = name_and_input_module_class_pair->first;
//      method_dict["InputTypeModuleAndClass"] =
//          name_and_input_module_class_pair->second;
//      IndentScope raii_indent(out);
//      out->Print(method_dict,
//                 "(\'$PackageQualifiedServiceName$\', \'$MethodName$\'): "
//                 "$InputTypeModuleAndClass$.FromString,\n");
//    }
//    out->Print("}\n");
//    out->Print("response_serializers = {\n");
//    for (StringMap::iterator name_and_output_module_class_pair =
//             output_message_modules_and_classes.begin();
//         name_and_output_module_class_pair !=
//         output_message_modules_and_classes.end();
//         name_and_output_module_class_pair++) {
//      method_dict["MethodName"] = name_and_output_module_class_pair->first;
//      method_dict["OutputTypeModuleAndClass"] =
//          name_and_output_module_class_pair->second;
//      IndentScope raii_indent(out);
//      out->Print(method_dict,
//                 "(\'$PackageQualifiedServiceName$\', \'$MethodName$\'): "
//                 "$OutputTypeModuleAndClass$.SerializeToString,\n");
//    }
//    out->Print("}\n");
    out->Print("method_implementations = {\n");
    for (StringMap::iterator name_and_implementation_constructor =
             method_implementation_constructors.begin();
         name_and_implementation_constructor !=
         method_implementation_constructors.end();
         name_and_implementation_constructor++) {
      method_dict["Method"] = name_and_implementation_constructor->first;
      method_dict["Constructor"] = name_and_implementation_constructor->second;
      IndentScope raii_descriptions_indent(out);
      const grpc::string method_name =
          name_and_implementation_constructor->first;
      out->Print(method_dict,
                 "(\'$PackageQualifiedServiceName$\', \'$Method$\'): "
                 "face_utilities.$Constructor$(servicer.$Method$),\n");
    }
    out->Print("}\n");
    out->Print(
        "server_options = beta_implementations.server_options("
        "thread_pool=pool, thread_pool_size=pool_size, "
        "default_timeout=default_timeout, "
        "maximum_timeout=maximum_timeout)\n");
    out->Print(
        "return beta_implementations.server(method_implementations, "
        "options=server_options)\n");
        //"request_deserializers=request_deserializers, "
        //"response_serializers=response_serializers, "
  }
}

void PrivateGenerator::PrintBetaStubFactory(
    const grpc::string& package_qualified_service_name,
    const grpc_generator::Service* service, grpc_generator::Printer* out) {
  StringMap dict;
  dict["Service"] = service->name();
  out->Print("\n\n");
  out->Print(dict,
             "def beta_create_$Service$_stub(channel, host=None,"
             " metadata_transformer=None, pool=None, pool_size=None):\n");
  {
    IndentScope raii_create_server_indent(out);
    out->Print(
        "\"\"\"The Beta API is deprecated for 0.15.0 and later.\n"
        "\nIt is recommended to use the GA API (classes and functions in this\n"
        "file not marked beta) for all further purposes. This function was\n"
        "generated only to ease transition from grpcio<0.15.0 to grpcio>=0.15.0"
        "\"\"\"\n");
    StringMap method_cardinalities;
    StringMap input_message_modules_and_classes;
    StringMap output_message_modules_and_classes;
    for (int i = 0; i < service->method_count(); ++i) {
      auto method = service->method(i);
      const grpc::string method_cardinality =
          grpc::string(method->ClientStreaming() ? "STREAM" : "UNARY") +
          "_" +
          grpc::string(method->ServerStreaming() ? "STREAM" : "UNARY");
      grpc::string input_message_module_and_class = method->get_fb_builder();
      grpc::string output_message_module_and_class = method->get_fb_builder();
      method_cardinalities.insert(
          make_pair(method->name(), method_cardinality));
      input_message_modules_and_classes.insert(
          make_pair(method->name(), input_message_module_and_class));
      output_message_modules_and_classes.insert(
          make_pair(method->name(), output_message_module_and_class));
    }
    StringMap method_dict;
    method_dict["PackageQualifiedServiceName"] = package_qualified_service_name;
//    out->Print("request_serializers = {\n");
//    for (StringMap::iterator name_and_input_module_class_pair =
//             input_message_modules_and_classes.begin();
//         name_and_input_module_class_pair !=
//         input_message_modules_and_classes.end();
//         name_and_input_module_class_pair++) {
//      method_dict["MethodName"] = name_and_input_module_class_pair->first;
//      method_dict["InputTypeModuleAndClass"] =
//          name_and_input_module_class_pair->second;
//      IndentScope raii_indent(out);
//      out->Print(method_dict,
//                 "(\'$PackageQualifiedServiceName$\', \'$MethodName$\'): "
//                 "$InputTypeModuleAndClass$.SerializeToString,\n");
//    }
//    out->Print("}\n");
//    out->Print("response_deserializers = {\n");
//    for (StringMap::iterator name_and_output_module_class_pair =
//             output_message_modules_and_classes.begin();
//         name_and_output_module_class_pair !=
//         output_message_modules_and_classes.end();
//         name_and_output_module_class_pair++) {
//      method_dict["MethodName"] = name_and_output_module_class_pair->first;
//      method_dict["OutputTypeModuleAndClass"] =
//          name_and_output_module_class_pair->second;
//      IndentScope raii_indent(out);
//      out->Print(method_dict,
//                 "(\'$PackageQualifiedServiceName$\', \'$MethodName$\'): "
//                 "$OutputTypeModuleAndClass$.FromString,\n");
//    }
//    out->Print("}\n");
    out->Print("cardinalities = {\n");
    for (StringMap::iterator name_and_cardinality =
             method_cardinalities.begin();
         name_and_cardinality != method_cardinalities.end();
         name_and_cardinality++) {
      method_dict["Method"] = name_and_cardinality->first;
      method_dict["Cardinality"] = name_and_cardinality->second;
      IndentScope raii_descriptions_indent(out);
      out->Print(method_dict,
                 "\'$Method$\': cardinality.Cardinality.$Cardinality$,\n");
    }
    out->Print("}\n");
    out->Print(
        "stub_options = beta_implementations.stub_options("
        "host=host, metadata_transformer=metadata_transformer, "
        "thread_pool=pool, thread_pool_size=pool_size)\n");
    out->Print(method_dict,
               "return beta_implementations.dynamic_stub(channel, "
               "\'$PackageQualifiedServiceName$\', "
               "cardinalities, options=stub_options)\n");
        // "request_serializers=request_serializers, "
        //"response_deserializers=response_deserializers, "
  }
}

void PrivateGenerator::PrintStub(
    const grpc::string& package_qualified_service_name,
    const grpc_generator::Service* service, grpc_generator::Printer* out) {
  StringMap dict;
  dict["Service"] = service->name();
  out->Print("\n\n");
  out->Print(dict, "class $Service$Stub(object):\n");
  {
    IndentScope raii_class_indent(out);
    out->Print("\n");
    out->Print("def __init__(self, channel):\n");
    {
      IndentScope raii_init_indent(out);
      out->Print("\"\"\"Constructor.\n");
      out->Print("\n");
      out->Print("Args:\n");
      {
        IndentScope raii_args_indent(out);
        out->Print("channel: A grpc.Channel.\n");
      }
      out->Print("\"\"\"\n");
      for (int i = 0; i < service->method_count(); ++i) {
        auto method = service->method(i);
        grpc::string multi_callable_constructor =
            grpc::string(method->ClientStreaming() ? "stream" : "unary") +
            "_" +
            grpc::string(method->ServerStreaming() ? "stream" : "unary");
        grpc::string request_module_and_class = method->get_fb_builder();
        grpc::string response_module_and_class = method->get_fb_builder();
        StringMap method_dict;
        method_dict["Method"] = method->name();
        method_dict["MultiCallableConstructor"] = multi_callable_constructor;
        out->Print(method_dict,
                   "self.$Method$ = channel.$MultiCallableConstructor$(\n");
        {
          method_dict["PackageQualifiedService"] =
              package_qualified_service_name;
          method_dict["RequestModuleAndClass"] = request_module_and_class;
          method_dict["ResponseModuleAndClass"] = response_module_and_class;
          IndentScope raii_first_attribute_indent(out);
          IndentScope raii_second_attribute_indent(out);
          out->Print(method_dict, "'/$PackageQualifiedService$/$Method$',\n");
          out->Print(method_dict,"\n");
          out->Print(
              method_dict,"\n");
          out->Print(")\n");
        }
      }
    }
  }
}

void PrivateGenerator::PrintServicer(const grpc_generator::Service* service,
                                     grpc_generator::Printer* out) {
  StringMap service_dict;
  service_dict["Service"] = service->name();
  out->Print("\n\n");
  out->Print(service_dict, "class $Service$Servicer(object):\n");
  {
    IndentScope raii_class_indent(out);
    for (int i = 0; i < service->method_count(); ++i) {
      auto method = service->method(i);
      grpc::string arg_name =
          method->ClientStreaming() ? "request_iterator" : "request";
      StringMap method_dict;
      method_dict["Method"] = method->name();
      method_dict["ArgName"] = arg_name;
      out->Print("\n");
      out->Print(method_dict, "def $Method$(self, $ArgName$, context):\n");
      {
        IndentScope raii_method_indent(out);
        out->Print("context.set_code(grpc.StatusCode.UNIMPLEMENTED)\n");
        out->Print("context.set_details('Method not implemented!')\n");
        out->Print("raise NotImplementedError('Method not implemented!')\n");
      }
    }
  }
}

void PrivateGenerator::PrintAddServicerToServer(
    const grpc::string& package_qualified_service_name,
    const grpc_generator::Service* service, grpc_generator::Printer* out) {
  StringMap service_dict;
  service_dict["Service"] = service->name();
  out->Print("\n\n");
  out->Print(service_dict,
             "def add_$Service$Servicer_to_server(servicer, server):\n");
  {
    IndentScope raii_class_indent(out);
    out->Print("rpc_method_handlers = {\n");
    {
      IndentScope raii_dict_first_indent(out);
      IndentScope raii_dict_second_indent(out);
      for (int i = 0; i < service->method_count(); ++i) {
        auto method = service->method(i);
        grpc::string method_handler_constructor =
            grpc::string(method->ClientStreaming() ? "stream" : "unary") +
            "_" +
            grpc::string(method->ServerStreaming() ? "stream" : "unary") +
            "_rpc_method_handler";
        grpc::string request_module_and_class = method->get_fb_builder();
        grpc::string response_module_and_class = method->get_fb_builder();
        StringMap method_dict;
        method_dict["Method"] = method->name();
        method_dict["MethodHandlerConstructor"] = method_handler_constructor;
        method_dict["RequestModuleAndClass"] = request_module_and_class;
        method_dict["ResponseModuleAndClass"] = response_module_and_class;
        out->Print(method_dict,
                   "'$Method$': grpc.$MethodHandlerConstructor$(\n");
        {
          IndentScope raii_call_first_indent(out);
          IndentScope raii_call_second_indent(out);
          out->Print(method_dict, "servicer.$Method$,\n");
          out->Print(
              method_dict,"\n");
          out->Print(
              method_dict,
              "\n");
        }
        out->Print("),\n");
      }
    }
    StringMap method_dict;
    method_dict["PackageQualifiedServiceName"] = package_qualified_service_name;
    out->Print("}\n");
    out->Print("generic_handler = grpc.method_handlers_generic_handler(\n");
    {
      IndentScope raii_call_first_indent(out);
      IndentScope raii_call_second_indent(out);
      out->Print(method_dict,
                 "'$PackageQualifiedServiceName$', rpc_method_handlers)\n");
    }
    out->Print("server.add_generic_rpc_handlers((generic_handler,))\n");
  }
}

void PrivateGenerator::PrintBetaPreamble(grpc_generator::Printer* out) {
  StringMap var;
  var["Package"] = config.beta_package_root;
  out->Print(var,
             "from $Package$ import implementations as beta_implementations\n");
  out->Print(var, "from $Package$ import interfaces as beta_interfaces\n");
  out->Print("from grpc.framework.common import cardinality\n");
  out->Print(
      "from grpc.framework.interfaces.face import utilities as "
      "face_utilities\n");
}

void PrivateGenerator::PrintPreamble(grpc_generator::Printer* out) {
  StringMap var;
  var["Package"] = config.grpc_package_root;
  out->Print(var, "import $Package$\n");
  out->Print("\n");
  StringPairSet imports_set;
  for (int i = 0; i < file->service_count(); ++i) {
    auto service = file->service(i);
    for (int j = 0; j < service->method_count(); ++j) {
      auto method = service.get()->method(j);

      grpc::string input_type_file_name = method->get_fb_builder();
      grpc::string input_module_name =
          ModuleName(input_type_file_name, config.import_prefix);
      grpc::string input_module_alias =
          ModuleAlias(input_type_file_name, config.import_prefix);
      imports_set.insert(
          std::make_tuple(input_module_name, input_module_alias));

      grpc::string output_type_file_name = method->get_fb_builder();
      grpc::string output_module_name =
          ModuleName(output_type_file_name, config.import_prefix);
      grpc::string output_module_alias =
          ModuleAlias(output_type_file_name, config.import_prefix);
      imports_set.insert(
          std::make_tuple(output_module_name, output_module_alias));
    }
  }

  for (StringPairSet::iterator it = imports_set.begin();
       it != imports_set.end(); ++it) {
    var["ModuleName"] = std::get<0>(*it);
    var["ModuleAlias"] = std::get<1>(*it);
    out->Print(var, "import $ModuleName$ as $ModuleAlias$\n");
  }
}

void PrivateGenerator::PrintGAServices(grpc_generator::Printer* out) {
  grpc::string package = file->package();
  if (!package.empty()) {
    package = package.append(".");
  }

  out->Print(file->additional_headers().c_str());

  for (int i = 0; i < file->service_count(); ++i) {
    auto service = file->service(i);

    grpc::string package_qualified_service_name = package + service->name();
    PrintStub(package_qualified_service_name, service.get(), out);
    PrintServicer(service.get(), out);
    PrintAddServicerToServer(package_qualified_service_name, service.get(),
                             out);
  }
}

void PrivateGenerator::PrintBetaServices(grpc_generator::Printer* out) {
  grpc::string package = file->package();
  if (!package.empty()) {
    package = package.append(".");
  }
  for (int i = 0; i < file->service_count(); ++i) {
    auto service = file->service(i);

    grpc::string package_qualified_service_name = package + service->name();
    PrintBetaServicer(service.get(), out);
    PrintBetaStub(service.get(), out);
    PrintBetaServerFactory(package_qualified_service_name, service.get(), out);
    PrintBetaStubFactory(package_qualified_service_name, service.get(), out);
  }
}

grpc::string PrivateGenerator::GetGrpcServices() {
  grpc::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    auto out = file->CreatePrinter(&output);
    out->Print(
        "# Generated by the gRPC Python protocol compiler plugin. "
        "DO NOT EDIT!\n");
    StringMap var;
    var["Package"] = config.grpc_package_root;
    out->Print(var, "import $Package$\n");
    PrintGAServices(out.get());
    out->Print("try:\n");
    {
      IndentScope raii_dict_try_indent(out.get());
      out->Print(
          "# THESE ELEMENTS WILL BE DEPRECATED.\n"
          "# Please use the generated *_pb2_grpc.py files instead.\n");
      out->Print(var, "import $Package$\n");
      PrintBetaPreamble(out.get());
      PrintGAServices(out.get());
      PrintBetaServices(out.get());
    }
    out->Print("except ImportError:\n");
    {
      IndentScope raii_dict_except_indent(out.get());
      out->Print("pass");
    }
  }
  return output;
}

}  // namespace grpc_python_generator
