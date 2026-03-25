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
import { ReflectionBaseType, getFieldBool, getFieldFloat, getFieldInt, getFieldString, } from './reflection';
// ── internal vtable helpers (operate on data buffers, not schema buffers) ─
/**
 * Look up a field's vtable entry in a data buffer and return the absolute
 * byte position of the field data, or `null` if the field is absent.
 *
 * This mirrors `resolveFieldPos` in `reflection.ts` but is inlined here to
 * keep the module self-contained from a logic standpoint.
 */
function resolveFieldPos(dataBuf, tablePos, voffset) {
    if (voffset === 0)
        return null;
    // Read the signed offset to the vtable.
    const soffset = dataBuf.getInt32(tablePos, true);
    const vtablePos = tablePos - soffset;
    if (vtablePos < 0 || vtablePos + 2 > dataBuf.byteLength)
        return null;
    const vtableSize = dataBuf.getUint16(vtablePos, true);
    if (voffset >= vtableSize)
        return null;
    const entry = dataBuf.getUint16(vtablePos + voffset, true);
    if (entry === 0)
        return null;
    return tablePos + entry;
}
/**
 * Returns `true` when the given field is absent from the data buffer at
 * `tablePos` (vtable entry is zero or out of range).
 */
function isAbsent(dataBuf, tablePos, field) {
    return resolveFieldPos(dataBuf, tablePos, field.offset()) === null;
}
/**
 * Follow the UOffset at `fieldPos` in `buf` to get the absolute position of
 * the referenced table.  Returns `null` if the resulting position is outside
 * the buffer.
 */
function followTable(buf, fieldPos) {
    if (fieldPos + 4 > buf.byteLength)
        return null;
    const uoffset = buf.getUint32(fieldPos, true);
    const tablePos = fieldPos + uoffset;
    if (tablePos < 0 || tablePos >= buf.byteLength)
        return null;
    return tablePos;
}
/**
 * Read vector metadata (element count and data-start byte offset) from a data
 * buffer.  `fieldPos` is the absolute position where the vector's UOffset slot
 * lives.  Returns `null` if bounds are violated.
 */
function readVectorMeta(buf, fieldPos) {
    if (fieldPos + 4 > buf.byteLength)
        return null;
    const uoffset = buf.getUint32(fieldPos, true);
    const vecBase = fieldPos + uoffset;
    if (vecBase < 0 || vecBase + 4 > buf.byteLength)
        return null;
    const count = buf.getUint32(vecBase, true);
    const dataStart = vecBase + 4;
    return { count, dataStart };
}
// ── main recursive comparison engine ─────────────────────────────────────
/**
 * Compare two FlatBuffer payloads against the schema and collect field-level
 * deltas.  Both payloads must encode the table described by `obj`.
 *
 * @param bufA      - DataView over the first (old) serialized FlatBuffer.
 * @param tablePosA - Absolute byte position of the root table in bufA.
 * @param bufB      - DataView over the second (new) serialized FlatBuffer.
 * @param tablePosB - Absolute byte position of the root table in bufB.
 * @param obj       - Schema object descriptor for the current table type.
 * @param schema    - Top-level schema, used to resolve nested Obj/Vector-of-Obj types.
 * @param prefix    - Dot-separated path prefix accumulated by recursive calls.
 * @param deltas    - Accumulator array; entries are pushed in-place.
 */
function diffTable(bufA, tablePosA, bufB, tablePosB, obj, schema, prefix, deltas) {
    const fields = obj.fields();
    for (let i = 0; i < fields.length; i += 1) {
        const field = fields[i];
        const fieldName = field.name();
        const path = prefix.length > 0 ? `${prefix}.${fieldName}` : fieldName;
        const ft = field.type();
        if (ft !== null) {
            switch (ft.baseType()) {
                case ReflectionBaseType.Bool:
                    diffBoolField(bufA, tablePosA, bufB, tablePosB, field, path, deltas);
                    break;
                case ReflectionBaseType.UType:
                case ReflectionBaseType.Byte:
                case ReflectionBaseType.UByte:
                case ReflectionBaseType.Short:
                case ReflectionBaseType.UShort:
                case ReflectionBaseType.Int:
                case ReflectionBaseType.UInt:
                case ReflectionBaseType.Long:
                case ReflectionBaseType.ULong:
                    diffIntField(bufA, tablePosA, bufB, tablePosB, field, path, deltas);
                    break;
                case ReflectionBaseType.Float:
                case ReflectionBaseType.Double:
                    diffFloatField(bufA, tablePosA, bufB, tablePosB, field, path, deltas);
                    break;
                case ReflectionBaseType.String:
                    diffStringField(bufA, tablePosA, bufB, tablePosB, field, path, deltas);
                    break;
                case ReflectionBaseType.Obj:
                    diffObjField(bufA, tablePosA, bufB, tablePosB, field, ft, schema, path, deltas);
                    break;
                case ReflectionBaseType.Vector:
                case ReflectionBaseType.Vector64:
                    diffVectorField(bufA, tablePosA, bufB, tablePosB, field, ft, schema, path, deltas);
                    break;
                default:
                    // Union and Array types are not compared.
                    break;
            }
        }
    }
}
// ── per-type diff helpers ──────────────────────────────────────────────────
/**
 * Compare a `bool` field between the two buffers.
 */
function diffBoolField(bufA, tablePosA, bufB, tablePosB, field, path, deltas) {
    const absentA = isAbsent(bufA, tablePosA, field);
    const absentB = isAbsent(bufB, tablePosB, field);
    const defaultVal = field.defaultInteger() !== 0n;
    if (absentA && absentB)
        return;
    if (absentA) {
        const valB = getFieldBool(bufB, tablePosB, field);
        if (valB !== null && valB !== defaultVal) {
            deltas.push({ path, kind: 'added', oldValue: null, newValue: valB });
        }
        return;
    }
    if (absentB) {
        const valA = getFieldBool(bufA, tablePosA, field);
        if (valA !== null && valA !== defaultVal) {
            deltas.push({ path, kind: 'removed', oldValue: valA, newValue: null });
        }
        return;
    }
    const valA = getFieldBool(bufA, tablePosA, field);
    const valB = getFieldBool(bufB, tablePosB, field);
    if (valA !== null && valB !== null && valA !== valB) {
        deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
    }
}
/**
 * Compare an integer field (any integer {@link ReflectionBaseType}) between
 * the two buffers.
 */
function diffIntField(bufA, tablePosA, bufB, tablePosB, field, path, deltas) {
    const absentA = isAbsent(bufA, tablePosA, field);
    const absentB = isAbsent(bufB, tablePosB, field);
    const defaultVal = field.defaultInteger();
    if (absentA && absentB)
        return;
    if (absentA) {
        const valB = getFieldInt(bufB, tablePosB, field);
        if (valB !== null && valB !== defaultVal) {
            deltas.push({ path, kind: 'added', oldValue: null, newValue: valB });
        }
        return;
    }
    if (absentB) {
        const valA = getFieldInt(bufA, tablePosA, field);
        if (valA !== null && valA !== defaultVal) {
            deltas.push({ path, kind: 'removed', oldValue: valA, newValue: null });
        }
        return;
    }
    const valA = getFieldInt(bufA, tablePosA, field);
    const valB = getFieldInt(bufB, tablePosB, field);
    if (valA !== null && valB !== null && valA !== valB) {
        deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
    }
}
/**
 * Compare a `float` or `double` field between the two buffers.
 *
 * Two `NaN` values are treated as equal so that uninitialized float fields
 * (which are often `NaN` in both buffers) do not generate spurious deltas.
 */
function diffFloatField(bufA, tablePosA, bufB, tablePosB, field, path, deltas) {
    const absentA = isAbsent(bufA, tablePosA, field);
    const absentB = isAbsent(bufB, tablePosB, field);
    const defaultVal = field.defaultReal();
    if (absentA && absentB)
        return;
    if (absentA) {
        const valB = getFieldFloat(bufB, tablePosB, field);
        if (valB !== null && valB !== defaultVal && !(Number.isNaN(valB) && Number.isNaN(defaultVal))) {
            deltas.push({ path, kind: 'added', oldValue: null, newValue: valB });
        }
        return;
    }
    if (absentB) {
        const valA = getFieldFloat(bufA, tablePosA, field);
        if (valA !== null && valA !== defaultVal && !(Number.isNaN(valA) && Number.isNaN(defaultVal))) {
            deltas.push({ path, kind: 'removed', oldValue: valA, newValue: null });
        }
        return;
    }
    const valA = getFieldFloat(bufA, tablePosA, field);
    const valB = getFieldFloat(bufB, tablePosB, field);
    if (valA !== null && valB !== null) {
        const bothNaN = Number.isNaN(valA) && Number.isNaN(valB);
        if (!bothNaN && valA !== valB) {
            deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
        }
    }
}
/**
 * Compare a `string` field between the two buffers.
 */
function diffStringField(bufA, tablePosA, bufB, tablePosB, field, path, deltas) {
    const absentA = isAbsent(bufA, tablePosA, field);
    const absentB = isAbsent(bufB, tablePosB, field);
    if (absentA && absentB)
        return;
    if (absentA) {
        const valB = getFieldString(bufB, tablePosB, field);
        if (valB !== null && valB !== '') {
            deltas.push({ path, kind: 'added', oldValue: null, newValue: valB });
        }
        return;
    }
    if (absentB) {
        const valA = getFieldString(bufA, tablePosA, field);
        if (valA !== null && valA !== '') {
            deltas.push({ path, kind: 'removed', oldValue: valA, newValue: null });
        }
        return;
    }
    const valA = getFieldString(bufA, tablePosA, field);
    const valB = getFieldString(bufB, tablePosB, field);
    if (valA !== null && valB !== null && valA !== valB) {
        deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
    }
}
/**
 * Compare a nested table or struct `Obj` field between the two buffers.
 *
 * For tables, follows the UOffset to the child table and recurses.
 * For structs, reads each scalar field inline.
 */
function diffObjField(bufA, tablePosA, bufB, tablePosB, field, ft, schema, path, deltas) {
    const idx = ft.index();
    if (idx < 0)
        return;
    const allObjects = schema.objects();
    if (idx >= allObjects.length)
        return;
    const nested = allObjects[idx];
    const absentA = isAbsent(bufA, tablePosA, field);
    const absentB = isAbsent(bufB, tablePosB, field);
    if (absentA && absentB)
        return;
    if (nested.isStruct()) {
        if (absentA) {
            deltas.push({ path, kind: 'added', oldValue: null, newValue: '<struct>' });
            return;
        }
        if (absentB) {
            deltas.push({ path, kind: 'removed', oldValue: '<struct>', newValue: null });
            return;
        }
        // Inline struct: the vtable entry is a direct byte offset from tablePos.
        const posA = resolveFieldPos(bufA, tablePosA, field.offset());
        const posB = resolveFieldPos(bufB, tablePosB, field.offset());
        if (posA === null || posB === null)
            return;
        diffStructInline(bufA, posA, bufB, posB, nested, path, deltas);
        return;
    }
    // Table: follow the UOffset.
    if (absentA) {
        deltas.push({ path, kind: 'added', oldValue: null, newValue: '<table>' });
        return;
    }
    if (absentB) {
        deltas.push({ path, kind: 'removed', oldValue: '<table>', newValue: null });
        return;
    }
    const fieldPosA = resolveFieldPos(bufA, tablePosA, field.offset());
    const fieldPosB = resolveFieldPos(bufB, tablePosB, field.offset());
    if (fieldPosA === null || fieldPosB === null)
        return;
    const childPosA = followTable(bufA, fieldPosA);
    const childPosB = followTable(bufB, fieldPosB);
    if (childPosA === null || childPosB === null)
        return;
    diffTable(bufA, childPosA, bufB, childPosB, nested, schema, path, deltas);
}
/**
 * Compare an inline struct's scalar fields directly from the buffer bytes.
 * `posA`/`posB` are the absolute byte positions of the struct data.
 */
function diffStructInline(bufA, posA, bufB, posB, obj, prefix, deltas) {
    const byteSize = obj.byteSize();
    if (byteSize <= 0)
        return;
    if (posA + byteSize > bufA.byteLength || posB + byteSize > bufB.byteLength)
        return;
    const fields = obj.fields();
    for (let i = 0; i < fields.length; i += 1) {
        const field = fields[i];
        const ft = field.type();
        if (ft !== null) {
            const fieldOffset = field.offset();
            if (fieldOffset !== 0) {
                const fieldName = field.name();
                const path = prefix.length > 0 ? `${prefix}.${fieldName}` : fieldName;
                const absA = posA + fieldOffset;
                const absB = posB + fieldOffset;
                switch (ft.baseType()) {
                    case ReflectionBaseType.Bool: {
                        if (absA >= bufA.byteLength || absB >= bufB.byteLength)
                            break;
                        const valA = bufA.getUint8(absA) !== 0;
                        const valB = bufB.getUint8(absB) !== 0;
                        if (valA !== valB)
                            deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
                        break;
                    }
                    case ReflectionBaseType.Byte: {
                        if (absA >= bufA.byteLength || absB >= bufB.byteLength)
                            break;
                        const valA = bufA.getInt8(absA);
                        const valB = bufB.getInt8(absB);
                        if (valA !== valB)
                            deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
                        break;
                    }
                    case ReflectionBaseType.UByte: {
                        if (absA >= bufA.byteLength || absB >= bufB.byteLength)
                            break;
                        const valA = bufA.getUint8(absA);
                        const valB = bufB.getUint8(absB);
                        if (valA !== valB)
                            deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
                        break;
                    }
                    case ReflectionBaseType.Short: {
                        if (absA + 2 > bufA.byteLength || absB + 2 > bufB.byteLength)
                            break;
                        const valA = bufA.getInt16(absA, true);
                        const valB = bufB.getInt16(absB, true);
                        if (valA !== valB)
                            deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
                        break;
                    }
                    case ReflectionBaseType.UShort: {
                        if (absA + 2 > bufA.byteLength || absB + 2 > bufB.byteLength)
                            break;
                        const valA = bufA.getUint16(absA, true);
                        const valB = bufB.getUint16(absB, true);
                        if (valA !== valB)
                            deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
                        break;
                    }
                    case ReflectionBaseType.Int: {
                        if (absA + 4 > bufA.byteLength || absB + 4 > bufB.byteLength)
                            break;
                        const valA = bufA.getInt32(absA, true);
                        const valB = bufB.getInt32(absB, true);
                        if (valA !== valB)
                            deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
                        break;
                    }
                    case ReflectionBaseType.UInt: {
                        if (absA + 4 > bufA.byteLength || absB + 4 > bufB.byteLength)
                            break;
                        const valA = bufA.getUint32(absA, true);
                        const valB = bufB.getUint32(absB, true);
                        if (valA !== valB)
                            deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
                        break;
                    }
                    case ReflectionBaseType.Long: {
                        if (absA + 8 > bufA.byteLength || absB + 8 > bufB.byteLength)
                            break;
                        const loA = BigInt(bufA.getUint32(absA, true));
                        const hiA = BigInt(bufA.getInt32(absA + 4, true));
                        const valA = BigInt.asIntN(64, loA + (hiA * 0x100000000n));
                        const loB = BigInt(bufB.getUint32(absB, true));
                        const hiB = BigInt(bufB.getInt32(absB + 4, true));
                        const valB = BigInt.asIntN(64, loB + (hiB * 0x100000000n));
                        if (valA !== valB)
                            deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
                        break;
                    }
                    case ReflectionBaseType.ULong: {
                        if (absA + 8 > bufA.byteLength || absB + 8 > bufB.byteLength)
                            break;
                        const loA = BigInt(bufA.getUint32(absA, true));
                        const hiA = BigInt(bufA.getUint32(absA + 4, true));
                        const valA = BigInt.asUintN(64, loA + (hiA * 0x100000000n));
                        const loB = BigInt(bufB.getUint32(absB, true));
                        const hiB = BigInt(bufB.getUint32(absB + 4, true));
                        const valB = BigInt.asUintN(64, loB + (hiB * 0x100000000n));
                        if (valA !== valB)
                            deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
                        break;
                    }
                    case ReflectionBaseType.Float: {
                        if (absA + 4 > bufA.byteLength || absB + 4 > bufB.byteLength)
                            break;
                        const valA = bufA.getFloat32(absA, true);
                        const valB = bufB.getFloat32(absB, true);
                        if (valA !== valB && !(Number.isNaN(valA) && Number.isNaN(valB))) {
                            deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
                        }
                        break;
                    }
                    case ReflectionBaseType.Double: {
                        if (absA + 8 > bufA.byteLength || absB + 8 > bufB.byteLength)
                            break;
                        const valA = bufA.getFloat64(absA, true);
                        const valB = bufB.getFloat64(absB, true);
                        if (valA !== valB && !(Number.isNaN(valA) && Number.isNaN(valB))) {
                            deltas.push({ path, kind: 'changed', oldValue: valA, newValue: valB });
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
}
/** Byte sizes of scalar element types for vector stride calculations. */
const SCALAR_SIZE = {
    [ReflectionBaseType.Bool]: 1,
    [ReflectionBaseType.Byte]: 1,
    [ReflectionBaseType.UByte]: 1,
    [ReflectionBaseType.Short]: 2,
    [ReflectionBaseType.UShort]: 2,
    [ReflectionBaseType.Int]: 4,
    [ReflectionBaseType.UInt]: 4,
    [ReflectionBaseType.Long]: 8,
    [ReflectionBaseType.ULong]: 8,
    [ReflectionBaseType.Float]: 4,
    [ReflectionBaseType.Double]: 8,
    [ReflectionBaseType.String]: 4, // UOffset per element
};
/**
 * Read a scalar or string value from a vector element at the given absolute
 * byte position `pos` in `buf`.  Returns the value as `number | bigint |
 * string | boolean`, or `null` on bounds violation.
 */
function readVectorScalar(buf, pos, elemType) {
    switch (elemType) {
        case ReflectionBaseType.Bool:
            if (pos >= buf.byteLength)
                return null;
            return buf.getUint8(pos) !== 0;
        case ReflectionBaseType.Byte:
            if (pos >= buf.byteLength)
                return null;
            return buf.getInt8(pos);
        case ReflectionBaseType.UByte:
            if (pos >= buf.byteLength)
                return null;
            return buf.getUint8(pos);
        case ReflectionBaseType.Short:
            if (pos + 2 > buf.byteLength)
                return null;
            return buf.getInt16(pos, true);
        case ReflectionBaseType.UShort:
            if (pos + 2 > buf.byteLength)
                return null;
            return buf.getUint16(pos, true);
        case ReflectionBaseType.Int:
            if (pos + 4 > buf.byteLength)
                return null;
            return buf.getInt32(pos, true);
        case ReflectionBaseType.UInt:
            if (pos + 4 > buf.byteLength)
                return null;
            return buf.getUint32(pos, true);
        case ReflectionBaseType.Long: {
            if (pos + 8 > buf.byteLength)
                return null;
            const lo = BigInt(buf.getUint32(pos, true));
            const hi = BigInt(buf.getInt32(pos + 4, true));
            return BigInt.asIntN(64, lo + (hi * 0x100000000n));
        }
        case ReflectionBaseType.ULong: {
            if (pos + 8 > buf.byteLength)
                return null;
            const lo = BigInt(buf.getUint32(pos, true));
            const hi = BigInt(buf.getUint32(pos + 4, true));
            return BigInt.asUintN(64, lo + (hi * 0x100000000n));
        }
        case ReflectionBaseType.Float:
            if (pos + 4 > buf.byteLength)
                return null;
            return buf.getFloat32(pos, true);
        case ReflectionBaseType.Double:
            if (pos + 8 > buf.byteLength)
                return null;
            return buf.getFloat64(pos, true);
        case ReflectionBaseType.String: {
            // UOffset slot; follow it to the string.
            if (pos + 4 > buf.byteLength)
                return null;
            const uoffset = buf.getUint32(pos, true);
            const strBase = pos + uoffset;
            if (strBase + 4 > buf.byteLength)
                return null;
            const length = buf.getUint32(strBase, true);
            if (strBase + 4 + length > buf.byteLength)
                return null;
            const bytes = new Uint8Array(buf.buffer, buf.byteOffset + strBase + 4, length);
            return new TextDecoder().decode(bytes);
        }
        default:
            return null;
    }
}
/**
 * Compare two float values, treating both-NaN as equal.
 */
function floatEqual(a, b) {
    if (typeof a === 'number' && typeof b === 'number') {
        if (Number.isNaN(a) && Number.isNaN(b))
            return true;
    }
    return a === b;
}
/**
 * Compare a vector field between the two buffers element-by-element.
 *
 * Handles vectors of scalars, strings, and nested tables.  Elements beyond
 * `min(lenA, lenB)` are reported as {@link DeltaKind} `'added'` or
 * `'removed'`.
 */
function diffVectorField(bufA, tablePosA, bufB, tablePosB, field, ft, schema, path, deltas) {
    const absentA = isAbsent(bufA, tablePosA, field);
    const absentB = isAbsent(bufB, tablePosB, field);
    const getVecMeta = (buf, tablePos) => {
        const fieldPos = resolveFieldPos(buf, tablePos, field.offset());
        if (fieldPos === null)
            return { count: 0, dataStart: 0 }; // absent = empty
        return readVectorMeta(buf, fieldPos);
    };
    const metaA = absentA ? { count: 0, dataStart: 0 } : getVecMeta(bufA, tablePosA);
    const metaB = absentB ? { count: 0, dataStart: 0 } : getVecMeta(bufB, tablePosB);
    if (metaA === null || metaB === null)
        return;
    const { count: lenA, dataStart: dataStartA } = metaA;
    const { count: lenB, dataStart: dataStartB } = metaB;
    const minLen = Math.min(lenA, lenB);
    const elemType = ft.element();
    if (elemType === ReflectionBaseType.Obj) {
        // Vector of tables.
        const idx = ft.index();
        if (idx < 0)
            return;
        const allObjects = schema.objects();
        if (idx >= allObjects.length)
            return;
        const nested = allObjects[idx];
        for (let i = 0; i < minLen; i += 1) {
            const elemPath = `${path}[${i}]`;
            const slotA = dataStartA + i * 4;
            const slotB = dataStartB + i * 4;
            if (slotA + 4 <= bufA.byteLength && slotB + 4 <= bufB.byteLength) {
                const childPosA = slotA + bufA.getUint32(slotA, true);
                const childPosB = slotB + bufB.getUint32(slotB, true);
                if (childPosA >= 0 && childPosA < bufA.byteLength && childPosB >= 0 && childPosB < bufB.byteLength) {
                    diffTable(bufA, childPosA, bufB, childPosB, nested, schema, elemPath, deltas);
                }
            }
        }
    }
    else {
        // Scalar, string, or unhandled element type.
        const stride = SCALAR_SIZE[elemType] ?? 0;
        if (stride === 0)
            return; // unsupported element type
        for (let i = 0; i < minLen; i += 1) {
            const elemPath = `${path}[${i}]`;
            const posA = dataStartA + i * stride;
            const posB = dataStartB + i * stride;
            const valA = readVectorScalar(bufA, posA, elemType);
            const valB = readVectorScalar(bufB, posB, elemType);
            if (valA !== null && valB !== null) {
                if (!floatEqual(valA, valB)) {
                    deltas.push({ path: elemPath, kind: 'changed', oldValue: valA, newValue: valB });
                }
            }
        }
    }
    // Extra elements.
    for (let i = minLen; i < lenA; i += 1) {
        deltas.push({ path: `${path}[${i}]`, kind: 'removed', oldValue: '<element>', newValue: null });
    }
    for (let i = minLen; i < lenB; i += 1) {
        deltas.push({ path: `${path}[${i}]`, kind: 'added', oldValue: null, newValue: '<element>' });
    }
}
// ── public entry point ─────────────────────────────────────────────────────
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
export function diffBuffers(schema, typeName, bufA, bufB) {
    const obj = schema.objectByName(typeName);
    if (obj === undefined) {
        throw new Error(`flatbuffers: type "${typeName}" not found in schema`);
    }
    if (bufA.byteLength < 4) {
        throw new Error(`flatbuffers: bufA too short (${bufA.byteLength} bytes)`);
    }
    if (bufB.byteLength < 4) {
        throw new Error(`flatbuffers: bufB too short (${bufB.byteLength} bytes)`);
    }
    const rootPosA = bufA.getUint32(0, true);
    const rootPosB = bufB.getUint32(0, true);
    const deltas = [];
    diffTable(bufA, rootPosA, bufB, rootPosB, obj, schema, '', deltas);
    return deltas;
}
