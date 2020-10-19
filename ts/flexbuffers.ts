/* eslint-disable @typescript-eslint/no-namespace */
import { Builder } from './flexbuffers/builder'
import { toReference as toReferenceFunction } from './flexbuffers/reference';

export function builder(): Builder {
    return new Builder();
}

export function toObject(buffer: Uint8Array): unknown {
    return toReferenceFunction(buffer).toObject();
}

export function encode(object: unknown, size = 2048, deduplicateStrings = true, deduplicateKeys = true, deduplicateKeyVectors = true): Uint8Array {
    const builder = new Builder(size > 0 ? size : 2048, deduplicateStrings, deduplicateKeys, deduplicateKeyVectors);
    builder.add(object);
    return builder.finish();
}

const builderFunction = builder
const toObjectFunction = toObject
const encodeFunction = encode

export namespace flexbuffers {
    export const builder = builderFunction;
    export const toObject = toObjectFunction;
    export const encode = encodeFunction;
    export const toReference = toReferenceFunction;
}

export default flexbuffers;
