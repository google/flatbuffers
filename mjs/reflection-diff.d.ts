/**
 * @module reflection-diff
 *
 * Field-level diffing of two FlatBuffer payloads using a binary schema
 * (`.bfbs`) file.  Intended for CDO configuration audit logging: given two
 * serialized FlatBuffer messages of the same root type, returns every field
 * that was added, removed, or changed between them.
 *
 * This module imports only from `./reflection` and uses the same standalone
 * `DataView`-based approach with no additional runtime dependencies.
 *
 * @example
 * ```ts
 * import { loadSchema } from './reflection';
 * import { diffBuffers } from './reflection-diff';
 *
 * const bfbs = new Uint8Array(await (await fetch('/schemas/ln2.bfbs')).arrayBuffer());
 * const schema = loadSchema(bfbs);
 *
 * const oldBuf = new DataView(oldArrayBuffer);
 * const newBuf = new DataView(newArrayBuffer);
 *
 * const deltas = diffBuffers(schema, 'Ln2.ControllerConfig', oldBuf, newBuf);
 * for (const d of deltas) {
 *   console.log(`${d.path}: ${d.oldValue} -> ${d.newValue} (${d.kind})`);
 * }
 * ```
 */
import { ReflectionSchema } from './reflection';
/**
 * Describes the kind of change detected for a single field between two
 * FlatBuffer payloads.
 *
 * - `'changed'` — the field is present in both buffers with different values.
 * - `'added'`   — the field is present in bufB but absent in bufA.
 * - `'removed'` — the field is present in bufA but absent in bufB.
 */
export type DeltaKind = 'changed' | 'added' | 'removed';
/**
 * Represents a single field-level difference between two FlatBuffer payloads
 * of the same root type.
 *
 * `path` uses dot notation for nested fields and bracket notation for vector
 * indices.  Examples:
 * - `"name"` — top-level string field
 * - `"pos.x"` — field `x` inside nested table `pos`
 * - `"inventory[2]"` — element at index 2 of the `inventory` vector
 */
export interface FieldDelta {
    /** Dot-separated (and bracket-indexed) field path within the root table. */
    path: string;
    /** Whether the field was changed, added, or removed. */
    kind: DeltaKind;
    /**
     * Value read from bufA; `null` when `kind === 'added'` (field was not
     * present in bufA).
     */
    oldValue: unknown;
    /**
     * Value read from bufB; `null` when `kind === 'removed'` (field was not
     * present in bufB).
     */
    newValue: unknown;
}
/**
 * Compare two FlatBuffer payloads against a compiled binary schema and return
 * the field-level differences between them.  Both buffers must encode the same
 * root table type identified by `typeName`.
 *
 * @param schema   - A {@link ReflectionSchema} loaded from a `.bfbs` file via
 *                   {@link loadSchema}.  Used solely for type introspection; it
 *                   is not a data buffer.
 * @param typeName - Fully qualified FlatBuffers type name of the root table,
 *                   e.g. `"Ln2.ControllerConfig"` or
 *                   `"MyGame.Example.Monster"`.  Must match exactly the name
 *                   that `flatc` writes into the `.bfbs` file.
 * @param bufA     - `DataView` over the first (old) serialized FlatBuffer data.
 *                   The first 4 bytes must contain the root UOffset as
 *                   produced by a FlatBuffers `Builder`.
 * @param bufB     - `DataView` over the second (new) serialized FlatBuffer.
 *
 * @returns An array of {@link FieldDelta} records, one per differing leaf
 *          value, ordered by schema field declaration order (depth-first for
 *          nested tables, element order for vectors).  An empty array
 *          indicates the two buffers are semantically identical for all fields
 *          described by the schema.
 *
 * @throws {Error} If `typeName` is not found in the schema.
 * @throws {Error} If either buffer is too short to contain a root UOffset
 *                 (fewer than 4 bytes).
 *
 * @example
 * ```ts
 * import { loadSchema } from './reflection';
 * import { diffBuffers } from './reflection-diff';
 *
 * const bfbs = new Uint8Array(fs.readFileSync('ln2.bfbs'));
 * const schema = loadSchema(bfbs);
 *
 * const deltas = diffBuffers(schema, 'Ln2.ControllerConfig', oldBuf, newBuf);
 * for (const d of deltas) {
 *   auditLog.append({ field: d.path, before: d.oldValue, after: d.newValue });
 * }
 * ```
 */
export declare function diffBuffers(schema: ReflectionSchema, typeName: string, bufA: DataView, bufB: DataView): FieldDelta[];
