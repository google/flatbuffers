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
/**
 * Computes the SHA-256 hash of binary schema data (contents of a .bfbs file).
 *
 * Uses the Web Crypto API (`crypto.subtle`), available in browsers and Node.js 15+.
 *
 * @param bfbsData - The raw bytes of the compiled FlatBuffers binary schema.
 * @returns A 32-byte Uint8Array containing the SHA-256 digest.
 */
export declare function computeSchemaHash(bfbsData: Uint8Array): Promise<Uint8Array>;
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
export declare function envelopeWrap(schemaHash: Uint8Array, payload: Uint8Array): Uint8Array;
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
export declare function envelopeUnwrap(data: Uint8Array, expectedHash: Uint8Array): Uint8Array | null;
/**
 * Extracts the FlatBuffer payload from an envelope without validating the
 * schema hash. Useful for inspection or routing before the expected schema
 * is known.
 *
 * @param data - The full envelope bytes (header + payload).
 * @returns The payload bytes on success, or `null` if the header is malformed.
 */
export declare function envelopePayload(data: Uint8Array): Uint8Array | null;
/**
 * Extracts the 32-byte schema hash from an envelope header without validating
 * the payload or comparing against an expected hash. Useful for routing messages
 * to the correct handler by schema identity.
 *
 * @param data - The full envelope bytes.
 * @returns A 32-byte Uint8Array containing the schema hash, or `null` if the
 *          header is malformed.
 */
export declare function envelopeSchemaHash(data: Uint8Array): Uint8Array | null;
/**
 * Returns `true` if `data` begins with the LN2E magic bytes and is at least
 * long enough to contain a complete envelope header.
 *
 * @param data - Bytes to test.
 */
export declare function isEnvelope(data: Uint8Array): boolean;
