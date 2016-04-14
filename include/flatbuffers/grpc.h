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

#ifndef FLATBUFFERS_GRPC_H_
#define FLATBUFFERS_GRPC_H_

// Helper functionality to glue FlatBuffers and GRPC.

#include "grpc++/support/byte_buffer.h"
#include "grpc/byte_buffer_reader.h"

namespace grpc {

template <class T>
class SerializationTraits<T, typename std::enable_if<std::is_base_of<
                                 flatbuffers::BufferRefBase, T>::value>::type> {
 public:
  // The type we're passing here is a BufferRef, which is already serialized
  // FlatBuffer data, which then gets passed to GRPC.
  static grpc::Status Serialize(const T& msg,
                                grpc_byte_buffer **buffer,
                                bool *own_buffer) {
    // TODO(wvo): make this work without copying.
    auto slice = gpr_slice_from_copied_buffer(
                   reinterpret_cast<const char *>(msg.buf), msg.len);
    *buffer = grpc_raw_byte_buffer_create(&slice, 1);
    *own_buffer = true;
    return grpc::Status();
  }

  // There is no de-serialization step in FlatBuffers, so we just receive
  // the data from GRPC.
  static grpc::Status Deserialize(grpc_byte_buffer *buffer,
                                  T *msg,
                                  int max_message_size) {
    // TODO(wvo): make this more efficient / zero copy when possible.
    auto len = grpc_byte_buffer_length(buffer);
    msg->buf = reinterpret_cast<uint8_t *>(malloc(len));
    msg->len = static_cast<flatbuffers::uoffset_t>(len);
    msg->must_free = true;
    uint8_t *current = msg->buf;
    grpc_byte_buffer_reader reader;
    grpc_byte_buffer_reader_init(&reader, buffer);
    gpr_slice slice;
    while (grpc_byte_buffer_reader_next(&reader, &slice)) {
      memcpy(current, GPR_SLICE_START_PTR(slice), GPR_SLICE_LENGTH(slice));
      current += GPR_SLICE_LENGTH(slice);
      gpr_slice_unref(slice);
    }
    GPR_ASSERT(current == msg->buf + msg->len);
    grpc_byte_buffer_reader_destroy(&reader);
    grpc_byte_buffer_destroy(buffer);
    return grpc::Status();
  }
};

}  // namespace grpc;

#endif  // FLATBUFFERS_GRPC_H_
