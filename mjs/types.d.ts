import { ByteBuffer } from './byte-buffer';
import { Builder } from './builder';
export declare type Offset = number;
export declare type Table = {
    bb: ByteBuffer;
    bb_pos: number;
};
export interface IGeneratedObject {
    pack(builder: Builder): Offset;
    unpack(): IGeneratedObject;
}
