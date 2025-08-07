import * as flatbuffers from 'flatbuffers';
import { KeyValue, KeyValueT } from '../reflection/KeyValue.js';
import { Type, TypeT } from '../reflection/Type.js';
export declare class Field implements flatbuffers.IUnpackableObject<FieldT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): Field;
    static getRootAsField(bb: flatbuffers.ByteBuffer, obj?: Field): Field;
    static getSizePrefixedRootAsField(bb: flatbuffers.ByteBuffer, obj?: Field): Field;
    name(): string | null;
    name(optionalEncoding: flatbuffers.Encoding): string | Uint8Array | null;
    type(obj?: Type): Type | null;
    id(): number;
    mutate_id(value: number): boolean;
    offset(): number;
    mutate_offset(value: number): boolean;
    default_integer(): bigint;
    mutate_default_integer(value: bigint): boolean;
    default_real(): number;
    mutate_default_real(value: number): boolean;
    deprecated(): boolean;
    mutate_deprecated(value: boolean): boolean;
    required(): boolean;
    mutate_required(value: boolean): boolean;
    key(): boolean;
    mutate_key(value: boolean): boolean;
    attributes(index: number, obj?: KeyValue): KeyValue | null;
    attributes_Length(): number;
    documentation(index: number): string;
    documentation(index: number, optionalEncoding: flatbuffers.Encoding): string | Uint8Array;
    documentation_Length(): number;
    optional(): boolean;
    mutate_optional(value: boolean): boolean;
    /**
     * Number of padding octets to always add after this field. Structs only.
     */
    padding(): number;
    mutate_padding(value: number): boolean;
    /**
     * If the field uses 64-bit offsets.
     */
    offset64(): boolean;
    mutate_offset64(value: boolean): boolean;
    static getFullyQualifiedName(): string;
    static startField(builder: flatbuffers.Builder): void;
    static add_name(builder: flatbuffers.Builder, nameOffset: flatbuffers.Offset): void;
    static add_type(builder: flatbuffers.Builder, typeOffset: flatbuffers.Offset): void;
    static add_id(builder: flatbuffers.Builder, id: number): void;
    static add_offset(builder: flatbuffers.Builder, offset: number): void;
    static add_default_integer(builder: flatbuffers.Builder, default_integer: bigint): void;
    static add_default_real(builder: flatbuffers.Builder, default_real: number): void;
    static add_deprecated(builder: flatbuffers.Builder, deprecated: boolean): void;
    static add_required(builder: flatbuffers.Builder, required: boolean): void;
    static add_key(builder: flatbuffers.Builder, key: boolean): void;
    static add_attributes(builder: flatbuffers.Builder, attributesOffset: flatbuffers.Offset): void;
    static create_attributes_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_attributes_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_documentation(builder: flatbuffers.Builder, documentationOffset: flatbuffers.Offset): void;
    static create_documentation_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_documentation_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_optional(builder: flatbuffers.Builder, optional: boolean): void;
    static add_padding(builder: flatbuffers.Builder, padding: number): void;
    static add_offset64(builder: flatbuffers.Builder, offset64: boolean): void;
    static endField(builder: flatbuffers.Builder): flatbuffers.Offset;
    unpack(): FieldT;
    unpackTo(_o: FieldT): void;
}
export declare class FieldT implements flatbuffers.IGeneratedObject {
    name: string | Uint8Array | null;
    type: TypeT | null;
    id: number;
    offset: number;
    default_integer: bigint;
    default_real: number;
    deprecated: boolean;
    required: boolean;
    key: boolean;
    attributes: (KeyValueT)[];
    documentation: (string)[];
    optional: boolean;
    padding: number;
    offset64: boolean;
    constructor(name?: string | Uint8Array | null, type?: TypeT | null, id?: number, offset?: number, default_integer?: bigint, default_real?: number, deprecated?: boolean, required?: boolean, key?: boolean, attributes?: (KeyValueT)[], documentation?: (string)[], optional?: boolean, padding?: number, offset64?: boolean);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
