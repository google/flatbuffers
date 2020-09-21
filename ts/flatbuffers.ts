/* eslint-disable @typescript-eslint/no-namespace */
import * as constants from './constants'
import * as types from './types'
import * as utils from './utils'

import { Long as LongClass } from './long'
import { Encoding as EncodingEnum } from './encoding'
import { Builder as BuilderClass } from './builder'
import { ByteBuffer as ByteBufferClass } from './byte-buffer'

export namespace flatbuffers {

    export type Offset = types.Offset;

    export type Table = types.Table;

    export const SIZEOF_SHORT = constants.SIZEOF_SHORT;
    export const SIZEOF_INT = constants.SIZEOF_INT;
    export const FILE_IDENTIFIER_LENGTH = constants.FILE_IDENTIFIER_LENGTH;
    export const SIZE_PREFIX_LENGTH = constants.SIZE_PREFIX_LENGTH;

    export const Encoding = EncodingEnum;

    export const int32 = utils.int32;
    export const float32 = utils.float32;
    export const float64 = utils.float64;
    export const isLittleEndian = utils.isLittleEndian;

    export const Long = LongClass;
    export const Builder = BuilderClass;
    export const ByteBuffer = ByteBufferClass;
}

export default flatbuffers;
