/*
 * Copyright 2017 Google Inc. All rights reserved.
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

#ifndef FLATBUFFERS_STL_EMULATION_H_
#define FLATBUFFERS_STL_EMULATION_H_

// clang-format off
#include "flatbuffers/base.h"

#include <string>
#include <type_traits>
#include <vector>
#include <memory>
#include <limits>

// Detect C++17 compatible compiler.
// __cplusplus >= 201703L - a compiler has support of 'static inline' variables.
#if defined(FLATBUFFERS_USE_STD_OPTIONAL) \
    || (defined(__cplusplus) && __cplusplus >= 201703L) \
    || (defined(_MSVC_LANG) &&  (_MSVC_LANG >= 201703L))
  #include <optional>
  #ifndef FLATBUFFERS_USE_STD_OPTIONAL
    #define FLATBUFFERS_USE_STD_OPTIONAL
  #endif
#endif

#if defined(_STLPORT_VERSION) && !defined(FLATBUFFERS_CPP98_STL)
  #define FLATBUFFERS_CPP98_STL
#endif  // defined(_STLPORT_VERSION) && !defined(FLATBUFFERS_CPP98_STL)

#if defined(FLATBUFFERS_CPP98_STL)
  #include <cctype>
#endif  // defined(FLATBUFFERS_CPP98_STL)

// This header provides backwards compatibility for C++98 STLs like stlport.
namespace flatbuffers {

// Retrieve ::back() from a string in a way that is compatible with pre C++11
// STLs (e.g stlport).
inline char& string_back(std::string &value) {
  return value[value.length() - 1];
}

inline char string_back(const std::string &value) {
  return value[value.length() - 1];
}

// Helper method that retrieves ::data() from a vector in a way that is
// compatible with pre C++11 STLs (e.g stlport).
template <typename T> inline T *vector_data(std::vector<T> &vector) {
  // In some debug environments, operator[] does bounds checking, so &vector[0]
  // can't be used.
  return vector.empty() ? nullptr : &vector[0];
}

template <typename T> inline const T *vector_data(
    const std::vector<T> &vector) {
  return vector.empty() ? nullptr : &vector[0];
}

template <typename T, typename V>
inline void vector_emplace_back(std::vector<T> *vector, V &&data) {
  #if defined(FLATBUFFERS_CPP98_STL)
    vector->push_back(data);
  #else
    vector->emplace_back(std::forward<V>(data));
  #endif  // defined(FLATBUFFERS_CPP98_STL)
}

#ifndef FLATBUFFERS_CPP98_STL
  #if defined(FLATBUFFERS_TEMPLATES_ALIASES)
    template <typename T>
    using numeric_limits = std::numeric_limits<T>;
  #else
    template <typename T> class numeric_limits :
      public std::numeric_limits<T> {};
  #endif  // defined(FLATBUFFERS_TEMPLATES_ALIASES)
#else
  template <typename T> class numeric_limits :
      public std::numeric_limits<T> {
    public:
      // Android NDK fix.
      static T lowest() {
        return std::numeric_limits<T>::min();
      }
  };

  template <> class numeric_limits<float> :
      public std::numeric_limits<float> {
    public:
      static float lowest() { return -FLT_MAX; }
  };

  template <> class numeric_limits<double> :
      public std::numeric_limits<double> {
    public:
      static double lowest() { return -DBL_MAX; }
  };

  template <> class numeric_limits<unsigned long long> {
   public:
    static unsigned long long min() { return 0ULL; }
    static unsigned long long max() { return ~0ULL; }
    static unsigned long long lowest() {
      return numeric_limits<unsigned long long>::min();
    }
  };

  template <> class numeric_limits<long long> {
   public:
    static long long min() {
      return static_cast<long long>(1ULL << ((sizeof(long long) << 3) - 1));
    }
    static long long max() {
      return static_cast<long long>(
          (1ULL << ((sizeof(long long) << 3) - 1)) - 1);
    }
    static long long lowest() {
      return numeric_limits<long long>::min();
    }
  };
#endif  // FLATBUFFERS_CPP98_STL

#if defined(FLATBUFFERS_TEMPLATES_ALIASES)
  #ifndef FLATBUFFERS_CPP98_STL
    template <typename T> using is_scalar = std::is_scalar<T>;
    template <typename T, typename U> using is_same = std::is_same<T,U>;
    template <typename T> using is_floating_point = std::is_floating_point<T>;
    template <typename T> using is_unsigned = std::is_unsigned<T>;
    template <typename T> using is_enum = std::is_enum<T>;
    template <typename T> using make_unsigned = std::make_unsigned<T>;
    template<bool B, class T, class F>
    using conditional = std::conditional<B, T, F>;
    template<class T, T v>
    using integral_constant = std::integral_constant<T, v>;
  #else
    // Map C++ TR1 templates defined by stlport.
    template <typename T> using is_scalar = std::tr1::is_scalar<T>;
    template <typename T, typename U> using is_same = std::tr1::is_same<T,U>;
    template <typename T> using is_floating_point =
        std::tr1::is_floating_point<T>;
    template <typename T> using is_unsigned = std::tr1::is_unsigned<T>;
    template <typename T> using is_enum = std::tr1::is_enum<T>;
    // Android NDK doesn't have std::make_unsigned or std::tr1::make_unsigned.
    template<typename T> struct make_unsigned {
      static_assert(is_unsigned<T>::value, "Specialization not implemented!");
      using type = T;
    };
    template<> struct make_unsigned<char> { using type = unsigned char; };
    template<> struct make_unsigned<short> { using type = unsigned short; };
    template<> struct make_unsigned<int> { using type = unsigned int; };
    template<> struct make_unsigned<long> { using type = unsigned long; };
    template<>
    struct make_unsigned<long long> { using type = unsigned long long; };
    template<bool B, class T, class F>
    using conditional = std::tr1::conditional<B, T, F>;
    template<class T, T v>
    using integral_constant = std::tr1::integral_constant<T, v>;
  #endif  // !FLATBUFFERS_CPP98_STL
#else
  // MSVC 2010 doesn't support C++11 aliases.
  template <typename T> struct is_scalar : public std::is_scalar<T> {};
  template <typename T, typename U> struct is_same : public std::is_same<T,U> {};
  template <typename T> struct is_floating_point :
        public std::is_floating_point<T> {};
  template <typename T> struct is_unsigned : public std::is_unsigned<T> {};
  template <typename T> struct is_enum : public std::is_enum<T> {};
  template <typename T> struct make_unsigned : public std::make_unsigned<T> {};
  template<bool B, class T, class F>
  struct conditional : public std::conditional<B, T, F> {};
  template<class T, T v>
  struct integral_constant : public std::integral_constant<T, v> {};
#endif  // defined(FLATBUFFERS_TEMPLATES_ALIASES)

#ifndef FLATBUFFERS_CPP98_STL
  #if defined(FLATBUFFERS_TEMPLATES_ALIASES)
    template <class T> using unique_ptr = std::unique_ptr<T>;
  #else
    // MSVC 2010 doesn't support C++11 aliases.
    // We're manually "aliasing" the class here as we want to bring unique_ptr
    // into the flatbuffers namespace.  We have unique_ptr in the flatbuffers
    // namespace we have a completely independent implementation (see below)
    // for C++98 STL implementations.
    template <class T> class unique_ptr : public std::unique_ptr<T> {
     public:
      unique_ptr() {}
      explicit unique_ptr(T* p) : std::unique_ptr<T>(p) {}
      unique_ptr(std::unique_ptr<T>&& u) { *this = std::move(u); }
      unique_ptr(unique_ptr&& u) { *this = std::move(u); }
      unique_ptr& operator=(std::unique_ptr<T>&& u) {
        std::unique_ptr<T>::reset(u.release());
        return *this;
      }
      unique_ptr& operator=(unique_ptr&& u) {
        std::unique_ptr<T>::reset(u.release());
        return *this;
      }
      unique_ptr& operator=(T* p) {
        return std::unique_ptr<T>::operator=(p);
      }
    };
  #endif  // defined(FLATBUFFERS_TEMPLATES_ALIASES)
#else
  // Very limited implementation of unique_ptr.
  // This is provided simply to allow the C++ code generated from the default
  // settings to function in C++98 environments with no modifications.
  template <class T> class unique_ptr {
   public:
    typedef T element_type;

    unique_ptr() : ptr_(nullptr) {}
    explicit unique_ptr(T* p) : ptr_(p) {}
    unique_ptr(unique_ptr&& u) : ptr_(nullptr) { reset(u.release()); }
    unique_ptr(const unique_ptr& u) : ptr_(nullptr) {
      reset(const_cast<unique_ptr*>(&u)->release());
    }
    ~unique_ptr() { reset(); }

    unique_ptr& operator=(const unique_ptr& u) {
      reset(const_cast<unique_ptr*>(&u)->release());
      return *this;
    }

    unique_ptr& operator=(unique_ptr&& u) {
      reset(u.release());
      return *this;
    }

    unique_ptr& operator=(T* p) {
      reset(p);
      return *this;
    }

    const T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    T* get() const noexcept { return ptr_; }
    explicit operator bool() const { return ptr_ != nullptr; }

    // modifiers
    T* release() {
      T* value = ptr_;
      ptr_ = nullptr;
      return value;
    }

    void reset(T* p = nullptr) {
      T* value = ptr_;
      ptr_ = p;
      if (value) delete value;
    }

    void swap(unique_ptr& u) {
      T* temp_ptr = ptr_;
      ptr_ = u.ptr_;
      u.ptr_ = temp_ptr;
    }

   private:
    T* ptr_;
  };

  template <class T> bool operator==(const unique_ptr<T>& x,
                                     const unique_ptr<T>& y) {
    return x.get() == y.get();
  }

  template <class T, class D> bool operator==(const unique_ptr<T>& x,
                                              const D* y) {
    return static_cast<D*>(x.get()) == y;
  }

  template <class T> bool operator==(const unique_ptr<T>& x, intptr_t y) {
    return reinterpret_cast<intptr_t>(x.get()) == y;
  }

  template <class T> bool operator!=(const unique_ptr<T>& x, decltype(nullptr)) {
    return !!x;
  }

  template <class T> bool operator!=(decltype(nullptr), const unique_ptr<T>& x) {
    return !!x;
  }

  template <class T> bool operator==(const unique_ptr<T>& x, decltype(nullptr)) {
    return !x;
  }

  template <class T> bool operator==(decltype(nullptr), const unique_ptr<T>& x) {
    return !x;
  }

#endif  // !FLATBUFFERS_CPP98_STL

#ifdef FLATBUFFERS_USE_STD_OPTIONAL
template<class T>
using Optional = std::optional<T>;
using nullopt_t = std::nullopt_t;
inline constexpr nullopt_t nullopt = std::nullopt;

#else
// Limited implementation of Optional<T> type for a scalar T.
// This implementation limited by trivial types compatible with
// std::is_arithmetic<T> or std::is_enum<T> type traits.

// A tag to indicate an empty flatbuffers::optional<T>.
struct nullopt_t {
  explicit FLATBUFFERS_CONSTEXPR_CPP11 nullopt_t(int) {}
};

#if defined(FLATBUFFERS_CONSTEXPR_DEFINED)
  namespace internal {
    template <class> struct nullopt_holder {
      static constexpr nullopt_t instance_ = nullopt_t(0);
    };
    template<class Dummy>
    constexpr nullopt_t nullopt_holder<Dummy>::instance_;
  }
  static constexpr const nullopt_t &nullopt = internal::nullopt_holder<void>::instance_;

#else
  namespace internal {
    template <class> struct nullopt_holder {
      static const nullopt_t instance_;
    };
    template<class Dummy>
    const nullopt_t nullopt_holder<Dummy>::instance_  = nullopt_t(0);
  }
  static const nullopt_t &nullopt = internal::nullopt_holder<void>::instance_;

#endif

template<class T>
class Optional FLATBUFFERS_FINAL_CLASS {
  // Non-scalar 'T' would extremely complicated Optional<T>.
  // Use is_scalar<T> checking because flatbuffers flatbuffers::is_arithmetic<T>
  // isn't implemented.
  static_assert(flatbuffers::is_scalar<T>::value, "unexpected type T");

 public:
  ~Optional() {}

  FLATBUFFERS_CONSTEXPR_CPP11 Optional() FLATBUFFERS_NOEXCEPT
    : value_(), has_value_(false) {}

  FLATBUFFERS_CONSTEXPR_CPP11 Optional(nullopt_t) FLATBUFFERS_NOEXCEPT
    : value_(), has_value_(false) {}

  FLATBUFFERS_CONSTEXPR_CPP11 Optional(T val) FLATBUFFERS_NOEXCEPT
    : value_(val), has_value_(true) {}

  FLATBUFFERS_CONSTEXPR_CPP11 Optional(const Optional &other) FLATBUFFERS_NOEXCEPT
    : value_(other.value_), has_value_(other.has_value_) {}

  FLATBUFFERS_CONSTEXPR_CPP14 Optional &operator=(const Optional &other) FLATBUFFERS_NOEXCEPT {
    value_ = other.value_;
    has_value_ = other.has_value_;
    return *this;
  }

  FLATBUFFERS_CONSTEXPR_CPP14 Optional &operator=(nullopt_t) FLATBUFFERS_NOEXCEPT {
    value_ = T();
    has_value_ = false;
    return *this;
  }

  FLATBUFFERS_CONSTEXPR_CPP14 Optional &operator=(T val) FLATBUFFERS_NOEXCEPT {
    value_ = val;
    has_value_ = true;
    return *this;
  }

  void reset() FLATBUFFERS_NOEXCEPT {
    *this = nullopt;
  }

  void swap(Optional &other) FLATBUFFERS_NOEXCEPT {
    std::swap(value_, other.value_);
    std::swap(has_value_, other.has_value_);
  }

  FLATBUFFERS_CONSTEXPR_CPP11 FLATBUFFERS_EXPLICIT_CPP11 operator bool() const FLATBUFFERS_NOEXCEPT {
    return has_value_;
  }

  FLATBUFFERS_CONSTEXPR_CPP11 bool has_value() const FLATBUFFERS_NOEXCEPT {
    return has_value_;
  }

  FLATBUFFERS_CONSTEXPR_CPP11 const T& operator*() const FLATBUFFERS_NOEXCEPT {
    return value_;
  }

  const T& value() const {
    FLATBUFFERS_ASSERT(has_value());
    return value_;
  }

  T value_or(T default_value) const FLATBUFFERS_NOEXCEPT {
    return has_value() ? value_ : default_value;
  }

 private:
  T value_;
  bool has_value_;
};

template<class T>
FLATBUFFERS_CONSTEXPR_CPP11 bool operator==(const Optional<T>& opt, nullopt_t) FLATBUFFERS_NOEXCEPT {
  return !opt;
}
template<class T>
FLATBUFFERS_CONSTEXPR_CPP11 bool operator==(nullopt_t, const Optional<T>& opt) FLATBUFFERS_NOEXCEPT {
  return !opt;
}

template<class T, class U>
FLATBUFFERS_CONSTEXPR_CPP11 bool operator==(const Optional<T>& lhs, const U& rhs) FLATBUFFERS_NOEXCEPT {
  return static_cast<bool>(lhs) && (*lhs == rhs);
}

template<class T, class U>
FLATBUFFERS_CONSTEXPR_CPP11 bool operator==(const T& lhs, const Optional<U>& rhs) FLATBUFFERS_NOEXCEPT {
  return static_cast<bool>(rhs) && (lhs == *rhs);
}

template<class T, class U>
FLATBUFFERS_CONSTEXPR_CPP11 bool operator==(const Optional<T>& lhs, const Optional<U>& rhs) FLATBUFFERS_NOEXCEPT {
  return static_cast<bool>(lhs) != static_cast<bool>(rhs)
              ? false
              : !static_cast<bool>(lhs) ? false : (*lhs == *rhs);
}
#endif // FLATBUFFERS_USE_STD_OPTIONAL

}  // namespace flatbuffers

#endif  // FLATBUFFERS_STL_EMULATION_H_
