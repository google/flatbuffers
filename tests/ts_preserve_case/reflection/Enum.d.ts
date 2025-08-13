import * as flatbuffers from 'flatbuffers';
import { EnumVal, EnumValT } from '../reflection/EnumVal.js';
import { KeyValue, KeyValueT } from '../reflection/KeyValue.js';
import { Type, TypeT } from '../reflection/Type.js';
export declare class Enum implements flatbuffers.IUnpackableObject<EnumT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): Enum;
    static getRootAsEnum(bb: flatbuffers.ByteBuffer, obj?: Enum): Enum;
    static getSizePrefixedRootAsEnum(bb: flatbuffers.ByteBuffer, obj?: Enum): Enum;
    name(): string | null;
    name(optionalEncoding: flatbuffers.Encoding): string | Uint8Array | null;
    values(index: number, obj?: EnumVal): EnumVal | null;
    values_Length(): number;
    is_union(): boolean;
    mutate_is_union(value: boolean): boolean;
    underlying_type(obj?: Type): Type | null;
    attributes(index: number, obj?: KeyValue): KeyValue | null;
    attributes_Length(): number;
    documentation(index: number): string;
    documentation(index: number, optionalEncoding: flatbuffers.Encoding): string | Uint8Array;
    documentation_Length(): number;
    /**
     * File that this Enum is declared in.
     */
    declaration_file(): string | null;
    declaration_file(optionalEncoding: flatbuffers.Encoding): string | Uint8Array | null;
    static getFullyQualifiedName(): string;
    static startEnum(builder: flatbuffers.Builder): void;
    static add_name(builder: flatbuffers.Builder, nameOffset: flatbuffers.Offset): void;
    static add_values(builder: flatbuffers.Builder, valuesOffset: flatbuffers.Offset): void;
    static create_values_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_values_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_is_union(builder: flatbuffers.Builder, is_union: boolean): void;
    static add_underlying_type(builder: flatbuffers.Builder, underlying_typeOffset: flatbuffers.Offset): void;
    static add_attributes(builder: flatbuffers.Builder, attributesOffset: flatbuffers.Offset): void;
    static create_attributes_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_attributes_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_documentation(builder: flatbuffers.Builder, documentationOffset: flatbuffers.Offset): void;
    static create_documentation_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_documentation_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_declaration_file(builder: flatbuffers.Builder, declaration_fileOffset: flatbuffers.Offset): void;
    static endEnum(builder: flatbuffers.Builder): flatbuffers.Offset;
    unpack(): EnumT;
    unpackTo(_o: EnumT): void;
}
export declare class EnumT implements flatbuffers.IGeneratedObject {
    name: string | Uint8Array | null;
    values: (EnumValT)[];
    is_union: boolean;
    underlying_type: TypeT | null;
    attributes: (KeyValueT)[];
    documentation: (string)[];
    declaration_file: string | Uint8Array | null;
    constructor(name?: string | Uint8Array | null, values?: (EnumValT)[], is_union?: boolean, underlying_type?: TypeT | null, attributes?: (KeyValueT)[], documentation?: (string)[], declaration_file?: string | Uint8Array | null);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
