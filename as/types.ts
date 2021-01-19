import { ByteBuffer } from './byte-buffer';
import { Builder } from './builder';

export type Offset = i32;

export class Table {
  bb: ByteBuffer;
  bb_pos: i32;
}

export interface IGeneratedObject {
  pack(builder: Builder): Offset;
  unpack(): IGeneratedObject;
}
