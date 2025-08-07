import * as flatbuffers from 'flatbuffers';
import { KeyValue, KeyValueT } from '../reflection/KeyValue.js';
import { RPCCall, RPCCallT } from '../reflection/RPCCall.js';
export declare class Service implements flatbuffers.IUnpackableObject<ServiceT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): Service;
    static getRootAsService(bb: flatbuffers.ByteBuffer, obj?: Service): Service;
    static getSizePrefixedRootAsService(bb: flatbuffers.ByteBuffer, obj?: Service): Service;
    name(): string | null;
    name(optionalEncoding: flatbuffers.Encoding): string | Uint8Array | null;
    calls(index: number, obj?: RPCCall): RPCCall | null;
    calls_Length(): number;
    attributes(index: number, obj?: KeyValue): KeyValue | null;
    attributes_Length(): number;
    documentation(index: number): string;
    documentation(index: number, optionalEncoding: flatbuffers.Encoding): string | Uint8Array;
    documentation_Length(): number;
    /**
     * File that this Service is declared in.
     */
    declaration_file(): string | null;
    declaration_file(optionalEncoding: flatbuffers.Encoding): string | Uint8Array | null;
    static getFullyQualifiedName(): string;
    static startService(builder: flatbuffers.Builder): void;
    static add_name(builder: flatbuffers.Builder, nameOffset: flatbuffers.Offset): void;
    static add_calls(builder: flatbuffers.Builder, callsOffset: flatbuffers.Offset): void;
    static create_calls_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_calls_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_attributes(builder: flatbuffers.Builder, attributesOffset: flatbuffers.Offset): void;
    static create_attributes_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_attributes_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_documentation(builder: flatbuffers.Builder, documentationOffset: flatbuffers.Offset): void;
    static create_documentation_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_documentation_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_declaration_file(builder: flatbuffers.Builder, declaration_fileOffset: flatbuffers.Offset): void;
    static endService(builder: flatbuffers.Builder): flatbuffers.Offset;
    static createService(builder: flatbuffers.Builder, nameOffset: flatbuffers.Offset, callsOffset: flatbuffers.Offset, attributesOffset: flatbuffers.Offset, documentationOffset: flatbuffers.Offset, declaration_fileOffset: flatbuffers.Offset): flatbuffers.Offset;
    unpack(): ServiceT;
    unpackTo(_o: ServiceT): void;
}
export declare class ServiceT implements flatbuffers.IGeneratedObject {
    name: string | Uint8Array | null;
    calls: (RPCCallT)[];
    attributes: (KeyValueT)[];
    documentation: (string)[];
    declaration_file: string | Uint8Array | null;
    constructor(name?: string | Uint8Array | null, calls?: (RPCCallT)[], attributes?: (KeyValueT)[], documentation?: (string)[], declaration_file?: string | Uint8Array | null);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
