import { A } from '../UnionUnderlyingType/A.js';
import { B } from '../UnionUnderlyingType/B.js';
import { C } from '../UnionUnderlyingType/C.js';
export declare enum ABC {
    NONE = 0,
    A = 555,
    B = 666,
    C = 777
}
export declare function unionToABC(type: ABC, accessor: (obj: A | B | C) => A | B | C | null): A | B | C | null;
export declare function unionListToABC(type: ABC, accessor: (index: number, obj: A | B | C) => A | B | C | null, index: number): A | B | C | null;
