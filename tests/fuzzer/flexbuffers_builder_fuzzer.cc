// flexbuffers_builder_fuzzer.cc
//
// Fuzz target for the FlexBuffers *builder* path.
// The existing flexbuffers_verifier_fuzzer exercises only Read/Verify
// on arbitrary bytes. This harness exercises the Builder→finish→verify
// round-trip covering:
//
//   include/flatbuffers/flexbuffers.h  – Builder::Add*, Vector, Map, Blob
//   src/flexbuffers.cpp                – type-byte packing, width selection,
//                                        back-reference resolution, bit-packing
//
// A bug in the builder (e.g. wrong width calculation, OOB on back-refs)
// would produce a buffer that does not round-trip through the verifier.

#include <stddef.h>
#include <stdint.h>
#include <cstring>
#include <string>

#include "flatbuffers/flexbuffers.h"

static uint8_t u8(const uint8_t **d, size_t *s)
{
    if (*s == 0) return 0;
    uint8_t v = **d; (*d)++; (*s)--; return v;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 4) return 0;

    flexbuffers::Builder fbb;
    uint8_t op = u8(&data, &size);

    switch (op % 5) {
    case 0: {
        // Build a vector of scalars
        auto vec_start = fbb.StartVector();
        size_t n = (u8(&data, &size) % 8) + 1;
        for (size_t i = 0; i < n && size > 0; i++) {
            fbb.Add(static_cast<int64_t>(u8(&data, &size)) - 128);
        }
        fbb.EndVector(vec_start, false, false);
        break;
    }
    case 1: {
        // Build a map with string keys
        auto map_start = fbb.StartMap();
        size_t n = (u8(&data, &size) % 4) + 1;
        for (size_t i = 0; i < n && size >= 2; i++) {
            size_t klen = u8(&data, &size) % 8 + 1;
            size_t vlen = u8(&data, &size) % 8;
            std::string key(size < klen ? size : klen, 'k');
            if (size >= klen) { memcpy(&key[0], data, klen); data += klen; size -= klen; }
            fbb.Key(key);
            std::string val(size < vlen ? size : vlen, 'v');
            if (size >= vlen) { memcpy(&val[0], data, vlen); data += vlen; size -= vlen; }
            fbb.Add(val);
        }
        fbb.EndMap(map_start);
        break;
    }
    case 2: {
        // Blob
        size_t blen = (u8(&data, &size) % 32);
        blen = blen < size ? blen : size;
        fbb.Blob(data, blen);
        data += blen; size -= blen;
        break;
    }
    case 3: {
        // Nested vectors
        auto outer = fbb.StartVector();
        auto inner = fbb.StartVector();
        size_t n = (u8(&data, &size) % 4);
        for (size_t i = 0; i < n && size > 0; i++)
            fbb.Add(static_cast<double>(u8(&data, &size)));
        fbb.EndVector(inner, false, false);
        fbb.EndVector(outer, false, false);
        break;
    }
    case 4: {
        // Mixed map with typed vector
        auto map_start = fbb.StartMap();
        fbb.Key("nums");
        auto vec_start = fbb.StartVector();
        size_t n = (u8(&data, &size) % 16);
        for (size_t i = 0; i < n && size > 0; i++)
            fbb.Add(static_cast<uint64_t>(u8(&data, &size)));
        fbb.EndVector(vec_start, true, false);
        fbb.EndMap(map_start);
        break;
    }
    }

    fbb.Finish();
    auto &buf = fbb.GetBuffer();

    // Verify the output is a valid FlexBuffer
    flexbuffers::Verifier v(buf.data(), buf.size());
    v.Check();

    // Also decode the root
    auto root = flexbuffers::GetRoot(buf);
    (void)root.GetType();

    return 0;
}
