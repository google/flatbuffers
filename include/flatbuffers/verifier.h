/*
 * Copyright 2021 Google Inc. All rights reserved.
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

#ifndef FLATBUFFERS_VERIFIER_H_
#define FLATBUFFERS_VERIFIER_H_

#include "flatbuffers/base.h"
#include "flatbuffers/util.h"
#include "flatbuffers/vector.h"

namespace flatbuffers {

// Helper class to verify the integrity of a FlatBuffer
class Verifier FLATBUFFERS_FINAL_CLASS {
 public:
  Verifier(const uint8_t *buf, size_t buf_len, uoffset_t _max_depth = 64,
           uoffset_t _max_tables = 1000000, bool _check_alignment = true)
      : buf_(buf),
        size_(buf_len),
        depth_(0),
        max_depth_(_max_depth),
        num_tables_(0),
        max_tables_(_max_tables),
        upper_bound_(0),
        check_alignment_(_check_alignment),
        flex_reuse_tracker_(nullptr) {
    FLATBUFFERS_ASSERT(size_ < FLATBUFFERS_MAX_BUFFER_SIZE);
  }

  // Central location where any verification failures register.
  bool Check(bool ok) const {
    // clang-format off
    #ifdef FLATBUFFERS_DEBUG_VERIFICATION_FAILURE
      FLATBUFFERS_ASSERT(ok);
    #endif
    #ifdef FLATBUFFERS_TRACK_VERIFIER_BUFFER_SIZE
      if (!ok)
        upper_bound_ = 0;
    #endif
    // clang-format on
    return ok;
  }

  // Verify any range within the buffer.
  bool Verify(size_t elem, size_t elem_len) const {
    // clang-format off
    #ifdef FLATBUFFERS_TRACK_VERIFIER_BUFFER_SIZE
      auto upper_bound = elem + elem_len;
      if (upper_bound_ < upper_bound)
        upper_bound_ =  upper_bound;
    #endif
    // clang-format on
    return Check(elem_len < size_ && elem <= size_ - elem_len);
  }

  template<typename T> bool VerifyAlignment(size_t elem) const {
    return Check((elem & (sizeof(T) - 1)) == 0 || !check_alignment_);
  }

  // Verify a range indicated by sizeof(T).
  template<typename T> bool Verify(size_t elem) const {
    return VerifyAlignment<T>(elem) && Verify(elem, sizeof(T));
  }

  bool VerifyFromPointer(const uint8_t *p, size_t len) {
    auto o = static_cast<size_t>(p - buf_);
    return Verify(o, len);
  }

  // Verify relative to a known-good base pointer.
  bool Verify(const uint8_t *base, voffset_t elem_off, size_t elem_len) const {
    return Verify(static_cast<size_t>(base - buf_) + elem_off, elem_len);
  }

  template<typename T>
  bool Verify(const uint8_t *base, voffset_t elem_off) const {
    return Verify(static_cast<size_t>(base - buf_) + elem_off, sizeof(T));
  }

  // Verify a pointer (may be NULL) of a table type.
  template<typename T> bool VerifyTable(const T *table) {
    return !table || table->Verify(*this);
  }

  // Verify a pointer (may be NULL) of any vector type.
  template<typename T> bool VerifyVector(const Vector<T> *vec) const {
    return !vec || VerifyVectorOrString(reinterpret_cast<const uint8_t *>(vec),
                                        sizeof(T));
  }

  // Verify a pointer (may be NULL) of a vector to struct.
  template<typename T> bool VerifyVector(const Vector<const T *> *vec) const {
    return VerifyVector(reinterpret_cast<const Vector<T> *>(vec));
  }

  // Verify a pointer (may be NULL) to string.
  bool VerifyString(const String *str) const {
    size_t end;
    return !str || (VerifyVectorOrString(reinterpret_cast<const uint8_t *>(str),
                                         1, &end) &&
                    Verify(end, 1) &&           // Must have terminator
                    Check(buf_[end] == '\0'));  // Terminating byte must be 0.
  }

  // Common code between vectors and strings.
  bool VerifyVectorOrString(const uint8_t *vec, size_t elem_size,
                            size_t *end = nullptr) const {
    auto veco = static_cast<size_t>(vec - buf_);
    // Check we can read the size field.
    if (!Verify<uoffset_t>(veco)) return false;
    // Check the whole array. If this is a string, the byte past the array
    // must be 0.
    auto size = ReadScalar<uoffset_t>(vec);
    auto max_elems = FLATBUFFERS_MAX_BUFFER_SIZE / elem_size;
    if (!Check(size < max_elems))
      return false;  // Protect against byte_size overflowing.
    auto byte_size = sizeof(size) + elem_size * size;
    if (end) *end = veco + byte_size;
    return Verify(veco, byte_size);
  }

  // Special case for string contents, after the above has been called.
  bool VerifyVectorOfStrings(const Vector<Offset<String>> *vec) const {
    if (vec) {
      for (uoffset_t i = 0; i < vec->size(); i++) {
        if (!VerifyString(vec->Get(i))) return false;
      }
    }
    return true;
  }

  // Special case for table contents, after the above has been called.
  template<typename T> bool VerifyVectorOfTables(const Vector<Offset<T>> *vec) {
    if (vec) {
      for (uoffset_t i = 0; i < vec->size(); i++) {
        if (!vec->Get(i)->Verify(*this)) return false;
      }
    }
    return true;
  }

  __supress_ubsan__("unsigned-integer-overflow") bool VerifyTableStart(
      const uint8_t *table) {
    // Check the vtable offset.
    auto tableo = static_cast<size_t>(table - buf_);
    if (!Verify<soffset_t>(tableo)) return false;
    // This offset may be signed, but doing the subtraction unsigned always
    // gives the result we want.
    auto vtableo = tableo - static_cast<size_t>(ReadScalar<soffset_t>(table));
    // Check the vtable size field, then check vtable fits in its entirety.
    return VerifyComplexity() && Verify<voffset_t>(vtableo) &&
           VerifyAlignment<voffset_t>(ReadScalar<voffset_t>(buf_ + vtableo)) &&
           Verify(vtableo, ReadScalar<voffset_t>(buf_ + vtableo));
  }

  template<typename T>
  bool VerifyBufferFromStart(const char *identifier, size_t start) {
    if (identifier && !Check((size_ >= 2 * sizeof(flatbuffers::uoffset_t) &&
                              BufferHasIdentifier(buf_ + start, identifier)))) {
      return false;
    }

    // Call T::Verify, which must be in the generated code for this type.
    auto o = VerifyOffset(start);
    return o && reinterpret_cast<const T *>(buf_ + start + o)->Verify(*this)
    // clang-format off
    #ifdef FLATBUFFERS_TRACK_VERIFIER_BUFFER_SIZE
           && GetComputedSize()
    #endif
        ;
    // clang-format on
  }

  template<typename T>
  bool VerifyNestedFlatBuffer(const Vector<uint8_t> *buf,
                              const char *identifier) {
    if (!buf) return true;
    Verifier nested_verifier(buf->data(), buf->size());
    return nested_verifier.VerifyBuffer<T>(identifier);
  }

  // Verify this whole buffer, starting with root type T.
  template<typename T> bool VerifyBuffer() { return VerifyBuffer<T>(nullptr); }

  template<typename T> bool VerifyBuffer(const char *identifier) {
    return VerifyBufferFromStart<T>(identifier, 0);
  }

  template<typename T> bool VerifySizePrefixedBuffer(const char *identifier) {
    return Verify<uoffset_t>(0U) &&
           ReadScalar<uoffset_t>(buf_) == size_ - sizeof(uoffset_t) &&
           VerifyBufferFromStart<T>(identifier, sizeof(uoffset_t));
  }

  uoffset_t VerifyOffset(size_t start) const {
    if (!Verify<uoffset_t>(start)) return 0;
    auto o = ReadScalar<uoffset_t>(buf_ + start);
    // May not point to itself.
    if (!Check(o != 0)) return 0;
    // Can't wrap around / buffers are max 2GB.
    if (!Check(static_cast<soffset_t>(o) >= 0)) return 0;
    // Must be inside the buffer to create a pointer from it (pointer outside
    // buffer is UB).
    if (!Verify(start + o, 1)) return 0;
    return o;
  }

  uoffset_t VerifyOffset(const uint8_t *base, voffset_t start) const {
    return VerifyOffset(static_cast<size_t>(base - buf_) + start);
  }

  // Called at the start of a table to increase counters measuring data
  // structure depth and amount, and possibly bails out with false if
  // limits set by the constructor have been hit. Needs to be balanced
  // with EndTable().
  bool VerifyComplexity() {
    depth_++;
    num_tables_++;
    return Check(depth_ <= max_depth_ && num_tables_ <= max_tables_);
  }

  // Called at the end of a table to pop the depth count.
  bool EndTable() {
    depth_--;
    return true;
  }

  // Returns the message size in bytes
  size_t GetComputedSize() const {
    // clang-format off
    #ifdef FLATBUFFERS_TRACK_VERIFIER_BUFFER_SIZE
      uintptr_t size = upper_bound_;
      // Align the size to uoffset_t
      size = (size - 1 + sizeof(uoffset_t)) & ~(sizeof(uoffset_t) - 1);
      return (size > size_) ?  0 : size;
    #else
      // Must turn on FLATBUFFERS_TRACK_VERIFIER_BUFFER_SIZE for this to work.
      (void)upper_bound_;
      FLATBUFFERS_ASSERT(false);
      return 0;
    #endif
    // clang-format on
  }

  std::vector<uint8_t> *GetFlexReuseTracker() {
    return flex_reuse_tracker_;
  }

  void SetFlexReuseTracker(std::vector<uint8_t> *rt) {
    flex_reuse_tracker_ = rt;
  }

 private:
  const uint8_t *buf_;
  size_t size_;
  uoffset_t depth_;
  uoffset_t max_depth_;
  uoffset_t num_tables_;
  uoffset_t max_tables_;
  mutable size_t upper_bound_;
  bool check_alignment_;
  std::vector<uint8_t> *flex_reuse_tracker_;
};

}  // namespace flatbuffers

#endif  // FLATBUFFERS_VERIFIER_H_
