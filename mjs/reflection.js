/**
 * @module reflection
 *
 * Standalone FlatBuffers reflection runtime for dynamic field access using a
 * binary schema (`.bfbs`) file produced by `flatc --binary --schema`.
 *
 * This module has **zero imports** from the core FlatBuffers runtime, making
 * it safe to tree-shake into dev-tool or schema-inspection bundles without
 * pulling in the full serialization library.  It uses only the browser-native
 * `DataView` and `TextDecoder` APIs.
 *
 * Typical usage in liftNet_frontend — load a schema once, then use it to read
 * fields from any data buffer that conforms to that schema:
 *
 * @example
 * ```ts
 * import { loadSchema, getFieldString } from './reflection';
 *
 * // 1. Fetch the compiled binary schema produced by flatc.
 * const bfbsBytes = new Uint8Array(await (await fetch('/schemas/ln2.bfbs')).arrayBuffer());
 *
 * // 2. Parse the schema.
 * const schema = loadSchema(bfbsBytes);
 *
 * // 3. Look up the table descriptor you want to read.
 * const msgObj = schema.objectByName('Ln2.StatusMessage');
 * if (!msgObj) throw new Error('StatusMessage not found in schema');
 *
 * // 4. Find the field descriptor for the field you care about.
 * const nameField = msgObj.fieldByName('device_name');
 * if (!nameField) throw new Error('device_name not found');
 *
 * // 5. Read the field from a live data buffer.
 * //    tablePos is the byte offset of the root table inside dataBuf
 * //    (typically obtained from the 4-byte root offset at position 0).
 * const dataBuf = new DataView(someArrayBuffer);
 * const rootOffset = dataBuf.getUint32(0, true);
 * const deviceName = getFieldString(dataBuf, rootOffset, nameField);
 * console.log(deviceName); // e.g. "Elevator-3A"
 * ```
 */
// ts/reflection.ts — separate entry point for dev tools
// Dynamic field access using binary schema (.bfbs) files.
// This file is standalone: no imports from other flatbuffers ts modules.
// Uses raw DataView + TextDecoder for minimal bundle size.
const textDecoder = new TextDecoder();
/**
 * Mirrors the `BaseType` enum from `reflection.fbs`.
 *
 * Each member names the scalar or structural category of a FlatBuffers field.
 * These values are embedded verbatim in the `.bfbs` binary schema by `flatc`,
 * so numeric equality is guaranteed to match the generated schema.
 *
 * Use these constants when switching on {@link ReflectionType.baseType} or
 * {@link ReflectionType.element} to dispatch to the correct `DataView` read
 * method (e.g. `getInt32` for `Int`, `getFloat32` for `Float`).
 *
 * @example
 * ```ts
 * const base = fieldType.baseType();
 * if (base === ReflectionBaseType.String) {
 *   // field holds a FlatBuffers string reference
 * } else if (base === ReflectionBaseType.Vector) {
 *   // field holds a vector; check element() for the element type
 * }
 * ```
 */
export var ReflectionBaseType;
(function (ReflectionBaseType) {
    /** No type / unset. */
    ReflectionBaseType[ReflectionBaseType["None"] = 0] = "None";
    /** Union type discriminant (u8). */
    ReflectionBaseType[ReflectionBaseType["UType"] = 1] = "UType";
    /** Boolean stored as a single byte. */
    ReflectionBaseType[ReflectionBaseType["Bool"] = 2] = "Bool";
    /** Signed 8-bit integer. */
    ReflectionBaseType[ReflectionBaseType["Byte"] = 3] = "Byte";
    /** Unsigned 8-bit integer. */
    ReflectionBaseType[ReflectionBaseType["UByte"] = 4] = "UByte";
    /** Signed 16-bit integer. */
    ReflectionBaseType[ReflectionBaseType["Short"] = 5] = "Short";
    /** Unsigned 16-bit integer. */
    ReflectionBaseType[ReflectionBaseType["UShort"] = 6] = "UShort";
    /** Signed 32-bit integer. */
    ReflectionBaseType[ReflectionBaseType["Int"] = 7] = "Int";
    /** Unsigned 32-bit integer. */
    ReflectionBaseType[ReflectionBaseType["UInt"] = 8] = "UInt";
    /** Signed 64-bit integer. */
    ReflectionBaseType[ReflectionBaseType["Long"] = 9] = "Long";
    /** Unsigned 64-bit integer. */
    ReflectionBaseType[ReflectionBaseType["ULong"] = 10] = "ULong";
    /** 32-bit IEEE 754 floating-point. */
    ReflectionBaseType[ReflectionBaseType["Float"] = 11] = "Float";
    /** 64-bit IEEE 754 floating-point. */
    ReflectionBaseType[ReflectionBaseType["Double"] = 12] = "Double";
    /** FlatBuffers inline string (UTF-8, length-prefixed). */
    ReflectionBaseType[ReflectionBaseType["String"] = 13] = "String";
    /** Variable-length vector of scalars or tables. */
    ReflectionBaseType[ReflectionBaseType["Vector"] = 14] = "Vector";
    /** Nested table or struct object. `index()` gives the schema object index. */
    ReflectionBaseType[ReflectionBaseType["Obj"] = 15] = "Obj";
    /** Tagged union. `index()` gives the schema enum index. */
    ReflectionBaseType[ReflectionBaseType["Union"] = 16] = "Union";
    /** Fixed-length array of scalars (only valid inside structs). */
    ReflectionBaseType[ReflectionBaseType["Array"] = 17] = "Array";
    /** 64-bit-aligned vector (FlatBuffers64 extension). */
    ReflectionBaseType[ReflectionBaseType["Vector64"] = 18] = "Vector64";
})(ReflectionBaseType || (ReflectionBaseType = {}));
// ── vtable primitives ──────────────────────────────────────────────────────
/**
 * Look up a field in the vtable.
 * Returns the byte offset from tablePos to the field data, or 0 if absent.
 *
 * Algorithm:
 *   1. soffset = buf.getInt32(tablePos)      — signed offset to vtable
 *   2. vtablePos = tablePos - soffset
 *   3. vtableSize = buf.getUint16(vtablePos) — total vtable byte size
 *   4. if voffset >= vtableSize: field absent, return 0
 *   5. entry = buf.getUint16(vtablePos + voffset)
 *   6. return entry (0 means field not set)
 */
function readVtableEntry(buf, tablePos, voffset) {
    const soffset = buf.getInt32(tablePos, true);
    const vtablePos = tablePos - soffset;
    const vtableSize = buf.getUint16(vtablePos, true);
    if (voffset >= vtableSize) {
        return 0;
    }
    return buf.getUint16(vtablePos + voffset, true);
}
/**
 * Read a FlatBuffers string from the given field position.
 * fieldPos is the absolute byte position in buf where the UOffset to the string lives.
 * Returns the decoded string.
 */
function readString(buf, fieldPos) {
    const uoffset = buf.getUint32(fieldPos, true);
    const strStart = fieldPos + uoffset;
    const length = buf.getUint32(strStart, true);
    const bytes = new Uint8Array(buf.buffer, buf.byteOffset + strStart + 4, length);
    return textDecoder.decode(bytes);
}
/**
 * Follow a UOffset at fieldPos to get the child table's absolute position.
 */
function readTable(buf, fieldPos) {
    return fieldPos + buf.getUint32(fieldPos, true);
}
/**
 * Read a vector at the given field position.
 * Returns [count, dataStart] where dataStart is the byte position of element 0.
 */
function readVector(buf, fieldPos) {
    const uoffset = buf.getUint32(fieldPos, true);
    const vecStart = fieldPos + uoffset;
    const count = buf.getUint32(vecStart, true);
    return [count, vecStart + 4];
}
/**
 * Read element i of a vector of tables.
 * Each element slot is a 4-byte UOffset; the table is at slot + uoffset.
 */
function readVectorTableElement(buf, dataStart, index) {
    const elemPos = dataStart + index * 4;
    return elemPos + buf.getUint32(elemPos, true);
}
// ── ReflectionType ─────────────────────────────────────────────────────────
/**
 * Describes the type of a single FlatBuffers field as recorded in the binary
 * schema.  Wraps the `reflection.Type` table from `reflection.fbs`.
 *
 * Obtain instances via {@link ReflectionField.type}.
 *
 * @example
 * ```ts
 * const fieldType = field.type();
 * if (fieldType?.baseType() === ReflectionBaseType.Vector) {
 *   console.log('element type:', fieldType.element());
 * }
 * ```
 */
export class ReflectionType {
    /**
     * @param buf      - `DataView` over the raw `.bfbs` schema bytes.
     * @param tablePos - Absolute byte position of the `reflection.Type` table
     *                   within `buf`.
     */
    constructor(buf, tablePos) {
        this.buf = buf;
        this.tablePos = tablePos;
    }
    /**
     * Returns the base type of this field.
     *
     * For scalar fields this is the concrete scalar kind (e.g.
     * `ReflectionBaseType.Int`).  For composite fields it is one of
     * `Vector`, `Obj`, `Union`, or `Array`; call {@link element} or
     * {@link index} for the inner type in those cases.
     *
     * @returns The {@link ReflectionBaseType} constant stored in the schema.
     *          Falls back to `ReflectionBaseType.None` when the vtable entry is
     *          absent (schema generated without type info).
     */
    baseType() {
        const entry = readVtableEntry(this.buf, this.tablePos, 4);
        if (entry === 0)
            return ReflectionBaseType.None;
        return this.buf.getUint8(this.tablePos + entry);
    }
    /**
     * Returns the element type for `Vector` and `Array` base types.
     *
     * For a `Vector<int>` field, `baseType()` returns `Vector` and
     * `element()` returns `Int`.  For all other base types this value is
     * `None` and should be ignored.
     *
     * @returns The {@link ReflectionBaseType} of each vector/array element,
     *          or `ReflectionBaseType.None` if not applicable.
     */
    element() {
        const entry = readVtableEntry(this.buf, this.tablePos, 6);
        if (entry === 0)
            return ReflectionBaseType.None;
        return this.buf.getUint8(this.tablePos + entry);
    }
    /**
     * Returns the schema-level index for `Obj`, `Union`, and enum-backed types.
     *
     * - For `Obj` base type: index into {@link ReflectionSchema.objects}.
     * - For `Union` / enum base type: index into the schema's `enums` vector.
     * - For all other base types: `-1` (not applicable).
     *
     * @returns A non-negative integer index, or `-1` when not applicable.
     */
    index() {
        const entry = readVtableEntry(this.buf, this.tablePos, 8);
        if (entry === 0)
            return -1;
        return this.buf.getInt32(this.tablePos + entry, true);
    }
    /**
     * Returns the declared element count for fixed-length `Array` fields.
     *
     * Only meaningful when {@link baseType} is `ReflectionBaseType.Array`.  For
     * example, a field declared as `[float:8]` has `baseType` == `Array`,
     * `element` == `Float`, and `fixedLength` == `8`.
     *
     * @returns The element count, or `0` for non-array types or absent entries.
     */
    fixedLength() {
        const entry = readVtableEntry(this.buf, this.tablePos, 10);
        if (entry === 0)
            return 0;
        return this.buf.getUint16(this.tablePos + entry, true);
    }
    /**
     * Returns the byte size of the value described by {@link baseType}.
     *
     * For example, an `Int` field has `baseSize` == `4`, a `Double` field has
     * `baseSize` == `8`.  Defaults to `4` when absent (the most common case:
     * 4-byte offsets for tables and strings).
     *
     * @returns The base size in bytes, or `4` if absent.
     */
    baseSize() {
        const entry = readVtableEntry(this.buf, this.tablePos, 12);
        if (entry === 0)
            return 4;
        return this.buf.getUint32(this.tablePos + entry, true);
    }
    /**
     * Returns the byte size of each element in a `Vector` or `Array` field.
     *
     * For scalar element types this matches the natural width of the scalar; for
     * table element types it is `4` (the size of a `UOffset` reference).
     * Returns `0` if absent or not applicable.
     *
     * @returns The element size in bytes, or `0` if absent.
     */
    elementSize() {
        const entry = readVtableEntry(this.buf, this.tablePos, 14);
        if (entry === 0)
            return 0;
        return this.buf.getUint32(this.tablePos + entry, true);
    }
}
// ── ReflectionField ────────────────────────────────────────────────────────
/**
 * Describes a single field declared inside a FlatBuffers table or struct.
 * Wraps the `reflection.Field` table from `reflection.fbs`.
 *
 * Obtain instances via {@link ReflectionObject.fields} or
 * {@link ReflectionObject.fieldByName}.
 *
 * @example
 * ```ts
 * const field = obj.fieldByName('status_code');
 * if (field) {
 *   console.log(field.name());    // "status_code"
 *   console.log(field.offset());  // vtable slot, e.g. 8
 *   const value = getFieldInt(dataBuf, tablePos, field);
 * }
 * ```
 */
export class ReflectionField {
    /**
     * @param buf      - `DataView` over the raw `.bfbs` schema bytes.
     * @param tablePos - Absolute byte position of the `reflection.Field` table
     *                   within `buf`.
     */
    constructor(buf, tablePos) {
        this.buf = buf;
        this.tablePos = tablePos;
    }
    /**
     * Returns the field name as declared in the `.fbs` source file.
     *
     * @returns The field name string, or an empty string if the vtable entry is
     *          absent (malformed schema).
     */
    name() {
        const entry = readVtableEntry(this.buf, this.tablePos, 4);
        if (entry === 0)
            return '';
        return readString(this.buf, this.tablePos + entry);
    }
    /**
     * Returns the {@link ReflectionType} descriptor for this field.
     *
     * Use the returned object to determine the field's base type (scalar,
     * string, vector, struct, union, …) and — for composite types — the
     * element type or schema object index.
     *
     * @returns A `ReflectionType` instance, or `null` if the type sub-table is
     *          absent in the schema.
     */
    type() {
        const entry = readVtableEntry(this.buf, this.tablePos, 6);
        if (entry === 0)
            return null;
        const typeTablePos = readTable(this.buf, this.tablePos + entry);
        return new ReflectionType(this.buf, typeTablePos);
    }
    /**
     * Returns the vtable offset (voffset) used to locate this field's data in
     * any buffer that conforms to the enclosing table's schema.
     *
     * Pass this value as the `voffset` argument to `readVtableEntry` when
     * performing a manual vtable lookup on a live data buffer.  The dynamic
     * accessor functions ({@link getFieldString}, {@link getFieldInt}, etc.)
     * call this internally.
     *
     * @returns The 16-bit vtable slot index, or `0` if absent (always non-zero
     *          for valid schemas).
     */
    offset() {
        const entry = readVtableEntry(this.buf, this.tablePos, 10);
        if (entry === 0)
            return 0;
        return this.buf.getUint16(this.tablePos + entry, true);
    }
    /**
     * Returns the default value for integer fields as declared in the schema.
     *
     * When a field is absent from a data buffer (vtable entry is 0), the
     * FlatBuffers specification mandates returning this default rather than
     * zero.  The dynamic accessor functions do NOT apply this default
     * automatically — callers that require default-value semantics should fall
     * back to `field.defaultInteger()` when {@link getFieldInt} returns `null`.
     *
     * The value is stored as a 64-bit signed integer in the schema; it is
     * returned as a `bigint` to avoid precision loss for values outside the
     * safe integer range.
     *
     * @returns The declared `int64` default, or `0n` if no default is stored.
     */
    defaultInteger() {
        const entry = readVtableEntry(this.buf, this.tablePos, 12);
        if (entry === 0)
            return 0n;
        const fieldPos = this.tablePos + entry;
        const lo = BigInt(this.buf.getUint32(fieldPos, true));
        const hi = BigInt(this.buf.getInt32(fieldPos + 4, true));
        return BigInt.asIntN(64, lo + (hi * 0x100000000n));
    }
    /**
     * Returns the default value for floating-point fields as declared in the
     * schema.
     *
     * Analogous to {@link defaultInteger}: callers that require default-value
     * semantics should use this when {@link getFieldFloat} returns `null`.
     *
     * @returns The declared `float64` default, or `0` if no default is stored.
     */
    defaultReal() {
        const entry = readVtableEntry(this.buf, this.tablePos, 14);
        if (entry === 0)
            return 0;
        return this.buf.getFloat64(this.tablePos + entry, true);
    }
    /**
     * Returns whether this field is marked `required` in the schema.
     *
     * A `required` field must always be present in a valid buffer.  Validation
     * tools can use this to flag non-conformant data without needing to know
     * the field's type.
     *
     * @returns `true` if the field is required, `false` otherwise.
     */
    required() {
        const entry = readVtableEntry(this.buf, this.tablePos, 18);
        if (entry === 0)
            return false;
        return this.buf.getUint8(this.tablePos + entry) !== 0;
    }
    /**
     * Returns the field identifier (`id`) assigned by `flatc`.  For table
     * fields this is the sequential index used to generate vtable slots; for
     * struct fields it is the zero-based declaration order.
     *
     * @returns The field ID, or `0` if absent.
     */
    id() {
        const entry = readVtableEntry(this.buf, this.tablePos, 8);
        if (entry === 0)
            return 0;
        return this.buf.getUint16(this.tablePos + entry, true);
    }
    /**
     * Returns whether this field is marked `(deprecated)` in the `.fbs` source.
     *
     * Deprecated fields should not be written by new code, but their vtable slot
     * is preserved for wire compatibility with older readers.
     *
     * @returns `true` if the field is deprecated, `false` otherwise.
     */
    deprecated() {
        const entry = readVtableEntry(this.buf, this.tablePos, 16);
        if (entry === 0)
            return false;
        return this.buf.getUint8(this.tablePos + entry) !== 0;
    }
    /**
     * Returns whether this field is annotated with `(key)` in the `.fbs` source,
     * meaning it is used as the sort key for binary search in sorted vectors.
     *
     * @returns `true` if the field is a sort key, `false` otherwise.
     */
    key() {
        const entry = readVtableEntry(this.buf, this.tablePos, 20);
        if (entry === 0)
            return false;
        return this.buf.getUint8(this.tablePos + entry) !== 0;
    }
    /**
     * Returns whether this field is marked `optional` in the `.fbs` source
     * (FlatBuffers feature for nullable scalars).
     *
     * Optional scalar fields may be absent from a buffer even when the table is
     * otherwise complete.
     *
     * @returns `true` if the field is optional, `false` otherwise.
     */
    optional() {
        const entry = readVtableEntry(this.buf, this.tablePos, 26);
        if (entry === 0)
            return false;
        return this.buf.getUint8(this.tablePos + entry) !== 0;
    }
    /**
     * Returns the number of explicit padding bytes that `flatc` inserts after
     * this field within a struct.
     *
     * Only meaningful for struct fields ({@link ReflectionObject.isStruct} is
     * `true`); for table fields this is always `0`.
     *
     * @returns The padding byte count, or `0` if absent.
     */
    padding() {
        const entry = readVtableEntry(this.buf, this.tablePos, 28);
        if (entry === 0)
            return 0;
        return this.buf.getUint16(this.tablePos + entry, true);
    }
    /**
     * Returns whether this field uses 64-bit offsets (FlatBuffers64 extension
     * for very large buffers).
     *
     * When `true`, the data referenced by this field uses 8-byte offsets instead
     * of the standard 4-byte offsets.
     *
     * @returns `true` if the field uses 64-bit offsets, `false` otherwise.
     */
    offset64() {
        const entry = readVtableEntry(this.buf, this.tablePos, 30);
        if (entry === 0)
            return false;
        return this.buf.getUint8(this.tablePos + entry) !== 0;
    }
    /**
     * Returns all user-defined key/value metadata pairs attached to this field
     * (e.g. `(version: "2")` or `(important)` from the `.fbs` source).
     *
     * @returns An array of {@link ReflectionKeyValue} instances (empty if none).
     */
    attributes() {
        return readTableVector(this.buf, this.tablePos, 22).map(pos => new ReflectionKeyValue(this.buf, pos));
    }
    /**
     * Returns the doc-comment lines attached to this field, in the order they
     * appear in the `.fbs` source.  Each element is one line of documentation
     * text (without leading `///` markers).
     *
     * @returns An array of documentation strings (empty if none recorded).
     */
    documentation() {
        return readStringVector(this.buf, this.tablePos, 24);
    }
}
// ── ReflectionObject ───────────────────────────────────────────────────────
/**
 * Describes a FlatBuffers table or struct declared in the schema.
 * Wraps the `reflection.Object` table from `reflection.fbs`.
 *
 * Obtain instances via {@link ReflectionSchema.objects},
 * {@link ReflectionSchema.objectByName}, or {@link ReflectionSchema.rootTable}.
 *
 * @example
 * ```ts
 * const obj = schema.objectByName('Ln2.StatusMessage');
 * if (obj) {
 *   console.log(obj.isStruct()); // false — it's a table
 *   const field = obj.fieldByName('device_name');
 * }
 * ```
 */
export class ReflectionObject {
    /**
     * @param buf      - `DataView` over the raw `.bfbs` schema bytes.
     * @param tablePos - Absolute byte position of the `reflection.Object` table
     *                   within `buf`.
     */
    constructor(buf, tablePos) {
        this.buf = buf;
        this.tablePos = tablePos;
    }
    /**
     * Returns the fully qualified name of this table or struct.
     *
     * The name includes the namespace declared in the `.fbs` file, using `.`
     * as a separator (e.g. `"Ln2.StatusMessage"` or `"MyGame.Example.Monster"`).
     *
     * @returns The fully qualified name string, or an empty string if absent.
     */
    name() {
        const entry = readVtableEntry(this.buf, this.tablePos, 4);
        if (entry === 0)
            return '';
        return readString(this.buf, this.tablePos + entry);
    }
    /**
     * Returns all fields declared in this table or struct, in the sorted order
     * stored by `flatc` (alphabetical by field name).
     *
     * Note: the sort order matches the schema source, not the vtable slot order.
     * To look up a field by name, prefer {@link fieldByName}.
     *
     * @returns An array of {@link ReflectionField} instances (may be empty for
     *          objects with no fields).
     */
    fields() {
        const entry = readVtableEntry(this.buf, this.tablePos, 6);
        if (entry === 0)
            return [];
        const [count, dataStart] = readVector(this.buf, this.tablePos + entry);
        const result = [];
        for (let i = 0; i < count; i += 1) {
            const fieldTablePos = readVectorTableElement(this.buf, dataStart, i);
            result.push(new ReflectionField(this.buf, fieldTablePos));
        }
        return result;
    }
    /**
     * Finds and returns a field by its declared name.
     *
     * This performs a linear scan over {@link fields} and compares each
     * field's name string.  For hot paths, cache the returned
     * `ReflectionField` rather than calling this repeatedly.
     *
     * @param name - The field name to search for (case-sensitive, as declared
     *               in the `.fbs` source).
     * @returns The matching {@link ReflectionField}, or `undefined` if no field
     *          with that name exists in this object.
     *
     * @example
     * ```ts
     * const field = obj.fieldByName('device_name');
     * if (field) {
     *   const value = getFieldString(dataBuf, tablePos, field);
     * }
     * ```
     */
    fieldByName(name) {
        return this.fields().find(f => f.name() === name);
    }
    /**
     * Returns whether this schema object is a fixed-size struct rather than a
     * variable-size table.
     *
     * Structs are laid out inline (no vtable, no indirection) and have a
     * deterministic byte size returned by {@link byteSize}.  Tables use a
     * vtable and support optional fields.
     *
     * @returns `true` for structs, `false` for tables.
     */
    isStruct() {
        const entry = readVtableEntry(this.buf, this.tablePos, 8);
        if (entry === 0)
            return false;
        return this.buf.getUint8(this.tablePos + entry) !== 0;
    }
    /**
     * Returns the fixed byte size of this object when it is a struct.
     *
     * Only meaningful when {@link isStruct} returns `true`.  For tables this
     * value is `0` (tables are not fixed-size).
     *
     * @returns The struct's byte size, or `0` for tables or absent entries.
     */
    byteSize() {
        const entry = readVtableEntry(this.buf, this.tablePos, 12);
        if (entry === 0)
            return 0;
        return this.buf.getInt32(this.tablePos + entry, true);
    }
    /**
     * Returns the minimum alignment (in bytes) required for this object when
     * embedded in a buffer.  For structs this is the maximum alignment of all
     * fields; for tables it may be `0` (alignment is not tracked for tables).
     *
     * @returns The minimum alignment, or `0` if absent.
     */
    minAlign() {
        const entry = readVtableEntry(this.buf, this.tablePos, 10);
        if (entry === 0)
            return 0;
        return this.buf.getInt32(this.tablePos + entry, true);
    }
    /**
     * Returns all user-defined key/value metadata pairs attached to this table
     * or struct (e.g. `(my_attr: "value")` annotations from the `.fbs` source).
     *
     * @returns An array of {@link ReflectionKeyValue} instances (empty if none).
     */
    attributes() {
        return readTableVector(this.buf, this.tablePos, 14).map(pos => new ReflectionKeyValue(this.buf, pos));
    }
    /**
     * Returns the doc-comment lines attached to this table or struct, in the
     * order they appear in the `.fbs` source.  Each element is one line of
     * documentation text (without leading `///` markers).
     *
     * @returns An array of documentation strings (empty if none recorded).
     */
    documentation() {
        return readStringVector(this.buf, this.tablePos, 16);
    }
    /**
     * Returns the relative path of the `.fbs` source file in which this table
     * or struct is declared.  The path is relative to the root directory passed
     * to `flatc` at compile time.
     *
     * @returns The declaration file path, or an empty string if absent.
     */
    declarationFile() {
        const entry = readVtableEntry(this.buf, this.tablePos, 18);
        if (entry === 0)
            return '';
        return readString(this.buf, this.tablePos + entry);
    }
}
// ── helper: read string vector / KeyValue vector ───────────────────────────
/**
 * Read a vector of FlatBuffers strings from the given field-level position.
 * Each element is a UOffset pointing to a length-prefixed string.
 */
function readStringVector(buf, tablePos, voffset) {
    const entry = readVtableEntry(buf, tablePos, voffset);
    if (entry === 0)
        return [];
    const [count, dataStart] = readVector(buf, tablePos + entry);
    const result = [];
    for (let i = 0; i < count; i += 1) {
        const elemPos = dataStart + i * 4;
        result.push(readString(buf, elemPos));
    }
    return result;
}
/**
 * Read a vector of table offsets and return absolute table positions for each
 * element.  Used for [KeyValue] vectors (and any other table vector).
 */
function readTableVector(buf, tablePos, voffset) {
    const entry = readVtableEntry(buf, tablePos, voffset);
    if (entry === 0)
        return [];
    const [count, dataStart] = readVector(buf, tablePos + entry);
    const positions = [];
    for (let i = 0; i < count; i += 1) {
        positions.push(readVectorTableElement(buf, dataStart, i));
    }
    return positions;
}
// ── ReflectionKeyValue ─────────────────────────────────────────────────────
/**
 * Represents a single user-defined attribute (key/value metadata pair)
 * attached to a schema element.  Wraps the `reflection.KeyValue` table from
 * `reflection.fbs`.
 *
 * In a `.fbs` source file, attributes look like:
 * ```fbs
 * table MyTable (my_attr: "my_value") { ... }
 * field: int (important, version: "2");
 * ```
 *
 * Obtain instances via {@link ReflectionObject.attributes},
 * {@link ReflectionField.attributes}, {@link ReflectionEnum.attributes}, or
 * {@link ReflectionEnumVal.attributes}.
 *
 * @example
 * ```ts
 * for (const attr of field.attributes()) {
 *   console.log(attr.key(), '=', attr.value()); // e.g. "version" = "2"
 * }
 * ```
 */
export class ReflectionKeyValue {
    /**
     * @param buf      - `DataView` over the raw `.bfbs` schema bytes.
     * @param tablePos - Absolute byte position of the `reflection.KeyValue`
     *                   table within `buf`.
     */
    constructor(buf, tablePos) {
        this.buf = buf;
        this.tablePos = tablePos;
    }
    /**
     * Returns the attribute key string (e.g. `"version"`, `"important"`).
     *
     * For boolean-style attributes (written without a value in the source), this
     * is the attribute name and {@link value} returns an empty string.
     *
     * @returns The key string, or an empty string if absent (malformed schema).
     */
    key() {
        const entry = readVtableEntry(this.buf, this.tablePos, 4);
        if (entry === 0)
            return '';
        return readString(this.buf, this.tablePos + entry);
    }
    /**
     * Returns the attribute value string (e.g. `"2"` for `(version: "2")`).
     *
     * For boolean-style attributes that have no explicit value (e.g. `(key)` or
     * `(required)`), this returns an empty string.
     *
     * @returns The value string, or an empty string if absent.
     */
    value() {
        const entry = readVtableEntry(this.buf, this.tablePos, 6);
        if (entry === 0)
            return '';
        return readString(this.buf, this.tablePos + entry);
    }
}
// ── ReflectionEnumVal ──────────────────────────────────────────────────────
/**
 * Describes a single named constant within an enum, or a single named variant
 * within a union.  Wraps the `reflection.EnumVal` table from `reflection.fbs`.
 *
 * Obtain instances via {@link ReflectionEnum.values} or
 * {@link ReflectionEnum.valueByName}.
 *
 * For **enum values**, {@link value} returns the declared integer discriminant
 * and {@link unionType} returns `null`.
 *
 * For **union variants**, {@link unionType} returns a {@link ReflectionType}
 * describing the table type for that variant, while {@link value} still
 * provides the implicit UType discriminant byte used on the wire.
 *
 * @example
 * ```ts
 * const enumDef = schema.enumByName('Ln2.FaultCode');
 * for (const v of enumDef?.values() ?? []) {
 *   console.log(v.name(), '=', v.value()); // e.g. "OverTemperature" = 3n
 * }
 * ```
 */
export class ReflectionEnumVal {
    /**
     * @param buf      - `DataView` over the raw `.bfbs` schema bytes.
     * @param tablePos - Absolute byte position of the `reflection.EnumVal`
     *                   table within `buf`.
     */
    constructor(buf, tablePos) {
        this.buf = buf;
        this.tablePos = tablePos;
    }
    /**
     * Returns the declared name of this enum constant or union variant
     * (e.g. `"OverTemperature"` or `"FaultMessage"`).
     *
     * @returns The name string, or an empty string if absent (malformed schema).
     */
    name() {
        const entry = readVtableEntry(this.buf, this.tablePos, 4);
        if (entry === 0)
            return '';
        return readString(this.buf, this.tablePos + entry);
    }
    /**
     * Returns the integer discriminant for this enum constant or union variant.
     *
     * For regular enums this is the value declared in the `.fbs` source
     * (e.g. `0n`, `1n`, `2n`, …).  For union variants it is the implicit
     * UType byte that `flatc` writes on the wire to select this variant.
     *
     * The value is stored as a 64-bit signed integer in the schema and is
     * returned as a `bigint` to preserve precision for large values.
     *
     * @returns The int64 discriminant, or `0n` if absent.
     */
    value() {
        const entry = readVtableEntry(this.buf, this.tablePos, 6);
        if (entry === 0)
            return 0n;
        const fieldPos = this.tablePos + entry;
        const lo = BigInt(this.buf.getUint32(fieldPos, true));
        const hi = BigInt(this.buf.getInt32(fieldPos + 4, true));
        return BigInt.asIntN(64, lo + (hi * 0x100000000n));
    }
    /**
     * Returns the {@link ReflectionType} for a union variant.
     *
     * Only meaningful when the parent {@link ReflectionEnum.isUnion} returns
     * `true`.  Describes the table type associated with this specific union
     * variant (e.g. `Ln2.FaultMessage` for the `FaultMessage` variant).
     *
     * @returns A `ReflectionType` instance, or `null` for regular enum values
     *          or when the `union_type` sub-table is absent.
     */
    unionType() {
        const entry = readVtableEntry(this.buf, this.tablePos, 10);
        if (entry === 0)
            return null;
        const typeTablePos = readTable(this.buf, this.tablePos + entry);
        return new ReflectionType(this.buf, typeTablePos);
    }
    /**
     * Returns the doc-comment lines attached to this enum value or union variant,
     * in the order they appear in the `.fbs` source.  Each element is one line
     * of documentation text (without leading `///` markers).
     *
     * @returns An array of documentation strings (empty if none recorded).
     */
    documentation() {
        return readStringVector(this.buf, this.tablePos, 12);
    }
    /**
     * Returns all user-defined key/value metadata pairs attached to this enum
     * value or union variant (e.g. custom tooling annotations).
     *
     * @returns An array of {@link ReflectionKeyValue} instances (empty if none).
     */
    attributes() {
        return readTableVector(this.buf, this.tablePos, 14).map(pos => new ReflectionKeyValue(this.buf, pos));
    }
}
// ── ReflectionEnum ─────────────────────────────────────────────────────────
/**
 * Describes a FlatBuffers enum or union declared in the schema.
 * Wraps the `reflection.Enum` table from `reflection.fbs`.
 *
 * Both regular integer-backed enums and union type descriptors are stored in
 * the schema's `enums` vector.  Call {@link isUnion} to distinguish them.
 *
 * Obtain instances via {@link ReflectionSchema.enums} or
 * {@link ReflectionSchema.enumByName}.
 *
 * @example
 * ```ts
 * // Look up an enum and print all its values.
 * const faultCode = schema.enumByName('Ln2.FaultCode');
 * if (faultCode) {
 *   for (const v of faultCode.values()) {
 *     console.log(v.name(), '->', Number(v.value()));
 *   }
 * }
 *
 * // Resolve a union discriminant to a variant name.
 * const msgContent = schema.enumByName('Ln2.MessageContent');
 * const variant = msgContent?.values().find(v => v.value() === discriminant);
 * console.log('variant:', variant?.name());
 * ```
 */
export class ReflectionEnum {
    /**
     * @param buf      - `DataView` over the raw `.bfbs` schema bytes.
     * @param tablePos - Absolute byte position of the `reflection.Enum` table
     *                   within `buf`.
     */
    constructor(buf, tablePos) {
        this.buf = buf;
        this.tablePos = tablePos;
    }
    /**
     * Returns the fully qualified name of this enum or union
     * (e.g. `"Ln2.FaultCode"` or `"Ln2.MessageContent"`).
     *
     * @returns The fully qualified name string, or an empty string if absent.
     */
    name() {
        const entry = readVtableEntry(this.buf, this.tablePos, 4);
        if (entry === 0)
            return '';
        return readString(this.buf, this.tablePos + entry);
    }
    /**
     * Returns whether this schema entry is a union type descriptor rather than
     * a regular integer-backed enum.
     *
     * Union variants are accessed the same way as enum values via
     * {@link values}, but each variant's {@link ReflectionEnumVal.unionType}
     * will be non-null.
     *
     * @returns `true` for unions, `false` for regular enums.
     */
    isUnion() {
        const entry = readVtableEntry(this.buf, this.tablePos, 8);
        if (entry === 0)
            return false;
        return this.buf.getUint8(this.tablePos + entry) !== 0;
    }
    /**
     * Returns all declared enum constants or union variants in the order stored
     * in the schema.
     *
     * For regular enums each element is a named integer constant.  For unions
     * each element is a named variant; call
     * {@link ReflectionEnumVal.unionType} on each to obtain its table type.
     *
     * @returns An array of {@link ReflectionEnumVal} instances (empty if the
     *          enum has no values, which should not occur in practice).
     */
    values() {
        const entry = readVtableEntry(this.buf, this.tablePos, 6);
        if (entry === 0)
            return [];
        const [count, dataStart] = readVector(this.buf, this.tablePos + entry);
        const result = [];
        for (let i = 0; i < count; i += 1) {
            const valTablePos = readVectorTableElement(this.buf, dataStart, i);
            result.push(new ReflectionEnumVal(this.buf, valTablePos));
        }
        return result;
    }
    /**
     * Finds and returns a value or variant by its declared name.
     *
     * This performs a linear scan over {@link values}.  Cache the result for
     * repeated lookups on hot paths.
     *
     * @param name - The value/variant name to search for (case-sensitive).
     * @returns The matching {@link ReflectionEnumVal}, or `undefined` if not found.
     *
     * @example
     * ```ts
     * const val = schema.enumByName('Ln2.FaultCode')?.valueByName('OverTemperature');
     * console.log(val?.value()); // e.g. 3n
     * ```
     */
    valueByName(name) {
        return this.values().find(v => v.name() === name);
    }
    /**
     * Returns the {@link ReflectionType} that describes the underlying integer
     * type used to represent this enum on the wire.
     *
     * For most enums this will be a byte, short, or int base type.  For union
     * type descriptors the underlying type is always `UType` (a `uint8`
     * discriminant).
     *
     * @returns A `ReflectionType` instance, or `null` if the sub-table is
     *          absent in the schema.
     */
    underlyingType() {
        const entry = readVtableEntry(this.buf, this.tablePos, 10);
        if (entry === 0)
            return null;
        const typeTablePos = readTable(this.buf, this.tablePos + entry);
        return new ReflectionType(this.buf, typeTablePos);
    }
    /**
     * Returns all user-defined key/value metadata pairs attached to this enum
     * or union (e.g. custom tooling annotations from the `.fbs` source).
     *
     * @returns An array of {@link ReflectionKeyValue} instances (empty if none).
     */
    attributes() {
        return readTableVector(this.buf, this.tablePos, 12).map(pos => new ReflectionKeyValue(this.buf, pos));
    }
    /**
     * Returns the doc-comment lines attached to this enum or union, in the
     * order they appear in the `.fbs` source.  Each element is one line of
     * documentation text (without leading `///` markers).
     *
     * @returns An array of documentation strings (empty if none recorded).
     */
    documentation() {
        return readStringVector(this.buf, this.tablePos, 14);
    }
    /**
     * Returns the relative path of the `.fbs` source file in which this enum or
     * union is declared.  The path is relative to the root directory passed to
     * `flatc` at compile time.
     *
     * @returns The declaration file path, or an empty string if absent.
     */
    declarationFile() {
        const entry = readVtableEntry(this.buf, this.tablePos, 16);
        if (entry === 0)
            return '';
        return readString(this.buf, this.tablePos + entry);
    }
}
// ── ReflectionSchema ───────────────────────────────────────────────────────
/**
 * Parses and exposes the top-level `reflection.Schema` table from a binary
 * FlatBuffers schema file (`.bfbs`).
 *
 * A `.bfbs` file is produced by running `flatc --binary --schema` on a
 * `.fbs` source file.  Pass the resulting bytes to {@link loadSchema} (or
 * construct this class directly) to obtain a `ReflectionSchema`.
 *
 * The schema object is the entry point for all reflection operations: it lets
 * you enumerate every table/struct declared in the schema and look up fields
 * by name, enabling dynamic serialization and deserialization without
 * generated code.
 *
 * @example
 * ```ts
 * const bfbs = new Uint8Array(await (await fetch('/schemas/ln2.bfbs')).arrayBuffer());
 * const schema = new ReflectionSchema(bfbs);
 *
 * const obj = schema.objectByName('Ln2.StatusMessage');
 * const field = obj?.fieldByName('device_name');
 * ```
 */
export class ReflectionSchema {
    /**
     * Parses the binary schema from the provided byte array.
     *
     * The constructor reads the root UOffset at byte 0 of the buffer to locate
     * the `reflection.Schema` table, mirroring the standard FlatBuffers root
     * table lookup.
     *
     * @param bfbs - `Uint8Array` containing the raw bytes of a `.bfbs` binary
     *               schema file produced by `flatc --binary --schema`.
     */
    constructor(bfbs) {
        this.buf = new DataView(bfbs.buffer, bfbs.byteOffset, bfbs.byteLength);
        const uoffset = this.buf.getUint32(0, true);
        this.rootTablePos = uoffset;
    }
    /**
     * Returns the object that was declared as the `root_type` in the `.fbs`
     * source file.
     *
     * The root table is the top-level message type that data buffers conforming
     * to this schema are expected to start with.  Use this when you always know
     * the schema's root type and do not need to look it up by name.
     *
     * @returns The {@link ReflectionObject} for the schema's root type, or
     *          `null` if the schema does not declare a root type.
     */
    rootTable() {
        const entry = readVtableEntry(this.buf, this.rootTablePos, 12);
        if (entry === 0)
            return null;
        const objTablePos = readTable(this.buf, this.rootTablePos + entry);
        return new ReflectionObject(this.buf, objTablePos);
    }
    /**
     * Returns the four-character file identifier declared with `file_identifier`
     * in the `.fbs` source (e.g. `"BFBS"`).
     *
     * This identifier is embedded in the first eight bytes of every FlatBuffer
     * built with this schema.  Returns an empty string if no `file_identifier`
     * was declared.
     *
     * @returns The file identifier string, or an empty string if absent.
     */
    fileIdent() {
        const entry = readVtableEntry(this.buf, this.rootTablePos, 8);
        if (entry === 0)
            return '';
        return readString(this.buf, this.rootTablePos + entry);
    }
    /**
     * Returns the file extension declared with `file_extension` in the `.fbs`
     * source (e.g. `"bin"`).
     *
     * @returns The file extension string, or an empty string if absent.
     */
    fileExt() {
        const entry = readVtableEntry(this.buf, this.rootTablePos, 10);
        if (entry === 0)
            return '';
        return readString(this.buf, this.rootTablePos + entry);
    }
    /**
     * Returns the bit-flag set indicating which advanced FlatBuffers schema
     * features are used in this schema.  Each bit corresponds to a value in
     * the `AdvancedFeatures` enum from `reflection.fbs`:
     *
     * - bit 0 (`1n`): `AdvancedArrayFeatures`
     * - bit 1 (`2n`): `AdvancedUnionFeatures`
     * - bit 2 (`4n`): `OptionalScalars`
     * - bit 3 (`8n`): `DefaultVectorsAndStrings`
     *
     * Returns `0n` if the field is absent (schemas compiled without this feature
     * tracking, i.e. older `flatc` versions).
     *
     * @returns A `bigint` bit mask of the advanced features flags.
     */
    advancedFeatures() {
        const entry = readVtableEntry(this.buf, this.rootTablePos, 16);
        if (entry === 0)
            return 0n;
        const fieldPos = this.rootTablePos + entry;
        const lo = BigInt(this.buf.getUint32(fieldPos, true));
        const hi = BigInt(this.buf.getUint32(fieldPos + 4, true));
        return BigInt.asUintN(64, lo + (hi * 0x100000000n));
    }
    /**
     * Returns all table and struct objects declared in the schema, in the
     * sorted order stored by `flatc`.
     *
     * This includes every type across all namespaces in the `.fbs` source.
     * To find a specific type by its fully qualified name, prefer
     * {@link objectByName}.
     *
     * @returns An array of {@link ReflectionObject} instances (empty if the
     *          schema has no objects, which should not occur in practice).
     */
    objects() {
        const entry = readVtableEntry(this.buf, this.rootTablePos, 4);
        if (entry === 0)
            return [];
        const [count, dataStart] = readVector(this.buf, this.rootTablePos + entry);
        const result = [];
        for (let i = 0; i < count; i += 1) {
            const objTablePos = readVectorTableElement(this.buf, dataStart, i);
            result.push(new ReflectionObject(this.buf, objTablePos));
        }
        return result;
    }
    /**
     * Finds and returns an object by its fully qualified name.
     *
     * The name must include the namespace prefix as declared in the `.fbs`
     * source (e.g. `"Ln2.StatusMessage"`, not `"StatusMessage"`).  This
     * performs a linear scan; cache the result for repeated lookups.
     *
     * @param name - Fully qualified object name to search for.
     * @returns The matching {@link ReflectionObject}, or `undefined` if no
     *          object with that name exists in the schema.
     *
     * @example
     * ```ts
     * const obj = schema.objectByName('Ln2.ControllerInfo');
     * if (!obj) throw new Error('ControllerInfo not found in schema');
     * ```
     */
    objectByName(name) {
        return this.objects().find(o => o.name() === name);
    }
    /**
     * Returns all enum and union definitions declared in the schema, in the
     * sorted order stored by `flatc`.
     *
     * Both regular integer-backed enums and union type descriptors are included.
     * Use {@link ReflectionEnum.isUnion} to distinguish between the two.  To
     * find a specific enum by its fully qualified name, prefer
     * {@link enumByName}.
     *
     * @returns An array of {@link ReflectionEnum} instances (empty if the
     *          schema declares no enums or unions).
     */
    enums() {
        const entry = readVtableEntry(this.buf, this.rootTablePos, 6);
        if (entry === 0)
            return [];
        const [count, dataStart] = readVector(this.buf, this.rootTablePos + entry);
        const result = [];
        for (let i = 0; i < count; i += 1) {
            const enumTablePos = readVectorTableElement(this.buf, dataStart, i);
            result.push(new ReflectionEnum(this.buf, enumTablePos));
        }
        return result;
    }
    /**
     * Finds and returns an enum or union by its fully qualified name.
     *
     * The name must include the namespace prefix as declared in the `.fbs`
     * source (e.g. `"Ln2.FaultCode"`, not `"FaultCode"`).  This performs a
     * linear scan; cache the result for repeated lookups.
     *
     * @param name - Fully qualified enum or union name to search for.
     * @returns The matching {@link ReflectionEnum}, or `undefined` if not found.
     *
     * @example
     * ```ts
     * const faultCode = schema.enumByName('Ln2.FaultCode');
     * const label = faultCode?.values().find(v => v.value() === BigInt(code))?.name();
     * ```
     */
    enumByName(name) {
        return this.enums().find(e => e.name() === name);
    }
}
// ── vtable lookup for dynamic field access ─────────────────────────────────
/**
 * Perform a vtable lookup on dataBuf at tablePos using the schema field's voffset.
 * Returns the absolute data position, or null if the field is absent.
 */
function resolveFieldPos(dataBuf, tablePos, field) {
    const voffset = field.offset();
    if (voffset === 0)
        return null;
    const entry = readVtableEntry(dataBuf, tablePos, voffset);
    if (entry === 0)
        return null;
    return tablePos + entry;
}
// ── dynamic field accessors ────────────────────────────────────────────────
/**
 * Reads a `string` field from a live data buffer using reflection metadata.
 *
 * This function operates on the **data** buffer (a serialized FlatBuffers
 * message), not the schema buffer.  It uses the vtable offset stored in
 * `field` to locate the string within the table at `tablePos`.
 *
 * @param dataBuf  - `DataView` over the serialized FlatBuffers data (not the
 *                   `.bfbs` schema).
 * @param tablePos - Absolute byte offset of the root table in `dataBuf`.
 *                   Typically `dataBuf.getUint32(0, true)` for root buffers.
 * @param field    - The {@link ReflectionField} descriptor from the schema,
 *                   obtained via `obj.fieldByName('my_field')`.
 * @returns The decoded UTF-8 string value, or `null` if the field is absent
 *          from the buffer (vtable entry is 0).
 *
 * @example
 * ```ts
 * const nameField = obj.fieldByName('device_name')!;
 * const rootPos = dataBuf.getUint32(0, true);
 * const name = getFieldString(dataBuf, rootPos, nameField);
 * // name === "Elevator-3A" or null if not set
 * ```
 */
export function getFieldString(dataBuf, tablePos, field) {
    const fieldPos = resolveFieldPos(dataBuf, tablePos, field);
    if (fieldPos === null)
        return null;
    return readString(dataBuf, fieldPos);
}
/**
 * Reads an integer field from a live data buffer using reflection metadata.
 *
 * Dispatches to the correct `DataView` read method based on the field's
 * {@link ReflectionBaseType}.  Signed and unsigned 64-bit integers are
 * returned as `bigint` to preserve precision; all narrower integer types
 * (including `Bool`) are returned as `number`.
 *
 * @param dataBuf  - `DataView` over the serialized FlatBuffers data.
 * @param tablePos - Absolute byte offset of the root table in `dataBuf`.
 * @param field    - The {@link ReflectionField} descriptor from the schema.
 * @returns The integer value as `number` (8/16/32-bit types) or `bigint`
 *          (64-bit types), or `null` if the field is absent.
 *
 * @example
 * ```ts
 * const statusField = obj.fieldByName('status_code')!;
 * const code = getFieldInt(dataBuf, rootPos, statusField);
 * if (code !== null) {
 *   console.log(Number(code)); // safe for <= 32-bit fields
 * }
 * ```
 */
export function getFieldInt(dataBuf, tablePos, field) {
    const fieldPos = resolveFieldPos(dataBuf, tablePos, field);
    if (fieldPos === null)
        return null;
    const fieldType = field.type();
    const base = fieldType !== null ? fieldType.baseType() : ReflectionBaseType.Int;
    switch (base) {
        case ReflectionBaseType.Bool:
        case ReflectionBaseType.UByte:
            return dataBuf.getUint8(fieldPos);
        case ReflectionBaseType.Byte:
            return dataBuf.getInt8(fieldPos);
        case ReflectionBaseType.UShort:
            return dataBuf.getUint16(fieldPos, true);
        case ReflectionBaseType.Short:
            return dataBuf.getInt16(fieldPos, true);
        case ReflectionBaseType.UInt:
            return dataBuf.getUint32(fieldPos, true);
        case ReflectionBaseType.Int:
            return dataBuf.getInt32(fieldPos, true);
        case ReflectionBaseType.ULong: {
            const lo = BigInt(dataBuf.getUint32(fieldPos, true));
            const hi = BigInt(dataBuf.getUint32(fieldPos + 4, true));
            return BigInt.asUintN(64, lo + (hi * 0x100000000n));
        }
        case ReflectionBaseType.Long: {
            const lo = BigInt(dataBuf.getUint32(fieldPos, true));
            const hi = BigInt(dataBuf.getInt32(fieldPos + 4, true));
            return BigInt.asIntN(64, lo + (hi * 0x100000000n));
        }
        default:
            return dataBuf.getInt32(fieldPos, true);
    }
}
/**
 * Reads a `float` or `double` field from a live data buffer using reflection
 * metadata.
 *
 * Checks the field's base type to select between `getFloat32` (`Float`) and
 * `getFloat64` (`Double`).  Defaults to `Float` when the type sub-table is
 * absent.
 *
 * @param dataBuf  - `DataView` over the serialized FlatBuffers data.
 * @param tablePos - Absolute byte offset of the root table in `dataBuf`.
 * @param field    - The {@link ReflectionField} descriptor from the schema.
 * @returns The floating-point value as a JavaScript `number`, or `null` if
 *          the field is absent.
 *
 * @example
 * ```ts
 * const tempField = obj.fieldByName('temperature')!;
 * const temp = getFieldFloat(dataBuf, rootPos, tempField);
 * if (temp !== null) console.log(`${temp.toFixed(2)} °C`);
 * ```
 */
export function getFieldFloat(dataBuf, tablePos, field) {
    const fieldPos = resolveFieldPos(dataBuf, tablePos, field);
    if (fieldPos === null)
        return null;
    const fieldType = field.type();
    const base = fieldType !== null ? fieldType.baseType() : ReflectionBaseType.Float;
    if (base === ReflectionBaseType.Double) {
        return dataBuf.getFloat64(fieldPos, true);
    }
    return dataBuf.getFloat32(fieldPos, true);
}
/**
 * Reads a `bool` field from a live data buffer using reflection metadata.
 *
 * FlatBuffers stores booleans as a single byte (`0` = false, non-zero = true).
 *
 * @param dataBuf  - `DataView` over the serialized FlatBuffers data.
 * @param tablePos - Absolute byte offset of the root table in `dataBuf`.
 * @param field    - The {@link ReflectionField} descriptor from the schema.
 * @returns `true` or `false`, or `null` if the field is absent from the
 *          buffer.
 *
 * @example
 * ```ts
 * const activeField = obj.fieldByName('is_active')!;
 * const active = getFieldBool(dataBuf, rootPos, activeField);
 * if (active !== null) console.log('Active:', active);
 * ```
 */
export function getFieldBool(dataBuf, tablePos, field) {
    const fieldPos = resolveFieldPos(dataBuf, tablePos, field);
    if (fieldPos === null)
        return null;
    return dataBuf.getUint8(fieldPos) !== 0;
}
// ── convenience loader ─────────────────────────────────────────────────────
/**
 * Convenience constructor for {@link ReflectionSchema}.
 *
 * Equivalent to `new ReflectionSchema(bfbs)` but reads more naturally in
 * call-site code and avoids exposing the class constructor in simple
 * import patterns.
 *
 * @param bfbs - `Uint8Array` containing the raw bytes of a `.bfbs` binary
 *               schema file produced by `flatc --binary --schema`.
 * @returns A fully initialized {@link ReflectionSchema} ready for object and
 *          field lookups.
 *
 * @example
 * ```ts
 * import { loadSchema, getFieldString } from './reflection';
 *
 * const bfbs = new Uint8Array(await (await fetch('/schemas/ln2.bfbs')).arrayBuffer());
 * const schema = loadSchema(bfbs);
 *
 * const obj = schema.objectByName('Ln2.StatusMessage');
 * const field = obj?.fieldByName('device_name');
 * if (obj && field) {
 *   const dataBuf = new DataView(someArrayBuffer);
 *   const rootPos = dataBuf.getUint32(0, true);
 *   const name = getFieldString(dataBuf, rootPos, field);
 *   console.log(name); // e.g. "Elevator-3A"
 * }
 * ```
 */
export function loadSchema(bfbs) {
    return new ReflectionSchema(bfbs);
}
