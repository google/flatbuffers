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

#ifndef FLATBUFFERS_UTIL_H_
#define FLATBUFFERS_UTIL_H_

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <fstream>
#include <iomanip>
#ifndef FLATBUFFERS_PREFER_PRINTF
#  include <sstream>
#else // FLATBUFFERS_PREFER_PRINTF
#  include <float.h>
#  include <stdio.h>
#endif // FLATBUFFERS_PREFER_PRINTF
#include <string>
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>  // Must be included before <direct.h>
#  include <direct.h>
#  include <winbase.h>
#  undef interface  // This is also important because of reasons
#else
#  include <limits.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>

#include "flatbuffers/base.h"

namespace flatbuffers {

// @locale-independent functions for ASCII characters set.

// Check that integer scalar is in closed range: (a <= x <= b).
template<typename T> static inline bool check_in_range(T x, T a, T b) {
  // (Hacker's Delight): `a <= x <= b` <=> `(x-a) <={u} (b-a)`.
  static_assert(std::is_integral<T>::value, "Integral required.");
  FLATBUFFERS_ASSERT(a <= b);  // static_assert only if 'a' & 'b' templated
  typedef typename std::make_unsigned<T>::type U;
  return (static_cast<U>(x - a) <= static_cast<U>(b - a));
}

// Check (case-insensitive) that `c` is alphabetic in closed range [a, b].
static inline bool is_alpha_range(char c, char a, char b) {
  FLATBUFFERS_ASSERT((('a' <= a) && (a <= 'z')) || (('A' <= a) && (a <= 'Z')));
  // ASCII only: alpha to upper case => reset bit 0x20 (~0x20 = 0xDF).
  return check_in_range(c & 0xDF, a & 0xDF, b & 0xDF);
}

// Check (case-insensitive) that `c` == `x`.
static inline bool is_alpha_char(char c, char x) {
  return is_alpha_range(c, x, x);
}

// Case-insensitive isalpha
static inline bool is_alpha(char c) { return is_alpha_range(c, 'A', 'Z'); }

// https://en.cppreference.com/w/cpp/string/byte/isxdigit
// isdigit and isxdigit are the only standard narrow character classification
// functions that are not affected by the currently installed C locale. although
// some implementations (e.g. Microsoft in 1252 codepage) may classify
// additional single-byte characters as digits.
static inline bool is_digit(char c) { return check_in_range(c, '0', '9'); }

static inline bool is_xdigit(char c) {
  // Is look-up table better?
  return check_in_range(c, '0', '9') || is_alpha_range(c, 'A', 'F');
}

// Case-insensitive isalnum
static inline bool is_alnum(char c) { return is_alpha(c) || is_digit(c); }
// @end-locale-independent functions for ASCII character set

#ifdef FLATBUFFERS_PREFER_PRINTF
template<typename T> size_t IntToDigitCount(T t) {
  size_t digit_count = 0;
  // Count the sign for negative numbers
  if (t < 0) digit_count++;
  // Count a single 0 left of the dot for fractional numbers
  if (-1 < t && t < 1) digit_count++;
  // Count digits until fractional part
  T eps = std::numeric_limits<float>::epsilon();
  while (t <= (-1 + eps) || (1 - eps) <= t) {
    t /= 10;
    digit_count++;
  }
  return digit_count;
}

template<typename T> size_t NumToStringWidth(T t, int precision = 0) {
  size_t string_width = IntToDigitCount(t);
  // Count the dot for floating point numbers
  if (precision) string_width += (precision + 1);
  return string_width;
}

template<typename T> std::string NumToStringImplWrapper(T t, const char* fmt,
                                                        int precision = 0) {
  size_t string_width = NumToStringWidth(t, precision);
  std::string s(string_width, 0x00);
  // Allow snprintf to use std::string trailing null to detect buffer overflow
  snprintf(const_cast<char*>(s.data()), (s.size()+1), fmt, precision, t);
  return s;
}
#endif // FLATBUFFERS_PREFER_PRINTF

// Convert an integer or floating point value to a string.
// In contrast to std::stringstream, "char" values are
// converted to a string of digits, and we don't use scientific notation.
template<typename T> std::string NumToString(T t) {
  // clang-format off
  #ifndef FLATBUFFERS_PREFER_PRINTF
    std::stringstream ss;
    ss << t;
    return ss.str();
  #else // FLATBUFFERS_PREFER_PRINTF
    auto v = static_cast<long long>(t);
    return NumToStringImplWrapper(v, "%.*lld");
  #endif // FLATBUFFERS_PREFER_PRINTF
  // clang-format on
}
// Avoid char types used as character data.
template<> inline std::string NumToString<signed char>(signed char t) {
  return NumToString(static_cast<int>(t));
}
template<> inline std::string NumToString<unsigned char>(unsigned char t) {
  return NumToString(static_cast<int>(t));
}
#if defined(FLATBUFFERS_CPP98_STL)
template<> inline std::string NumToString<long long>(long long t) {
  char buf[21];  // (log((1 << 63) - 1) / log(10)) + 2
  snprintf(buf, sizeof(buf), "%lld", t);
  return std::string(buf);
}

template<>
inline std::string NumToString<unsigned long long>(unsigned long long t) {
  char buf[22];  // (log((1 << 63) - 1) / log(10)) + 1
  snprintf(buf, sizeof(buf), "%llu", t);
  return std::string(buf);
}
#endif  // defined(FLATBUFFERS_CPP98_STL)

// Special versions for floats/doubles.
template<typename T> std::string FloatToString(T t, int precision) {
  // clang-format off
  #ifndef FLATBUFFERS_PREFER_PRINTF
    // to_string() prints different numbers of digits for floats depending on
    // platform and isn't available on Android, so we use stringstream
    std::stringstream ss;
    // Use std::fixed to suppress scientific notation.
    ss << std::fixed;
    // Default precision is 6, we want that to be higher for doubles.
    ss << std::setprecision(precision);
    ss << t;
    auto s = ss.str();
  #else // FLATBUFFERS_PREFER_PRINTF
    auto v = static_cast<double>(t);
    auto s = NumToStringImplWrapper(v, "%0.*f", precision);
  #endif // FLATBUFFERS_PREFER_PRINTF
  // clang-format on
  // Sadly, std::fixed turns "1" into "1.00000", so here we undo that.
  auto p = s.find_last_not_of('0');
  if (p != std::string::npos) {
    // Strip trailing zeroes. If it is a whole number, keep one zero.
    s.resize(p + (s[p] == '.' ? 2 : 1));
  }
  return s;
}

template<> inline std::string NumToString<double>(double t) {
  return FloatToString(t, 12);
}
template<> inline std::string NumToString<float>(float t) {
  return FloatToString(t, 6);
}

// Convert an integer value to a hexadecimal string.
// The returned string length is always xdigits long, prefixed by 0 digits.
// For example, IntToStringHex(0x23, 8) returns the string "00000023".
inline std::string IntToStringHex(int i, int xdigits) {
  FLATBUFFERS_ASSERT(i >= 0);
  // clang-format off
  #ifndef FLATBUFFERS_PREFER_PRINTF
    std::stringstream ss;
    ss << std::setw(xdigits) << std::setfill('0') << std::hex << std::uppercase
       << i;
    return ss.str();
  #else // FLATBUFFERS_PREFER_PRINTF
    return NumToStringImplWrapper(i, "%.*X", xdigits);
  #endif // FLATBUFFERS_PREFER_PRINTF
  // clang-format on
}

static inline double strtod_impl(const char *str, char **str_end){
  // Result of strtod (printf, etc) depends from current C-locale.
  return strtod(str, str_end);
}

static inline float strtof_impl(const char *str, char **str_end)
{
// Use "strtof" for float and strtod for double to avoid double=>float rounding
// problems (see https://en.cppreference.com/w/cpp/numeric/fenv/feround) or
// problems with std::numeric_limits<float>::is_iec559==false.
// Example:
//  for (int mode : { FE_DOWNWARD, FE_TONEAREST, FE_TOWARDZERO, FE_UPWARD }){
//    const char *s = "-4e38";
//    std::fesetround(mode);
//    std::cout << strtof(s, nullptr) << "; " << strtod(s, nullptr) << "; "
//              << static_cast<float>(strtod(s, nullptr)) << "\n";
//  }
// Gives:
//  -inf; -4e+38; -inf
//  -inf; -4e+38; -inf
//  -inf; -4e+38; -3.40282e+38
//  -inf; -4e+38; -3.40282e+38

  // clang-format off
  #ifdef FLATBUFFERS_HAS_NEW_STRTOD
    return strtof(str, str_end);
  #else
    return static_cast<float>(strtod_impl(str, str_end));
  #endif // !FLATBUFFERS_HAS_NEW_STRTOD
  // clang-format on
}

// Adaptor for strtoull()/strtoll().
// Flatbuffers accepts numbers with any count of leading zeros (-009 is -9),
// while strtoll with base=0 interprets first leading zero as octal prefix.
// In future, it is possible to add prefixed 0b0101.
// 1) Checks errno code for overflow condition (out of range).
// 2) If base <= 0, function try to detect base of number by prefix.
//
// Return value (like strtoull and strtoll, but reject partial result):
// - If successful, an integer value corresponding to the str is returned.
// - If full string conversion can't be performed, ​0​ is returned.
// - If the converted value falls out of range of corresponding return type, a
// range error occurs. In this case value MAX(T)/MIN(T) is returned.
template<typename T>
inline T StringToInteger64Impl(const char *const str, const char **endptr,
                               const int base, const bool check_errno = true) {
  static_assert(
      std::is_same<T, int64_t>::value || std::is_same<T, uint64_t>::value,
      "Type T must be either int64_t or uint64_t");
  FLATBUFFERS_ASSERT(str && endptr);  // endptr must be not null
  if (base <= 0) {
    auto s = str;
    while (*s && !is_digit(*s)) s++;
    if (s[0] == '0' && is_alpha_char(s[1], 'X'))
      return StringToInteger64Impl<T>(str, endptr, 16, check_errno);
    // if a prefix not match, try base=10
    return StringToInteger64Impl<T>(str, endptr, 10, check_errno);
  } else {
    if (check_errno) errno = 0;  // clear thread-local errno
    // calculate result
    T result;
      // clang-format off
    #if defined(_MSC_VER)
      #pragma warning(push)
      #pragma warning(disable: 4127) // C4127: expression is constant
    #endif
    if (std::is_same<T, int64_t>::value) {
    #if defined(_MSC_VER)
      #pragma warning(pop)
    #endif
      #ifdef _MSC_VER
      result = _strtoi64(str, const_cast<char**>(endptr), base);
      #else
      result = strtoll(str, const_cast<char**>(endptr), base);
      #endif
      // clang-format on
    } else {  // T is uint64_t
      // clang-format off
      #ifdef _MSC_VER
      result = _strtoui64(str, const_cast<char**>(endptr), base);
      #else
      result = strtoull(str, const_cast<char**>(endptr), base);
      #endif
      // clang-format on

      // The strtoull accepts negative numbers:
      // If the minus sign was part of the input sequence, the numeric value
      // calculated from the sequence of digits is negated as if by unary minus
      // in the result type, which applies unsigned integer wraparound rules.
      // Fix this behaviour (except -0).
      if ((**endptr == '\0') && (0 != result)) {
        auto s = str;
        while (*s && !is_digit(*s)) s++;
        s = (s > str) ? (s - 1) : s;  // step back to one symbol
        if (*s == '-') {
          // For unsigned types return max to distinguish from
          // "no conversion can be performed".
          result = flatbuffers::numeric_limits<T>::max();
          // point to the start of string, like errno
          *endptr = str;
        }
      }
    }
    // check for overflow
    if (check_errno && errno) *endptr = str; // point it to start of input
    // erase partial result, but save an overflow
    if ((*endptr != str) && (**endptr != '\0')) result = 0;
    return result;
  }
}

// Convert a string to an instance of T.
// Return value (matched with StringToInteger64Impl and strtod):
// - If successful, a numeric value corresponding to the str is returned.
// - If full string conversion can't be performed, ​0​ is returned.
// - If the converted value falls out of range of corresponding return type, a
// range error occurs. In this case value MAX(T)/MIN(T) is returned.
template<typename T> inline bool StringToNumber(const char *s, T *val) {
  FLATBUFFERS_ASSERT(s && val);
  const char *end = nullptr;
  // The errno check isn't needed. strtoll will return MAX/MIN on overlow.
  const int64_t i = StringToInteger64Impl<int64_t>(s, &end, -1, false);
  *val = static_cast<T>(i);
  const auto done = (s != end) && (*end == '\0');
  if (done) {
    const int64_t max = flatbuffers::numeric_limits<T>::max();
    const int64_t min = flatbuffers::numeric_limits<T>::lowest();
    if (i > max) {
      *val = static_cast<T>(max);
      return false;
    }
    if (i < min) {
      // For unsigned types return max to distinguish from
      // "no conversion can be performed" when 0 is returned.
      *val = static_cast<T>(std::is_signed<T>::value ? min : max);
      return false;
    }
  }
  return done;
}
template<> inline bool StringToNumber<int64_t>(const char *s, int64_t *val) {
  const char *end = s;  // request errno checking
  *val = StringToInteger64Impl<int64_t>(s, &end, -1);
  return (s != end) && (*end == '\0');
}
template<> inline bool StringToNumber<uint64_t>(const char *s, uint64_t *val) {
  const char *end = s;  // request errno checking
  *val = StringToInteger64Impl<uint64_t>(s, &end, -1);
  return (s != end) && (*end == '\0');
}

template<> inline bool StringToNumber<double>(const char *s, double *val) {
  FLATBUFFERS_ASSERT(s && val);
  char *end = nullptr;
  *val = strtod_impl(s, &end);
  auto done = (s != end) && (*end == '\0');
  if (!done) *val = 0;  // erase partial result
  return done;
}

template<> inline bool StringToNumber<float>(const char *s, float *val) {
  FLATBUFFERS_ASSERT(s && val);
  char *end = nullptr;
  *val = strtof_impl(s, &end);
  auto done = (s != end) && (*end == '\0');
  if (!done) *val = 0;  // erase partial result
  return done;
}

inline int64_t StringToInt(const char *str, const char **endptr = nullptr,
                           int base = 10) {
  const char *ep = nullptr;
  return StringToInteger64Impl<int64_t>(str, endptr ? endptr : &ep, base);
}

inline uint64_t StringToUInt(const char *str, const char **endptr = nullptr,
                             int base = 10) {
  const char *ep = nullptr;
  return StringToInteger64Impl<uint64_t>(str, endptr ? endptr : &ep, base);
}

typedef bool (*LoadFileFunction)(const char *filename, bool binary,
                                 std::string *dest);
typedef bool (*FileExistsFunction)(const char *filename);

LoadFileFunction SetLoadFileFunction(LoadFileFunction load_file_function);

FileExistsFunction SetFileExistsFunction(
    FileExistsFunction file_exists_function);

// Check if file "name" exists.
bool FileExists(const char *name);

// Check if "name" exists and it is also a directory.
bool DirExists(const char *name);

// Load file "name" into "buf" returning true if successful
// false otherwise.  If "binary" is false data is read
// using ifstream's text mode, otherwise data is read with
// no transcoding.
bool LoadFile(const char *name, bool binary, std::string *buf);

// Save data "buf" of length "len" bytes into a file
// "name" returning true if successful, false otherwise.
// If "binary" is false data is written using ifstream's
// text mode, otherwise data is written with no
// transcoding.
inline bool SaveFile(const char *name, const char *buf, size_t len,
                     bool binary) {
  std::ofstream ofs(name, binary ? std::ofstream::binary : std::ofstream::out);
  if (!ofs.is_open()) return false;
  ofs.write(buf, len);
  return !ofs.bad();
}

// Save data "buf" into file "name" returning true if
// successful, false otherwise.  If "binary" is false
// data is written using ifstream's text mode, otherwise
// data is written with no transcoding.
inline bool SaveFile(const char *name, const std::string &buf, bool binary) {
  return SaveFile(name, buf.c_str(), buf.size(), binary);
}

// Functionality for minimalistic portable path handling.

// The functions below behave correctly regardless of whether posix ('/') or
// Windows ('/' or '\\') separators are used.

// Any new separators inserted are always posix.

// We internally store paths in posix format ('/'). Paths supplied
// by the user should go through PosixPath to ensure correct behavior
// on Windows when paths are string-compared.

static const char kPathSeparator = '/';
static const char kPathSeparatorWindows = '\\';
static const char *PathSeparatorSet = "\\/";  // Intentionally no ':'

// Returns the path with the extension, if any, removed.
inline std::string StripExtension(const std::string &filepath) {
  size_t i = filepath.find_last_of(".");
  return i != std::string::npos ? filepath.substr(0, i) : filepath;
}

// Returns the extension, if any.
inline std::string GetExtension(const std::string &filepath) {
  size_t i = filepath.find_last_of(".");
  return i != std::string::npos ? filepath.substr(i + 1) : "";
}

// Return the last component of the path, after the last separator.
inline std::string StripPath(const std::string &filepath) {
  size_t i = filepath.find_last_of(PathSeparatorSet);
  return i != std::string::npos ? filepath.substr(i + 1) : filepath;
}

// Strip the last component of the path + separator.
inline std::string StripFileName(const std::string &filepath) {
  size_t i = filepath.find_last_of(PathSeparatorSet);
  return i != std::string::npos ? filepath.substr(0, i) : "";
}

// Concatenates a path with a filename, regardless of wether the path
// ends in a separator or not.
inline std::string ConCatPathFileName(const std::string &path,
                                      const std::string &filename) {
  std::string filepath = path;
  if (filepath.length()) {
    char &filepath_last_character = string_back(filepath);
    if (filepath_last_character == kPathSeparatorWindows) {
      filepath_last_character = kPathSeparator;
    } else if (filepath_last_character != kPathSeparator) {
      filepath += kPathSeparator;
    }
  }
  filepath += filename;
  // Ignore './' at the start of filepath.
  if (filepath[0] == '.' && filepath[1] == kPathSeparator) {
    filepath.erase(0, 2);
  }
  return filepath;
}

// Replaces any '\\' separators with '/'
inline std::string PosixPath(const char *path) {
  std::string p = path;
  std::replace(p.begin(), p.end(), '\\', '/');
  return p;
}

// This function ensure a directory exists, by recursively
// creating dirs for any parts of the path that don't exist yet.
inline void EnsureDirExists(const std::string &filepath) {
  auto parent = StripFileName(filepath);
  if (parent.length()) EnsureDirExists(parent);
    // clang-format off
  #ifdef _WIN32
    (void)_mkdir(filepath.c_str());
  #else
    mkdir(filepath.c_str(), S_IRWXU|S_IRGRP|S_IXGRP);
  #endif
  // clang-format on
}

// Obtains the absolute path from any other path.
// Returns the input path if the absolute path couldn't be resolved.
inline std::string AbsolutePath(const std::string &filepath) {
  // clang-format off
  #ifdef FLATBUFFERS_NO_ABSOLUTE_PATH_RESOLUTION
    return filepath;
  #else
    #ifdef _WIN32
      char abs_path[MAX_PATH];
      return GetFullPathNameA(filepath.c_str(), MAX_PATH, abs_path, nullptr)
    #else
      char abs_path[PATH_MAX];
      return realpath(filepath.c_str(), abs_path)
    #endif
      ? abs_path
      : filepath;
  #endif // FLATBUFFERS_NO_ABSOLUTE_PATH_RESOLUTION
  // clang-format on
}

// To and from UTF-8 unicode conversion functions

// Convert a unicode code point into a UTF-8 representation by appending it
// to a string. Returns the number of bytes generated.
inline int ToUTF8(uint32_t ucc, std::string *out) {
  FLATBUFFERS_ASSERT(!(ucc & 0x80000000));  // Top bit can't be set.
  // 6 possible encodings: http://en.wikipedia.org/wiki/UTF-8
  for (int i = 0; i < 6; i++) {
    // Max bits this encoding can represent.
    uint32_t max_bits = 6 + i * 5 + static_cast<int>(!i);
    if (ucc < (1u << max_bits)) {  // does it fit?
      // Remaining bits not encoded in the first byte, store 6 bits each
      uint32_t remain_bits = i * 6;
      // Store first byte:
      (*out) += static_cast<char>((0xFE << (max_bits - remain_bits)) |
                                  (ucc >> remain_bits));
      // Store remaining bytes:
      for (int j = i - 1; j >= 0; j--) {
        (*out) += static_cast<char>(((ucc >> (j * 6)) & 0x3F) | 0x80);
      }
      return i + 1;  // Return the number of bytes added.
    }
  }
  FLATBUFFERS_ASSERT(0);  // Impossible to arrive here.
  return -1;
}

// Converts whatever prefix of the incoming string corresponds to a valid
// UTF-8 sequence into a unicode code. The incoming pointer will have been
// advanced past all bytes parsed.
// returns -1 upon corrupt UTF-8 encoding (ignore the incoming pointer in
// this case).
inline int FromUTF8(const char **in) {
  int len = 0;
  // Count leading 1 bits.
  for (int mask = 0x80; mask >= 0x04; mask >>= 1) {
    if (**in & mask) {
      len++;
    } else {
      break;
    }
  }
  if ((static_cast<unsigned char>(**in) << len) & 0x80) return -1;  // Bit after leading 1's must be 0.
  if (!len) return *(*in)++;
  // UTF-8 encoded values with a length are between 2 and 4 bytes.
  if (len < 2 || len > 4) { return -1; }
  // Grab initial bits of the code.
  int ucc = *(*in)++ & ((1 << (7 - len)) - 1);
  for (int i = 0; i < len - 1; i++) {
    if ((**in & 0xC0) != 0x80) return -1;  // Upper bits must 1 0.
    ucc <<= 6;
    ucc |= *(*in)++ & 0x3F;  // Grab 6 more bits of the code.
  }
  // UTF-8 cannot encode values between 0xD800 and 0xDFFF (reserved for
  // UTF-16 surrogate pairs).
  if (ucc >= 0xD800 && ucc <= 0xDFFF) { return -1; }
  // UTF-8 must represent code points in their shortest possible encoding.
  switch (len) {
    case 2:
      // Two bytes of UTF-8 can represent code points from U+0080 to U+07FF.
      if (ucc < 0x0080 || ucc > 0x07FF) { return -1; }
      break;
    case 3:
      // Three bytes of UTF-8 can represent code points from U+0800 to U+FFFF.
      if (ucc < 0x0800 || ucc > 0xFFFF) { return -1; }
      break;
    case 4:
      // Four bytes of UTF-8 can represent code points from U+10000 to U+10FFFF.
      if (ucc < 0x10000 || ucc > 0x10FFFF) { return -1; }
      break;
  }
  return ucc;
}

#ifndef FLATBUFFERS_PREFER_PRINTF
// Wraps a string to a maximum length, inserting new lines where necessary. Any
// existing whitespace will be collapsed down to a single space. A prefix or
// suffix can be provided, which will be inserted before or after a wrapped
// line, respectively.
inline std::string WordWrap(const std::string in, size_t max_length,
                            const std::string wrapped_line_prefix,
                            const std::string wrapped_line_suffix) {
  std::istringstream in_stream(in);
  std::string wrapped, line, word;

  in_stream >> word;
  line = word;

  while (in_stream >> word) {
    if ((line.length() + 1 + word.length() + wrapped_line_suffix.length()) <
        max_length) {
      line += " " + word;
    } else {
      wrapped += line + wrapped_line_suffix + "\n";
      line = wrapped_line_prefix + word;
    }
  }
  wrapped += line;

  return wrapped;
}
#endif // !FLATBUFFERS_PREFER_PRINTF

inline bool EscapeString(const char *s, size_t length, std::string *_text,
                         bool allow_non_utf8, bool natural_utf8) {
  std::string &text = *_text;
  text += "\"";
  for (uoffset_t i = 0; i < length; i++) {
    char c = s[i];
    switch (c) {
      case '\n': text += "\\n"; break;
      case '\t': text += "\\t"; break;
      case '\r': text += "\\r"; break;
      case '\b': text += "\\b"; break;
      case '\f': text += "\\f"; break;
      case '\"': text += "\\\""; break;
      case '\\': text += "\\\\"; break;
      default:
        if (c >= ' ' && c <= '~') {
          text += c;
        } else {
          // Not printable ASCII data. Let's see if it's valid UTF-8 first:
          const char *utf8 = s + i;
          int ucc = FromUTF8(&utf8);
          if (ucc < 0) {
            if (allow_non_utf8) {
              text += "\\x";
              text += IntToStringHex(static_cast<uint8_t>(c), 2);
            } else {
              // There are two cases here:
              //
              // 1) We reached here by parsing an IDL file. In that case,
              // we previously checked for non-UTF-8, so we shouldn't reach
              // here.
              //
              // 2) We reached here by someone calling GenerateText()
              // on a previously-serialized flatbuffer. The data might have
              // non-UTF-8 Strings, or might be corrupt.
              //
              // In both cases, we have to give up and inform the caller
              // they have no JSON.
              return false;
            }
          } else {
            if (natural_utf8) {
              // utf8 points to past all utf-8 bytes parsed
              text.append(s + i, static_cast<size_t>(utf8 - s - i));
            } else if (ucc <= 0xFFFF) {
              // Parses as Unicode within JSON's \uXXXX range, so use that.
              text += "\\u";
              text += IntToStringHex(ucc, 4);
            } else if (ucc <= 0x10FFFF) {
              // Encode Unicode SMP values to a surrogate pair using two \u
              // escapes.
              uint32_t base = ucc - 0x10000;
              auto high_surrogate = (base >> 10) + 0xD800;
              auto low_surrogate = (base & 0x03FF) + 0xDC00;
              text += "\\u";
              text += IntToStringHex(high_surrogate, 4);
              text += "\\u";
              text += IntToStringHex(low_surrogate, 4);
            }
            // Skip past characters recognized.
            i = static_cast<uoffset_t>(utf8 - s - 1);
          }
        }
        break;
    }
  }
  text += "\"";
  return true;
}

}  // namespace flatbuffers

#endif  // FLATBUFFERS_UTIL_H_
