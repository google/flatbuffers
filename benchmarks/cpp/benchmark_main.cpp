#include <benchmark/benchmark.h>
#include <gtest/gtest.h>

#include "benchmarks/cpp/bench.h"
#include "benchmarks/cpp/flatbuffers/fb_bench.h"

static void BM_Flatbuffers_Encode(benchmark::State &state) {
  const int64_t kBufferLength = 1024;
  uint8_t buffer[kBufferLength];

  int64_t length;

  StaticAllocator allocator(&buffer[0]);
  std::unique_ptr<Bench> bench = NewFlatBuffersBench(kBufferLength, &allocator);

  for (auto _ : state) {
    bench->Encode(buffer, length);
    benchmark::DoNotOptimize(length);
  }
}
BENCHMARK(BM_Flatbuffers_Encode);

static void BM_Flatbuffers_Decode(benchmark::State &state) {
  const int64_t kBufferLength = 1024;
  uint8_t buffer[kBufferLength];

  int64_t length;

  StaticAllocator allocator(&buffer[0]);
  std::unique_ptr<Bench> bench = NewFlatBuffersBench(kBufferLength, &allocator);

  uint8_t* encoded = bench->Encode(buffer, length);
  
  for (auto _ : state) {
    void* decoded = bench->Decode(encoded, length);
    benchmark::DoNotOptimize(decoded);
  }
}
BENCHMARK(BM_Flatbuffers_Decode);

static void BM_Flatbuffers_Use(benchmark::State &state) {
  const int64_t kBufferLength = 1024;
  uint8_t buffer[kBufferLength];

  int64_t length;

  StaticAllocator allocator(&buffer[0]);
  std::unique_ptr<Bench> bench = NewFlatBuffersBench(kBufferLength, &allocator);

  uint8_t* encoded = bench->Encode(buffer, length);
  void* decoded = bench->Decode(encoded, length);

  int64_t sum = 0;

  for (auto _ : state) {
    sum = bench->Use(decoded);
  }

  EXPECT_EQ(sum , 218812692406581874);
}
BENCHMARK(BM_Flatbuffers_Use);