// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_BASIC_H_
#define FLATBUFFERS_GENERATED_BASIC_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 5 &&
              FLATBUFFERS_VERSION_REVISION == 9,
             "Non-compatible flatbuffers version included");

struct Galaxy;
struct GalaxyBuilder;

struct Universe;
struct UniverseBuilder;

struct Galaxy FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef GalaxyBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NUM_STARS = 4
  };
  int64_t num_stars() const {
    return GetField<int64_t>(VT_NUM_STARS, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_NUM_STARS, 8) &&
           verifier.EndTable();
  }
};

struct GalaxyBuilder {
  typedef Galaxy Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_num_stars(int64_t num_stars) {
    fbb_.AddElement<int64_t>(Galaxy::VT_NUM_STARS, num_stars, 0);
  }
  explicit GalaxyBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Galaxy> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Galaxy>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Galaxy> CreateGalaxy(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    int64_t num_stars = 0) {
  GalaxyBuilder builder_(_fbb);
  builder_.add_num_stars(num_stars);
  return builder_.Finish();
}

struct Universe FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef UniverseBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_AGE = 4,
    VT_GALAXIES = 6
  };
  double age() const {
    return GetField<double>(VT_AGE, 0.0);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<Galaxy>> *galaxies() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<Galaxy>> *>(VT_GALAXIES);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<double>(verifier, VT_AGE, 8) &&
           VerifyOffset(verifier, VT_GALAXIES) &&
           verifier.VerifyVector(galaxies()) &&
           verifier.VerifyVectorOfTables(galaxies()) &&
           verifier.EndTable();
  }
};

struct UniverseBuilder {
  typedef Universe Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_age(double age) {
    fbb_.AddElement<double>(Universe::VT_AGE, age, 0.0);
  }
  void add_galaxies(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<Galaxy>>> galaxies) {
    fbb_.AddOffset(Universe::VT_GALAXIES, galaxies);
  }
  explicit UniverseBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Universe> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Universe>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Universe> CreateUniverse(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    double age = 0.0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<Galaxy>>> galaxies = 0) {
  UniverseBuilder builder_(_fbb);
  builder_.add_age(age);
  builder_.add_galaxies(galaxies);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<Universe> CreateUniverseDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    double age = 0.0,
    const std::vector<::flatbuffers::Offset<Galaxy>> *galaxies = nullptr) {
  auto galaxies__ = galaxies ? _fbb.CreateVector<::flatbuffers::Offset<Galaxy>>(*galaxies) : 0;
  return CreateUniverse(
      _fbb,
      age,
      galaxies__);
}

inline const Universe *GetUniverse(const void *buf) {
  return ::flatbuffers::GetRoot<Universe>(buf);
}

inline const Universe *GetSizePrefixedUniverse(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<Universe>(buf);
}

inline bool VerifyUniverseBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<Universe>(nullptr);
}

inline bool VerifySizePrefixedUniverseBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<Universe>(nullptr);
}

inline void FinishUniverseBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<Universe> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedUniverseBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<Universe> root) {
  fbb.FinishSizePrefixed(root);
}

#endif  // FLATBUFFERS_GENERATED_BASIC_H_
