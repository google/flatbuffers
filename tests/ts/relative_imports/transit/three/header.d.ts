import * as flatbuffers from 'flatbuffers';
import { Info, InfoT } from '../one/info.js';
import { Identity, IdentityT } from '../two/identity.js';
export declare class Header implements flatbuffers.IUnpackableObject<HeaderT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): Header;
    static getRootAsHeader(bb: flatbuffers.ByteBuffer, obj?: Header): Header;
    static getSizePrefixedRootAsHeader(bb: flatbuffers.ByteBuffer, obj?: Header): Header;
    info(obj?: Info): Info | null;
    id(obj?: Identity): Identity | null;
    static getFullyQualifiedName(): "Transit.Three.Header";
    static startHeader(builder: flatbuffers.Builder): void;
    static addInfo(builder: flatbuffers.Builder, infoOffset: flatbuffers.Offset): void;
    static addId(builder: flatbuffers.Builder, idOffset: flatbuffers.Offset): void;
    static endHeader(builder: flatbuffers.Builder): flatbuffers.Offset;
    static finishHeaderBuffer(builder: flatbuffers.Builder, offset: flatbuffers.Offset): void;
    static finishSizePrefixedHeaderBuffer(builder: flatbuffers.Builder, offset: flatbuffers.Offset): void;
    unpack(): HeaderT;
    unpackTo(_o: HeaderT): void;
}
export declare class HeaderT implements flatbuffers.IGeneratedObject {
    info: InfoT | null;
    id: IdentityT | null;
    constructor(info?: InfoT | null, id?: IdentityT | null);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
