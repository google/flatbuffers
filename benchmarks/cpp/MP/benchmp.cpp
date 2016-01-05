#include "stdafx.h"

//#include <msgpack.hpp>
#include <vector>
#include <string>
#include <iostream>
//#include <msgpack/type/tuple.hpp>
//#include <msgpack/type/define.hpp>
#include <assert.h>

using namespace std;

// FIXME: this doesn't do the same as what PB/FB do, in that I can only
// ever add fields at the end, and I can't leave out anything before the end.

enum { Apples, Pears, Bananas };

// comment this out for now, as it doesn't work.

/*

struct Foo {
  uint64_t id;
  short count;
  char prefix;
  unsigned int length;
  //This doesn't seem to work in VS2010:
  //MSGPACK_DEFINE(id, count, prefix, length);
  //You'd have to write it out by hand instead, but what's the point?
  //it doesn't even do forwards/backwards compatability for you.
  template <typename Packer>
  void msgpack_pack(Packer& pk) const {
    pk.pack_array(4);
    pk.pack(id);
    pk.pack(count);
    pk.pack(prefix);
    pk.pack(length);
  }
  void msgpack_unpack(msgpack::object o) {
    if(o.type != msgpack::type::ARRAY) { throw msgpack::type_error(); }
    const size_t size = o.via.array.size;
    if(size <= 0) return;
    id = o.via.array.ptr[0].via.u64;
    count = o.via.array.ptr[1].via.i64;
    prefix = o.via.array.ptr[2].via.i64;
    length = o.via.array.ptr[3].via.i64;
  }
};

struct Bar {
  Foo parent;
  int time;
  float ratio;
  unsigned short size;
  MSGPACK_DEFINE(parent, time, ratio, size);
};

struct FooBar {
  Bar sibling;
  string name;
  double rating;
  unsigned char postfix;
  MSGPACK_DEFINE(sibling, name, rating, postfix);
};

struct FooBarContainer {
  vector<FooBar> list;
  bool initialized;
  int fruit;
  string location;
  MSGPACK_DEFINE(list, initialized, fruit, location);
};

struct MPBench : Bench {
  void Encode(void *buf, size_t &len) {
    const int veclen = 3;
    msgpack::sbuffer buffer;
    FooBarContainer foobarcontainer;
    for (int i = 0; i < veclen; i++) {
      // We add + i to not make these identical copies for a more realistic
      // compression test.
      foobarcontainer.list.push_back(FooBar());
      auto &foobar = foobarcontainer.list.back();
      auto &bar = foobar.sibling;
      auto &foo = bar.parent;
      foo.id = 0xABADCAFEABADCAFE + i;
      foo.count = 10000 + i;
      foo.prefix = '@' + i;
      foo.length = 1000000 + i;
      bar.time = 123456 + i;
      bar.ratio = 3.14159f + i;
      bar.size = 10000 + i;
      foobar.name = "Hello, World!";
      foobar.rating = 3.1415432432445543543 + i;
      foobar.postfix = '!' + i;
    }
    foobarcontainer.location = "http://google.com/flatbuffers/";
    foobarcontainer.initialized = true;
    foobarcontainer.fruit = Bananas;
    msgpack::pack(buffer, foobarcontainer);
    assert(len >= buffer.size());
    len = buffer.size();
    memcpy(buf, buffer.data(), len);
  }

  void *Decode(void *buf, size_t len)
  {
    auto up = new msgpack::unpacked();
    msgpack::unpack(up, (char *)buf, len);
    return buf;
  }

  int64_t Use(void *decoded) {
    auto up = (msgpack::unpacked *)decoded;
    msgpack::object obj = up->get();
    FooBarContainer foobarcontainer;
    obj.convert(&foobarcontainer);

    sum = 0;
    Add(foobarcontainer.initialized);
    Add(foobarcontainer.location.length());
    Add(foobarcontainer.fruit);
    for (unsigned int i = 0; i < foobarcontainer.list.size(); i++) {
      auto foobar = foobarcontainer.list[i];
      Add(foobar.name.length());
      Add(foobar.postfix);
      Add(static_cast<int64_t>(foobar.rating));
      auto bar = foobar.sibling;
      Add(static_cast<int64_t>(bar.ratio));
      Add(bar.size);
      Add(bar.time);
      auto foo = bar.parent;
      Add(foo.count);
      Add(foo.id);
      Add(foo.length);
      Add(foo.prefix);
    }
    return sum;
  }

  void Dealloc(void *decoded) { delete (msgpack::unpacked *)decoded; }
};

Bench *NewMPBench() { return new MPBench(); }

*/
