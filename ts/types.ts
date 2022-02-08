import { ByteBuffer } from './byte-buffer.js'
import { Builder } from './builder.js'

export type Offset = number;

export type Table = {
  bb: ByteBuffer
  bb_pos: number
};

export interface IGeneratedObject {
  pack(builder:Builder): Offset
  unpack(): IGeneratedObject
}