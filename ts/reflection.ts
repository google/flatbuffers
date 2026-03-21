// ts/reflection.ts — separate entry point for dev tools
// Dynamic field access using binary schema (.bfbs) files.
// This file is standalone: no imports from other flatbuffers ts modules.
// Uses raw DataView + TextDecoder for minimal bundle size.

const textDecoder = new TextDecoder();

export enum ReflectionBaseType {
  None = 0,
  UType = 1,
  Bool = 2,
  Byte = 3,
  UByte = 4,
  Short = 5,
  UShort = 6,
  Int = 7,
  UInt = 8,
  Long = 9,
  ULong = 10,
  Float = 11,
  Double = 12,
  String = 13,
  Vector = 14,
  Obj = 15,
  Union = 16,
  Array = 17,
  Vector64 = 18,
}

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
function readVtableEntry(buf: DataView, tablePos: number, voffset: number): number {
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
function readString(buf: DataView, fieldPos: number): string {
  const uoffset = buf.getUint32(fieldPos, true);
  const strStart = fieldPos + uoffset;
  const length = buf.getUint32(strStart, true);
  const bytes = new Uint8Array(buf.buffer, buf.byteOffset + strStart + 4, length);
  return textDecoder.decode(bytes);
}

/**
 * Follow a UOffset at fieldPos to get the child table's absolute position.
 */
function readTable(buf: DataView, fieldPos: number): number {
  return fieldPos + buf.getUint32(fieldPos, true);
}

/**
 * Read a vector at the given field position.
 * Returns [count, dataStart] where dataStart is the byte position of element 0.
 */
function readVector(buf: DataView, fieldPos: number): [number, number] {
  const uoffset = buf.getUint32(fieldPos, true);
  const vecStart = fieldPos + uoffset;
  const count = buf.getUint32(vecStart, true);
  return [count, vecStart + 4];
}

/**
 * Read element i of a vector of tables.
 * Each element slot is a 4-byte UOffset; the table is at slot + uoffset.
 */
function readVectorTableElement(buf: DataView, dataStart: number, index: number): number {
  const elemPos = dataStart + index * 4;
  return elemPos + buf.getUint32(elemPos, true);
}

// ── ReflectionType ─────────────────────────────────────────────────────────

export class ReflectionType {
  private readonly buf: DataView;
  private readonly tablePos: number;

  constructor(buf: DataView, tablePos: number) {
    this.buf = buf;
    this.tablePos = tablePos;
  }

  baseType(): ReflectionBaseType {
    const entry = readVtableEntry(this.buf, this.tablePos, 4);
    if (entry === 0) return ReflectionBaseType.None;
    return this.buf.getUint8(this.tablePos + entry) as ReflectionBaseType;
  }

  element(): ReflectionBaseType {
    const entry = readVtableEntry(this.buf, this.tablePos, 6);
    if (entry === 0) return ReflectionBaseType.None;
    return this.buf.getUint8(this.tablePos + entry) as ReflectionBaseType;
  }

  index(): number {
    const entry = readVtableEntry(this.buf, this.tablePos, 8);
    if (entry === 0) return -1;
    return this.buf.getInt32(this.tablePos + entry, true);
  }
}

// ── ReflectionField ────────────────────────────────────────────────────────

export class ReflectionField {
  private readonly buf: DataView;
  private readonly tablePos: number;

  constructor(buf: DataView, tablePos: number) {
    this.buf = buf;
    this.tablePos = tablePos;
  }

  name(): string {
    const entry = readVtableEntry(this.buf, this.tablePos, 4);
    if (entry === 0) return '';
    return readString(this.buf, this.tablePos + entry);
  }

  type(): ReflectionType | null {
    const entry = readVtableEntry(this.buf, this.tablePos, 6);
    if (entry === 0) return null;
    const typeTablePos = readTable(this.buf, this.tablePos + entry);
    return new ReflectionType(this.buf, typeTablePos);
  }

  offset(): number {
    const entry = readVtableEntry(this.buf, this.tablePos, 10);
    if (entry === 0) return 0;
    return this.buf.getUint16(this.tablePos + entry, true);
  }

  defaultInteger(): bigint {
    const entry = readVtableEntry(this.buf, this.tablePos, 12);
    if (entry === 0) return 0n;
    const fieldPos = this.tablePos + entry;
    const lo = BigInt(this.buf.getUint32(fieldPos, true));
    const hi = BigInt(this.buf.getInt32(fieldPos + 4, true));
    return BigInt.asIntN(64, lo + (hi << 32n));
  }

  defaultReal(): number {
    const entry = readVtableEntry(this.buf, this.tablePos, 14);
    if (entry === 0) return 0;
    return this.buf.getFloat64(this.tablePos + entry, true);
  }

  required(): boolean {
    const entry = readVtableEntry(this.buf, this.tablePos, 18);
    if (entry === 0) return false;
    return this.buf.getUint8(this.tablePos + entry) !== 0;
  }
}

// ── ReflectionObject ───────────────────────────────────────────────────────

export class ReflectionObject {
  private readonly buf: DataView;
  private readonly tablePos: number;

  constructor(buf: DataView, tablePos: number) {
    this.buf = buf;
    this.tablePos = tablePos;
  }

  name(): string {
    const entry = readVtableEntry(this.buf, this.tablePos, 4);
    if (entry === 0) return '';
    return readString(this.buf, this.tablePos + entry);
  }

  fields(): ReflectionField[] {
    const entry = readVtableEntry(this.buf, this.tablePos, 6);
    if (entry === 0) return [];
    const [count, dataStart] = readVector(this.buf, this.tablePos + entry);
    const result: ReflectionField[] = [];
    for (let i = 0; i < count; i++) {
      const fieldTablePos = readVectorTableElement(this.buf, dataStart, i);
      result.push(new ReflectionField(this.buf, fieldTablePos));
    }
    return result;
  }

  fieldByName(name: string): ReflectionField | undefined {
    return this.fields().find(f => f.name() === name);
  }

  isStruct(): boolean {
    const entry = readVtableEntry(this.buf, this.tablePos, 8);
    if (entry === 0) return false;
    return this.buf.getUint8(this.tablePos + entry) !== 0;
  }

  byteSize(): number {
    const entry = readVtableEntry(this.buf, this.tablePos, 12);
    if (entry === 0) return 0;
    return this.buf.getInt32(this.tablePos + entry, true);
  }
}

// ── ReflectionSchema ───────────────────────────────────────────────────────

export class ReflectionSchema {
  private readonly buf: DataView;
  private readonly rootTablePos: number;

  constructor(bfbs: Uint8Array) {
    this.buf = new DataView(bfbs.buffer, bfbs.byteOffset, bfbs.byteLength);
    const uoffset = this.buf.getUint32(0, true);
    this.rootTablePos = uoffset;
  }

  rootTable(): ReflectionObject | null {
    const entry = readVtableEntry(this.buf, this.rootTablePos, 12);
    if (entry === 0) return null;
    const objTablePos = readTable(this.buf, this.rootTablePos + entry);
    return new ReflectionObject(this.buf, objTablePos);
  }

  objects(): ReflectionObject[] {
    const entry = readVtableEntry(this.buf, this.rootTablePos, 4);
    if (entry === 0) return [];
    const [count, dataStart] = readVector(this.buf, this.rootTablePos + entry);
    const result: ReflectionObject[] = [];
    for (let i = 0; i < count; i++) {
      const objTablePos = readVectorTableElement(this.buf, dataStart, i);
      result.push(new ReflectionObject(this.buf, objTablePos));
    }
    return result;
  }

  objectByName(name: string): ReflectionObject | undefined {
    return this.objects().find(o => o.name() === name);
  }
}

// ── vtable lookup for dynamic field access ─────────────────────────────────

/**
 * Perform a vtable lookup on dataBuf at tablePos using the schema field's voffset.
 * Returns the absolute data position, or null if the field is absent.
 */
function resolveFieldPos(
  dataBuf: DataView,
  tablePos: number,
  field: ReflectionField,
): number | null {
  const voffset = field.offset();
  if (voffset === 0) return null;
  const entry = readVtableEntry(dataBuf, tablePos, voffset);
  if (entry === 0) return null;
  return tablePos + entry;
}

// ── dynamic field accessors ────────────────────────────────────────────────

export function getFieldString(
  dataBuf: DataView,
  tablePos: number,
  field: ReflectionField,
): string | null {
  const fieldPos = resolveFieldPos(dataBuf, tablePos, field);
  if (fieldPos === null) return null;
  return readString(dataBuf, fieldPos);
}

export function getFieldInt(
  dataBuf: DataView,
  tablePos: number,
  field: ReflectionField,
): number | bigint | null {
  const fieldPos = resolveFieldPos(dataBuf, tablePos, field);
  if (fieldPos === null) return null;
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
      return BigInt.asUintN(64, lo + (hi << 32n));
    }
    case ReflectionBaseType.Long: {
      const lo = BigInt(dataBuf.getUint32(fieldPos, true));
      const hi = BigInt(dataBuf.getInt32(fieldPos + 4, true));
      return BigInt.asIntN(64, lo + (hi << 32n));
    }
    default:
      return dataBuf.getInt32(fieldPos, true);
  }
}

export function getFieldFloat(
  dataBuf: DataView,
  tablePos: number,
  field: ReflectionField,
): number | null {
  const fieldPos = resolveFieldPos(dataBuf, tablePos, field);
  if (fieldPos === null) return null;
  const fieldType = field.type();
  const base = fieldType !== null ? fieldType.baseType() : ReflectionBaseType.Float;
  if (base === ReflectionBaseType.Double) {
    return dataBuf.getFloat64(fieldPos, true);
  }
  return dataBuf.getFloat32(fieldPos, true);
}

export function getFieldBool(
  dataBuf: DataView,
  tablePos: number,
  field: ReflectionField,
): boolean | null {
  const fieldPos = resolveFieldPos(dataBuf, tablePos, field);
  if (fieldPos === null) return null;
  return dataBuf.getUint8(fieldPos) !== 0;
}

// ── convenience loader ─────────────────────────────────────────────────────

export function loadSchema(bfbs: Uint8Array): ReflectionSchema {
  return new ReflectionSchema(bfbs);
}
