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
export declare enum ReflectionBaseType {
    /** No type / unset. */
    None = 0,
    /** Union type discriminant (u8). */
    UType = 1,
    /** Boolean stored as a single byte. */
    Bool = 2,
    /** Signed 8-bit integer. */
    Byte = 3,
    /** Unsigned 8-bit integer. */
    UByte = 4,
    /** Signed 16-bit integer. */
    Short = 5,
    /** Unsigned 16-bit integer. */
    UShort = 6,
    /** Signed 32-bit integer. */
    Int = 7,
    /** Unsigned 32-bit integer. */
    UInt = 8,
    /** Signed 64-bit integer. */
    Long = 9,
    /** Unsigned 64-bit integer. */
    ULong = 10,
    /** 32-bit IEEE 754 floating-point. */
    Float = 11,
    /** 64-bit IEEE 754 floating-point. */
    Double = 12,
    /** FlatBuffers inline string (UTF-8, length-prefixed). */
    String = 13,
    /** Variable-length vector of scalars or tables. */
    Vector = 14,
    /** Nested table or struct object. `index()` gives the schema object index. */
    Obj = 15,
    /** Tagged union. `index()` gives the schema enum index. */
    Union = 16,
    /** Fixed-length array of scalars (only valid inside structs). */
    Array = 17,
    /** 64-bit-aligned vector (FlatBuffers64 extension). */
    Vector64 = 18
}
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
export declare class ReflectionType {
    private readonly buf;
    private readonly tablePos;
    /**
     * @param buf      - `DataView` over the raw `.bfbs` schema bytes.
     * @param tablePos - Absolute byte position of the `reflection.Type` table
     *                   within `buf`.
     */
    constructor(buf: DataView, tablePos: number);
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
    baseType(): ReflectionBaseType;
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
    element(): ReflectionBaseType;
    /**
     * Returns the schema-level index for `Obj`, `Union`, and enum-backed types.
     *
     * - For `Obj` base type: index into {@link ReflectionSchema.objects}.
     * - For `Union` / enum base type: index into the schema's `enums` vector.
     * - For all other base types: `-1` (not applicable).
     *
     * @returns A non-negative integer index, or `-1` when not applicable.
     */
    index(): number;
    /**
     * Returns the declared element count for fixed-length `Array` fields.
     *
     * Only meaningful when {@link baseType} is `ReflectionBaseType.Array`.  For
     * example, a field declared as `[float:8]` has `baseType` == `Array`,
     * `element` == `Float`, and `fixedLength` == `8`.
     *
     * @returns The element count, or `0` for non-array types or absent entries.
     */
    fixedLength(): number;
    /**
     * Returns the byte size of the value described by {@link baseType}.
     *
     * For example, an `Int` field has `baseSize` == `4`, a `Double` field has
     * `baseSize` == `8`.  Defaults to `4` when absent (the most common case:
     * 4-byte offsets for tables and strings).
     *
     * @returns The base size in bytes, or `4` if absent.
     */
    baseSize(): number;
    /**
     * Returns the byte size of each element in a `Vector` or `Array` field.
     *
     * For scalar element types this matches the natural width of the scalar; for
     * table element types it is `4` (the size of a `UOffset` reference).
     * Returns `0` if absent or not applicable.
     *
     * @returns The element size in bytes, or `0` if absent.
     */
    elementSize(): number;
}
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
export declare class ReflectionField {
    private readonly buf;
    private readonly tablePos;
    /**
     * @param buf      - `DataView` over the raw `.bfbs` schema bytes.
     * @param tablePos - Absolute byte position of the `reflection.Field` table
     *                   within `buf`.
     */
    constructor(buf: DataView, tablePos: number);
    /**
     * Returns the field name as declared in the `.fbs` source file.
     *
     * @returns The field name string, or an empty string if the vtable entry is
     *          absent (malformed schema).
     */
    name(): string;
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
    type(): ReflectionType | null;
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
    offset(): number;
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
    defaultInteger(): bigint;
    /**
     * Returns the default value for floating-point fields as declared in the
     * schema.
     *
     * Analogous to {@link defaultInteger}: callers that require default-value
     * semantics should use this when {@link getFieldFloat} returns `null`.
     *
     * @returns The declared `float64` default, or `0` if no default is stored.
     */
    defaultReal(): number;
    /**
     * Returns whether this field is marked `required` in the schema.
     *
     * A `required` field must always be present in a valid buffer.  Validation
     * tools can use this to flag non-conformant data without needing to know
     * the field's type.
     *
     * @returns `true` if the field is required, `false` otherwise.
     */
    required(): boolean;
    /**
     * Returns the field identifier (`id`) assigned by `flatc`.  For table
     * fields this is the sequential index used to generate vtable slots; for
     * struct fields it is the zero-based declaration order.
     *
     * @returns The field ID, or `0` if absent.
     */
    id(): number;
    /**
     * Returns whether this field is marked `(deprecated)` in the `.fbs` source.
     *
     * Deprecated fields should not be written by new code, but their vtable slot
     * is preserved for wire compatibility with older readers.
     *
     * @returns `true` if the field is deprecated, `false` otherwise.
     */
    deprecated(): boolean;
    /**
     * Returns whether this field is annotated with `(key)` in the `.fbs` source,
     * meaning it is used as the sort key for binary search in sorted vectors.
     *
     * @returns `true` if the field is a sort key, `false` otherwise.
     */
    key(): boolean;
    /**
     * Returns whether this field is marked `optional` in the `.fbs` source
     * (FlatBuffers feature for nullable scalars).
     *
     * Optional scalar fields may be absent from a buffer even when the table is
     * otherwise complete.
     *
     * @returns `true` if the field is optional, `false` otherwise.
     */
    optional(): boolean;
    /**
     * Returns the number of explicit padding bytes that `flatc` inserts after
     * this field within a struct.
     *
     * Only meaningful for struct fields ({@link ReflectionObject.isStruct} is
     * `true`); for table fields this is always `0`.
     *
     * @returns The padding byte count, or `0` if absent.
     */
    padding(): number;
    /**
     * Returns whether this field uses 64-bit offsets (FlatBuffers64 extension
     * for very large buffers).
     *
     * When `true`, the data referenced by this field uses 8-byte offsets instead
     * of the standard 4-byte offsets.
     *
     * @returns `true` if the field uses 64-bit offsets, `false` otherwise.
     */
    offset64(): boolean;
    /**
     * Returns all user-defined key/value metadata pairs attached to this field
     * (e.g. `(version: "2")` or `(important)` from the `.fbs` source).
     *
     * @returns An array of {@link ReflectionKeyValue} instances (empty if none).
     */
    attributes(): ReflectionKeyValue[];
    /**
     * Returns the doc-comment lines attached to this field, in the order they
     * appear in the `.fbs` source.  Each element is one line of documentation
     * text (without leading `///` markers).
     *
     * @returns An array of documentation strings (empty if none recorded).
     */
    documentation(): string[];
}
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
export declare class ReflectionObject {
    private readonly buf;
    private readonly tablePos;
    /**
     * @param buf      - `DataView` over the raw `.bfbs` schema bytes.
     * @param tablePos - Absolute byte position of the `reflection.Object` table
     *                   within `buf`.
     */
    constructor(buf: DataView, tablePos: number);
    /**
     * Returns the fully qualified name of this table or struct.
     *
     * The name includes the namespace declared in the `.fbs` file, using `.`
     * as a separator (e.g. `"Ln2.StatusMessage"` or `"MyGame.Example.Monster"`).
     *
     * @returns The fully qualified name string, or an empty string if absent.
     */
    name(): string;
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
    fields(): ReflectionField[];
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
    fieldByName(name: string): ReflectionField | undefined;
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
    isStruct(): boolean;
    /**
     * Returns the fixed byte size of this object when it is a struct.
     *
     * Only meaningful when {@link isStruct} returns `true`.  For tables this
     * value is `0` (tables are not fixed-size).
     *
     * @returns The struct's byte size, or `0` for tables or absent entries.
     */
    byteSize(): number;
    /**
     * Returns the minimum alignment (in bytes) required for this object when
     * embedded in a buffer.  For structs this is the maximum alignment of all
     * fields; for tables it may be `0` (alignment is not tracked for tables).
     *
     * @returns The minimum alignment, or `0` if absent.
     */
    minAlign(): number;
    /**
     * Returns all user-defined key/value metadata pairs attached to this table
     * or struct (e.g. `(my_attr: "value")` annotations from the `.fbs` source).
     *
     * @returns An array of {@link ReflectionKeyValue} instances (empty if none).
     */
    attributes(): ReflectionKeyValue[];
    /**
     * Returns the doc-comment lines attached to this table or struct, in the
     * order they appear in the `.fbs` source.  Each element is one line of
     * documentation text (without leading `///` markers).
     *
     * @returns An array of documentation strings (empty if none recorded).
     */
    documentation(): string[];
    /**
     * Returns the relative path of the `.fbs` source file in which this table
     * or struct is declared.  The path is relative to the root directory passed
     * to `flatc` at compile time.
     *
     * @returns The declaration file path, or an empty string if absent.
     */
    declarationFile(): string;
}
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
export declare class ReflectionKeyValue {
    private readonly buf;
    private readonly tablePos;
    /**
     * @param buf      - `DataView` over the raw `.bfbs` schema bytes.
     * @param tablePos - Absolute byte position of the `reflection.KeyValue`
     *                   table within `buf`.
     */
    constructor(buf: DataView, tablePos: number);
    /**
     * Returns the attribute key string (e.g. `"version"`, `"important"`).
     *
     * For boolean-style attributes (written without a value in the source), this
     * is the attribute name and {@link value} returns an empty string.
     *
     * @returns The key string, or an empty string if absent (malformed schema).
     */
    key(): string;
    /**
     * Returns the attribute value string (e.g. `"2"` for `(version: "2")`).
     *
     * For boolean-style attributes that have no explicit value (e.g. `(key)` or
     * `(required)`), this returns an empty string.
     *
     * @returns The value string, or an empty string if absent.
     */
    value(): string;
}
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
export declare class ReflectionEnumVal {
    private readonly buf;
    private readonly tablePos;
    /**
     * @param buf      - `DataView` over the raw `.bfbs` schema bytes.
     * @param tablePos - Absolute byte position of the `reflection.EnumVal`
     *                   table within `buf`.
     */
    constructor(buf: DataView, tablePos: number);
    /**
     * Returns the declared name of this enum constant or union variant
     * (e.g. `"OverTemperature"` or `"FaultMessage"`).
     *
     * @returns The name string, or an empty string if absent (malformed schema).
     */
    name(): string;
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
    value(): bigint;
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
    unionType(): ReflectionType | null;
    /**
     * Returns the doc-comment lines attached to this enum value or union variant,
     * in the order they appear in the `.fbs` source.  Each element is one line
     * of documentation text (without leading `///` markers).
     *
     * @returns An array of documentation strings (empty if none recorded).
     */
    documentation(): string[];
    /**
     * Returns all user-defined key/value metadata pairs attached to this enum
     * value or union variant (e.g. custom tooling annotations).
     *
     * @returns An array of {@link ReflectionKeyValue} instances (empty if none).
     */
    attributes(): ReflectionKeyValue[];
}
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
export declare class ReflectionEnum {
    private readonly buf;
    private readonly tablePos;
    /**
     * @param buf      - `DataView` over the raw `.bfbs` schema bytes.
     * @param tablePos - Absolute byte position of the `reflection.Enum` table
     *                   within `buf`.
     */
    constructor(buf: DataView, tablePos: number);
    /**
     * Returns the fully qualified name of this enum or union
     * (e.g. `"Ln2.FaultCode"` or `"Ln2.MessageContent"`).
     *
     * @returns The fully qualified name string, or an empty string if absent.
     */
    name(): string;
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
    isUnion(): boolean;
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
    values(): ReflectionEnumVal[];
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
    valueByName(name: string): ReflectionEnumVal | undefined;
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
    underlyingType(): ReflectionType | null;
    /**
     * Returns all user-defined key/value metadata pairs attached to this enum
     * or union (e.g. custom tooling annotations from the `.fbs` source).
     *
     * @returns An array of {@link ReflectionKeyValue} instances (empty if none).
     */
    attributes(): ReflectionKeyValue[];
    /**
     * Returns the doc-comment lines attached to this enum or union, in the
     * order they appear in the `.fbs` source.  Each element is one line of
     * documentation text (without leading `///` markers).
     *
     * @returns An array of documentation strings (empty if none recorded).
     */
    documentation(): string[];
    /**
     * Returns the relative path of the `.fbs` source file in which this enum or
     * union is declared.  The path is relative to the root directory passed to
     * `flatc` at compile time.
     *
     * @returns The declaration file path, or an empty string if absent.
     */
    declarationFile(): string;
}
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
export declare class ReflectionSchema {
    private readonly buf;
    private readonly rootTablePos;
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
    constructor(bfbs: Uint8Array);
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
    rootTable(): ReflectionObject | null;
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
    fileIdent(): string;
    /**
     * Returns the file extension declared with `file_extension` in the `.fbs`
     * source (e.g. `"bin"`).
     *
     * @returns The file extension string, or an empty string if absent.
     */
    fileExt(): string;
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
    advancedFeatures(): bigint;
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
    objects(): ReflectionObject[];
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
    objectByName(name: string): ReflectionObject | undefined;
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
    enums(): ReflectionEnum[];
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
    enumByName(name: string): ReflectionEnum | undefined;
}
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
export declare function getFieldString(dataBuf: DataView, tablePos: number, field: ReflectionField): string | null;
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
export declare function getFieldInt(dataBuf: DataView, tablePos: number, field: ReflectionField): number | bigint | null;
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
export declare function getFieldFloat(dataBuf: DataView, tablePos: number, field: ReflectionField): number | null;
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
export declare function getFieldBool(dataBuf: DataView, tablePos: number, field: ReflectionField): boolean | null;
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
export declare function loadSchema(bfbs: Uint8Array): ReflectionSchema;
