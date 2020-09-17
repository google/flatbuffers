import { ByteBuffer } from './byte-buffer'
import { Builder } from './builder'

export type Offset = number;

export type Table = {
  bb: ByteBuffer
  bb_pos: number
};

export interface IGeneratedObject {
  pack(builder:Builder): Offset
  unpack(): IGeneratedObject
}