#include "key_field_test.h"

#include <iostream>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "key_field/key_field_sample_generated.h"
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
  auto baz_vec = fbb.CreateVectorOfSortedStructs(&bazs);

  auto test_string = fbb.CreateString("TEST");

  float test_float_array1[3] = { 1.5, 2.5, 0 };
  float test_float_array2[3] = { 7.5, 2.5, 0 };
  float test_float_array3[3] = { 1.5, 2.5, -1 };
  float test_float_array4[3] = { -1.5, 2.5, 0 };
  std::vector<Bar> bars;
  bars.push_back(Bar(flatbuffers::make_span(test_float_array1), 3));
  bars.push_back(Bar(flatbuffers::make_span(test_float_array2), 4));
  bars.push_back(Bar(flatbuffers::make_span(test_float_array3), 2));
  bars.push_back(Bar(flatbuffers::make_span(test_float_array4), 1));
  auto bar_vec = fbb.CreateVectorOfSortedStructs(&bars);


  auto t = CreateFooTable(fbb, 1, 2, test_string, baz_vec, bar_vec);
  fbb.Finish(t);

  uint8_t *buf = fbb.GetBufferPointer();
  auto foo_table = GetFooTable(buf);

  auto sorted_baz_vec = foo_table->d();
  TEST_EQ(sorted_baz_vec->Get(0)->b(), 1);
  TEST_EQ(sorted_baz_vec->Get(3)->b(), 4);

  uint8_t test_array[4];
  auto* key_array = &flatbuffers::CastToArray(test_array);
  key_array->CopyFromSpan(flatbuffers::make_span(test_array1));


  TEST_NOTNULL(
      sorted_baz_vec->LookupByKey(key_array));
  TEST_EQ(
      sorted_baz_vec->LookupByKey(key_array)->b(),
      4);
  uint8_t array_int[4] = { 7, 2, 3, 0 };
  key_array->CopyFromSpan(flatbuffers::make_span(array_int));
  TEST_EQ(sorted_baz_vec->LookupByKey(key_array),
          static_cast<const Baz *>(nullptr));

  auto sorted_bar_vec = foo_table->e();
  TEST_EQ(sorted_bar_vec->Get(0)->b(), 1);
  TEST_EQ(sorted_bar_vec->Get(3)->b(), 4);

  float test_float_array[3];
  auto* key_float_array = &flatbuffers::CastToArray(test_float_array);
  key_float_array->CopyFromSpan(flatbuffers::make_span(test_float_array1));
  TEST_NOTNULL(sorted_bar_vec->LookupByKey(key_float_array));
  TEST_EQ(sorted_bar_vec->LookupByKey(key_float_array)->b(), 3);
  float array_float[3] = { -1, -2, -3 };
  key_float_array->CopyFromSpan(flatbuffers::make_span(array_float));
  TEST_EQ(sorted_bar_vec->LookupByKey(key_float_array),
          static_cast<const Bar *>(nullptr));
}

}  // namespace tests
}  // namespace flatbuffers
