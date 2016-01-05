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

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"
#include "rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;

struct JSONBench : Bench {
  void Encode(void *buf, size_t &len) {
    const int veclen = 3;
    Document &d = *new Document();  // FIXME: leaking on purpose, since it
                                    // crashes in destructor
    auto alloc = d.GetAllocator();
    d.SetObject();
    Value list;
    list.SetArray();
    for (int i = 0; i < veclen; i++) {
      // We add + i to not make these identical copies for a more realistic
      // compression test.
      Value foobar;
      foobar.SetObject();
      foobar.AddMember("name", "Hello, World!", alloc);
      foobar.AddMember("rating", 3.1415432432445543543 + i, alloc);
      foobar.AddMember("postfix", '!' + i, alloc);
      Value bar;
      bar.SetObject();
      bar.AddMember("time", 123456 + i, alloc);
      bar.AddMember("ratio", 3.14159f + i, alloc);
      bar.AddMember("size", 10000 + i, alloc);
      Value foo;
      foo.SetObject();
      foo.AddMember("id", 0xABADCAFEABADCAFEL + i, alloc);
      foo.AddMember("count", 10000 + i, alloc);
      foo.AddMember("prefix", '@' + i, alloc);
      foo.AddMember("length", 1000000 + i, alloc);
      bar.AddMember("parent", foo, alloc);
      foobar.AddMember("sibling", bar, alloc);
      list.PushBack(foobar, alloc);
    }
    d.AddMember("initialized", true, alloc);
    d.AddMember("location", "http://google.com/flatbuffers/", alloc);
    d.AddMember("fruit", 2, alloc); // FIXME: better way to do enums?
    d.AddMember("list", list, alloc);
    GenericStringBuffer<UTF8<char>> gsb;
    PrettyWriter<GenericStringBuffer<UTF8<char>>> writer(gsb);
    d.Accept(writer);	// Accept() traverses the DOM and fires Handler events.
    assert(len >= gsb.GetSize() + 1);
    len = gsb.GetSize();
    memcpy(buf, gsb.GetString(), len + 1);
  }

  void *Decode(void *buf, size_t len) {
    auto d = new Document();
    auto err = d->Parse<0>((char *)buf).HasParseError();
    assert(!err);
    return d;
  }

  int64_t Use(void *decoded) {
    auto &foobarcontainer = *(Document *)decoded;
    sum = 0;
    // FIXME: this is not a fair representation of code size or speed,
    // since any real code would need to test for HasMember/IsInt etc. and
    // do error handling.
    Add(foobarcontainer["initialized"].GetBool());
    Add(foobarcontainer["location"].GetStringLength());
    Add(foobarcontainer["fruit"].GetInt());
    for (unsigned int i = 0; i < foobarcontainer["list"].Size(); i++) {
      auto &foobar = foobarcontainer["list"][i];
      Add(foobar["name"].GetStringLength());
      Add(foobar["postfix"].GetInt());
      Add(static_cast<int64_t>(foobar["rating"].GetDouble()));
      auto &bar = foobar["sibling"];
      Add(static_cast<int64_t>(bar["ratio"].GetDouble()));
      Add(bar["size"].GetInt());
      Add(bar["time"].GetInt());
      auto &foo = bar["parent"];
      Add(foo["count"].GetInt());
      Add(foo["id"].GetUint64());
      Add(foo["length"].GetInt());
      Add(foo["prefix"].GetInt());
    }
    return sum;
  }

  void Dealloc(void *decoded) {
    delete (Document *)decoded;
  }
};

Bench *NewJSONBench() { return new JSONBench(); }
