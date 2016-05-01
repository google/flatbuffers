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
#include "bench.pb.h"

using namespace benchpb;
using namespace std;

struct PBBench : Bench {
  void Encode(void *buf, size_t &len) {
    FooBarContainer foobarcontainer;
    const int veclen = 3;
    for (int i = 0; i < veclen; i++) {
      auto foobar = foobarcontainer.add_list();
      // We add + i to not make these identical copies for a more realistic
      // compression test.
      foobar->set_name("Hello, World!");
      foobar->set_rating(3.1415432432445543543 + i);
      foobar->set_postfix('!' + i);
      auto bar = foobar->mutable_sibling();
      bar->set_time(123456 + i);
      bar->set_ratio(3.14159f + i);
      bar->set_size(10000 + i);
      auto foo = bar->mutable_parent();
      foo->set_id(0xABADCAFEABADCAFE + i);
      foo->set_count(10000 + i);
      foo->set_prefix('@' + i);
      foo->set_length(1000000 + i);
    }
    foobarcontainer.set_location("http://google.com/flatbuffers/");
    foobarcontainer.set_initialized(true);
    foobarcontainer.set_fruit(Bananas);
    string s;
    foobarcontainer.SerializeToString(&s);
    assert(len >= s.length());
    len = s.length();
    memcpy(buf, s.c_str(), len);
  }

  void *Decode(void *buf, size_t len)
  {
    auto foobarcontainer = new FooBarContainer;
    foobarcontainer->ParseFromArray(buf, (int)len);
    return foobarcontainer;
  }

  int64_t Use(void *decoded) {
    auto foobarcontainer = (FooBarContainer *)decoded;
    sum = 0;
    Add(foobarcontainer->initialized());
    Add(foobarcontainer->location().length());
    Add(foobarcontainer->fruit());
    for (int i = 0; i < foobarcontainer->list().size(); i++) {
      auto &foobar = foobarcontainer->list().Get(i);
      Add(foobar.name().length());
      Add(foobar.postfix());
      Add(static_cast<int64_t>(foobar.rating()));
      auto &bar = foobar.sibling();
      Add(static_cast<int64_t>(bar.ratio()));
      Add(bar.size());
      Add(bar.time());
      auto &foo = bar.parent();
      Add(foo.count());
      Add(foo.id());
      Add(foo.length());
      Add(foo.prefix());
    }
    return sum;
  }

  void Dealloc(void *decoded)
  {
    delete (FooBarContainer *)decoded;
  }

  void ShutDown() {
    google::protobuf::ShutdownProtobufLibrary();
  }
};

void PBShutdownReally() { google::protobuf::ShutdownProtobufLibrary(); }

Bench *NewPBBench() { return new PBBench(); }
