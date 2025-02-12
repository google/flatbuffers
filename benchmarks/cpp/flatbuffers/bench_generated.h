// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_BENCH_BENCHMARKS_FLATBUFFERS_H_
#define FLATBUFFERS_GENERATED_BENCH_BENCHMARKS_FLATBUFFERS_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 12 &&
              FLATBUFFERS_VERSION_REVISION == 23,
             "Non-compatible flatbuffers version included");

namespace benchmarks_flatbuffers {

struct Foo;

struct Bar;

struct FooBar;
struct FooBarBuilder;

struct FooBarContainer;
struct FooBarContainerBuilder;

enum Enum : int16_t {
  Enum_Apples = 0,
  Enum_Pears = 1,
  Enum_Bananas = 2,
  Enum_MIN = Enum_Apples,
  Enum_MAX = Enum_Bananas
};

inline const Enum (&EnumValuesEnum())[3] {
  static const Enum values[] = {
    Enum_Apples,
    Enum_Pears,
    Enum_Bananas
  };
  return values;
}

inline const char * const *EnumNamesEnum() {
  static const char * const names[4] = {
    "Apples",
    "Pears",
    "Bananas",
    nullptr
  };
  return names;
}

inline const char *EnumNameEnum(Enum e) {
  if (flatbuffers::IsOutRange(e, Enum_Apples, Enum_Bananas)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesEnum()[index];
}

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(8) Foo FLATBUFFERS_FINAL_CLASS {
 private:
  uint64_t id_;
  int16_t count_;
  int8_t prefix_;
  int8_t padding0__;
  uint32_t length_;

 public:
  Foo()
      : id_(0),
        count_(0),
        prefix_(0),
        padding0__(0),
        length_(0) {
    (void)padding0__;
  }
  Foo(uint64_t _id, int16_t _count, int8_t _prefix, uint32_t _length)
      : id_(flatbuffers::EndianScalar(_id)),
        count_(flatbuffers::EndianScalar(_count)),
        prefix_(flatbuffers::EndianScalar(_prefix)),
        padding0__(0),
        length_(flatbuffers::EndianScalar(_length)) {
    (void)padding0__;
  }
  uint64_t id() const {
    return flatbuffers::EndianScalar(id_);
  }
  int16_t count() const {
    return flatbuffers::EndianScalar(count_);
  }
  int8_t prefix() const {
    return flatbuffers::EndianScalar(prefix_);
  }
  uint32_t length() const {
    return flatbuffers::EndianScalar(length_);
  }
};
FLATBUFFERS_STRUCT_END(Foo, 16);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(8) Bar FLATBUFFERS_FINAL_CLASS {
 private:
  benchmarks_flatbuffers::Foo parent_;
  int32_t time_;
  float ratio_;
  uint16_t size_;
  int16_t padding0__;  int32_t padding1__;

 public:
  Bar()
      : parent_(),
        time_(0),
        ratio_(0),
        size_(0),
        padding0__(0),
        padding1__(0) {
    (void)padding0__;
    (void)padding1__;
  }
  Bar(const benchmarks_flatbuffers::Foo &_parent, int32_t _time, float _ratio, uint16_t _size)
      : parent_(_parent),
        time_(flatbuffers::EndianScalar(_time)),
        ratio_(flatbuffers::EndianScalar(_ratio)),
        size_(flatbuffers::EndianScalar(_size)),
        padding0__(0),
        padding1__(0) {
    (void)padding0__;
    (void)padding1__;
  }
  const benchmarks_flatbuffers::Foo &parent() const {
    return parent_;
  }
  int32_t time() const {
    return flatbuffers::EndianScalar(time_);
  }
  float ratio() const {
    return flatbuffers::EndianScalar(ratio_);
  }
  uint16_t size() const {
    return flatbuffers::EndianScalar(size_);
  }
};
FLATBUFFERS_STRUCT_END(Bar, 32);

struct FooBar FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef FooBarBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SIBLING = 4,
    VT_NAME = 6,
    VT_RATING = 8,
    VT_POSTFIX = 10
  };
  const benchmarks_flatbuffers::Bar *sibling() const {
    return GetStruct<const benchmarks_flatbuffers::Bar *>(VT_SIBLING);
  }
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  double rating() const {
    return GetField<double>(VT_RATING, 0.0);
  }
  uint8_t postfix() const {
    return GetField<uint8_t>(VT_POSTFIX, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<benchmarks_flatbuffers::Bar>(verifier, VT_SIBLING, 8) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<double>(verifier, VT_RATING, 8) &&
           VerifyField<uint8_t>(verifier, VT_POSTFIX, 1) &&
           verifier.EndTable();
  }
};

struct FooBarBuilder {
  typedef FooBar Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_sibling(const benchmarks_flatbuffers::Bar *sibling) {
    fbb_.AddStruct(FooBar::VT_SIBLING, sibling);
  }
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(FooBar::VT_NAME, name);
  }
  void add_rating(double rating) {
    fbb_.AddElement<double>(FooBar::VT_RATING, rating, 0.0);
  }
  void add_postfix(uint8_t postfix) {
    fbb_.AddElement<uint8_t>(FooBar::VT_POSTFIX, postfix, 0);
  }
  explicit FooBarBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<FooBar> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<FooBar>(end);
    return o;
  }
};

inline flatbuffers::Offset<FooBar> CreateFooBar(
    flatbuffers::FlatBufferBuilder &_fbb,
    const benchmarks_flatbuffers::Bar *sibling = nullptr,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    double rating = 0.0,
    uint8_t postfix = 0) {
  FooBarBuilder builder_(_fbb);
  builder_.add_rating(rating);
  builder_.add_name(name);
  builder_.add_sibling(sibling);
  builder_.add_postfix(postfix);
  return builder_.Finish();
}

inline flatbuffers::Offset<FooBar> CreateFooBarDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const benchmarks_flatbuffers::Bar *sibling = nullptr,
    const char *name = nullptr,
    double rating = 0.0,
    uint8_t postfix = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return benchmarks_flatbuffers::CreateFooBar(
      _fbb,
      sibling,
      name__,
      rating,
      postfix);
}

struct FooBarContainer FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef FooBarContainerBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_LIST = 4,
    VT_INITIALIZED = 6,
    VT_FRUIT = 8,
    VT_LOCATION = 10
  };
  const flatbuffers::Vector<flatbuffers::Offset<benchmarks_flatbuffers::FooBar>> *list() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<benchmarks_flatbuffers::FooBar>> *>(VT_LIST);
  }
  bool initialized() const {
    return GetField<uint8_t>(VT_INITIALIZED, 0) != 0;
  }
  benchmarks_flatbuffers::Enum fruit() const {
    return static_cast<benchmarks_flatbuffers::Enum>(GetField<int16_t>(VT_FRUIT, 0));
  }
  const flatbuffers::String *location() const {
    return GetPointer<const flatbuffers::String *>(VT_LOCATION);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_LIST) &&
           verifier.VerifyVector(list()) &&
           verifier.VerifyVectorOfTables(list()) &&
           VerifyField<uint8_t>(verifier, VT_INITIALIZED, 1) &&
           VerifyField<int16_t>(verifier, VT_FRUIT, 2) &&
           VerifyOffset(verifier, VT_LOCATION) &&
           verifier.VerifyString(location()) &&
           verifier.EndTable();
  }
};

struct FooBarContainerBuilder {
  typedef FooBarContainer Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_list(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<benchmarks_flatbuffers::FooBar>>> list) {
    fbb_.AddOffset(FooBarContainer::VT_LIST, list);
  }
  void add_initialized(bool initialized) {
    fbb_.AddElement<uint8_t>(FooBarContainer::VT_INITIALIZED, static_cast<uint8_t>(initialized), 0);
  }
  void add_fruit(benchmarks_flatbuffers::Enum fruit) {
    fbb_.AddElement<int16_t>(FooBarContainer::VT_FRUIT, static_cast<int16_t>(fruit), 0);
  }
  void add_location(flatbuffers::Offset<flatbuffers::String> location) {
    fbb_.AddOffset(FooBarContainer::VT_LOCATION, location);
  }
  explicit FooBarContainerBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<FooBarContainer> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<FooBarContainer>(end);
    return o;
  }
};

inline flatbuffers::Offset<FooBarContainer> CreateFooBarContainer(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<benchmarks_flatbuffers::FooBar>>> list = 0,
    bool initialized = false,
    benchmarks_flatbuffers::Enum fruit = benchmarks_flatbuffers::Enum_Apples,
    flatbuffers::Offset<flatbuffers::String> location = 0) {
  FooBarContainerBuilder builder_(_fbb);
  builder_.add_location(location);
  builder_.add_list(list);
  builder_.add_fruit(fruit);
  builder_.add_initialized(initialized);
  return builder_.Finish();
}

inline flatbuffers::Offset<FooBarContainer> CreateFooBarContainerDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<flatbuffers::Offset<benchmarks_flatbuffers::FooBar>> *list = nullptr,
    bool initialized = false,
    benchmarks_flatbuffers::Enum fruit = benchmarks_flatbuffers::Enum_Apples,
    const char *location = nullptr) {
  auto list__ = list ? _fbb.CreateVector<flatbuffers::Offset<benchmarks_flatbuffers::FooBar>>(*list) : 0;
  auto location__ = location ? _fbb.CreateString(location) : 0;
  return benchmarks_flatbuffers::CreateFooBarContainer(
      _fbb,
      list__,
      initialized,
      fruit,
      location__);
}

inline const benchmarks_flatbuffers::FooBarContainer *GetFooBarContainer(const void *buf) {
  return flatbuffers::GetRoot<benchmarks_flatbuffers::FooBarContainer>(buf);
}

inline const benchmarks_flatbuffers::FooBarContainer *GetSizePrefixedFooBarContainer(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<benchmarks_flatbuffers::FooBarContainer>(buf);
}

inline bool VerifyFooBarContainerBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<benchmarks_flatbuffers::FooBarContainer>(nullptr);
}

inline bool VerifySizePrefixedFooBarContainerBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<benchmarks_flatbuffers::FooBarContainer>(nullptr);
}

inline void FinishFooBarContainerBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<benchmarks_flatbuffers::FooBarContainer> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedFooBarContainerBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<benchmarks_flatbuffers::FooBarContainer> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace benchmarks_flatbuffers

#endif  // FLATBUFFERS_GENERATED_BENCH_BENCHMARKS_FLATBUFFERS_H_
