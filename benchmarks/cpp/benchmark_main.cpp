#include <benchmark/benchmark.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "benchmarks/cpp/bench.h"
#include "benchmarks/cpp/flatbuffers/fb_bench.h"
#include "benchmarks/cpp/raw/raw_bench.h"

static inline void Encode(benchmark::State& state,
                          std::unique_ptr<Bench>& bench, uint8_t* buffer) {
  int64_t length;
  for (auto _ : state) {
    bench->Encode(buffer, length);
    benchmark::DoNotOptimize(length);
  }
}

static inline void Decode(benchmark::State& state,
                          std::unique_ptr<Bench>& bench, uint8_t* buffer) {
  int64_t length;
  uint8_t* encoded = bench->Encode(buffer, length);

  for (auto _ : state) {
    void* decoded = bench->Decode(encoded, length);
    benchmark::DoNotOptimize(decoded);
  }
}

static inline void Use(benchmark::State& state, std::unique_ptr<Bench>& bench,
                       uint8_t* buffer, int64_t check_sum) {
  int64_t length;
  uint8_t* encoded = bench->Encode(buffer, length);
  void* decoded = bench->Decode(encoded, length);

  int64_t sum = 0;

  for (auto _ : state) {
    sum = bench->Use(decoded);
  }

  EXPECT_EQ(sum, check_sum);
}

static void BM_Flatbuffers_SharedStringHit(benchmark::State& state) {
  const std::string value(static_cast<size_t>(state.range(0)), 'x');
  flatbuffers::FlatBufferBuilder builder(value.size() + 8);
  builder.CreateSharedString(value.data(), value.size());
  for (auto _ : state) {
    benchmark::DoNotOptimize(
        builder.CreateSharedString(value.data(), value.size()).o);
  }
  state.SetBytesProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Flatbuffers_SharedStringHit)
    ->Arg(8)
    ->Arg(64)
    ->Arg(1024)
    ->Arg(65536);

static void BM_Flatbuffers_SharedStringMiss(benchmark::State& state) {
  std::vector<std::string> values;
  for (int i = 0; i < 1024; ++i) {
    values.push_back(std::to_string(i) + std::string(64, 'x'));
  }
  flatbuffers::FlatBufferBuilder builder;
  for (auto _ : state) {
    builder.Clear();
    for (const auto& value : values) {
      benchmark::DoNotOptimize(
          builder.CreateSharedString(value.data(), value.size()).o);
    }
  }
  state.SetItemsProcessed(state.iterations() *
                          static_cast<int64_t>(values.size()));
}
BENCHMARK(BM_Flatbuffers_SharedStringMiss);

static void BM_Flatbuffers_Encode(benchmark::State& state) {
  const int64_t kBufferLength = 1024;
  uint8_t buffer[kBufferLength];

  StaticAllocator allocator(&buffer[0]);
  std::unique_ptr<Bench> bench = NewFlatBuffersBench(kBufferLength, &allocator);
  Encode(state, bench, buffer);
}
BENCHMARK(BM_Flatbuffers_Encode);

static void BM_Flatbuffers_Decode(benchmark::State& state) {
  const int64_t kBufferLength = 1024;
  uint8_t buffer[kBufferLength];

  StaticAllocator allocator(&buffer[0]);
  std::unique_ptr<Bench> bench = NewFlatBuffersBench(kBufferLength, &allocator);
  Decode(state, bench, buffer);
}
BENCHMARK(BM_Flatbuffers_Decode);

static void BM_Flatbuffers_Use(benchmark::State& state) {
  const int64_t kBufferLength = 1024;
  uint8_t buffer[kBufferLength];

  StaticAllocator allocator(&buffer[0]);
  std::unique_ptr<Bench> bench = NewFlatBuffersBench(kBufferLength, &allocator);
  Use(state, bench, buffer, 218812692406581874);
}
BENCHMARK(BM_Flatbuffers_Use);

static void BM_Raw_Encode(benchmark::State& state) {
  const int64_t kBufferLength = 1024;
  uint8_t buffer[kBufferLength];

  std::unique_ptr<Bench> bench = NewRawBench();
  Encode(state, bench, buffer);
}
BENCHMARK(BM_Raw_Encode);

static void BM_Raw_Decode(benchmark::State& state) {
  const int64_t kBufferLength = 1024;
  uint8_t buffer[kBufferLength];

  std::unique_ptr<Bench> bench = NewRawBench();
  Decode(state, bench, buffer);
}
BENCHMARK(BM_Raw_Decode);

static void BM_Raw_Use(benchmark::State& state) {
  const int64_t kBufferLength = 1024;
  uint8_t buffer[kBufferLength];

  std::unique_ptr<Bench> bench = NewRawBench();
  Use(state, bench, buffer, 218812692406581874);
}
BENCHMARK(BM_Raw_Use);
