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
// The returned string length is the number of nibbles in
// the supplied value prefixed by 0 digits.  For example,
// IntToStringHex(static_cast<int>(0x23)) returns the
// string "00000023".
template<typename T> std::string IntToStringHex(T i) {
  std::stringstream ss;
  ss << std::setw(sizeof(T) * 2)
     << std::setfill('0')
     << std::hex
     << std::uppercase
     << i;
  return ss.str();
}

// Portable implementation of strtoull().
inline int64_t StringToInt(const char *str) {
  #ifdef _MSC_VER
    return _strtoui64(str, nullptr, 10);
  #else
    return strtoull(str, nullptr, 10);
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

}  // namespace flatbuffers

#endif  // FLATBUFFERS_UTIL_H_
