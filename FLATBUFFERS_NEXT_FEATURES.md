# FlatBuffers Next Features — LN2 Implementation Plan

Council-debated priority ranking for 7 ecosystem improvements.
flatc v25.12.21 on branch `rust-object-api-improvements`.

## Decision: BFBS in Production Binaries (Always Embedded)

The council initially considered restricting .bfbs to dev/tooling builds. This was **overridden** — bfbs belongs in production binaries because LN2 needs runtime reflection for:

1. **CDO schema version detection** — VTS loads CDO files from disk at runtime and needs to determine which schema generation produced them
2. **Generic reflection Pack/Unpack** — replaces LiftCloud's 200-line CDO packer switch/case in production
3. **Dynamic field access for structured logging** — VTS uses SafeBuffer (reflection-based) for production telemetry
4. **Legacy format detection** — three-tier verifier checks field layout to determine buffer generation at runtime
5. **Wire protocol negotiation** — VTS/LiftCloud interrogate schemas to determine field availability across versions

**Schema leakage mitigation**: field names and types are already exposed via Go struct tags, TS property names, Rust debug impls, and error messages. Access control on inspection APIs (don't expose "dump schema" endpoints publicly) is the correct mitigation, not stripping schemas.

**Embedding approach (no feature flags, always present):**
- **Rust VTS:** `include_bytes!("../bfbs/ln2_message.bfbs")` — compiled in unconditionally
- **Go LiftCloud:** `//go:embed bfbs/*.bfbs` in a schemas package — available at runtime
- **TS frontend:** Bundled as static assets via `--bfbs-gen-embed`, loaded at startup

The `--bfbs-gen-embed` flag (already implemented in flatc v25.12.20) generates embedded schema constants for all three languages. Use it for every schema compilation.

## Phase 1 — Codegen Wins + Envelope (Ship First)

### 1.1 Union Ergonomics: Go Match Method
**Effort:** S | **Risk:** Low | **Wire change:** None

Generate a typed `Match` method on Go union types with exhaustive callbacks:
```go
// Generated for union MessageContent:
func (u *MessageContentT) Match(
    onStatus func(*StatusMessageT),
    onTraffic func(*TrafficMessageT),
    onFault func(*FaultMessageT),
    // ... one per variant
) {
    switch u.Type {
    case MessageContentStatusMessage: onStatus(u.Value.(*StatusMessageT))
    // ...
    }
}
```

**Files:** `src/idl_gen_go.cpp` — add `GenUnionMatch` method
**Consumers:** LiftCloud WebSocket handler, all Go message dispatch paths

### 1.2 Optional Scalars: Go `*T` Codegen
**Effort:** S | **Risk:** Medium (breaking accessor return types) | **Wire change:** None

For fields marked `optional` in the schema, generate pointer-typed accessors:
```go
// Before: func (rcv *Sensor) Temperature() int16 { ... }
// After:  func (rcv *Sensor) Temperature() *int16 { ... }
```

**Files:** `src/idl_gen_go.cpp` — modify scalar accessor generation for optional fields
**Consumers:** LiftCloud sensor handling, any field where 0 is a valid value

### 1.3 Self-Describing Envelope (Schema Fingerprint)
**Effort:** M | **Risk:** Medium (transport integration) | **Wire change:** None (envelope OUTSIDE buffer)

Add a binary envelope wrapper that carries a schema content hash alongside the FlatBuffer payload:

```
[4 bytes: envelope magic "LN2E"]
[1 byte:  envelope version]
[1 byte:  hash algorithm (0=SHA256)]
[32 bytes: SHA-256 of the .bfbs binary schema]
[4 bytes: payload length (little-endian)]
[N bytes: FlatBuffer payload (unchanged)]
```

**Implementation:**
- `flatc --gen-envelope` flag generates envelope pack/unpack helpers per language
- Rust: `EnvelopedBuffer::wrap(bfbs_hash, flatbuf_bytes) -> Vec<u8>`
- Go: `EnvelopeWrap(bfbsHash, flatbufBytes) []byte`
- TS: `envelopeWrap(bfbsHash: Uint8Array, payload: Uint8Array): Uint8Array`
- Schema hash computed from .bfbs content (deterministic across platforms)
- Transport layer (WebSocket, MQTT) validates hash on receipt, rejects mismatches

**Security conditions (from council):**
- SHA-256 minimum (no CRC32)
- Hash computed over canonical .bfbs, not .fbs source
- Envelope validation mandatory at transport boundary
- No bypass path — unknown hashes must be rejected

**Files:** New runtime files + `src/flatc.cpp` flag + codegen in all 3 backends
**Consumers:** All LN2 services at transport boundaries

## Phase 2 — Runtime Features

### 2.1 Selective Object API Unpack
**Effort:** L | **Risk:** Low | **Wire change:** None

Instead of materializing the entire `*T` tree, allow field-targeted unpacking:

**Rust:** `table.unpack_field::<StatusT>("status") -> Option<StatusT>`
**Go:** `msg.UnpackField("status") -> (interface{}, error)` or generated per-field unpackers
**TS:** Document and promote accessor-first pattern over eager `unpack()`

**Key insight from council:** FlatBuffers zero-copy accessors ARE lazy deserialization. The bottleneck is `unpack()` materializing the full tree. The fix is making `unpack()` field-selective, not building a new deserialization layer.

### 2.2 Reflection-Based Pack/Unpack from JSON
**Effort:** L | **Risk:** Low | **Wire change:** None

Add to Go and TS reflection runtimes:
```go
// Collapses the 200-line CDO packer switch/case
schema := flatbuffers.LoadReflectionSchema(bfbsData)
buf, err := flatbuffers.ReflectPack(schema, "Mapping", jsonData)
obj, err := flatbuffers.ReflectUnpack(schema, "Mapping", flatbufBytes)
```

**Architecture:**
- Recursive schema walk: enumerate fields, dispatch by BaseType
- Union handling: JSON must include discriminant field (e.g. `"content_type": "StatusMessage"`)
- Tree-shakeable: lives in `flatbuffers/reflection` import path (TS), separate from core
- NOT for hot paths — use generated Pack/Unpack for performance-critical code

**Files:** `go/reflect.go` (extend), `ts/reflection.ts` (extend)
**Consumers:** LiftCloud CDO packer, liftNet_frontend config UI

### 2.3 Buffer Diffing
**Effort:** M | **Risk:** Low | **Wire change:** None

Add `DiffBuffers(schema, bufA, bufB) -> []FieldDelta` to reflection runtimes:
```go
deltas, err := flatbuffers.DiffBuffers(schema, oldBuf, newBuf)
for _, d := range deltas {
    fmt.Printf("%s: %v -> %v\n", d.Path, d.OldValue, d.NewValue)
}
```

**Output:** `[]FieldDelta{Path string, OldValue any, NewValue any, Kind DeltaKind}`
**Implementation:** Recursive field-by-field comparison using reflection accessors
**Files:** `go/reflect_diff.go` (new), `ts/reflection-diff.ts` (new)
**Consumers:** cdo-inspect, LiftCloud CDO audit logging

## Phase 3 — Performance (Gated on Profiling)

### 3.1 Chunked Builder Allocator (Rust)
**Effort:** XL | **Risk:** High | **Wire change:** None

Replace FlatBufferBuilder's single contiguous allocation with a slab/arena allocator for embedded ARM targets. Configurable segment size (e.g., 32KB chunks).

**Gate:** Must demonstrate >30% memory reduction on BatchedFrontEndMessage with ARM profiling data before implementation.

**Council note:** Streaming/incremental buffer emission is fundamentally incompatible with FlatBuffers' back-to-front layout. This is an allocator optimization, not a protocol change.

## DO NOT DO

1. **Do NOT embed schema fingerprint inside the FlatBuffer wire format** — breaks every consumer simultaneously, fleet-wide outage risk
2. **Do NOT attempt forward-streaming FlatBuffer emission** — wire format requires complete vtable/offset tree before finishing
3. **Do NOT bundle reflection runtime into core TS module** — tree-shake boundary is non-negotiable for frontend bundle size
4. **Do NOT use CRC32 for schema fingerprint** — collision risk unacceptable for safety-critical elevator control
