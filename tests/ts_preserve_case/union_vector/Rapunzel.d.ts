import * as flatbuffers from 'flatbuffers';
export declare class Rapunzel implements flatbuffers.IUnpackableObject<RapunzelT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): Rapunzel;
    hair_length(): number;
    mutate_hair_length(value: number): boolean;
    static getFullyQualifiedName(): string;
    static sizeOf(): number;
    static createRapunzel(builder: flatbuffers.Builder, hair_length: number): flatbuffers.Offset;
    unpack(): RapunzelT;
    unpackTo(_o: RapunzelT): void;
}
export declare class RapunzelT implements flatbuffers.IGeneratedObject {
    hair_length: number;
    constructor(hair_length?: number);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
