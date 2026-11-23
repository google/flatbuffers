#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <new>
#include <type_traits>

#include "64bit/test_64bit_bfbs_generated.h"
#include "64bit/test_64bit_generated.h"
#include "flatbuffers/base.h"
#include "flatbuffers/flatbuffer_builder.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/reflection.h"
#include "flatbuffers/verifier.h"
#include "test_assert.h"
#include "test_init.h"

OneTimeTestInit OneTimeTestInit::one_time_init_;

static RootTableBinarySchema schema;

static constexpr uint8_t flags_sized_prefixed = 0b00000001;

static const uint64_t kFnvPrime = 0x00000100000001b3ULL;
static const uint64_t kOffsetBasis = 0xcbf29ce484222645ULL;

namespace flatbuffers {

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
uint64_t Hash(T value, uint64_t hash) {
  return (hash * kFnvPrime) ^ value;
}

uint64_t Hash(double value, uint64_t hash) {
  static_assert(sizeof(double) == sizeof(uint64_t));
  // Converting a double to uint64_t is undefined when the value is NaN, is
  // infinite, or is simply outside the range of the target type, and fuzzed
  // buffers reach all three. Hash the bit pattern instead, which is what the
  // static_assert above was reaching for anyway.
  uint64_t bits;
  std::memcpy(&bits, &value, sizeof(bits));
  return (hash * kFnvPrime) ^ bits;
}

uint64_t Hash(const flatbuffers::String* value, uint64_t hash) {
  if (value == nullptr) {
    return hash * kFnvPrime;
  }
  for (auto& c : value->str()) {
    hash = Hash(static_cast<uint8_t>(c), hash);
  }
  return hash;
}

uint64_t Hash(const LeafStruct* value, uint64_t hash) {
  if (value == nullptr) {
    return hash * kFnvPrime;
  }
  hash = Hash(value->a(), hash);
  hash = Hash(value->b(), hash);
  return hash;
}

template <typename T>
uint64_t Hash(const Vector<T>* value, uint64_t hash) {
  if (value == nullptr) {
    return hash * kFnvPrime;
  }
  for (const T c : *value) {
    hash = Hash(c, hash);
  }
  return hash;
}

template <typename T>
uint64_t Hash(const Vector64<T>* value, uint64_t hash) {
  if (value == nullptr) {
    return hash * kFnvPrime;
  }
  for (const T c : *value) {
    hash = Hash(c, hash);
  }
  return hash;
}

uint64_t Hash(const RootTable* value, uint64_t hash) {
  if (value == nullptr) {
    return hash * kFnvPrime;
  }
  // Hash all the fields so we can exercise all parts of the code.
  hash = Hash(value->far_vector(), hash);
  hash = Hash(value->a(), hash);
  hash = Hash(value->far_string(), hash);
  hash = Hash(value->big_vector(), hash);
  hash = Hash(value->near_string(), hash);
  hash = Hash(value->nested_root(), hash);
  hash = Hash(value->far_struct_vector(), hash);
  hash = Hash(value->big_struct_vector(), hash);
  return hash;
}

// libFuzzer makes no promise about the alignment of the data it hands out, and
// this target additionally consumes a leading flag byte, which leaves the
// remaining pointer at an odd address. Every flatbuffers accessor assumes the
// buffer starts at an aligned address -- GetRoot() reads a uoffset_t straight
// off buf_ -- so reads through such a pointer are misaligned no matter what the
// library does, and UBSan reports on the very first input that verifies. Copy
// the payload into an over-aligned allocation first.
//
// The allocation is exactly `size` bytes so that ASan still catches reads past
// the end of the buffer; an oversized static scratch buffer would hide them.
namespace {
constexpr std::size_t kBufferAlignment = 32;

struct AlignedDelete {
  void operator()(uint8_t* p) const noexcept {
    ::operator delete(p, std::align_val_t(kBufferAlignment));
  }
};

using AlignedBuffer = std::unique_ptr<uint8_t[], AlignedDelete>;

AlignedBuffer AlignedCopy(const uint8_t* data, std::size_t size) {
  AlignedBuffer buffer(static_cast<uint8_t*>(
      ::operator new(size, std::align_val_t(kBufferAlignment))));
  std::memcpy(buffer.get(), data, size);
  return buffer;
}
}  // namespace

static int AccessBuffer(const uint8_t* data, size_t size,
                        bool is_size_prefixed) {
  const RootTable* root_table =
      is_size_prefixed ? GetSizePrefixedRootTable(data) : GetRootTable(data);
  TEST_NOTNULL(root_table);

  uint64_t hash = kOffsetBasis;
  hash = Hash(root_table, hash);
  hash = Hash(root_table->nested_root_nested_root(), hash);

  return 0;
}

extern "C" int LLVMFuzzerInitialize(int*, char*** argv) {
  Verifier verifier(schema.begin(), schema.size());
  TEST_EQ(true, reflection::VerifySchemaBuffer(verifier));

  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < FLATBUFFERS_MIN_BUFFER_SIZE) {
    return 0;
  }

  // Take the first bit of data as a flag to control things.
  const uint8_t flags = data[0];
  data++;
  size--;

  const AlignedBuffer aligned = AlignedCopy(data, size);
  data = aligned.get();

  Verifier::Options options;
  options.assert = true;
  options.check_alignment = true;
  options.check_nested_flatbuffers = true;

  Verifier verifier(data, size, options);

  const bool is_size_prefixed = flags & flags_sized_prefixed;

  // Filter out data that isn't valid.
  if ((is_size_prefixed && !VerifySizePrefixedRootTableBuffer(verifier)) ||
      !VerifyRootTableBuffer(verifier)) {
    return 0;
  }

  return AccessBuffer(data, size, is_size_prefixed);
}

}  // namespace flatbuffers