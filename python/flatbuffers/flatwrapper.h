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

#ifndef FLATWRAPPER_H_
#define FLATWRAPPER_H_

#include <cstddef>
#include <memory>
#include <Python.h>

#include "flatbuffers/flatbuffers.h"


class PythonAllocator : public flatbuffers::simple_allocator {
 public:
  PythonAllocator() {}
  virtual ~PythonAllocator() {}

  virtual uint8_t *allocate(size_t size) const {
    uint8_t *p = static_cast<uint8_t *>(PyMem_Malloc(size));
    if (!p) {
      throw std::bad_alloc();
    }
    return p;
  }

  virtual void deallocate(uint8_t *p) const {
    PyMem_Free((void*)p);
  }
};


inline flatbuffers::uoffset_t PushUOffsetElement(
    flatbuffers::FlatBufferBuilder *builder,
    flatbuffers::Offset<void> off) {
  return builder->PushElement<void>(off);
}


inline const flatbuffers::Table *GetTable(const void *buf) {
  return flatbuffers::GetRoot<flatbuffers::Table>(buf);
}


#endif  // FLATWRAPPER_H_
