import * as flatbuffers from 'flatbuffers';
import { AT } from '../UnionUnderlyingType/A.js';
import { ABC } from '../UnionUnderlyingType/ABC.js';
import { BT } from '../UnionUnderlyingType/B.js';
import { CT } from '../UnionUnderlyingType/C.js';
export declare class D implements flatbuffers.IUnpackableObject<DT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): D;
    static getRootAsD(bb: flatbuffers.ByteBuffer, obj?: D): D;
    static getSizePrefixedRootAsD(bb: flatbuffers.ByteBuffer, obj?: D): D;
    test_union_type(): ABC;
    test_union<T extends flatbuffers.Table>(obj: any): any | null;
    test_vector_of_union_type(index: number): ABC | null;
    test_vector_of_union_type_Length(): number;
    test_vector_of_union_type_Array(): Int32Array | null;
    test_vector_of_union(index: number, obj: any): any | null;
    test_vector_of_union_Length(): number;
    static getFullyQualifiedName(): string;
    static startD(builder: flatbuffers.Builder): void;
    static add_test_union_type(builder: flatbuffers.Builder, test_union_type: ABC): void;
    static add_test_union(builder: flatbuffers.Builder, test_unionOffset: flatbuffers.Offset): void;
    static add_test_vector_of_union_type(builder: flatbuffers.Builder, test_vector_of_union_typeOffset: flatbuffers.Offset): void;
    static create_test_vector_of_union_type_Vector(builder: flatbuffers.Builder, data: ABC[]): flatbuffers.Offset;
    static start_test_vector_of_union_type_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_test_vector_of_union(builder: flatbuffers.Builder, test_vector_of_unionOffset: flatbuffers.Offset): void;
    static create_test_vector_of_union_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_test_vector_of_union_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static endD(builder: flatbuffers.Builder): flatbuffers.Offset;
    static createD(builder: flatbuffers.Builder, test_union_type: ABC, test_unionOffset: flatbuffers.Offset, test_vector_of_union_typeOffset: flatbuffers.Offset, test_vector_of_unionOffset: flatbuffers.Offset): flatbuffers.Offset;
    unpack(): DT;
    unpackTo(_o: DT): void;
}
export declare class DT implements flatbuffers.IGeneratedObject {
    test_union_type: ABC;
    test_union: AT | BT | CT | null;
    test_vector_of_union_type: (ABC)[];
    test_vector_of_union: (AT | BT | CT)[];
    constructor(test_union_type?: ABC, test_union?: AT | BT | CT | null, test_vector_of_union_type?: (ABC)[], test_vector_of_union?: (AT | BT | CT)[]);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
