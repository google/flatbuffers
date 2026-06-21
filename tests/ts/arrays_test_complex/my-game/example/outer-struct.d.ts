import * as flatbuffers from 'flatbuffers';
import { InnerStruct, InnerStructT } from './inner-struct.js';
export declare class OuterStruct implements flatbuffers.IUnpackableObject<OuterStructT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): OuterStruct;
    a(): boolean;
    b(): number;
    cUnderscore(obj?: InnerStruct): InnerStruct | null;
    d(index: number, obj?: InnerStruct): InnerStruct | null;
    e(obj?: InnerStruct): InnerStruct | null;
    f(index: number): number | null;
    static getFullyQualifiedName(): "MyGame.Example.OuterStruct";
    static sizeOf(): number;
    static createOuterStruct(builder: flatbuffers.Builder, a: boolean, b: number, c_underscore_a: number, c_underscore_b: number[], c_underscore_c: number, c_underscore_d_underscore: bigint, d: (any | InnerStructT)[], e_a: number, e_b: number[], e_c: number, e_d_underscore: bigint, f: number[]): flatbuffers.Offset;
    unpack(): OuterStructT;
    unpackTo(_o: OuterStructT): void;
}
export declare class OuterStructT implements flatbuffers.IGeneratedObject {
    a: boolean;
    b: number;
    cUnderscore: InnerStructT | null;
    d: (InnerStructT)[];
    e: InnerStructT | null;
    f: (number)[];
    constructor(a?: boolean, b?: number, cUnderscore?: InnerStructT | null, d?: (InnerStructT)[], e?: InnerStructT | null, f?: (number)[]);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
