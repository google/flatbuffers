/*
 * Copyright 2015 Google Inc. All rights reserved.
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

#ifndef FLATBUFFERS_HASH_H_
#define FLATBUFFERS_HASH_H_

#include <cstdint>
#include <cstring>

#include "flatbuffers/flatbuffers.h"

namespace flatbuffers {

template<typename T> struct FnvTraits {
  static const T kFnvPrime;
  static const T kOffsetBasis;
};

template<> struct FnvTraits<uint32_t> {
  static const uint32_t kFnvPrime = 0x01000193;
  static const uint32_t kOffsetBasis = 0x811C9DC5;
};

template<> struct FnvTraits<uint64_t> {
  static const uint64_t kFnvPrime = 0x00000100000001b3ULL;
  static const uint64_t kOffsetBasis = 0xcbf29ce484222645ULL;
};

template<typename T> T HashFnv1(const char *input) {
  T hash = FnvTraits<T>::kOffsetBasis;
  for (const char *c = input; *c; ++c) {
    hash *= FnvTraits<T>::kFnvPrime;
    hash ^= static_cast<unsigned char>(*c);
  }
  return hash;
}

template<typename T> T HashFnv1a(const char *input) {
  T hash = FnvTraits<T>::kOffsetBasis;
  for (const char *c = input; *c; ++c) {
    hash ^= static_cast<unsigned char>(*c);
    hash *= FnvTraits<T>::kFnvPrime;
  }
  return hash;
}

template<> inline uint16_t HashFnv1<uint16_t>(const char *input) {
  uint32_t hash = HashFnv1<uint32_t>(input);
  return (hash >> 16) ^ (hash & 0xffff);
}

template<> inline uint16_t HashFnv1a<uint16_t>(const char *input) {
  uint32_t hash = HashFnv1a<uint32_t>(input);
  return (hash >> 16) ^ (hash & 0xffff);
}

// TOME_EDIT - WR: Murmur3 hashing
namespace impl
{
  constexpr uint32_t c1 = 0xcc9e2d51u;
  constexpr uint32_t c2 = 0x1b873593u;

  template<int r>
  constexpr uint32_t rotl(uint32_t x)
  {
    return (x << r) | (x >> (32 - r));
  }

  // WR: Using this method to prevent MSVC from throwing overflow errors in constexpr
  constexpr uint32_t M3FMix_nooverflow(uint32_t h)
  {
    uint64_t hull = h;
    hull ^= hull >> 16;
    hull *= 0x85ebca6b; hull &= 0x00000000ffffffffull;
    hull ^= hull >> 13;
    hull *= 0xc2b2ae35; hull &= 0x00000000ffffffffull;
    hull ^= hull >> 16;

    return uint32_t(hull);
  }

  constexpr uint32_t Murmur3(uint32_t k1, uint32_t seed)
  {
    uint32_t h1 = seed;

    k1 *= c1;
    k1 = rotl<15>(k1);
    k1 *= c2;

    h1 ^= k1;
    h1 = rotl<13>(h1);
    h1 = h1 * 5 + 0xe6546b64;

    //----------
    // finalization

    h1 ^= sizeof(uint32_t);
    return h1;
  }

  constexpr uint32_t strlen(const char* _string)
  {
    uint32_t i = 0;
    while (_string[i] != '\0') ++i;
    return i;
  }

  constexpr uint32_t word32le(const char* _string, uint32_t _len)
  {
    return (_len > 0 ? static_cast<uint32_t>(_string[0]) : 0)
      + (_len > 1 ? static_cast<uint32_t>(_string[1]) << 8 : 0)
      + (_len > 2 ? static_cast<uint32_t>(_string[2]) << 16 : 0)
      + (_len > 3 ? static_cast<uint32_t>(_string[3]) << 24 : 0);
  }

  constexpr uint32_t word32le(const char* _string) { return word32le(_string, 4); }

  constexpr uint32_t Murmur3(const char* _data, const uint32_t _len, uint32_t _seed)
  {
    uint64_t h = _seed;

    for (uint32_t i = 0; i < _len >> 2; ++i)
    {
      uint64_t ki = word32le(_data + i * 4);

      ki *= c1;
      ki = rotl<15>(uint32_t(ki));
      ki *= c2;

      h ^= ki;
      h = rotl<13>(uint32_t(h));
      h = h * 5 + 0xe6546b64;
    }

    uint32_t rlen = _len & 3;
    if (rlen)
    {
      uint64_t ki = word32le(_data + _len - rlen, rlen);

      ki *= c1;
      ki = rotl<15>(uint32_t(ki));
      ki *= c2;

      h ^= ki;
    }

    h ^= _len;

    return M3FMix_nooverflow(uint32_t(h));
  }
} // impl

constexpr uint32_t Murmur3(const char* _string)
{
  return impl::Murmur3(_string, impl::strlen(_string), 0);
}
// TOME_END

template <typename T> struct NamedHashFunction {
  const char *name;

  typedef T (*HashFunction)(const char *);
  HashFunction function;
};

const NamedHashFunction<uint16_t> kHashFunctions16[] = {
  { "fnv1_16", HashFnv1<uint16_t> },
  { "fnv1a_16", HashFnv1a<uint16_t> },
};

const NamedHashFunction<uint32_t> kHashFunctions32[] = {
  { "fnv1_32", HashFnv1<uint32_t> },
  { "fnv1a_32", HashFnv1a<uint32_t> },
// TOME_EDIT - WR: Murmur3 hashing
  { "murmur3", Murmur3 },
// TOME_END
};

const NamedHashFunction<uint64_t> kHashFunctions64[] = {
  { "fnv1_64", HashFnv1<uint64_t> },
  { "fnv1a_64", HashFnv1a<uint64_t> },
};

inline NamedHashFunction<uint16_t>::HashFunction FindHashFunction16(
    const char *name) {
  std::size_t size = sizeof(kHashFunctions16) / sizeof(kHashFunctions16[0]);
  for (std::size_t i = 0; i < size; ++i) {
    if (std::strcmp(name, kHashFunctions16[i].name) == 0) {
      return kHashFunctions16[i].function;
    }
  }
  return nullptr;
}

inline NamedHashFunction<uint32_t>::HashFunction FindHashFunction32(
    const char *name) {
  std::size_t size = sizeof(kHashFunctions32) / sizeof(kHashFunctions32[0]);
  for (std::size_t i = 0; i < size; ++i) {
    if (std::strcmp(name, kHashFunctions32[i].name) == 0) {
      return kHashFunctions32[i].function;
    }
  }
  return nullptr;
}

inline NamedHashFunction<uint64_t>::HashFunction FindHashFunction64(
    const char *name) {
  std::size_t size = sizeof(kHashFunctions64) / sizeof(kHashFunctions64[0]);
  for (std::size_t i = 0; i < size; ++i) {
    if (std::strcmp(name, kHashFunctions64[i].name) == 0) {
      return kHashFunctions64[i].function;
    }
  }
  return nullptr;
}

}  // namespace flatbuffers

#endif  // FLATBUFFERS_HASH_H_
