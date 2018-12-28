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

#ifndef SRC_COMPILER_CONFIG_H
#define SRC_COMPILER_CONFIG_H

namespace grpc {}  // namespace grpc

namespace grpc_cpp_generator {

static const char* const kCppGeneratorMessageHeaderExt = "_generated.h";
static const char* const kCppGeneratorServiceHeaderExt = ".grpc.fb.h";

}  // namespace grpc_cpp_generator

#endif  // SRC_COMPILER_CONFIG_H
