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
#include "src/compiler/swift_generator.h"

namespace grpc_swift_generator {

grpc::string GenerateMessage(const grpc::string &name) {
  return "Message<" + name + ">";
}

// MARK: - Client

grpc::string GenerateClientFuncName(const grpc_generator::Method *method) {
  if (method->NoStreaming()) {
    return "$GenAccess$ func $MethodName$(_ request: $Input$"
           ", callOptions: CallOptions?$isNil$) -> UnaryCall<$Input$,$Output$>";
  }

  if (method->ClientStreaming()) {
    return "$GenAccess$ func $MethodName$"
           "(callOptions: CallOptions?$isNil$) -> "
           "ClientStreamingCall<$Input$,$Output$>";
  }

  if (method->ServerStreaming()) {
    return "$GenAccess$ func $MethodName$(_ request: $Input$"
           ", callOptions: CallOptions?$isNil$, handler: @escaping ($Output$"
           ") -> Void) -> ServerStreamingCall<$Input$, $Output$>";
  }
  return "$GenAccess$ func $MethodName$"
         "(callOptions: CallOptions?$isNil$, handler: @escaping ($Output$"
         ") -> Void) -> BidirectionalStreamingCall<$Input$, $Output$>";
}

grpc::string GenerateClientFuncBody(const grpc_generator::Method *method) {
  if (method->NoStreaming()) {
    return "return self.makeUnaryCall(path: "
           "\"/$PATH$$ServiceName$/$MethodName$\", request: request, "
           "callOptions: callOptions ?? self.defaultCallOptions)";
  }

  if (method->ClientStreaming()) {
    return "return self.makeClientStreamingCall(path: "
           "\"/$PATH$$ServiceName$/$MethodName$\", callOptions: callOptions ?? "
           "self.defaultCallOptions)";
  }

  if (method->ServerStreaming()) {
    return "return self.makeServerStreamingCall(path: "
           "\"/$PATH$$ServiceName$/$MethodName$\", request: request, "
           "callOptions: callOptions ?? self.defaultCallOptions, handler: "
           "handler)";
  }
  return "return self.makeBidirectionalStreamingCall(path: "
         "\"/$PATH$$ServiceName$/$MethodName$\", callOptions: callOptions ?? "
         "self.defaultCallOptions, handler: handler)";
}

void GenerateClientProtocol(const grpc_generator::Service *service,
                            grpc_generator::Printer *printer,
                            std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  printer->Print(vars, "$ACCESS$ protocol $ServiceName$Service {\n");
  vars["GenAccess"] = "";
  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    vars["Input"] = GenerateMessage(method->get_input_type_name());
    vars["Output"] = GenerateMessage(method->get_output_type_name());
    vars["MethodName"] = method->name();
    vars["isNil"] = "";
    printer->Print("\t");
    auto func = GenerateClientFuncName(method.get());
    printer->Print(vars, func.c_str());
    printer->Print("\n");
  }
  printer->Print("}\n\n");
}

void GenerateClientClass(const grpc_generator::Service *service,
                         grpc_generator::Printer *printer,
                         std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  printer->Print(vars,
                 "$ACCESS$ final class $ServiceName$ServiceClient: GRPCClient, "
                 "$ServiceName$Service {\n");
  printer->Print(vars, "\t$ACCESS$ let connection: ClientConnection\n");
  printer->Print(vars, "\t$ACCESS$ var defaultCallOptions: CallOptions\n");
  printer->Print("\n");
  printer->Print(vars,
                 "\t$ACCESS$ init(connection: ClientConnection, "
                 "defaultCallOptions: CallOptions = CallOptions()) {\n");
  printer->Print("\t\tself.connection = connection\n");
  printer->Print("\t\tself.defaultCallOptions = defaultCallOptions\n");
  printer->Print("\t}");
  printer->Print("\n");
  vars["GenAccess"] = "public";
  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    vars["Input"] = GenerateMessage(method->get_input_type_name());
    vars["Output"] = GenerateMessage(method->get_output_type_name());
    vars["MethodName"] = method->name();
    vars["isNil"] = " = nil";
    printer->Print("\n\t");
    auto func = GenerateClientFuncName(method.get());
    printer->Print(vars, func.c_str());
    printer->Print(" {\n");
    auto body = GenerateClientFuncBody(method.get());
    printer->Print("\t\t");
    printer->Print(vars, body.c_str());
    printer->Print("\n\t}\n");
  }
  printer->Print("}\n");
}

// MARK: - Server

grpc::string GenerateServerFuncName(const grpc_generator::Method *method) {
  if (method->NoStreaming()) {
    return "func $MethodName$(_ request: $Input$"
           ", context: StatusOnlyCallContext) -> EventLoopFuture<$Output$>";
  }

  if (method->ClientStreaming()) {
    return "func $MethodName$(context: UnaryResponseCallContext<$Output$>) -> "
           "EventLoopFuture<(StreamEvent<$Input$"
           ">) -> Void>";
  }

  if (method->ServerStreaming()) {
    return "func $MethodName$(request: $Input$"
           ", context: StreamingResponseCallContext<$Output$>) -> "
           "EventLoopFuture<GRPCStatus>";
  }
  return "func $MethodName$(context: StreamingResponseCallContext<$Output$>) "
         "-> EventLoopFuture<(StreamEvent<$Input$>) -> Void>";
}

grpc::string GenerateServerExtensionBody(const grpc_generator::Method *method) {
  grpc::string start = "\t\tcase \"$MethodName$\":\n\t\t";
  if (method->NoStreaming()) {
    return start +
           "return UnaryCallHandler(callHandlerContext: callHandlerContext) { "
           "context in"
           "\n\t\t\t"
           "return { request in"
           "\n\t\t\t\t"
           "self.$MethodName$(request, context: context)"
           "\n\t\t\t}"
           "\n\t\t}";
  }
  if (method->ClientStreaming()) {
    return start +
           "return ClientStreamingCallHandler(callHandlerContext: "
           "callHandlerContext) { context in"
           "\n\t\t\t"
           "return { request in"
           "\n\t\t\t\t"
           "self.$MethodName$(request: request, context: context)"
           "\n\t\t\t}"
           "\n\t\t}";
  }
  if (method->ServerStreaming()) {
    return start +
           "return ServerStreamingCallHandler(callHandlerContext: "
           "callHandlerContext) { context in"
           "\n\t\t\t"
           "return { request in"
           "\n\t\t\t\t"
           "self.$MethodName$(request: request, context: context)"
           "\n\t\t\t}"
           "\n\t\t}";
  }
  if (method->BidiStreaming()) {
    return start +
           "return BidirectionalStreamingCallHandler(callHandlerContext: "
           "callHandlerContext) { context in"
           "\n\t\t\t"
           "return { request in"
           "\n\t\t\t\t"
           "self.$MethodName$(request: request, context: context)"
           "\n\t\t\t}"
           "\n\t\t}";
  }
  return "";
}

void GenerateServerProtocol(const grpc_generator::Service *service,
                            grpc_generator::Printer *printer,
                            std::map<grpc::string, grpc::string> *dictonary) {
  auto vars = *dictonary;
  printer->Print(
      vars, "$ACCESS$ protocol $ServiceName$Provider: CallHandlerProvider {\n");
  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    vars["Input"] = GenerateMessage(method->get_input_type_name());
    vars["Output"] = GenerateMessage(method->get_output_type_name());
    vars["MethodName"] = method->name();
    printer->Print("\t");
    auto func = GenerateServerFuncName(method.get());
    printer->Print(vars, func.c_str());
    printer->Print("\n");
  }
  printer->Print("}\n\n");

  printer->Print(vars, "$ACCESS$ extension $ServiceName$Provider {\n");
  printer->Print(vars,
                 "\tvar serviceName: String { return "
                 "\"$PATH$$ServiceName$\" }\n");
  printer->Print(
      "\tfunc handleMethod(_ methodName: String, callHandlerContext: "
      "CallHandlerContext) -> GRPCCallHandler? {\n");
  printer->Print("\t\tswitch methodName {\n");
  for (auto it = 0; it < service->method_count(); it++) {
    auto method = service->method(it);
    vars["Input"] = GenerateMessage(method->get_input_type_name());
    vars["Output"] = GenerateMessage(method->get_output_type_name());
    vars["MethodName"] = method->name();
    auto body = GenerateServerExtensionBody(method.get());
    printer->Print(vars, body.c_str());
    printer->Print("\n");
  }
  printer->Print("\t\tdefault: return nil;\n");
  printer->Print("\t\t}\n");
  printer->Print("\t}\n\n");
  printer->Print("}\n\n");
}

grpc::string Generate(grpc_generator::File *file,
                      const grpc_generator::Service *service) {
  grpc::string output;
  std::map<grpc::string, grpc::string> vars;
  vars["PATH"] = file->package();
  if (!file->package().empty()) { vars["PATH"].append("."); }
  vars["ServiceName"] = service->name();
  vars["ACCESS"] = "public";
  auto printer = file->CreatePrinter(&output);
  printer->Print(vars,
                 "/// Usage: instantiate $ServiceName$ServiceClient, then call "
                 "methods of this protocol to make API calls.\n");
  GenerateClientProtocol(service, &*printer, &vars);
  GenerateClientClass(service, &*printer, &vars);
  printer->Print("\n");
  GenerateServerProtocol(service, &*printer, &vars);
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
  code += "import Foundation\n";
  code += "import GRPC\n";
  code += "import NIO\n";
  code += "import NIOHTTP1\n";
  code += "import FlatBuffers\n";
  code += "\n";
  code +=
      "public protocol GRPCFlatBufPayload: GRPCPayload, FlatBufferGRPCMessage "
      "{}\n";

  code += "public extension GRPCFlatBufPayload {\n";
  code += "    init(serializedByteBuffer: inout NIO.ByteBuffer) throws {\n";
  code +=
      "        self.init(byteBuffer: FlatBuffers.ByteBuffer(contiguousBytes: "
      "serializedByteBuffer.readableBytesView, count: "
      "serializedByteBuffer.readableBytes))\n";
  code += "    }\n";

  code += "    func serialize(into buffer: inout NIO.ByteBuffer) throws {\n";
  code +=
      "        let buf = UnsafeRawBufferPointer(start: self.rawPointer, count: "
      "Int(self.size))\n";
  code += "        buffer.writeBytes(buf)\n";
  code += "    }\n";
  code += "}\n";
  code += "extension Message: GRPCFlatBufPayload {}\n";
  return code;
}
}  // namespace grpc_swift_generator
