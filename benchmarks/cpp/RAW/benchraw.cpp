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

// Compare against the fastest possible serialization: just write out naked
// structs. This is of course very apples to oranges, since this would be
// quite unusable as is in real world code, but good as a performance baseline.

// We have to hardcode everything, including the string length.
// 32 wastes a bit of memory but is overal a fair comparison.
#define STRING_LENGTH 32
#define VEC_LENGTH 3

enum Enum { Apples, Pears, Bananas };

struct Foo {
  int64_t id;
  short count;
  char prefix;
  int length;
};

struct Bar {
  Foo parent;
  int time;
  float ratio;
  unsigned short size;
};

struct FooBar {
  Bar sibling;
  // We have to stick this in, otherwise strlen() will make it slower than
  // FlatBuffers:
  int name_len;
  char name[STRING_LENGTH];
  double rating;
  unsigned char postfix;
};

struct FooBarContainer {
  FooBar list[VEC_LENGTH];  // 3 copies of the above
  bool initialized;
  Enum fruit;
  int location_len;
  char location[STRING_LENGTH];
};

struct RAWBench : Bench {
  void Encode(void *buf, size_t &len) {
    FooBarContainer fbc;
    strcpy(fbc.location, "http://google.com/flatbuffers/");  // Unsafe eek!
    fbc.location_len = (int)strlen(fbc.location);
    fbc.fruit = Bananas;
    fbc.initialized = true;
    for (int i = 0; i < VEC_LENGTH; i++) {
      // We add + i to not make these identical copies for a more realistic
      // compression test.
      auto &foobar = fbc.list[i];
      foobar.rating = 3.1415432432445543543 + i;
      foobar.postfix = '!' + i;
      strcpy(foobar.name, "Hello, World!");
      foobar.name_len = (int)strlen(foobar.name);
      auto &bar = foobar.sibling;
      bar.ratio = 3.14159f + i;
      bar.size = 10000 + i;
      bar.time = 123456 + i;
      auto &foo = bar.parent;
      foo.id = 0xABADCAFEABADCAFE + i;
      foo.count = 10000 + i;
      foo.length = 1000000 + i;
      foo.prefix = '@' + i;
    }
    assert(len >= sizeof(FooBarContainer));
    len = sizeof(FooBarContainer);
    memcpy(buf, &fbc, len);
  }

  void *Decode(void *buf, size_t len) { return buf; }

  int64_t Use(void *decoded) {
    auto foobarcontainer = (FooBarContainer *)decoded;
    sum = 0;
    Add(foobarcontainer->initialized);
    Add(foobarcontainer->location_len);
    Add(foobarcontainer->fruit);
    for (unsigned int i = 0; i < VEC_LENGTH; i++) {
      auto foobar = &foobarcontainer->list[i];
      Add(foobar->name_len);
      Add(foobar->postfix);
      Add(static_cast<int64_t>(foobar->rating));
      auto bar = &foobar->sibling;
      Add(static_cast<int64_t>(bar->ratio));
      Add(bar->size);
      Add(bar->time);
      auto &foo = bar->parent;
      Add(foo.count);
      Add(foo.id);
      Add(foo.length);
      Add(foo.prefix);
    }
    return sum;
  }

  void Dealloc(void *decoded) {}
};

Bench *NewRAWBench() { return new RAWBench(); }
