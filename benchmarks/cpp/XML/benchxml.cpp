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

/*

This xml parser selected from:
http://stackoverflow.com/questions/170686/best-open-xml-parser-for-c

According to the site: "Extremely fast non-validating XML parser which
constructs the DOM tree from an XML file/buffer"

Looked at RapidXML as well, but doesn't have a way to access fields by name
and convert them to right type.


*/


#include "stdafx.h"

#include "pugixml-1.4/src/pugixml.hpp"
#include "flatbuffers/util.h"

using namespace std;
using namespace pugi;

struct XMLBench : Bench {
  void Encode(void *buf, size_t &len) {
    const int veclen = 3;
    xml_document d;
    auto list = d.append_child("list");
    for (int i = 0; i < veclen; i++) {
      // We add + i to not make these identical copies for a more realistic
      // compression test.
      auto foobar = list.append_child("foobar");
      foobar.append_child("name").append_child(pugi::node_pcdata).
        set_value("Hello, World!");
      foobar.append_child("rating").append_child(pugi::node_pcdata).
        set_value(flatbuffers::NumToString(3.1415432432445543543 + i).c_str());
      foobar.append_child("postfix").append_child(pugi::node_pcdata).
        set_value(flatbuffers::NumToString('!' + i).c_str());
      auto bar = foobar.append_child("sibling");
      bar.append_child("time").append_child(pugi::node_pcdata).
        set_value(flatbuffers::NumToString(123456 + i).c_str());
      bar.append_child("ratio").append_child(pugi::node_pcdata).
        set_value(flatbuffers::NumToString(3.14159f + i).c_str());
      bar.append_child("size").append_child(pugi::node_pcdata).
        set_value(flatbuffers::NumToString(10000 + i).c_str());
      auto foo = bar.append_child("parent");
      foo.append_child("id").append_child(pugi::node_pcdata).
        set_value(flatbuffers::NumToString(0xABADCAFEABADCAFEL + i).c_str());
      foo.append_child("count").append_child(pugi::node_pcdata).
        set_value(flatbuffers::NumToString(10000 + i).c_str());
      foo.append_child("prefix").append_child(pugi::node_pcdata).
        set_value(flatbuffers::NumToString('@' + i).c_str());
      foo.append_child("length").append_child(pugi::node_pcdata).
        set_value(flatbuffers::NumToString(1000000 + i).c_str());
    }
    d.append_child("initialized").append_child(pugi::node_pcdata).
      set_value(flatbuffers::NumToString(true).c_str());
    d.append_child("location").append_child(pugi::node_pcdata).
      set_value("http://google.com/flatbuffers/");
    // FIXME: better way to do enums?
    d.append_child("fruit").append_child(pugi::node_pcdata).
      set_value(flatbuffers::NumToString(2).c_str());
    std::stringstream ss;
    d.save(ss);
    assert(len >= ss.str().length() + 1);
    len = ss.str().length();
    memcpy(buf, ss.str().c_str(), len + 1);
  }

  void *Decode(void *buf, size_t len) {
    auto d = new xml_document();
    auto result = d->load_buffer(buf, len);
    assert(result);
    return d;
  }

  int64_t Use(void *decoded) {
    auto &foobarcontainer = *(xml_document *)decoded;
    sum = 0;
    // FIXME: this is not a fair representation of code size or speed,
    // since any real code would need to check if the string is of the desired
    // type and do error handling.
    Add(flatbuffers::StringToInt(foobarcontainer.child("initialized").
      first_child().value()));
    Add(strlen(foobarcontainer.child("location").first_child().value()));
    Add(flatbuffers::StringToInt(foobarcontainer.child("fruit").first_child().
      value()));
    auto list = foobarcontainer.child("list");
    for (pugi::xml_node foobar = list.child("foobar"); foobar;
         foobar = foobar.next_sibling("foobar")) {
      Add(strlen(foobar.child("name").first_child().value()));
      Add(flatbuffers::StringToInt(foobar.child("postfix").first_child().
        value()));
      Add(flatbuffers::StringToInt(foobar.child("rating").first_child().
        value()));
      auto &bar = foobar.child("sibling");
      Add(flatbuffers::StringToInt(bar.child("ratio").first_child().value()));
      Add(flatbuffers::StringToInt(bar.child("size").first_child().value()));
      Add(flatbuffers::StringToInt(bar.child("time").first_child().value()));
      auto &foo = bar.child("parent");
      Add(flatbuffers::StringToInt(foo.child("count").first_child().value()));
      Add(flatbuffers::StringToInt(foo.child("id").first_child().value()));
      Add(flatbuffers::StringToInt(foo.child("length").first_child().value()));
      Add(flatbuffers::StringToInt(foo.child("prefix").first_child().value()));
    }
    return sum;
  }

  void Dealloc(void *decoded) {
    delete (xml_document *)decoded;
  }
};

Bench *NewXMLBench() { return new XMLBench(); }
