#include "key_field_test.h"

#include <iostream>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "key_field_sample_generated.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {

using namespace keyfield::sample;

void FixedSizedScalarKeyInStructTest() {
  flatbuffers::FlatBufferBuilder fbb;
  std::vector<Baz> bazs;
  uint8_t test_array1[4] = { 8, 2, 3, 0 };
  uint8_t test_array2[4] = { 1, 2, 3, 4 };
  uint8_t test_array3[4] = { 2, 2, 3, 4 };
  uint8_t test_array4[4] = { 3, 2, 3, 4 };
  bazs.push_back(Baz(flatbuffers::make_span(test_array1), 4));
  bazs.push_back(Baz(flatbuffers::make_span(test_array2), 1));
  bazs.push_back(Baz(flatbuffers::make_span(test_array3), 2));
  bazs.push_back(Baz(flatbuffers::make_span(test_array4), 3));
  auto vecofbaz = fbb.CreateVectorOfSortedStructs(&bazs);
  auto teststring = fbb.CreateString("TEST");
  float test_float_array1[3] = { 1.5, 2.5, 0 };
  float test_float_array2[3] = { 7.5, 2.5, 0 };
  float test_float_array3[3] = { 1.5, 2.5, -1 };
  float test_float_array4[3] = { -1.5, 2.5, 0 };
  std::vector<Bar> bars;
  bars.push_back(Bar(flatbuffers::make_span(test_float_array1), 3));
  bars.push_back(Bar(flatbuffers::make_span(test_float_array2), 4));
  bars.push_back(Bar(flatbuffers::make_span(test_float_array3), 2));
  bars.push_back(Bar(flatbuffers::make_span(test_float_array4), 1));
  auto vecofbar = fbb.CreateVectorOfSortedStructs(&bars);

  auto t = CreateFooTable(fbb, 1, 2, teststring, vecofbaz, vecofbar);
  fbb.Finish(t);

  uint8_t *buf = fbb.GetBufferPointer();
  auto footable = GetFooTable(buf);

  auto sortedvecofbaz = footable->d();
  TEST_EQ(sortedvecofbaz->Get(0)->b(), 1);
  TEST_EQ(sortedvecofbaz->Get(3)->b(), 4);
  TEST_NOTNULL(sortedvecofbaz->LookupByKey(test_array1));
  TEST_EQ(sortedvecofbaz->LookupByKey(test_array1)->b(), 4);
  uint8_t array_int[4] = { 7, 2, 3, 0 };
  TEST_EQ(sortedvecofbaz->LookupByKey(array_int),
          static_cast<const Baz *>(nullptr));

  auto sortedvecofbar = footable->e();
  TEST_EQ(sortedvecofbar->Get(0)->b(), 1);
  TEST_EQ(sortedvecofbar->Get(3)->b(), 4);
  TEST_NOTNULL(sortedvecofbar->LookupByKey(test_float_array1));
  TEST_EQ(sortedvecofbar->LookupByKey(test_float_array1)->b(), 3);
  float array_float[3] = { -1, -2, -3 };
  TEST_EQ(sortedvecofbar->LookupByKey(array_float),
          static_cast<const Bar *>(nullptr));
}

}  // namespace tests
}  // namespace flatbuffers
