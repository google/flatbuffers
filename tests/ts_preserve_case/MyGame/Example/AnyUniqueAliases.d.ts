import { Monster as MyGame_Example2_Monster } from '../../MyGame/Example2/Monster.js';
import { Monster } from '../../MyGame/Example/Monster.js';
import { TestSimpleTableWithEnum } from '../../MyGame/Example/TestSimpleTableWithEnum.js';
export declare enum AnyUniqueAliases {
    NONE = 0,
    M = 1,
    TS = 2,
    M2 = 3
}
export declare function unionToAnyUniqueAliases(type: AnyUniqueAliases, accessor: (obj: Monster | MyGame_Example2_Monster | TestSimpleTableWithEnum) => Monster | MyGame_Example2_Monster | TestSimpleTableWithEnum | null): Monster | MyGame_Example2_Monster | TestSimpleTableWithEnum | null;
export declare function unionListToAnyUniqueAliases(type: AnyUniqueAliases, accessor: (index: number, obj: Monster | MyGame_Example2_Monster | TestSimpleTableWithEnum) => Monster | MyGame_Example2_Monster | TestSimpleTableWithEnum | null, index: number): Monster | MyGame_Example2_Monster | TestSimpleTableWithEnum | null;
