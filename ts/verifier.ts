import {SIZEOF_INT, SIZEOF_SHORT} from './constants.js';

export enum ErrorKind {
  MissingRequiredField = 'MissingRequiredField',
  RangeOutOfBounds = 'RangeOutOfBounds',
  DepthLimitReached = 'DepthLimitReached',
  TooManyTables = 'TooManyTables',
  ApparentSizeTooLarge = 'ApparentSizeTooLarge',
  Utf8Error = 'Utf8Error',
  MissingNullTerminator = 'MissingNullTerminator',
  Unaligned = 'Unaligned',
  InconsistentUnion = 'InconsistentUnion',
  InvalidVtable = 'InvalidVtable',
  InvalidVectorLength = 'InvalidVectorLength',
  SignedOffsetOutOfBounds = 'SignedOffsetOutOfBounds',
}

export enum TraceKind {
  TableField = 'TableField',
  VectorElement = 'VectorElement',
  UnionVariant = 'UnionVariant',
}

export interface TraceDetail {
  kind: TraceKind;
  name: string;
  index: number;
  position: number;
}

export interface VerificationError {
  kind: ErrorKind;
  field?: string;
  offset: number;
  trace: TraceDetail[];
}

export interface VerifierOptions {
  maxDepth?: number;
  maxTables?: number;
  maxApparentSize?: number;
  ignoreNullTerminator?: boolean;
}

export type VerifyResult<T> =
  | {ok: true; value: T}
  | {ok: false; error: VerificationError};

const DEFAULT_MAX_DEPTH = 64;
const DEFAULT_MAX_TABLES = 1_000_000;
const DEFAULT_MAX_APPARENT_SIZE = 1_073_741_824;

export class Verifier {
  private buf: DataView;
  private opts: Required<VerifierOptions>;
  private depth: number;
  private numTables: number;
  private apparentSize: number;

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

  checkAlignment(pos: number, align: number): void {
    if (align > 1 && pos % align !== 0) {
      throw {
        kind: ErrorKind.Unaligned,
        offset: pos,
        trace: [],
      } satisfies VerificationError;
    }
  }

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

  readUint32(pos: number): number {
    this.checkRange(pos, SIZEOF_INT);
    return this.buf.getUint32(pos, true);
  }

  readInt32(pos: number): number {
    this.checkRange(pos, SIZEOF_INT);
    return this.buf.getInt32(pos, true);
  }

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

  checkTable(tablePos: number): void {
    this.checkVtable(tablePos);
    this.countTable();
    this.pushDepth();
  }

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

  checkScalarField(tablePos: number, vOffset: number, size: number): void {
    const fieldPos = this._resolveVtableField(tablePos, vOffset);
    if (fieldPos !== 0) {
      this.checkRange(fieldPos, size);
    }
  }

  checkOffsetField(tablePos: number, vOffset: number): number {
    return this._resolveVtableField(tablePos, vOffset);
  }

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

  popDepth(): void {
    this.depth -= 1;
  }

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
