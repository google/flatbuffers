/*
 * Copyright 2015 Google Inc. All rights reserved.
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

#include "stdafx.h"
#include "windows.h"
#include "assert.h"
#include "psapi.h"

LARGE_INTEGER freq, start;

void InitTime()
{
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
}

double SecondsSinceStart()
{
    LARGE_INTEGER end;
    QueryPerformanceCounter(&end);
    return double(end.QuadPart - start.QuadPart) / double(freq.QuadPart);
}

size_t DeflateTest(void *buf, size_t len) {
  auto outbuf = malloc(len * 2);
  uLongf size;
  auto err = compress((Bytef *)outbuf, &size, (Bytef *)buf, (uLong)len);
  assert(err == Z_OK);
  free(outbuf);
  return size;
}

void Run(Bench *bench, const char *name) {
  bench->Init();

  printf("=================================\n");

  const size_t bufsize = 10000;
  char buf[bufsize];
  size_t len;
  const int iterations = 1000;
  void *decoded[iterations];

  printf("%s bench start...\n", name);
  int64_t total = 0;
  double encode = 0, decode = 0, use = 0, dealloc = 0;

  // we use an outer loop also, since bumping up "iterations" to 10000 or so
  // puts so much strain on the allocator that use of free() dwarfs all
  // timings. Running the benchmark in batches gives more realistic timings and
  // keeps it accurate
  for (int j = 0; j < 1000; j++) {

    InitTime();

    double time1 = SecondsSinceStart();
    for (int i = 0; i < iterations; i++) {
      len = bufsize;
      bench->Encode(buf, len);
    }
    double time2 = SecondsSinceStart();
    #ifdef _CRTDBG_MAP_ALLOC
      _CrtMemState ms1;
      _CrtMemCheckpoint(&ms1);
    #endif
    double time3 = SecondsSinceStart();
    for (int i = 0; i < iterations; i++) {
      decoded[i] = bench->Decode(buf, len);
    }
    double time4 = SecondsSinceStart();
    #ifdef _CRTDBG_MAP_ALLOC
      _CrtMemState ms2;
      _CrtMemCheckpoint(&ms2);
      _CrtMemState msdiff;
      _CrtMemDifference(&msdiff, &ms1, &ms2);
      // This shows the amount of bytes & blocks needed for a decode,
      // also transient memory in totalcount
      if (!j) _CrtMemDumpStatistics(&msdiff);
    #endif
    /*
    PROCESS_MEMORY_COUNTERS pmc = { sizeof(PROCESS_MEMORY_COUNTERS) };
    auto ok = GetProcessMemoryInfo(GetCurrentProcess(), &pmc,
                                   sizeof(PROCESS_MEMORY_COUNTERS));
    assert(ok);
    */
    double time5 = SecondsSinceStart();
    for (int i = 0; i < iterations; i++) {
      auto result = bench->Use(decoded[i]);
      //printf("\n");
      assert(result == 218812692406581874);
      total += result;
    }
    double time6 = SecondsSinceStart();
    for (int i = 0; i < iterations; i++) {
      bench->Dealloc(decoded[i]);
    }
    double time7 = SecondsSinceStart();
    encode += time2 - time1;
    decode += time4 - time3;
    use += time6 - time5;
    dealloc += time7 - time6;
  }
  auto complen = DeflateTest(buf, len);
  // Ensure none of the code gets optimized out.
  printf("total = %I64d\n", total);
  printf("%s bench: %d wire size, %d compressed wire size\n", name, len, complen);
  printf("* %f encode time, %f decode time\n", encode, decode);
  printf("* %f use time, %f dealloc time\n", use, dealloc);
  printf("* %f decode/use/dealloc\n", decode + use + dealloc);
  //printf("* %d K paged pool\n", pmc.PagefileUsage / 1024);

  bench->ShutDown();
  delete bench;
}


int _tmain(int argc, _TCHAR* argv[])
{
  InitTime();

  Bench *NewRAWBench();  Run(NewRAWBench(),  "Raw structs");
  Bench *NewFBBench();   Run(NewFBBench(),   "FlatBuffers");
  Bench *NewFJBench();   Run(NewFJBench(),   "FlatBuffers JSON");
  Bench *NewPBBench();   Run(NewPBBench(),   "Protocol Buffers LITE");
  Bench *NewXMLBench();  Run(NewXMLBench(),  "pugixml");
  // Must be last, since it leaks.
  Bench *NewJSONBench(); Run(NewJSONBench(), "Rapid JSON");

  //Bench *NewPBBench(); Run(NewPBBench(), "MessagePack");
  //extern void PBShutdownReally();PBShutdownReally();
  //_CrtDumpMemoryLeaks();
  getchar();
  return 0;
}

