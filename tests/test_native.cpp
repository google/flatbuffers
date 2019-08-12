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

int TopologicalSortTests() {
  std::vector<int> vertex {1, 2, 3, 4};
  flatbuffers::doTopologicalSort(vertex.begin(), vertex.end(), [] (auto a, auto b) {
    // hard coded dependencies
    if ((a == 4 && b == 2) || (a == 4 && b == 3) || (a == 3 && b == 1) || (a == 2 && b == 1))
      return true;
    else
      return false;
  }
  );

  TEST_EQ(vertex[0], 4);
  TEST_EQ(vertex[1], 2);
  TEST_EQ(vertex[2], 3);
  TEST_EQ(vertex[3], 1);
  return 0;
}

void CompileTest() {
  TestN::Native::Foo foo;
  foo.enumData = TestN::BundleSize_Size3;

  for (auto i = 0; i < 3; ++i) {
    auto c = std::complex<double>(static_cast<double>(i), 1.);
    foo.iqData.push_back(c);
  }

  foo.variant = MyMat(1, {std::complex<double>(static_cast<double>(42), 1.)});

  flatbuffers::FlatBufferBuilder fbb;
  fbb.ForceDefaults(true);
  fbb.Finish(Native::Pack(fbb, foo));

  // debugging + access to serialized data
  auto s = flatbuffers::FlatBufferToString(fbb.GetBufferPointer(),
                                           TestN::FooTypeTable());

  TEST_EQ_STR(s.c_str(), "{ enumData: Size3, bitData: { rows: 0 }, iqData: [ { i: 0.0, q: 1.0 }, { i: 1.0, q: 1.0 }, { i: 2.0, q: 1.0 } ], iqSample: { i: 0.0, q: 0.0 }, iqSample2: { i: 0.0, q: 0.0 }, newInt: 0, variant_type: Mat, variant: { rows: 1, data: [ { i: 42.0, q: 1.0 } ] } }");
}

void TestNamespaces() {
  TestN::NamespaceFoo::Native::Foo foofoo;
  flatbuffers::FlatBufferBuilder fbbfoo;
  fbbfoo.Finish(Native::Pack(fbbfoo, foofoo));

  flatbuffers::FlatBufferBuilder fbbbar;
  TestN::NamespaceBar::Native::Foo barfoo;
  fbbbar.Finish(Native::Pack(fbbbar, barfoo));
}

int FlatBufferTests() {
  TopologicalSortTests();
  CompileTest();
  TestNamespaces();

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
