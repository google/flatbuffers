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
import { ReflectionBaseType, } from './reflection';
// ── FlatBuffers builder ─────────────────────────────────────────────────────
// Minimal self-contained builder.  The full runtime builder is a class with
// many concerns; here we only need what Pack requires.
const INITIAL_BUFFER_SIZE = 1024;
const VTABLE_BASE_OFFSET = 4;
const BYTES_PER_VTABLE_SLOT = 2;
/** Convert a vtable byte offset to a Builder slot index. */
function vtableSlot(voffset) {
    if (voffset < VTABLE_BASE_OFFSET)
        return 0;
    return (voffset - VTABLE_BASE_OFFSET) / BYTES_PER_VTABLE_SLOT;
}
// ── Primitive DataView helpers ──────────────────────────────────────────────
function readUint32(buf, pos) {
    return (buf[pos] |
        (buf[pos + 1] << 8) |
        (buf[pos + 2] << 16) |
        ((buf[pos + 3] << 24) >>> 0));
}
function readInt16(buf, pos) {
    const v = buf[pos] | (buf[pos + 1] << 8);
    return v >= 0x8000 ? v - 0x10000 : v;
}
function readUint16(buf, pos) {
    return buf[pos] | (buf[pos + 1] << 8);
}
function readInt32(buf, pos) {
    return ((buf[pos] | (buf[pos + 1] << 8) | (buf[pos + 2] << 16) | (buf[pos + 3] << 24)) | 0);
}
function readFloat32(buf, pos) {
    const dv = new DataView(buf.buffer, buf.byteOffset + pos, 4);
    return dv.getFloat32(0, true);
}
function readFloat64(buf, pos) {
    const dv = new DataView(buf.buffer, buf.byteOffset + pos, 8);
    return dv.getFloat64(0, true);
}
function readInt64(buf, pos) {
    const lo = BigInt(readUint32(buf, pos));
    const hi = BigInt(readInt32(buf, pos + 4));
    return BigInt.asIntN(64, lo + (hi << 32n));
}
function readUint64(buf, pos) {
    const lo = BigInt(readUint32(buf, pos));
    const hi = BigInt(readUint32(buf, pos + 4));
    return BigInt.asUintN(64, lo + (hi << 32n));
}
// ── Schema vtable helpers ───────────────────────────────────────────────────
function readVtableEntry(buf, tablePos, voffset) {
    const soffset = readInt32(buf, tablePos);
    const vtablePos = tablePos - soffset;
    const vtableSize = readUint16(buf, vtablePos);
    if (voffset >= vtableSize)
        return 0;
    return readUint16(buf, vtablePos + voffset);
}
function resolveVtableField(buf, tablePos, voffset) {
    if (voffset === 0)
        return 0;
    const entry = readVtableEntry(buf, tablePos, voffset);
    if (entry === 0)
        return 0;
    return tablePos + entry;
}
// ── String reading ──────────────────────────────────────────────────────────
function readFbString(buf, fieldPos) {
    const uoffset = readUint32(buf, fieldPos);
    const strStart = fieldPos + uoffset;
    const length = readUint32(buf, strStart);
    return new TextDecoder().decode(buf.subarray(strStart + 4, strStart + 4 + length));
}
// ── Enum/union reflection ───────────────────────────────────────────────────
// Vtable offsets for reflection.Enum and reflection.EnumVal tables in .bfbs.
const ENUM_VOFF_NAME = 4;
const ENUM_VOFF_VALUES = 6;
const ENUMVAL_VOFF_NAME = 4;
const ENUMVAL_VOFF_VALUE = 6;
/** Returns the name of the schema's enum at index enumIdx, or '' if out of range. */
function enumName(schema, enumIdx) {
    if (enumIdx < 0)
        return '';
    const rawBuf = rawSchemaBuffer(schema);
    if (rawBuf === null)
        return '';
    const enumsVec = enumsVector(rawBuf, schema);
    if (enumIdx >= enumsVec.count)
        return '';
    const enumPos = vectorTableAt(rawBuf, enumsVec.dataStart, enumIdx);
    const nameEntry = readVtableEntry(rawBuf, enumPos, ENUM_VOFF_NAME);
    if (nameEntry === 0)
        return '';
    return readFbString(rawBuf, enumPos + nameEntry);
}
/** Returns the integer discriminant for variantName within the schema enum at enumIdx. */
function resolveUnionDiscriminant(schema, enumIdx, variantName) {
    if (enumIdx < 0)
        return 0;
    const rawBuf = rawSchemaBuffer(schema);
    if (rawBuf === null)
        return 0;
    const enumsVec = enumsVector(rawBuf, schema);
    if (enumIdx >= enumsVec.count)
        return 0;
    const enumPos = vectorTableAt(rawBuf, enumsVec.dataStart, enumIdx);
    const valsEntry = readVtableEntry(rawBuf, enumPos, ENUM_VOFF_VALUES);
    if (valsEntry === 0)
        return 0;
    const valsFieldPos = enumPos + valsEntry;
    const valsUoffset = readUint32(rawBuf, valsFieldPos);
    const valsStart = valsFieldPos + valsUoffset;
    const valsCount = readUint32(rawBuf, valsStart);
    const valsDataStart = valsStart + 4;
    for (let i = 0; i < valsCount; i++) {
        const evPos = vectorTableAt(rawBuf, valsDataStart, i);
        const evNameEntry = readVtableEntry(rawBuf, evPos, ENUMVAL_VOFF_NAME);
        if (evNameEntry === 0)
            continue;
        const evName = readFbString(rawBuf, evPos + evNameEntry);
        if (evName === variantName) {
            const evValueEntry = readVtableEntry(rawBuf, evPos, ENUMVAL_VOFF_VALUE);
            if (evValueEntry === 0)
                return 0;
            return Number(readInt64(rawBuf, evPos + evValueEntry));
        }
    }
    return 0;
}
/** Find a union variant name and its schema object for the given discriminant. */
function resolveUnionVariant(schema, enumIdx, discriminant) {
    if (enumIdx < 0)
        return null;
    const rawBuf = rawSchemaBuffer(schema);
    if (rawBuf === null)
        return null;
    const enumsVec = enumsVector(rawBuf, schema);
    if (enumIdx >= enumsVec.count)
        return null;
    const enumPos = vectorTableAt(rawBuf, enumsVec.dataStart, enumIdx);
    const enumNameStr = (() => {
        const e = readVtableEntry(rawBuf, enumPos, ENUM_VOFF_NAME);
        return e === 0 ? '' : readFbString(rawBuf, enumPos + e);
    })();
    const valsEntry = readVtableEntry(rawBuf, enumPos, ENUM_VOFF_VALUES);
    if (valsEntry === 0)
        return null;
    const valsFieldPos = enumPos + valsEntry;
    const valsUoffset = readUint32(rawBuf, valsFieldPos);
    const valsStart = valsFieldPos + valsUoffset;
    const valsCount = readUint32(rawBuf, valsStart);
    const valsDataStart = valsStart + 4;
    for (let i = 0; i < valsCount; i++) {
        const evPos = vectorTableAt(rawBuf, valsDataStart, i);
        const evValueEntry = readVtableEntry(rawBuf, evPos, ENUMVAL_VOFF_VALUE);
        if (evValueEntry === 0)
            continue;
        const evValue = Number(readInt64(rawBuf, evPos + evValueEntry));
        if (evValue !== discriminant)
            continue;
        const evNameEntry = readVtableEntry(rawBuf, evPos, ENUMVAL_VOFF_NAME);
        if (evNameEntry === 0)
            continue;
        const variantName = readFbString(rawBuf, evPos + evNameEntry);
        const obj = resolveUnionObject(schema, enumNameStr, variantName);
        if (obj != null)
            return { variantName, obj };
    }
    return null;
}
/** Locate a union variant's ReflectionObject by checking exact name then namespace prefix. */
function resolveUnionObject(schema, enumNameStr, variantName) {
    const direct = schema.objectByName(variantName);
    if (direct != null)
        return direct;
    const dotPos = enumNameStr.lastIndexOf('.');
    if (dotPos >= 0) {
        const ns = enumNameStr.slice(0, dotPos + 1);
        return schema.objectByName(ns + variantName);
    }
    return undefined;
}
// ── Raw schema buffer access ────────────────────────────────────────────────
// ReflectionSchema keeps its DataView private, so we need a way to access the
// underlying bytes.  We construct a DataView from the schema in the Pack/Unpack
// helpers and pass it around explicitly.
/**
 * Extract the underlying Uint8Array from a ReflectionSchema.
 * ReflectionSchema stores a DataView; we reconstruct a Uint8Array from it.
 */
function rawSchemaBuffer(schema) {
    // Access the private buf via a brand-check cast.  The schema always has a
    // DataView; we get the underlying ArrayBuffer from it.
    const s = schema;
    if (!s.buf)
        return null;
    return new Uint8Array(s.buf.buffer, s.buf.byteOffset, s.buf.byteLength);
}
/** Read the enums vector from the raw schema bytes. */
function enumsVector(rawBuf, schema) {
    const s = schema;
    const rootPos = s.rootTablePos ?? 0;
    // Enums are at vtable offset 6 in the schema table (reflection.fbs).
    const entry = readVtableEntry(rawBuf, rootPos, 6);
    if (entry === 0)
        return { count: 0, dataStart: 0 };
    const fieldPos = rootPos + entry;
    const uoffset = readUint32(rawBuf, fieldPos);
    const vecStart = fieldPos + uoffset;
    const count = readUint32(rawBuf, vecStart);
    return { count, dataStart: vecStart + 4 };
}
/** Read the absolute position of table element i from a vector's dataStart. */
function vectorTableAt(rawBuf, dataStart, i) {
    const elemPos = dataStart + i * 4;
    return elemPos + readUint32(rawBuf, elemPos);
}
// ── ReflectUnpack ───────────────────────────────────────────────────────────
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
export function reflectUnpack(schema, typeName, buf) {
    const obj = schema.objectByName(typeName);
    if (obj == null)
        return null;
    if (buf.length < 4)
        return null;
    const rootPos = readUint32(buf, 0);
    return unpackTable(buf, rootPos, obj, schema);
}
/** Unpack a table or struct at tablePos into a plain record. */
function unpackTable(buf, tablePos, obj, schema) {
    const result = {};
    const isStruct = obj.isStruct();
    for (const field of obj.fields()) {
        const fieldType = field.type();
        if (fieldType == null)
            continue;
        const name = field.name();
        // Resolve the absolute byte position of the field data.
        let abs;
        if (isStruct) {
            abs = tablePos + field.offset();
        }
        else {
            const absOrZero = resolveVtableField(buf, tablePos, field.offset());
            if (absOrZero === 0)
                continue;
            abs = absOrZero;
        }
        unpackFieldIntoRecord(buf, tablePos, name, abs, fieldType, obj, schema, result);
    }
    return result;
}
/** Decode a single field and store it in result. */
function unpackFieldIntoRecord(buf, tablePos, name, abs, fieldType, obj, schema, result) {
    switch (fieldType.baseType()) {
        case ReflectionBaseType.Bool:
            if (abs < buf.length)
                result[name] = buf[abs] !== 0;
            break;
        case ReflectionBaseType.Byte:
            result[name] = buf.length > abs ? (buf[abs] > 127 ? buf[abs] - 256 : buf[abs]) : 0;
            break;
        case ReflectionBaseType.UByte:
            result[name] = abs < buf.length ? buf[abs] : 0;
            break;
        case ReflectionBaseType.Short:
            if (abs + 2 <= buf.length)
                result[name] = readInt16(buf, abs);
            break;
        case ReflectionBaseType.UShort:
            if (abs + 2 <= buf.length)
                result[name] = readUint16(buf, abs);
            break;
        case ReflectionBaseType.Int:
            if (abs + 4 <= buf.length)
                result[name] = readInt32(buf, abs);
            break;
        case ReflectionBaseType.UInt:
            if (abs + 4 <= buf.length)
                result[name] = readUint32(buf, abs);
            break;
        case ReflectionBaseType.Long:
            if (abs + 8 <= buf.length)
                result[name] = readInt64(buf, abs);
            break;
        case ReflectionBaseType.ULong:
            if (abs + 8 <= buf.length)
                result[name] = readUint64(buf, abs);
            break;
        case ReflectionBaseType.Float:
            if (abs + 4 <= buf.length)
                result[name] = readFloat32(buf, abs);
            break;
        case ReflectionBaseType.Double:
            if (abs + 8 <= buf.length)
                result[name] = readFloat64(buf, abs);
            break;
        case ReflectionBaseType.String:
            if (abs + 4 <= buf.length)
                result[name] = readFbString(buf, abs);
            break;
        case ReflectionBaseType.Obj: {
            const child = unpackObjField(buf, abs, fieldType, schema);
            if (child != null)
                result[name] = child;
            break;
        }
        case ReflectionBaseType.Vector: {
            const elems = unpackVector(buf, abs, fieldType, schema);
            result[name] = elems;
            break;
        }
        case ReflectionBaseType.UType:
            // Companion discriminant for unions — consumed by the Union case.
            break;
        case ReflectionBaseType.Union:
            unpackUnionField(buf, tablePos, name, abs, fieldType, obj, schema, result);
            break;
        default:
            // None, Array, Vector64: not valid field base types in this context.
            break;
    }
}
/** Unpack a nested table or struct field. */
function unpackObjField(buf, abs, fieldType, schema) {
    const idx = fieldType.index();
    if (idx < 0)
        return null;
    const objs = schema.objects();
    if (idx >= objs.length)
        return null;
    const childObj = objs[idx];
    let childPos;
    if (childObj.isStruct()) {
        childPos = abs;
    }
    else {
        if (abs + 4 > buf.length)
            return null;
        childPos = abs + readUint32(buf, abs);
    }
    return unpackTable(buf, childPos, childObj, schema);
}
/** Unpack a vector field into an array. */
function unpackVector(buf, abs, fieldType, schema) {
    if (abs + 4 > buf.length)
        return [];
    const vecStart = abs + readUint32(buf, abs);
    if (vecStart + 4 > buf.length)
        return [];
    const count = readUint32(buf, vecStart);
    const dataStart = vecStart + 4;
    return unpackVectorElements(buf, dataStart, count, fieldType, schema);
}
/** Decode each element of a vector. */
function unpackVectorElements(buf, dataStart, count, fieldType, schema) {
    const elem = fieldType.element();
    const result = [];
    switch (elem) {
        case ReflectionBaseType.Bool:
            for (let i = 0; i < count; i++)
                result.push(buf[dataStart + i] !== 0);
            break;
        case ReflectionBaseType.Byte:
            for (let i = 0; i < count; i++) {
                const v = buf[dataStart + i];
                result.push(v > 127 ? v - 256 : v);
            }
            break;
        case ReflectionBaseType.UByte:
            for (let i = 0; i < count; i++)
                result.push(buf[dataStart + i]);
            break;
        case ReflectionBaseType.Short:
            for (let i = 0; i < count; i++)
                result.push(readInt16(buf, dataStart + i * 2));
            break;
        case ReflectionBaseType.UShort:
            for (let i = 0; i < count; i++)
                result.push(readUint16(buf, dataStart + i * 2));
            break;
        case ReflectionBaseType.Int:
            for (let i = 0; i < count; i++)
                result.push(readInt32(buf, dataStart + i * 4));
            break;
        case ReflectionBaseType.UInt:
            for (let i = 0; i < count; i++)
                result.push(readUint32(buf, dataStart + i * 4));
            break;
        case ReflectionBaseType.Long:
            for (let i = 0; i < count; i++)
                result.push(readInt64(buf, dataStart + i * 8));
            break;
        case ReflectionBaseType.ULong:
            for (let i = 0; i < count; i++)
                result.push(readUint64(buf, dataStart + i * 8));
            break;
        case ReflectionBaseType.Float:
            for (let i = 0; i < count; i++)
                result.push(readFloat32(buf, dataStart + i * 4));
            break;
        case ReflectionBaseType.Double:
            for (let i = 0; i < count; i++)
                result.push(readFloat64(buf, dataStart + i * 8));
            break;
        case ReflectionBaseType.String:
            for (let i = 0; i < count; i++) {
                const elemPos = dataStart + i * 4;
                result.push(readFbString(buf, elemPos));
            }
            break;
        case ReflectionBaseType.Obj: {
            const idx = fieldType.index();
            if (idx >= 0 && idx < schema.objects().length) {
                const childObj = schema.objects()[idx];
                for (let i = 0; i < count; i++) {
                    const elemPos = dataStart + i * 4;
                    const childPos = childObj.isStruct() ? elemPos : elemPos + readUint32(buf, elemPos);
                    result.push(unpackTable(buf, childPos, childObj, schema));
                }
            }
            break;
        }
        default:
            // None, UType, Vector, Union, Array, Vector64: not valid element types.
            break;
    }
    return result;
}
/** Unpack a union field from the buffer into result. */
function unpackUnionField(buf, tablePos, name, abs, fieldType, obj, schema, result) {
    const typeField = obj.fieldByName(`${name}_type`);
    if (typeField == null)
        return;
    const typeAbs = resolveVtableField(buf, tablePos, typeField.offset());
    if (typeAbs === 0)
        return;
    const discriminant = buf[typeAbs] ?? 0;
    if (discriminant === 0)
        return;
    const variant = resolveUnionVariant(schema, fieldType.index(), discriminant);
    if (variant == null)
        return;
    if (abs + 4 > buf.length)
        return;
    const childPos = abs + readUint32(buf, abs);
    result[`${name}_type`] = variant.variantName;
    result[name] = unpackTable(buf, childPos, variant.obj, schema);
}
// ── ReflectPack ─────────────────────────────────────────────────────────────
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
export function reflectPack(schema, typeName, inputObj) {
    const obj = schema.objectByName(typeName);
    if (obj == null)
        return null;
    const builder = new FbBuilder(INITIAL_BUFFER_SIZE);
    const rootOffset = packTable(builder, inputObj, obj, schema);
    builder.finish(rootOffset);
    return builder.finishedBytes();
}
/** Pack a plain object as a FlatBuffers table. */
function packTable(builder, inputObj, obj, schema) {
    const fields = obj.fields();
    // Compute the maximum vtable slot.
    let maxSlot = 0;
    for (const field of fields) {
        const slot = vtableSlot(field.offset());
        if (slot > maxSlot)
            maxSlot = slot;
    }
    // Pass 1: pre-build all offset-typed fields before StartObject.
    const prebuilt = new Map();
    for (const field of fields) {
        const fieldType = field.type();
        if (fieldType == null)
            continue;
        const name = field.name();
        const val = inputObj[name];
        if (val == null)
            continue;
        const slot = vtableSlot(field.offset());
        const off = buildOffsetField(builder, inputObj, name, val, fieldType, schema);
        if (off !== 0)
            prebuilt.set(slot, off);
    }
    // Pass 2: write the table.
    builder.startObject(maxSlot + 1);
    for (const field of fields) {
        const fieldType = field.type();
        if (fieldType == null)
            continue;
        const name = field.name();
        const slot = vtableSlot(field.offset());
        packFieldSlot(builder, inputObj, name, slot, field, fieldType, obj, schema, prebuilt);
    }
    return builder.endObject();
}
/** Build an offset-typed field (String, Obj, Vector, Union) before the parent table opens. */
function buildOffsetField(builder, inputObj, name, val, fieldType, schema) {
    switch (fieldType.baseType()) {
        case ReflectionBaseType.String:
            if (typeof val === 'string')
                return builder.createString(val);
            break;
        case ReflectionBaseType.Obj: {
            if (typeof val === 'object' && val !== null && !Array.isArray(val)) {
                return packObjField(builder, val, fieldType, schema);
            }
            break;
        }
        case ReflectionBaseType.Vector:
            if (Array.isArray(val))
                return packVector(builder, val, fieldType, schema);
            break;
        case ReflectionBaseType.Union:
            return packUnionField(builder, inputObj, name, val, fieldType, schema);
        default:
            break;
    }
    return 0;
}
/** Write a single scalar or offset field into the open table. */
function packFieldSlot(builder, inputObj, name, slot, field, fieldType, obj, schema, prebuilt) {
    const base = fieldType.baseType();
    switch (base) {
        case ReflectionBaseType.Bool: {
            const val = inputObj[name];
            if (val != null)
                builder.prependBoolSlot(slot, Boolean(val));
            break;
        }
        case ReflectionBaseType.Byte:
        case ReflectionBaseType.UByte: {
            const val = inputObj[name];
            if (val != null)
                builder.prependUint8Slot(slot, Number(val) & 0xff);
            break;
        }
        case ReflectionBaseType.Short:
        case ReflectionBaseType.UShort: {
            const val = inputObj[name];
            if (val != null)
                builder.prependUint16Slot(slot, Number(val) & 0xffff);
            break;
        }
        case ReflectionBaseType.Int:
        case ReflectionBaseType.UInt: {
            const val = inputObj[name];
            if (val != null)
                builder.prependUint32Slot(slot, Number(val) >>> 0);
            break;
        }
        case ReflectionBaseType.Long:
        case ReflectionBaseType.ULong: {
            const val = inputObj[name];
            if (val != null)
                builder.prependUint64Slot(slot, BigInt(val));
            break;
        }
        case ReflectionBaseType.Float: {
            const val = inputObj[name];
            if (val != null)
                builder.prependFloat32Slot(slot, Number(val));
            break;
        }
        case ReflectionBaseType.Double: {
            const val = inputObj[name];
            if (val != null)
                builder.prependFloat64Slot(slot, Number(val));
            break;
        }
        case ReflectionBaseType.String:
        case ReflectionBaseType.Obj:
        case ReflectionBaseType.Vector:
        case ReflectionBaseType.Union: {
            const off = prebuilt.get(slot);
            if (off != null && off !== 0)
                builder.prependUOffsetSlot(slot, off);
            break;
        }
        case ReflectionBaseType.UType:
            packUTypeSlot(builder, inputObj, name, slot, field, obj, schema);
            break;
        default:
            break;
    }
}
/** Write the uint8 discriminant for a union companion _type field. */
function packUTypeSlot(builder, inputObj, name, slot, _field, obj, schema) {
    const typeSuffix = '_type';
    const unionFieldName = name.endsWith(typeSuffix) ? name.slice(0, -typeSuffix.length) : name;
    const unionField = obj.fieldByName(unionFieldName);
    if (unionField == null)
        return;
    const typeVal = inputObj[name];
    if (typeVal == null || typeof typeVal !== 'string')
        return;
    const uft = unionField.type();
    if (uft == null)
        return;
    const discriminant = resolveUnionDiscriminant(schema, uft.index(), typeVal);
    builder.prependUint8Slot(slot, discriminant & 0xff);
}
/** Pack a nested table or struct object field. */
function packObjField(builder, val, fieldType, schema) {
    const idx = fieldType.index();
    if (idx < 0 || idx >= schema.objects().length)
        return 0;
    return packTable(builder, val, schema.objects()[idx], schema);
}
/** Pack a union field using the companion _type key. */
function packUnionField(builder, inputObj, name, val, fieldType, schema) {
    const typeVal = inputObj[`${name}_type`];
    if (typeof typeVal !== 'string')
        return 0;
    if (typeof val !== 'object' || val === null || Array.isArray(val))
        return 0;
    const childObj = resolveUnionObject(schema, enumName(schema, fieldType.index()), typeVal);
    if (childObj == null)
        return 0;
    return packTable(builder, val, childObj, schema);
}
/** Pack a vector field from a JavaScript array. */
function packVector(builder, elems, fieldType, schema) {
    const elem = fieldType.element();
    const count = elems.length;
    if (count === 0) {
        builder.startVector(0, 0, 1);
        return builder.endVector(0);
    }
    switch (elem) {
        case ReflectionBaseType.String:
            return packVectorStrings(builder, elems);
        case ReflectionBaseType.Obj:
            return packVectorTables(builder, elems, fieldType, schema);
        case ReflectionBaseType.Bool:
        case ReflectionBaseType.Byte:
        case ReflectionBaseType.UByte:
            return packVectorBytes(builder, elems, count);
        case ReflectionBaseType.Short:
        case ReflectionBaseType.UShort:
            return packVectorShorts(builder, elems, count);
        case ReflectionBaseType.Int:
        case ReflectionBaseType.UInt:
            return packVectorInts(builder, elems, count);
        case ReflectionBaseType.Long:
        case ReflectionBaseType.ULong:
            return packVectorLongs(builder, elems, count);
        case ReflectionBaseType.Float:
            return packVectorFloat32s(builder, elems, count);
        case ReflectionBaseType.Double:
            return packVectorFloat64s(builder, elems, count);
        default:
            return 0;
    }
}
function packVectorStrings(builder, elems) {
    const offsets = elems.map(v => builder.createString(String(v ?? '')));
    builder.startVector(4, offsets.length, 4);
    for (let i = offsets.length - 1; i >= 0; i--)
        builder.prependUOffset(offsets[i]);
    return builder.endVector(offsets.length);
}
function packVectorTables(builder, elems, fieldType, schema) {
    const idx = fieldType.index();
    if (idx < 0 || idx >= schema.objects().length)
        return 0;
    const childObj = schema.objects()[idx];
    const offsets = elems.map(v => packTable(builder, v ?? {}, childObj, schema));
    builder.startVector(4, offsets.length, 4);
    for (let i = offsets.length - 1; i >= 0; i--)
        builder.prependUOffset(offsets[i]);
    return builder.endVector(offsets.length);
}
function packVectorBytes(builder, elems, count) {
    builder.startVector(1, count, 1);
    for (let i = count - 1; i >= 0; i--)
        builder.prependByte(Number(elems[i]) & 0xff);
    return builder.endVector(count);
}
function packVectorShorts(builder, elems, count) {
    builder.startVector(2, count, 2);
    for (let i = count - 1; i >= 0; i--)
        builder.prependUint16(Number(elems[i]) & 0xffff);
    return builder.endVector(count);
}
function packVectorInts(builder, elems, count) {
    builder.startVector(4, count, 4);
    for (let i = count - 1; i >= 0; i--)
        builder.prependUint32(Number(elems[i]) >>> 0);
    return builder.endVector(count);
}
function packVectorLongs(builder, elems, count) {
    builder.startVector(8, count, 8);
    for (let i = count - 1; i >= 0; i--) {
        const v = BigInt(elems[i]);
        builder.prependUint64(v);
    }
    return builder.endVector(count);
}
function packVectorFloat32s(builder, elems, count) {
    builder.startVector(4, count, 4);
    for (let i = count - 1; i >= 0; i--)
        builder.prependFloat32(Number(elems[i]));
    return builder.endVector(count);
}
function packVectorFloat64s(builder, elems, count) {
    builder.startVector(8, count, 8);
    for (let i = count - 1; i >= 0; i--)
        builder.prependFloat64(Number(elems[i]));
    return builder.endVector(count);
}
// ── Minimal FlatBuffers Builder ─────────────────────────────────────────────
// A self-contained builder that only uses browser-native APIs (DataView,
// Uint8Array).  It implements the standard FlatBuffers little-endian, growing
// back-to-front layout.
class FbBuilder {
    constructor(initialSize) {
        this.buf = new ArrayBuffer(initialSize);
        this.view = new DataView(this.buf);
        this.bytes = new Uint8Array(this.buf);
        this.space = initialSize;
        this.minalign = 1;
        this.vtable = null;
        this.vtableSize = 0;
        this.objectStart = 0;
        this.vtables = [];
        this.head = initialSize;
    }
    capacity() {
        return this.buf.byteLength;
    }
    grow(neededSpace) {
        let newSize = this.capacity();
        while (newSize - this.head + this.space < neededSpace)
            newSize *= 2;
        const newBuf = new ArrayBuffer(newSize);
        const newBytes = new Uint8Array(newBuf);
        newBytes.set(this.bytes.subarray(this.head), newSize - (this.capacity() - this.head));
        this.buf = newBuf;
        this.view = new DataView(this.buf);
        this.bytes = newBytes;
        this.space += newSize - this.capacity() + newBuf.byteLength - newSize;
        this.head += newSize - this.capacity() + newBuf.byteLength - newSize;
    }
    pad(n) {
        for (let i = 0; i < n; i++)
            this.prependByte(0);
    }
    align(size) {
        if (size > this.minalign)
            this.minalign = size;
        const alignSize = (~(this.capacity() - this.head + this.space) + 1) & (size - 1);
        if (this.space < alignSize + size)
            this.growBytes(alignSize + size);
        this.pad(alignSize);
    }
    growBytes(n) {
        const newSize = Math.max(this.capacity() * 2, this.capacity() + n + 256);
        const newBuf = new ArrayBuffer(newSize);
        const newBytes = new Uint8Array(newBuf);
        const oldUsed = this.capacity() - this.head;
        newBytes.set(this.bytes.subarray(this.head), newSize - oldUsed);
        const delta = newSize - this.capacity();
        this.buf = newBuf;
        this.view = new DataView(this.buf);
        this.bytes = newBytes;
        this.space += delta;
        this.head += delta;
    }
    prependByte(v) {
        if (this.space < 1)
            this.growBytes(1);
        this.space -= 1;
        this.head -= 1;
        this.bytes[this.head] = v & 0xff;
    }
    prependUint16Internal(v) {
        if (this.space < 2)
            this.growBytes(2);
        this.space -= 2;
        this.head -= 2;
        this.view.setUint16(this.head, v, true);
    }
    prependInt32Internal(v) {
        if (this.space < 4)
            this.growBytes(4);
        this.space -= 4;
        this.head -= 4;
        this.view.setInt32(this.head, v, true);
    }
    prependUint32Internal(v) {
        if (this.space < 4)
            this.growBytes(4);
        this.space -= 4;
        this.head -= 4;
        this.view.setUint32(this.head, v, true);
    }
    prependUint16(v) {
        this.align(2);
        this.prependUint16Internal(v);
    }
    prependUint32(v) {
        this.align(4);
        this.prependUint32Internal(v);
    }
    prependUOffset(v) {
        this.align(4);
        const relOffset = this.capacity() - this.head + v - 4;
        this.prependUint32Internal(relOffset);
    }
    prependFloat32Internal(v) {
        if (this.space < 4)
            this.growBytes(4);
        this.space -= 4;
        this.head -= 4;
        this.view.setFloat32(this.head, v, true);
    }
    prependFloat64Internal(v) {
        if (this.space < 8)
            this.growBytes(8);
        this.space -= 8;
        this.head -= 8;
        this.view.setFloat64(this.head, v, true);
    }
    prependUint64Internal(v) {
        if (this.space < 8)
            this.growBytes(8);
        this.space -= 8;
        this.head -= 8;
        this.view.setBigUint64(this.head, v, true);
    }
    offset() {
        return this.capacity() - this.head;
    }
    startObject(numFields) {
        this.vtable = new Array(numFields).fill(0);
        this.vtableSize = numFields;
        this.objectStart = this.offset();
    }
    slot(slotIndex) {
        if (this.vtable != null && slotIndex < this.vtable.length) {
            this.vtable[slotIndex] = this.offset();
        }
    }
    prependBoolSlot(slotIndex, v) {
        this.align(1);
        this.prependByte(v ? 1 : 0);
        this.slot(slotIndex);
    }
    prependUint8Slot(slotIndex, v) {
        this.align(1);
        this.prependByte(v & 0xff);
        this.slot(slotIndex);
    }
    prependUint16Slot(slotIndex, v) {
        this.align(2);
        this.prependUint16Internal(v & 0xffff);
        this.slot(slotIndex);
    }
    prependUint32Slot(slotIndex, v) {
        this.align(4);
        this.prependUint32Internal(v >>> 0);
        this.slot(slotIndex);
    }
    prependUint64Slot(slotIndex, v) {
        this.align(8);
        this.prependUint64Internal(BigInt.asUintN(64, v));
        this.slot(slotIndex);
    }
    prependFloat32Slot(slotIndex, v) {
        this.align(4);
        this.prependFloat32Internal(v);
        this.slot(slotIndex);
    }
    prependFloat64Slot(slotIndex, v) {
        this.align(8);
        this.prependFloat64Internal(v);
        this.slot(slotIndex);
    }
    prependUOffsetSlot(slotIndex, off) {
        this.prependUOffset(off);
        this.slot(slotIndex);
    }
    prependUint64(v) {
        this.align(8);
        this.prependUint64Internal(BigInt.asUintN(64, v));
    }
    prependFloat32(v) {
        this.align(4);
        this.prependFloat32Internal(v);
    }
    prependFloat64(v) {
        this.align(8);
        this.prependFloat64Internal(v);
    }
    endObject() {
        if (this.vtable == null)
            return 0;
        // Write soffset_t back to start of table (points to vtable).
        this.prependInt32Internal(0); // placeholder
        const tableOff = this.offset();
        // Build vtable: [vtableSize, tableDataSize, slot0, slot1, ...]
        const numSlots = this.vtableSize;
        const vtableNumBytes = 4 + numSlots * 2; // header(4) + slots
        const vtableBuf = new Uint8Array(vtableNumBytes);
        const vtableView = new DataView(vtableBuf.buffer);
        vtableView.setUint16(0, vtableNumBytes, true); // vtableBytes
        vtableView.setUint16(2, tableOff - this.objectStart, true); // tableDataSize
        let hasNonZero = false;
        for (let i = 0; i < numSlots; i++) {
            const voff = this.vtable[i] === 0 ? 0 : this.vtable[i] - this.objectStart;
            vtableView.setUint16(4 + i * 2, voff, true);
            if (voff !== 0)
                hasNonZero = true;
        }
        this.vtable = null;
        // Attempt to share vtable.
        let vtableOffset = 0;
        if (hasNonZero) {
            // Check existing vtables for equality.
            outer: for (const existVtable of this.vtables) {
                const existPos = this.capacity() - existVtable;
                const existSize = readUint16(this.bytes, existPos);
                if (existSize !== vtableNumBytes)
                    continue;
                for (let bi = 0; bi < vtableNumBytes; bi++) {
                    if (this.bytes[existPos + bi] !== vtableBuf[bi])
                        continue outer;
                }
                vtableOffset = existVtable;
                break;
            }
        }
        if (vtableOffset === 0 && hasNonZero) {
            // Write vtable into buffer.
            for (let bi = vtableNumBytes - 1; bi >= 0; bi--) {
                this.prependByte(vtableBuf[bi]);
            }
            vtableOffset = this.offset();
            this.vtables.push(vtableOffset);
        }
        // Write soffset (table pos - vtable pos, signed) into the table's first field.
        const tableHeadPos = this.capacity() - tableOff;
        const vtableHeadPos = vtableOffset === 0 ? tableHeadPos : this.capacity() - vtableOffset;
        const soffset = tableHeadPos - vtableHeadPos;
        this.view.setInt32(tableHeadPos, soffset, true);
        return tableOff;
    }
    startVector(elemSize, count, alignment) {
        this.align(alignment);
        this.align(elemSize);
        void count; // count is written by endVector
    }
    endVector(count) {
        this.prependUint32Internal(count);
        return this.offset();
    }
    createString(str) {
        const encoded = new TextEncoder().encode(str);
        // null terminator
        this.align(1);
        this.prependByte(0);
        // string bytes in reverse
        for (let i = encoded.length - 1; i >= 0; i--)
            this.prependByte(encoded[i]);
        // length prefix
        this.align(4);
        this.prependUint32Internal(encoded.length);
        return this.offset();
    }
    finish(rootOffset) {
        this.align(4);
        this.prependUOffset(rootOffset);
    }
    finishedBytes() {
        return this.bytes.slice(this.head);
    }
}
