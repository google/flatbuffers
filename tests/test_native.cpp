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
#include <cmath>
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/minireflect.h"
#include "flatbuffers/registry.h"
#include "flatbuffers/util.h"

// clang-format off
#ifdef FLATBUFFERS_CPP98_STL
  #include "flatbuffers/stl_emulation.h"
  namespace std {
    using flatbuffers::unique_ptr;
  }
#endif
// clang-format on

#include "native_test_native.h"
#include "test_assert.h"

void CompileTest() {
  TestN::Native::Foo foo;
  foo.enumData = TestN::BundleSize_Size3;

  for (auto i = 0; i < 3; ++i) {
    auto c = std::complex<double>(static_cast<double>(i), 1.);
    foo.iqData.push_back(c);
  }

  flatbuffers::FlatBufferBuilder fbb;
  fbb.ForceDefaults(true);
  FinishFooBuffer(fbb, Native::Pack(fbb, foo));

  // debugging + access to serialized data
  auto s = flatbuffers::FlatBufferToString(fbb.GetBufferPointer(),
                                           TestN::FooTypeTable(), true);

  TEST_EQ_STR(s.c_str(), "sdf");
}


int FlatBufferTests() {

  CompileTest();

  return 0;
}

int main(int /*argc*/, const char * /*argv*/ []) {
  InitTestEngine();

  std::string req_locale;
  if (flatbuffers::ReadEnvironmentVariable("FLATBUFFERS_TEST_LOCALE",
                                          &req_locale)) {
    TEST_OUTPUT_LINE("The environment variable FLATBUFFERS_TEST_LOCALE=%s",
                     req_locale.c_str());
    req_locale = flatbuffers::RemoveStringQuotes(req_locale);
    std::string the_locale;
    TEST_ASSERT_FUNC(
        flatbuffers::SetGlobalTestLocale(req_locale.c_str(), &the_locale));
    TEST_OUTPUT_LINE("The global C-locale changed: %s", the_locale.c_str());
  }

  FlatBufferTests();

  if (!testing_fails) {
    TEST_OUTPUT_LINE("ALL TESTS PASSED");
  } else {
    TEST_OUTPUT_LINE("%d FAILED TESTS", testing_fails);
  }
  return CloseTestEngine();
}
