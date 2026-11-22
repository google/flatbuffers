/*
 * Fuzzes the C++ reflection API with an untrusted schema and an untrusted
 * data buffer, mirroring the documented runtime-schema use case:
 *   1. reflection::VerifySchemaBuffer() on the schema,
 *   2. flatbuffers::Verify(schema, root_object, ...) on the data,
 *   3. flatbuffers::CopyTable() round-trip of the verified root table.
 *
 * Input format (all little-endian):
 *   u32       schema_length
 *   u8[schema_length]  binary schema (.bfbs)
 *   u8[...]            flatbuffer data verified against the schema
 *
 * Splitting the input this way lets the fuzzer mutate both the schema and
 * the data, which is the attack surface of every schema-driven consumer.
 */
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/reflection.h"
#include "flatbuffers/util.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size < sizeof(flatbuffers::uoffset_t) * 2) return 0;

  const size_t schema_length =
      flatbuffers::ReadScalar<flatbuffers::uoffset_t>(data);
  if (schema_length > size - sizeof(flatbuffers::uoffset_t)) return 0;

  const uint8_t *schema_buf = data + sizeof(flatbuffers::uoffset_t);
  const size_t remaining =
      size - sizeof(flatbuffers::uoffset_t) - schema_length;
  if (remaining < sizeof(flatbuffers::uoffset_t)) return 0;
  const uint8_t *data_buf = schema_buf + schema_length;

  // 1. The schema itself must verify before it is used.
  flatbuffers::Verifier schema_verifier(schema_buf, schema_length);
  if (!reflection::VerifySchemaBuffer(schema_verifier)) return 0;

  const auto *schema = reflection::GetSchema(schema_buf);
  const auto *root_object = schema->root_table();
  if (root_object == nullptr) return 0;

  // 2. The data buffer must verify against the schema.
  if (!flatbuffers::Verify(*schema, *root_object, data_buf, remaining))
    return 0;

  // 3. Consume the verified pair the way a schema-driven tool would.
  const auto *root = flatbuffers::GetAnyRoot(data_buf);
  flatbuffers::FlatBufferBuilder builder;
  flatbuffers::CopyTable(builder, *schema, *root_object, *root);
  return 0;
}
