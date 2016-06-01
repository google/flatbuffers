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
#include "bench_generated.h"
#include "flatbuffers/idl.h"

using namespace flatbuffers;
using namespace benchfb;

struct FJBench : Bench {

  Parser *parser;
  char *wire_buf;
  size_t wire_len;

  FJBench() : parser(NULL), wire_buf(NULL) {}
  ~FJBench() { if (parser) delete parser; if (wire_buf) delete wire_buf; }

  void Init() {
    // pre-load a parser with a schema, ready to parse JSON
    // we do not consider schema loading to be part of the benchmark time
    // for data loading, because realistically if you were loading tons of
    // data, you'd have the schema for that loaded at start-up
    static const char *schema =
      "namespace benchfb;"
      "enum Enum : short { Apples, Pears, Bananas }"
      "struct Foo { id:ulong; count:short; prefix:byte; length:uint; }"
      "struct Bar { parent:Foo; time:int; ratio:float; size:ushort; }"
      "table FooBar { sibling:Bar; name:string; rating:double; postfix:ubyte; }"
      "table FooBarContainer { list:[FooBar]; initialized:bool; fruit:Enum;"
      "location:string; }  root_type FooBarContainer;";
    parser = new Parser();
    auto ok = parser->Parse(schema);
    assert(ok);

    // use the other benchmark to generate a data block from which we can
    // run Encode() below. This make sense, since FlatBuffers only way to
    // generate a JSON wire format is from a binary buffer, not thru a DOM
    Bench *NewFBBench();
    auto bench = NewFBBench();
    wire_len = 10000;
    wire_buf = new char[wire_len];
    bench->Encode(wire_buf, wire_len);
    delete bench;
  }

  void Encode(void *buf, size_t &len) {
    /*
    static const char *json =
      "{ initialized: true, location: \"http://google.com/flatbuffers/\","
      "fruit: 2, list: [ { name: \"Hello, World!\", rating: 3.14154,"
      "postfix: 33, sibling: { parent: { id: -6075977127102133506,"
      "count: 10000, prefix: 64, length: 1000000 }, time: 123456,"
      "ratio: 3.14159, size: 10000 } }, { name: \"Hello, World!\","
      "rating: 4.14154, postfix: 34, sibling: {"
      "parent: { id: -6075977127102133505, count: 10001, prefix: 65,"
      "length: 1000001 }, time: 123457, ratio: 4.14159, size: 10001 } }, {"
      "name: \"Hello, World!\", rating: 5.14154, postfix: 35, sibling: {"
      "parent: { id: -6075977127102133504, count: 10002, prefix: 66,"
      "length: 1000002 }, time: 123458, ratio: 5.14159, size: 10002 } } ] } ";
    auto size = strlen(json);
    assert(len >= size + 1);
    len = size;
    memcpy(buf, json, len + 1);
    */
    std::string s;
    GenerateText(*parser, wire_buf, &s);
    assert(len >= s.length() + 1);
    len = s.length();
    memcpy(buf, s.c_str(), len + 1);
  }


  void *Decode(void *buf, size_t len)
  {
    auto ok = parser->Parse((char *)buf);
    assert(ok);
    auto mem = malloc(parser->builder_.GetSize());
    memcpy(mem, parser->builder_.GetBufferPointer(),
           parser->builder_.GetSize());
    return mem;
  }

  int64_t Use(void *decoded) {
    auto foobarcontainer = GetFooBarContainer(decoded);
    sum = 0;
    Add(foobarcontainer->initialized());
    Add(foobarcontainer->location()->Length());
    Add(foobarcontainer->fruit());
    for (unsigned int i = 0; i < foobarcontainer->list()->Length(); i++) {
      auto foobar = foobarcontainer->list()->Get(i);
      Add(foobar->name()->Length());
      Add(foobar->postfix());
      Add(static_cast<int64_t>(foobar->rating()));
      auto bar = foobar->sibling();
      Add(static_cast<int64_t>(bar->ratio()));
      Add(bar->size());
      Add(bar->time());
      auto &foo = bar->parent();
      Add(foo.count());
      Add(foo.id());
      Add(foo.length());
      Add(foo.prefix());
    }
    return sum;
  }

  void Dealloc(void *decoded) { free(decoded); }
};

Bench *NewFJBench() { return new FJBench(); }
