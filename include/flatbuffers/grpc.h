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

#include "flatbuffers/flatbuffers.h"
#include "grpc++/support/byte_buffer.h"
#include "grpc/byte_buffer_reader.h"

namespace flatbuffers {
namespace grpc {

enum AddRef { ADD_REF };
enum StealRef { STEAL_REF };

// Message is a typed wrapper around a buffer that manages the underlying
// `grpc_slice` and also provides flatbuffers-specific helpers such as `Verify`
// and `GetRoot`. Since it is backed by a `grpc_slice`, the underlying buffer
// is refcounted and ownership is be managed automatically.
template <class T>
class Message {
 public:
  inline Message() : slice_(grpc_empty_slice()) {}

  inline Message(grpc_slice slice, AddRef) : slice_(grpc_slice_ref(slice)) {}

  inline Message(grpc_slice slice, StealRef) : slice_(slice) {}

  inline Message(const Message &other) : slice_(grpc_slice_ref(other.slice_)) {}

  inline Message(Message &&other) : slice_(other.slice_) {
    other.slice_ = grpc_empty_slice();
  }

  inline Message &operator=(const Message &other) {
    slice_ = grpc_slice_ref(other.slice_);
    return *this;
  }

  inline Message &operator=(Message &&other) {
    slice_ = other.slice_;
    other.slice_ = grpc_empty_slice();
    return *this;
  }

  inline ~Message() { grpc_slice_unref(slice_); }

  inline const uint8_t *mutable_data() const {
    return const_cast<uint8_t*>(data());
  }

  inline const uint8_t *data() const {
    if (slice_.refcount != nullptr) {
      return slice_.data.refcounted.bytes;
    } else {
      return slice_.data.inlined.bytes;
    }
  }

  inline size_t size() const {
    if (slice_.refcount != nullptr) {
      return slice_.data.refcounted.length;
    } else {
      return slice_.data.inlined.length;
    }
  }

  inline bool Verify() const {
    Verifier verifier(data(), size());
    return verifier.VerifyBuffer<T>(nullptr);
  }

  inline T *GetMutableRoot() {
    return flatbuffers::GetMutableRoot<T>(mutable_data());
  }

  inline const T *GetRoot() const {
    return flatbuffers::GetRoot<T>(data());
  }

  // This is only intended for serializer use, or if you know what you're doing
  inline const grpc_slice &BorrowSlice() const { return slice_; }

 private:
  grpc_slice slice_;
};

class MessageBuilder;

// SliceAllocator is a gRPC-specific allocator that uses the `grpc_slice`
// refcounted slices to manage memory ownership. This makes it easy and
// efficient to transfer buffers to gRPC.
class SliceAllocator : public Allocator {
 public:
  inline SliceAllocator() : slice_(grpc_empty_slice()) {}

  SliceAllocator(const SliceAllocator &other) = delete;
  SliceAllocator &operator=(const SliceAllocator &other) = delete;

  virtual inline ~SliceAllocator() { grpc_slice_unref(slice_); }

  virtual inline uint8_t *allocate(size_t size) override {
    assert(GRPC_SLICE_IS_EMPTY(slice_));
    slice_ = grpc_slice_malloc(size);
    return GRPC_SLICE_START_PTR(slice_);
  }

  virtual inline void deallocate(uint8_t *p, size_t size) override {
    assert(p == GRPC_SLICE_START_PTR(slice_));
    assert(size == GRPC_SLICE_LENGTH(slice_));
    grpc_slice_unref(slice_);
    slice_ = grpc_empty_slice();
  }

  virtual inline uint8_t *reallocate_downward(uint8_t *old_p, size_t old_size,
                                              size_t new_size) override {
    assert(old_p == GRPC_SLICE_START_PTR(slice_));
    assert(old_size == GRPC_SLICE_LENGTH(slice_));
    assert(new_size > old_size);
    grpc_slice old_slice = slice_;
    grpc_slice new_slice = grpc_slice_malloc(new_size);
    uint8_t *new_p = GRPC_SLICE_START_PTR(new_slice);
    memcpy(new_p + (new_size - old_size), old_p, old_size);
    slice_ = new_slice;
    grpc_slice_unref(old_slice);
    return new_p;
  }

 private:
  inline grpc_slice &get_slice(uint8_t *p, size_t size) {
    assert(p == GRPC_SLICE_START_PTR(slice_));
    assert(size == GRPC_SLICE_LENGTH(slice_));
    return slice_;
  }

  grpc_slice slice_;

  friend class MessageBuilder;
};

// SliceAllocatorMember is a hack to ensure that the MessageBuilder's
// slice_allocator_ member is constructed before the FlatBufferBuilder, since
// the allocator is used in the FlatBufferBuilder ctor.
namespace detail {
struct SliceAllocatorMember {
  SliceAllocator slice_allocator_;
};
}

// MessageBuilder is a gRPC-specific FlatBufferBuilder that uses SliceAllocator
// to allocate gRPC buffers.
class MessageBuilder : private detail::SliceAllocatorMember,
                       public FlatBufferBuilder {
 public:
  explicit inline MessageBuilder(uoffset_t initial_size = 1024)
    : FlatBufferBuilder(initial_size, &slice_allocator_, false),
      initial_size_(initial_size) {}

  MessageBuilder(const MessageBuilder &other) = delete;
  MessageBuilder &operator=(const MessageBuilder &other) = delete;

  inline ~MessageBuilder() {}

  // GetMessage extracts the subslice of the buffer corresponding to the
  // flatbuffers-encoded region and wraps it in a `Message<T>` to handle buffer
  // ownership.
  template <class T>
  inline Message<T> GetMessage() {
    vector_downward &vec = GetVectorDownward();
    uint8_t *buf_data = vec.buf();     // pointer to memory
    size_t buf_size = vec.capacity();  // size of memory
    uint8_t *msg_data = vec.data();    // pointer to msg
    size_t msg_size = vec.size();      // size of msg

    // Do some sanity checks on data/size
    assert(msg_data >= msg_data);
    assert(msg_data + msg_size <= buf_data + buf_size);

    // Calculate offsets from the buffer start
    size_t begin = msg_data - buf_data;
    size_t end = begin + msg_size;

    // Get the slice we are working with (no refcount change)
    grpc_slice slice = slice_allocator_.get_slice(buf_data, buf_size);

    // Extract a subslice of the existing slice (increment refcount)
    grpc_slice subslice = grpc_slice_sub(slice, begin, end);

    // Wrap the subslice in a `Message<T>` (steal ref since incremented above)
    Message<T> msg(subslice, STEAL_REF);

    return msg;
  }

  template <class T>
  inline Message<T> ReleaseMessage() {
    Message<T> msg = GetMessage<T>();
    Reset(initial_size_, &slice_allocator_, false);
    return msg;
  }

 private:
  // SliceAllocator slice_allocator_;  // part of SliceAllocatorMember
  uoffset_t initial_size_;
};

template <class T>
class MessageVerifier {
 public:
  static inline ::grpc::Status Verify(const Message<T>& msg) {
    if (msg.Verify()) {
      return ::grpc::Status::OK;
    } else {
      // DATA_LOSS: "Unrecoverable data loss or corruption."
      return ::grpc::Status(::grpc::StatusCode::DATA_LOSS, "Message failed verification");
    }
  }
};

}  // namespace grpc
}  // namespace flatbuffers

namespace grpc {

template <class T>
class SerializationTraits<flatbuffers::grpc::Message<T>> {
 public:
  static grpc::Status Serialize(const flatbuffers::grpc::Message<T> &msg,
                                grpc_byte_buffer **buffer, bool *own_buffer) {
    // We are passed in a `Message<T>`, which is a wrapper around a
    // `grpc_slice`. We extract it here using `BorrowSlice()`. The const cast
    // is necesary because the `grpc_raw_byte_buffer_create` func expects
    // non-const slices in order to increment their refcounts.
    grpc_slice* slice = const_cast<grpc_slice *>(&msg.BorrowSlice());

    // Now use `grpc_raw_byte_buffer_create` to package the single slice into a
    // `grpc_byte_buffer`, incrementing the refcount in the process.
    *buffer = grpc_raw_byte_buffer_create(slice, 1);
    *own_buffer = true;
    return grpc::Status();
  }

  // Deserialize by pulling the
  static grpc::Status Deserialize(grpc_byte_buffer *buffer,
                                  flatbuffers::grpc::Message<T> *msg) {
    // Check if this is a single uncompressed slice.
    if ((buffer->type == GRPC_BB_RAW) &&
        (buffer->data.raw.compression == GRPC_COMPRESS_NONE) &&
        (buffer->data.raw.slice_buffer.count == 1)) {
      // If it is, then we can reference the `grpc_slice` directly.
      grpc_slice slice = buffer->data.raw.slice_buffer.slices[0];
      // We wrap a `Message<T>` around the slice, incrementing the refcount.
      *msg = flatbuffers::grpc::Message<T>(slice, flatbuffers::grpc::ADD_REF);
    } else {
      // Otherwise, we need to use `grpc_byte_buffer_reader_readall` to read
      // `buffer` into a single contiguous `grpc_slice`. The gRPC reader gives
      // us back a new slice with the refcount already incremented.
      grpc_byte_buffer_reader reader;
      grpc_byte_buffer_reader_init(&reader, buffer);
      grpc_slice slice = grpc_byte_buffer_reader_readall(&reader);
      grpc_byte_buffer_reader_destroy(&reader);
      // We wrap a `Message<T>` around the slice, but steal the reference
      *msg = flatbuffers::grpc::Message<T>(slice, flatbuffers::grpc::STEAL_REF);
    }
    return flatbuffers::grpc::MessageVerifier<T>::Verify(*msg);
  }
};

}  // namespace grpc;

#endif  // FLATBUFFERS_GRPC_H_
