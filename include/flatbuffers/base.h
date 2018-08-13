#ifndef FLATBUFFERS_BASE_H_
#define FLATBUFFERS_BASE_H_

// clang-format off
#if defined(FLATBUFFERS_MEMORY_LEAK_TRACKING) && \
    defined(_MSC_VER) && defined(_DEBUG)
  #define _CRTDBG_MAP_ALLOC
#endif

#include <assert.h>

#if !defined(FLATBUFFERS_ASSERT)
#define FLATBUFFERS_ASSERT assert
#endif

#ifndef ARDUINO
#include <cstdint>
#endif

#include <cstddef>
#include <cstdlib>
#include <cstring>

#if defined(FLATBUFFERS_MEMORY_LEAK_TRACKING) && \
    defined(_MSC_VER) && defined(_DEBUG)
  #include <crtdbg.h>
  #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
  #define new DEBUG_NEW
#endif

#if defined(ARDUINO) && !defined(ARDUINOSTL_M_H)
  #include <utility.h>
#else
  #include <utility>
#endif

#include <string>
#include <type_traits>
#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include <memory>

#ifdef _STLPORT_VERSION
  #define FLATBUFFERS_CPP98_STL
#endif
#ifndef FLATBUFFERS_CPP98_STL
  #include <functional>
#endif

#include "flatbuffers/stl_emulation.h"

/// @cond FLATBUFFERS_INTERNAL
#if __cplusplus <= 199711L && \
    (!defined(_MSC_VER) || _MSC_VER < 1600) && \
    (!defined(__GNUC__) || \
      (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ < 40400))
  #error A C++11 compatible compiler with support for the auto typing is \
         required for FlatBuffers.
  #error __cplusplus _MSC_VER __GNUC__  __GNUC_MINOR__  __GNUC_PATCHLEVEL__
#endif

#if !defined(__clang__) && \
    defined(__GNUC__) && \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ < 40600)
  // Backwards compatability for g++ 4.4, and 4.5 which don't have the nullptr
  // and constexpr keywords. Note the __clang__ check is needed, because clang
  // presents itself as an older GNUC compiler.
  #ifndef nullptr_t
    const class nullptr_t {
    public:
      template<class T> inline operator T*() const { return 0; }
    private:
      void operator&() const;
    } nullptr = {};
  #endif
  #ifndef constexpr
    #define constexpr const
  #endif
#endif

// The wire format uses a little endian encoding (since that's efficient for
// the common platforms).
#if defined(__s390x__)
  #define FLATBUFFERS_LITTLEENDIAN 0
#endif // __s390x__
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
#define FLATBUFFERS_VERSION_MINOR 9
#define FLATBUFFERS_VERSION_REVISION 0
#define FLATBUFFERS_STRING_EXPAND(X) #X
#define FLATBUFFERS_STRING(X) FLATBUFFERS_STRING_EXPAND(X)

#if (!defined(_MSC_VER) || _MSC_VER > 1600) && \
    (!defined(__GNUC__) || (__GNUC__ * 100 + __GNUC_MINOR__ >= 407))
  #define FLATBUFFERS_FINAL_CLASS final
  #define FLATBUFFERS_OVERRIDE override
#else
  #define FLATBUFFERS_FINAL_CLASS
  #define FLATBUFFERS_OVERRIDE
#endif

#if (!defined(_MSC_VER) || _MSC_VER >= 1900) && \
    (!defined(__GNUC__) || (__GNUC__ * 100 + __GNUC_MINOR__ >= 406))
  #define FLATBUFFERS_CONSTEXPR constexpr
#else
  #define FLATBUFFERS_CONSTEXPR
#endif

#if (defined(__cplusplus) && __cplusplus >= 201402L) || \
    (defined(__cpp_constexpr) && __cpp_constexpr >= 201304)
  #define FLATBUFFERS_CONSTEXPR_CPP14 FLATBUFFERS_CONSTEXPR
#else
  #define FLATBUFFERS_CONSTEXPR_CPP14
#endif

#if defined(__GXX_EXPERIMENTAL_CXX0X__) && __GNUC__ * 10 + __GNUC_MINOR__ >= 46 || \
    defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023026
  #define FLATBUFFERS_NOEXCEPT noexcept
#else
  #define FLATBUFFERS_NOEXCEPT
#endif

// NOTE: the FLATBUFFERS_DELETE_FUNC macro may change the access mode to
// private, so be sure to put it at the end or reset access mode explicitly.
#if (!defined(_MSC_VER) || _MSC_FULL_VER >= 180020827) && \
    (!defined(__GNUC__) || (__GNUC__ * 100 + __GNUC_MINOR__ >= 404))
  #define FLATBUFFERS_DELETE_FUNC(func) func = delete;
#else
  #define FLATBUFFERS_DELETE_FUNC(func) private: func;
#endif

#if defined(_MSC_VER)
  #pragma warning(push)
  #pragma warning(disable: 4127) // C4127: conditional expression is constant
#endif

#ifndef FLATBUFFERS_HAS_STRING_VIEW
  // Only provide flatbuffers::string_view if __has_include can be used
  // to detect a header that provides an implementation
  #if defined(__has_include)
    // Check for std::string_view (in c++17)
    #if __has_include(<string_view>) && (__cplusplus >= 201606 || _HAS_CXX17)
      #include <string_view>
      namespace flatbuffers {
        typedef std::string_view string_view;
      }
      #define FLATBUFFERS_HAS_STRING_VIEW 1
    // Check for std::experimental::string_view (in c++14, compiler-dependent)
    #elif __has_include(<experimental/string_view>) && (__cplusplus >= 201411)
      #include <experimental/string_view>
      namespace flatbuffers {
        typedef std::experimental::string_view string_view;
      }
      #define FLATBUFFERS_HAS_STRING_VIEW 1
    #endif
  #endif // __has_include
#endif // !FLATBUFFERS_HAS_STRING_VIEW

/// @endcond

/// @file
namespace flatbuffers {

/// @cond FLATBUFFERS_INTERNAL
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

// In 32bits, this evaluates to 2GB - 1
#define FLATBUFFERS_MAX_BUFFER_SIZE ((1ULL << (sizeof(soffset_t) * 8 - 1)) - 1)

// We support aligning the contents of buffers up to this size.
#define FLATBUFFERS_MAX_ALIGNMENT 16

template<typename T> T EndianSwap(T t) {
  #if defined(_MSC_VER)
    #define FLATBUFFERS_BYTESWAP16 _byteswap_ushort
    #define FLATBUFFERS_BYTESWAP32 _byteswap_ulong
    #define FLATBUFFERS_BYTESWAP64 _byteswap_uint64
  #else
    #if defined(__GNUC__) && __GNUC__ * 100 + __GNUC_MINOR__ < 408 && !defined(__clang__)
      // __builtin_bswap16 was missing prior to GCC 4.8.
      #define FLATBUFFERS_BYTESWAP16(x) \
        static_cast<uint16_t>(__builtin_bswap32(static_cast<uint32_t>(x) << 16))
    #else
      #define FLATBUFFERS_BYTESWAP16 __builtin_bswap16
    #endif
    #define FLATBUFFERS_BYTESWAP32 __builtin_bswap32
    #define FLATBUFFERS_BYTESWAP64 __builtin_bswap64
  #endif
  if (sizeof(T) == 1) {   // Compile-time if-then's.
    return t;
  } else if (sizeof(T) == 2) {
    union { T t; uint16_t i; } u;
    u.t = t;
    u.i = FLATBUFFERS_BYTESWAP16(u.i);
    return u.t;
  } else if (sizeof(T) == 4) {
    union { T t; uint32_t i; } u;
    u.t = t;
    u.i = FLATBUFFERS_BYTESWAP32(u.i);
    return u.t;
  } else if (sizeof(T) == 8) {
    union { T t; uint64_t i; } u;
    u.t = t;
    u.i = FLATBUFFERS_BYTESWAP64(u.i);
    return u.t;
  } else {
    FLATBUFFERS_ASSERT(0);
  }
}


template<typename T> T EndianScalar(T t) {
  #if FLATBUFFERS_LITTLEENDIAN
    return t;
  #else
    return EndianSwap(t);
  #endif
}

template<typename T> T ReadScalar(const void *p) {
  return EndianScalar(*reinterpret_cast<const T *>(p));
}

template<typename T> void WriteScalar(void *p, T t) {
  *reinterpret_cast<T *>(p) = EndianScalar(t);
}

// Computes how many bytes you'd have to pad to be able to write an
// "scalar_size" scalar if the buffer had grown to "buf_size" (downwards in
// memory).
inline size_t PaddingBytes(size_t buf_size, size_t scalar_size) {
  return ((~buf_size) + 1) & (scalar_size - 1);
}

}  // namespace flatbuffers
#endif  // FLATBUFFERS_BASE_H_
