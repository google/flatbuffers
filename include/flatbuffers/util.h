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

#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdlib.h>

namespace flatbuffers {

// Convert an integer or floating point value to a string.
// In contrast to std::stringstream, "char" values are
// converted to a string of digits.
template<typename T> std::string NumToString(T t) {
  // to_string() prints different numbers of digits for floats depending on
  // platform and isn't available on Android, so we use stringstream
  std::stringstream ss;
  ss << t;
  return ss.str();
}
// Avoid char types used as character data.
template<> inline std::string NumToString<signed char>(signed char t) {
  return NumToString(static_cast<int>(t));
}
template<> inline std::string NumToString<unsigned char>(unsigned char t) {
  return NumToString(static_cast<int>(t));
}

// Convert an integer value to a hexadecimal string.
// The returned string length is always xdigits long, prefixed by 0 digits.
// For example, IntToStringHex(0x23, 8) returns the string "00000023".
inline std::string IntToStringHex(int i, int xdigits) {
  std::stringstream ss;
  ss << std::setw(xdigits)
     << std::setfill('0')
     << std::hex
     << std::uppercase
     << i;
  return ss.str();
}

// Portable implementation of strtoull().
inline int64_t StringToInt(const char *str, int base = 10) {
  #ifdef _MSC_VER
    return _strtoui64(str, nullptr, base);
  #else
    return strtoull(str, nullptr, base);
  #endif
}

// Load file "name" into "buf" returning true if successful
// false otherwise.  If "binary" is false data is read
// using ifstream's text mode, otherwise data is read with
// no transcoding.
inline bool LoadFile(const char *name, bool binary, std::string *buf) {
  std::ifstream ifs(name, binary ? std::ifstream::binary : std::ifstream::in);
  if (!ifs.is_open()) return false;
  *buf = std::string(std::istreambuf_iterator<char>(ifs),
                    std::istreambuf_iterator<char>());
  return !ifs.bad();
}

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

// Functionality for minimalistic portable path handling:

static const char kPosixPathSeparator = '/';
#ifdef _WIN32
static const char kPathSeparator = '\\';
static const char *PathSeparatorSet = "\\:/";
#else
static const char kPathSeparator = kPosixPathSeparator;
static const char *PathSeparatorSet = "/";
#endif // _WIN32

inline std::string StripExtension(const std::string &filepath) {
  size_t i = filepath.find_last_of(".");
  return i != std::string::npos ? filepath.substr(0, i) : filepath;
}

inline std::string StripPath(const std::string &filepath) {
  size_t i = filepath.find_last_of(PathSeparatorSet);
  return i != std::string::npos ? filepath.substr(i + 1) : filepath;
}

inline std::string StripFileName(const std::string &filepath) {
  size_t i = filepath.find_last_of(PathSeparatorSet);
  return i != std::string::npos ? filepath.substr(0, i + 1) : "";
}

// To and from UTF-8 unicode conversion functions

// Convert a unicode code point into a UTF-8 representation by appending it
// to a string. Returns the number of bytes generated.
inline int ToUTF8(uint32_t ucc, std::string *out) {
  assert(!(ucc & 0x80000000));  // Top bit can't be set.
  // 6 possible encodings: http://en.wikipedia.org/wiki/UTF-8
  for (int i = 0; i < 6; i++) {
    // Max bits this encoding can represent.
    uint32_t max_bits = 6 + i * 5 + static_cast<int>(!i);
    if (ucc < (1 << max_bits)) {  // does it fit?
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
  assert(0);  // Impossible to arrive here.
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
  if ((**in << len) & 0x80) return -1;  // Bit after leading 1's must be 0.
  if (!len) return *(*in)++;
  // Grab initial bits of the code.
  int ucc = *(*in)++ & ((1 << (7 - len)) - 1);
  for (int i = 0; i < len - 1; i++) {
    if ((**in & 0xC0) != 0x80) return -1;  // Upper bits must 1 0.
    ucc <<= 6;
    ucc |= *(*in)++ & 0x3F;  // Grab 6 more bits of the code.
  }
  return ucc;
}

}  // namespace flatbuffers

#endif  // FLATBUFFERS_UTIL_H_
