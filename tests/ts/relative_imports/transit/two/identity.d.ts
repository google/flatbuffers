import * as flatbuffers from 'flatbuffers';
export declare class Identity implements flatbuffers.IUnpackableObject<IdentityT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): Identity;
    static getRootAsIdentity(bb: flatbuffers.ByteBuffer, obj?: Identity): Identity;
    static getSizePrefixedRootAsIdentity(bb: flatbuffers.ByteBuffer, obj?: Identity): Identity;
    id(): number;
    static getFullyQualifiedName(): "Transit.Two.Identity";
    static startIdentity(builder: flatbuffers.Builder): void;
    static addId(builder: flatbuffers.Builder, id: number): void;
    static endIdentity(builder: flatbuffers.Builder): flatbuffers.Offset;
    static createIdentity(builder: flatbuffers.Builder, id: number): flatbuffers.Offset;
    unpack(): IdentityT;
    unpackTo(_o: IdentityT): void;
}
export declare class IdentityT implements flatbuffers.IGeneratedObject {
    id: number;
    constructor(id?: number);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
