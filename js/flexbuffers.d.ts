import { Builder } from './flexbuffers/builder';
export { toReference } from './flexbuffers/reference';
export declare function builder(): Builder;
export declare function toObject(buffer: Uint8Array): unknown;
export declare function encode(object: unknown, size?: number, deduplicateStrings?: boolean, deduplicateKeys?: boolean, deduplicateKeyVectors?: boolean): Uint8Array;
