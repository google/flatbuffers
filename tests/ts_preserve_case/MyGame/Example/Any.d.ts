import { Monster as MyGame_Example2_Monster } from '../../MyGame/Example2/Monster.js';
import { Monster } from '../../MyGame/Example/Monster.js';
import { TestSimpleTableWithEnum } from '../../MyGame/Example/TestSimpleTableWithEnum.js';
export declare enum Any {
    NONE = 0,
    Monster = 1,
    TestSimpleTableWithEnum = 2,
    MyGame_Example2_Monster = 3
}
export declare function unionToAny(type: Any, accessor: (obj: Monster | MyGame_Example2_Monster | TestSimpleTableWithEnum) => Monster | MyGame_Example2_Monster | TestSimpleTableWithEnum | null): Monster | MyGame_Example2_Monster | TestSimpleTableWithEnum | null;
export declare function unionListToAny(type: Any, accessor: (index: number, obj: Monster | MyGame_Example2_Monster | TestSimpleTableWithEnum) => Monster | MyGame_Example2_Monster | TestSimpleTableWithEnum | null, index: number): Monster | MyGame_Example2_Monster | TestSimpleTableWithEnum | null;
