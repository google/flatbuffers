/*
 * Copyright 2016 Google Inc. All rights reserved.
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

#include <clocale>

#include "flatbuffers/util.h"

namespace flatbuffers {

bool FileExistsRaw(const char *name) {
  std::ifstream ifs(name);
  return ifs.good();
}

bool LoadFileRaw(const char *name, bool binary, std::string *buf) {
  if (DirExists(name)) return false;
  std::ifstream ifs(name, binary ? std::ifstream::binary : std::ifstream::in);
  if (!ifs.is_open()) return false;
  if (binary) {
    // The fastest way to read a file into a string.
    ifs.seekg(0, std::ios::end);
    auto size = ifs.tellg();
    (*buf).resize(static_cast<size_t>(size));
    ifs.seekg(0, std::ios::beg);
    ifs.read(&(*buf)[0], (*buf).size());
  } else {
    // This is slower, but works correctly on all platforms for text files.
    std::ostringstream oss;
    oss << ifs.rdbuf();
    *buf = oss.str();
  }
  return !ifs.bad();
}

static LoadFileFunction g_load_file_function = LoadFileRaw;
static FileExistsFunction g_file_exists_function = FileExistsRaw;

bool LoadFile(const char *name, bool binary, std::string *buf) {
  FLATBUFFERS_ASSERT(g_load_file_function);
  return g_load_file_function(name, binary, buf);
}

bool FileExists(const char *name) {
  FLATBUFFERS_ASSERT(g_file_exists_function);
  return g_file_exists_function(name);
}

bool DirExists(const char *name) {
  // clang-format off

  #ifdef _WIN32
    #define flatbuffers_stat _stat
    #define FLATBUFFERS_S_IFDIR _S_IFDIR
  #else
    #define flatbuffers_stat stat
    #define FLATBUFFERS_S_IFDIR S_IFDIR
  #endif
  // clang-format on
  struct flatbuffers_stat file_info;
  if (flatbuffers_stat(name, &file_info) != 0) return false;
  return (file_info.st_mode & FLATBUFFERS_S_IFDIR) != 0;
}

LoadFileFunction SetLoadFileFunction(LoadFileFunction load_file_function) {
  LoadFileFunction previous_function = g_load_file_function;
  g_load_file_function = load_file_function ? load_file_function : LoadFileRaw;
  return previous_function;
}

FileExistsFunction SetFileExistsFunction(
    FileExistsFunction file_exists_function) {
  FileExistsFunction previous_function = g_file_exists_function;
  g_file_exists_function =
      file_exists_function ? file_exists_function : FileExistsRaw;
  return previous_function;
}

// Locale-independent code.
#if defined(FLATBUFFERS_LOCALE_INDEPENDENT) && \
    (FLATBUFFERS_LOCALE_INDEPENDENT > 0)

// clang-format off
// Allocate locale instance at startup of application.
ClassicLocale ClassicLocale::instance_;

#ifdef _MSC_VER
  ClassicLocale::ClassicLocale()
    : locale_(_create_locale(LC_ALL, "C")) {}
  ClassicLocale::~ClassicLocale() { _free_locale(locale_); }
#else
  ClassicLocale::ClassicLocale()
    : locale_(newlocale(LC_ALL, "C", nullptr)) {}
  ClassicLocale::~ClassicLocale() { freelocale(locale_); }
#endif
// clang-format on

#endif  // !FLATBUFFERS_LOCALE_INDEPENDENT

std::string RemoveStringQuotes(const std::string &s) {
  auto ch = *s.c_str();
  return ((s.size() >= 2) && (ch == '\"' || ch == '\'') &&
          (ch == string_back(s)))
             ? s.substr(1, s.length() - 2)
             : s;
}

bool SetGlobalTestLocale(const char *locale_name, std::string *_value) {
  const auto the_locale = setlocale(LC_ALL, locale_name);
  if (!the_locale) return false;
  if (_value) *_value = std::string(the_locale);
  return true;
}

#ifdef _MSC_VER
#  pragma warning(disable : 4996)  // _CRT_SECURE_NO_WARNINGS
#endif
bool ReadEnvironmentVariable(const char *var_name, std::string *_value) {
  auto env_str = std::getenv(var_name);
  if (!env_str) return false;
  if (_value) *_value = std::string(env_str);
  return true;
}

}  // namespace flatbuffers
