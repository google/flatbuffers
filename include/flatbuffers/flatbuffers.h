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

#ifndef FLATBUFFERS_H_
#define FLATBUFFERS_H_

#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>
#include <algorithm>

#if __cplusplus <= 199711L && \
    (!defined(_MSC_VER) || _MSC_VER < 1600) && \
    (!defined(__GNUC__) || \
      (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ < 40603))
  #error A C++11 compatible compiler is required for FlatBuffers.
  #error __cplusplus _MSC_VER __GNUC__  __GNUC_MINOR__  __GNUC_PATCHLEVEL__
#endif

// The wire format uses a little endian encoding (since that's efficient for
// the common platforms).
#if !defined(FLATBUFFERS_LITTLEENDIAN)
  #if defined(__GNUC__) || defined(__clang__)
    #ifdef __BIG_ENDIAN__
      #define FLATBUFFERS_LITTLEENDIAN 0
    #else
      #define FLATBUFFERS_LITTLEENDIAN 1
    #endif // __BIG_ENDIAN__
  #elif defined(_MSC_VER)
    #if defined(_M_PPC)
      #define FLATBUFFERS_LITTLEENDIAN 0
    #else
      #define FLATBUFFERS_LITTLEENDIAN 1
    #endif
  #else
    #error Unable to determine endianness, define FLATBUFFERS_LITTLEENDIAN.
  #endif
#endif // !defined(FLATBUFFERS_LITTLEENDIAN)

#define FLATBUFFERS_VERSION_MAJOR 1
#define FLATBUFFERS_VERSION_MINOR 0
#define FLATBUFFERS_VERSION_REVISION 0
#define FLATBUFFERS_STRING_EXPAND(X) #X
#define FLATBUFFERS_STRING(X) FLATBUFFERS_STRING_EXPAND(X)

namespace flatbuffers {

// Our default offset / size type, 32bit on purpose on 64bit systems.
// Also, using a consistent offset type maintains compatibility of serialized
// offset values between 32bit and 64bit systems.
typedef uint32_t uoffset_t;

// Signed offsets for references that can go in both directions.
typedef int32_t soffset_t;

// Offset/index used in v-tables, can be changed to uint8_t in
// format forks to save a bit of space if desired.
typedef uint16_t voffset_t;

typedef uintmax_t largest_scalar_t;

// Wrapper for uoffset_t to allow safe template specialization.
template<typename T> struct Offset {
  uoffset_t o;
  Offset() : o(0) {}
  Offset(uoffset_t _o) : o(_o) {}
  Offset<void> Union() const { return Offset<void>(o); }
};

inline void EndianCheck() {
  int endiantest = 1;
  // If this fails, see FLATBUFFERS_LITTLEENDIAN above.
  assert(*reinterpret_cast<char *>(&endiantest) == FLATBUFFERS_LITTLEENDIAN);
  (void)endiantest;
}

template<typename T> T EndianScalar(T t) {
  #if FLATBUFFERS_LITTLEENDIAN
    return t;
  #else
    #if defined(_MSC_VER)
      #pragma push_macro("__builtin_bswap16")
      #pragma push_macro("__builtin_bswap32")
      #pragma push_macro("__builtin_bswap64")
      #define __builtin_bswap16 _byteswap_ushort
      #define __builtin_bswap32 _byteswap_ulong
      #define __builtin_bswap64 _byteswap_uint64
    #endif
    // If you're on the few remaining big endian platforms, we make the bold
    // assumption you're also on gcc/clang, and thus have bswap intrinsics:
    if (sizeof(T) == 1) {   // Compile-time if-then's.
      return t;
    } else if (sizeof(T) == 2) {
      auto r = __builtin_bswap16(*reinterpret_cast<uint16_t *>(&t));
      return *reinterpret_cast<T *>(&r);
    } else if (sizeof(T) == 4) {
      auto r = __builtin_bswap32(*reinterpret_cast<uint32_t *>(&t));
      return *reinterpret_cast<T *>(&r);
    } else if (sizeof(T) == 8) {
      auto r = __builtin_bswap64(*reinterpret_cast<uint64_t *>(&t));
      return *reinterpret_cast<T *>(&r);
    } else {
      assert(0);
    }
    #if defined(_MSC_VER)
      #pragma pop_macro("__builtin_bswap16")
      #pragma pop_macro("__builtin_bswap32")
      #pragma pop_macro("__builtin_bswap64")
    #endif
  #endif
}

template<typename T> T ReadScalar(const void *p) {
  return EndianScalar(*reinterpret_cast<const T *>(p));
}

template<typename T> void WriteScalar(void *p, T t) {
  *reinterpret_cast<T *>(p) = EndianScalar(t);
}

template<typename T> size_t AlignOf() {
  #ifdef _MSC_VER
    return __alignof(T);
  #else
    return alignof(T);
  #endif
}

// When we read serialized data from memory, in the case of most scalars,
// we want to just read T, but in the case of Offset, we want to actually
// perform the indirection and return a pointer.
// The template specialization below does just that.
// It is wrapped in a struct since function templates can't overload on the
// return type like this.
// The typedef is for the convenience of callers of this function
// (avoiding the need for a trailing return decltype)
template<typename T> struct IndirectHelper {
  typedef T return_type;
  static const size_t element_stride = sizeof(T);
  static return_type Read(const uint8_t *p, uoffset_t i) {
    return EndianScalar((reinterpret_cast<const T *>(p))[i]);
  }
};
template<typename T> struct IndirectHelper<Offset<T>> {
  typedef const T *return_type;
  static const size_t element_stride = sizeof(uoffset_t);
  static return_type Read(const uint8_t *p, uoffset_t i) {
    p += i * sizeof(uoffset_t);
    return reinterpret_cast<return_type>(p + ReadScalar<uoffset_t>(p));
  }
};
template<typename T> struct IndirectHelper<const T *> {
  typedef const T *return_type;
  static const size_t element_stride = sizeof(T);
  static return_type Read(const uint8_t *p, uoffset_t i) {
    return reinterpret_cast<const T *>(p + i * sizeof(T));
  }
};

// An STL compatible iterator implementation for Vector below, effectively
// calling Get() for every element.
template<typename T, bool bConst>
struct VectorIterator : public
  std::iterator < std::input_iterator_tag,
  typename std::conditional < bConst,
  const typename IndirectHelper<T>::return_type,
  typename IndirectHelper<T>::return_type > ::type, uoffset_t > {

  typedef std::iterator<std::input_iterator_tag,
    typename std::conditional<bConst,
    const typename IndirectHelper<T>::return_type,
    typename IndirectHelper<T>::return_type>::type, uoffset_t> super_type;

public:
  VectorIterator(const uint8_t *data, uoffset_t i) :
      data_(data + IndirectHelper<T>::element_stride * i) {};
  VectorIterator(const VectorIterator &other) : data_(other.data_) {}
  VectorIterator(VectorIterator &&other) : data_(std::move(other.data_)) {}

  VectorIterator &operator=(const VectorIterator &other) {
    data_ = other.data_;
    return *this;
  }

  VectorIterator &operator=(VectorIterator &&other) {
    data_ = other.data_;
    return *this;
  }

  bool operator==(const VectorIterator& other) const {
    return data_ == other.data_;
  }

  bool operator!=(const VectorIterator& other) const {
    return data_ != other.data_;
  }

  ptrdiff_t operator-(const VectorIterator& other) const {
    return (data_ - other.data_) / IndirectHelper<T>::element_stride;
  }

  typename super_type::value_type operator *() const {
    return IndirectHelper<T>::Read(data_, 0);
  }

  typename super_type::value_type operator->() const {
    return IndirectHelper<T>::Read(data_, 0);
  }

  VectorIterator &operator++() {
    data_ += IndirectHelper<T>::element_stride;
    return *this;
  }

  VectorIterator operator++(int) {
    VectorIterator temp(data_);
    data_ += IndirectHelper<T>::element_stride;
    return temp;
  }

private:
  const uint8_t *data_;
};

// This is used as a helper type for accessing vectors.
// Vector::data() assumes the vector elements start after the length field.
template<typename T> class Vector {
public:
  typedef VectorIterator<T, false> iterator;
  typedef VectorIterator<T, true> const_iterator;

  uoffset_t size() const { return EndianScalar(length_); }

  // Deprecated: use size(). Here for backwards compatibility.
  uoffset_t Length() const { return size(); }

  typedef typename IndirectHelper<T>::return_type return_type;

  return_type Get(uoffset_t i) const {
    assert(i < size());
    return IndirectHelper<T>::Read(Data(), i);
  }

  // If this is a Vector of enums, T will be its storage type, not the enum
  // type. This function makes it convenient to retrieve value with enum
  // type E.
  template<typename E> E GetEnum(uoffset_t i) const {
    return static_cast<E>(Get(i));
  }

  const void *GetStructFromOffset(size_t o) const {
    return reinterpret_cast<const void *>(Data() + o);
  }

  iterator begin() { return iterator(Data(), 0); }
  const_iterator begin() const { return const_iterator(Data(), 0); }

  iterator end() { return iterator(Data(), length_); }
  const_iterator end() const { return const_iterator(Data(), length_); }

  // The raw data in little endian format. Use with care.
  const uint8_t *Data() const {
    return reinterpret_cast<const uint8_t *>(&length_ + 1);
  }

protected:
  // This class is only used to access pre-existing data. Don't ever
  // try to construct these manually.
  Vector();

  uoffset_t length_;
};

struct String : public Vector<char> {
  const char *c_str() const { return reinterpret_cast<const char *>(Data()); }
};

// Simple indirection for buffer allocation, to allow this to be overridden
// with custom allocation (see the FlatBufferBuilder constructor).
class simple_allocator {
 public:
  virtual uint8_t *allocate(size_t size) const { return new uint8_t[size]; }
  virtual void deallocate(uint8_t *p) const { delete[] p; }
};

// This is a minimal replication of std::vector<uint8_t> functionality,
// except growing from higher to lower addresses. i.e push_back() inserts data
// in the lowest address in the vector.
class vector_downward {
 public:
  explicit vector_downward(size_t initial_size,
                           const simple_allocator &allocator)
    : reserved_(initial_size),
      buf_(allocator.allocate(reserved_)),
      cur_(buf_ + reserved_),
      allocator_(allocator) {
    assert((initial_size & (sizeof(largest_scalar_t) - 1)) == 0);
  }

  ~vector_downward() { allocator_.deallocate(buf_); }

  void clear() { cur_ = buf_ + reserved_; }

  size_t growth_policy(size_t bytes) {
    return (bytes / 2) & ~(sizeof(largest_scalar_t) - 1);
  }

  uint8_t *make_space(size_t len) {
    if (buf_ > cur_ - len) {
      auto old_size = size();
      reserved_ += std::max(len, growth_policy(reserved_));
      auto new_buf = allocator_.allocate(reserved_);
      auto new_cur = new_buf + reserved_ - old_size;
      memcpy(new_cur, cur_, old_size);
      cur_ = new_cur;
      allocator_.deallocate(buf_);
      buf_ = new_buf;
    }
    cur_ -= len;
    // Beyond this, signed offsets may not have enough range:
    // (FlatBuffers > 2GB not supported).
    assert(size() < (1UL << (sizeof(soffset_t) * 8 - 1)) - 1);
    return cur_;
  }

  uoffset_t size() const {
    return static_cast<uoffset_t>(reserved_ - (cur_ - buf_));
  }

  uint8_t *data() const { return cur_; }

  uint8_t *data_at(size_t offset) { return buf_ + reserved_ - offset; }

  // push() & fill() are most frequently called with small byte counts (<= 4),
  // which is why we're using loops rather than calling memcpy/memset.
  void push(const uint8_t *bytes, size_t num) {
    auto dest = make_space(num);
    for (size_t i = 0; i < num; i++) dest[i] = bytes[i];
  }

  void fill(size_t zero_pad_bytes) {
    auto dest = make_space(zero_pad_bytes);
    for (size_t i = 0; i < zero_pad_bytes; i++) dest[i] = 0;
  }

  void pop(size_t bytes_to_remove) { cur_ += bytes_to_remove; }

 private:
  // You shouldn't really be copying instances of this class.
  vector_downward(const vector_downward &);
  vector_downward &operator=(const vector_downward &);

  size_t reserved_;
  uint8_t *buf_;
  uint8_t *cur_;  // Points at location between empty (below) and used (above).
  const simple_allocator &allocator_;
};

// Converts a Field ID to a virtual table offset.
inline voffset_t FieldIndexToOffset(voffset_t field_id) {
  // Should correspond to what EndTable() below builds up.
  const int fixed_fields = 2;  // Vtable size and Object Size.
  return (field_id + fixed_fields) * sizeof(voffset_t);
}

// Computes how many bytes you'd have to pad to be able to write an
// "scalar_size" scalar if the buffer had grown to "buf_size" (downwards in
// memory).
inline size_t PaddingBytes(size_t buf_size, size_t scalar_size) {
  return ((~buf_size) + 1) & (scalar_size - 1);
}

// Helper class to hold data needed in creation of a flat buffer.
// To serialize data, you typically call one of the Create*() functions in
// the generated code, which in turn call a sequence of StartTable/PushElement/
// AddElement/EndTable, or the builtin CreateString/CreateVector functions.
// Do this is depth-first order to build up a tree to the root.
// Finish() wraps up the buffer ready for transport.
class FlatBufferBuilder {
 public:
  explicit FlatBufferBuilder(uoffset_t initial_size = 1024,
                             const simple_allocator *allocator = nullptr)
      : buf_(initial_size, allocator ? *allocator : default_allocator),
        minalign_(1), force_defaults_(false) {
    offsetbuf_.reserve(16);  // Avoid first few reallocs.
    vtables_.reserve(16);
    EndianCheck();
  }

  // Reset all the state in this FlatBufferBuilder so it can be reused
  // to construct another buffer.
  void Clear() {
    buf_.clear();
    offsetbuf_.clear();
    vtables_.clear();
  }

  // The current size of the serialized buffer, counting from the end.
  uoffset_t GetSize() const { return buf_.size(); }

  // Get the serialized buffer (after you call Finish()).
  uint8_t *GetBufferPointer() const { return buf_.data(); }

  void ForceDefaults(bool fd) { force_defaults_ = fd; }

  void Pad(size_t num_bytes) { buf_.fill(num_bytes); }

  void Align(size_t elem_size) {
    if (elem_size > minalign_) minalign_ = elem_size;
    buf_.fill(PaddingBytes(buf_.size(), elem_size));
  }

  void PushBytes(const uint8_t *bytes, size_t size) {
    buf_.push(bytes, size);
  }

  void PopBytes(size_t amount) { buf_.pop(amount); }

  template<typename T> void AssertScalarT() {
    // The code assumes power of 2 sizes and endian-swap-ability.
    static_assert(std::is_scalar<T>::value
        // The Offset<T> type is essentially a scalar but fails is_scalar.
        || sizeof(T) == sizeof(Offset<void>),
           "T must be a scalar type");
  }

  // Write a single aligned scalar to the buffer
  template<typename T> uoffset_t PushElement(T element) {
    AssertScalarT<T>();
    T litle_endian_element = EndianScalar(element);
    Align(sizeof(T));
    PushBytes(reinterpret_cast<uint8_t *>(&litle_endian_element), sizeof(T));
    return GetSize();
  }

  template<typename T> uoffset_t PushElement(Offset<T> off) {
    // Special case for offsets: see ReferTo below.
    return PushElement(ReferTo(off.o));
  }

  // When writing fields, we track where they are, so we can create correct
  // vtables later.
  void TrackField(voffset_t field, uoffset_t off) {
    FieldLoc fl = { off, field };
    offsetbuf_.push_back(fl);
  }

  // Like PushElement, but additionally tracks the field this represents.
  template<typename T> void AddElement(voffset_t field, T e, T def) {
    // We don't serialize values equal to the default.
    if (e == def && !force_defaults_) return;
    auto off = PushElement(e);
    TrackField(field, off);
  }

  template<typename T> void AddOffset(voffset_t field, Offset<T> off) {
    if (!off.o) return;  // An offset of 0 means NULL, don't store.
    AddElement(field, ReferTo(off.o), static_cast<uoffset_t>(0));
  }

  template<typename T> void AddStruct(voffset_t field, const T *structptr) {
    if (!structptr) return;  // Default, don't store.
    Align(AlignOf<T>());
    PushBytes(reinterpret_cast<const uint8_t *>(structptr), sizeof(T));
    TrackField(field, GetSize());
  }

  void AddStructOffset(voffset_t field, uoffset_t off) {
    TrackField(field, off);
  }

  // Offsets initially are relative to the end of the buffer (downwards).
  // This function converts them to be relative to the current location
  // in the buffer (when stored here), pointing upwards.
  uoffset_t ReferTo(uoffset_t off) {
    Align(sizeof(uoffset_t));  // To ensure GetSize() below is correct.
    assert(off <= GetSize());  // Must refer to something already in buffer.
    return GetSize() - off + sizeof(uoffset_t);
  }

  void NotNested() {
    // If you hit this, you're trying to construct an object when another
    // hasn't finished yet.
    assert(!offsetbuf_.size());
  }

  // From generated code (or from the parser), we call StartTable/EndTable
  // with a sequence of AddElement calls in between.
  uoffset_t StartTable() {
    NotNested();
    return GetSize();
  }

  // This finishes one serialized object by generating the vtable if it's a
  // table, comparing it against existing vtables, and writing the
  // resulting vtable offset.
  uoffset_t EndTable(uoffset_t start, voffset_t numfields) {
    // Write the vtable offset, which is the start of any Table.
    // We fill it's value later.
    auto vtableoffsetloc = PushElement<uoffset_t>(0);
    // Write a vtable, which consists entirely of voffset_t elements.
    // It starts with the number of offsets, followed by a type id, followed
    // by the offsets themselves. In reverse:
    buf_.fill(numfields * sizeof(voffset_t));
    auto table_object_size = vtableoffsetloc - start;
    assert(table_object_size < 0x10000);  // Vtable use 16bit offsets.
    PushElement<voffset_t>(static_cast<voffset_t>(table_object_size));
    PushElement<voffset_t>(FieldIndexToOffset(numfields));
    // Write the offsets into the table
    for (auto field_location = offsetbuf_.begin();
              field_location != offsetbuf_.end();
            ++field_location) {
      auto pos = static_cast<voffset_t>(vtableoffsetloc - field_location->off);
      // If this asserts, it means you've set a field twice.
      assert(!ReadScalar<voffset_t>(buf_.data() + field_location->id));
      WriteScalar<voffset_t>(buf_.data() + field_location->id, pos);
    }
    offsetbuf_.clear();
    auto vt1 = reinterpret_cast<voffset_t *>(buf_.data());
    auto vt1_size = ReadScalar<voffset_t>(vt1);
    auto vt_use = GetSize();
    // See if we already have generated a vtable with this exact same
    // layout before. If so, make it point to the old one, remove this one.
    for (auto it = vtables_.begin(); it != vtables_.end(); ++it) {
      if (memcmp(buf_.data_at(*it), vt1, vt1_size)) continue;
      vt_use = *it;
      buf_.pop(GetSize() - vtableoffsetloc);
      break;
    }
    // If this is a new vtable, remember it.
    if (vt_use == GetSize()) {
      vtables_.push_back(vt_use);
    }
    // Fill the vtable offset we created above.
    // The offset points from the beginning of the object to where the
    // vtable is stored.
    // Offsets default direction is downward in memory for future format
    // flexibility (storing all vtables at the start of the file).
    WriteScalar(buf_.data_at(vtableoffsetloc),
                static_cast<soffset_t>(vt_use) -
                  static_cast<soffset_t>(vtableoffsetloc));
    return vtableoffsetloc;
  }

  // This checks a required field has been set in a given table that has
  // just been constructed.
  template<typename T> void Required(Offset<T> table, voffset_t field) {
    auto table_ptr = buf_.data_at(table.o);
    auto vtable_ptr = table_ptr - ReadScalar<uoffset_t>(table_ptr);
    bool ok = ReadScalar<voffset_t>(vtable_ptr + field) != 0;
    // If this fails, the caller will show what field needs to be set.
    assert(ok);
    (void)ok;
  }

  uoffset_t StartStruct(size_t alignment) {
    Align(alignment);
    return GetSize();
  }

  uoffset_t EndStruct() { return GetSize(); }

  void ClearOffsets() { offsetbuf_.clear(); }

  // Aligns such that when "len" bytes are written, an object can be written
  // after it with "alignment" without padding.
  void PreAlign(size_t len, size_t alignment) {
    buf_.fill(PaddingBytes(GetSize() + len, alignment));
  }
  template<typename T> void PreAlign(size_t len) {
    AssertScalarT<T>();
    PreAlign(len, sizeof(T));
  }

  // Functions to store strings, which are allowed to contain any binary data.
  Offset<String> CreateString(const char *str, size_t len) {
    NotNested();
    PreAlign<uoffset_t>(len + 1);  // Always 0-terminated.
    buf_.fill(1);
    PushBytes(reinterpret_cast<const uint8_t *>(str), len);
    PushElement(static_cast<uoffset_t>(len));
    return Offset<String>(GetSize());
  }

  Offset<String> CreateString(const char *str) {
    return CreateString(str, strlen(str));
  }

  Offset<String> CreateString(const std::string &str) {
    return CreateString(str.c_str(), str.length());
  }

  uoffset_t EndVector(size_t len) {
    return PushElement(static_cast<uoffset_t>(len));
  }

  void StartVector(size_t len, size_t elemsize) {
    PreAlign<uoffset_t>(len * elemsize);
    PreAlign(len * elemsize, elemsize);  // Just in case elemsize > uoffset_t.
  }

  uint8_t *ReserveElements(size_t len, size_t elemsize) {
    return buf_.make_space(len * elemsize);
  }

  template<typename T> Offset<Vector<T>> CreateVector(const T *v, size_t len) {
    NotNested();
    StartVector(len, sizeof(T));
    for (auto i = len; i > 0; ) {
      PushElement(v[--i]);
    }
    return Offset<Vector<T>>(EndVector(len));
  }

  // Specialized version for non-copying use cases. Data to be written later.
  // After calling this function, GetBufferPointer() can be cast to the
  // corresponding Vector<> type to write the data (through Data()).
  template<typename T> Offset<Vector<T>> CreateUninitializedVector(size_t len) {
    NotNested();
    StartVector(len, sizeof(T));
    buf_.make_space(len * sizeof(T));
    return Offset<Vector<T>>(EndVector(len));
  }

  template<typename T> Offset<Vector<T>> CreateVector(const std::vector<T> &v){
    return CreateVector(v.data(), v.size());
  }

  template<typename T> Offset<Vector<const T *>> CreateVectorOfStructs(
                                                      const T *v, size_t len) {
    NotNested();
    StartVector(len * sizeof(T) / AlignOf<T>(), AlignOf<T>());
    PushBytes(reinterpret_cast<const uint8_t *>(v), sizeof(T) * len);
    return Offset<Vector<const T *>>(EndVector(len));
  }

  template<typename T> Offset<Vector<const T *>> CreateVectorOfStructs(
                                                     const std::vector<T> &v) {
    return CreateVectorOfStructs(v.data(), v.size());
  }

  static const size_t kFileIdentifierLength = 4;

  // Finish serializing a buffer by writing the root offset.
  // If a file_identifier is given, the buffer will be prefix with a standard
  // FlatBuffers file header.
  template<typename T> void Finish(Offset<T> root,
                                   const char *file_identifier = nullptr) {
    // This will cause the whole buffer to be aligned.
    PreAlign(sizeof(uoffset_t) + (file_identifier ? kFileIdentifierLength : 0),
             minalign_);
    if (file_identifier) {
      assert(strlen(file_identifier) == kFileIdentifierLength);
      buf_.push(reinterpret_cast<const uint8_t *>(file_identifier),
                kFileIdentifierLength);
    }
    PushElement(ReferTo(root.o));  // Location of root.
  }

 private:
  // You shouldn't really be copying instances of this class.
  FlatBufferBuilder(const FlatBufferBuilder &);
  FlatBufferBuilder &operator=(const FlatBufferBuilder &);

  struct FieldLoc {
    uoffset_t off;
    voffset_t id;
  };

  simple_allocator default_allocator;

  vector_downward buf_;

  // Accumulating offsets of table members while it is being built.
  std::vector<FieldLoc> offsetbuf_;

  std::vector<uoffset_t> vtables_;  // todo: Could make this into a map?

  size_t minalign_;

  bool force_defaults_;  // Serialize values equal to their defaults anyway.
};

// Helper to get a typed pointer to the root object contained in the buffer.
template<typename T> const T *GetRoot(const void *buf) {
  EndianCheck();
  return reinterpret_cast<const T *>(reinterpret_cast<const uint8_t *>(buf) +
    EndianScalar(*reinterpret_cast<const uoffset_t *>(buf)));
}

// Helper to see if the identifier in a buffer has the expected value.
inline bool BufferHasIdentifier(const void *buf, const char *identifier) {
  return strncmp(reinterpret_cast<const char *>(buf) + sizeof(uoffset_t),
                 identifier, FlatBufferBuilder::kFileIdentifierLength) == 0;
}

// Helper class to verify the integrity of a FlatBuffer
class Verifier {
 public:
  Verifier(const uint8_t *buf, size_t buf_len, size_t _max_depth = 64,
           size_t _max_tables = 1000000)
    : buf_(buf), end_(buf + buf_len), depth_(0), max_depth_(_max_depth),
      num_tables_(0), max_tables_(_max_tables)
    {}

  // Central location where any verification failures register.
  bool Check(bool ok) const {
    #ifdef FLATBUFFERS_DEBUG_VERIFICATION_FAILURE
      assert(ok);
    #endif
    return ok;
  }

  // Verify any range within the buffer.
  bool Verify(const void *elem, size_t elem_len) const {
    return Check(elem >= buf_ && elem <= end_ - elem_len);
  }

  // Verify a range indicated by sizeof(T).
  template<typename T> bool Verify(const void *elem) const {
    return Verify(elem, sizeof(T));
  }

  // Verify a pointer (may be NULL) of a table type.
  template<typename T> bool VerifyTable(const T *table) {
    return !table || table->Verify(*this);
  }

  // Verify a pointer (may be NULL) of any vector type.
  template<typename T> bool Verify(const Vector<T> *vec) const {
    const uint8_t *end;
    return !vec ||
           VerifyVector(reinterpret_cast<const uint8_t *>(vec), sizeof(T),
                        &end);
  }

  // Verify a pointer (may be NULL) to string.
  bool Verify(const String *str) const {
    const uint8_t *end;
    return !str ||
           (VerifyVector(reinterpret_cast<const uint8_t *>(str), 1, &end) &&
            Verify(end, 1) &&      // Must have terminator
            Check(*end == '\0'));  // Terminating byte must be 0.
  }

  // Common code between vectors and strings.
  bool VerifyVector(const uint8_t *vec, size_t elem_size,
                    const uint8_t **end) const {
    // Check we can read the size field.
    if (!Verify<uoffset_t>(vec)) return false;
    // Check the whole array. If this is a string, the byte past the array
    // must be 0.
    auto size = ReadScalar<uoffset_t>(vec);
    auto byte_size = sizeof(size) + elem_size * size;
    *end = vec + byte_size;
    return Verify(vec, byte_size);
  }

  // Special case for string contents, after the above has been called.
  bool VerifyVectorOfStrings(const Vector<Offset<String>> *vec) const {
      if (vec) {
        for (uoffset_t i = 0; i < vec->size(); i++) {
          if (!Verify(vec->Get(i))) return false;
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

  // Verify this whole buffer, starting with root type T.
  template<typename T> bool VerifyBuffer() {
    // Call T::Verify, which must be in the generated code for this type.
    return Verify<uoffset_t>(buf_) &&
      reinterpret_cast<const T *>(buf_ + ReadScalar<uoffset_t>(buf_))->
        Verify(*this);
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

 private:
  const uint8_t *buf_;
  const uint8_t *end_;
  size_t depth_;
  size_t max_depth_;
  size_t num_tables_;
  size_t max_tables_;
};

// "structs" are flat structures that do not have an offset table, thus
// always have all members present and do not support forwards/backwards
// compatible extensions.

class Struct {
 public:
  template<typename T> T GetField(uoffset_t o) const {
    return ReadScalar<T>(&data_[o]);
  }

  template<typename T> T GetPointer(uoffset_t o) const {
    auto p = &data_[o];
    return reinterpret_cast<T>(p + ReadScalar<uoffset_t>(p));
  }

  template<typename T> T GetStruct(uoffset_t o) const {
    return reinterpret_cast<T>(&data_[o]);
  }

 private:
  uint8_t data_[1];
};

// "tables" use an offset table (possibly shared) that allows fields to be
// omitted and added at will, but uses an extra indirection to read.
class Table {
 public:
  // This gets the field offset for any of the functions below it, or 0
  // if the field was not present.
  voffset_t GetOptionalFieldOffset(voffset_t field) const {
    // The vtable offset is always at the start.
    auto vtable = data_ - ReadScalar<soffset_t>(data_);
    // The first element is the size of the vtable (fields + type id + itself).
    auto vtsize = ReadScalar<voffset_t>(vtable);
    // If the field we're accessing is outside the vtable, we're reading older
    // data, so it's the same as if the offset was 0 (not present).
    return field < vtsize ? ReadScalar<voffset_t>(vtable + field) : 0;
  }

  template<typename T> T GetField(voffset_t field, T defaultval) const {
    auto field_offset = GetOptionalFieldOffset(field);
    return field_offset ? ReadScalar<T>(data_ + field_offset) : defaultval;
  }

  template<typename P> P GetPointer(voffset_t field) const {
    auto field_offset = GetOptionalFieldOffset(field);
    auto p = data_ + field_offset;
    return field_offset
      ? reinterpret_cast<P>(p + ReadScalar<uoffset_t>(p))
      : nullptr;
  }

  template<typename P> P GetStruct(voffset_t field) const {
    auto field_offset = GetOptionalFieldOffset(field);
    return field_offset ? reinterpret_cast<P>(data_ + field_offset) : nullptr;
  }

  template<typename T> void SetField(voffset_t field, T val) {
    auto field_offset = GetOptionalFieldOffset(field);
    // If this asserts, you're trying to set a field that's not there
    // (or should we return a bool instead?).
    // check if it exists first using CheckField()
    assert(field_offset);
    WriteScalar(data_ + field_offset, val);
  }

  bool CheckField(voffset_t field) const {
    return GetOptionalFieldOffset(field) != 0;
  }

  // Verify the vtable of this table.
  // Call this once per table, followed by VerifyField once per field.
  bool VerifyTableStart(Verifier &verifier) const {
    // Check the vtable offset.
    if (!verifier.Verify<soffset_t>(data_)) return false;
    auto vtable = data_ - ReadScalar<soffset_t>(data_);
    // Check the vtable size field, then check vtable fits in its entirety.
    return verifier.VerifyComplexity() &&
           verifier.Verify<voffset_t>(vtable) &&
           verifier.Verify(vtable, ReadScalar<voffset_t>(vtable));
  }

  // Verify a particular field.
  template<typename T> bool VerifyField(const Verifier &verifier,
                                        voffset_t field) const {
    // Calling GetOptionalFieldOffset should be safe now thanks to
    // VerifyTable().
    auto field_offset = GetOptionalFieldOffset(field);
    // Check the actual field.
    return !field_offset || verifier.Verify<T>(data_ + field_offset);
  }

  // VerifyField for required fields.
  template<typename T> bool VerifyFieldRequired(const Verifier &verifier,
                                        voffset_t field) const {
    auto field_offset = GetOptionalFieldOffset(field);
    return verifier.Check(field_offset != 0) &&
           verifier.Verify<T>(data_ + field_offset);
  }

 private:
  // private constructor & copy constructor: you obtain instances of this
  // class by pointing to existing data only
  Table();
  Table(const Table &other);

  uint8_t data_[1];
};

// Utility function for reverse lookups on the EnumNames*() functions
// (in the generated C++ code)
// names must be NULL terminated.
inline int LookupEnum(const char **names, const char *name) {
  for (const char **p = names; *p; p++)
    if (!strcmp(*p, name))
      return static_cast<int>(p - names);
  return -1;
}

// These macros allow us to layout a struct with a guarantee that they'll end
// up looking the same on different compilers and platforms.
// It does this by disallowing the compiler to do any padding, and then
// does padding itself by inserting extra padding fields that make every
// element aligned to its own size.
// Additionally, it manually sets the alignment of the struct as a whole,
// which is typically its largest element, or a custom size set in the schema
// by the force_align attribute.
// These are used in the generated code only.

#if defined(_MSC_VER)
  #define MANUALLY_ALIGNED_STRUCT(alignment) \
    __pragma(pack(1)); \
    struct __declspec(align(alignment))
  #define STRUCT_END(name, size) \
    __pragma(pack()); \
    static_assert(sizeof(name) == size, "compiler breaks packing rules")
#elif defined(__GNUC__) || defined(__clang__)
  #define MANUALLY_ALIGNED_STRUCT(alignment) \
    _Pragma("pack(1)") \
    struct __attribute__((aligned(alignment)))
  #define STRUCT_END(name, size) \
    _Pragma("pack()") \
    static_assert(sizeof(name) == size, "compiler breaks packing rules")
#else
  #error Unknown compiler, please define structure alignment macros
#endif

// String which identifies the current version of FlatBuffers.
// flatbuffer_version_string is used by Google developers to identify which
// applications uploaded to Google Play are using this library.  This allows
// the development team at Google to determine the popularity of the library.
// How it works: Applications that are uploaded to the Google Play Store are
// scanned for this version string.  We track which applications are using it
// to measure popularity.  You are free to remove it (of course) but we would
// appreciate if you left it in.

// Weak linkage is culled by VS & doesn't work on cygwin.
#if !defined(_WIN32) && !defined(__CYGWIN__)

extern volatile __attribute__((weak)) const char *flatbuffer_version_string;
volatile __attribute__((weak)) const char *flatbuffer_version_string =
  "FlatBuffers "
  FLATBUFFERS_STRING(FLATBUFFERS_VERSION_MAJOR) "."
  FLATBUFFERS_STRING(FLATBUFFERS_VERSION_MINOR) "."
  FLATBUFFERS_STRING(FLATBUFFERS_VERSION_REVISION);

#endif  // !defined(_WIN32) && !defined(__CYGWIN__)

}  // namespace flatbuffers

#endif  // FLATBUFFERS_H_
