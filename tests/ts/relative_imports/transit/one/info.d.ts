import * as flatbuffers from 'flatbuffers';
export declare class Info implements flatbuffers.IUnpackableObject<InfoT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): Info;
    static getRootAsInfo(bb: flatbuffers.ByteBuffer, obj?: Info): Info;
    static getSizePrefixedRootAsInfo(bb: flatbuffers.ByteBuffer, obj?: Info): Info;
    timestamp(): bigint;
    static getFullyQualifiedName(): "Transit.One.Info";
    static startInfo(builder: flatbuffers.Builder): void;
    static addTimestamp(builder: flatbuffers.Builder, timestamp: bigint): void;
    static endInfo(builder: flatbuffers.Builder): flatbuffers.Offset;
    static createInfo(builder: flatbuffers.Builder, timestamp: bigint): flatbuffers.Offset;
    unpack(): InfoT;
    unpackTo(_o: InfoT): void;
}
export declare class InfoT implements flatbuffers.IGeneratedObject {
    timestamp: bigint;
    constructor(timestamp?: bigint);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
