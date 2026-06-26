"use strict";
/**
 * Self-describing envelope for FlatBuffer payloads.
 *
 * Envelope format (42-byte header followed by the payload):
 * ```
 * [4 bytes]  magic: "LN2E" (0x4C 0x4E 0x32 0x45)
 * [1 byte]   version: 0x01
 * [1 byte]   hash algorithm: 0x01 = SHA-256
 * [32 bytes] SHA-256 hash of the .bfbs binary schema
 * [4 bytes]  payload length, little-endian uint32
 * [N bytes]  FlatBuffer payload
 * ```
 *
 * Total header size: 42 bytes.
 *
 * Usage:
 * ```typescript
 * const hash = await computeSchemaHash(bfbsBytes);
 * const envelope = envelopeWrap(hash, flatbufferPayload);
 * const payload = envelopeUnwrap(envelope, hash); // null if hash mismatch
 * ```
 */
Object.defineProperty(exports, "__esModule", { value: true });
exports.computeSchemaHash = computeSchemaHash;
exports.envelopeWrap = envelopeWrap;
exports.envelopeUnwrap = envelopeUnwrap;
exports.envelopePayload = envelopePayload;
exports.envelopeSchemaHash = envelopeSchemaHash;
exports.isEnvelope = isEnvelope;
const ENVELOPE_MAGIC = new Uint8Array([0x4c, 0x4e, 0x32, 0x45]); // "LN2E"
const ENVELOPE_VERSION = 0x01;
const ENVELOPE_HASH_ALGO_SHA256 = 0x01;
/** Total size of the fixed envelope header in bytes. */
const ENVELOPE_HEADER_SIZE = 42;
/**
 * Computes the SHA-256 hash of binary schema data (contents of a .bfbs file).
 *
 * Uses the Web Crypto API (`crypto.subtle`), available in browsers and Node.js 15+.
 *
 * @param bfbsData - The raw bytes of the compiled FlatBuffers binary schema.
 * @returns A 32-byte Uint8Array containing the SHA-256 digest.
 */
async function computeSchemaHash(bfbsData) {
    const hash = await crypto.subtle.digest("SHA-256", bfbsData);
    return new Uint8Array(hash);
}
/**
 * Wraps a FlatBuffer payload in a self-describing envelope embedding the
 * SHA-256 hash of its associated .bfbs binary schema.
 *
 * The resulting bytes can be unwrapped with {@link envelopeUnwrap} after
 * verifying the schema identity.
 *
 * @param schemaHash - 32-byte SHA-256 hash of the .bfbs schema (from {@link computeSchemaHash}).
 * @param payload    - The raw FlatBuffer bytes to wrap.
 * @returns A new Uint8Array containing the 42-byte header followed by the payload.
 * @throws {Error} If schemaHash is not exactly 32 bytes.
 */
function envelopeWrap(schemaHash, payload) {
    if (schemaHash.length !== 32) {
        throw new Error(`envelopeWrap: schemaHash must be 32 bytes, got ${schemaHash.length}`);
    }
    const out = new Uint8Array(ENVELOPE_HEADER_SIZE + payload.length);
    const view = new DataView(out.buffer);
    // magic
    out.set(ENVELOPE_MAGIC, 0);
    // version
    out[4] = ENVELOPE_VERSION;
    // hash algorithm
    out[5] = ENVELOPE_HASH_ALGO_SHA256;
    // schema hash
    out.set(schemaHash, 6);
    // payload length (little-endian uint32)
    view.setUint32(38, payload.length, /* littleEndian= */ true);
    // payload
    out.set(payload, ENVELOPE_HEADER_SIZE);
    return out;
}
/**
 * Validates an envelope and extracts the FlatBuffer payload, comparing the
 * embedded schema hash against `expectedHash`.
 *
 * @param data         - The full envelope bytes (header + payload).
 * @param expectedHash - 32-byte SHA-256 hash of the expected .bfbs schema.
 * @returns The payload bytes (a sub-array of `data`) on success, or `null` if
 *          the magic bytes, version, hash algorithm, or schema hash do not match,
 *          or if the data is malformed.
 */
function envelopeUnwrap(data, expectedHash) {
    const parsed = parseEnvelope(data);
    if (parsed === null) {
        return null;
    }
    const { payload, hash } = parsed;
    if (!bytesEqual(hash, expectedHash)) {
        return null;
    }
    return payload;
}
/**
 * Extracts the FlatBuffer payload from an envelope without validating the
 * schema hash. Useful for inspection or routing before the expected schema
 * is known.
 *
 * @param data - The full envelope bytes (header + payload).
 * @returns The payload bytes on success, or `null` if the header is malformed.
 */
function envelopePayload(data) {
    const parsed = parseEnvelope(data);
    return parsed !== null ? parsed.payload : null;
}
/**
 * Extracts the 32-byte schema hash from an envelope header without validating
 * the payload or comparing against an expected hash. Useful for routing messages
 * to the correct handler by schema identity.
 *
 * @param data - The full envelope bytes.
 * @returns A 32-byte Uint8Array containing the schema hash, or `null` if the
 *          header is malformed.
 */
function envelopeSchemaHash(data) {
    const parsed = parseEnvelope(data);
    return parsed !== null ? parsed.hash : null;
}
/**
 * Returns `true` if `data` begins with the LN2E magic bytes and is at least
 * long enough to contain a complete envelope header.
 *
 * @param data - Bytes to test.
 */
function isEnvelope(data) {
    if (data.length < ENVELOPE_HEADER_SIZE) {
        return false;
    }
    return (data[0] === ENVELOPE_MAGIC[0] &&
        data[1] === ENVELOPE_MAGIC[1] &&
        data[2] === ENVELOPE_MAGIC[2] &&
        data[3] === ENVELOPE_MAGIC[3]);
}
/** Parses and validates envelope header fields. Returns null on any error. */
function parseEnvelope(data) {
    if (data.length < ENVELOPE_HEADER_SIZE) {
        return null;
    }
    // magic check
    if (data[0] !== ENVELOPE_MAGIC[0] ||
        data[1] !== ENVELOPE_MAGIC[1] ||
        data[2] !== ENVELOPE_MAGIC[2] ||
        data[3] !== ENVELOPE_MAGIC[3]) {
        return null;
    }
    // version check
    if (data[4] !== ENVELOPE_VERSION) {
        return null;
    }
    // hash algorithm check
    if (data[5] !== ENVELOPE_HASH_ALGO_SHA256) {
        return null;
    }
    const hash = data.subarray(6, 38);
    const view = new DataView(data.buffer, data.byteOffset, data.byteLength);
    const payloadLen = view.getUint32(38, /* littleEndian= */ true);
    const end = ENVELOPE_HEADER_SIZE + payloadLen;
    if (end > data.length) {
        return null;
    }
    return { payload: data.subarray(ENVELOPE_HEADER_SIZE, end), hash };
}
/** Constant-time-ish byte equality for two same-length arrays. */
function bytesEqual(a, b) {
    if (a.length !== b.length) {
        return false;
    }
    let diff = 0;
    for (let i = 0; i < a.length; i++) {
        diff |= a[i] ^ b[i];
    }
    return diff === 0;
}
