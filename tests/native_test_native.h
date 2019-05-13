// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_NATIVE_NATIVETEST_TESTN_H_
#define FLATBUFFERS_NATIVE_NATIVETEST_TESTN_H_

#include "flatbuffers/flatbuffers.h"
#include <variant>
#include "native_test_generated.h"
#include "custom_native.h"


namespace TestN {
namespace Native {

struct Foo;
}  // namespace Native

namespace NamespaceFoo {
namespace Native {

struct Foo;
}  // namespace Native
}  // namespace NamespaceFoo

namespace NamespaceBar {
namespace Native {

struct Foo;
}  // namespace Native
}  // namespace NamespaceBar

namespace Native {

struct MyUnionUnion;

bool operator==(const Foo &lhs, const Foo &rhs);

}  // namespace Native

namespace NamespaceFoo {
namespace Native {

bool operator==(const Foo &lhs, const Foo &rhs);

}  // namespace Native
}  // namespace NamespaceFoo

namespace NamespaceBar {
namespace Native {

bool operator==(const Foo &lhs, const Foo &rhs);

}  // namespace Native
}  // namespace NamespaceBar

namespace Native {

struct MyUnionUnion : public std::variant<flatbuffers::NoneType, MyMat> {
    using std::variant<flatbuffers::NoneType, MyMat>::variant;
};

struct Foo : public flatbuffers::NativeTable {
  typedef ::TestN::Foo TableType;
  ::TestN::BundleSize enumData;
  MyMat bitData;
  std::vector<std::complex<double>> iqData;
  std::complex<double> iqSample;
  Comp iqSample2;
  int32_t newInt;
  MyUnionUnion variant;
  Foo()
      : enumData(::TestN::BundleSize_Size2),
        newInt(0) {
  }
};

inline bool operator==(const Foo &lhs, const Foo &rhs) {
  return
      (lhs.enumData == rhs.enumData) &&
      (lhs.bitData == rhs.bitData) &&
      (lhs.iqData == rhs.iqData) &&
      (lhs.iqSample == rhs.iqSample) &&
      (lhs.iqSample2 == rhs.iqSample2) &&
      (lhs.newInt == rhs.newInt) &&
      (lhs.variant == rhs.variant);
}

}  // namespace Native

namespace NamespaceFoo {
namespace Native {

struct Foo : public flatbuffers::NativeTable {
  typedef ::TestN::NamespaceFoo::Foo TableType;
  ::TestN::Native::Foo foo;
  Foo() {
  }
};

inline bool operator==(const Foo &lhs, const Foo &rhs) {
  return
      (lhs.foo == rhs.foo);
}

}  // namespace Native
}  // namespace NamespaceFoo

namespace NamespaceBar {
namespace Native {

struct Foo : public flatbuffers::NativeTable {
  typedef ::TestN::NamespaceBar::Foo TableType;
  ::TestN::Native::Foo foo2;
  Foo() {
  }
};

inline bool operator==(const Foo &lhs, const Foo &rhs) {
  return
      (lhs.foo2 == rhs.foo2);
}

}  // namespace Native
}  // namespace NamespaceBar
}  // namespace TestN

namespace Native {

flatbuffers::Offset<::TestN::Foo> Pack(flatbuffers::FlatBufferBuilder &_fbb, const ::TestN::Native::Foo &_o, const flatbuffers::rehasher_function_t *_rehasher = nullptr);
::TestN::Native::Foo UnPack(const ::TestN::Foo &_f, const flatbuffers::resolver_function_t *_resolver = nullptr);

flatbuffers::Offset<::TestN::NamespaceFoo::Foo> Pack(flatbuffers::FlatBufferBuilder &_fbb, const ::TestN::NamespaceFoo::Native::Foo &_o, const flatbuffers::rehasher_function_t *_rehasher = nullptr);
::TestN::NamespaceFoo::Native::Foo UnPack(const ::TestN::NamespaceFoo::Foo &_f, const flatbuffers::resolver_function_t *_resolver = nullptr);

flatbuffers::Offset<::TestN::NamespaceBar::Foo> Pack(flatbuffers::FlatBufferBuilder &_fbb, const ::TestN::NamespaceBar::Native::Foo &_o, const flatbuffers::rehasher_function_t *_rehasher = nullptr);
::TestN::NamespaceBar::Native::Foo UnPack(const ::TestN::NamespaceBar::Foo &_f, const flatbuffers::resolver_function_t *_resolver = nullptr);

flatbuffers::Offset<void> Pack(flatbuffers::FlatBufferBuilder &_fbb, const ::TestN::Native::MyUnionUnion &_o, const flatbuffers::rehasher_function_t *_rehasher = nullptr);
::TestN::Native::MyUnionUnion UnPack(const void *obj, const ::TestN::MyUnion type, const flatbuffers::resolver_function_t *resolver = nullptr);

inline ::TestN::Native::Foo UnPack(const ::TestN::Foo &_f, const flatbuffers::resolver_function_t *_resolver) {
  (void)_f;
  (void)_resolver;
  auto _o = ::TestN::Native::Foo();
  { auto _e = _f.enumData(); _o.enumData = _e; };
  { auto _e = _f.bitData(); if (_e) _o.bitData = ::Native::UnPack(*_e, _resolver); };
  { auto _e = _f.iqData(); if (_e) { _o.iqData.resize(_e->size()); for (flatbuffers::uoffset_t _i = 0; _i < _e->size(); _i++) { _o.iqData[_i] = ::Native::UnPack(*_e->Get(_i)); } } };
  { auto _e = _f.iqSample(); if (_e) _o.iqSample = ::Native::UnPack(*_e); };
  { auto _e = _f.iqSample2(); if (_e) _o.iqSample2 = *_e; };
  { auto _e = _f.newInt(); _o.newInt = _e; };
  { auto _e = _f.variant(); if (_e) _o.variant = ::Native::UnPack(_e, _f.variant_type(), _resolver); };
  return _o;
}

inline flatbuffers::Offset<::TestN::Foo> Pack(flatbuffers::FlatBufferBuilder &_fbb, const ::TestN::Native::Foo &_o, const flatbuffers::rehasher_function_t *_rehasher) {
  (void)_rehasher;
  (void)_o;
  struct _VectorArgs { flatbuffers::FlatBufferBuilder *__fbb; const ::TestN::Native::Foo& __o; const flatbuffers::rehasher_function_t *__rehasher; } _va = { &_fbb, _o, _rehasher}; (void)_va;
  auto variant__ = ::Native::Pack(_fbb, _o.variant, _rehasher);
  auto newInt__ = _o.newInt;
  auto iqSample2__ = &_o.iqSample2;
  auto iqSample__ = & static_cast<const ::TestN::Complex &>(::Native::Pack(_o.iqSample));
  auto iqData__ = _o.iqData.size() ? _fbb.CreateVectorOfStructs<::TestN::Complex>(_o.iqData.size(),[&](size_t i, ::TestN::Complex *r) {*r = ::Native::Pack(_o.iqData[i]);}) : 0;
  auto bitData__ = ::Native::Pack(_fbb, _o.bitData, _rehasher);
  auto variant_type__ = static_cast<::TestN::MyUnion>(_o.variant.index());
  auto enumData__ = _o.enumData;
  ::TestN::FooBuilder builder_(_fbb);
  builder_.add_variant(variant__);
  builder_.add_newInt(newInt__);
  builder_.add_iqSample2(iqSample2__);
  builder_.add_iqSample(iqSample__);
  builder_.add_iqData(iqData__);
  builder_.add_bitData(bitData__);
  builder_.add_variant_type(variant_type__);
  builder_.add_enumData(enumData__);
  return builder_.Finish();
}

inline ::TestN::NamespaceFoo::Native::Foo UnPack(const ::TestN::NamespaceFoo::Foo &_f, const flatbuffers::resolver_function_t *_resolver) {
  (void)_f;
  (void)_resolver;
  auto _o = ::TestN::NamespaceFoo::Native::Foo();
  { auto _e = _f.foo(); if (_e) _o.foo = ::Native::UnPack(*_e, _resolver); };
  return _o;
}

inline flatbuffers::Offset<::TestN::NamespaceFoo::Foo> Pack(flatbuffers::FlatBufferBuilder &_fbb, const ::TestN::NamespaceFoo::Native::Foo &_o, const flatbuffers::rehasher_function_t *_rehasher) {
  (void)_rehasher;
  (void)_o;
  struct _VectorArgs { flatbuffers::FlatBufferBuilder *__fbb; const ::TestN::NamespaceFoo::Native::Foo& __o; const flatbuffers::rehasher_function_t *__rehasher; } _va = { &_fbb, _o, _rehasher}; (void)_va;
  auto foo__ = ::Native::Pack(_fbb, _o.foo, _rehasher);
  ::TestN::NamespaceFoo::FooBuilder builder_(_fbb);
  builder_.add_foo(foo__);
  return builder_.Finish();
}

inline ::TestN::NamespaceBar::Native::Foo UnPack(const ::TestN::NamespaceBar::Foo &_f, const flatbuffers::resolver_function_t *_resolver) {
  (void)_f;
  (void)_resolver;
  auto _o = ::TestN::NamespaceBar::Native::Foo();
  { auto _e = _f.foo2(); if (_e) _o.foo2 = ::Native::UnPack(*_e, _resolver); };
  return _o;
}

inline flatbuffers::Offset<::TestN::NamespaceBar::Foo> Pack(flatbuffers::FlatBufferBuilder &_fbb, const ::TestN::NamespaceBar::Native::Foo &_o, const flatbuffers::rehasher_function_t *_rehasher) {
  (void)_rehasher;
  (void)_o;
  struct _VectorArgs { flatbuffers::FlatBufferBuilder *__fbb; const ::TestN::NamespaceBar::Native::Foo& __o; const flatbuffers::rehasher_function_t *__rehasher; } _va = { &_fbb, _o, _rehasher}; (void)_va;
  auto foo2__ = ::Native::Pack(_fbb, _o.foo2, _rehasher);
  ::TestN::NamespaceBar::FooBuilder builder_(_fbb);
  builder_.add_foo2(foo2__);
  return builder_.Finish();
}

}  // namespace Native

#endif  // FLATBUFFERS_NATIVE_NATIVETEST_TESTN_H_
