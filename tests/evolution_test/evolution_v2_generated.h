// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_EVOLUTIONV2_EVOLUTION_V2_H_
#define FLATBUFFERS_GENERATED_EVOLUTIONV2_EVOLUTION_V2_H_

#include "flatbuffers/flatbuffers.h"

namespace Evolution {
namespace V2 {

struct TableA;
struct TableABuilder;

struct TableB;
struct TableBBuilder;

struct TableC;
struct TableCBuilder;

struct Struct;

struct Root;
struct RootBuilder;

enum class Enum : int8_t {
  King = 0,
  Queen = 1,
  Rook = 2,
  Bishop = 3,
  MIN = King,
  MAX = Bishop
};

inline const Enum (&EnumValuesEnum())[4] {
  static const Enum values[] = {
    Enum::King,
    Enum::Queen,
    Enum::Rook,
    Enum::Bishop
  };
  return values;
}

inline const char * const *EnumNamesEnum() {
  static const char * const names[5] = {
    "King",
    "Queen",
    "Rook",
    "Bishop",
    nullptr
  };
  return names;
}

inline const char *EnumNameEnum(Enum e) {
  if (flatbuffers::IsOutRange(e, Enum::King, Enum::Bishop)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesEnum()[index];
}

enum class Union : uint8_t {
  NONE = 0,
  TableA = 1,
  TableB = 2,
  TableC = 3,
  MIN = NONE,
  MAX = TableC
};

inline const Union (&EnumValuesUnion())[4] {
  static const Union values[] = {
    Union::NONE,
    Union::TableA,
    Union::TableB,
    Union::TableC
  };
  return values;
}

inline const char * const *EnumNamesUnion() {
  static const char * const names[5] = {
    "NONE",
    "TableA",
    "TableB",
    "TableC",
    nullptr
  };
  return names;
}

inline const char *EnumNameUnion(Union e) {
  if (flatbuffers::IsOutRange(e, Union::NONE, Union::TableC)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesUnion()[index];
}

template<typename T> struct UnionTraits {
  static const Union enum_value = Union::NONE;
};

template<> struct UnionTraits<Evolution::V2::TableA> {
  static const Union enum_value = Union::TableA;
};

template<> struct UnionTraits<Evolution::V2::TableB> {
  static const Union enum_value = Union::TableB;
};

template<> struct UnionTraits<Evolution::V2::TableC> {
  static const Union enum_value = Union::TableC;
};

bool VerifyUnion(flatbuffers::Verifier &verifier, const void *obj, Union type);
bool VerifyUnionVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<Union> *types);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(8) Struct FLATBUFFERS_FINAL_CLASS {
 private:
  int32_t a_;
  int32_t padding0__;
  double b_;

 public:
  Struct()
      : a_(0),
        padding0__(0),
        b_(0) {
    (void)padding0__;
  }
  Struct(int32_t _a, double _b)
      : a_(flatbuffers::EndianScalar(_a)),
        padding0__(0),
        b_(flatbuffers::EndianScalar(_b)) {
    (void)padding0__;
  }
  int32_t a() const {
    return flatbuffers::EndianScalar(a_);
  }
  double b() const {
    return flatbuffers::EndianScalar(b_);
  }
};
FLATBUFFERS_STRUCT_END(Struct, 16);

inline bool operator==(const Struct &lhs, const Struct &rhs) {
  return
      (lhs.a() == rhs.a()) &&
      (lhs.b() == rhs.b());
}

inline bool operator!=(const Struct &lhs, const Struct &rhs) {
    return !(lhs == rhs);
}


struct TableA FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef TableABuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_A = 4,
    VT_B = 6,
    VT_C = 8
  };
  float a() const {
    return GetField<float>(VT_A, 0.0f);
  }
  int32_t b() const {
    return GetField<int32_t>(VT_B, 0);
  }
  const flatbuffers::String *c() const {
    return GetPointer<const flatbuffers::String *>(VT_C);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<float>(verifier, VT_A, 4) &&
           VerifyField<int32_t>(verifier, VT_B, 4) &&
           VerifyOffset(verifier, VT_C) &&
           verifier.VerifyString(c()) &&
           verifier.EndTable();
  }
};

struct TableABuilder {
  typedef TableA Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_a(float a) {
    fbb_.AddElement<float>(TableA::VT_A, a, 0.0f);
  }
  void add_b(int32_t b) {
    fbb_.AddElement<int32_t>(TableA::VT_B, b, 0);
  }
  void add_c(flatbuffers::Offset<flatbuffers::String> c) {
    fbb_.AddOffset(TableA::VT_C, c);
  }
  explicit TableABuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<TableA> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<TableA>(end);
    return o;
  }
};

inline flatbuffers::Offset<TableA> CreateTableA(
    flatbuffers::FlatBufferBuilder &_fbb,
    float a = 0.0f,
    int32_t b = 0,
    flatbuffers::Offset<flatbuffers::String> c = 0) {
  TableABuilder builder_(_fbb);
  builder_.add_c(c);
  builder_.add_b(b);
  builder_.add_a(a);
  return builder_.Finish();
}

inline flatbuffers::Offset<TableA> CreateTableADirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    float a = 0.0f,
    int32_t b = 0,
    const char *c = nullptr) {
  auto c__ = c ? _fbb.CreateString(c) : 0;
  return Evolution::V2::CreateTableA(
      _fbb,
      a,
      b,
      c__);
}

struct TableB FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef TableBBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_A = 4
  };
  int32_t a() const {
    return GetField<int32_t>(VT_A, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int32_t>(verifier, VT_A, 4) &&
           verifier.EndTable();
  }
};

struct TableBBuilder {
  typedef TableB Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_a(int32_t a) {
    fbb_.AddElement<int32_t>(TableB::VT_A, a, 0);
  }
  explicit TableBBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<TableB> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<TableB>(end);
    return o;
  }
};

inline flatbuffers::Offset<TableB> CreateTableB(
    flatbuffers::FlatBufferBuilder &_fbb,
    int32_t a = 0) {
  TableBBuilder builder_(_fbb);
  builder_.add_a(a);
  return builder_.Finish();
}

struct TableC FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef TableCBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_A = 4,
    VT_B = 6
  };
  double a() const {
    return GetField<double>(VT_A, 0.0);
  }
  const flatbuffers::String *b() const {
    return GetPointer<const flatbuffers::String *>(VT_B);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<double>(verifier, VT_A, 8) &&
           VerifyOffset(verifier, VT_B) &&
           verifier.VerifyString(b()) &&
           verifier.EndTable();
  }
};

struct TableCBuilder {
  typedef TableC Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_a(double a) {
    fbb_.AddElement<double>(TableC::VT_A, a, 0.0);
  }
  void add_b(flatbuffers::Offset<flatbuffers::String> b) {
    fbb_.AddOffset(TableC::VT_B, b);
  }
  explicit TableCBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<TableC> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<TableC>(end);
    return o;
  }
};

inline flatbuffers::Offset<TableC> CreateTableC(
    flatbuffers::FlatBufferBuilder &_fbb,
    double a = 0.0,
    flatbuffers::Offset<flatbuffers::String> b = 0) {
  TableCBuilder builder_(_fbb);
  builder_.add_a(a);
  builder_.add_b(b);
  return builder_.Finish();
}

inline flatbuffers::Offset<TableC> CreateTableCDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    double a = 0.0,
    const char *b = nullptr) {
  auto b__ = b ? _fbb.CreateString(b) : 0;
  return Evolution::V2::CreateTableC(
      _fbb,
      a,
      b__);
}

struct Root FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef RootBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_B = 6,
    VT_C_TYPE = 8,
    VT_C = 10,
    VT_D = 12,
    VT_E = 14,
    VT_FF = 16,
    VT_G = 18,
    VT_H = 20,
    VT_I = 22,
    VT_K = 28,
    VT_L = 30
  };
  bool b() const {
    return GetField<uint8_t>(VT_B, 0) != 0;
  }
  Evolution::V2::Union c_type() const {
    return static_cast<Evolution::V2::Union>(GetField<uint8_t>(VT_C_TYPE, 0));
  }
  const void *c() const {
    return GetPointer<const void *>(VT_C);
  }
  template<typename T> const T *c_as() const;
  const Evolution::V2::TableA *c_as_TableA() const {
    return c_type() == Evolution::V2::Union::TableA ? static_cast<const Evolution::V2::TableA *>(c()) : nullptr;
  }
  const Evolution::V2::TableB *c_as_TableB() const {
    return c_type() == Evolution::V2::Union::TableB ? static_cast<const Evolution::V2::TableB *>(c()) : nullptr;
  }
  const Evolution::V2::TableC *c_as_TableC() const {
    return c_type() == Evolution::V2::Union::TableC ? static_cast<const Evolution::V2::TableC *>(c()) : nullptr;
  }
  Evolution::V2::Enum d() const {
    return static_cast<Evolution::V2::Enum>(GetField<int8_t>(VT_D, 0));
  }
  const Evolution::V2::TableA *e() const {
    return GetPointer<const Evolution::V2::TableA *>(VT_E);
  }
  const Evolution::V2::Struct *ff() const {
    return GetStruct<const Evolution::V2::Struct *>(VT_FF);
  }
  const flatbuffers::Vector<int32_t> *g() const {
    return GetPointer<const flatbuffers::Vector<int32_t> *>(VT_G);
  }
  const flatbuffers::Vector<flatbuffers::Offset<Evolution::V2::TableB>> *h() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<Evolution::V2::TableB>> *>(VT_H);
  }
  uint32_t i() const {
    return GetField<uint32_t>(VT_I, 1234);
  }
  const Evolution::V2::TableC *k() const {
    return GetPointer<const Evolution::V2::TableC *>(VT_K);
  }
  uint8_t l() const {
    return GetField<uint8_t>(VT_L, 56);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_B, 1) &&
           VerifyField<uint8_t>(verifier, VT_C_TYPE, 1) &&
           VerifyOffset(verifier, VT_C) &&
           VerifyUnion(verifier, c(), c_type()) &&
           VerifyField<int8_t>(verifier, VT_D, 1) &&
           VerifyOffset(verifier, VT_E) &&
           verifier.VerifyTable(e()) &&
           VerifyField<Evolution::V2::Struct>(verifier, VT_FF, 8) &&
           VerifyOffset(verifier, VT_G) &&
           verifier.VerifyVector(g()) &&
           VerifyOffset(verifier, VT_H) &&
           verifier.VerifyVector(h()) &&
           verifier.VerifyVectorOfTables(h()) &&
           VerifyField<uint32_t>(verifier, VT_I, 4) &&
           VerifyOffset(verifier, VT_K) &&
           verifier.VerifyTable(k()) &&
           VerifyField<uint8_t>(verifier, VT_L, 1) &&
           verifier.EndTable();
  }
};

template<> inline const Evolution::V2::TableA *Root::c_as<Evolution::V2::TableA>() const {
  return c_as_TableA();
}

template<> inline const Evolution::V2::TableB *Root::c_as<Evolution::V2::TableB>() const {
  return c_as_TableB();
}

template<> inline const Evolution::V2::TableC *Root::c_as<Evolution::V2::TableC>() const {
  return c_as_TableC();
}

struct RootBuilder {
  typedef Root Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_b(bool b) {
    fbb_.AddElement<uint8_t>(Root::VT_B, static_cast<uint8_t>(b), 0);
  }
  void add_c_type(Evolution::V2::Union c_type) {
    fbb_.AddElement<uint8_t>(Root::VT_C_TYPE, static_cast<uint8_t>(c_type), 0);
  }
  void add_c(flatbuffers::Offset<void> c) {
    fbb_.AddOffset(Root::VT_C, c);
  }
  void add_d(Evolution::V2::Enum d) {
    fbb_.AddElement<int8_t>(Root::VT_D, static_cast<int8_t>(d), 0);
  }
  void add_e(flatbuffers::Offset<Evolution::V2::TableA> e) {
    fbb_.AddOffset(Root::VT_E, e);
  }
  void add_ff(const Evolution::V2::Struct *ff) {
    fbb_.AddStruct(Root::VT_FF, ff);
  }
  void add_g(flatbuffers::Offset<flatbuffers::Vector<int32_t>> g) {
    fbb_.AddOffset(Root::VT_G, g);
  }
  void add_h(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Evolution::V2::TableB>>> h) {
    fbb_.AddOffset(Root::VT_H, h);
  }
  void add_i(uint32_t i) {
    fbb_.AddElement<uint32_t>(Root::VT_I, i, 1234);
  }
  void add_k(flatbuffers::Offset<Evolution::V2::TableC> k) {
    fbb_.AddOffset(Root::VT_K, k);
  }
  void add_l(uint8_t l) {
    fbb_.AddElement<uint8_t>(Root::VT_L, l, 56);
  }
  explicit RootBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Root> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Root>(end);
    return o;
  }
};

inline flatbuffers::Offset<Root> CreateRoot(
    flatbuffers::FlatBufferBuilder &_fbb,
    bool b = false,
    Evolution::V2::Union c_type = Evolution::V2::Union::NONE,
    flatbuffers::Offset<void> c = 0,
    Evolution::V2::Enum d = Evolution::V2::Enum::King,
    flatbuffers::Offset<Evolution::V2::TableA> e = 0,
    const Evolution::V2::Struct *ff = nullptr,
    flatbuffers::Offset<flatbuffers::Vector<int32_t>> g = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Evolution::V2::TableB>>> h = 0,
    uint32_t i = 1234,
    flatbuffers::Offset<Evolution::V2::TableC> k = 0,
    uint8_t l = 56) {
  RootBuilder builder_(_fbb);
  builder_.add_k(k);
  builder_.add_i(i);
  builder_.add_h(h);
  builder_.add_g(g);
  builder_.add_ff(ff);
  builder_.add_e(e);
  builder_.add_c(c);
  builder_.add_l(l);
  builder_.add_d(d);
  builder_.add_c_type(c_type);
  builder_.add_b(b);
  return builder_.Finish();
}

inline flatbuffers::Offset<Root> CreateRootDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    bool b = false,
    Evolution::V2::Union c_type = Evolution::V2::Union::NONE,
    flatbuffers::Offset<void> c = 0,
    Evolution::V2::Enum d = Evolution::V2::Enum::King,
    flatbuffers::Offset<Evolution::V2::TableA> e = 0,
    const Evolution::V2::Struct *ff = nullptr,
    const std::vector<int32_t> *g = nullptr,
    const std::vector<flatbuffers::Offset<Evolution::V2::TableB>> *h = nullptr,
    uint32_t i = 1234,
    flatbuffers::Offset<Evolution::V2::TableC> k = 0,
    uint8_t l = 56) {
  auto g__ = g ? _fbb.CreateVector<int32_t>(*g) : 0;
  auto h__ = h ? _fbb.CreateVector<flatbuffers::Offset<Evolution::V2::TableB>>(*h) : 0;
  return Evolution::V2::CreateRoot(
      _fbb,
      b,
      c_type,
      c,
      d,
      e,
      ff,
      g__,
      h__,
      i,
      k,
      l);
}

inline bool VerifyUnion(flatbuffers::Verifier &verifier, const void *obj, Union type) {
  switch (type) {
    case Union::NONE: {
      return true;
    }
    case Union::TableA: {
      auto ptr = reinterpret_cast<const Evolution::V2::TableA *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case Union::TableB: {
      auto ptr = reinterpret_cast<const Evolution::V2::TableB *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case Union::TableC: {
      auto ptr = reinterpret_cast<const Evolution::V2::TableC *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyUnionVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<Union> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyUnion(
        verifier,  values->Get(i), types->GetEnum<Union>(i))) {
      return false;
    }
  }
  return true;
}

inline const Evolution::V2::Root *GetRoot(const void *buf) {
  return flatbuffers::GetRoot<Evolution::V2::Root>(buf);
}

inline const Evolution::V2::Root *GetSizePrefixedRoot(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<Evolution::V2::Root>(buf);
}

inline bool VerifyRootBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<Evolution::V2::Root>(nullptr);
}

inline bool VerifySizePrefixedRootBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<Evolution::V2::Root>(nullptr);
}

inline void FinishRootBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<Evolution::V2::Root> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedRootBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<Evolution::V2::Root> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace V2
}  // namespace Evolution

#endif  // FLATBUFFERS_GENERATED_EVOLUTIONV2_EVOLUTION_V2_H_
