/*
 * Copyright 2020 Google Inc. All rights reserved.
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

/*
 * NOTE: The following implementation is a translation for the Swift-grpc
 * generator since flatbuffers doesnt allow plugins for now. if an issue arises
 * please open an issue in the flatbuffers repository. This file should always
 * be maintained according to the Swift-grpc repository
 */

#include <map>
#include <sstream>

#include "flatbuffers/util.h"
#include "src/compiler/schema_interface.h"
#include "src/compiler/ts_generator.h"

namespace grpc_ts_generator {

// MARK: - Shared code

void GenerateImports(grpc_generator::Printer *printer,
                     std::map<grpc::string, grpc::string> *dictonary,
                     const bool grpc_var_import) {
  auto vars = *dictonary;
  printer->Print(
      "// Generated GRPC code for FlatBuffers TS *** DO NOT EDIT ***\n");
  printer->Print("import { flatbuffers } from 'flatbuffers';\n");
  printer->Print(vars,
                 "import *  as $FBSFile$ from './$Filename$_generated';\n");
  printer->Print("\n");
  if (grpc_var_import)
    printer->Print("var grpc = require('grpc');\n");
  else
    printer->Print("import * as grpc from 'grpc';\n");
  printer->Print("\n");
}

// MARK: - Generate Main GRPC Code

void GetStreamType(grpc_generator::Printer *printer,
                   const grpc_generator::Method *method,
                   std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  auto client_streaming = method->ClientStreaming() || method->BidiStreaming();
  auto server_streaming = method->ServerStreaming() || method->BidiStreaming();
  vars["ClientStreaming"] = client_streaming ? "true" : "false";
  vars["ServerStreaming"] = server_streaming ? "true" : "false";
  printer->Print(vars, "requestStream: $ClientStreaming$,\n");
  printer->Print(vars, "responseStream: $ServerStreaming$,\n");
}

void GenerateSerializeMethod(grpc_generator::Printer *printer,
                             std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  printer->Print(vars, "function serialize_$Type$(buffer_args) {\n");
  printer->Indent();
  printer->Print(vars, "if (!(buffer_args instanceof $FBSFile$.$Type$)) {\n");
  printer->Indent();
  printer->Print(
      vars, "throw new Error('Expected argument of type $FBSFile$.$Type$');\n");
  printer->Outdent();
  printer->Print("}\n");
  printer->Print(vars, "return buffer_args.serialize();\n");
  printer->Outdent();
  printer->Print("}\n\n");
}

void GenerateDeserializeMethod(
    grpc_generator::Printer *printer,
    std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  printer->Print(vars, "function deserialize_$Type$(buffer) {\n");
  printer->Indent();
  printer->Print(vars,
                 "return $FBSFile$.$Type$.getRootAs$Type$(new "
                 "flatbuffers.ByteBuffer(buffer))\n");
  printer->Outdent();
  printer->Print("}\n\n");
}

void GenerateMethods(const grpc_generator::Service *service,
                     grpc_generator::Printer *printer,
                     std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;

  std::set<grpc::string> generated_functions;

  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    auto output = method->get_output_type_name();
    auto input = method->get_input_type_name();

    if (generated_functions.find(output) == generated_functions.end()) {
      generated_functions.insert(output);
      vars["Type"] = output;
      GenerateSerializeMethod(printer, &vars);
      GenerateDeserializeMethod(printer, &vars);
    }
    printer->Print("\n");
    if (generated_functions.find(input) == generated_functions.end()) {
      generated_functions.insert(input);
      vars["Type"] = input;
      GenerateSerializeMethod(printer, &vars);
      GenerateDeserializeMethod(printer, &vars);
    }
  }
}

void GenerateService(const grpc_generator::Service *service,
                     grpc_generator::Printer *printer,
                     std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  vars["NAME"] = service->name() + "Service";

  printer->Print(vars, "var $NAME$ = exports.$NAME$ = {\n");
  printer->Indent();
  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    vars["MethodName"] = method->name();
    vars["Output"] = method->get_output_type_name();
    vars["Input"] = method->get_input_type_name();
    printer->Print(vars, "$MethodName$: {\n");
    printer->Indent();
    printer->Print(vars, "path: '/$PATH$$ServiceName$/$MethodName$',\n");
    GetStreamType(printer, &*method, &vars);
    printer->Print(vars, "requestType: flatbuffers.ByteBuffer,\n");
    printer->Print(vars, "responseType: $FBSFile$.$Output$,\n");
    printer->Print(vars, "requestSerialize: serialize_$Input$,\n");
    printer->Print(vars, "requestDeserialize: deserialize_$Input$,\n");
    printer->Print(vars, "responseSerialize: serialize_$Output$,\n");
    printer->Print(vars, "responseDeserialize: deserialize_$Output$,\n");
    printer->Outdent();
    printer->Print("},\n");
  }
  printer->Outdent();
  printer->Print("};\n");
  printer->Print(vars,
                 "exports.$ServiceName$Client = "
                 "grpc.makeGenericClientConstructor($NAME$);");
}

grpc::string Generate(grpc_generator::File *file,
                      const grpc_generator::Service *service,
                      const grpc::string &filename) {
  grpc::string output;
  std::map<grpc::string, grpc::string> vars;

  vars["PATH"] = file->package();

  if (!file->package().empty()) { vars["PATH"].append("."); }

  vars["ServiceName"] = service->name();
  vars["FBSFile"] = service->name() + "_fbs";
  vars["Filename"] = filename;
  auto printer = file->CreatePrinter(&output);

  GenerateImports(&*printer, &vars, true);
  GenerateMethods(service, &*printer, &vars);
  GenerateService(service, &*printer, &vars);
  return output;
}

// MARK: - Generate Interface

void FillInterface(grpc_generator::Printer *printer,
                   std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  printer->Print(
      vars,
      "interface I$ServiceName$Service_I$MethodName$ extends "
      "grpc.MethodDefinition<$FBSFile$.$INPUT$, $FBSFile$.$OUTPUT$> {\n");
  printer->Indent();
  printer->Print(vars, "path: string; // /$PATH$$ServiceName$/$MethodName$\n");
  printer->Print(vars, "requestStream: boolean; // $ClientStreaming$\n");
  printer->Print(vars, "responseStream: boolean; // $ServerStreaming$\n");
  printer->Print(vars,
                 "requestSerialize: grpc.serialize<$FBSFile$.$INPUT$>;\n");
  printer->Print(vars,
                 "requestDeserialize: grpc.deserialize<$FBSFile$.$INPUT$>;\n");
  printer->Print(vars,
                 "responseSerialize: grpc.serialize<$FBSFile$.$OUTPUT$>;\n");
  printer->Print(
      vars, "responseDeserialize: grpc.deserialize<$FBSFile$.$OUTPUT$>;\n");
  printer->Outdent();
  printer->Print("}\n");
}

void GenerateInterfaces(const grpc_generator::Service *service,
                        grpc_generator::Printer *printer,
                        std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    auto client_streaming =
        method->ClientStreaming() || method->BidiStreaming();
    auto server_streaming =
        method->ServerStreaming() || method->BidiStreaming();
    vars["ClientStreaming"] = client_streaming ? "true" : "false";
    vars["ServerStreaming"] = server_streaming ? "true" : "false";
    vars["MethodName"] = method->name();
    vars["INPUT"] = method->get_input_type_name();
    vars["OUTPUT"] = method->get_output_type_name();
    FillInterface(printer, &vars);
    printer->Print("\n");
  }
}

void GenerateExportedInterface(
    const grpc_generator::Service *service, grpc_generator::Printer *printer,
    std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  printer->Print(vars, "export interface I$ServiceName$Server {\n");
  printer->Indent();
  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    vars["Name"] = method->name();
    vars["INPUT"] = method->get_input_type_name();
    vars["OUTPUT"] = method->get_output_type_name();
    if (method->BidiStreaming()) {
      printer->Print(vars,
                     "$Name$: grpc.handleBidiStreamingCall<$FBSFile$.$INPUT$, "
                     "$FBSFile$.$OUTPUT$>;\n");
      continue;
    }
    if (method->NoStreaming()) {
      printer->Print(vars,
                     "$Name$: grpc.handleUnaryCall<$FBSFile$.$INPUT$, "
                     "$FBSFile$.$OUTPUT$>;\n");
      continue;
    }
    if (method->ClientStreaming()) {
      printer->Print(
          vars,
          "$Name$: grpc.handleClientStreamingCall<$FBSFile$.$INPUT$, "
          "$FBSFile$.$OUTPUT$>;\n");
      continue;
    }
    if (method->ServerStreaming()) {
      printer->Print(
          vars,
          "$Name$: grpc.handleServerStreamingCall<$FBSFile$.$INPUT$, "
          "$FBSFile$.$OUTPUT$>;\n");
      continue;
    }
  }
  printer->Outdent();
  printer->Print("}\n");
}

void GenerateMainInterface(const grpc_generator::Service *service,
                           grpc_generator::Printer *printer,
                           std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  printer->Print(
      vars,
      "interface I$ServiceName$Service extends "
      "grpc.ServiceDefinition<grpc.UntypedServiceImplementation> {\n");
  printer->Indent();
  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    vars["MethodName"] = method->name();
    printer->Print(vars,
                   "$MethodName$: I$ServiceName$Service_I$MethodName$;\n");
  }
  printer->Outdent();
  printer->Print("}\n");
  GenerateInterfaces(service, printer, &vars);
  printer->Print("\n");
  printer->Print(vars,
                 "export const $ServiceName$Service: I$ServiceName$Service;\n");
  printer->Print("\n");
  GenerateExportedInterface(service, printer, &vars);
}

grpc::string GenerateMetaData() { return "metadata: grpc.Metadata"; }

grpc::string GenerateOptions() { return "options: Partial<grpc.CallOptions>"; }

void GenerateUnaryClientInterface(
    grpc_generator::Printer *printer,
    std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  grpc::string main = "$ISPUBLIC$$MethodName$(request: $FBSFile$.$INPUT$, ";
  grpc::string callback =
      "callback: (error: grpc.ServiceError | null, response: "
      "$FBSFile$.$OUTPUT$) => void): grpc.ClientUnaryCall;\n";
  auto meta_data = GenerateMetaData() + ", ";
  auto options = GenerateOptions() + ", ";
  printer->Print(vars, (main + callback).c_str());
  printer->Print(vars, (main + meta_data + callback).c_str());
  printer->Print(vars, (main + meta_data + options + callback).c_str());
}

void GenerateClientWriteStreamInterface(
    grpc_generator::Printer *printer,
    std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  grpc::string main = "$ISPUBLIC$$MethodName$(";
  grpc::string callback =
      "callback: (error: grpc.ServiceError | null, response: "
      "$FBSFile$.$INPUT$) => void): "
      "grpc.ClientWritableStream<$FBSFile$.$OUTPUT$>;\n";
  auto meta_data = GenerateMetaData() + ", ";
  auto options = GenerateOptions() + ", ";
  printer->Print(vars, (main + callback).c_str());
  printer->Print(vars, (main + meta_data + callback).c_str());
  printer->Print(vars, (main + options + callback).c_str());
  printer->Print(vars, (main + meta_data + options + callback).c_str());
}

void GenerateClientReadableStreamInterface(
    grpc_generator::Printer *printer,
    std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  grpc::string main = "$ISPUBLIC$$MethodName$(request: $FBSFile$.$INPUT$, ";
  grpc::string end_function =
      "): grpc.ClientReadableStream<$FBSFile$.$OUTPUT$>;\n";
  auto meta_data = GenerateMetaData();
  auto options = GenerateOptions();
  printer->Print(vars, (main + meta_data + end_function).c_str());
  printer->Print(vars, (main + options + end_function).c_str());
}

void GenerateDepluxStreamInterface(
    grpc_generator::Printer *printer,
    std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  grpc::string main = "$ISPUBLIC$$MethodName$(";
  grpc::string end_function =
      "): grpc.ClientDuplexStream<$FBSFile$.$INPUT$, $FBSFile$.$OUTPUT$>;\n";
  auto meta_data = GenerateMetaData();
  auto options = GenerateOptions();
  printer->Print(vars, (main + end_function).c_str());
  printer->Print(vars, (main + options + end_function).c_str());
  printer->Print(vars, (main + meta_data +
                        ", options?: Partial<grpc.CallOptions>" + end_function)
                           .c_str());
}

void GenerateClientInterface(const grpc_generator::Service *service,
                             grpc_generator::Printer *printer,
                             std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  printer->Print(vars, "export interface I$ServiceName$Client {\n");
  printer->Indent();
  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    vars["MethodName"] = method->name();
    vars["INPUT"] = method->get_input_type_name();
    vars["OUTPUT"] = method->get_output_type_name();
    vars["ISPUBLIC"] = "";

    if (method->NoStreaming()) {
      GenerateUnaryClientInterface(printer, &vars);
      continue;
    }
    if (method->BidiStreaming()) {
      GenerateDepluxStreamInterface(printer, &vars);
      continue;
    }

    if (method->ClientStreaming()) {
      GenerateClientWriteStreamInterface(printer, &vars);
      continue;
    }

    if (method->ServerStreaming()) {
      GenerateClientReadableStreamInterface(printer, &vars);
      continue;
    }
  }
  printer->Outdent();
  printer->Print("}\n");
}

void GenerateClientClassInterface(
    const grpc_generator::Service *service, grpc_generator::Printer *printer,
    std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  printer->Print(vars,
                 "export class $ServiceName$Client extends grpc.Client "
                 "implements I$ServiceName$Client {\n");
  printer->Indent();
  printer->Print(
      "constructor(address: string, credentials: grpc.ChannelCredentials, "
      "options?: object);");
  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    vars["MethodName"] = method->name();
    vars["INPUT"] = method->get_input_type_name();
    vars["OUTPUT"] = method->get_output_type_name();
    vars["ISPUBLIC"] = "public ";
    if (method->NoStreaming()) {
      GenerateUnaryClientInterface(printer, &vars);
      continue;
    }
    if (method->BidiStreaming()) {
      GenerateDepluxStreamInterface(printer, &vars);
      continue;
    }

    if (method->ClientStreaming()) {
      GenerateClientWriteStreamInterface(printer, &vars);
      continue;
    }

    if (method->ServerStreaming()) {
      GenerateClientReadableStreamInterface(printer, &vars);
      continue;
    }
  }
  printer->Outdent();
  printer->Print("}\n");
}

grpc::string GenerateInterface(grpc_generator::File *file,
                               const grpc_generator::Service *service,
                               const grpc::string &filename) {
  grpc::string output;

  std::set<grpc::string> generated_functions;
  std::map<grpc::string, grpc::string> vars;

  vars["PATH"] = file->package();

  if (!file->package().empty()) { vars["PATH"].append("."); }

  vars["ServiceName"] = service->name();
  vars["FBSFile"] = service->name() + "_fbs";
  vars["Filename"] = filename;
  auto printer = file->CreatePrinter(&output);

  GenerateImports(&*printer, &vars, false);
  GenerateMainInterface(service, &*printer, &vars);
  printer->Print("\n");
  GenerateClientInterface(service, &*printer, &vars);
  printer->Print("\n");
  GenerateClientClassInterface(service, &*printer, &vars);
  return output;
}
}  // namespace grpc_ts_generator
