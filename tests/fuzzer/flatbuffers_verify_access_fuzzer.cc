// Fuzz target for the property the verifier exists to provide:
//
//   VerifyMonsterBuffer(v) == true  =>  reading every field is memory-safe.
//
// The existing flatbuffers_verifier_fuzzer.cc calls VerifyMonsterBuffer() and
// discards the result, so it only proves the verifier itself does not crash.
// Nothing in tree exercises the verify-then-access invariant on arbitrary
// input, which is the guarantee every consumer of FlatBuffers relies on when
// parsing untrusted data.
//
// Any out-of-bounds access reported by ASan below is a verifier bypass.

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cpp17/generated_cpp17/monster_test_generated.h"

namespace {

volatile uint64_t g_sink = 0;

// Read every byte so ASan sees the access, and keep the result observable so
// the compiler cannot elide the loads.
void Touch(const void *p, size_t n) {
  if (p == nullptr) return;
  const uint8_t *b = static_cast<const uint8_t *>(p);
  uint64_t s = 0;
  for (size_t i = 0; i < n; i++) s += b[i];
  g_sink += s;
}

void TouchString(const flatbuffers::String *s) {
  if (s == nullptr) return;
  Touch(s->Data(), s->size());
}

template <typename T> void TouchScalarVector(const flatbuffers::Vector<T> *v) {
  if (v == nullptr) return;
  for (flatbuffers::uoffset_t i = 0; i < v->size(); i++) {
    T x = v->Get(i);
    Touch(&x, sizeof(x));   // no lossy scalar cast; just observe the bytes
  }
}

void TouchStringVector(
    const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *v) {
  if (v == nullptr) return;
  for (flatbuffers::uoffset_t i = 0; i < v->size(); i++) TouchString(v->Get(i));
}

constexpr int kMaxDepth = 6;

void WalkMonster(const MyGame::Example::Monster *m, int depth);

void WalkStat(const MyGame::Example::Stat *s) {
  if (s == nullptr) return;
  TouchString(s->id());
  g_sink += (uint64_t)s->val() + s->count();
}

void WalkReferrable(const MyGame::Example::Referrable *r) {
  if (r == nullptr) return;
  g_sink += r->id();
}

void WalkMonster(const MyGame::Example::Monster *m, int depth) {
  if (m == nullptr || depth > kMaxDepth) return;

  // Inline scalars and structs.
  g_sink += (uint64_t)m->mana() + m->hp() + (int)m->color() + m->testbool();
  if (m->pos() != nullptr) Touch(m->pos(), sizeof(MyGame::Example::Vec3));
  if (m->native_inline() != nullptr)
    Touch(m->native_inline(), sizeof(MyGame::Example::Test));

  // Indirected fields: this is where a verifier bypass would show up.
  TouchString(m->name());
  TouchScalarVector(m->inventory());
  TouchScalarVector(m->testarrayofbools());
  TouchScalarVector(m->vector_of_longs());
  TouchScalarVector(m->vector_of_doubles());
  TouchScalarVector(m->vector_of_weak_references());
  TouchScalarVector(m->vector_of_co_owning_references());
  TouchScalarVector(m->vector_of_non_owning_references());
  TouchScalarVector(m->vector_of_enums());
  TouchScalarVector(m->flex());
  TouchScalarVector(m->testnestedflatbuffer());
  TouchScalarVector(m->testrequirednestedflatbuffer());

  TouchStringVector(m->testarrayofstring());
  TouchStringVector(m->testarrayofstring2());

  if (m->test4() != nullptr) {
    for (flatbuffers::uoffset_t i = 0; i < m->test4()->size(); i++)
      Touch(m->test4()->Get(i), sizeof(MyGame::Example::Test));
  }
  if (m->test5() != nullptr) {
    for (flatbuffers::uoffset_t i = 0; i < m->test5()->size(); i++)
      Touch(m->test5()->Get(i), sizeof(MyGame::Example::Test));
  }
  if (m->testarrayofsortedstruct() != nullptr) {
    for (flatbuffers::uoffset_t i = 0; i < m->testarrayofsortedstruct()->size(); i++)
      Touch(m->testarrayofsortedstruct()->Get(i), sizeof(MyGame::Example::Ability));
  }

  WalkStat(m->testempty());
  if (m->scalar_key_sorted_tables() != nullptr) {
    for (flatbuffers::uoffset_t i = 0; i < m->scalar_key_sorted_tables()->size(); i++)
      WalkStat(m->scalar_key_sorted_tables()->Get(i));
  }
  if (m->vector_of_referrables() != nullptr) {
    for (flatbuffers::uoffset_t i = 0; i < m->vector_of_referrables()->size(); i++)
      WalkReferrable(m->vector_of_referrables()->Get(i));
  }
  if (m->vector_of_strong_referrables() != nullptr) {
    for (flatbuffers::uoffset_t i = 0; i < m->vector_of_strong_referrables()->size(); i++)
      WalkReferrable(m->vector_of_strong_referrables()->Get(i));
  }

  // Unions: resolve through the discriminator, as a consumer would.
  if (m->test_type() == MyGame::Example::Any::Monster)
    WalkMonster(m->test_as_Monster(), depth + 1);
  else if (m->test_type() == MyGame::Example::Any::TestSimpleTableWithEnum)
    g_sink += (uint64_t)(m->test_as_TestSimpleTableWithEnum() != nullptr);
  else if (m->test_type() == MyGame::Example::Any::MyGame_Example2_Monster)
    g_sink += (uint64_t)(m->test_as_MyGame_Example2_Monster() != nullptr);

  // Nested tables.
  WalkMonster(m->enemy(), depth + 1);
  WalkMonster(m->testnestedflatbuffer_nested_root(), depth + 1);
  if (m->testarrayoftables() != nullptr) {
    for (flatbuffers::uoffset_t i = 0; i < m->testarrayoftables()->size(); i++)
      WalkMonster(m->testarrayoftables()->Get(i), depth + 1);
  }
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // libFuzzer gives no alignment guarantee for `data`, and FlatBuffers requires
  // the buffer start to be suitably aligned. Copy into an over-aligned block so
  // that any misalignment reported below is internal to the buffer layout and
  // therefore attributable to the verifier, not to the harness.
  alignas(16) static uint8_t aligned[64 * 1024];
  if (size > sizeof(aligned)) return 0;
  memcpy(aligned, data, size);

  flatbuffers::Verifier verifier(aligned, size);
  if (!MyGame::Example::VerifyMonsterBuffer(verifier)) return 0;

  // The verifier accepted this buffer, so every read below must be in bounds.
  WalkMonster(MyGame::Example::GetMonster(aligned), 0);
  return 0;
}
