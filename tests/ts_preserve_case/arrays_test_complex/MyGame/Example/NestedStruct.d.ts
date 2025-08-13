import * as flatbuffers from 'flatbuffers';
import { OuterStruct, OuterStructT } from '../../MyGame/Example/OuterStruct.js';
import { TestEnum } from '../../MyGame/Example/TestEnum.js';
export declare class NestedStruct implements flatbuffers.IUnpackableObject<NestedStructT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): NestedStruct;
    a(index: number): number | null;
    b(): TestEnum;
    c_underscore(index: number): TestEnum | null;
    d_outer(index: number, obj?: OuterStruct): OuterStruct | null;
    e(index: number): bigint | null;
    static getFullyQualifiedName(): string;
    static sizeOf(): number;
    static createNestedStruct(builder: flatbuffers.Builder, a: number[] | null, b: TestEnum, c_underscore: number[] | null, d_outer: (any | OuterStructT)[] | null, e: bigint[] | null): flatbuffers.Offset;
    unpack(): NestedStructT;
    unpackTo(_o: NestedStructT): void;
}
export declare class NestedStructT implements flatbuffers.IGeneratedObject {
    a: (number)[];
    b: TestEnum;
    c_underscore: (TestEnum)[];
    d_outer: (OuterStructT)[];
    e: (bigint)[];
    constructor(a?: (number)[], b?: TestEnum, c_underscore?: (TestEnum)[], d_outer?: (OuterStructT)[], e?: (bigint)[]);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
