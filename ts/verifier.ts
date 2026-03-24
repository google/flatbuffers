/**
 * FlatBuffers buffer verifier for structural integrity validation.
 *
 * This module provides a {@link Verifier} class that walks a raw FlatBuffer byte
 * buffer and confirms that every offset, length, alignment, vtable structure,
 * string encoding, and nesting depth is self-consistent before any field is
 * read by application code. Verifying the buffer up-front prevents denial-of-service
 * attacks and silent data corruption that would otherwise occur when a malformed or
 * truncated buffer is processed by generated accessor code.
 *
 * Generated `verifyRootAs*` functions produced by `flatc --ts` call the methods on
 * this class internally. Application code typically uses those generated helpers
 * rather than constructing a `Verifier` directly.
 *
 * @example
 * ```typescript
 * import { ByteBuffer } from 'flatbuffers';
 * import { MyMessage, verifyRootAsMyMessage } from './generated/my-message.js';
 * import { Verifier } from 'flatbuffers/verifier.js';
 *
 * // Assume `bytes` is a Uint8Array received from a WebSocket or network call.
 * const dv = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
 * const verifier = new Verifier(dv, { maxDepth: 32, maxTables: 10_000 });
 *
 * const result = verifyRootAsMyMessage(verifier, 0);
 * if (!result.ok) {
 *   console.error('Malformed buffer:', result.error);
 * } else {
 *   const msg = MyMessage.getRootAsMyMessage(new ByteBuffer(bytes));
 *   // Safe to read fields now.
 * }
 * ```
 *
 * @module
 */
import {SIZEOF_INT, SIZEOF_SHORT} from './constants.js';

/**
 * Categorises the structural error detected during buffer verification.
 *
 * Each variant corresponds to a distinct failure mode that the {@link Verifier}
 * can encounter while walking a FlatBuffer. Inspect the `kind` field of a
 * {@link VerificationError} to determine which check failed.
 */
export enum ErrorKind {
  /** A field marked `required` in the schema was absent from the vtable. */
  MissingRequiredField = 'MissingRequiredField',

  /**
   * An offset or length would place a read outside the bounds of the buffer,
   * or the running `apparentSize` accumulator exceeded `maxApparentSize`.
   */
  RangeOutOfBounds = 'RangeOutOfBounds',

  /**
   * The recursive nesting depth of tables/vectors exceeded the configured
   * `maxDepth` limit, preventing stack overflow on cyclic or deeply nested
   * malicious buffers.
   */
  DepthLimitReached = 'DepthLimitReached',

  /**
   * The total number of tables visited exceeded the configured `maxTables`
   * limit, preventing unbounded CPU cost on adversarial inputs.
   */
  TooManyTables = 'TooManyTables',

  /**
   * The sum of all byte ranges validated so far exceeded the configured
   * `maxApparentSize` limit. This catches buffers that reference overlapping
   * regions to inflate apparent work.
   */
  ApparentSizeTooLarge = 'ApparentSizeTooLarge',

  /** The string data is not valid UTF-8. */
  Utf8Error = 'Utf8Error',

  /**
   * The byte immediately after a string's character data was not `0x00`.
   * All FlatBuffer strings must be null-terminated. This check can be
   * suppressed via {@link VerifierOptions.ignoreNullTerminator}.
   */
  MissingNullTerminator = 'MissingNullTerminator',

  /**
   * A value's byte offset within the buffer is not a multiple of the
   * required alignment for its scalar type.
   */
  Unaligned = 'Unaligned',

  /**
   * A union value field is present without its corresponding type field,
   * or vice versa, violating the paired encoding contract for unions.
   */
  InconsistentUnion = 'InconsistentUnion',

  /**
   * The vtable header is malformed: its reported byte-length is less than the
   * two mandatory header shorts, is not a multiple of `sizeof(uint16)`, or the
   * vtable position derived from the signed offset falls outside the buffer.
   */
  InvalidVtable = 'InvalidVtable',

  /**
   * The element count stored at the head of a vector, when multiplied by the
   * element size, overflows a 32-bit integer, indicating a corrupted length.
   */
  InvalidVectorLength = 'InvalidVectorLength',

  /**
   * The signed soffset stored at a table's root position, when subtracted from
   * that position, yields a vtable address that lies outside the buffer.
   */
  SignedOffsetOutOfBounds = 'SignedOffsetOutOfBounds',
}

/**
 * Identifies the kind of location within a FlatBuffer structure that is being
 * traced when a {@link VerificationError} is built. Used in {@link TraceDetail}
 * to reconstruct a human-readable path to the failing field.
 */
export enum TraceKind {
  /** The trace entry refers to a field slot inside a table. */
  TableField = 'TableField',

  /** The trace entry refers to a positional element inside a vector. */
  VectorElement = 'VectorElement',

  /** The trace entry refers to a typed variant of a union field. */
  UnionVariant = 'UnionVariant',
}

/**
 * A single breadcrumb in the path from the root of the buffer to the field
 * that triggered a {@link VerificationError}.
 *
 * The trace array in a `VerificationError` is ordered from outermost to
 * innermost, so element `[0]` is closest to the root and the last element is
 * the immediate parent of the failed field.
 */
export interface TraceDetail {
  /** Whether this breadcrumb represents a table field, vector element, or union variant. */
  kind: TraceKind;

  /** Human-readable name of the field or structural location (e.g. `"positions"`). */
  name: string;

  /**
   * Zero-based numeric index of the field within its parent container.
   * For table fields this is the vtable slot index; for vector elements it is
   * the element index; for union variants it is the union type discriminant.
   */
  index: number;

  /** Byte offset within the buffer where this structural element begins. */
  position: number;
}

/**
 * Describes a structural error found during buffer verification.
 *
 * A `VerificationError` is thrown as a plain object (not an `Error` subclass)
 * from any `check*` method on {@link Verifier}. Catch it in a `try/catch` block
 * or inspect the `error` field of a failed {@link VerifyResult}.
 *
 * @example
 * ```typescript
 * try {
 *   verifier.checkTable(tablePos);
 * } catch (e) {
 *   const err = e as VerificationError;
 *   console.error(err.kind, 'at offset', err.offset);
 * }
 * ```
 */
export interface VerificationError {
  /** The category of structural failure that was detected. */
  kind: ErrorKind;

  /**
   * For {@link ErrorKind.MissingRequiredField}, the schema name of the absent
   * field. Undefined for all other error kinds.
   */
  field?: string;

  /** Byte offset within the buffer where the inconsistency was detected. */
  offset: number;

  /**
   * Ordered path of structural locations traversed before the error occurred,
   * from root to the immediate parent of the failing field.
   * May be empty when the error is detected outside a field-walking context.
   */
  trace: TraceDetail[];
}

/**
 * Tuning options for the {@link Verifier}.
 *
 * All fields are optional; omitting a field uses the built-in safe default.
 */
export interface VerifierOptions {
  /**
   * Maximum allowed nesting depth of tables and vectors before a
   * {@link ErrorKind.DepthLimitReached} error is raised.
   *
   * @default 64
   */
  maxDepth?: number;

  /**
   * Maximum total number of tables the verifier may visit across the entire
   * buffer traversal before raising {@link ErrorKind.TooManyTables}.
   *
   * @default 1_000_000
   */
  maxTables?: number;

  /**
   * Maximum sum of all byte ranges validated during a single traversal, in
   * bytes. Because range checks are cumulative, this limit catches buffers
   * that contain many overlapping regions. Raises
   * {@link ErrorKind.ApparentSizeTooLarge} when exceeded.
   *
   * @default 1_073_741_824 (1 GiB)
   */
  maxApparentSize?: number;

  /**
   * When `true`, skip the null-terminator check on FlatBuffer strings.
   * Useful when interoperating with non-conformant encoders.
   *
   * @default false
   */
  ignoreNullTerminator?: boolean;
}

/**
 * Discriminated union result type returned by generated `verifyRootAs*` helpers.
 *
 * On success, `ok` is `true` and `value` holds the verified root object.
 * On failure, `ok` is `false` and `error` holds the {@link VerificationError}
 * describing the first structural problem found. This pattern lets callers
 * handle errors without wrapping in a try/catch.
 *
 * @typeParam T - The root FlatBuffer table type being verified (e.g. `MyMessage`).
 *
 * @example
 * ```typescript
 * const result: VerifyResult<MyMessage> = verifyRootAsMyMessage(verifier, 0);
 * if (result.ok) {
 *   processMessage(result.value);
 * } else {
 *   reportError(result.error);
 * }
 * ```
 */
export type VerifyResult<T> =
  | {ok: true; value: T}
  | {ok: false; error: VerificationError};

const DEFAULT_MAX_DEPTH = 64;
const DEFAULT_MAX_TABLES = 1_000_000;
const DEFAULT_MAX_APPARENT_SIZE = 1_073_741_824;

/**
 * Stateful verifier that walks a FlatBuffer byte buffer and confirms structural
 * integrity before any application code reads field values.
 *
 * Instantiate once per buffer, call the appropriate `check*` methods (or let
 * generated `verifyRootAs*` functions do so), then discard the instance.
 * A `Verifier` is not reusable across multiple independent buffers.
 *
 * All `check*` methods throw a {@link VerificationError} plain object on failure.
 * They do **not** return error values; callers should wrap verification in a
 * `try/catch` or use the generated helpers that return a {@link VerifyResult}.
 */
export class Verifier {
  private buf: DataView;
  private opts: Required<VerifierOptions>;
  private depth: number;
  private numTables: number;
  private apparentSize: number;

  /**
   * Creates a new `Verifier` bound to the given buffer.
   *
   * @param buf - A `DataView` wrapping the raw FlatBuffer bytes to verify.
   *   The view's `byteOffset` and `byteLength` are respected so a sub-view of
   *   a larger `ArrayBuffer` is handled correctly.
   * @param opts - Optional limits controlling how much of the buffer the
   *   verifier will walk. Uses safe built-in defaults when omitted.
   */
  constructor(buf: DataView, opts?: VerifierOptions) {
    this.buf = buf;
    this.opts = {
      maxDepth: opts?.maxDepth ?? DEFAULT_MAX_DEPTH,
      maxTables: opts?.maxTables ?? DEFAULT_MAX_TABLES,
      maxApparentSize: opts?.maxApparentSize ?? DEFAULT_MAX_APPARENT_SIZE,
      ignoreNullTerminator: opts?.ignoreNullTerminator ?? false,
    };
    this.depth = 0;
    this.numTables = 0;
    this.apparentSize = 0;
  }

  /**
   * Asserts that `pos` is aligned to `align` bytes.
   *
   * A scalar of size `align` must begin at a buffer offset that is an exact
   * multiple of `align`; misaligned reads may produce incorrect values on
   * strict-alignment architectures.
   *
   * @param pos - Byte offset within the buffer to check.
   * @param align - Required byte alignment (e.g. `4` for `uint32`). Values of
   *   `1` or less are treated as always aligned and no check is performed.
   * @throws {VerificationError} With `kind === ErrorKind.Unaligned` when
   *   `pos % align !== 0`.
   */
  checkAlignment(pos: number, align: number): void {
    if (align > 1 && pos % align !== 0) {
      throw {
        kind: ErrorKind.Unaligned,
        offset: pos,
        trace: [],
      } satisfies VerificationError;
    }
  }

  /**
   * Asserts that the byte range `[pos, pos + size)` lies entirely within the
   * buffer and that the running `apparentSize` accumulator stays within the
   * configured limit.
   *
   * `apparentSize` accumulates every range checked, including overlapping ones,
   * so an adversarial buffer that points many fields to the same small region
   * will still be caught when the cumulative total exceeds `maxApparentSize`.
   *
   * @param pos - Start byte offset of the range to validate.
   * @param size - Number of bytes in the range. Must be non-negative.
   * @throws {VerificationError} With `kind === ErrorKind.RangeOutOfBounds` when
   *   `pos < 0`, `size < 0`, or `pos + size > buf.byteLength`.
   * @throws {VerificationError} With `kind === ErrorKind.ApparentSizeTooLarge`
   *   when the updated accumulator exceeds `opts.maxApparentSize`.
   */
  checkRange(pos: number, size: number): void {
    const end = pos + size;
    if (end > this.buf.byteLength || pos < 0 || size < 0) {
      throw {
        kind: ErrorKind.RangeOutOfBounds,
        offset: pos,
        trace: [],
      } satisfies VerificationError;
    }
    this.apparentSize += size;
    if (this.apparentSize > this.opts.maxApparentSize) {
      throw {
        kind: ErrorKind.ApparentSizeTooLarge,
        offset: pos,
        trace: [],
      } satisfies VerificationError;
    }
  }

  /**
   * Performs a range-checked little-endian `uint32` read at `pos`.
   *
   * Validates that `[pos, pos + 4)` is within bounds before reading, so callers
   * do not need to call `checkRange` separately for four-byte unsigned values.
   *
   * @param pos - Byte offset to read from.
   * @returns The unsigned 32-bit integer stored at `pos` in little-endian order.
   * @throws {VerificationError} With `kind === ErrorKind.RangeOutOfBounds` when
   *   the four bytes starting at `pos` exceed the buffer bounds.
   */
  readUint32(pos: number): number {
    this.checkRange(pos, SIZEOF_INT);
    return this.buf.getUint32(pos, true);
  }

  /**
   * Performs a range-checked little-endian `int32` read at `pos`.
   *
   * Validates that `[pos, pos + 4)` is within bounds before reading, so callers
   * do not need to call `checkRange` separately for four-byte signed values.
   *
   * @param pos - Byte offset to read from.
   * @returns The signed 32-bit integer stored at `pos` in little-endian order.
   * @throws {VerificationError} With `kind === ErrorKind.RangeOutOfBounds` when
   *   the four bytes starting at `pos` exceed the buffer bounds.
   */
  readInt32(pos: number): number {
    this.checkRange(pos, SIZEOF_INT);
    return this.buf.getInt32(pos, true);
  }

  /**
   * Performs a range-checked `uint8` read at `pos`.
   *
   * @param pos - Byte offset to read from.
   * @returns The unsigned 8-bit integer stored at `pos`.
   * @throws {VerificationError} With `kind === ErrorKind.RangeOutOfBounds` when
   *   the byte at `pos` exceeds the buffer bounds.
   */
  readUint8(pos: number): number {
    this.checkRange(pos, 1);
    return this.buf.getUint8(pos);
  }

  /**
   * Validates the vtable referenced by the table rooted at `tablePos`.
   *
   * A FlatBuffer table begins with a signed 32-bit `soffset`. The vtable is
   * located at `tablePos - soffset`. This method confirms that:
   *
   * 1. `tablePos` itself is within buffer bounds.
   * 2. The derived `vtablePos` is a non-negative in-bounds address.
   * 3. The vtable's `vtable_size` field (first `uint16`) is at least
   *    `2 * sizeof(uint16)` (the two mandatory header fields) and is a
   *    multiple of `sizeof(uint16)`.
   * 4. The full `vtable_size` range starting at `vtablePos` is within bounds.
   *
   * @param tablePos - Byte offset within the buffer where the table begins.
   * @throws {VerificationError} With `kind === ErrorKind.SignedOffsetOutOfBounds`
   *   when the computed vtable position lies outside the buffer.
   * @throws {VerificationError} With `kind === ErrorKind.InvalidVtable` when
   *   the vtable header is malformed or its declared size exceeds the buffer.
   * @throws {VerificationError} With `kind === ErrorKind.RangeOutOfBounds` on
   *   any out-of-bounds byte access during validation.
   */
  checkVtable(tablePos: number): void {
    this.checkRange(tablePos, SIZEOF_INT);
    const soffset = this.buf.getInt32(tablePos, true);
    const vtablePos = tablePos - soffset;
    if (vtablePos < 0 || vtablePos >= this.buf.byteLength) {
      throw {
        kind: ErrorKind.SignedOffsetOutOfBounds,
        offset: tablePos,
        trace: [],
      } satisfies VerificationError;
    }
    this.checkRange(vtablePos, SIZEOF_SHORT);
    const vtableLen = this.buf.getUint16(vtablePos, true);
    if (vtableLen < 2 * SIZEOF_SHORT || vtableLen % SIZEOF_SHORT !== 0) {
      throw {
        kind: ErrorKind.InvalidVtable,
        offset: vtablePos,
        trace: [],
      } satisfies VerificationError;
    }
    this.checkRange(vtablePos, vtableLen);
  }

  /**
   * Validates the structural envelope of a table: its vtable, table counter,
   * and nesting depth.
   *
   * This is the primary entry point for generated per-table verify functions.
   * It performs three sequential checks:
   *
   * 1. {@link checkVtable} — validates the vtable header and slot layout.
   * 2. {@link countTable} — increments the visited-table counter and enforces
   *    the `maxTables` limit.
   * 3. {@link pushDepth} — increments the recursion depth and enforces the
   *    `maxDepth` limit.
   *
   * Generated code is expected to call {@link popDepth} once all fields of
   * the table have been checked so that the depth counter correctly reflects
   * the current nesting level.
   *
   * @param tablePos - Byte offset within the buffer where the table begins.
   * @throws {VerificationError} Propagates any error thrown by `checkVtable`,
   *   `countTable`, or `pushDepth`.
   */
  checkTable(tablePos: number): void {
    this.checkVtable(tablePos);
    this.countTable();
    this.pushDepth();
  }

  /**
   * Validates a FlatBuffer string field at the given offset position.
   *
   * A FlatBuffer string is encoded as a relative signed offset followed by a
   * length-prefixed UTF-8 byte array with a null terminator. This method:
   *
   * 1. Reads the signed offset at `pos` and derives the absolute string start.
   * 2. Reads the `uint32` character-count prefix.
   * 3. Confirms the data bytes plus one terminator byte are within bounds.
   * 4. Decodes the character data with a fatal-mode UTF-8 decoder.
   * 5. Unless `ignoreNullTerminator` is set, confirms the byte at
   *    `dataStart + strLen` is `0x00`.
   *
   * @param pos - Byte offset within the buffer of the string offset field.
   * @throws {VerificationError} With `kind === ErrorKind.RangeOutOfBounds` when
   *   any part of the string lies outside the buffer.
   * @throws {VerificationError} With `kind === ErrorKind.Utf8Error` when the
   *   byte sequence is not valid UTF-8.
   * @throws {VerificationError} With `kind === ErrorKind.MissingNullTerminator`
   *   when the terminator byte is absent and `ignoreNullTerminator` is `false`.
   */
  checkString(pos: number): void {
    this.checkRange(pos, SIZEOF_INT);
    const strOffset = this.buf.getInt32(pos, true);
    const strStart = pos + strOffset;
    this.checkRange(strStart, SIZEOF_INT);
    const strLen = this.buf.getUint32(strStart, true);
    const dataStart = strStart + SIZEOF_INT;
    this.checkRange(dataStart, strLen + 1);

    const bytes = new Uint8Array(
      this.buf.buffer,
      this.buf.byteOffset + dataStart,
      strLen,
    );
    try {
      new TextDecoder('utf-8', {fatal: true}).decode(bytes);
    } catch {
      throw {
        kind: ErrorKind.Utf8Error,
        offset: dataStart,
        trace: [],
      } satisfies VerificationError;
    }

    if (!this.opts.ignoreNullTerminator) {
      const terminator = this.buf.getUint8(dataStart + strLen);
      if (terminator !== 0) {
        throw {
          kind: ErrorKind.MissingNullTerminator,
          offset: dataStart + strLen,
          trace: [],
        } satisfies VerificationError;
      }
    }
  }

  /**
   * Validates a FlatBuffer vector field and returns its element count.
   *
   * A FlatBuffer vector is encoded as a relative signed offset followed by a
   * `uint32` element count and then `count * elementSize` bytes of inline data.
   * This method confirms that:
   *
   * 1. The offset and element-count fields are in bounds.
   * 2. `count * elementSize` does not overflow a 32-bit integer.
   * 3. The complete data region is within buffer bounds.
   *
   * For vectors of scalar types the element size is the scalar's byte width.
   * For vectors of offsets (tables or strings) the element size is
   * `sizeof(uint32)` — the per-element offset field width.
   *
   * @param pos - Byte offset within the buffer of the vector offset field.
   * @param elementSize - Size in bytes of each vector element. Must be `> 0`.
   * @returns The number of elements declared by the vector's length prefix.
   * @throws {VerificationError} With `kind === ErrorKind.RangeOutOfBounds` on
   *   any out-of-bounds access.
   * @throws {VerificationError} With `kind === ErrorKind.InvalidVectorLength`
   *   when `count * elementSize` overflows.
   */
  checkVector(pos: number, elementSize: number): number {
    this.checkRange(pos, SIZEOF_INT);
    const vecOffset = this.buf.getInt32(pos, true);
    const vecStart = pos + vecOffset;
    this.checkRange(vecStart, SIZEOF_INT);
    const length = this.buf.getUint32(vecStart, true);
    const dataStart = vecStart + SIZEOF_INT;
    const totalSize = length * elementSize;
    if (elementSize > 0 && totalSize / elementSize !== length) {
      throw {
        kind: ErrorKind.InvalidVectorLength,
        offset: vecStart,
        trace: [],
      } satisfies VerificationError;
    }
    this.checkRange(dataStart, totalSize);
    return length;
  }

  /**
   * Validates a vector of FlatBuffer tables by first checking the vector
   * envelope and then recursively verifying each element.
   *
   * Combines {@link checkVector} (with `elementSize = sizeof(uint32)` for the
   * per-element offset fields) with a loop that calls `verifyElem` for every
   * element position. Generated per-table verify functions are passed as
   * `verifyElem` so each nested table is fully validated.
   *
   * @param pos - Byte offset within the buffer of the vector offset field.
   * @param verifyElem - Callback invoked once per element. Receives this
   *   `Verifier` instance and the byte offset of the element's offset field.
   *   It is expected to call {@link checkTable} and recursively check all
   *   nested fields before calling {@link popDepth}.
   * @throws {VerificationError} Propagates any error thrown by
   *   {@link checkVector} or by `verifyElem`.
   */
  checkVectorOfTables(
    pos: number,
    verifyElem: (v: Verifier, pos: number) => void,
  ): void {
    const length = this.checkVector(pos, SIZEOF_INT);
    this.checkRange(pos, SIZEOF_INT);
    const vecOffset = this.buf.getInt32(pos, true);
    const vecStart = pos + vecOffset;
    const dataStart = vecStart + SIZEOF_INT;
    for (let i = 0; i < length; i++) {
      const elemPos = dataStart + i * SIZEOF_INT;
      verifyElem(this, elemPos);
    }
  }

  /**
   * Validates that a scalar field's data bytes are within buffer bounds.
   *
   * Looks up vtable slot `vOffset` for the table at `tablePos` via
   * {@link _resolveVtableField}. If the slot is present (non-zero field offset),
   * confirms that `size` bytes starting at the resolved position are in bounds.
   * If the slot is absent the field is optional and no error is raised.
   *
   * @param tablePos - Byte offset within the buffer where the table begins.
   * @param vOffset - Vtable slot byte offset for the target field (e.g. `4`
   *   for the first user field, `6` for the second, etc.).
   * @param size - Byte size of the scalar type (e.g. `1` for `bool`/`byte`,
   *   `2` for `int16`, `4` for `int32`/`float`, `8` for `int64`/`double`).
   * @throws {VerificationError} Propagates any error thrown by `checkRange` or
   *   `_resolveVtableField`.
   */
  checkScalarField(tablePos: number, vOffset: number, size: number): void {
    const fieldPos = this._resolveVtableField(tablePos, vOffset);
    if (fieldPos !== 0) {
      this.checkRange(fieldPos, size);
    }
  }

  /**
   * Resolves a vtable slot to an absolute buffer position for offset-type fields.
   *
   * Offset fields include strings, nested tables, and vectors — any field whose
   * value is a relative offset rather than an inline scalar. Generated verify
   * functions call this method and then pass the returned position to the
   * appropriate type-specific `check*` method.
   *
   * Returns `0` when the vtable slot is absent, meaning the field is not present
   * in the buffer (it is optional or defaulted).
   *
   * @param tablePos - Byte offset within the buffer where the table begins.
   * @param vOffset - Vtable slot byte offset for the target field.
   * @returns The absolute byte offset of the field within the buffer, or `0` if
   *   the field is absent.
   * @throws {VerificationError} Propagates any error thrown by
   *   `_resolveVtableField`.
   */
  checkOffsetField(tablePos: number, vOffset: number): number {
    return this._resolveVtableField(tablePos, vOffset);
  }

  /**
   * Asserts that a required field is present in the vtable.
   *
   * Looks up vtable slot `vOffset` for the table at `tablePos`. If the resolved
   * field offset is `0` (slot absent), throws a
   * {@link ErrorKind.MissingRequiredField} error so callers can distinguish a
   * missing-required-field failure from other range or structural errors.
   *
   * @param tablePos - Byte offset within the buffer where the table begins.
   * @param vOffset - Vtable slot byte offset for the required field.
   * @param fieldName - Schema name of the field, included in the thrown
   *   {@link VerificationError} to aid debugging.
   * @throws {VerificationError} With `kind === ErrorKind.MissingRequiredField`
   *   when the vtable slot is absent. Propagates any structural error thrown by
   *   `_resolveVtableField`.
   */
  checkRequiredField(
    tablePos: number,
    vOffset: number,
    fieldName: string,
  ): void {
    const fieldPos = this._resolveVtableField(tablePos, vOffset);
    if (fieldPos === 0) {
      throw {
        kind: ErrorKind.MissingRequiredField,
        field: fieldName,
        offset: tablePos,
        trace: [],
      } satisfies VerificationError;
    }
  }

  /**
   * Asserts that a union's type and value fields are either both present or both
   * absent in the vtable.
   *
   * FlatBuffer unions are encoded as a pair of fields: a `uint8` type discriminant
   * and an offset to the union value table. If only one of these fields is present
   * the buffer is malformed. This method reads both vtable slots and throws
   * {@link ErrorKind.InconsistentUnion} when exactly one of them is populated.
   *
   * @param tablePos - Byte offset within the buffer where the owning table begins.
   * @param typeVOffset - Vtable slot byte offset for the union type discriminant field.
   * @param valueVOffset - Vtable slot byte offset for the union value offset field.
   * @param fieldName - Schema name of the union field, included in the thrown
   *   {@link VerificationError} to aid debugging.
   * @throws {VerificationError} With `kind === ErrorKind.InconsistentUnion` when
   *   exactly one of the two fields is present.
   */
  checkUnionConsistency(
    tablePos: number,
    typeVOffset: number,
    valueVOffset: number,
    fieldName: string,
  ): void {
    const typeFieldPos = this._resolveVtableField(tablePos, typeVOffset);
    const valueFieldPos = this._resolveVtableField(tablePos, valueVOffset);
    const typePresent = typeFieldPos !== 0;
    const valuePresent = valueFieldPos !== 0;
    if (typePresent !== valuePresent) {
      throw {
        kind: ErrorKind.InconsistentUnion,
        field: fieldName,
        offset: tablePos,
        trace: [],
      } satisfies VerificationError;
    }
  }

  /**
   * Reads a `uint8` value from the field at vtable slot `vOffset` within the
   * table at `tablePos`. Returns `0` when the field is absent. Used by
   * generated code to read union type discriminants for per-variant dispatch.
   *
   * @param tablePos - Byte offset within the buffer where the table begins.
   * @param vOffset - Vtable slot byte offset for the target field.
   * @returns The uint8 value at the field position, or `0` if absent.
   */
  readFieldUint8(tablePos: number, vOffset: number): number {
    const fieldPos = this._resolveVtableField(tablePos, vOffset);
    if (fieldPos === 0) {
      return 0;
    }
    this.checkRange(fieldPos, 1);
    return this.buf.getUint8(fieldPos);
  }

  /**
   * Increments the recursion depth counter and enforces the `maxDepth` limit.
   *
   * Called at the start of every table or vector nesting level. Generated verify
   * functions call `pushDepth` after {@link checkTable} and must pair each call
   * with a corresponding {@link popDepth} once all nested fields have been
   * validated. This pairing ensures the depth counter accurately reflects the
   * current call depth.
   *
   * @throws {VerificationError} With `kind === ErrorKind.DepthLimitReached` when
   *   the depth counter exceeds `opts.maxDepth` after incrementing.
   */
  pushDepth(): void {
    this.depth += 1;
    if (this.depth > this.opts.maxDepth) {
      throw {
        kind: ErrorKind.DepthLimitReached,
        offset: 0,
        trace: [],
      } satisfies VerificationError;
    }
  }

  /**
   * Decrements the recursion depth counter.
   *
   * Must be called by generated verify functions once all fields of a table or
   * all elements of a vector have been checked, to mirror the preceding
   * {@link pushDepth} call and keep the depth counter consistent.
   */
  popDepth(): void {
    this.depth -= 1;
  }

  /**
   * Increments the visited-table counter and enforces the `maxTables` limit.
   *
   * Called once per table encountered during traversal (via {@link checkTable}).
   * Prevents unbounded CPU consumption on adversarial buffers that reference a
   * very large number of distinct table objects.
   *
   * @throws {VerificationError} With `kind === ErrorKind.TooManyTables` when
   *   the counter exceeds `opts.maxTables` after incrementing.
   */
  countTable(): void {
    this.numTables += 1;
    if (this.numTables > this.opts.maxTables) {
      throw {
        kind: ErrorKind.TooManyTables,
        offset: 0,
        trace: [],
      } satisfies VerificationError;
    }
  }

  private _resolveVtableField(tablePos: number, vOffset: number): number {
    this.checkRange(tablePos, SIZEOF_INT);
    const soffset = this.buf.getInt32(tablePos, true);
    const vtablePos = tablePos - soffset;
    this.checkRange(vtablePos, SIZEOF_SHORT);
    const vtableLen = this.buf.getUint16(vtablePos, true);
    if (vOffset >= vtableLen) {
      return 0;
    }
    this.checkRange(vtablePos + vOffset, SIZEOF_SHORT);
    const fieldOffset = this.buf.getUint16(vtablePos + vOffset, true);
    if (fieldOffset === 0) {
      return 0;
    }
    return tablePos + fieldOffset;
  }
}
