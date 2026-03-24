/**
 * @module reflection-json
 *
 * Reflection-based JSON ↔ FlatBuffer serialization for TypeScript.
 *
 * Uses a binary schema (`.bfbs`) to dynamically pack and unpack FlatBuffers
 * messages to/from plain JavaScript objects without generated code.
 *
 * This module depends only on `./reflection` — it adds no other runtime
 * imports and has no dependency on the core FlatBuffers TypeScript runtime.
 *
 * @example
 * ```ts
 * import { loadSchema } from './reflection';
 * import { reflectUnpack, reflectPack } from './reflection-json';
 *
 * const bfbs = new Uint8Array(await (await fetch('/schemas/ln2.bfbs')).arrayBuffer());
 * const schema = loadSchema(bfbs);
 *
 * // Deserialize an existing buffer:
 * const data = new Uint8Array(someArrayBuffer);
 * const obj = reflectUnpack(schema, 'Ln2.StatusMessage', data);
 * console.log(obj?.device_name);
 *
 * // Serialize a plain object to a FlatBuffer:
 * const buf = reflectPack(schema, 'Ln2.StatusMessage', { device_name: 'Elev-1A', floor: 3 });
 * ```
 */
import { ReflectionSchema } from './reflection';
/**
 * Deserializes a FlatBuffer byte slice into a plain JavaScript object using
 * schema reflection from a binary `.bfbs` schema.
 *
 * @param schema   - Parsed binary schema from {@link loadSchema}.
 * @param typeName - Fully qualified FlatBuffers table name, e.g.
 *                   `"MyGame.Example.Monster"`.
 * @param buf      - Complete root-offset-prefixed FlatBuffer bytes.
 * @returns A `Record<string, unknown>` containing all present fields, or
 *          `null` if the type name is not found in the schema.
 *
 * Nested tables are returned as nested records.  Vectors of scalars and
 * strings are arrays.  Union fields produce two keys:
 * `"<field>_type"` (string variant name) and `"<field>"` (nested record).
 *
 * @example
 * ```ts
 * const result = reflectUnpack(schema, 'MyGame.Example.Monster', data);
 * console.log(result?.name); // "MyMonster"
 * ```
 */
export declare function reflectUnpack(schema: ReflectionSchema, typeName: string, buf: Uint8Array): Record<string, unknown> | null;
/**
 * Serializes a plain JavaScript object into a FlatBuffer using schema reflection.
 *
 * @param schema     - Parsed binary schema from {@link loadSchema}.
 * @param typeName   - Fully qualified FlatBuffers table name, e.g.
 *                     `"MyGame.Example.Monster"`.
 * @param inputObj   - Plain object whose keys match field names declared in
 *                     the `.fbs` source.
 * @returns A `Uint8Array` containing the finished FlatBuffer, or `null` if
 *          the type name is not found in the schema.
 *
 * Value representations:
 * - Bool: JavaScript `boolean`.
 * - Scalars (int/float): JavaScript `number` or `bigint` for 64-bit fields.
 * - Strings: JavaScript `string`.
 * - Nested tables: nested plain object.
 * - Vectors: JavaScript `Array`.
 * - Unions: object with `"<field>_type"` (string variant name) and
 *   `"<field>"` (nested object) keys.
 *
 * @example
 * ```ts
 * const buf = reflectPack(schema, 'MyGame.Example.Monster', { name: 'Orc', hp: 300 });
 * if (buf) { socket.send(buf); }
 * ```
 */
export declare function reflectPack(schema: ReflectionSchema, typeName: string, inputObj: Record<string, unknown>): Uint8Array | null;
