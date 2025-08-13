import * as flatbuffers from 'flatbuffers';
import { Field, FieldT } from '../reflection/Field.js';
import { KeyValue, KeyValueT } from '../reflection/KeyValue.js';
export declare class Object_ implements flatbuffers.IUnpackableObject<Object_T> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): Object_;
    static getRootAsObject(bb: flatbuffers.ByteBuffer, obj?: Object_): Object_;
    static getSizePrefixedRootAsObject(bb: flatbuffers.ByteBuffer, obj?: Object_): Object_;
    name(): string | null;
    name(optionalEncoding: flatbuffers.Encoding): string | Uint8Array | null;
    fields(index: number, obj?: Field): Field | null;
    fields_Length(): number;
    is_struct(): boolean;
    mutate_is_struct(value: boolean): boolean;
    minalign(): number;
    mutate_minalign(value: number): boolean;
    bytesize(): number;
    mutate_bytesize(value: number): boolean;
    attributes(index: number, obj?: KeyValue): KeyValue | null;
    attributes_Length(): number;
    documentation(index: number): string;
    documentation(index: number, optionalEncoding: flatbuffers.Encoding): string | Uint8Array;
    documentation_Length(): number;
    /**
     * File that this Object is declared in.
     */
    declaration_file(): string | null;
    declaration_file(optionalEncoding: flatbuffers.Encoding): string | Uint8Array | null;
    static getFullyQualifiedName(): string;
    static startObject(builder: flatbuffers.Builder): void;
    static add_name(builder: flatbuffers.Builder, nameOffset: flatbuffers.Offset): void;
    static add_fields(builder: flatbuffers.Builder, fieldsOffset: flatbuffers.Offset): void;
    static create_fields_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_fields_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_is_struct(builder: flatbuffers.Builder, is_struct: boolean): void;
    static add_minalign(builder: flatbuffers.Builder, minalign: number): void;
    static add_bytesize(builder: flatbuffers.Builder, bytesize: number): void;
    static add_attributes(builder: flatbuffers.Builder, attributesOffset: flatbuffers.Offset): void;
    static create_attributes_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_attributes_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_documentation(builder: flatbuffers.Builder, documentationOffset: flatbuffers.Offset): void;
    static create_documentation_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_documentation_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_declaration_file(builder: flatbuffers.Builder, declaration_fileOffset: flatbuffers.Offset): void;
    static endObject(builder: flatbuffers.Builder): flatbuffers.Offset;
    static createObject(builder: flatbuffers.Builder, nameOffset: flatbuffers.Offset, fieldsOffset: flatbuffers.Offset, is_struct: boolean, minalign: number, bytesize: number, attributesOffset: flatbuffers.Offset, documentationOffset: flatbuffers.Offset, declaration_fileOffset: flatbuffers.Offset): flatbuffers.Offset;
    unpack(): Object_T;
    unpackTo(_o: Object_T): void;
}
export declare class Object_T implements flatbuffers.IGeneratedObject {
    name: string | Uint8Array | null;
    fields: (FieldT)[];
    is_struct: boolean;
    minalign: number;
    bytesize: number;
    attributes: (KeyValueT)[];
    documentation: (string)[];
    declaration_file: string | Uint8Array | null;
    constructor(name?: string | Uint8Array | null, fields?: (FieldT)[], is_struct?: boolean, minalign?: number, bytesize?: number, attributes?: (KeyValueT)[], documentation?: (string)[], declaration_file?: string | Uint8Array | null);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
