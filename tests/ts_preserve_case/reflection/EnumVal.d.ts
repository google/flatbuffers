import * as flatbuffers from 'flatbuffers';
import { KeyValue, KeyValueT } from '../reflection/KeyValue.js';
import { Type, TypeT } from '../reflection/Type.js';
export declare class EnumVal implements flatbuffers.IUnpackableObject<EnumValT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): EnumVal;
    static getRootAsEnumVal(bb: flatbuffers.ByteBuffer, obj?: EnumVal): EnumVal;
    static getSizePrefixedRootAsEnumVal(bb: flatbuffers.ByteBuffer, obj?: EnumVal): EnumVal;
    name(): string | null;
    name(optionalEncoding: flatbuffers.Encoding): string | Uint8Array | null;
    value(): bigint;
    mutate_value(value: bigint): boolean;
    union_type(obj?: Type): Type | null;
    documentation(index: number): string;
    documentation(index: number, optionalEncoding: flatbuffers.Encoding): string | Uint8Array;
    documentation_Length(): number;
    attributes(index: number, obj?: KeyValue): KeyValue | null;
    attributes_Length(): number;
    static getFullyQualifiedName(): string;
    static startEnumVal(builder: flatbuffers.Builder): void;
    static add_name(builder: flatbuffers.Builder, nameOffset: flatbuffers.Offset): void;
    static add_value(builder: flatbuffers.Builder, value: bigint): void;
    static add_union_type(builder: flatbuffers.Builder, union_typeOffset: flatbuffers.Offset): void;
    static add_documentation(builder: flatbuffers.Builder, documentationOffset: flatbuffers.Offset): void;
    static create_documentation_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_documentation_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_attributes(builder: flatbuffers.Builder, attributesOffset: flatbuffers.Offset): void;
    static create_attributes_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_attributes_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static endEnumVal(builder: flatbuffers.Builder): flatbuffers.Offset;
    unpack(): EnumValT;
    unpackTo(_o: EnumValT): void;
}
export declare class EnumValT implements flatbuffers.IGeneratedObject {
    name: string | Uint8Array | null;
    value: bigint;
    union_type: TypeT | null;
    documentation: (string)[];
    attributes: (KeyValueT)[];
    constructor(name?: string | Uint8Array | null, value?: bigint, union_type?: TypeT | null, documentation?: (string)[], attributes?: (KeyValueT)[]);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
