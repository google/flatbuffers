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

#ifndef GRPC_INTERNAL_COMPILER_PYTHON_PRIVATE_GENERATOR_H
#define GRPC_INTERNAL_COMPILER_PYTHON_PRIVATE_GENERATOR_H

#include <iostream>
#include <vector>

#include "src/compiler/python_generator.h"
#include "src/compiler/schema_interface.h"

namespace grpc_python_generator {

// Tucks all generator state in an anonymous namespace away from
// PythonGrpcGenerator and the header file, mostly to encourage future changes
// to not require updates to the grpcio-tools C++ code part. Assumes that it is
// only ever used from a single thread.
struct PrivateGenerator {
  const GeneratorConfiguration& config;
  const grpc_generator::File* file;

  PrivateGenerator(const GeneratorConfiguration& config,
                   const grpc_generator::File* file);

  grpc::string GetGrpcServices();

 private:
  void PrintPreamble(grpc_generator::Printer* out);
  void PrintBetaPreamble(grpc_generator::Printer* out);
  void PrintGAServices(grpc_generator::Printer* out);
  void PrintBetaServices(grpc_generator::Printer* out);

  void PrintAddServicerToServer(
      const grpc::string& package_qualified_service_name,
      const grpc_generator::Service* service, grpc_generator::Printer* out);
  void PrintServicer(const grpc_generator::Service* service,
                     grpc_generator::Printer* out);
  void PrintStub(const grpc::string& package_qualified_service_name,
                 const grpc_generator::Service* service,
                 grpc_generator::Printer* out);

  void PrintBetaServicer(const grpc_generator::Service* service,
                         grpc_generator::Printer* out);
  void PrintBetaServerFactory(
      const grpc::string& package_qualified_service_name,
      const grpc_generator::Service* service, grpc_generator::Printer* out);
  void PrintBetaStub(const grpc_generator::Service* service,
                     grpc_generator::Printer* out);
  void PrintBetaStubFactory(const grpc::string& package_qualified_service_name,
                            const grpc_generator::Service* service,
                            grpc_generator::Printer* out);
};

}  // namespace grpc_python_generator

#endif  // GRPC_INTERNAL_COMPILER_PYTHON_PRIVATE_GENERATOR_H
