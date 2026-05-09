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
#include "src/compiler/swift_generator.h"

#include <map>
#include <sstream>

#include "flatbuffers/util.h"
#include "src/compiler/schema_interface.h"

namespace grpc_swift_generator {
namespace {

static std::string ServerResponse() { return "GRPCCore.ServerResponse"; }

static std::string ServerRequest() { return "GRPCCore.ServerRequest"; }

static std::string StreamingServerRequest() {
  return "GRPCCore.StreamingServerRequest";
}

static std::string StreamingServerResponse() {
  return "GRPCCore.StreamingServerResponse";
}

static std::string QualifiedName(const std::vector<std::string>& components,
                                 const grpc::string& name,
                                 const std::string& separator = "_") {
  std::string qualified_name;
  for (auto it = components.begin(); it != components.end(); ++it)
    qualified_name += *it + separator;
  return qualified_name + name;
}

static std::string GenerateGRPCMessage(const std::string& name) {
  return "GRPCMessage<" + name + ">";
}

static std::string GenerateType(const std::string& name,
                                const std::string& wrapper) {
  return wrapper + "<" + GenerateGRPCMessage(name) + ">";
}

static std::string GenerateInputMethodTypes(
    const grpc_generator::Method* method) {
  if (method->BidiStreaming()) {
    return StreamingServerRequest();
  } else if (method->ClientStreaming()) {
    return StreamingServerRequest();
  } else if (method->ServerStreaming()) {
    return ServerRequest();
  } else {
    return ServerRequest();
  }
}

static std::string GenerateOutputMethodTypes(
    const grpc_generator::Method* method) {
  if (method->BidiStreaming()) {
    return StreamingServerResponse();
  } else if (method->ClientStreaming()) {
    return ServerResponse();
  } else if (method->ServerStreaming()) {
    return StreamingServerResponse();
  } else {
    return ServerResponse();
  }
}

static std::string GenerateMethodTypes(const grpc_generator::Method* method) {
  if (method->BidiStreaming()) {
    return "bidirectionalStreaming";
  } else if (method->ClientStreaming()) {
    return "clientStreaming";
  } else if (method->ServerStreaming()) {
    return "serverStreaming";
  } else {
    return "unary";
  }
}

void EnforceOSVersion(grpc_generator::Printer* printer) {
  printer->Print(
      "@available(macOS 15.0, iOS 18.0, watchOS 11.0, tvOS 18.0, visionOS 2.0, "
      "*)\n");
}

void GenerateMethodDescriptor(grpc_generator::Printer* printer,
                              std::map<grpc::string, grpc::string>* dictionary) {
  auto vars = *dictionary;
  printer->Print(vars, "$ACCESS$ enum $MethodName$: Sendable {\n");
  printer->Indent();
  printer->Print(vars, "$ACCESS$ typealias Input = FlatBufferBuilder\n");
  printer->Print(vars, "$ACCESS$ typealias Output = $Output$\n");
  printer->Print(
      vars, "$ACCESS$ static let descriptor = GRPCCore.MethodDescriptor(\n");
  printer->Indent();
  printer->Print(vars,
                 "service: GRPCCore.ServiceDescriptor(fullyQualifiedService: "
                 "\"$ServiceQualifiedName$\"),\n");
  printer->Print(vars, "method: \"$MethodName$\"\n");
  printer->Outdent();
  printer->Print(")\n");
  printer->Outdent();
  printer->Print(vars, "}\n");
}

void GenerateCoders(grpc_generator::Printer* printer) {
  EnforceOSVersion(printer);
  printer->Print(
      "extension FlatBuffersMessageSerializer: MessageSerializer {\n");
  printer->Indent();
  printer->Print(
      "public func serialize<Bytes>(_ message: Message) throws -> Bytes where "
      "Bytes : GRPCCore.GRPCContiguousBytes {\n");
  printer->Indent();
  printer->Print("do {\n");
  printer->Indent();
  printer->Print(
      "return try self.serialize(message: message) { GRPCNIOTransportBytes($0) "
      "} as! Bytes\n");
  printer->Outdent();
  printer->Print("} catch let error {\n");
  printer->Indent();
  printer->Print("throw RPCError(\n");
  printer->Indent();
  printer->Print("code: .invalidArgument,\n");
  printer->Print("message: \"Can't serialize message\",\n");
  printer->Print("cause: error\n");
  printer->Outdent();
  printer->Print(")\n");
  printer->Outdent();
  printer->Print("}\n");
  printer->Outdent();
  printer->Print("}\n");
  printer->Outdent();
  printer->Print("}\n\n");

  EnforceOSVersion(printer);
  printer->Print(
      "extension FlatBuffersMessageDeserializer: MessageDeserializer {\n");
  printer->Indent();
  printer->Print(
      "public func deserialize<Bytes>(_ serializedMessageBytes: Bytes) throws "
      "-> Message where Bytes : GRPCCore.GRPCContiguousBytes {\n");
  printer->Indent();
  printer->Print("do {\n");
  printer->Indent();
  printer->Print("return try serializedMessageBytes.withUnsafeBytes {\n");
  printer->Indent();
  printer->Print("try self.deserialize(pointer: $0)\n");
  printer->Outdent();
  printer->Print("}\n");
  printer->Outdent();
  printer->Print("} catch let error {\n");
  printer->Indent();
  printer->Print("throw RPCError(\n");
  printer->Indent();
  printer->Print("code: .invalidArgument,\n");
  printer->Print(
      "message: \"Can't Decode message of type \\(Message.self)\",\n");
  printer->Print("cause: error\n");
  printer->Outdent();
  printer->Print(")\n");
  printer->Outdent();
  printer->Print("}\n");
  printer->Outdent();
  printer->Print("}\n");
  printer->Outdent();
  printer->Print("}\n\n");
}

void GenerateSharedContent(const grpc_generator::Service* service,
                           grpc_generator::Printer* printer,
                           std::map<grpc::string, grpc::string>* dictionary) {
  auto vars = *dictionary;
  EnforceOSVersion(printer);
  printer->Print(vars,
                 "$ACCESS$ enum $SwiftServiceQualifiedName$: Sendable {\n");
  printer->Indent();
  printer->Print(vars,
                 "$ACCESS$ static let descriptor = "
                 "GRPCCore.ServiceDescriptor(fullyQualifiedService: "
                 "\"$ServiceQualifiedName$\")\n");
  printer->Print(vars, "$ACCESS$ enum Method: Sendable {\n");
  printer->Indent();

  std::vector<std::string> descriptors;
  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    vars["Input"] = QualifiedName(method->get_input_namespace_parts(),
                                  method->get_output_type_name());
    vars["Output"] = QualifiedName(method->get_output_namespace_parts(),
                                   method->get_output_type_name());
    auto name = method->name();
    vars["MethodName"] = name;
    descriptors.push_back(name);
    GenerateMethodDescriptor(printer, &vars);
  }

  printer->Print(
      vars,
      "$ACCESS$ static let descriptors: [GRPCCore.MethodDescriptor] = [\n");
  printer->Indent();
  for (auto it = descriptors.begin(); it < descriptors.end(); it++) {
    vars["MethodName"] = *it;
    printer->Print(vars, "$MethodName$.descriptor,\n");
  }
  printer->Outdent();
  printer->Print("]\n");
  printer->Outdent();
  printer->Print("}\n");

  printer->Outdent();
  printer->Print("}\n\n");

  EnforceOSVersion(printer);
  printer->Print("extension GRPCCore.ServiceDescriptor {\n");
  printer->Indent();
  printer->Print(vars,
                 "$ACCESS$ static let $SwiftServiceQualifiedName$ = "
                 "GRPCCore.ServiceDescriptor(fullyQualifiedService: "
                 "\"$ServiceQualifiedName$\")\n");
  printer->Outdent();
  printer->Print("}\n");
}

// Service Generation

void GenerateFunction(grpc_generator::Printer* printer,
                      std::map<grpc::string, grpc::string>* dictionary) {
  auto vars = *dictionary;
  printer->Print(vars, "func $MethodName$(\n");
  printer->Indent();

  printer->Print(vars, "request: $Input$,\n");
  printer->Print("context: GRPCCore.ServerContext\n");

  printer->Outdent();
  printer->Print(vars, ") async throws -> $Output$\n\n");
}

void GenerateServiceProtocols(const grpc_generator::Service* service,
                              grpc_generator::Printer* printer,
                              std::map<grpc::string, grpc::string>* dictionary) {
  auto vars = *dictionary;
  EnforceOSVersion(printer);
  printer->Print(vars, "extension $SwiftServiceQualifiedName$ {\n");
  printer->Indent();
  // Base protocol
  printer->Print(vars,
                 "$ACCESS$ protocol StreamingServiceProtocol: "
                 "GRPCCore.RegistrableRPCService {\n");
  printer->Indent();

  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    vars["Input"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     StreamingServerRequest());

    vars["Output"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     StreamingServerResponse());
    auto name = method->name();
    vars["MethodName"] = name;
    GenerateFunction(printer, &vars);
  }
  printer->Outdent();
  printer->Print("}\n\n");

  // Service
  printer->Print(vars,
                 "$ACCESS$ protocol ServiceProtocol: "
                 "$SwiftServiceQualifiedName$.StreamingServiceProtocol {\n");
  printer->Indent();

  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);

    vars["Input"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     GenerateInputMethodTypes(&*method));

    vars["Output"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     GenerateOutputMethodTypes(&*method));
    auto name = method->name();
    vars["MethodName"] = name;
    GenerateFunction(printer, &vars);
  }

  printer->Outdent();
  printer->Print("}\n\n");

  // Simple service
  printer->Print(vars,
                 "$ACCESS$ protocol SimpleServiceProtocol: "
                 "$SwiftServiceQualifiedName$.ServiceProtocol {\n");
  printer->Indent();

  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);

    vars["Input"] = GenerateGRPCMessage(QualifiedName(
        method->get_input_namespace_parts(), method->get_output_type_name()));

    vars["Output"] = GenerateGRPCMessage(QualifiedName(
        method->get_input_namespace_parts(), method->get_output_type_name()));
    auto name = method->name();
    vars["MethodName"] = name;

    printer->Print(vars, "func $MethodName$(\n");
    printer->Indent();

    if (method->ClientStreaming() || method->BidiStreaming()) {
      printer->Print(
          vars,
          "request: GRPCCore.RPCAsyncSequence<$Input$, any Swift.Error>,\n");
    } else {
      printer->Print(vars, "request: $Input$,\n");
    }

    if (method->BidiStreaming() || method->ServerStreaming()) {
      printer->Print(vars, "response: GRPCCore.RPCWriter<$Output$>,\n");
    }
    printer->Print("context: GRPCCore.ServerContext\n");

    printer->Outdent();

    if (!method->BidiStreaming() && !method->ServerStreaming()) {
      printer->Print(vars, ") async throws -> $Output$\n\n");
    } else {
      printer->Print(") async throws\n\n");
    }
  }

  printer->Outdent();
  printer->Print("}\n");

  printer->Outdent();
  printer->Print("}\n\n");

  EnforceOSVersion(printer);
  printer->Print(
      vars,
      "extension $SwiftServiceQualifiedName$.StreamingServiceProtocol {\n");
  printer->Indent();
  printer->Print(
      "public func registerMethods<Transport>(with router: inout "
      "GRPCCore.RPCRouter<Transport>) where Transport: "
      "GRPCCore.ServerTransport {\n");
  printer->Indent();

  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    vars["Input"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     "FlatBuffersMessageSerializer");

    vars["Output"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     "FlatBuffersMessageDeserializer");

    auto name = method->name();
    vars["MethodName"] = name;

    printer->Print("router.registerHandler(\n");
    printer->Indent();
    printer->Print(
        vars,
        "forMethod: "
        "$SwiftServiceQualifiedName$.Method.$MethodName$.descriptor,\n");
    printer->Print(vars, "deserializer: $Output$(),\n");
    printer->Print(vars, "serializer: $Input$(),\n");
    printer->Print("handler: { request, context in\n");
    printer->Indent();
    printer->Print(vars, "try await self.$MethodName$(\n");
    printer->Indent();
    printer->Print("request: request,\n");
    printer->Print("context: context\n");
    printer->Outdent();
    printer->Print(")\n");
    printer->Outdent();
    printer->Print("}\n");
    printer->Outdent();
    printer->Print(")\n");
  }

  printer->Outdent();
  printer->Print("}\n");
  printer->Outdent();
  printer->Print("}\n\n");
}

void CreateServiceProtocolFunctionsImplementations(
    grpc_generator::Printer* printer,
    std::map<grpc::string, grpc::string>* dictionary) {
  auto vars = *dictionary;
  printer->Print(vars, "$ACCESS$ func $MethodName$(\n");
  printer->Indent();
  printer->Print(vars, "request: $Input$,\n");
  printer->Print("context: GRPCCore.ServerContext\n");
  printer->Outdent();
  printer->Print(vars, ") async throws -> $Output$ {\n");
  printer->Indent();
  printer->Print(vars, "let response = try await self.$MethodName$(\n");
  printer->Indent();
  printer->Print(vars, "request: $InputRequest$,\n");
  printer->Print("context: context\n");
  printer->Outdent();
  printer->Print(")\n");
  printer->Print(vars, "return $OutputResponse$\n");
  printer->Outdent();
  printer->Print("}\n");
}

std::string GenerateInputRequest(const grpc_generator::Method* method) {
  if (method->NoStreaming() || method->ServerStreaming())
    return ServerRequest() + "(stream: request)";
  return "request";
}

std::string GenerateOutputResponse(const grpc_generator::Method* method) {
  if (method->NoStreaming() || method->ClientStreaming())
    return StreamingServerResponse() + "(single: response)";
  return "response";
}

void GenerateServiceDefaultImplementation(
    const grpc_generator::Service* service, grpc_generator::Printer* printer,
    std::map<grpc::string, grpc::string>* dictionary) {
  auto vars = *dictionary;
  EnforceOSVersion(printer);
  printer->Print(vars,
                 "extension $SwiftServiceQualifiedName$.ServiceProtocol {\n");
  printer->Indent();

  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    if (method->BidiStreaming()) {
      continue;
    }

    vars["Input"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     StreamingServerRequest());

    vars["Output"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     StreamingServerResponse());
    auto name = method->name();
    vars["MethodName"] = name;

    vars["InputRequest"] = GenerateInputRequest(&*method);
    vars["OutputResponse"] = GenerateOutputResponse(&*method);

    CreateServiceProtocolFunctionsImplementations(printer, &vars);
  }
  printer->Outdent();
  printer->Print("}\n\n");
}

void GenerateSimpleServiceImplemetation(
    const grpc_generator::Service* service, grpc_generator::Printer* printer,
    std::map<grpc::string, grpc::string>* dictionary) {
  auto vars = *dictionary;
  EnforceOSVersion(printer);
  printer->Print(
      vars, "extension $SwiftServiceQualifiedName$.SimpleServiceProtocol {\n");
  printer->Indent();

  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);

    vars["Input"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     GenerateInputMethodTypes(&*method));

    vars["Output"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     GenerateOutputMethodTypes(&*method));
    auto name = method->name();
    vars["MethodName"] = name;

    printer->Print(vars, "$ACCESS$ func $MethodName$(\n");
    printer->Indent();
    printer->Print(vars, "request: $Input$,\n");
    printer->Print("context: GRPCCore.ServerContext\n");
    printer->Outdent();
    printer->Print(vars, ") async throws -> $Output$ {\n");
    printer->Indent();
    printer->Print(vars, "return $Output$(\n");
    printer->Indent();

    if (method->NoStreaming() || method->ClientStreaming()) {
      vars["Message"] = method->ClientStreaming() ? "messages" : "message";
      printer->Print(vars, "message: try await self.$MethodName$(\n");
      printer->Indent();
      printer->Print(vars, "request: request.$Message$,\n");
      printer->Print("context: context\n");
      printer->Outdent();
      printer->Print(vars, "),\n");
      printer->Print(vars, "metadata: [:]\n");
    } else {
      vars["Message"] = method->BidiStreaming() ? "messages" : "message";
      printer->Print(vars, "metadata: [:],\n");
      printer->Print(vars, "producer: { writer in\n");
      printer->Indent();
      printer->Print(vars, "try await self.$MethodName$(\n");
      printer->Indent();
      printer->Print(vars, "request: request.$Message$,\n");
      printer->Print("response: writer,\n");
      printer->Print("context: context\n");
      printer->Outdent();
      printer->Print(")\n");
      printer->Print("return [:]\n");
      printer->Outdent();
      printer->Print(vars, "}\n");
    }

    printer->Outdent();
    printer->Print(")\n");
    printer->Outdent();
    printer->Print("}\n");
  }
  printer->Outdent();
  printer->Print("}\n\n");
}

void GenerateService(const grpc_generator::Service* service,
                     grpc_generator::Printer* printer,
                     std::map<grpc::string, grpc::string>* dictionary) {
  GenerateServiceProtocols(service, printer, dictionary);
  GenerateServiceDefaultImplementation(service, printer, dictionary);
  GenerateSimpleServiceImplemetation(service, printer, dictionary);
}

// Client Generation

void GenerateClientProtocols(const grpc_generator::Service* service,
                             grpc_generator::Printer* printer,
                             std::map<grpc::string, grpc::string>* dictionary) {
  auto vars = *dictionary;
  EnforceOSVersion(printer);
  printer->Print(vars, "extension $SwiftServiceQualifiedName$ {\n");
  printer->Indent();
  // Base protocol
  printer->Print(vars, "$ACCESS$ protocol ClientProtocol: Sendable {\n");
  printer->Indent();

  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    auto name = method->name();
    vars["MethodName"] = name;

    vars["serializer"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     "GRPCCore.MessageSerializer");

    vars["deserializer"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     "GRPCCore.MessageDeserializer");

    if (method->ClientStreaming() || method->BidiStreaming()) {
      vars["request"] =
          GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                     method->get_output_type_name()),
                       "GRPCCore.StreamingClientRequest");
    } else {
      vars["request"] =
          GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                     method->get_output_type_name()),
                       "GRPCCore.ClientRequest");
    }

    if (method->ServerStreaming() || method->BidiStreaming()) {
      vars["response"] =
          GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                     method->get_output_type_name()),
                       "GRPCCore.StreamingClientResponse");
    } else {
      vars["response"] =
          GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                     method->get_output_type_name()),
                       "GRPCCore.ClientResponse");
    }

    printer->Print(vars, "func $MethodName$<Result>(\n");
    printer->Indent();
    printer->Print(vars, "request: $request$,\n");
    printer->Print(vars, "serializer: some $serializer$,\n");
    printer->Print(vars, "deserializer: some $deserializer$,\n");
    printer->Print("options: GRPCCore.CallOptions,\n");
    printer->Print(vars,
                   "onResponse handleResponse: @Sendable @escaping "
                   "($response$) async throws -> Result\n");
    printer->Outdent();
    printer->Print(") async throws -> Result where Result: Sendable\n\n");
  }

  printer->Outdent();
  printer->Print("}\n");
  printer->Outdent();
  printer->Print("}\n\n");
}

void GenerateClientStruct(const grpc_generator::Service* service,
                          grpc_generator::Printer* printer,
                          std::map<grpc::string, grpc::string>* dictionary) {
  auto vars = *dictionary;
  EnforceOSVersion(printer);
  printer->Print(vars, "extension $SwiftServiceQualifiedName$ {\n");
  printer->Indent();
  // Base protocol
  printer->Print(vars,
                 "$ACCESS$ struct Client<Transport>: ClientProtocol where "
                 "Transport: GRPCCore.ClientTransport {\n");
  printer->Indent();
  printer->Print("private let client: GRPCCore.GRPCClient<Transport>\n\n");
  printer->Print(
      vars,
      "$ACCESS$ init(wrapping client: GRPCCore.GRPCClient<Transport>) {\n");
  printer->Indent();
  printer->Print("self.client = client\n");
  printer->Outdent();
  printer->Print("}\n\n");

  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    auto name = method->name();
    vars["MethodType"] = GenerateMethodTypes(&*method);
    vars["MethodName"] = name;

    vars["serializer"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     "GRPCCore.MessageSerializer");

    vars["deserializer"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     "GRPCCore.MessageDeserializer");

    if (method->ClientStreaming() || method->BidiStreaming()) {
      vars["request"] =
          GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                     method->get_output_type_name()),
                       "GRPCCore.StreamingClientRequest");
    } else {
      vars["request"] =
          GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                     method->get_output_type_name()),
                       "GRPCCore.ClientRequest");
    }

    if (method->ServerStreaming() || method->BidiStreaming()) {
      vars["response"] =
          GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                     method->get_output_type_name()),
                       "GRPCCore.StreamingClientResponse");
      vars["CompletionBlock"] = "";
    } else {
      vars["response"] =
          GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                     method->get_output_type_name()),
                       "GRPCCore.ClientResponse");
      vars["CompletionBlock"] = " = { response in try response.message }";
    }

    printer->Print(vars, "$ACCESS$ func $MethodName$<Result>(\n");
    printer->Indent();
    printer->Print(vars, "request: $request$,\n");
    printer->Print(vars, "serializer: some $serializer$,\n");
    printer->Print(vars, "deserializer: some $deserializer$,\n");
    printer->Print("options: GRPCCore.CallOptions = .defaults,\n");
    printer->Print(vars,
                   "onResponse handleResponse: @Sendable @escaping "
                   "($response$) async throws -> Result$CompletionBlock$\n");
    printer->Outdent();
    printer->Print(") async throws -> Result where Result: Sendable {\n");
    printer->Indent();
    printer->Print(vars, "try await self.client.$MethodType$(\n");
    printer->Indent();
    printer->Print("request: request,\n");
    printer->Print(
        vars,
        "descriptor: "
        "$SwiftServiceQualifiedName$.Method.$MethodName$.descriptor,\n");
    printer->Print("serializer: serializer,\n");
    printer->Print("deserializer: deserializer,\n");
    printer->Print("options: options,\n");
    printer->Print("onResponse: handleResponse\n");
    printer->Outdent();
    printer->Print(")\n");
    printer->Outdent();
    printer->Print("}\n\n");
  }

  printer->Outdent();
  printer->Print("}\n");
  printer->Outdent();
  printer->Print("}\n\n");
}

void GenerateClientDefaultImplementation(
    const grpc_generator::Service* service, grpc_generator::Printer* printer,
    std::map<grpc::string, grpc::string>* dictionary) {
  auto vars = *dictionary;
  EnforceOSVersion(printer);
  printer->Print(vars,
                 "extension $SwiftServiceQualifiedName$.ClientProtocol {\n");
  printer->Indent();

  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    auto name = method->name();
    vars["MethodName"] = name;

    vars["serializer"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     "FlatBuffersMessageSerializer");

    vars["deserializer"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     "FlatBuffersMessageDeserializer");

    if (method->ClientStreaming() || method->BidiStreaming()) {
      vars["request"] =
          GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                     method->get_output_type_name()),
                       "GRPCCore.StreamingClientRequest");
    } else {
      vars["request"] =
          GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                     method->get_output_type_name()),
                       "GRPCCore.ClientRequest");
    }

    if (method->ServerStreaming() || method->BidiStreaming()) {
      vars["response"] =
          GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                     method->get_output_type_name()),
                       "GRPCCore.StreamingClientResponse");
      vars["CompletionBlock"] = "";
    } else {
      vars["response"] =
          GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                     method->get_output_type_name()),
                       "GRPCCore.ClientResponse");
      vars["CompletionBlock"] = " = { response in try response.message }";
    }

    printer->Print(vars, "$ACCESS$ func $MethodName$<Result>(\n");
    printer->Indent();
    printer->Print(vars, "request: $request$,\n");
    printer->Print("options: GRPCCore.CallOptions = .defaults,\n");
    printer->Print(vars,
                   "onResponse handleResponse: @Sendable @escaping "
                   "($response$) async throws -> Result$CompletionBlock$\n");
    printer->Outdent();
    printer->Print(") async throws -> Result where Result: Sendable {\n");
    printer->Indent();
    printer->Print(vars, "try await self.$MethodName$(\n");
    printer->Indent();
    printer->Print("request: request,\n");
    printer->Print(vars, "serializer: $serializer$(),\n");
    printer->Print(vars, "deserializer: $deserializer$(),\n");
    printer->Print("options: options,\n");
    printer->Print("onResponse: handleResponse\n");
    printer->Outdent();
    printer->Print(")\n");
    printer->Outdent();
    printer->Print("}\n\n");
  }

  printer->Outdent();
  printer->Print("}\n\n");
}

void GenerateClientHelperMethods(
    const grpc_generator::Service* service, grpc_generator::Printer* printer,
    std::map<grpc::string, grpc::string>* dictionary) {
  auto vars = *dictionary;

  EnforceOSVersion(printer);
  printer->Print(vars,
                 "extension $SwiftServiceQualifiedName$.ClientProtocol {\n");
  printer->Indent();

  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    auto name = method->name();
    vars["MethodName"] = name;

    vars["serializer"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     "FlatBuffersMessageSerializer");

    vars["deserializer"] =
        GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                   method->get_output_type_name()),
                     "FlatBuffersMessageDeserializer");

    printer->Print(vars, "$ACCESS$ func $MethodName$<Result>(\n");
    printer->Indent();

    if (method->NoStreaming() || method->ServerStreaming()) {
      vars["request"] = GenerateGRPCMessage(QualifiedName(
          method->get_input_namespace_parts(), method->get_output_type_name()));
      printer->Print(vars, "_ message: $request$,\n");
    }

    printer->Print("metadata: GRPCCore.Metadata = [:],\n");
    printer->Print("options: GRPCCore.CallOptions = .defaults,\n");

    if (method->ClientStreaming() || method->BidiStreaming()) {
      vars["request"] = GenerateGRPCMessage(QualifiedName(
          method->get_input_namespace_parts(), method->get_output_type_name()));
      printer->Print(vars,
                     "requestProducer producer: @Sendable @escaping "
                     "(GRPCCore.RPCWriter<$request$>) async throws -> Void,\n");
    }

    if (method->NoStreaming() || method->ClientStreaming()) {
      vars["response"] = GenerateGRPCMessage(QualifiedName(
          method->get_input_namespace_parts(), method->get_output_type_name()));

      printer->Print(vars,
                     "onResponse handleResponse: @Sendable @escaping "
                     "(GRPCCore.ClientResponse<$response$>) async throws -> "
                     "Result = { try $0.message }\n");
    } else {
      vars["response"] = GenerateGRPCMessage(QualifiedName(
          method->get_input_namespace_parts(), method->get_output_type_name()));

      printer->Print(vars,
                     "onResponse handleResponse: @Sendable @escaping "
                     "(GRPCCore.StreamingClientResponse<$response$>) async "
                     "throws -> Result\n");
    }

    printer->Outdent();
    printer->Print(") async throws -> Result where Result: Sendable {\n");
    printer->Indent();

    if (method->ClientStreaming() || method->BidiStreaming()) {
      vars["request"] =
          GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                     method->get_output_type_name()),
                       "GRPCCore.StreamingClientRequest");
    } else {
      vars["request"] =
          GenerateType(QualifiedName(method->get_input_namespace_parts(),
                                     method->get_output_type_name()),
                       "GRPCCore.ClientRequest");
    }

    printer->Print(vars, "let request = $request$(\n");
    printer->Indent();
    if (method->NoStreaming() || method->ServerStreaming()) {
      printer->Print("message: message,\n");
      printer->Print("metadata: metadata\n");
    } else {
      printer->Print("metadata: metadata,\n");
      printer->Print("producer: producer\n");
    }

    printer->Outdent();
    printer->Print(")\n");

    printer->Print(vars, "return try await self.$MethodName$(\n");
    printer->Indent();
    printer->Print("request: request,\n");
    printer->Print("options: options,\n");
    printer->Print("onResponse: handleResponse\n");
    printer->Outdent();
    printer->Print(")\n");
    printer->Outdent();
    printer->Print("}\n\n");
  }

  printer->Outdent();
  printer->Print("}\n\n");
}

void GenerateClient(const grpc_generator::Service* service,
                    grpc_generator::Printer* printer,
                    std::map<grpc::string, grpc::string>* dictionary) {
  GenerateClientProtocols(service, printer, dictionary);
  GenerateClientStruct(service, printer, dictionary);
  GenerateClientDefaultImplementation(service, printer, dictionary);
  GenerateClientHelperMethods(service, printer, dictionary);
}

}  // namespace

grpc::string Generate(grpc_generator::File* file,
                      const grpc_generator::Service* service) {
  grpc::string output;
  std::map<grpc::string, grpc::string> vars;
  vars["PATH"] = file->package();
  if (!file->package().empty()) {
    vars["PATH"].append(".");
  }
  vars["SwiftServiceQualifiedName"] =
      QualifiedName(service->namespace_parts(), service->name());
  vars["ServiceQualifiedName"] =
      QualifiedName(service->namespace_parts(), service->name(), ".");
  vars["ServiceName"] = service->name();
  vars["ACCESS"] = service->is_internal() ? "internal" : "public";
  auto printer = file->CreatePrinter(&output);
  printer->Print(
      vars,
      "/// Usage: instantiate $ServiceQualifiedName$ServiceClient, then call "
      "methods of this protocol to make API calls.\n");
  GenerateCoders(&*printer);
  GenerateSharedContent(service, &*printer, &vars);
  printer->Print("\n");
  printer->Print(vars, "// MARK: $ServiceQualifiedName$ Server\n\n");
  GenerateService(service, &*printer, &vars);
  printer->Print("\n");
  printer->Print(vars, "// MARK: $ServiceQualifiedName$ Client\n\n");

  GenerateClient(service, &*printer, &vars);
  printer->Print("#endif\n");
  return output;
}

grpc::string GenerateHeader() {
  grpc::string code;
  code +=
      "/// The following code is generated by the Flatbuffers library which "
      "might not be in sync with grpc-swift\n";
  code +=
      "/// in case of an issue please open github issue, though it would be "
      "maintained\n";
  code += "\n";
  code += "// swiftlint:disable all\n";
  code += "// swiftformat:disable all\n";
  code += "\n";
  code += "#if !os(Windows) && compiler(>=6.0)\n";
  code += "import FlatBuffers\n";
  code += "import Foundation\n";
  code += "import GRPCCore\n";
  code += "import GRPCNIOTransportCore\n";
  code += "\n";
  return code;
}
}  // namespace grpc_swift_generator
