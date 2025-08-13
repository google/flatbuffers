import * as flatbuffers from 'flatbuffers';
import { BaseType } from '../reflection/BaseType.js';
export declare class Type implements flatbuffers.IUnpackableObject<TypeT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): Type;
    static getRootAsType(bb: flatbuffers.ByteBuffer, obj?: Type): Type;
    static getSizePrefixedRootAsType(bb: flatbuffers.ByteBuffer, obj?: Type): Type;
    base_type(): BaseType;
    mutate_base_type(value: BaseType): boolean;
    element(): BaseType;
    mutate_element(value: BaseType): boolean;
    index(): number;
    mutate_index(value: number): boolean;
    fixed_length(): number;
    mutate_fixed_length(value: number): boolean;
    /**
     * The size (octets) of the `base_type` field.
     */
    base_size(): number;
    mutate_base_size(value: number): boolean;
    /**
     * The size (octets) of the `element` field, if present.
     */
    element_size(): number;
    mutate_element_size(value: number): boolean;
    static getFullyQualifiedName(): string;
    static startType(builder: flatbuffers.Builder): void;
    static add_base_type(builder: flatbuffers.Builder, base_type: BaseType): void;
    static add_element(builder: flatbuffers.Builder, element: BaseType): void;
    static add_index(builder: flatbuffers.Builder, index: number): void;
    static add_fixed_length(builder: flatbuffers.Builder, fixed_length: number): void;
    static add_base_size(builder: flatbuffers.Builder, base_size: number): void;
    static add_element_size(builder: flatbuffers.Builder, element_size: number): void;
    static endType(builder: flatbuffers.Builder): flatbuffers.Offset;
    static createType(builder: flatbuffers.Builder, base_type: BaseType, element: BaseType, index: number, fixed_length: number, base_size: number, element_size: number): flatbuffers.Offset;
    unpack(): TypeT;
    unpackTo(_o: TypeT): void;
}
export declare class TypeT implements flatbuffers.IGeneratedObject {
    base_type: BaseType;
    element: BaseType;
    index: number;
    fixed_length: number;
    base_size: number;
    element_size: number;
    constructor(base_type?: BaseType, element?: BaseType, index?: number, fixed_length?: number, base_size?: number, element_size?: number);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
