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

#pragma once

#define _CRT_SECURE_NO_WARNINGS 1

//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <stdio.h>
#include <tchar.h>
#include <cstdint>
#include <stdlib.h>
#include <string>
#include <vector>
#include <cstdio>
#include "assert.h"

#include "zlib.h"

struct Bench {
  int64_t sum;

  virtual ~Bench() {}

  void Add(int64_t x) {
    sum += x;
    //printf("%I64d ", x);
  }

  virtual void Encode(void *buf, size_t &len) = 0;
  virtual void *Decode(void *buf, size_t len) = 0;
  virtual int64_t Use(void *decoded) = 0;
  virtual void Dealloc(void *decoded) = 0;

  virtual void Init() {}
  virtual void ShutDown() {}
};
